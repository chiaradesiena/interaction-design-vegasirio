// =====================================================================
//  LOGICA CANE — stato proprio di Sirio: con salute alta, ogni tanto e da sola, la stella gira su se stessa,
//  diventa un cane che zompetta avanti e indietro, si allontana e si avvicina, poi torna stella.
//  Si sceglie con STATO_PROPRIO_TIPO (config_sirio.h / config_vega.h).
//  Legge bisogni, carezza, scossa e voce; scrive caneForma, caneX/caneY, caneRotazione e "aspetto".
// =====================================================================

// Sceglie a caso fra quanti secondi parte il prossimo cane (fra SIRIO_CANE_ATTESA_MIN_S e SIRIO_CANE_ATTESA_MAX_S)
void programmaProssimoCane(uint32_t ora) {
  float attesaS = SIRIO_CANE_ATTESA_MIN_S + casuale01() * (SIRIO_CANE_ATTESA_MAX_S - SIRIO_CANE_ATTESA_MIN_S);
  prossimoCaneMs = ora + (uint32_t)(attesaS * 1000);
}

// Il cane non parte mentre succede già qualcos'altro: festa, stella ferma per poca energia, carezza, scossa o voce
bool caneLibero() {
  return !festaAttiva && !staticaEnergia && !carezzando && !scossaRecente && !voceAttiva && !malataVisibile();   // da malata, o mentre guarisce, niente cane (logica_malata.ino)
}

// Vero quando un bisogno sta salendo davvero grazie a chi gioca con la stella (la socialità non conta):
// carezza abbastanza lunga (nutrimento) o scossa abbastanza lunga (energia)
bool bisognoMigliora() {
  bool nutrimentoSale = carezzando && tempoContinuoS >= CAREZZA_DURATA_MIN_S;
  bool energiaSale = scossaRecente && tempoScossaS >= ENERGIA_SCOSSA_DURATA_MIN_S;
  return nutrimentoSale || energiaSale;
}

// Il cane appena formato è al centro, guarda a destra e ha le zampe dritte
void azzeraPassoCane() {
  caneX = 0; caneY = 0;
  caneProfondita = 1;
  caneVerso = 1;
  caneZampe = 0;
  caneInclinazione = 0;
}

void iniziaCane(uint32_t ora) {
  caneAttivo = true;
  caneInRitorno = false;
  caneInizioMs = ora;
  azzeraPassoCane();
  Serial.println("cane: inizio");
}

// Interruzione: da dove si trova fa una giravolta veloce, torna al centro e torna stella
void interrompiCane(uint32_t ora) {
  caneInRitorno = true;
  caneRitornoMs = ora;
  caneFormaRitorno = caneForma;
  caneGiroRitorno = caneGiro;
  caneAngoloRitorno = caneAngolo;
  caneXRitorno = caneX;
  caneYRitorno = caneY;
  caneProfonditaRitorno = caneProfondita;
  Serial.println("cane: interrotto");
}

void fineCane(uint32_t ora) {
  caneAttivo = false;
  caneInRitorno = false;
  caneForma = 0;
  caneGiro = 0;
  caneAngolo = 0;
  azzeraPassoCane();
  programmaProssimoCane(ora);
  Serial.println("cane: fine");
}

// Il cane che zompetta, t secondi dopo che si è formato (durata = quanto dura questa parte).
// Va avanti e indietro (SIRIO_CANE_TRAVERSATE volte), si allontana (più piccolo e più in alto) e poi si avvicina,
// fa saltelli lenti muovendo le zampe e ogni tanto inciampa (Sirio è impacciato).
void calcolaPassoCane(float t, float durata, float dt) {
  float u = t / durata;                                             // a che punto del percorso (0-1)
  caneUPasso = u;                                                   // serve all'abbaio (logica_abbaio.ino)
  float avvio = min(1.0f, min(t, durata - t) / SIRIO_CANE_AVVIO_S); // parte e si ferma piano: niente scatti col giro
  if (avvio < 0) avvio = 0;
  float onda = PI * SIRIO_CANE_TRAVERSATE * u;
  caneX = SIRIO_CANE_CORSA_X_PX * sinf(onda);                       // parte e arriva al centro
  float versoBersaglio = cosf(onda) >= 0 ? 1.0f : -1.0f;            // guarda dove sta andando
  float passoGirata = 2.0f * dt / SIRIO_CANE_GIRATA_S;              // per girarsi passa da 1 a -1 (diventa sottile e si rigira)
  if (caneVerso < versoBersaglio) caneVerso = min(caneVerso + passoGirata, versoBersaglio);
  else caneVerso = max(caneVerso - passoGirata, versoBersaglio);
  float profondita = -SIRIO_CANE_PROFONDITA * sinf(2 * PI * u);     // prima metà si allontana, seconda si avvicina
  caneProfondita = 1.0f + profondita;
  // saltelli: uno ogni SIRIO_CANE_SALTO_S secondi; le zampe vanno avanti e indietro una volta ogni due saltelli
  float salti = t / SIRIO_CANE_SALTO_S;
  int numeroSalto = (int)salti;
  float s = salti - numeroSalto;                                    // a che punto del saltello (0-1)
  bool inciampo = SIRIO_CANE_INCIAMPO_OGNI > 0 && numeroSalto % SIRIO_CANE_INCIAMPO_OGNI == SIRIO_CANE_INCIAMPO_OGNI - 1;
  float altezza = SIRIO_CANE_SALTO_PX * sinf(PI * s) * (inciampo ? SIRIO_CANE_INCIAMPO_SALTO : 1.0f);
  caneZampe = SIRIO_CANE_ZAMPE_GRADI * sinf(PI * salti) * avvio;
  caneInclinazione = inciampo ? -caneVerso * SIRIO_CANE_INCIAMPO_GRADI * sinf(PI * s) * avvio : 0;   // inciampa: il muso va giù
  caneY = -altezza * caneProfondita * avvio + profondita / SIRIO_CANE_PROFONDITA * SIRIO_CANE_PROFONDITA_Y_PX;   // lontano = più in alto
}

