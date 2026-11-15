// =====================================================================
//  DISEGNO MENU — il pannello delle impostazioni: nero, con una griglia di puntini grigi,
//  la riga del volume e quella delle stelle (tasti - e +, in mezzo un puntino per livello o per stella)
//  e in basso i parametri (salute, nutrimento, socialita, energia).
//  Legge menuApertura, volumeLivello, costellazioneStelle, bisogni e menuTastoAcceso; non legge i sensori.
// =====================================================================

// Un tasto rotondo con dentro un segno + o - fatto di linee. Appena toccato si riempie di bianco per un attimo.
void disegnaTastoMenu(M5Canvas& c, int x, int y, bool piu, bool acceso) {
  uint16_t segno = acceso ? NERO : BIANCO;
  if (acceso) c.fillCircle(x, y, MENU_TASTO_RAGGIO_PX, BIANCO);
  else { c.fillCircle(x, y, MENU_TASTO_RAGGIO_PX, NERO); c.drawCircle(x, y, MENU_TASTO_RAGGIO_PX, GRIGIO_BORDO_MENU); }
  c.drawFastHLine(x - MENU_SEGNO_PX, y, 2 * MENU_SEGNO_PX + 1, segno);
  if (piu) c.drawFastVLine(x, y - MENU_SEGNO_PX, 2 * MENU_SEGNO_PX + 1, segno);
}

// Una riga del menu: nome piccolo in alto, tasto - a sinistra, tasto + a destra e in mezzo una fila di puntini
// che crescono verso destra (halftone): accesi (bianchi) fino al valore, spenti (grigi) dopo.
void disegnaRigaPiuMeno(M5Canvas& c, int x0, int y, const char* nome, int valore, int numeroPuntini,
                        TastoMenu tastoMeno, TastoMenu tastoPiu) {
  c.setTextColor(COLORE_VALORI);
  c.setTextSize(1);
  c.setTextDatum(middle_center);
  c.drawString(nome, x0 + LARGHEZZA / 2, y - MENU_ETICHETTA_SOPRA_PX);
  disegnaTastoMenu(c, x0 + MENU_TASTO_MENO_X, y, false, menuTastoAcceso == tastoMeno);
  disegnaTastoMenu(c, x0 + MENU_TASTO_PIU_X, y, true, menuTastoAcceso == tastoPiu);
  int xPrimo = x0 + LARGHEZZA / 2 - (numeroPuntini - 1) * MENU_PASSO_LIVELLI_PX / 2;
  for (int i = 0; i < numeroPuntini; i++) {
    int r = MENU_LIVELLO_RAGGIO_MIN_PX
            + (int)((MENU_LIVELLO_RAGGIO_MAX_PX - MENU_LIVELLO_RAGGIO_MIN_PX) * i / (float)max(numeroPuntini - 1, 1) + 0.5f);
    c.fillCircle(xPrimo + i * MENU_PASSO_LIVELLI_PX, y, r, i < valore ? BIANCO : GRIGIO_LIVELLO_SPENTO);
  }
}

// Un parametro scritto, per esempio "salute 60%", centrato in (x, y) (senza accenti; niente String)
void disegnaParametroMenu(M5Canvas& c, int x, int y, const char* nome, float valore) {
  char testo[24];
  snprintf(testo, sizeof(testo), "%s %d%%", nome, (int)(valore + 0.5f));
  c.drawString(testo, x, y);
}

// I quattro parametri in basso, su due colonne, aggiornati a ogni fotogramma
void disegnaParametriMenu(M5Canvas& c, int x0, const Bisogni& b) {
  c.setTextColor(COLORE_VALORI);
  c.setTextSize(MENU_PARAMETRI_DIMENSIONE_TESTO);
  c.setTextDatum(middle_center);
  int xSinistra = x0 + MENU_PARAMETRI_COLONNA_SINISTRA_X, xDestra = x0 + MENU_PARAMETRI_COLONNA_DESTRA_X;
  int ySopra = MENU_PARAMETRI_Y, ySotto = MENU_PARAMETRI_Y + MENU_PARAMETRI_INTERLINEA_PX;
  disegnaParametroMenu(c, xSinistra, ySopra, "salute", b.salute);
  disegnaParametroMenu(c, xDestra,   ySopra, "nutrimento", b.nutrimento);
  disegnaParametroMenu(c, xSinistra, ySotto, "socialita", b.socialita);
  disegnaParametroMenu(c, xDestra,   ySotto, "energia", b.energia);
}

// Il pannello entra da destra: x0 è dove sta il suo bordo sinistro (da LARGHEZZA a 0).
// Se la molla va un po' oltre (x0 < 0) il fondo resta nero e si spostano solo i puntini: si vede il rimbalzo.
void disegnaMenu(M5Canvas& c, int x0) {
  int destra = max(x0 + LARGHEZZA, LARGHEZZA);
  c.fillRect(x0, 0, destra - x0, ALTEZZA, NERO);
  for (int y = MENU_PASSO_GRIGLIA / 2; y < ALTEZZA; y += MENU_PASSO_GRIGLIA)
    for (int x = MENU_PASSO_GRIGLIA / 2; x < LARGHEZZA; x += MENU_PASSO_GRIGLIA)
      if (x0 + x < LARGHEZZA) c.fillCircle(x0 + x, y, MENU_RAGGIO_PUNTINI, GRIGIO_MENU);
  disegnaRigaPiuMeno(c, x0, MENU_VOLUME_Y, "volume", volumeLivello, VOLUME_NUMERO_LIVELLI, TASTO_VOLUME_MENO, TASTO_VOLUME_PIU);
  disegnaRigaPiuMeno(c, x0, MENU_STELLE_Y, "stelle", costellazioneStelle, COSTELLAZIONE_NUMERO_STELLE, TASTO_STELLE_MENO, TASTO_STELLE_PIU);
  disegnaParametriMenu(c, x0, bisogni);
  if (x0 > 0) c.drawFastVLine(x0, 0, ALTEZZA, GRIGIO_BORDO_MENU);   // il bordo, mentre entra o esce
}
