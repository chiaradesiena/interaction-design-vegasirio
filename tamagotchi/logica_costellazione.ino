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

// Dal menu: mette subito n stelle (da 1 a COSTELLAZIONE_NUMERO_STELLE), senza animazione, e le salva.
// Non tocca l'ora dell'ultima stella: la pausa prima della prossima stella resta quella di prima.
void impostaStelleCostellazione(int n) {
  costellazioneStelle = constrain(n, 1, COSTELLAZIONE_NUMERO_STELLE);
  costellazioneStelleMostrate = costellazioneStelle;
  costellazioneTenutaS = 0;
  memoria.putInt("cost_stelle", costellazioneStelle);
  Serial.printf("costellazione dal menu: %d stelle di %d\n", costellazioneStelle, COSTELLAZIONE_NUMERO_STELLE);
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
  if (schermataCorrente != ANIMAZIONE_COSTELLAZIONE) costellazioneStelleMostrate = costellazioneStelle;   // appena partita l'animazione la stella nuova non si vede ancora
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
  if (COSTELLAZIONE_DISEGNATA) { posizioneStellaDisegnata(i, cx, cy, x, y); return; }   // costellazione vera (Sirio): posti della tabella
  float angolo = costellazioneAngolo + 2 * PI * i / n;
  x = cx + (int)(COSTELLAZIONE_RAGGIO_CERCHIO_PX * cosf(angolo));
  y = cy + (int)(COSTELLAZIONE_RAGGIO_CERCHIO_PX * sinf(angolo));
}

// ---- Costellazione disegnata (COSTELLAZIONE_DISEGNATA = true): posti e tratti presi dalle tabelle in config_sirio.h / config_vega.h ----

// Dove sta la stella piccola numero i della tabella: il suo posto, ingrandito di COSTELLAZIONE_SCALA_PX,
// rimpicciolito con la scena (costellazioneZoom) e girato attorno alla principale insieme a tutta la costellazione
void posizioneStellaDisegnata(int i, int cx, int cy, int& x, int& y) {
  float s = COSTELLAZIONE_SCALA_PX * (costellazioneZoom > 0 ? costellazioneZoom : 1.0f);
  float px = COSTELLAZIONE_POSIZIONI[i][0] * s;
  float py = COSTELLAZIONE_POSIZIONI[i][1] * s;
  float ca = cosf(costellazioneAngolo), sa = sinf(costellazioneAngolo);
  x = cx + (int)(px * ca - py * sa);
  y = cy + (int)(px * sa + py * ca);
}

// Con quale stella è unita la stella piccola i (0 = la prima): 0 = la principale, k = la stella piccola numero k.
// "secondo" = il tratto in più che chiude i triangoli (-1 = nessuno). Sul cerchio: ognuna alla precedente, la prima alla principale.
int stellaCollegata(int i, bool secondo) {
  if (!COSTELLAZIONE_DISEGNATA) return secondo ? -1 : i;
  int k = secondo ? COSTELLAZIONE_COLLEGATA_ANCHE_A[i] : COSTELLAZIONE_COLLEGATA_A[i];
  if (k > i) return -1;                                  // unita a una stella che non è ancora comparsa: niente tratto
  return k;
}

// Dove sta la stella k: 0 = la principale (cx, cy), k = la stella piccola numero k
void puntoCostellazione(int k, int n, int cx, int cy, int& x, int& y) {
  if (k <= 0) { x = cx; y = cy; return; }
  posizioneStellaPiccola(k - 1, n, cx, cy, x, y);
}

// Grandezza giusta della scena con n stelle (principale compresa): 1 finché la stella più lontana sta entro
// COSTELLAZIONE_SPAZIO_PX, poi meno. Siccome la costellazione gira, conta la distanza dal centro in ogni direzione.
float zoomGiustoCostellazione(int n) {
  if (!COSTELLAZIONE_DISEGNATA) return 1.0f;
  float lontana = 0;
  for (int i = 0; i < n - 1 && i < COSTELLAZIONE_NUMERO_STELLE - 1; i++) {
    float d = COSTELLAZIONE_SCALA_PX * sqrtf(COSTELLAZIONE_POSIZIONI[i][0] * COSTELLAZIONE_POSIZIONI[i][0]
                                             + COSTELLAZIONE_POSIZIONI[i][1] * COSTELLAZIONE_POSIZIONI[i][1]);
    if (d > lontana) lontana = d;
  }
  if (lontana <= COSTELLAZIONE_SPAZIO_PX) return 1.0f;
  return COSTELLAZIONE_SPAZIO_PX / lontana;
}

// Ogni fotogramma (fuori dall'animazione): la scena va piano verso la grandezza giusta (per esempio se si cambiano le stelle dal menu).
// All'accensione prende subito la grandezza giusta.
void aggiornaZoomCostellazione(float dt) {
  float bersaglio = zoomGiustoCostellazione(costellazioneStelle);
  if (costellazioneZoom < 0) { costellazioneZoom = bersaglio; return; }
  float k = COSTELLAZIONE_ZOOM_VELOCITA * dt;
  if (k > 1.0f) k = 1.0f;
  costellazioneZoom += (bersaglio - costellazioneZoom) * k;
}

// Rimpicciolisce anche la stella principale insieme alla scena (dopo tutti gli altri cambi di grandezza)
void applicaZoomCostellazione() {
  if (costellazioneZoom > 0) aspetto.raggio *= costellazioneZoom;
}
