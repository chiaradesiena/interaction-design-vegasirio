// =====================================================================
//  LOGICA ORCHESTRA — la musica di Unisono. Tutte le stelle seguono l'orologio del "direttore"
//  (la stella con l'indirizzo radio più basso), così i ritmi di 0 e 1 vanno a tempo su ogni dispositivo.
//  Ogni dispositivo suona solo la propria voce e illumina ogni stella quando suona.
//  Legge galassia e stelleVicine; usa suona(); scrive luceMia, galassia[i].luce e orchestra*.
// =====================================================================

static_assert(sizeof(((MessaggioUnisono*)0)->ritmo) == ORCHESTRA_PASSI, "il ritmo nel messaggio deve essere lungo ORCHESTRA_PASSI");

// Copia un ritmo scritto in configurazione nelle 32 caselle; se è più corto, il resto è silenzio
void copiaRitmo(char* destinazione, const char* ritmo) {
  int lunghezza = strnlen(ritmo, ORCHESTRA_PASSI);
  for (int i = 0; i < ORCHESTRA_PASSI; i++) destinazione[i] = i < lunghezza ? ritmo[i] : '0';
}

// In questa casella del ritmo c'è un 1?
bool ritmoSuona(const char* ritmo, int passo) {
  return passo < (int)strnlen(ritmo, ORCHESTRA_PASSI) && ritmo[passo] == '1';
}

// Il direttore è la stella con l'indirizzo radio più basso fra questa e le stelle vere della galassia:
// tutti scelgono la stessa, senza bisogno di mettersi d'accordo. Si prende la differenza fra il suo orologio e il nostro.
void scegliDirettore() {
  const uint8_t* direttore = mioMac;
  for (int i = 0; i < numeroGalassia; i++)
    if (memcmp(galassia[i].mac, direttore, 6) < 0) direttore = galassia[i].mac;
  sonoDirettore = direttore == mioMac;
  if (sonoDirettore) { differenzaDirettoreMs = 0; return; }
  portENTER_CRITICAL(&lucchettoVicine);
  for (int i = 0; i < numeroStelleVicine; i++)
    if (memcmp(stelleVicine[i].mac, direttore, 6) == 0) differenzaDirettoreMs = stelleVicine[i].differenzaMs;
  portEXIT_CRITICAL(&lucchettoVicine);
}

// L'ora dell'orologio del direttore, in ms: è la stessa su tutti i dispositivi
uint32_t tempoComune() {
  return millis() + (uint32_t)differenzaDirettoreMs;
}

// Dove punta il braccio di luce: un giro per battuta, a tempo col direttore (0 = in alto)
float angoloLuceComune() {
  uint32_t battutaMs = ORCHESTRA_PASSO_MS * ORCHESTRA_PASSI;
  return 2 * PI * (tempoComune() % battutaMs) / (float)battutaMs - PI / 2;
}

// Suona una casella del ritmo sul canale ORCHESTRA_CANALE_MIO. Ogni stella della galassia con un 1 si illumina
// (le altre stelle suonano sul loro dispositivo).
void suonaPasso(int passo, uint32_t ora) {
  if (ritmoSuona(ORCHESTRA_RITMO, passo)) {
    suonaSuCanale(ORCHESTRA_NOTE_HZ[orchestraNotaMia], ORCHESTRA_DURATA_MS, ORCHESTRA_CANALE_MIO);
    orchestraNotaMia = (orchestraNotaMia + 1) % ORCHESTRA_NUMERO_NOTE;
    luceMia = 1;
  }
  for (int i = 0; i < numeroGalassia; i++) {
    StellaGalassia& s = galassia[i];
    if (!ritmoSuona(s.ritmo, passo)) continue;
    illuminaStellaGalassia(i);
  }
}

// Volumi dei canali: all'inizio di Unisono la propria voce; alla fine tutti di nuovo pieni,
// così gli altri suoni della stella (che scelgono il canale da soli) restano come prima
void impostaVolumiOrchestra() {
  impostaVolumeCanale(ORCHESTRA_CANALE_MIO, ORCHESTRA_VOLUME_MIO);
}

void ripristinaVolumiOrchestra() {
  impostaVolumeCanale(ORCHESTRA_CANALE_MIO, 255);
  ripristinaVolumiBrano();
}

// Ogni fotogramma durante Unisono: durante la galassia (tranne l'ultimo secondo) suona la casella del ritmo
// quando ne inizia una nuova. Il microfono resta spento per tutto Unisono.
void aggiornaOrchestra(uint32_t ora) {
  suonoInCorso = true;
  scegliDirettore();
  bool attiva = unisonoFase == UNISONO_ANIMAZIONE
                && ora - unisonoAnimazioneMs < (uint32_t)((UNISONO_DURATA_S - ORCHESTRA_FINE_PRIMA_S) * 1000);
  if (MUSICA_TIPO == MUSICA_BRANO) { aggiornaBrano(ora, attiva); return; }   // al posto del ritmo, il brano (logica_brano.ino)
  if (!attiva) { orchestraUltimoPasso = -1; return; }
  int passo = (int)((tempoComune() / ORCHESTRA_PASSO_MS) % ORCHESTRA_PASSI);
  if (passo == orchestraUltimoPasso) return;
  orchestraUltimoPasso = passo;
  suonaPasso(passo, ora);
}

// All'inizio di Unisono l'orchestra riparte dalla prima nota
void azzeraOrchestra() {
  orchestraUltimoPasso = -1;
  orchestraNotaMia = 0;
  luceMia = 0;
  differenzaDirettoreMs = 0;
  sonoDirettore = true;
  impostaVolumiOrchestra();
  azzeraBrano();
}
