// =====================================================================
//  LOGICA MOVIMENTO — energia bassa = stella ferma; orbita quando è attiva; festa quando
//  l'energia arriva al 100%. Orbita e festa si scelgono con ORBITA_TIPO e FESTA_TIPO. Scrive movimento e profondità.
// =====================================================================

// Attività: con energia bassa la stella si ferma al centro; per ripartire deve superare una soglia più alta
// (così non balla fra i due stati). Durante la festa è ferma, perché il percorso è deciso dalla festa.
void aggiornaAttivita(float dt) {
  if (bisogni.energia <= ENERGIA_SOGLIA_STATICA) staticaEnergia = true;
  else if (bisogni.energia > ENERGIA_SOGLIA_USCITA_STATICA) staticaEnergia = false;
  float bersaglio = (staticaEnergia || festaAttiva || caneAttivo) ? 0.0f : 1.0f;   // anche durante il cane l'orbita si ferma
  float k = ENERGIA_ATTIVITA_VELOCITA * dt;
  if (k > 1.0f) k = 1.0f;
  attivita += (bersaglio - attivita) * k;
}

// Durata del percorso della festa scelta con FESTA_TIPO
float festaDurataPercorsoS() {
  switch (FESTA_TIPO) {
    case FESTA_SALTI_BORDO: return festaDurataSaltiBordoS();
    case FESTA_RIMBALZI:    return festaDurataRimbalziS();
  }
  return 0;
}

// Dove si trova la stella t secondi dopo l'inizio della festa scelta con FESTA_TIPO
void festaPosizione(float t, float& x, float& y) {
  switch (FESTA_TIPO) {
    case FESTA_SALTI_BORDO: festaPosizioneSaltiBordo(t, x, y); break;
    case FESTA_RIMBALZI:    festaPosizioneRimbalzi(t, x, y);   break;
  }
}

// Sirio: punto del bordo (rettangolo dentro lo schermo, a SIRIO_FESTA_MARGINE_PX dal bordo) dopo aver fatto "s" px
// in senso orario partendo dal centro del lato alto. Coordinate rispetto al centro dello schermo.
void festaPuntoBordo(float s, float& x, float& y) {
  float bx = LARGHEZZA / 2.0f - SIRIO_FESTA_MARGINE_PX;
  float by = ALTEZZA / 2.0f - SIRIO_FESTA_MARGINE_PX;
  s = fmodf(s, 4 * (bx + by));
  if (s < bx)      { x = s;        y = -by;       return; }   // lato alto verso destra
  s -= bx;
  if (s < 2 * by)  { x = bx;       y = -by + s;   return; }   // lato destro verso il basso
  s -= 2 * by;
  if (s < 2 * bx)  { x = bx - s;   y = by;        return; }   // lato basso verso sinistra
  s -= 2 * bx;
  if (s < 2 * by)  { x = -bx;      y = by - s;    return; }   // lato sinistro verso l'alto
  s -= 2 * by;
  x = -bx + s; y = -by;                                       // ultimo pezzo del lato alto
}

// Sirio: durata del percorso = andata al bordo + salti + ritorno al centro
float festaDurataSaltiBordoS() {
  return 2.0f * SIRIO_FESTA_TRANSIZIONE_S + SIRIO_FESTA_GIRI * SIRIO_FESTA_SALTI_PER_GIRO * SIRIO_FESTA_SALTO_S;
}

