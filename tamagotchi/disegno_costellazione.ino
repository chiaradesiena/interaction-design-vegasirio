// =====================================================================
//  DISEGNO COSTELLAZIONE — le stelle piccole sul cerchio che gira attorno alla stella principale
//  e i tratti sottili che le uniscono. Legge aspettoPiccola, costellazioneStelle, costellazioneAngolo
//  e costellazioneVisibile; non legge i sensori. Si disegna prima della stella principale, che la copre al centro.
// =====================================================================

// Un colore più scuro: k = 1 colore pieno, k = 0 nero
uint16_t scurisciColore(uint16_t c, float k) {
  uint16_t r = (uint16_t)(((c >> 11) & 0x1F) * k + 0.5f);
  uint16_t g = (uint16_t)(((c >> 5) & 0x3F) * k + 0.5f);
  uint16_t b = (uint16_t)((c & 0x1F) * k + 0.5f);
  return (r << 11) | (g << 5) | b;
}

// Tratti: la prima stella piccola è unita alla principale, ogni altra alla precedente;
// a costellazione completa l'ultima si unisce alla prima e il cerchio si chiude. Poi le stelle piccole.
void disegnaCostellazione(M5Canvas& c, int cx, int cy, const AspettoStella& piccola) {
  int n = costellazioneStelleMostrate - 1;               // quante stelle piccole
  if (n <= 0 || costellazioneVisibile < 0.05f) return;   // nessuna, oppure nascoste (festa)
  bool completa = costellazioneStelleMostrate >= COSTELLAZIONE_NUMERO_STELLE;
  uint16_t coloreTratto = scurisciColore(piccola.colore, COSTELLAZIONE_LUCE_TRATTO * costellazioneVisibile);
  int xPrima = cx, yPrima = cy;                          // il primo tratto parte dalla stella principale
  for (int i = 0; i < n; i++) {
    int x, y;
    posizioneStellaPiccola(i, n, cx, cy, x, y);
    c.drawLine(xPrima, yPrima, x, y, coloreTratto);
    xPrima = x; yPrima = y;
  }
  if (completa && n > 2) {
    int x0, y0;
    posizioneStellaPiccola(0, n, cx, cy, x0, y0);
    c.drawLine(xPrima, yPrima, x0, y0, coloreTratto);
  }
  for (int i = 0; i < n; i++) {
    int x, y;
    posizioneStellaPiccola(i, n, cx, cy, x, y);
    disegnaStella(c, x, y, piccola);
  }
}

// Un tratto nero che cresce da (ax, ay) verso (bx, by): parte = 0 niente, 1 intero
void disegnaTrattoCrescente(M5Canvas& c, int ax, int ay, int bx, int by, float parte) {
  if (parte <= 0) return;
  c.drawLine(ax, ay, ax + (int)((bx - ax) * parte), ay + (int)((by - ay) * parte), NERO);
}

// La costellazione in nero sul bianco: tutte le stelle come puntini (la principale più grande), uniti dai tratti
// che c'erano già; il tratto della stella nuova cresce (tratto = 0 niente, 1 intero) e il suo puntino può lampeggiare.
// Le posizioni sono quelle che le stelle avranno dopo, con la stella nuova al suo posto sul cerchio.
void disegnaPuntiCostellazione(M5Canvas& c, int cx, int cy, float tratto, bool nuovoAcceso) {
  int n = costellazioneStelle - 1;                       // stelle piccole, compresa la nuova (l'ultima)
  if (n <= 0) return;
  bool completa = costellazioneStelle >= COSTELLAZIONE_NUMERO_STELLE;
  int xPrima = cx, yPrima = cy;                          // il primo tratto parte dalla stella principale
  for (int i = 0; i < n; i++) {
    int x, y;
    posizioneStellaPiccola(i, n, cx, cy, x, y);
    bool nuova = i == n - 1;
    if (nuova) disegnaTrattoCrescente(c, xPrima, yPrima, x, y, tratto);
    else c.drawLine(xPrima, yPrima, x, y, NERO);
    if (!nuova || nuovoAcceso) c.fillCircle(x, y, ANIM_COST_PUNTO_PX, NERO);
    xPrima = x; yPrima = y;
  }
  if (completa && n > 2) {                               // ultima stella: si chiude anche il cerchio, insieme al tratto nuovo
    int x0, y0;
    posizioneStellaPiccola(0, n, cx, cy, x0, y0);
    disegnaTrattoCrescente(c, xPrima, yPrima, x0, y0, tratto);
  }
  c.fillCircle(cx, cy, ANIM_COST_PUNTO_PRINCIPALE_PX, NERO);
}
