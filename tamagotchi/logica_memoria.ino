// =====================================================================
//  LOGICA MEMORIA — salva i bisogni nella memoria permanente e li riprende all'accensione
//  (salvo reset o tocco all'avvio). Legge e scrive "bisogni".
// =====================================================================

// Un valore è accettabile se è un numero fra 0 e 100 (un NaN o un dato rovinato non passa)
bool bisognoValido(float v) {
  return v >= 0.0f && v <= BISOGNO_MAX;
}

// All'accensione: decide se azzerare (reset o tocco) oppure riprendere i valori salvati.
// Se non c'è nulla di salvato o il dato non è valido, restano i valori dello script.
void caricaStato() {
  memoria.begin("stella", false);
  esp_reset_reason_t motivo = esp_reset_reason();
  // reset = pin esterno, oppure "accensione" ma con la firma ancora in memoria (dopo una vera riaccensione la firma è sparita)
  bool resetPremuto = (motivo == ESP_RST_EXT) || (motivo == ESP_RST_POWERON && firmaRiavvio == FIRMA_RIAVVIO);
  firmaRiavvio = FIRMA_RIAVVIO;
  bool toccoAvvio = false;
  if (AZZERA_CON_TOCCO) {
    for (int i = 0; i < 10 && !toccoAvvio; i++) {    // il tocco ha bisogno di qualche istante per essere letto
      CoreS3.update();
      toccoAvvio = CoreS3.Touch.getDetail().isPressed();
      delay(20);
    }
  }
  Serial.printf("avvio: motivo %d   reset riconosciuto %d   tocco %d\n", (int)motivo, resetPremuto, toccoAvvio);
  if ((AZZERA_CON_RESET && resetPremuto) || toccoAvvio) {
    memoria.remove("bisogni");
    Serial.println("bisogni azzerati: valori dello script");
  } else if (memoria.getBytesLength("bisogni") == sizeof(Bisogni)) {
    Bisogni letti;
    memoria.getBytes("bisogni", &letti, sizeof(Bisogni));
    if (bisognoValido(letti.nutrimento) && bisognoValido(letti.socialita) && bisognoValido(letti.energia)
        && bisognoValido(letti.umore) && bisognoValido(letti.salute)) {
      bisogni = letti;
      Serial.println("bisogni ripresi dalla memoria");
    }
  }
  bisogniSalvati = bisogni;
  livelloSalvato = calcolaLivelloSalute(bisogni.salute);
}

float differenzaMassima(const Bisogni& a, const Bisogni& b) {
  float d = fabsf(a.nutrimento - b.nutrimento);
  d = max(d, fabsf(a.socialita - b.socialita));
  d = max(d, fabsf(a.energia - b.energia));
  d = max(d, fabsf(a.umore - b.umore));
  d = max(d, fabsf(a.salute - b.salute));
  return d;
}

// Salva ogni SALVATAGGIO_INTERVALLO_S secondi (se qualcosa è cambiato) e, più in fretta, quando cambia il colore della salute.
// Non si salva a ogni fotogramma: la memoria permanente sopporta un numero limitato di scritture.
void salvaStato(uint32_t ora) {
  uint32_t dallUltimo = ora - ultimoSalvataggioMs;
  bool periodico = dallUltimo >= (uint32_t)(SALVATAGGIO_INTERVALLO_S * 1000)
                   && differenzaMassima(bisogni, bisogniSalvati) >= SALVATAGGIO_VARIAZIONE_MIN;
  bool cambioColore = livelloCorrente != livelloSalvato && dallUltimo >= (uint32_t)(SALVATAGGIO_ATTESA_MIN_S * 1000);
  if (!periodico && !cambioColore) return;
  memoria.putBytes("bisogni", &bisogni, sizeof(Bisogni));
  bisogniSalvati = bisogni;
  livelloSalvato = livelloCorrente;
  ultimoSalvataggioMs = ora;
}
