// =====================================================================
//  DISEGNO CANE — la sagoma del cane di Sirio fatta di puntini (halftone), e il passaggio stella -> cane.
//  Legge "aspetto" (campi cane e caneAngolo) e la forma SIRIO_CANE_PEZZI; non legge i sensori.
//  La sagoma è fatta di ovali fusi fra loro: per ogni puntino si misura quanto è lontano dal bordo.
// =====================================================================

// Unione "morbida" di due distanze: come prendere la più piccola, ma l'attaccatura fra i pezzi si arrotonda
float unioneMorbida(float a, float b, float k) {
  if (k <= 0) return min(a, b);
  float h = max(k - fabsf(a - b), 0.0f) / k;
  return min(a, b) - h * h * k * 0.25f;
}

// Da fare una volta per fotogramma: posizione e inclinazione dei pezzi (le zampe si piegano), giro, verso e grandezza.
// Le zampe si piegano attorno alla loro cima, a coppie incrociate (davanti-fuori con dietro-dentro), come quando si trotta.
void preparaCane(const AspettoStella& a) {
  caneRaggioSagoma = 0;
  for (int p = 0; p < SIRIO_CANE_NUMERO_PEZZI; p++) {
    const PezzoCane& pz = SIRIO_CANE_PEZZI[p];
    float angolo = pz.angoloGradi;
    caneXPezzo[p] = pz.x;
    caneYPezzo[p] = pz.y;
    if (p >= SIRIO_CANE_PRIMA_ZAMPA) {
      float verso = ((p - SIRIO_CANE_PRIMA_ZAMPA) % 3 == 0) ? 1.0f : -1.0f;
      float piega = verso * a.caneZampe * GRADI_A_RADIANTI;
      caneXPezzo[p] = pz.x + pz.ry * sinf(piega);                    // la cima della zampa resta attaccata al corpo
      caneYPezzo[p] = pz.y + pz.ry - pz.ry * cosf(piega);
      angolo += verso * a.caneZampe;
    }
    caneCosPezzo[p] = cosf(angolo * GRADI_A_RADIANTI);
    caneSinPezzo[p] = sinf(angolo * GRADI_A_RADIANTI);
    float fine = sqrtf(caneXPezzo[p] * caneXPezzo[p] + caneYPezzo[p] * caneYPezzo[p]) + max(pz.rx, pz.ry);
    caneRaggioSagoma = max(caneRaggioSagoma, fine + SIRIO_CANE_SFUMATURA_PX);   // cerchio che contiene tutto il cane
  }
  caneCosGiro = cosf(a.caneAngolo * GRADI_A_RADIANTI);
  caneSinGiro = sinf(a.caneAngolo * GRADI_A_RADIANTI);
  caneScala = a.raggio / STELLA_RAGGIO * SIRIO_CANE_GRANDEZZA;   // cresce con la carezza e con la vicinanza
  caneVersoDisegno = a.caneVerso;                                // mentre si gira diventa sottile, poi si rigira
  if (fabsf(caneVersoDisegno) < 0.2f) caneVersoDisegno = caneVersoDisegno < 0 ? -0.2f : 0.2f;
}

// Distanza (px) di un punto dal bordo della sagoma: negativa dentro, positiva fuori
float distanzaCane(float dx, float dy) {
  float x = (caneCosGiro * dx + caneSinGiro * dy) / caneScala;   // il punto girato al contrario = la sagoma girata
  float y = (-caneSinGiro * dx + caneCosGiro * dy) / caneScala;
  x /= caneVersoDisegno;                                         // verso -1 = cane allo specchio, guarda a sinistra
  float lontano = sqrtf(x * x + y * y) - caneRaggioSagoma;       // punto lontano da tutto il cane: inutile guardare i pezzi (più veloce)
  if (lontano > 0) return 1e6f;                                  // molto fuori: nessun puntino
  float d = 1e6f;
  for (int p = 0; p < SIRIO_CANE_NUMERO_PEZZI; p++) {
    const PezzoCane& pz = SIRIO_CANE_PEZZI[p];
    float qx = x - caneXPezzo[p], qy = y - caneYPezzo[p];
    float lx = caneCosPezzo[p] * qx + caneSinPezzo[p] * qy;      // il punto visto "dritto" rispetto all'ovale
    float ly = -caneSinPezzo[p] * qx + caneCosPezzo[p] * qy;
    float k = sqrtf((lx / pz.rx) * (lx / pz.rx) + (ly / pz.ry) * (ly / pz.ry));
    float dp = (k - 1.0f) * min(pz.rx, pz.ry);                    // distanza dal bordo dell'ovale (circa)
    d = unioneMorbida(d, dp, SIRIO_CANE_MORBIDEZZA_PX);
  }
  return d * caneScala * min(1.0f, fabsf(caneVersoDisegno));    // mentre si gira il bordo resta netto
}

// Quanto un punto è "dentro" il cane: 1 dentro, poi i puntini si rimpiccioliscono verso il bordo
float densitaCane(float dx, float dy) {
  float v = 0.5f - distanzaCane(dx, dy) / SIRIO_CANE_SFUMATURA_PX;
  return constrain(v, 0.0f, 1.0f);
}

// Un colore più scuro: k = 1 colore pieno, k = 0 nero
uint16_t coloreSfumato(uint16_t c, float k) {
  uint16_t r = (uint16_t)(((c >> 11) & 0x1F) * k + 0.5f);
  uint16_t g = (uint16_t)(((c >> 5) & 0x3F) * k + 0.5f);
  uint16_t b = (uint16_t)((c & 0x1F) * k + 0.5f);
  return (r << 11) | (g << 5) | b;
}

// Un puntino del cane. Tutti i puntini hanno la stessa grandezza e la stessa luce: o ci sono o non ci sono,
// così il bordo è netto e, mentre il cane si muove, i puntini non cambiano grandezza tutti insieme (sfarfallio).
// Durante la trasformazione (densità mescolata con la stella) il bordo sfuma ancora di luce, per un passaggio morbido.
void disegnaPuntinoCane(M5Canvas& c, int x, int y, int passo, float densita, uint16_t colore, bool formato) {
  if (densita < (formato ? SIRIO_CANE_SOGLIA_BORDO : SIRIO_CANE_LUCE_MIN)) return;   // fuori dalla sagoma: niente puntino
  c.fillRect(x - passo / 2, y - passo / 2, passo, passo, coloreFondo);   // copre il cielo dietro il cane (col colore dello sfondo)
  uint16_t luce = formato ? colore : coloreSfumato(colore, densita);
  c.fillCircle(x, y, (int)(SIRIO_CANE_PUNTINO_MAX_PX + 0.5f), luce);
}

// Durante la trasformazione si mescolano le due forme: a = 0 solo stella, a = 1 solo cane
float densitaStellaOCane(float dx, float dy, const AspettoStella& a) {
  if (a.cane <= 0) return densitaStella(dx, dy, a);
  if (a.cane >= 1) return densitaCane(dx, dy);
  return (1.0f - a.cane) * densitaStella(dx, dy, a) + a.cane * densitaCane(dx, dy);
}
