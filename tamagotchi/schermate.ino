// =====================================================================
//  SCHERMATE — una funzione per schermata; lo switch è nel loop (tamagotchi.ino).
// =====================================================================

void schermataHome(M5Canvas& c) {
  c.fillScreen(NERO);
  disegnaCielo(c, orientamento, orientamentoPrecedente);
  disegnaStella(c, LARGHEZZA / 2 + (int)(dragX.valore + aspetto.spostamentoX), ALTEZZA / 2 + (int)(dragY.valore + aspetto.spostamentoY), aspetto);
  disegnaSlider(c, volumeCorrente);
  disegnaValori(c, bisogni);
}
