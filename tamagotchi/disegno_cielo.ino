// =====================================================================
//  DISEGNO CIELO — crea i punti del cielo e li disegna girati secondo l'orientamento,
//  con le scie. Legge orientamento e cielo, non legge i sensori.
// =====================================================================

float casuale01() {                       // numero fra 0 e 1
  statoCasuale = statoCasuale * 1664525UL + 1013904223UL;
  return (statoCasuale >> 8) / 16777216.0f;
}

void creaCielo() {
  for (int i = 0; i < PUNTI_SULLA_SFERA; i++) {
    // direzione distribuita in modo uniforme sulla sfera
    float z = casuale01() * 2 - 1;
    float angolo = casuale01() * 2 * PI;
    float r = sqrtf(1 - z * z);
    cielo[i].x = r * cosf(angolo);
    cielo[i].y = r * sinf(angolo);
    cielo[i].z = z;

    // i punti grandi sono rari; più grandi = più luminosi
    float t = casuale01() * casuale01();
    int dim = DIMENSIONE_MIN + (int)(t * (DIMENSIONE_MAX - DIMENSIONE_MIN + 1));
    if (dim > DIMENSIONE_MAX) dim = DIMENSIONE_MAX;
    cielo[i].dimensione = dim;
    uint8_t g = GRIGIO_MIN + (255 - GRIGIO_MIN) * (dim - DIMENSIONE_MIN) / (DIMENSIONE_MAX - DIMENSIONE_MIN);
    cielo[i].grigio = g;
    cielo[i].colore = rgb(g, g, g);
  }
  // ritmo del respiro: in un secondo giro, così la disposizione del cielo resta la stessa di prima
  for (int i = 0; i < PUNTI_SULLA_SFERA; i++) {
    cielo[i].fase = casuale01();
    cielo[i].periodoMs = RESPIRO_CIELO_MIN_MS + casuale01() * (RESPIRO_CIELO_MAX_MS - RESPIRO_CIELO_MIN_MS);
  }
}

// Dalla direzione 3D alla posizione sullo schermo; false se il punto è alle spalle
bool proietta(const float m[3][3], const PuntoCielo& p, int& sx, int& sy) {
  float x = m[0][0]*p.x + m[0][1]*p.y + m[0][2]*p.z;
  float y = m[1][0]*p.x + m[1][1]*p.y + m[1][2]*p.z;
  float z = m[2][0]*p.x + m[2][1]*p.y + m[2][2]*p.z;
  if (z >= -0.05f) return false;
  sx = LARGHEZZA / 2 + (int)(CAMPO_VISIVO * x / -z);
  sy = ALTEZZA / 2   - (int)(CAMPO_VISIVO * y / -z);
  return true;
}

// Motion blur: una scia dal punto verso dove si trovava nel fotogramma prima
void disegnaScia(M5Canvas& c, int x, int y, int xPrima, int yPrima, int dim, uint16_t col) {
  float dx = (xPrima - x) * MOTION_BLUR, dy = (yPrima - y) * MOTION_BLUR;
  float lunghezza = sqrtf(dx * dx + dy * dy);
  if (lunghezza < 1.5f) return;                   // quasi fermo: niente scia
  if (lunghezza > 40) { dx *= 40 / lunghezza; dy *= 40 / lunghezza; }
  int x2 = x + (int)dx, y2 = y + (int)dy;
  c.drawLine(x, y, x2, y2, col);
  if (dim >= 2) c.drawLine(x + 1, y, x2 + 1, y2, col);   // punti grandi: scia più spessa
  if (dim >= 3) c.drawLine(x, y + 1, x2, y2 + 1, col);
}

void disegnaPunto(M5Canvas& c, int x, int y, int dim, uint16_t col) {
  if (dim % 2 == 1) c.fillCircle(x, y, dim / 2, col);          // dispari (1, 3, 5 px): tondo
  else              c.fillRect(x - dim / 2, y - dim / 2, dim, dim, col);  // pari (2, 4 px): quadrato
}

// Ogni punto viene girato secondo l'orientamento del dispositivo e poi
// proiettato guardando "dietro lo schermo" (verso -z).

void disegnaCielo(M5Canvas& c, const float m[3][3], const float mPrima[3][3]) {
  puntiVisibili = 0;
  for (int i = 0; i < PUNTI_SULLA_SFERA; i++) {
    if (i >= PUNTI_SULLA_SFERA * cieloFrazione) break;   // il cielo che ritorna dopo l'animazione della costellazione
    const PuntoCielo& p = cielo[i];
    int sx, sy, px, py;
    if (!proietta(m, p, sx, sy)) continue;
    if (sx < -45 || sx > LARGHEZZA + 45 || sy < -45 || sy > ALTEZZA + 45) continue;
    if (MOTION_BLUR > 0 && proietta(mPrima, p, px, py))
      disegnaScia(c, sx, sy, px, py, p.dimensione, p.coloreScia);
    disegnaPunto(c, sx, sy, p.dimensione, p.colore);
    puntiVisibili++;
  }
}
