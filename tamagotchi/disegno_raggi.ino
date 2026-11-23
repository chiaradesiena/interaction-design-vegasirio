// =====================================================================
//  DISEGNO RAGGI — la terza versione della stella (si sceglie con lo swipe, logica_stile.ino):
//  un nucleo di puntini e tanti raggi fatti di colonne di puntini, disposti in cerchio,
//  un po' più fitti dove la stella ha le punte. I raggi girano intorno al nucleo e respirano
//  (si allungano e si accorciano). Legge solo "aspetto": fame (buco), solitudine (lampeggio),
//  ascolto, carezza, orbite e festa si vedono uguali. I puntini hanno il colore della salute.
// =====================================================================

const int RAGGI_MAX = 128;                       // massimo di raggi che si disegnano (sicurezza, non serve cambiarlo)

// Numero fra 0 e 1 sempre uguale per lo stesso raggio / puntino: così ogni raggio ha la sua lunghezza,
// il suo spessore e i suoi "vuoti", ma sempre gli stessi (non tremano a caso a ogni fotogramma)
float casualeRaggio(int a, int b, int c) {
  uint32_t h = (uint32_t)a * 73856093UL ^ (uint32_t)b * 19349663UL ^ (uint32_t)c * 83492791UL ^ RAGGI_SEME;
  h ^= h >> 15;
  h *= 0x2c1b3c6dUL;
  h ^= h >> 12;
  h *= 0x297a2d39UL;
  h ^= h >> 15;
  return (h & 0xFFFF) / 65536.0f;
}

// Un puntino, solo se non cade nel buco della fame
void puntinoRaggi(M5Canvas& c, float x, float y, float distanza, float rBuco, uint16_t colore) {
  if (distanza < rBuco) return;
  c.fillCircle((int)(x + 0.5f), (int)(y + 0.5f), RAGGI_PUNTINO_PX, colore);
}

// Il nucleo: puntini su cerchi uno dentro l'altro (così restano in "fila" come i raggi), che girano coi raggi
void disegnaNucleoRaggi(M5Canvas& c, int cx, int cy, float rNucleo, float rBuco, float giro, uint16_t colore) {
  for (int k = 0; k * RAGGI_PASSO_PX <= rNucleo; k++) {
    float r = k * RAGGI_PASSO_PX;
    int quanti = k == 0 ? 1 : (int)(2 * PI * r / RAGGI_PASSO_PX);
    for (int i = 0; i < quanti; i++) {
      float angolo = giro + i * 2 * PI / quanti + k * 0.5f;   // ogni cerchio un po' sfasato
      puntinoRaggi(c, cx + r * cosf(angolo), cy - r * sinf(angolo), r, rBuco, colore);
    }
  }
}

// Corrispondenze con la stella a puntini:
//  raggio = fin dove arrivano i raggi;  vita = grandezza del nucleo (vita alta = pallino: i raggi rientrano);
//  numeroPunte = dove i raggi sono più fitti;  buco = puntini tolti al centro;  luminosita 0 = spenta.
// La propria stella: i suoi raggi (RAGGI_MIEI) e la sua larghezza fra le punte (STELLA_VITA)
void disegnaStellaRaggi(M5Canvas& c, int cx, int cy, const AspettoStella& a) {
  disegnaStellaRaggiCon(c, cx, cy, a, RAGGI_MIEI, STELLA_VITA);
}

// Una stella qualsiasi coi raggi di "carattere"; vitaNormale = la sua larghezza fra le punte quando sta bene (serve a capire quando diventa pallino)
void disegnaStellaRaggiCon(M5Canvas& c, int cx, int cy, const AspettoStella& a, const CarattereRaggi& carattere, float vitaNormale) {
  if (a.luminosita <= 0) return;                      // fase "spenta" del lampeggio
  if (!isfinite(a.raggio) || a.raggio <= 0) return;   // grandezza sbagliata: meglio non disegnare

  float R = a.raggio * carattere.lunghezza;
  float rNucleo = a.raggio * a.vita * carattere.nucleo;
  float rBuco = a.buco * a.raggio;
  float apertura = constrain((1.0f - a.vita) / max(1.0f - vitaNormale, 0.01f), 0.0f, 1.0f);   // 0 = pallino: i raggi rientrano nel nucleo
  float tempoS = (millis() % 3600000UL) / 1000.0f;     // secondi (riparte ogni ora)
  float giro = a.rotazione * PI / 180.0f + fmodf(tempoS * carattere.giroGradiS, 360.0f) * PI / 180.0f;   // i raggi girano intorno al nucleo
  int punte = max(a.numeroPunte, 1);
  int n = constrain(carattere.numero, 1, RAGGI_MAX);

  disegnaNucleoRaggi(c, cx, cy, rNucleo, rBuco, giro, a.colore);

  for (int i = 0; i < n; i++) {
    // posizione: in cerchio, ma "tirata" verso le punte (più fitti lì, più radi fra una punta e l'altra)
    float t = i * 2 * PI / n;
    float angolo = giro + t - carattere.addensamento * sinf(punte * t) / punte;
    float ca = cosf(angolo), sa = sinf(angolo);

    // lunghezza: un po' diversa per ogni raggio, e respira col suo ritmo
    float periodoS = RAGGI_RESPIRO_PERIODO_MS / 1000.0f * (0.7f + 0.6f * casualeRaggio(i, 0, 1));
    float respiro = sinf(2 * PI * tempoS / periodoS + casualeRaggio(i, 0, 2) * 2 * PI);
    float lunghezza = R * (1.0f - RAGGI_VARIAZIONE * casualeRaggio(i, 0, 3)) * (1.0f + RAGGI_RESPIRO_AMPIEZZA * respiro);
    float fine = rNucleo + (lunghezza - rNucleo) * apertura;

    // spessore: ogni raggio ha da 1 a RAGGI_COLONNE_MAX colonne di puntini affiancate
    int colonne = 1 + (int)(casualeRaggio(i, 0, 4) * RAGGI_COLONNE_MAX);
    if (colonne > RAGGI_COLONNE_MAX) colonne = RAGGI_COLONNE_MAX;
    for (int col = 0; col < colonne; col++) {
      float scostamento = (col - (colonne - 1) * 0.5f) * RAGGI_PASSO_PX;   // di lato rispetto al centro del raggio
      float fineColonna = col == colonne / 2 ? fine : rNucleo + (fine - rNucleo) * (0.6f + 0.4f * casualeRaggio(i, col, 5));   // le colonne laterali un po' più corte
      int passo = 0;
      for (float r = rNucleo + RAGGI_PASSO_PX; r <= fineColonna; r += RAGGI_PASSO_PX, passo++) {
        if (casualeRaggio(i, col * 1000 + passo, 6) < RAGGI_VUOTI) continue;   // qualche puntino manca, come nell'immagine
        float x = cx + r * ca - scostamento * sa;
        float y = cy - r * sa - scostamento * ca;
        puntinoRaggi(c, x, y, sqrtf(r * r + scostamento * scostamento), rBuco, a.colore);
      }
    }
  }
}
