// =====================================================================
//  LOGICA COSTELLAZIONE — quando la salute resta al 100% per COSTELLAZIONE_TENUTA_S secondi
//  (e l'ultima volta è stata almeno COSTELLAZIONE_PAUSA_S secondi fa) si aggiunge una stella alla costellazione:
//  Cane Maggiore per Sirio, Lira per Vega (posizioni in config_sirio.h / config_vega.h).
//  Legge bisogni e l'orologio (sensi); scrive costellazioneStelle e la salva in memoria.
// =====================================================================

// All'accensione: riprende dalla memoria quante stelle ha la costellazione e quando è stata aggiunta l'ultima.
// Con "azzera" (reset o tocco all'avvio, come per i bisogni) si riparte dalla sola stella principale.
void caricaCostellazione(bool azzera) {
  if (azzera && COSTELLAZIONE_AZZERA_CON_RESET) {
    memoria.remove("cost_stelle");
    memoria.remove("cost_ultima");
    Serial.println("costellazione azzerata");
  }
  costellazioneStelle = memoria.getInt("cost_stelle", 1);
  if (costellazioneStelle < 1 || costellazioneStelle > COSTELLAZIONE_NUMERO_STELLE) costellazioneStelle = 1;
  costellazioneUltimaS = memoria.getUInt("cost_ultima", 0);
  costellazioneStelleMostrate = costellazioneStelle;
  Serial.printf("costellazione: %d stelle di %d\n", costellazioneStelle, COSTELLAZIONE_NUMERO_STELLE);
}

// Secondi passati dall'ultima stella aggiunta. Con l'orologio della board contano anche a dispositivo spento;
// senza orologio contano solo da quando la board è accesa (o dall'ultima stella di questa accensione).
uint32_t secondiDallUltimaStella(uint32_t ora) {
  if (sensi.orologioOk) {
    if (costellazioneUltimaS == 0) return COSTELLAZIONE_PAUSA_S;            // mai aggiunta una stella: si può subito
    if (sensi.orologioS < costellazioneUltimaS) return COSTELLAZIONE_PAUSA_S;   // orologio rimesso indietro: non si resta bloccati
    return sensi.orologioS - costellazioneUltimaS;
  }
  if (costellazioneUltimaS == 0 && !costellazioneInQuestaAccensione) return COSTELLAZIONE_PAUSA_S;
  return (ora - costellazioneUltimaMs) / 1000;
}

// Aggiunge la stella successiva e salva subito (succede di rado, la memoria non si consuma)
void aggiungiStellaCostellazione(uint32_t ora) {
  costellazioneStelle++;
  costellazioneUltimaS = sensi.orologioOk ? max(sensi.orologioS, (uint32_t)1) : 1;   // 0 vuol dire "mai"
  costellazioneUltimaMs = ora;
  costellazioneInQuestaAccensione = true;
  memoria.putInt("cost_stelle", costellazioneStelle);
  memoria.putUInt("cost_ultima", costellazioneUltimaS);
  Serial.printf("costellazione: stella %d di %d\n", costellazioneStelle, COSTELLAZIONE_NUMERO_STELLE);
}

// Ogni fotogramma: conta da quanto la salute è al 100%; dopo COSTELLAZIONE_TENUTA_S secondi,
// se la pausa è finita e la costellazione non è completa, aggiunge una stella.
void aggiornaCostellazione(uint32_t ora, float dt) {
  if (bisogni.salute >= COSTELLAZIONE_SALUTE_SOGLIA) costellazioneTenutaS += dt;
  else costellazioneTenutaS = 0;
  bool completa = costellazioneStelle >= COSTELLAZIONE_NUMERO_STELLE;
  bool pausaFinita = secondiDallUltimaStella(ora) >= COSTELLAZIONE_PAUSA_S;
  if (!completa && pausaFinita && !festaAttiva && costellazioneTenutaS >= COSTELLAZIONE_TENUTA_S) {
    aggiungiStellaCostellazione(ora);
    costellazioneTenutaS = 0;
    iniziaAnimazioneCostellazione(ora);
  }
}

// Minuti che mancano alla prossima stella possibile (per il Monitor seriale)
int minutiAllaProssimaStella(uint32_t ora) {
  uint32_t passati = secondiDallUltimaStella(ora);
  if (passati >= COSTELLAZIONE_PAUSA_S) return 0;
  return (int)((COSTELLAZIONE_PAUSA_S - passati + 59) / 60);
}

// Il cerchio delle stelle piccole gira piano; durante la festa le stelle piccole spariscono (si rimpiccioliscono fino a zero)
// e poi ricompaiono, così non si sovrappongono ai salti della stella principale.
// (Durante l'animazione della nuova stella non viene chiamata: il cerchio resta fermo e i due puntini neri combaciano con le stelle.)
void aggiornaCerchioCostellazione(uint32_t ora, float dt) {
  costellazioneAngolo = fmodf(costellazioneAngolo + 2 * PI * dt / COSTELLAZIONE_GIRO_S, 2 * PI);
  if (schermataCorrente == HOME) costellazioneStelleMostrate = costellazioneStelle;   // appena partita l'animazione la stella nuova non si vede ancora
  else costellazioneStelleMostrate = costellazioneStelle - 1;
  float bersaglio = festaAttiva ? 0.0f : 1.0f;
  float k = COSTELLAZIONE_VELOCITA_SPARIZIONE * dt;
  if (k > 1.0f) k = 1.0f;
  costellazioneVisibile += (bersaglio - costellazioneVisibile) * k;
}

// Aspetto delle stelle piccole: stesso colore, stesse punte e stessa rotazione della principale, grandi il 20%.
// Non prendono la grandezza "di adesso" della principale (respiro, carezza, ascolto), ma quella normale: così restano calme.
void applicaCostellazione() {
  aspettoPiccola = aspetto;
  aspettoPiccola.raggio = STELLA_RAGGIO * COSTELLAZIONE_GRANDEZZA * costellazioneVisibile;
  aspettoPiccola.numeroPunte = STELLA_NUMERO_PUNTE;
  aspettoPiccola.vita = STELLA_VITA;
  aspettoPiccola.affilatura = STELLA_AFFILATURA;
  aspettoPiccola.buco = 0;
  aspettoPiccola.spostamentoX = 0;
  aspettoPiccola.spostamentoY = 0;
  aspettoPiccola.passoGriglia = COSTELLAZIONE_PASSO_GRIGLIA;
}

// Dove sta la stella piccola numero i (0 = la prima) su n: tutte alla stessa distanza fra loro sul cerchio
void posizioneStellaPiccola(int i, int n, int cx, int cy, int& x, int& y) {
  float angolo = costellazioneAngolo + 2 * PI * i / n;
  x = cx + (int)(COSTELLAZIONE_RAGGIO_CERCHIO_PX * cosf(angolo));
  y = cy + (int)(COSTELLAZIONE_RAGGIO_CERCHIO_PX * sinf(angolo));
}
