// =====================================================================
//  LOGICA SUONI — richiamo della fame, melodia del nutrimento pieno, battito della carezza
//  e slider del volume. Ogni suono va una nota alla volta guardando l'orologio. Usa suona().
// =====================================================================

// Slider del volume: si trascina il dito nella fascia in basso; quando lo si toglie suona una nota di prova

void aggiornaSlider() {
  if (sensi.toccato && sensi.toccoY >= ALTEZZA - SLIDER_ZONA_ALTEZZA) sliderInUso = true;
  if (!sliderInUso) return;
  if (sensi.premuto) {
    float f = (sensi.toccoX - SLIDER_MARGINE_X) / (float)(LARGHEZZA - 2 * SLIDER_MARGINE_X);
    if (f < 0.0f) f = 0.0f;
    if (f > 1.0f) f = 1.0f;
    volumeCorrente = (int)(f * 255);
    CoreS3.Speaker.setVolume(volumeCorrente);
  } else {                                // dito tolto: fine trascinamento e nota di prova
    sliderInUso = false;
    suona(SLIDER_NOTA_PROVA_HZ, SLIDER_NOTA_PROVA_MS);
  }
}

// Richiamo del nutrimento: quando scende alla soglia suona e poi si ripete a intervalli regolari.
// Suona una nota alla volta, controllando solo l'orologio, senza fermare il resto.

void aggiornaSuonoNutrimento(uint32_t ora) {
  bool sotto = bisogni.nutrimento <= NUTRIMENTO_SOGLIA_SUONO;
  if (sotto && !nutrimentoSottoSoglia) prossimoRichiamoMs = ora;          // appena arrivato: subito
  nutrimentoSottoSoglia = sotto;
  if (sotto && suonoNotaCorrente < 0 && (int32_t)(ora - prossimoRichiamoMs) >= 0) {
    suonoNotaCorrente = 0;
    suonoProssimaNotaMs = ora;
    prossimoRichiamoMs = ora + (uint32_t)(RICHIAMO_RIPETIZIONE_S * 1000);
  }
  if (suonoNotaCorrente < 0) return;
  if ((int32_t)(ora - suonoProssimaNotaMs) < 0) return;                  // non è ancora il momento
  if (SUONO_NOTE_HZ[suonoNotaCorrente] > 0)
    suona(SUONO_NOTE_HZ[suonoNotaCorrente], SUONO_DURATE_MS[suonoNotaCorrente]);
  suonoProssimaNotaMs = ora + SUONO_DURATE_MS[suonoNotaCorrente] + SUONO_PAUSA_MS;
  suonoNotaCorrente++;
  if (suonoNotaCorrente >= SUONO_NUMERO_NOTE) suonoNotaCorrente = -1;    // finito
}

// Melodia del nutrimento al 100%: suona una volta quando, accarezzando, il nutrimento arriva al massimo.
// Per risuonare il nutrimento deve prima scendere sotto PIENO_RIARMO_SOGLIA.

void aggiornaMelodiaPieno(uint32_t ora) {
  if (bisogni.nutrimento < PIENO_RIARMO_SOGLIA) pienoPronto = true;
  bool arrivatoAlMassimo = tempoContinuoS > 0 && nutrimentoPrecedente < BISOGNO_MAX && bisogni.nutrimento >= BISOGNO_MAX;
  nutrimentoPrecedente = bisogni.nutrimento;
  if (arrivatoAlMassimo && pienoPronto) {
    pienoPronto = false;
    pienoNotaCorrente = 0;
    pienoProssimaNotaMs = ora;
    Serial.println("nutrimento al 100%: melodia");
  }
  if (pienoNotaCorrente < 0) return;
  if ((int32_t)(ora - pienoProssimaNotaMs) < 0) return;                   // non è ancora il momento
  if (PIENO_NOTE_HZ[pienoNotaCorrente] > 0)
    suona(PIENO_NOTE_HZ[pienoNotaCorrente], PIENO_DURATE_MS[pienoNotaCorrente]);
  pienoProssimaNotaMs = ora + PIENO_DURATE_MS[pienoNotaCorrente] + PIENO_PAUSA_MS;
  pienoNotaCorrente++;
  if (pienoNotaCorrente >= PIENO_NUMERO_NOTE) pienoNotaCorrente = -1;     // finita
}

// Battito della carezza: finché la si accarezza suona una nota breve ogni BATTITO_INTERVALLO_S secondi
// (non quando sta suonando il richiamo del nutrimento o la melodia, per non sovrapporli).

void aggiornaSuonoCarezza(uint32_t ora) {
  if (tempoContinuoS <= 0) { battitoAttivo = false; return; }
  if (!battitoAttivo) { battitoAttivo = true; prossimoBattitoMs = ora; }   // inizio carezza: primo battito subito
  if ((int32_t)(ora - prossimoBattitoMs) < 0) return;
  prossimoBattitoMs = ora + (uint32_t)(BATTITO_INTERVALLO_S * 1000);
  if (suonoNotaCorrente < 0 && pienoNotaCorrente < 0 && festaNotaCorrente < 0) suona(BATTITO_HZ, BATTITO_DURATA_MS);
}