// A che punto è l'animazione: caneForma (0 = stella, 1 = cane), caneGiro (0 = la stella è ferma, 1 = gira al massimo)
// e caneAngolo (giro della sagoma del cane: arriva a 0, cioè dritta, quando il cane è formato).
// Inizio: gira sempre più veloce mentre diventa cane. Fine: gira e torna stella, rallentando fino a fermarsi.
void calcolaFaseCane(uint32_t ora, float dt) {
  const float giroSagoma = 360.0f * SIRIO_CANE_GIRI_SAGOMA;
  if (caneInRitorno) {
    float f = (ora - caneRitornoMs) / 1000.0f / SIRIO_CANE_RITORNO_S;
    if (f >= 1.0f) { fineCane(ora); return; }
    float g = partenzaArrivoDolci(f);
    caneForma = caneFormaRitorno * (1.0f - g);
    caneGiro = max(caneGiroRitorno * (1.0f - f), sinf(PI * f));   // giravolta: accelera e poi si ferma
    caneAngolo = caneAngoloRitorno + giroSagoma * g;
    caneX = caneXRitorno * (1.0f - g);                              // intanto torna al centro
    caneY = caneYRitorno * (1.0f - g);
    caneProfondita = caneProfonditaRitorno + (1.0f - caneProfonditaRitorno) * g;
    caneZampe = 0;
    caneInclinazione = 0;
    return;
  }
  float t = (ora - caneInizioMs) / 1000.0f;
  float durata = SIRIO_CANE_DURATA_S;
  if (t >= durata) { fineCane(ora); return; }
  if (t < SIRIO_CANE_GIRO_INIZIO_S) {                               // stella -> cane
    float f = partenzaArrivoDolci(t / SIRIO_CANE_GIRO_INIZIO_S);
    caneForma = f;
    caneGiro = sinf(PI * f);                                        // accelera, poi rallenta mentre il cane si raddrizza
    caneAngolo = -giroSagoma * (1.0f - f);
  } else if (t > durata - SIRIO_CANE_GIRO_FINE_S) {                 // cane -> stella
    float f = partenzaArrivoDolci((durata - t) / SIRIO_CANE_GIRO_FINE_S);
    caneForma = f;
    caneGiro = sinf(PI * f);                                        // la stella che ricompare gira e poi rallenta fino a fermarsi
    caneAngolo = giroSagoma * (1.0f - f);
    caneX = 0; caneY = 0; caneProfondita = 1; caneZampe = 0; caneInclinazione = 0;
  } else {                                                          // il cane formato: zompetta
    caneForma = 1;
    caneGiro = 0;
    caneAngolo = 0;
    float durataPasso = durata - SIRIO_CANE_GIRO_INIZIO_S - SIRIO_CANE_GIRO_FINE_S;
    calcolaPassoCane(t - SIRIO_CANE_GIRO_INIZIO_S, durataPasso, dt);
  }
}

// Ogni fotogramma: controlla se si è nello stato proprio, fa partire il cane quando è il momento
// e lo ferma se qualcuno migliora un bisogno della stella.
void aggiornaCane(uint32_t ora, float dt) {
  if (STATO_PROPRIO_TIPO != PROPRIO_CANE) return;
  bool proprio = bisogni.salute >= STATO_PROPRIO_SOGLIA_SALUTE;
  if (proprio && !statoProprio) programmaProssimoCane(ora);        // appena entrata nello stato proprio: parte l'attesa
  statoProprio = proprio;
  if (!caneAttivo) {
    if (statoProprio && (int32_t)(ora - prossimoCaneMs) >= 0 && caneLibero()) iniziaCane(ora);
  } else if (!caneInRitorno && bisognoMigliora()) {
    interrompiCane(ora);
  }
  caneUPasso = -1;                                                  // diventa 0-1 solo mentre passeggia
  if (caneAttivo) calcolaFaseCane(ora, dt);
  caneRotazione = fmodf(caneRotazione + SIRIO_CANE_GIRO_GRADI_S * caneGiro * dt, 360);
}

// Applica il cane all'aspetto già calcolato: aggiunge il giro, passa forma, verso e zampe al disegno
// e chiude il buco della fame (il cane è sempre "pieno").
// Il cane non "respira": col respiro i puntini del bordo cambiavano grandezza tutti insieme e il cane sembrava lampeggiare.
void applicaCane() {
  float raggioFermo = STELLA_RAGGIO * scalaCarezza.valore;         // raggio senza respiro
  aspetto.raggio = (aspetto.raggio + (raggioFermo - aspetto.raggio) * caneForma) * caneProfondita;
  aspetto.rotazione += caneRotazione;
  aspetto.buco *= 1.0f - caneForma;
  aspetto.spostamentoX += caneX;
  aspetto.spostamentoY += caneY;
  aspetto.cane = caneForma;
  aspetto.caneAngolo = caneAngolo + caneInclinazione;
  aspetto.caneVerso = caneVerso;
  aspetto.caneZampe = caneZampe;
}
