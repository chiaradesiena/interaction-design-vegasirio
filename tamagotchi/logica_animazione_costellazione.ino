// =====================================================================
//  LOGICA ANIMAZIONE COSTELLAZIONE — i 10 secondi in cui si aggiunge una stella alla costellazione:
//  la stella scompare, lo schermo diventa bianco, due puntini neri lampeggiano, fra loro si disegna un tratto,
//  lo schermo torna nero, ritorna il cielo, ricompare la stella lampeggiando con la stella nuova.
//  Intanto i bisogni restano fermi e tocchi, voce e scosse non contano. Scrive le variabili anim* e "aspetto".
// =====================================================================

// Fa partire l'animazione: ferma la posizione della stella principale, attorno alla quale sul bianco
// si disegnano in nero tutte le stelle (le vecchie già unite fra loro, più la nuova).
void iniziaAnimazioneCostellazione(uint32_t ora) {
  animInizioMs = ora;
  chiudiMenuSubito();                                // se il menu era aperto, l'animazione gli passa davanti
  schermataCorrente = ANIMAZIONE_COSTELLAZIONE;
  animCentroX = LARGHEZZA / 2 + (int)(dragX.valore + aspetto.spostamentoX);
  animCentroY = ALTEZZA / 2 + (int)(dragY.valore + aspetto.spostamentoY);
  Serial.println("costellazione: animazione");
}

// Da 0 a 1 mentre il tempo t va da "da" ad "a" (prima 0, dopo 1), lento all'inizio e alla fine
float frazioneDolce(float t, float da, float a) {
  if (t <= da) return 0.0f;
  if (t >= a) return 1.0f;
  return partenzaArrivoDolci((t - da) / (a - da));
}

// Ogni fotogramma durante l'animazione (al posto del solito aggiornaStato): solo cielo, colore e aspetto,
// niente bisogni, carezza, suoni, voce o scosse. Calcola cosa si vede in questo momento.
void aggiornaAnimazioneCostellazione(uint32_t ora, float dt) {
  float t = (ora - animInizioMs) / 1000.0f;
  if (t >= ANIM_COST_DURATA_S) { fineAnimazioneCostellazione(); return; }
  aggiornaOrientamento(dt);
  aggiornaRespiroCielo(ora);
  aggiornaColoreSalute(ora);
  aggiornaAspettoStella(ora);
  applicaMovimento();                                               // il movimento è fermo: la stella resta dov'era

  // lo schermo: nero -> bianco -> nero
  animBianco = frazioneDolce(t, ANIM_COST_FINE_SCOMPARSA_S, ANIM_COST_FINE_BIANCO_S)
               * (1.0f - frazioneDolce(t, ANIM_COST_FINE_TRATTO_S, ANIM_COST_FINE_NERO_S));
  // i puntini neri: compaiono col bianco (le stelle vecchie già unite fra loro); il puntino nuovo lampeggia,
  // poi resta acceso mentre cresce il suo tratto. Spariscono col nero.
  animPuntiAccesi = t >= ANIM_COST_FINE_BIANCO_S && t < ANIM_COST_FINE_NERO_S;
  animNuovoAcceso = t >= ANIM_COST_FINE_PUNTI_S || ((ora - animInizioMs) / ANIM_COST_LAMPEGGIO_PUNTI_MS) % 2 == 0;
  animTratto = constrain((t - ANIM_COST_FINE_PUNTI_S) / (ANIM_COST_FINE_TRATTO_S - ANIM_COST_FINE_PUNTI_S), 0.0f, 1.0f);
  // il cielo: c'è all'inizio, sparisce col bianco, poi ritorna un punto alla volta
  if (t < ANIM_COST_FINE_SCOMPARSA_S) cieloFrazione = 1.0f;
  else cieloFrazione = frazioneDolce(t, ANIM_COST_FINE_NERO_S, ANIM_COST_FINE_CIELO_S);
  animCielo = animBianco <= 0.0f;

  // le stelle: all'inizio si rimpiccioliscono e scompaiono (con le stelle piccole di prima),
  // alla fine ricompaiono con la stella nuova e la principale lampeggia
  bool ritorno = t >= ANIM_COST_FINE_CIELO_S;
  float scala = ritorno ? 1.0f : 1.0f - frazioneDolce(t, 0.0f, ANIM_COST_FINE_SCOMPARSA_S);
  animStelle = scala > 0.01f;
  costellazioneStelleMostrate = ritorno ? costellazioneStelle : costellazioneStelle - 1;
  costellazioneVisibile = scala;
  aspetto.raggio *= scala;
  bool accesa = !ritorno || ((ora - animInizioMs) / ANIM_COST_LAMPEGGIO_STELLA_MS) % 2 == 0;
  aspetto.luminosita = accesa ? 1.0f : 0.0f;
  applicaCostellazione();
}

// Fine dei 10 secondi: si torna alla schermata normale, con tutto visibile
void fineAnimazioneCostellazione() {
  schermataCorrente = HOME;
  cieloFrazione = 1.0f;
  costellazioneVisibile = 1.0f;
  costellazioneStelleMostrate = costellazioneStelle;
  Serial.println("costellazione: fine animazione");
}
