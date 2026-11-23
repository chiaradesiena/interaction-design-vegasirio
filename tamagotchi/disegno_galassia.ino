// =====================================================================
//  DISEGNO GALASSIA — Unisono: i bracci della spirale (puntini grigi), il braccio di luce che gira,
//  le altre stelle sui bracci e la propria al centro. Legge bracciVisibili, luceAngolo, luceVisibile,
//  numeroGalassia, galassiaX/Y, aspettiGalassia, aspettoUnisono (li calcola logica_unisono.ino); non legge i sensori.
// =====================================================================

// I bracci: righe di puntini che si avvolgono attorno al centro, più chiari vicino al centro e sempre più spenti verso fuori
void disegnaBracci(M5Canvas& c) {
  if (bracciVisibili < 0.02f) return;
  for (int b = 0; b < UNISONO_BRACCI; b++) {
    for (float r = UNISONO_RAGGIO_INTERNO_PX; r <= UNISONO_RAGGIO_BRACCI_PX; r += UNISONO_BRACCIO_PASSO_PX) {
      float fuori = (r - UNISONO_RAGGIO_INTERNO_PX) / (UNISONO_RAGGIO_BRACCI_PX - UNISONO_RAGGIO_INTERNO_PX);
      float luce = UNISONO_BRACCIO_LUCE * (1.0f - fuori) * bracciVisibili;
      if (luce < 0.05f) continue;
      int x, y;
      puntoBraccio(b, r, x, y);
      c.fillCircle(x, y, 1, scurisciColore(BIANCO, luce));
    }
  }
}

// Il braccio di luce: una riga di puntini dal centro verso fuori, con una scia di righe più deboli dietro
void disegnaLuceGalassia(M5Canvas& c) {
  if (luceVisibile < 0.02f) return;
  for (int s = UNISONO_LUCE_SCIA; s >= 0; s--) {                 // prima la scia (più debole), poi la riga principale
    float angolo = luceAngolo - s * UNISONO_LUCE_SCIA_GRADI * GRADI_A_RADIANTI;
    float forza = UNISONO_LUCE_FORZA * luceVisibile * (1.0f - (float)s / (UNISONO_LUCE_SCIA + 1));
    float ca = cosf(angolo), sa = sinf(angolo);
    for (float r = UNISONO_RAGGIO_INTERNO_PX; r <= UNISONO_LUCE_LUNGHEZZA_PX; r += UNISONO_LUCE_PASSO_PX) {
      float luce = forza * (1.0f - r / (UNISONO_LUCE_LUNGHEZZA_PX * 1.2f));   // si spegne verso la punta
      c.drawPixel(unisonoCentroX + (int)(r * ca), unisonoCentroY + (int)(r * sa), scurisciColore(BIANCO, luce));
    }
  }
}

// I raggi di un'altra stella, riconosciuta dal nome che manda via radio; una stella sconosciuta prende quelli di questa
const CarattereRaggi& carattereRaggiDi(const char* nome) {
  if (strcmp(nome, "Sirio") == 0) return RAGGI_SIRIO;
  if (strcmp(nome, "Vega") == 0) return RAGGI_VEGA;
  return RAGGI_MIEI;
}

// Con la versione a raggi (scelta con lo swipe) tutte le stelle della galassia sono a raggi, ognuna col suo carattere
void disegnaGalassia(M5Canvas& c) {
  disegnaBracci(c);
  disegnaLuceGalassia(c);
  for (int i = 0; i < numeroGalassia; i++) {
    if (stileStella == STILE_RAGGI) disegnaStellaRaggiCon(c, galassiaX[i], galassiaY[i], aspettiGalassia[i], carattereRaggiDi(galassia[i].nome), galassia[i].vita);   // a raggi, col carattere di quella stella
    else disegnaStella(c, galassiaX[i], galassiaY[i], aspettiGalassia[i]);
  }
  if (stileStella == STILE_RAGGI && aspettoUnisono.cane <= 0) disegnaStellaRaggi(c, unisonoCentroX, unisonoCentroY, aspettoUnisono);   // la propria stella nella versione scelta con lo swipe (logica_stile.ino)
  else disegnaStella(c, unisonoCentroX, unisonoCentroY, aspettoUnisono);
}