// Sirio: dove si trova la stella t secondi dopo l'inizio della festa. Va al bordo, fa i salti lungo il bordo
// (in ogni salto si stacca verso l'interno e ricade piano, come con poca gravità) e poi torna al centro.
void festaPosizioneSaltiBordo(float t, float& x, float& y) {
  float by = ALTEZZA / 2.0f - SIRIO_FESTA_MARGINE_PX;
  float durataSalti = SIRIO_FESTA_GIRI * SIRIO_FESTA_SALTI_PER_GIRO * SIRIO_FESTA_SALTO_S;
  if (t < SIRIO_FESTA_TRANSIZIONE_S) {                     // dal centro al bordo alto
    x = 0; y = -by * partenzaArrivoDolci(t / SIRIO_FESTA_TRANSIZIONE_S);
    return;
  }
  t -= SIRIO_FESTA_TRANSIZIONE_S;
  if (t < durataSalti) {
    float giro = 4 * ((LARGHEZZA / 2.0f - SIRIO_FESTA_MARGINE_PX) + by);
    float s = t / SIRIO_FESTA_SALTO_S * giro / SIRIO_FESTA_SALTI_PER_GIRO;      // strada fatta lungo il bordo
    float u = fmodf(t, SIRIO_FESTA_SALTO_S) / SIRIO_FESTA_SALTO_S;              // a che punto del salto (0-1)
    float h = SIRIO_FESTA_ALTEZZA_SALTO_PX * 4.0f * u * (1.0f - u);             // parabola: sale piano, ricade piano
    festaPuntoBordo(s, x, y);
    float d = sqrtf(x * x + y * y);
    if (d > 0.001f) { x -= x / d * h; y -= y / d * h; }                         // il salto va verso il centro dello schermo
    return;
  }
  t -= durataSalti;
  float f = t / SIRIO_FESTA_TRANSIZIONE_S;
  if (f > 1.0f) f = 1.0f;
  x = 0; y = -by * (1.0f - partenzaArrivoDolci(f));        // dal bordo al centro
}

// Vega: durata del percorso della festa. Tratti: centro -> bordo, (rimbalzi - 1) traversate, bordo -> centro.
float festaDurataRimbalziS() {
  float y = ALTEZZA / 2.0f - VEGA_FESTA_MARGINE_PX;
  return 2.0f * VEGA_FESTA_RIMBALZI * y / VEGA_FESTA_VELOCITA_PX_S;
}

// Vega: dove si trova la stella (rispetto al centro) t secondi dopo l'inizio della festa. Rimbalza fra bordo alto e basso a velocità costante.
void festaPosizioneRimbalzi(float t, float& x, float& y) {
  float yb = ALTEZZA / 2.0f - VEGA_FESTA_MARGINE_PX;      // distanza del centro stella dal centro schermo quando tocca il bordo
  float s = VEGA_FESTA_VELOCITA_PX_S * t;                  // strada fatta
  x = 0;
  if (s < yb) { y = -s; return; }                          // dal centro al bordo alto
  s -= yb;
  int k = (int)(s / (2 * yb));                             // quale traversata
  if (k < VEGA_FESTA_RIMBALZI - 1) {
    float r = s - k * 2 * yb;
    y = (k % 2 == 0) ? -yb + r : yb - r;                   // alto -> basso, poi basso -> alto, ...
    return;
  }
  s -= (VEGA_FESTA_RIMBALZI - 1) * 2 * yb;                 // ultimo tratto: dal bordo al centro
  float bordo = ((VEGA_FESTA_RIMBALZI - 1) % 2 == 0) ? -yb : yb;
  float f = s / yb;
  if (f > 1.0f) f = 1.0f;
  y = bordo * (1.0f - f);
}

