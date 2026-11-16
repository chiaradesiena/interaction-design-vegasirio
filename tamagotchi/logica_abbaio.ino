// =====================================================================
//  LOGICA ABBAIO — il cane di Sirio abbaia durante la passeggiata ("bau bau", grave e piano).
//  Il suono si costruisce una volta sola, campione per campione, poi si suona con suonaCampioni().
//  Legge caneAttivo, caneInRitorno e caneUPasso; scrive abbaioCampioni e lo stato dell'abbaio.
// =====================================================================

// Tono del "bau" al punto u (0-1): sale un poco fino al picco, poi scende piano
float tonoAbbaio(float u) {
  if (u < SIRIO_ABBAIO_PICCO)
    return SIRIO_ABBAIO_TONO_INIZIO_HZ + (SIRIO_ABBAIO_TONO_PICCO_HZ - SIRIO_ABBAIO_TONO_INIZIO_HZ) * u / SIRIO_ABBAIO_PICCO;
  float f = (u - SIRIO_ABBAIO_PICCO) / (1.0f - SIRIO_ABBAIO_PICCO);
  return SIRIO_ABBAIO_TONO_PICCO_HZ + (SIRIO_ABBAIO_TONO_FINE_HZ - SIRIO_ABBAIO_TONO_PICCO_HZ) * f;
}

// Forza del "bau" al punto u (0-1): arriva al massimo in SIRIO_ABBAIO_ATTACCO_MS, resta forte fino a SIRIO_ABBAIO_TENUTA, poi si spegne
float forzaAbbaio(float u) {
  float attacco = SIRIO_ABBAIO_ATTACCO_MS / (float)SIRIO_ABBAIO_DURATA_MS;
  if (u < attacco) return u / attacco;
  if (u < SIRIO_ABBAIO_TENUTA) return 1.0f;
  float f = (u - SIRIO_ABBAIO_TENUTA) / (1.0f - SIRIO_ABBAIO_TENUTA);
  return (1.0f - f) * (1.0f - f);
}

// Costruisce il "bau": un tono (con due armoniche, così è più caldo di un fischio) mescolato a un soffio,
// reso un po' "graffiato" da un tremolio veloce della forza.
void creaAbbaio() {
  size_t n = (size_t)((uint32_t)SIRIO_ABBAIO_DURATA_MS * SIRIO_ABBAIO_CAMPIONI_HZ / 1000);
  if (n > (size_t)SIRIO_ABBAIO_MAX_CAMPIONI) n = SIRIO_ABBAIO_MAX_CAMPIONI;
  float fase = 0, soffio = 0;
  uint32_t casuale = SEME_CIELO;                                  // numeri casuali ripetibili per il soffio
  for (size_t i = 0; i < n; i++) {
    float u = i / (float)n;                                       // a che punto del "bau" (0-1)
    fase += 2 * PI * tonoAbbaio(u) / SIRIO_ABBAIO_CAMPIONI_HZ;
    float voce = (sinf(fase) + 0.5f * sinf(2 * fase) + 0.25f * sinf(3 * fase)) / 1.75f;
    casuale = casuale * 1664525UL + 1013904223UL;
    float rumore = ((casuale >> 8) / 8388608.0f) - 1.0f;          // fra -1 e 1
    soffio += (rumore - soffio) * 0.3f;                           // soffio "morbido" (toglie i fruscii più acuti)
    float tempoS = i / (float)SIRIO_ABBAIO_CAMPIONI_HZ;
    float graffio = 1.0f - SIRIO_ABBAIO_RUVIDEZZA * 0.5f * (1.0f + sinf(2 * PI * SIRIO_ABBAIO_RUVIDEZZA_HZ * tempoS));
    float forza = forzaAbbaio(u) * graffio;
    float s = ((1.0f - SIRIO_ABBAIO_FRUSCIO) * voce + SIRIO_ABBAIO_FRUSCIO * soffio * 2.0f) * forza * SIRIO_ABBAIO_VOLUME;
    abbaioCampioni[i] = (int16_t)(constrain(s, -1.0f, 1.0f) * 32000);
  }
  abbaioNumeroCampioni = n;
}

// Ogni fotogramma: quando la passeggiata arriva a uno dei SIRIO_ABBAI_MOMENTI parte un abbaio
// (SIRIO_ABBAIO_RIPETIZIONI "bau" di fila). Se il cane viene interrotto o finisce, smette di abbaiare.
void aggiornaAbbaio(uint32_t ora) {
  if (STATO_PROPRIO_TIPO != PROPRIO_CANE) return;
  if (abbaioNumeroCampioni == 0) creaAbbaio();
  if (!caneAttivo || caneInRitorno) { abbaioProssimo = 0; abbaioUfRimasti = 0; return; }
  if (abbaioProssimo < SIRIO_ABBAI_NUMERO && caneUPasso >= SIRIO_ABBAI_MOMENTI[abbaioProssimo]) {
    abbaioProssimo++;
    abbaioUfRimasti = SIRIO_ABBAIO_RIPETIZIONI;
    abbaioProssimoUfMs = ora;
    Serial.println("cane: abbaia");
  }
  if (abbaioUfRimasti <= 0 || (int32_t)(ora - abbaioProssimoUfMs) < 0) return;
  suonaCampioni(abbaioCampioni, abbaioNumeroCampioni, SIRIO_ABBAIO_CAMPIONI_HZ);
  abbaioUfRimasti--;
  abbaioProssimoUfMs = ora + SIRIO_ABBAIO_DURATA_MS + SIRIO_ABBAIO_PAUSA_MS;
}
