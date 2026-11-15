// =====================================================================
//  SENSORI — l'unico modulo che parla con l'hardware: giroscopio, accelerometro,
//  tocco, microfono e altoparlante. Scrive in "sensi"; suona() manda le note.
// =====================================================================

void calibraGiroscopio() {
  Serial.println("calibrazione: tieni fermo il dispositivo...");
  float sx = 0, sy = 0, sz = 0;
  int letti = 0;
  for (int i = 0; i < CAMPIONI_CALIBRAZIONE; i++) {
    float gx, gy, gz;
    if (CoreS3.Imu.getGyro(&gx, &gy, &gz)) { sx += gx; sy += gy; sz += gz; letti++; }
    delay(10);
  }
  if (letti > 0) { offsetX = sx / letti; offsetY = sy / letti; offsetZ = sz / letti; }
  Serial.printf("offset giroscopio  x %.2f  y %.2f  z %.2f\n", offsetX, offsetY, offsetZ);
}

float zonaMorta(float v) {
  return fabsf(v) < ZONA_MORTA_GRADI_S ? 0 : v;
}

// Microfono e altoparlante usano lo stesso canale audio e non possono lavorare insieme:
// di solito ascolta il microfono; quando serve un suono si passa all'altoparlante e, finito il suono, si torna al microfono.
// Tutti i suoni passano di qui (al posto di CoreS3.Speaker.tone): se il microfono è acceso, lo spegne e riaccende l'altoparlante
void suona(float hz, uint32_t durataMs) {
  if (micAttivo) {
    CoreS3.Mic.end();
    CoreS3.Speaker.begin();
    micAttivo = false;
    micHaDati = false;
    volumeVoceLetto = 0;
  }
  CoreS3.Speaker.tone(hz, durataMs);
  fineSuonoMs = millis() + durataMs;
}

// Volume dell'altoparlante (0-255): lo sceglie il menu, qui lo si manda all'hardware
void impostaVolume(uint8_t volume) {
  CoreS3.Speaker.setVolume(volume);
}

// Misura il volume dell'ambiente (0-1, stessa scala della pagina Microfono dell'esploratore) senza fermare il loop
void leggiMicrofono(uint32_t ora) {
  if (!micAttivo) {
    bool libero = !suonoInCorso && !CoreS3.Speaker.isPlaying() && (int32_t)(ora - fineSuonoMs) >= (int32_t)MIC_ATTESA_DOPO_SUONO_MS;
    if (!libero) { sensi.volumeVoce = 0; return; }
    CoreS3.Speaker.end();
    CoreS3.Mic.begin();
    micAttivo = true;
    micHaDati = false;
  }
  if (CoreS3.Mic.isEnabled() && !CoreS3.Mic.isRecording()) {
    if (micHaDati) {                                              // c'è una misura pronta: la trasformo in volume
      double somma = 0;
      for (size_t i = 0; i < MIC_CAMPIONI; i++) somma += (double)bufferMic[i] * bufferMic[i];
      float rms = sqrtf((float)(somma / MIC_CAMPIONI)) / 32768.0f;
      float db = 20.0f * log10f(rms < 1e-5f ? 1e-5f : rms);
      volumeVoceLetto = constrain((db + 60.0f) / 60.0f, 0.0f, 1.0f);
    }
    micHaDati = CoreS3.Mic.record(bufferMic, MIC_CAMPIONI, MIC_FREQUENZA_HZ);   // chiede la misura successiva
  }
  sensi.volumeVoce = volumeVoceLetto;
}

// Giorni dal 1° gennaio 2000 a una data (calendario normale, con gli anni bisestili)
uint32_t giorniDal2000(int anno, int mese, int giorno) {
  anno -= mese <= 2;                                              // gennaio e febbraio contano come fine dell'anno prima
  int era = anno / 400;
  int annoEra = anno - era * 400;
  int giornoAnno = (153 * (mese + (mese > 2 ? -3 : 9)) + 2) / 5 + giorno - 1;
  int giornoEra = annoEra * 365 + annoEra / 4 - annoEra / 100 + giornoAnno;
  return (uint32_t)(era * 146097 + giornoEra - 730425);         // 730425 = giorni dall'anno 0 al 2000
}

// Orologio della board (RTC): va a batteria tampone, quindi conta il tempo anche da spenta.
// Si legge una volta al secondo; se non risponde, orologioOk resta falso e si usa millis().
void leggiOrologio(uint32_t ora) {
  if (ultimaLetturaOrologioMs != 0 && ora - ultimaLetturaOrologioMs < OROLOGIO_INTERVALLO_MS) return;
  ultimaLetturaOrologioMs = ora;
  sensi.orologioOk = false;
  if (!CoreS3.Rtc.isEnabled()) {
    if (!orologioAvvisato) { Serial.println("orologio: non disponibile, la pausa conta solo a board accesa"); orologioAvvisato = true; }
    return;
  }
  auto dt = CoreS3.Rtc.getDateTime();
  if (dt.date.year < 2000 || dt.date.month < 1 || dt.date.month > 12 || dt.date.date < 1) {
    // orologio mai regolato: lo si fa partire da una data qualsiasi (OROLOGIO_ANNO_PARTENZA): conta solo il tempo che passa
    m5::rtc_datetime_t partenza;
    partenza.date.year = OROLOGIO_ANNO_PARTENZA; partenza.date.month = 1; partenza.date.date = 1; partenza.date.weekDay = 0;
    partenza.time.hours = 0; partenza.time.minutes = 0; partenza.time.seconds = 0;
    CoreS3.Rtc.setDateTime(partenza);
    Serial.printf("orologio: non era regolato, riparte dal 1/1/%d\n", OROLOGIO_ANNO_PARTENZA);
    return;
  }
  sensi.orologioS = giorniDal2000(dt.date.year, dt.date.month, dt.date.date) * 86400UL
                    + dt.time.hours * 3600UL + dt.time.minutes * 60UL + dt.time.seconds;
  sensi.orologioOk = true;
}

void leggiSensori() {
  leggiOrologio(millis());
  auto tocco = CoreS3.Touch.getDetail();
  sensi.toccato = tocco.wasPressed();
  sensi.premuto = tocco.isPressed();
  sensi.toccoX  = tocco.x;
  sensi.toccoY  = tocco.y;
  sensi.cliccato = tocco.wasClicked();
  sensi.swipe   = tocco.wasFlicked();
  sensi.swipeDX = tocco.distanceX();
  sensi.swipeDY = tocco.distanceY();
  leggiMicrofono(millis());

  float gx, gy, gz;
  sensi.imuOk = CoreS3.Imu.isEnabled() && CoreS3.Imu.getGyro(&gx, &gy, &gz);
  float ax, ay, az;                                                // scossa = quanto l'accelerazione si allontana dalla sola gravità (1 g)
  sensi.scossa = 0;
  if (sensi.imuOk && CoreS3.Imu.getAccel(&ax, &ay, &az)) sensi.scossa = fabsf(sqrtf(ax * ax + ay * ay + az * az) - 1.0f);
  if (!sensi.imuOk) { sensi.giroX = sensi.giroY = sensi.giroZ = 0; return; }
  sensi.giroX = zonaMorta(gx - offsetX) * SEGNO_ASSE_X;
  sensi.giroY = zonaMorta(gy - offsetY) * SEGNO_ASSE_Y;
  sensi.giroZ = zonaMorta(gz - offsetZ) * SEGNO_ASSE_Z;
}