// Festa: parte quando, scuotendo, l'energia arriva al 100% (e prima era scesa sotto FESTA_RIARMO_SOGLIA).
// Dura il percorso più FESTA_FERMO_S secondi fermi al centro. Suona il suono della festa, una nota alla volta.
void aggiornaFesta(uint32_t ora) {
  if (bisogni.energia < FESTA_RIARMO_SOGLIA) festaPronta = true;
  bool arrivata = scossaRecente && energiaPrecedente < BISOGNO_MAX && bisogni.energia >= BISOGNO_MAX;
  energiaPrecedente = bisogni.energia;
  if (arrivata && festaPronta && !festaAttiva && !malataVisibile()) {   // da malata niente festa (logica_malata.ino)
    festaPronta = false;
    festaAttiva = true;
    festaInizioMs = ora;
    festaNotaCorrente = 0;
    festaProssimaNotaMs = ora;
    Serial.println("energia al 100%: festa");
  }
  festaX = 0; festaY = 0;
  if (festaAttiva) {
    float t = (ora - festaInizioMs) / 1000.0f;
    float durata = festaDurataPercorsoS();
    if (t >= durata + FESTA_FERMO_S) festaAttiva = false;
    else if (t < durata) festaPosizione(t, festaX, festaY);
  }
  if (festaNotaCorrente < 0) return;
  if ((int32_t)(ora - festaProssimaNotaMs) < 0) return;                   // non è ancora il momento
  if (FESTA_NOTE_HZ[festaNotaCorrente] > 0)
    suona(FESTA_NOTE_HZ[festaNotaCorrente], FESTA_DURATE_MS[festaNotaCorrente]);
  festaProssimaNotaMs = ora + FESTA_DURATE_MS[festaNotaCorrente] + FESTA_PAUSA_MS;
  festaNotaCorrente++;
  if (festaNotaCorrente >= FESTA_NUMERO_NOTE) festaNotaCorrente = -1;     // finito
}

// Orbita scelta con ORBITA_TIPO. Tutto è moltiplicato per "attivita": a energia bassa o durante la festa torna piano al centro.
void aggiornaMovimento(uint32_t ora) {
  switch (ORBITA_TIPO) {
    case ORBITA_OTTO:    aggiornaOrbitaOtto(ora);    break;
    case ORBITA_CASUALE: aggiornaOrbitaCasuale(ora); break;
  }
  movimentoX = orbitaX + festaX;
  movimentoY = orbitaY + festaY;
}

// Sirio: orbita a 8 senza profondità (x segue il seno, y ne fa due nello stesso tempo).
void aggiornaOrbitaOtto(uint32_t ora) {
  float fase = 2 * PI * (ora / 1000.0f) / SIRIO_ORBITA_PERIODO_S;
  orbitaX = SIRIO_ORBITA_AMPIEZZA_X_PX * sinf(fase) * attivita;
  orbitaY = SIRIO_ORBITA_AMPIEZZA_Y_PX * sinf(2 * fase) * attivita;
  profondita = 0;
}

// Un'oscillazione irregolare fra -1 e 1: due onde con durate che non si sovrappongono mai, così l'orbita non si ripete
float ondaOrbita(float t, float periodoS, float fase) {
  float prima = sinf(2 * PI * t / periodoS + fase);
  float seconda = sinf(2 * PI * t / (periodoS * ORBITA_SECONDA_ONDA_RAPPORTO) + fase + ORBITA_SECONDA_ONDA_FASE);
  return (1.0f - ORBITA_SECONDA_ONDA_PESO) * prima + ORBITA_SECONDA_ONDA_PESO * seconda;
}

// Vega: orbite casuali con profondità.
void aggiornaOrbitaCasuale(uint32_t ora) {
  float t = ora / 1000.0f;
  orbitaX = VEGA_ORBITA_AMPIEZZA_X_PX * ondaOrbita(t, VEGA_ORBITA_PERIODO_X_S, ORBITA_FASI[0]) * attivita;
  orbitaY = VEGA_ORBITA_AMPIEZZA_Y_PX * ondaOrbita(t, VEGA_ORBITA_PERIODO_Y_S, ORBITA_FASI[1]) * attivita;
  profondita = VEGA_PROFONDITA * ondaOrbita(t, VEGA_ORBITA_PERIODO_Z_S, ORBITA_FASI[2]) * attivita;
}

// Applica il movimento all'aspetto già calcolato: sposta la stella e ne cambia la grandezza (profondità)
void applicaMovimento() {
  aspetto.spostamentoX += movimentoX;
  aspetto.spostamentoY += movimentoY;
  aspetto.raggio *= 1.0f + profondita;
}
