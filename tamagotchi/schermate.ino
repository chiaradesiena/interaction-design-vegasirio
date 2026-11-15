// =====================================================================
//  SCHERMATE — una funzione per schermata; lo switch è nel loop (tamagotchi.ino).
// =====================================================================

void schermataHome(M5Canvas& c) {
  c.fillScreen(NERO);
  disegnaCielo(c, orientamento, orientamentoPrecedente);
  disegnaCostellazione(c, LARGHEZZA / 2 + (int)(dragX.valore + aspetto.spostamentoX), ALTEZZA / 2 + (int)(dragY.valore + aspetto.spostamentoY), aspettoPiccola);
  disegnaStella(c, LARGHEZZA / 2 + (int)(dragX.valore + aspetto.spostamentoX), ALTEZZA / 2 + (int)(dragY.valore + aspetto.spostamentoY), aspetto);
}

// Menu delle impostazioni: mentre entra o esce si vede ancora la stella dietro, poi solo il pannello
void schermataMenu(M5Canvas& c) {
  int x0 = (int)((1.0f - menuApertura.valore) * LARGHEZZA);
  if (x0 > 0) schermataHome(c);
  disegnaMenu(c, x0);
}

// Animazione della nuova stella della costellazione (10 secondi). La logica (logica_animazione_costellazione.ino)
// decide cosa si vede; qui si disegna: fondo da nero a bianco e ritorno, cielo, due puntini neri col tratto, stelle.
void schermataAnimazioneCostellazione(M5Canvas& c) {
  c.fillScreen(scurisciColore(BIANCO, animBianco));
  if (animCielo) disegnaCielo(c, orientamento, orientamentoPrecedente);
  if (animPuntiAccesi) disegnaPuntiCostellazione(c, animCentroX, animCentroY, animTratto, animNuovoAcceso);
  if (animStelle) {
    int cx = LARGHEZZA / 2 + (int)(dragX.valore + aspetto.spostamentoX), cy = ALTEZZA / 2 + (int)(dragY.valore + aspetto.spostamentoY);
    disegnaCostellazione(c, cx, cy, aspettoPiccola);
    disegnaStella(c, cx, cy, aspetto);
  }
}
