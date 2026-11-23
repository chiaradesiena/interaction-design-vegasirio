// =====================================================================
//  LOGICA CAREZZA — riconosce la carezza col dito, fa crescere e tremare la stella.
//  Il modo di crescere si sceglie con CRESCITA_TIPO. Legge "sensi", scrive scala e tremolio.
// =====================================================================

float partenzaArrivoDolci(float t) {      // da 0-1 a 0-1, lento all'inizio e alla fine
  return t * t * (3.0f - 2.0f * t);
}

// Fa crescere la stella col modo scelto con CRESCITA_TIPO.
// Finita la carezza (e passata l'attesa) il bersaglio torna 1 e la stella si rimpicciolisce dolcemente.
void aggiornaScalaCarezza(float dt) {
  float bersaglio = 1.0f;
  switch (CRESCITA_TIPO) {
    case CRESCITA_TRATTI:   bersaglio = bersaglioCrescitaTratti();   break;
    case CRESCITA_CONTINUA: bersaglio = bersaglioCrescitaContinua(); break;
  }
  aggiornaMolla(scalaCarezza, bersaglio, CAREZZA_RITORNO_FREQUENZA, 1.0f, dt);
}

// Sirio: durante la carezza la stella segue i tratti SIRIO_* (cresce, si riduce, cresce ancora) e alla fine resta all'ultima grandezza.
float bersaglioCrescitaTratti() {
  float t = tempoCrescitaS;
  float scalaPrima = 1.0f;
  float bersaglio = SIRIO_SCALA[SIRIO_NUMERO_TRATTI - 1];        // finiti i tratti: resta qui
  for (int i = 0; i < SIRIO_NUMERO_TRATTI; i++) {
    if (t < SIRIO_DURATE_S[i]) {                                  // siamo dentro il tratto i
      bersaglio = scalaPrima + (SIRIO_SCALA[i] - scalaPrima) * partenzaArrivoDolci(t / SIRIO_DURATE_S[i]);
      break;
    }
    t -= SIRIO_DURATE_S[i];
    scalaPrima = SIRIO_SCALA[i];
  }
  return bersaglio;
}

// Vega: più la carezza è continua, più la stella cresce, fino a occupare CAREZZA_OCCUPAZIONE_MAX dello schermo.
float bersaglioCrescitaContinua() {
  float scalaMax = CAREZZA_OCCUPAZIONE_MAX * (ALTEZZA / 2.0f) / (STELLA_RAGGIO * STELLA_SCALA * (1 + PULSAZIONE_AMPIEZZA));   // anche ingrandita, la stella non esce dallo schermo
  float k = tempoCrescitaS / VEGA_DURATA_CRESCITA_S;
  if (k > 1.0f) k = 1.0f;
  float bersaglio = 1.0f + (scalaMax - 1.0f) * partenzaArrivoDolci(k);
  return bersaglio;
}

// Carezza: dito appoggiato sulla stella che si muove soprattutto in orizzontale.
// Qui si decide se si sta accarezzando e si accumulano i tempi; il nutrimento sale in aggiornaNutrimento.
void aggiornaCarezza(uint32_t ora, float dt) {
  if (sensi.premuto && carezzaPrecPremuto && dt > 0) {            // velocità del dito, un po' lisciata
    float vx = (sensi.toccoX - carezzaPrecX) / dt;
    float vy = (sensi.toccoY - carezzaPrecY) / dt;
    carezzaVelX += (vx - carezzaVelX) * 0.4f;
    carezzaVelY += (vy - carezzaVelY) * 0.4f;
  } else {
    carezzaVelX = 0; carezzaVelY = 0;
  }
  carezzaPrecPremuto = sensi.premuto;
  carezzaPrecX = sensi.toccoX;
  carezzaPrecY = sensi.toccoY;

  float dx = sensi.toccoX - (LARGHEZZA / 2 + dragX.valore + movimentoX);
  float dy = sensi.toccoY - (ALTEZZA / 2 + dragY.valore + movimentoY);
  float r = aspetto.raggio * CAREZZA_AREA_STELLA;
  bool sullaStella = sensi.premuto && !sliderInUso && !festaAttiva && dx * dx + dy * dy <= r * r;   // durante la festa la carezza non conta
  carezzando = sullaStella && fabsf(carezzaVelX) >= CAREZZA_VELOCITA_MIN_PX_S
                           && fabsf(carezzaVelX) >= fabsf(carezzaVelY);

  if (carezzando) {
    ultimaCarezzaMs = ora;
    tempoContinuoS += dt;
    tempoCrescitaS += dt;
  }
  uint32_t daUltima = ora - ultimaCarezzaMs;
  if (daUltima > CAREZZA_PAUSA_TOLLERATA_MS) tempoContinuoS = 0;
  if (daUltima > CAREZZA_RITORNO_ATTESA_MS)  tempoCrescitaS = 0;
  aggiornaScalaCarezza(dt);
  aggiornaTremolio(ora, dt);
}

// Tremolio: mentre la si accarezza la stella vibra appena. Si sommano due oscillazioni a ritmi diversi
// per non farlo sembrare meccanico; parte e si spegne in modo graduale.
void aggiornaTremolio(uint32_t ora, float dt) {
  float bersaglio = (tempoContinuoS > 0) ? 1.0f : 0.0f;
  float k = TREMOLIO_VELOCITA_CAMBIO * dt;
  if (k > 1.0f) k = 1.0f;
  tremolioIntensita += (bersaglio - tremolioIntensita) * k;
  float t = ora / 1000.0f;
  float w = 2 * PI * TREMOLIO_FREQUENZA_HZ;
  float a = TREMOLIO_AMPIEZZA_PX * tremolioIntensita / 1.5f;
  tremolioX = a * (sinf(w * t) + 0.5f * sinf(w * 1.7f * t + 1.3f));
  tremolioY = a * (sinf(w * 1.3f * t + 0.7f) + 0.5f * sinf(w * 2.1f * t));
}
