// =====================================================================
//  LOGICA MALATA — lo stato MALATA (in decadenza): ci si entra quando la salute scende sotto
//  MALATA_SOGLIA_ENTRATA e se ne esce solo quando torna a MALATA_SOGLIA_USCITA; all'uscita lo schermo
//  passa dal rosso al bianco e poi torna al cielo nero. Legge "bisogni"; scrive malata, le sfumature,
//  coloreFondo, luminositaStella, i colori del cielo e delle stelle, e "malata" nella memoria permanente. Non disegna.
// =====================================================================

// Mescola due colori: k = 0 solo il primo, k = 1 solo il secondo
uint16_t mescolaColori(uint16_t a, uint16_t b, float k) {
  if (k <= 0) return a;
  if (k >= 1) return b;
  float ca[3], cb[3];
  componentiColore(a, ca);
  componentiColore(b, cb);
  return rgb((uint8_t)(ca[0] + (cb[0] - ca[0]) * k), (uint8_t)(ca[1] + (cb[1] - ca[1]) * k), (uint8_t)(ca[2] + (cb[2] - ca[2]) * k));
}

// Numero fra 0 e 1 per il ritmo dei lampi: con lo stesso seme (MALATA_SEME) esce sempre la stessa sequenza
float casualeMalata01() {
  statoCasualeMalata = statoCasualeMalata * 1664525UL + 1013904223UL;
  return (statoCasualeMalata >> 8) / 16777216.0f;
}

// Una durata a caso fra minimo e massimo, in ms
uint32_t durataCasualeMalata(uint32_t minimo, uint32_t massimo) {
  if (massimo <= minimo) return minimo;
  return minimo + (uint32_t)(casualeMalata01() * (massimo - minimo));
}

// Entra ed esce dallo stato. Le due soglie diverse evitano che la stella si ammali e guarisca
// di continuo quando la salute sta proprio intorno a un solo valore.
void aggiornaStatoMalata(uint32_t ora) {
  if (!malata && bisogni.salute < MALATA_SOGLIA_ENTRATA) {
    malata = true;
    malataInUscita = false;                        // se stava ancora guarendo, il passaggio si interrompe
    malataGuarigione = 0;                          // si riparte dal rosso pieno e dal lampeggio affannoso
    malataAccesa = true;
    malataProssimoLampoMs = ora + durataCasualeMalata(MALATA_ACCESO_MIN_MS, MALATA_ACCESO_MAX_MS);
    festaAttiva = false;                           // se c'era la festa dell'energia, finisce subito (anche la sua musica)
    festaNotaCorrente = -1;
    memoria.putBool("malata", true);               // resta malata anche dopo lo spegnimento
    Serial.println("malata: la stella si e' ammalata");
  } else if (malata && bisogni.salute >= MALATA_SOGLIA_USCITA) {
    malata = false;
    malataInUscita = true;
    malataGuaritaMs = ora;
    memoria.putBool("malata", false);
    Serial.println("malata: la stella e' guarita");
  }
}

// Vero finché si vede qualcosa della malattia (anche durante il passaggio rosso -> bianco -> nero):
// intanto festa, cane e nuove stelle della costellazione aspettano
bool malataVisibile() {
  return malata || malataInUscita || malataSfumatura > 0;
}

// All'accensione (dopo caricaStato): se era malata quando si è spenta e la salute non è ancora tornata
// a MALATA_SOGLIA_USCITA, resta malata. Dopo un azzeramento la salute è quella di partenza e decide lei.
void caricaMalata() {
  bool salvata = memoria.isKey("bisogni") && memoria.getBool("malata", false);   // senza bisogni salvati (azzeramento) non conta
  bool adesso = bisogni.salute < MALATA_SOGLIA_ENTRATA || (salvata && bisogni.salute < MALATA_SOGLIA_USCITA);
  if (memoria.getBool("malata", false) != adesso) memoria.putBool("malata", adesso);
  if (!adesso) return;
  malata = true;
  malataAccesa = true;
  malataProssimoLampoMs = millis() + durataCasualeMalata(MALATA_ACCESO_MIN_MS, MALATA_ACCESO_MAX_MS);
  Serial.println("malata: era malata allo spegnimento, resta malata");
}

// Quanto è vicina a guarire (0-1): segue la salute piano, così lampeggio e rosso cambiano senza scatti
void aggiornaGuarigioneMalata(float dt) {
  float obiettivo = (bisogni.salute - MALATA_SOGLIA_ENTRATA) / (MALATA_SOGLIA_USCITA - MALATA_SOGLIA_ENTRATA);
  obiettivo = constrain(obiettivo, 0.0f, 1.0f);
  float k = min(MALATA_GUARIGIONE_VELOCITA * dt, 1.0f);
  malataGuarigione += (obiettivo - malataGuarigione) * k;
}

// Un valore che va da 0 a 1 fra due istanti (in ms dal momento della guarigione)
float avanzamentoMalata(uint32_t trascorsi, uint32_t inizio, uint32_t durata) {
  if (trascorsi <= inizio) return 0.0f;
  if (durata == 0 || trascorsi >= inizio + durata) return 1.0f;
  return (trascorsi - inizio) / (float)durata;
}

