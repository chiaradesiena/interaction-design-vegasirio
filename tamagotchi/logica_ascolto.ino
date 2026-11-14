// =====================================================================
//  LOGICA ASCOLTO — come reagisce la stella quando qualcuno parla vicino.
//  Il tipo di reazione si sceglie con ASCOLTO_TIPO (config_sirio.h / config_vega.h). Scrive in "aspetto".
// =====================================================================

// Ascolto: finché qualcuno parla (voceAttiva) il valore sale verso 1, poi torna piano verso 0.
void aggiornaAscolto(float dt) {
  bool ascoltando = voceAttiva && !festaAttiva;                  // durante la festa l'ascolto non si vede
  float velocita = ascoltando ? ASCOLTO_VELOCITA_SALITA : ASCOLTO_VELOCITA_DISCESA;
  float k = velocita * dt;
  if (k > 1.0f) k = 1.0f;
  ascolto += ((ascoltando ? 1.0f : 0.0f) - ascolto) * k;
}

// Applica la reazione scelta con ASCOLTO_TIPO all'aspetto già calcolato
void applicaAscolto() {
  switch (ASCOLTO_TIPO) {
    case ASCOLTO_PALLINO: applicaAscoltoPallino(); break;
    case ASCOLTO_RAGGI:   applicaAscoltoRaggi();   break;
  }
}

// Sirio: durante l'ascolto la stella si rimpicciolisce e le punte si "gonfiano" fino a un cerchio (vita = 1):
// con la griglia di puntini diventa un pallino halftone. Si parte dall'aspetto già calcolato e si modificano solo raggio e vita.
void applicaAscoltoPallino() {
  aspetto.raggio = aspetto.raggio + (SIRIO_ASCOLTO_RAGGIO_PX - aspetto.raggio) * ascolto;
  aspetto.vita   = aspetto.vita + (1.0f - aspetto.vita) * ascolto;
}

// Vega: durante l'ascolto i raggi si allungano ma il nucleo (la parte fra un raggio e l'altro) e il buco della fame
// restano della stessa grandezza in px. Si parte dall'aspetto già calcolato e si modifica solo raggio, vita e buco.
void applicaAscoltoRaggi() {
  float raggioBase = aspetto.raggio;
  float raggioAscolto = max(raggioBase, VEGA_ASCOLTO_RAGGIO_PX);
  float raggioNuovo = raggioBase + (raggioAscolto - raggioBase) * ascolto;
  aspetto.vita = aspetto.vita * raggioBase / raggioNuovo;
  aspetto.buco = aspetto.buco * raggioBase / raggioNuovo;
  aspetto.raggio = raggioNuovo;
}
