// =====================================================================
//  LOGICA BRANO — la musica di Unisono con MUSICA_TIPO = MUSICA_BRANO: ogni stella suona la sua parte
//  di brano.h (Vega le note alte, Sirio le basse) dall'inizio della galassia, fino a BRANO_VOCI note insieme.
//  Legge brano.h, unisonoAnimazioneMs e galassia; usa suonaSuCanale(); scrive luceMia, galassia[i].luce e brano*.
// =====================================================================

// La parte di questa stella e quella dell'altra (si sceglie con BRANO_PARTE in config_sirio.h / config_vega.h)
const NotaBrano* partePropria() { return BRANO_PARTE == PARTE_ALTA ? BRANO_VEGA : BRANO_SIRIO; }
int numeroNotePropria()         { return BRANO_PARTE == PARTE_ALTA ? BRANO_VEGA_NUMERO : BRANO_SIRIO_NUMERO; }
const NotaBrano* parteAltra()   { return BRANO_PARTE == PARTE_ALTA ? BRANO_SIRIO : BRANO_VEGA; }
int numeroNoteAltra()           { return BRANO_PARTE == PARTE_ALTA ? BRANO_SIRIO_NUMERO : BRANO_VEGA_NUMERO; }

// Una voce libera (la sua nota è finita); se sono tutte occupate, quella che finisce per prima
int scegliVoceLibera(uint32_t ora) {
  int migliore = 0;
  for (int v = 0; v < BRANO_VOCI; v++) {
    if ((int32_t)(ora - branoFineVoceMs[v]) >= 0) return v;
    if ((int32_t)(branoFineVoceMs[v] - branoFineVoceMs[migliore]) < 0) migliore = v;
  }
  return migliore;
}

// Suona una nota su una voce libera: le note forti nell'originale si sentono più forti, la stella si illumina
void suonaNotaBrano(const NotaBrano& nota, uint32_t ora) {
  int voce = scegliVoceLibera(ora);
  int canale = BRANO_CANALE_PRIMO + voce;
  float forza = BRANO_FORZA_MINIMA + (1.0f - BRANO_FORZA_MINIMA) * nota.forza / 100.0f;
  impostaVolumeCanale(canale, (uint8_t)(BRANO_VOLUME * forza));
  uint32_t durata = (uint32_t)(nota.durataMs * BRANO_LEGATO);
  suonaSuCanale(nota.hz, durata, canale);
  branoFineVoceMs[voce] = ora + durata;
  luceMia = 1;
}

// Quando suona l'altra parte si illuminano le stelle della galassia (l'altra stella suona sul suo dispositivo)
void illuminaStelleGalassia() {
  for (int i = 0; i < numeroGalassia; i++) illuminaStellaGalassia(i);
}

// Ogni fotogramma durante Unisono: suona le note della propria parte il cui momento è arrivato
// e illumina l'altra stella quando parte una nota della sua parte
void aggiornaBrano(uint32_t ora, bool attiva) {
  if (!attiva) return;
  uint32_t inizio = unisonoAnimazioneMs + BRANO_RITARDO_MS;
  if ((int32_t)(ora - inizio) < 0) return;                             // non è ancora il momento
  uint32_t t = ora - inizio;
  const NotaBrano* mia = partePropria();
  while (branoProssimaMia < numeroNotePropria() && mia[branoProssimaMia].inizioMs <= t) {
    suonaNotaBrano(mia[branoProssimaMia], ora);
    branoProssimaMia++;
  }
  const NotaBrano* altra = parteAltra();
  bool altraSuona = false;
  while (branoProssimaAltra < numeroNoteAltra() && altra[branoProssimaAltra].inizioMs <= t) {
    altraSuona = true;
    branoProssimaAltra++;
  }
  if (altraSuona) illuminaStelleGalassia();
}

// All'inizio di Unisono il brano riparte dalla prima nota
void azzeraBrano() {
  branoProssimaMia = 0;
  branoProssimaAltra = 0;
  for (int v = 0; v < BRANO_VOCI; v++) branoFineVoceMs[v] = 0;
}

// Alla fine di Unisono i canali del brano tornano a volume pieno, come gli altri
void ripristinaVolumiBrano() {
  for (int v = 0; v < BRANO_VOCI; v++) impostaVolumeCanale(BRANO_CANALE_PRIMO + v, 255);
}
