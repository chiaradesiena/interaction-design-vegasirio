// =====================================================================
//  DISEGNO INTERFACCIA — slider del volume e valori dei bisogni in alto a sinistra.
// =====================================================================

// Slider del volume: una fila di puntini che crescono verso destra (halftone);
// quelli fino al livello del volume sono bianchi, gli altri grigi, e un puntino grande fa da cursore
void disegnaSlider(M5Canvas& c, int volume) {
  const int y = ALTEZZA - SLIDER_ZONA_ALTEZZA / 2;
  const int lunghezza = LARGHEZZA - 2 * SLIDER_MARGINE_X;
  const int n = lunghezza / SLIDER_PASSO_PUNTINI + 1;
  const int xCursore = SLIDER_MARGINE_X + (int)(volume / 255.0f * lunghezza);
  for (int i = 0; i < n; i++) {
    int x = SLIDER_MARGINE_X + i * SLIDER_PASSO_PUNTINI;
    int r = 1 + (int)(2.0f * i / (n - 1) + 0.5f);                 // da 1 a 3 px
    c.fillCircle(x, y, r, x <= xCursore ? BIANCO : GRIGIO_SLIDER);
  }
  c.fillCircle(xCursore, y, SLIDER_RAGGIO_CURSORE, BIANCO);
}

// Una riga di testo, per esempio "salute: 60%" (senza accenti; niente String, solo un testo di lunghezza fissa)
void disegnaRigaValore(M5Canvas& c, int riga, const char* nome, float valore) {
  char testo[24];
  snprintf(testo, sizeof(testo), "%s: %d%%", nome, (int)(valore + 0.5f));
  c.drawString(testo, VALORI_X, VALORI_Y + riga * VALORI_INTERLINEA_PX);
}

// Valori dei bisogni e della salute in alto a sinistra, in tempo reale
void disegnaValori(M5Canvas& c, const Bisogni& b) {
  if (!VALORI_MOSTRA) return;
  c.setTextColor(COLORE_VALORI);
  c.setTextSize(VALORI_DIMENSIONE_TESTO);
  c.setTextDatum(top_left);
  int riga = 0;
  disegnaRigaValore(c, riga++, "salute", b.salute);
  disegnaRigaValore(c, riga++, "nutrimento", b.nutrimento);
  disegnaRigaValore(c, riga++, "socialita", b.socialita);
  disegnaRigaValore(c, riga++, "energia", b.energia);
  if (VALORI_MOSTRA_ENERGIA_UMORE) disegnaRigaValore(c, riga++, "umore", b.umore);
}
