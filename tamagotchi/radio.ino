// =====================================================================
//  RADIO — parla con la radio ESP-NOW (come sensori.ino parla con i sensori).
//  Ogni UNISONO_CHIAMATA_S secondi manda a tutti una richiesta di unisono e ascolta
//  sempre quelle delle altre stelle. Scrive l'elenco stelleVicine; non disegna.
// =====================================================================

const uint8_t INDIRIZZO_TUTTI[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };   // "broadcast": il messaggio va a chiunque ascolti

// Accende la radio. Se qualcosa non va lo scrive sul Monitor seriale e la stella va avanti senza radio.
void avviaRadio() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(UNISONO_CANALE_RADIO, WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() != ESP_OK) { Serial.println("radio: ESP-NOW non partito"); return; }
  esp_now_register_recv_cb(riceviMessaggioUnisono);
  esp_now_peer_info_t tutti = {};
  memcpy(tutti.peer_addr, INDIRIZZO_TUTTI, 6);
  tutti.channel = 0;            // il canale corrente
  tutti.ifidx = WIFI_IF_STA;
  tutti.encrypt = false;
  radioOk = esp_now_add_peer(&tutti) == ESP_OK;
  esp_wifi_get_mac(WIFI_IF_STA, mioMac);   // il proprio indirizzo: serve a scegliere il "direttore" dell'orchestra
  Serial.println(radioOk ? "radio: pronta" : "radio: errore nell'aggiungere il broadcast");
}

// Arriva un messaggio. Questa funzione gira "di nascosto", fuori dal loop:
// fa solo copie veloci, chiuse dal lucchetto.
void riceviMessaggioUnisono(const esp_now_recv_info_t* info, const uint8_t* dati, int lunghezza) {
  if (lunghezza != sizeof(MessaggioUnisono) || memcmp(dati, "UNIS", 4) != 0) return;
  MessaggioUnisono m;
  memcpy(&m, dati, sizeof(m));
  if (m.versione != UNISONO_VERSIONE_MESSAGGIO) return;
  uint32_t ora = millis();
  portENTER_CRITICAL(&lucchettoVicine);
  int i = 0;
  while (i < numeroStelleVicine && memcmp(stelleVicine[i].mac, info->src_addr, 6) != 0) i++;
  if (i == numeroStelleVicine && numeroStelleVicine < UNISONO_MAX_STELLE) {   // stella nuova
    StellaVicina& nuova = stelleVicine[numeroStelleVicine++];
    memcpy(nuova.mac, info->src_addr, 6);
    nuova.conferme = 0;
    nuova.confermataMs = 0;
  }
  if (i < numeroStelleVicine) {
    StellaVicina& v = stelleVicine[i];
    memcpy(v.nome, m.nome, sizeof(v.nome));
    v.nome[sizeof(v.nome) - 1] = 0;
    v.colore = m.colore;
    v.punte = m.punte;
    v.vita = m.vita;
    v.affilatura = m.affilatura;
    v.disponibile = m.disponibile != 0;
    v.differenzaMs = (int32_t)(m.accesaDaMs - ora);   // il messaggio ci mette pochi ms ad arrivare: basta per andare a tempo
    memcpy(v.ritmo, m.ritmo, sizeof(v.ritmo));
    v.attesaDaMs = m.attesaDaMs;
    v.rssi = info->rx_ctrl->rssi;
    v.ultimoMs = ora;
    if (v.rssi >= UNISONO_RSSI_SOGLIA) {           // segnale forte: una conferma in più
      if (v.conferme < UNISONO_CONFERME) v.conferme++;
      if (v.conferme >= UNISONO_CONFERME && v.confermataMs == 0) v.confermataMs = ora;
    } else {                                       // segnale debole: si riparte da zero
      v.conferme = 0;
      v.confermataMs = 0;
    }
  }
  portEXIT_CRITICAL(&lucchettoVicine);
}

// Una stella è "vicina" quando ha mandato UNISONO_CONFERME messaggi forti di fila
bool stellaVicinaConfermata(const StellaVicina& v) {
  return v.conferme >= UNISONO_CONFERME;
}

// Toglie dall'elenco le stelle che tacciono da più di UNISONO_SILENZIO_MAX_S secondi
void dimenticaStelleSilenziose(uint32_t ora) {
  portENTER_CRITICAL(&lucchettoVicine);
  int tenute = 0;
  for (int i = 0; i < numeroStelleVicine; i++) {
    if (ora - stelleVicine[i].ultimoMs <= (uint32_t)(UNISONO_SILENZIO_MAX_S * 1000)) stelleVicine[tenute++] = stelleVicine[i];
  }
  numeroStelleVicine = tenute;
  portEXIT_CRITICAL(&lucchettoVicine);
}

