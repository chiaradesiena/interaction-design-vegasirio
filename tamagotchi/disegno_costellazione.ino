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

// Tratti: ogni stella piccola è unita alla sua stella (stellaCollegata: sul cerchio la precedente, nella costellazione
// disegnata quella della tabella); sul cerchio, a costellazione completa, l'ultima si unisce alla prima. Poi le stelle piccole.
void disegnaCostellazione(M5Canvas& c, int cx, int cy, const AspettoStella& piccola) {
  int n = costellazioneStelleMostrate - 1;               // quante stelle piccole
  if (n <= 0 || costellazioneVisibile < 0.05f) return;   // nessuna, oppure nascoste (festa)
  bool completa = costellazioneStelleMostrate >= COSTELLAZIONE_NUMERO_STELLE;
  uint16_t coloreTratto = scurisciColore(piccola.colore, COSTELLAZIONE_LUCE_TRATTO * costellazioneVisibile);
  int xPrima = cx, yPrima = cy;                          // l'ultima stella disegnata (serve a chiudere il cerchio)
  for (int i = 0; i < n; i++) {
    int x, y;
    posizioneStellaPiccola(i, n, cx, cy, x, y);
    for (int secondo = 0; secondo < 2; secondo++) {      // il tratto e, se c'è, il secondo tratto dei triangoli
      int k = stellaCollegata(i, secondo == 1);
      if (k < 0) continue;
      int xa, ya;
      puntoCostellazione(k, n, cx, cy, xa, ya);
      c.drawLine(xa, ya, x, y, coloreTratto);
    }
    xPrima = x; yPrima = y;
  }
  if (completa && n > 2 && !COSTELLAZIONE_DISEGNATA) {
    int x0, y0;
    posizioneStellaPiccola(0, n, cx, cy, x0, y0);
    c.drawLine(xPrima, yPrima, x0, y0, coloreTratto);
  }
  for (int i = 0; i < n; i++) {
    int x, y;
    posizioneStellaPiccola(i, n, cx, cy, x, y);
    if (COSTELLAZIONE_PALLINI) disegnaPallinoCostellazione(c, x, y, piccola, COSTELLAZIONE_GRANDEZZE[i]);   // Sirio: pallino con anello, grande come nel disegno vero
    else disegnaStella(c, x, y, piccola);
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
  int xPrima = cx, yPrima = cy;                          // l'ultima stella disegnata (serve a chiudere il cerchio)
  for (int i = 0; i < n; i++) {
    int x, y;
    posizioneStellaPiccola(i, n, cx, cy, x, y);
    bool nuova = i == n - 1;
    for (int secondo = 0; secondo < 2; secondo++) {      // il tratto verso la sua stella e, se c'è, il secondo tratto dei triangoli
      int k = stellaCollegata(i, secondo == 1);
      if (k < 0) continue;
      int xa, ya;
      puntoCostellazione(k, n, cx, cy, xa, ya);
      if (nuova) disegnaTrattoCrescente(c, xa, ya, x, y, tratto);   // cresce dalla stella a cui è unita verso la nuova
      else c.drawLine(xa, ya, x, y, NERO);
    }
    int puntino = max(1, (int)(ANIM_COST_PUNTO_PX * COSTELLAZIONE_GRANDEZZE[i] + 0.5f));   // grande come nel disegno vero
    if (!nuova || nuovoAcceso) c.fillCircle(x, y, puntino, NERO);
    xPrima = x; yPrima = y;
  }
  if (completa && n > 2 && !COSTELLAZIONE_DISEGNATA) {   // ultima stella sul cerchio: si chiude anche il cerchio, insieme al tratto nuovo
    int x0, y0;
    posizioneStellaPiccola(0, n, cx, cy, x0, y0);
    disegnaTrattoCrescente(c, xPrima, yPrima, x0, y0, tratto);
  }
  c.fillCircle(cx, cy, ANIM_COST_PUNTO_PRINCIPALE_PX, NERO);
}

// Stella piccola a pallino (COSTELLAZIONE_PALLINI): un pallino pieno col colore della salute e intorno un anello
// fermo di puntini un po' più scuri, che gira solo insieme alla costellazione. Sparisce e ricompare con la festa
// (costellazioneVisibile) e lampeggia insieme alla stella principale (luminosita). "grandezza" = quella della stella nel disegno vero (1 = le più grandi).
void disegnaPallinoCostellazione(M5Canvas& c, int x, int y, const AspettoStella& piccola, float grandezza) {
  if (piccola.luminosita <= 0) return;
  float k = costellazioneVisibile * grandezza;
  int raggio = (int)(COSTELLAZIONE_PALLINO_PX * k + 0.5f);
  if (raggio < 1 && k > 0.05f) raggio = 1;                // anche la più piccola resta almeno un pixel
  if (raggio < 1) return;
  c.fillCircle(x, y, raggio, piccola.colore);
  uint16_t coloreAnello = scurisciColore(piccola.colore, COSTELLAZIONE_ANELLO_LUCE);
  float r = max(COSTELLAZIONE_ANELLO_PX * k, raggio + COSTELLAZIONE_ANELLO_STACCO_MIN_PX * costellazioneVisibile);   // mai attaccato al pallino
  int quanti = max(6, (int)(COSTELLAZIONE_ANELLO_PUNTINI * r / COSTELLAZIONE_ANELLO_PX + 0.5f));   // anelli piccoli = meno puntini, così restano staccati
  for (int i = 0; i < quanti; i++) {
    float angolo = costellazioneAngolo + 2 * PI * i / quanti;
    int px = x + (int)(r * cosf(angolo) + 0.5f), py = y + (int)(r * sinf(angolo) + 0.5f);
    if (COSTELLAZIONE_ANELLO_PUNTINO_PX <= 0) c.drawPixel(px, py, coloreAnello);
    else c.fillCircle(px, py, COSTELLAZIONE_ANELLO_PUNTINO_PX, coloreAnello);
  }
}
