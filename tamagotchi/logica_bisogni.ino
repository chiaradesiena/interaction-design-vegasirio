// =====================================================================
//  LOGICA BISOGNI — nutrimento, socialità, energia, salute e colore della stella,
//  buco della fame e lampeggio della solitudine. Legge "sensi" e la carezza, scrive "bisogni".
// =====================================================================

// Nutrimento: scende piano col tempo (NUTRIMENTO_DECADIMENTO punti ogni NUTRIMENTO_INTERVALLO_S secondi)
void aggiornaNutrimento(float dt) {
  bisogni.nutrimento -= NUTRIMENTO_DECADIMENTO / NUTRIMENTO_INTERVALLO_S * MOLTIPLICATORE_TEMPO * dt;
  if (carezzando && tempoContinuoS >= CAREZZA_DURATA_MIN_S)           // accarezzata da abbastanza: sale
    bisogni.nutrimento += NUTRIMENTO_AUMENTO_PUNTI_S * dt;
  if (bisogni.nutrimento < 0) bisogni.nutrimento = 0;
  if (bisogni.nutrimento > BISOGNO_MAX) bisogni.nutrimento = BISOGNO_MAX;
}

// Socialità: scende col tempo (SOCIALITA_DECADIMENTO punti ogni SOCIALITA_INTERVALLO_S secondi)
// e risale mentre il microfono sente una voce sopra la soglia.
// La voce conta ancora per un attimo dopo l'ultimo momento forte, così le pause fra le parole non la interrompono.

void aggiornaSocialita(uint32_t ora, float dt) {
  bisogni.socialita -= SOCIALITA_DECADIMENTO / SOCIALITA_INTERVALLO_S * MOLTIPLICATORE_TEMPO * dt;
  if (sensi.volumeVoce >= SOCIALITA_VOCE_SOGLIA) { voceAttiva = true; ultimaVoceMs = ora; }
  else if (ora - ultimaVoceMs > SOCIALITA_TENUTA_VOCE_MS) voceAttiva = false;
  if (voceAttiva) bisogni.socialita += SOCIALITA_AUMENTO_PUNTI_S * dt;
  if (bisogni.socialita < 0) bisogni.socialita = 0;
  if (bisogni.socialita > BISOGNO_MAX) bisogni.socialita = BISOGNO_MAX;
}

// Lampeggio: sotto la soglia di socialità la luminosità passa a gradino da 1 a 0 a ritmo costante.
// Agisce solo sulla luce, mentre la fame agisce solo sul buco: i due segnali non si disturbano.
void aggiornaLampeggio(uint32_t ora) {
  bool solitaria = bisogni.socialita < SOCIALITA_SOGLIA_LAMPEGGIO;
  bool acceso = (ora % LAMPEGGIO_PERIODO_MS) < LAMPEGGIO_PERIODO_MS / 2;
  luminositaStella = (!solitaria || acceso || voceAttiva || festaAttiva || caneAttivo) ? 1.0f : 0.0f;   // mentre si ascolta, c'è la festa o il cane la stella resta accesa
}

// Energia: scende col tempo e, dopo ENERGIA_SCOSSA_DURATA_MIN_S secondi di scuotimento continuo, sale finché si scuote.
// Le scosse sono a colpi: una pausa breve (ENERGIA_SCOSSA_PAUSA_MS) non interrompe lo scuotimento.
void aggiornaEnergia(uint32_t ora, float dt) {
  bisogni.energia -= ENERGIA_DECADIMENTO / ENERGIA_INTERVALLO_S * MOLTIPLICATORE_TEMPO * dt;
  if (sensi.scossa >= ENERGIA_SCOSSA_SOGLIA_G) { scossaRecente = true; ultimaScossaMs = ora; }
  else if (ora - ultimaScossaMs > ENERGIA_SCOSSA_PAUSA_MS) scossaRecente = false;
  if (scossaRecente) tempoScossaS += dt; else tempoScossaS = 0;
  if (scossaRecente && tempoScossaS >= ENERGIA_SCOSSA_DURATA_MIN_S) bisogni.energia += ENERGIA_AUMENTO_PUNTI_S * dt;
  if (bisogni.energia < 0) bisogni.energia = 0;
  if (bisogni.energia > BISOGNO_MAX) bisogni.energia = BISOGNO_MAX;
}

// Salute = media dei bisogni già attivi: per ora nutrimento, socialità ed energia.
// Quando si aggiungeranno altri bisogni: sommarli qui e dividere per quanti sono.
void aggiornaSalute() {
  bisogni.salute = (bisogni.nutrimento + bisogni.socialita + bisogni.energia) / 3.0f;
}

// Da 0 a 4: 0 = salute 100-80, 1 = 79-60, 2 = 59-40, 3 = 39-20, 4 = 19-0
int calcolaLivelloSalute(float salute) {
  for (int i = 0; i < 4; i++)
    if (salute >= SALUTE_SOGLIE[i]) return i;
  return 4;
}

// Scompone un colore RGB565 nei suoi tre canali (0-255)
void componentiColore(uint16_t c, float out[3]) {
  out[0] = ((c >> 11) & 0x1F) * 255.0f / 31.0f;
  out[1] = ((c >> 5)  & 0x3F) * 255.0f / 63.0f;
  out[2] = (c & 0x1F) * 255.0f / 31.0f;
}

// Il colore della stella segue il livello di salute, con una sfumatura al cambio di livello
void aggiornaColoreSalute(uint32_t ora) {
  int livello = calcolaLivelloSalute(bisogni.salute);
  if (livello != livelloCorrente) {
    for (int i = 0; i < 3; i++) coloreDa[i] = coloreStellaRGB[i];   // si parte dal colore di adesso
    livelloCorrente = livello;
    cambioColoreMs = ora;
  }
  float k = 1.0f;                                                  // quanto della sfumatura è fatto (0-1)
  if (SALUTE_DURATA_COLORE_MS > 0) k = (ora - cambioColoreMs) / SALUTE_DURATA_COLORE_MS;
  if (k > 1.0f) k = 1.0f;
  float arrivo[3];
  componentiColore(COLORI_SALUTE[livello], arrivo);
  for (int i = 0; i < 3; i++) coloreStellaRGB[i] = coloreDa[i] + (arrivo[i] - coloreDa[i]) * k;
}

// Buco: il nutrimento sceglie un livello (0-5) e il buco si apre o si chiude piano verso la sua grandezza.
void aggiornaBuco(float dt) {
  int livello = 5;
  for (int i = 0; i < 5; i++)
    if (bisogni.nutrimento >= BUCO_SOGLIE[i]) { livello = i; break; }
  float k = BUCO_VELOCITA * dt;
  if (k > 1.0f) k = 1.0f;
  bucoCorrente += (BUCO_RAGGIO[livello] - bucoCorrente) * k;
}