// Si chiama spesso (ogni UNISONO_VERIFICA_MS) quando c'è una stella forte non ancora confermata,
// oppure confermata da meno di UNISONO_ATTESA_S: così anche l'altra stella riceve presto le sue conferme
bool radioInVerifica(uint32_t ora) {
  if (unisonoFase == UNISONO_ATTESA) return true;   // durante l'attesa si chiama spesso: così una stella in più ci trova subito
  bool verifica = false;
  portENTER_CRITICAL(&lucchettoVicine);
  for (int i = 0; i < numeroStelleVicine; i++) {
    const StellaVicina& v = stelleVicine[i];
    if (v.conferme > 0 && v.conferme < UNISONO_CONFERME) verifica = true;
    if (v.confermataMs != 0 && ora - v.confermataMs < (uint32_t)(UNISONO_ATTESA_S * 1000)) verifica = true;
  }
  portEXIT_CRITICAL(&lucchettoVicine);
  return verifica;
}

// Manda a tutti la richiesta di unisono con nome, colore e punte di questa stella
void inviaRichiestaUnisono(uint32_t ora) {
  MessaggioUnisono m = {};
  memcpy(m.firma, "UNIS", 4);
  m.versione = UNISONO_VERSIONE_MESSAGGIO;
  strncpy(m.nome, NOME_PERSONAGGIO, sizeof(m.nome) - 1);
  m.colore = aspetto.colore;
  m.punte = (uint8_t)STELLA_NUMERO_PUNTE;
  m.vita = STELLA_VITA;
  m.affilatura = STELLA_AFFILATURA;
  m.disponibile = stellaDisponibile(ora) ? 1 : 0;
  m.contatore = messaggiInviati;
  m.accesaDaMs = ora;
  copiaRitmo(m.ritmo, ORCHESTRA_RITMO);
  if (unisonoFase != UNISONO_NO) m.attesaDaMs = unisonoInizioMs == 0 ? 1 : unisonoInizioMs;   // 0 vorrebbe dire "non in Unisono"
  if (esp_now_send(INDIRIZZO_TUTTI, (const uint8_t*)&m, sizeof(m)) == ESP_OK) {
    messaggiInviati++;
    Serial.printf("radio: richiesta di unisono inviata (%u)\n", (unsigned)messaggiInviati);
  }
}

// A ogni giro del loop: dimentica chi tace e, se è il momento, manda la richiesta
void aggiornaRadio() {
  if (!radioOk) return;
  uint32_t ora = millis();
  dimenticaStelleSilenziose(ora);
  uint32_t intervallo = radioInVerifica(ora) ? UNISONO_VERIFICA_MS : (uint32_t)(UNISONO_CHIAMATA_S * 1000);
  if (primoInvioFatto && ora - ultimoInvioMs < intervallo) return;   // non è ancora il momento
  primoInvioFatto = true;
  ultimoInvioMs = ora;
  inviaRichiestaUnisono(ora);
}

// Una riga per il Monitor seriale: stato della radio e stelle sentite, con forza del segnale e conferme
void stampaRadio() {
  if (!radioOk) Serial.println("radio: spenta");
  else stampaStelleSentite();
}

// La parte della riga sulla radio: richieste inviate e stelle sentite
void stampaStelleSentite() {
  StellaVicina copia[UNISONO_MAX_STELLE];
  portENTER_CRITICAL(&lucchettoVicine);
  int n = numeroStelleVicine;
  memcpy(copia, stelleVicine, sizeof(copia));
  portEXIT_CRITICAL(&lucchettoVicine);
  uint32_t ora = millis();
  int prossimaS = (int)((UNISONO_CHIAMATA_S * 1000 - (ora - ultimoInvioMs)) / 1000);
  Serial.printf("radio: inviate %u   prossima fra %d s   stelle sentite %d", (unsigned)messaggiInviati, prossimaS < 0 ? 0 : prossimaS, n);
  for (int i = 0; i < n; i++) {
    const StellaVicina& v = copia[i];
    Serial.printf("   | %s segnale %d conferme %d/%d %s%s", v.nome, v.rssi, v.conferme, UNISONO_CONFERME,
                  stellaVicinaConfermata(v) ? "VICINA" : "lontana", v.disponibile ? "" : " (non in salute)");
  }
  Serial.println();
  Serial.printf("unisono: fase %s   stelle nella galassia %d   pausa ancora %d s   disponibile %d\n",
                nomeFaseUnisono(), numeroGalassia + 1, secondiPausaUnisono(ora), stellaDisponibile(ora));
}