// Il passaggio della guarigione: rosso -> bianco, bianco fermo, bianco -> cielo nero (intanto la stella riprende colore)
void aggiornaUscitaMalata(uint32_t ora) {
  uint32_t trascorsi = ora - malataGuaritaMs;
  uint32_t inizioNero = MALATA_USCITA_BIANCO_MS + MALATA_USCITA_TENUTA_MS;
  float versoBianco = avanzamentoMalata(trascorsi, 0, MALATA_USCITA_BIANCO_MS);
  float versoNero   = avanzamentoMalata(trascorsi, inizioNero, MALATA_USCITA_NERO_MS);
  malataBianco = versoBianco * (1.0f - versoNero);
  if (versoBianco >= 1.0f) malataRosso = 0;        // sotto il bianco il rosso se ne va: dopo si torna al nero
  malataSfumatura = 1.0f - versoNero;
  if (versoNero >= 1.0f) malataInUscita = false;   // passaggio finito: aspetto normale
}

// Sfumature: si ammala -> rosso e stella nera salgono piano; guarisce -> il passaggio qui sopra.
// Negli altri casi tutto torna piano verso l'aspetto normale.
void aggiornaSfumatureMalata(uint32_t ora, float dt) {
  float passo = MALATA_ENTRATA_DURATA_MS > 0 ? dt * 1000.0f / MALATA_ENTRATA_DURATA_MS : 1.0f;
  if (malataInUscita) aggiornaUscitaMalata(ora);
  else if (malata) {
    malataSfumatura = min(malataSfumatura + passo, 1.0f);
    malataRosso     = min(malataRosso + passo, 1.0f);
    malataBianco    = max(malataBianco - passo, 0.0f);
  } else {
    malataSfumatura = max(malataSfumatura - passo, 0.0f);
    malataRosso     = max(malataRosso - passo, 0.0f);
    malataBianco    = max(malataBianco - passo, 0.0f);
  }
  uint16_t rosso = mescolaColori(MALATA_COLORE_FONDO, MALATA_COLORE_FONDO_GUARENDO, malataGuarigione);   // più guarisce, più il rosso si attenua
  coloreFondo = mescolaColori(mescolaColori(NERO, rosso, malataRosso), BIANCO, malataBianco);
}

// Lampeggio affannoso: accesa e spenta per tempi a caso, così il ritmo è irregolare.
// Più guarisce, più i tempi si avvicinano a quelli calmi (MALATA_CALMO_*): il lampeggio rallenta e diventa regolare.
// Da malata prende il posto del lampeggio della solitudine.
void aggiornaLampeggioMalata(uint32_t ora) {
  if (!malata) return;
  if ((int32_t)(ora - malataProssimoLampoMs) >= 0) {
    malataAccesa = !malataAccesa;
    uint32_t affannoso = malataAccesa ? durataCasualeMalata(MALATA_ACCESO_MIN_MS, MALATA_ACCESO_MAX_MS)
                                      : durataCasualeMalata(MALATA_SPENTO_MIN_MS, MALATA_SPENTO_MAX_MS);
    uint32_t calmo = malataAccesa ? MALATA_CALMO_ACCESO_MS : MALATA_CALMO_SPENTO_MS;
    malataProssimoLampoMs = ora + (uint32_t)(affannoso + ((float)calmo - affannoso) * malataGuarigione);
  }
  luminositaStella = malataAccesa ? 1.0f : 0.0f;
}

// Il cielo diventa rosso scuro (ogni puntino tiene la sua luce: più luminoso = rosso più acceso)
// e, durante la guarigione, sparisce nel bianco
void tingiCieloMalata() {
  if (malataRosso <= 0 && malataBianco <= 0) return;
  uint16_t rossoCielo = mescolaColori(MALATA_COLORE_CIELO, MALATA_COLORE_CIELO_GUARENDO, malataGuarigione);
  for (int i = 0; i < PUNTI_SULLA_SFERA; i++) {
    PuntoCielo& p = cielo[i];
    float luce = ((p.colore >> 11) & 0x1F) / 31.0f;        // i puntini sono grigi: basta un canale per sapere quanto sono luminosi
    float luceScia = ((p.coloreScia >> 11) & 0x1F) / 31.0f;
    p.colore     = mescolaColori(mescolaColori(p.colore, scurisciColore(rossoCielo, luce), malataRosso), BIANCO, malataBianco);
    p.coloreScia = mescolaColori(mescolaColori(p.coloreScia, scurisciColore(rossoCielo, luceScia), malataRosso), BIANCO, malataBianco);
  }
}

// Ogni fotogramma (dopo il lampeggio della solitudine e il respiro del cielo)
void aggiornaMalata(uint32_t ora, float dt) {
  aggiornaStatoMalata(ora);
  aggiornaGuarigioneMalata(dt);
  aggiornaSfumatureMalata(ora, dt);
  aggiornaLampeggioMalata(ora);
  tingiCieloMalata();
}

// Dopo che tutti gli altri moduli hanno deciso l'aspetto: la stella sfuma verso il nero,
// e con lei le stelle piccole della costellazione
void applicaMalata() {
  if (malataSfumatura <= 0) return;
  aspetto.colore = mescolaColori(aspetto.colore, MALATA_COLORE_STELLA, malataSfumatura);
  aspettoPiccola.colore = mescolaColori(aspettoPiccola.colore, MALATA_COLORE_STELLA, malataSfumatura);
}
