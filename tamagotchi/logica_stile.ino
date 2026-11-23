// =====================================================================
//  LOGICA STILE — sceglie come si disegna la stella. Le versioni, in ordine:
//  puntini di sempre -> puntini fitti (FITTO_PASSO_GRIGLIA, solo se STILE_FITTO_ATTIVO) -> raggi di puntini (disegno_raggi.ino).
//  Swipe verso l'alto = versione successiva, swipe verso il basso = precedente. La scelta resta in memoria anche da spenta.
//  Legge "sensi" (swipe); scrive stileStella.
// =====================================================================

// Uno swipe verticale vale se il dito ha fatto abbastanza strada e soprattutto in verticale.
// verso = 1 verso l'alto, -1 verso il basso (sullo schermo la y cresce scendendo).
bool swipeVerticale(int verso) {
  if (!sensi.swipe) return false;
  int dy = -sensi.swipeDY * verso;
  return dy >= STILE_SWIPE_MIN_PX && dy >= abs(sensi.swipeDX) * STILE_SWIPE_VERTICALE;
}

// Questa stella ha questa versione? (i puntini fitti, per ora, solo Vega)
bool stileDisponibile(int s) {
  if (s < 0 || s >= STILI_NUMERO) return false;
  if (s == STILE_FITTO) return STILE_FITTO_ATTIVO;
  return true;
}

// Nome della versione per il Monitor seriale
const char* nomeStile(StileStella s) {
  if (s == STILE_FITTO) return "puntini fitti";
  if (s == STILE_RAGGI) return "raggi";
  return "puntini";
}

// All'accensione: riprende dalla memoria lo stile scelto l'ultima volta (se non c'è o non vale, la stella a puntini)
void caricaStile() {
  int letto = memoria.isKey("stile") ? memoria.getUChar("stile", STILE_PUNTINI) : STILE_PUNTINI;
  stileStella = stileDisponibile(letto) ? (StileStella)letto : STILE_PUNTINI;
  Serial.printf("stile della stella: %s\n", nomeStile(stileStella));
}

// Cambia stile e lo salva (si scrive in memoria solo quando cambia davvero)
void cambiaStile(StileStella nuovo) {
  if (nuovo == stileStella) return;
  stileStella = nuovo;
  memoria.putUChar("stile", (uint8_t)nuovo);
  Serial.printf("stile della stella: %s\n", nomeStile(nuovo));
}

// La versione dopo (verso = 1) o prima (verso = -1), saltando quelle che questa stella non ha.
// Arrivati in fondo (o all'inizio) si resta lì.
StileStella stileVicino(int verso) {
  int s = stileStella;
  for (int k = 0; k < STILI_NUMERO; k++) {
    s += verso;
    if (s < 0 || s >= STILI_NUMERO) break;
    if (stileDisponibile(s)) return (StileStella)s;
  }
  return stileStella;
}

// Ogni fotogramma: guarda gli swipe verticali, solo sulla schermata della stella (non col menu aperto)
// e non alla fine di una carezza lunga, come per il menu.
void aggiornaStile() {
  if (schermataCorrente != HOME) return;
  if (tempoContinuoS > MENU_IGNORA_DOPO_CAREZZA_S) return;
  if (swipeVerticale(1)) cambiaStile(stileVicino(1));
  else if (swipeVerticale(-1)) cambiaStile(stileVicino(-1));
}
