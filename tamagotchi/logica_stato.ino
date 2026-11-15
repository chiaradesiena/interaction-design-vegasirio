// =====================================================================
//  LOGICA STATO — mette insieme tutto: calcola l'aspetto della stella e, a ogni fotogramma,
//  chiama gli aggiornamenti di tutti i moduli nell'ordine giusto.
// =====================================================================

// La stella "respira" piano; il colore arriva dalla salute (aggiornaColoreSalute).
// Qui, più avanti, ogni stato dell'automa sceglierà colore, punte, ritmo...
void aggiornaAspettoStella(uint32_t ora) {
  float fase = (ora % PULSAZIONE_PERIODO_MS) / (float)PULSAZIONE_PERIODO_MS;
  aspetto.colore      = rgb((uint8_t)coloreStellaRGB[0], (uint8_t)coloreStellaRGB[1], (uint8_t)coloreStellaRGB[2]);
  aspetto.raggio      = STELLA_RAGGIO * (1 + PULSAZIONE_AMPIEZZA * sinf(2 * PI * fase)) * scalaCarezza.valore;
  aspetto.buco = bucoCorrente;
  aspetto.luminosita = luminositaStella;
  aspetto.spostamentoX = tremolioX;
  aspetto.spostamentoY = tremolioY;
  aspetto.numeroPunte = STELLA_NUMERO_PUNTE;
  aspetto.vita        = STELLA_VITA;
  aspetto.affilatura  = STELLA_AFFILATURA;
  aspetto.rotazione   = fmodf(ora / 1000.0f * STELLA_ROTAZIONE_GRADI_S, 360);
}

void aggiornaStato() {
  uint32_t ora = millis();
  float dt = (ora - ultimoAggiornamento) / 1000.0;   // secondi dall'ultimo fotogramma
  ultimoAggiornamento = ora;
  if (dt > 0.1) dt = 0.1;                            // evita salti se un fotogramma tarda
  if (schermataCorrente == ANIMAZIONE_COSTELLAZIONE) { aggiornaAnimazioneCostellazione(ora, dt); return; }   // tutto il resto aspetta
  aggiornaOrientamento(dt);
  aggiornaDragStella(dt);
  aggiornaRespiroCielo(ora);
  aggiornaCarezza(ora, dt);
  aggiornaNutrimento(dt);
  aggiornaSocialita(ora, dt);
  aggiornaEnergia(ora, dt);
  aggiornaSalute();
  aggiornaCostellazione(ora, dt);
  aggiornaLampeggio(ora);
  aggiornaBuco(dt);
  aggiornaColoreSalute(ora);
  aggiornaSuonoNutrimento(ora);
  aggiornaMelodiaPieno(ora);
  aggiornaSuonoCarezza(ora);
  aggiornaSlider();
  aggiornaFesta(ora);
  aggiornaAttivita(dt);
  aggiornaMovimento(ora);
  aggiornaAscolto(dt);
  aggiornaAspettoStella(ora);
  applicaMovimento();
  applicaAscolto();
  aggiornaCerchioCostellazione(ora, dt);
  applicaCostellazione();
  salvaStato(ora);
  suonoInCorso = suonoNotaCorrente >= 0 || pienoNotaCorrente >= 0 || festaNotaCorrente >= 0 || sliderInUso;   // il microfono aspetta finché c'è un suono
}
