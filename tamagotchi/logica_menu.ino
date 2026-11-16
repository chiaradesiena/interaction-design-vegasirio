// =====================================================================
//  LOGICA MENU — la schermata delle impostazioni: entra da destra con uno swipe verso sinistra e
//  si chiude con uno swipe verso destra, con un movimento elastico (molla menuApertura).
//  Dentro: volume a 10 livelli e numero di stelle della costellazione, con i tasti + e -. Legge "sensi" (swipe e tocchi);
//  scrive menuAperto, menuApertura, volumeLivello, menuTastoAcceso e schermataCorrente.
// =====================================================================

// Uno swipe vale se il dito ha fatto abbastanza strada e soprattutto in orizzontale.
// verso = 1 per destra, -1 per sinistra.
bool swipeVerso(int verso) {
  if (!sensi.swipe) return false;
  int dx = sensi.swipeDX * verso;
  return dx >= MENU_SWIPE_MIN_PX && dx >= abs(sensi.swipeDY) * MENU_SWIPE_ORIZZONTALE;
}

// Il dito ha toccato vicino al tasto con centro (cx, cy)?
bool dentroTasto(int x, int y, int cx, int cy) {
  int dx = x - cx, dy = y - cy;
  return dx * dx + dy * dy <= MENU_TASTO_AREA_PX * MENU_TASTO_AREA_PX;
}

// Volume: cambia livello (da 1 = muto a VOLUME_NUMERO_LIVELLI = massimo) e, se non è muto, suona una nota di prova
void cambiaVolume(int passo) {
  int nuovo = constrain(volumeLivello + passo, 1, VOLUME_NUMERO_LIVELLI);
  if (nuovo == volumeLivello) return;              // già al minimo o al massimo
  volumeLivello = nuovo;
  volumeCorrente = VOLUME_LIVELLI[volumeLivello - 1];
  impostaVolume(volumeCorrente);
  if (volumeCorrente > 0) suona(VOLUME_NOTA_PROVA_HZ, VOLUME_NOTA_PROVA_MS);
  Serial.printf("volume: livello %d (%d su 255)\n", volumeLivello, volumeCorrente);
}

// Un tocco (dito appoggiato e staccato senza scorrere) su un tasto del menu
void aggiornaTastiMenu(uint32_t ora) {
  if (!menuAperto || batteriaAperta || !sensi.cliccato) return;   // con la pagina della batteria davanti i tasti del menu non si toccano
  TastoMenu tasto = TASTO_NESSUNO;
  if (dentroTasto(sensi.toccoX, sensi.toccoY, MENU_TASTO_MENO_X, MENU_VOLUME_Y)) tasto = TASTO_VOLUME_MENO;
  else if (dentroTasto(sensi.toccoX, sensi.toccoY, MENU_TASTO_PIU_X, MENU_VOLUME_Y)) tasto = TASTO_VOLUME_PIU;
  else if (dentroTasto(sensi.toccoX, sensi.toccoY, MENU_TASTO_MENO_X, MENU_STELLE_Y)) tasto = TASTO_STELLE_MENO;
  else if (dentroTasto(sensi.toccoX, sensi.toccoY, MENU_TASTO_PIU_X, MENU_STELLE_Y)) tasto = TASTO_STELLE_PIU;
  if (tasto == TASTO_NESSUNO) return;
  if (tasto == TASTO_VOLUME_MENO) cambiaVolume(-1);
  if (tasto == TASTO_VOLUME_PIU)  cambiaVolume(+1);
  if (tasto == TASTO_STELLE_MENO) impostaStelleCostellazione(costellazioneStelle - 1);   // logica_costellazione.ino
  if (tasto == TASTO_STELLE_PIU)  impostaStelleCostellazione(costellazioneStelle + 1);
  menuTastoAcceso = tasto;                          // il tasto si illumina per un attimo
  menuTastoAccesoMs = ora;
}

// Ogni fotogramma: guarda gli swipe e i tasti, muove la molla e decide quale schermata si vede.
// La stella intanto continua a vivere (bisogni, cielo, suoni): il menu non la mette in pausa.
void aggiornaMenu(uint32_t ora, float dt) {
  if (!menuAperto && swipeVerso(-1) && tempoContinuoS <= MENU_IGNORA_DOPO_CAREZZA_S) {   // uno swipe alla fine di una carezza non apre il menu
    menuAperto = true;
    schermataCorrente = MENU;
    Serial.println("menu: aperto");
  } else if (menuAperto && !batteriaAperta && swipeVerso(-1)) {   // un altro swipe a sinistra: pagina della batteria
    batteriaAperta = true;
    Serial.println("batteria: aperta");
  } else if (batteriaAperta && swipeVerso(1)) {                   // uno swipe a destra la chiude e si torna al menu
    batteriaAperta = false;
    Serial.println("batteria: chiusa");
  } else if (menuAperto && swipeVerso(1)) {
    menuAperto = false;
    Serial.println("menu: chiuso");
  }
  aggiornaTastiMenu(ora);
  if (menuTastoAcceso != TASTO_NESSUNO && ora - menuTastoAccesoMs >= MENU_LAMPO_TASTO_MS) menuTastoAcceso = TASTO_NESSUNO;
  aggiornaMolla(menuApertura, menuAperto ? 1.0f : 0.0f, MENU_FREQUENZA, MENU_SMORZAMENTO, dt);
  aggiornaMolla(batteriaApertura, batteriaAperta ? 1.0f : 0.0f, MENU_FREQUENZA, MENU_SMORZAMENTO, dt);
  if (!batteriaAperta && batteriaApertura.valore < 0.01f && fabsf(batteriaApertura.velocita) < 0.1f) batteriaApertura = {0, 0};   // finita di uscire
  if (!menuAperto && menuApertura.valore < 0.01f && fabsf(menuApertura.velocita) < 0.1f) {   // finito di uscire: si torna alla stella
    menuApertura = {0, 0};
    schermataCorrente = HOME;
  }
  if (schermataCorrente == MENU) { sensi.toccato = false; sensi.premuto = false; }   // con il menu davanti i tocchi non sono carezze
}

// Se mentre il menu è aperto parte l'animazione della costellazione, il menu si chiude di colpo
void chiudiMenuSubito() {
  menuAperto = false;
  menuApertura = {0, 0};
  batteriaAperta = false;
  batteriaApertura = {0, 0};
}
