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

void leggiSensori() {
  auto tocco = CoreS3.Touch.getDetail();
  sensi.toccato = tocco.wasPressed();
  sensi.premuto = tocco.isPressed();
  sensi.toccoX  = tocco.x;
  sensi.toccoY  = tocco.y;
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
