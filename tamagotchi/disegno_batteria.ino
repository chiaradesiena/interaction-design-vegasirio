// =====================================================================
//  DISEGNO BATTERIA — la pagina dopo il menu delle impostazioni: nero con la stessa griglia di puntini,
//  una batteria che si riempie in base alla carica, la percentuale scritta e, se è in carica,
//  una saetta sopra la batteria e la scritta "in carica".
//  Legge sensi.batteriaPercento e sensi.inCarica; non legge altro.
// =====================================================================

// Una saetta gialla con un contorno nero (così si vede sia sul riempimento sia sul vuoto), centrata in (cx, cy).
// s = mezza altezza. I punti sono in frazioni di s; la saetta è fatta di 4 triangoli.
void disegnaSaetta(M5Canvas& c, int cx, int cy, int s, uint16_t colore) {
  const float px[6] = {  0.20f, -0.50f, -0.05f, -0.20f,  0.50f,  0.05f };
  const float py[6] = { -1.00f,  0.10f,  0.10f,  1.00f, -0.20f, -0.20f };
  const int triangoli[4][3] = { {0, 1, 2}, {0, 2, 5}, {2, 3, 4}, {2, 4, 5} };
  const int spostamenti[5][2] = { {-2, 0}, {2, 0}, {0, -2}, {0, 2}, {0, 0} };   // le prime 4 fanno il contorno, l'ultima il colore
  for (int k = 0; k < 5; k++) {
    uint16_t col = (k < 4) ? NERO : colore;
    for (int t = 0; t < 4; t++) {
      c.fillTriangle(cx + (int)(px[triangoli[t][0]] * s) + spostamenti[k][0], cy + (int)(py[triangoli[t][0]] * s) + spostamenti[k][1],
                     cx + (int)(px[triangoli[t][1]] * s) + spostamenti[k][0], cy + (int)(py[triangoli[t][1]] * s) + spostamenti[k][1],
                     cx + (int)(px[triangoli[t][2]] * s) + spostamenti[k][0], cy + (int)(py[triangoli[t][2]] * s) + spostamenti[k][1], col);
    }
  }
}

// La batteria: contorno, polo a destra e riempimento che cresce da sinistra a destra con la percentuale
void disegnaIconaBatteria(M5Canvas& c, int x0, int percento, bool inCarica) {
  int x = x0 + BATTERIA_X, y = BATTERIA_Y, w = BATTERIA_LARGHEZZA_PX, h = BATTERIA_ALTEZZA_PX;
  for (int i = 0; i < BATTERIA_BORDO_PX; i++)
    c.drawRoundRect(x + i, y + i, w - 2 * i, h - 2 * i, 10 - i > 1 ? 10 - i : 1, BIANCO);
  c.fillRoundRect(x + w, y + (h - BATTERIA_PUNTA_ALTEZZA_PX) / 2, BATTERIA_PUNTA_LARGHEZZA_PX, BATTERIA_PUNTA_ALTEZZA_PX, 3, BIANCO);
  int dentro = BATTERIA_BORDO_PX + BATTERIA_MARGINE_PX;
  int larghezzaPiena = w - 2 * dentro;
  int riempimento = (int)(larghezzaPiena * constrain(percento, 0, 100) / 100.0f + 0.5f);
  if (riempimento > 0)
    c.fillRect(x + dentro, y + dentro, riempimento, h - 2 * dentro, percento <= BATTERIA_SOGLIA_BASSA ? ROSSO_STELLA : BIANCO);
  if (inCarica) disegnaSaetta(c, x + w / 2, y + h / 2, BATTERIA_SAETTA_PX, GIALLO_STELLA);
}

// Il pannello entra da destra, come il menu: x0 è dove sta il suo bordo sinistro (da LARGHEZZA a 0).
void disegnaBatteria(M5Canvas& c, int x0) {
  int destra = max(x0 + LARGHEZZA, LARGHEZZA);
  c.fillRect(x0, 0, destra - x0, ALTEZZA, NERO);
  for (int y = MENU_PASSO_GRIGLIA / 2; y < ALTEZZA; y += MENU_PASSO_GRIGLIA)
    for (int x = MENU_PASSO_GRIGLIA / 2; x < LARGHEZZA; x += MENU_PASSO_GRIGLIA)
      if (x0 + x < LARGHEZZA) c.fillCircle(x0 + x, y, MENU_RAGGIO_PUNTINI, GRIGIO_MENU);
  c.setTextColor(COLORE_VALORI);
  c.setTextDatum(middle_center);
  c.setTextSize(1);
  c.drawString("batteria", x0 + LARGHEZZA / 2, BATTERIA_Y - 22);
  disegnaIconaBatteria(c, x0, sensi.batteriaPercento, sensi.inCarica);
  char testo[8];
  snprintf(testo, sizeof(testo), "%d%%", sensi.batteriaPercento);
  c.setTextColor(BIANCO);
  c.setTextSize(BATTERIA_PERCENTO_TESTO);
  c.drawString(testo, x0 + LARGHEZZA / 2, BATTERIA_PERCENTO_Y);
  if (sensi.inCarica) {
    c.setTextColor(COLORE_VALORI);
    c.setTextSize(BATTERIA_CARICA_TESTO);
    c.drawString("in carica", x0 + LARGHEZZA / 2, BATTERIA_CARICA_Y);
  }
  if (x0 > 0) c.drawFastVLine(x0, 0, ALTEZZA, GRIGIO_BORDO_MENU);   // il bordo, mentre entra o esce
}
