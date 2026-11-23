// =====================================================================
//  DATI — tutte le variabili globali (lo stato della stella fra un fotogramma e l'altro).
//  Stanno qui, e non sparse nei moduli, perché Arduino IDE unisce i .ino in ordine
//  alfabetico: così ogni modulo le trova già dichiarate.
// =====================================================================
#pragma once

Schermata schermataCorrente = HOME;

// Canvas (o "sprite"): immagine in memoria su cui si disegna tutto,
// poi la si copia sullo schermo in un colpo solo per evitare sfarfallio
M5Canvas tela(&CoreS3.Display);

PuntoCielo cielo[PUNTI_SULLA_SFERA];
Sensi sensi;

// Orientamento: tabella 3x3 (matrice) che dice come è girato il dispositivo
// rispetto al cielo. All'accensione = nessuna rotazione.
float orientamento[3][3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
float orientamentoPrecedente[3][3];   // quello del fotogramma prima, per il motion blur

// Velocità di rotazione "elastica" del cielo, una molla per asse
Molla elasticoX = {0, 0}, elasticoY = {0, 0}, elasticoZ = {0, 0};

// Spostamento della stella dal centro (drag), in px
Molla dragX = {0, 0}, dragY = {0, 0};

AspettoStella aspetto;
StileStella stileStella = STILE_PUNTINI;          // come si disegna la stella (logica_stile.ino); all'accensione si riprende dalla memoria

// Bisogni (per ora scendono nutrimento, socialità ed energia) e colore della stella legato alla salute
Bisogni bisogni = { NUTRIMENTO_INIZIALE, SOCIALITA_INIZIALE, ENERGIA_INIZIALE, BISOGNO_MAX, BISOGNO_MAX };
int livelloCorrente = 0;                         // 0 = bianco ... 4 = rosso
float coloreStellaRGB[3] = {255, 255, 255};      // colore di questo fotogramma (rosso, verde, blu)
float coloreDa[3]        = {255, 255, 255};      // colore da cui è partita l'ultima sfumatura
uint32_t cambioColoreMs = 0;                     // quando è iniziata l'ultima sfumatura

// Carezza
bool carezzando = false;                         // in questo fotogramma il dito sta accarezzando la stella
float tempoContinuoS = 0;                        // secondi di carezza continua (riparte da zero dopo una pausa)
float tempoCrescitaS = 0;                        // secondi di carezza che guidano la grandezza della stella
Molla scalaCarezza = {1, 0};                     // grandezza dovuta alla carezza (1 = normale)
float carezzaVelX = 0, carezzaVelY = 0;          // velocità del dito in px al secondo
int carezzaPrecX = 0, carezzaPrecY = 0;          // posizione del dito nel fotogramma prima
bool carezzaPrecPremuto = false;
uint32_t ultimaCarezzaMs = 0;                    // quando c'è stata l'ultima carezza
float tremolioIntensita = 0;                     // 0 = la stella è ferma, 1 = trema al massimo
float tremolioX = 0, tremolioY = 0;              // spostamento di questo fotogramma, in px
float bucoCorrente = 0;                          // buco di questo fotogramma (frazione del raggio)

// Socialità e lampeggio
float luminositaStella = 1;                      // 1 = accesa, 0 = spenta (cambia a gradino quando lampeggia)
bool suonoInCorso = false;                       // c'è un suono in corso o si sta usando lo slider: il microfono aspetta
float ascolto = 0;                               // 0 = stella normale, 1 = feedback di ascolto al massimo (sale e scende piano)

// Memoria dei bisogni
Preferences memoria;                             // la memoria permanente
RTC_NOINIT_ATTR uint32_t firmaRiavvio;           // sopravvive a un reset ma non a uno spegnimento: serve a riconoscere il reset
Bisogni bisogniSalvati;                          // i valori scritti l'ultima volta in memoria
int livelloSalvato = 0;                          // livello di salute (colore) dell'ultimo salvataggio
uint32_t ultimoSalvataggioMs = 0;                // quando è stato fatto l'ultimo salvataggio

// Energia, movimento e festa
uint32_t ultimaScossaMs = 0;                     // quando il dispositivo è stato scosso l'ultima volta
bool scossaRecente = false;                      // si sta scuotendo (o si è appena smesso)
float tempoScossaS = 0;                          // secondi di scuotimento continuo (ripartono da zero dopo una pausa)
bool staticaEnergia = false;                     // energia bassa: la stella sta ferma al centro
float attivita = 1;                              // 1 = la stella si muove, 0 = ferma (cambia piano)
float orbitaX = 0, orbitaY = 0;                  // spostamento dovuto all'orbita, in px
float movimentoX = 0, movimentoY = 0;            // spostamento totale dal centro (orbita + festa), in px
float energiaPrecedente = ENERGIA_INIZIALE;      // energia del fotogramma prima
bool festaAttiva = false;                        // è in corso la festa dell'energia al 100%
bool festaPronta = true;                         // la festa può partire quando l'energia arriva al 100%
uint32_t festaInizioMs = 0;                      // quando è partita la festa
float festaX = 0, festaY = 0;                    // spostamento dovuto al percorso della festa, in px
int festaNotaCorrente = -1;                      // -1 = suono fermo, altrimenti numero della prossima nota
uint32_t festaProssimaNotaMs = 0;                // a che ora deve partire la prossima nota
float profondita = 0;                            // variazione di grandezza dovuta alla profondità (0 = nessuna)

// Costellazione
int costellazioneStelle = 1;                     // quante stelle ha la costellazione (1 = solo la stella principale)
uint32_t costellazioneUltimaS = 0;               // ora dell'orologio quando si è aggiunta l'ultima stella (0 = mai)
uint32_t costellazioneUltimaMs = 0;              // stessa cosa in millis(), se l'orologio non risponde
bool costellazioneInQuestaAccensione = false;    // si è aggiunta una stella da quando la board è accesa
float costellazioneTenutaS = 0;                  // secondi di fila con la salute al 100%
float costellazioneAngolo = 0;                   // radianti, a che punto del giro è il cerchio delle stelle piccole
float costellazioneVisibile = 1;                 // 1 = stelle piccole visibili, 0 = nascoste (durante la festa)
AspettoStella aspettoPiccola;                    // aspetto delle stelle piccole (lo calcola la logica, lo usa il disegno)
int costellazioneStelleMostrate = 1;             // quante stelle si disegnano (durante l'animazione la nuova arriva solo alla fine)
float costellazioneZoom = -1;                  // grandezza della scena della costellazione, Sirio compreso (1 = normale; -1 = non ancora calcolata)
float animZoomInizio = 1;                        // grandezza della scena quando è partita l'animazione della stella nuova
// Animazione della nuova stella
uint32_t animInizioMs = 0;                       // quando è partita
float animBianco = 0;                            // 0 = schermo nero, 1 = bianco
bool animPuntiAccesi = false;                    // si vedono i puntini neri (le stelle della costellazione)
bool animNuovoAcceso = true;                     // il puntino della stella nuova è acceso (lampeggia)
float animTratto = 0;                            // quanto del tratto fra i puntini è disegnato (0-1)
bool animCielo = true;                           // si disegna il cielo
bool animStelle = true;                          // si disegnano le stelle
int animCentroX = 0, animCentroY = 0;            // dove sta la stella principale durante l'animazione, in px
float cieloFrazione = 1;                         // quanti punti del cielo si disegnano (1 = tutti; serve quando il cielo ritorna)
// Stato proprio e animazione del cane (Sirio)
bool statoProprio = false;                       // salute alta: si è nello stato proprio
uint32_t prossimoCaneMs = 0;                     // a che ora può partire il prossimo cane
bool caneAttivo = false;                         // è in corso l'animazione del cane
bool caneInRitorno = false;                      // è stata interrotta: sta facendo la giravolta per tornare stella
uint32_t caneInizioMs = 0;                       // quando è partito il cane
uint32_t caneRitornoMs = 0;                      // quando è iniziata la giravolta di ritorno
float caneForma = 0;                             // 0 = stella, 1 = cane
float caneGiro = 0;                              // 0 = non gira, 1 = gira al massimo
float caneFormaRitorno = 0, caneGiroRitorno = 0; // forma e giro nel momento dell'interruzione
float caneRotazione = 0;                         // gradi di giro aggiunti alla stella
float caneAngolo = 0, caneAngoloRitorno = 0;     // giro della sagoma del cane (0 = dritta) e quello al momento dell'interruzione
float caneX = 0, caneY = 0;                      // spostamento del cane dal centro, in px
float caneProfondita = 1;                        // grandezza dovuta alla distanza (meno di 1 = lontano, più di 1 = vicino)
float caneVerso = 1;                             // 1 = guarda a destra, -1 = a sinistra
float caneZampe = 0;                             // gradi di piega delle zampe
float caneInclinazione = 0;                      // gradi di inclinazione quando inciampa
float caneXRitorno = 0, caneYRitorno = 0, caneProfonditaRitorno = 1;   // dove era al momento dell'interruzione
// Calcoli del disegno del cane fatti una volta per fotogramma invece che per ogni puntino (più veloce)
float caneXPezzo[SIRIO_CANE_NUMERO_PEZZI], caneYPezzo[SIRIO_CANE_NUMERO_PEZZI];
float caneCosPezzo[SIRIO_CANE_NUMERO_PEZZI], caneSinPezzo[SIRIO_CANE_NUMERO_PEZZI];
float caneCosGiro = 1, caneSinGiro = 0, caneScala = 1, caneVersoDisegno = 1;
float caneRaggioSagoma = 0;                      // px, cerchio che contiene tutto il cane (fuori non si fanno i calcoli)
float caneUPasso = -1;                           // a che punto della passeggiata (0-1); -1 = non sta passeggiando
// Abbaio
int16_t abbaioCampioni[SIRIO_ABBAIO_MAX_CAMPIONI];   // il suono del "bau", creato una volta sola
size_t abbaioNumeroCampioni = 0;                 // quanti campioni del suono si usano (0 = non ancora creato)
int abbaioProssimo = 0;                          // quale abbaio della passeggiata tocca adesso
int abbaioUfRimasti = 0;                         // quanti "bau" mancano in questo abbaio
uint32_t abbaioProssimoUfMs = 0;                 // a che ora suona il prossimo "bau"

// Sensori
uint32_t ultimaLetturaOrologioMs = 0;            // quando si è letto l'orologio l'ultima volta
bool orologioAvvisato = false;                   // già scritto sul Monitor seriale che l'orologio non c'è
// Il giroscopio da fermo non segna mai zero esatto: questo errore (offset)
// si misura all'accensione e poi si sottrae
float offsetX = 0, offsetY = 0, offsetZ = 0;

int16_t bufferMic[MIC_CAMPIONI];         // ultima misura del microfono
bool micAttivo = false;                  // vero = il canale audio è in mano al microfono
bool micHaDati = false;                  // una misura è stata richiesta e va letta
uint32_t fineSuonoMs = 0;                // a che ora finisce l'ultimo suono
float volumeVoceLetto = 0;               // ultimo volume misurato (0-1)

// Socialità
uint32_t ultimaVoceMs = 0;               // quando il microfono ha sentito l'ultima volta un suono sopra la soglia
bool voceAttiva = false;                 // qualcuno sta parlando (o ha appena smesso)

// Slider del volume
int volumeCorrente = VOLUME_LIVELLI[VOLUME_LIVELLO_INIZIALE - 1];   // volume dell'altoparlante adesso (0-255)
bool sliderInUso = false;                 // il dito sta trascinando lo slider

// Menu delle impostazioni
bool menuAperto = false;                  // vero = il menu deve stare sullo schermo (dopo lo swipe verso destra)
Molla menuApertura = {0, 0};              // 0 = menu fuori dallo schermo, 1 = menu al suo posto (elastico)
int volumeLivello = VOLUME_LIVELLO_INIZIALE;   // livello del volume scelto nel menu (1 = muto ... VOLUME_NUMERO_LIVELLI = massimo)
TastoMenu menuTastoAcceso = TASTO_NESSUNO;     // il tasto appena toccato, che si illumina per un attimo
uint32_t menuTastoAccesoMs = 0;                // quando è stato toccato
// Pagina della batteria (dopo il menu)
bool batteriaAperta = false;                   // vero = la pagina della batteria deve stare sullo schermo (sopra il menu)
Molla batteriaApertura = {0, 0};               // 0 = pagina fuori dallo schermo, 1 = pagina al suo posto (elastico)
uint32_t ultimaLetturaBatteriaMs = 0;          // quando si è letta la batteria l'ultima volta

// Suoni
int suonoNotaCorrente = -1;               // -1 = fermo, altrimenti numero della prossima nota
uint32_t suonoProssimaNotaMs = 0;         // a che ora deve partire la prossima nota
uint32_t prossimoRichiamoMs = 0;          // a che ora deve ripartire il richiamo
bool nutrimentoSottoSoglia = false;       // il nutrimento è alla soglia o sotto?
int pienoNotaCorrente = -1;               // -1 = ferma, altrimenti numero della prossima nota
uint32_t pienoProssimaNotaMs = 0;         // a che ora deve partire la prossima nota
float nutrimentoPrecedente = NUTRIMENTO_INIZIALE;   // nutrimento del fotogramma prima
bool pienoPronto = true;                  // può suonare quando il nutrimento arriva al 100%
bool battitoAttivo = false;               // la carezza è in corso e il battito sta andando
uint32_t prossimoBattitoMs = 0;           // a che ora suona il prossimo battito

// Radio di Unisono
bool radioOk = false;                     // la radio ESP-NOW è partita
StellaVicina stelleVicine[UNISONO_MAX_STELLE];   // le stelle sentite dalla radio
int numeroStelleVicine = 0;               // quante ce ne sono nell'elenco
// Lucchetto: i messaggi arrivano "di nascosto" mentre gira il loop; il lucchetto evita
// che il loop legga l'elenco proprio mentre la radio lo sta scrivendo
portMUX_TYPE lucchettoVicine = portMUX_INITIALIZER_UNLOCKED;
uint32_t messaggiInviati = 0;             // quante richieste ha mandato la stella
uint32_t ultimoInvioMs = 0;               // quando ha mandato l'ultima richiesta
bool primoInvioFatto = false;             // la prima richiesta parte subito dopo l'accensione
uint8_t mioMac[6] = { 0, 0, 0, 0, 0, 0 }; // indirizzo della propria radio

// Orchestra di Unisono
int orchestraUltimoPasso = -1;            // l'ultimo passo del ritmo già suonato (-1 = nessuno)
int orchestraNotaMia = 0;                 // quale delle proprie note suona la prossima volta
float luceMia = 0;                        // 1 = la propria stella ha appena suonato, 0 = normale
int32_t differenzaDirettoreMs = 0;        // quanto l'orologio del direttore è avanti rispetto al nostro (0 = siamo noi il direttore)
bool sonoDirettore = true;                // vero = è questa stella a dare il tempo

// Brano di Unisono
int branoProssimaMia = 0;                 // la prossima nota della propria parte da suonare
int branoProssimaAltra = 0;               // la prossima nota dell'altra parte (non si suona: serve a illuminare l'altra stella)
uint32_t branoFineVoceMs[BRANO_VOCI] = { 0 };   // quando finisce la nota di ogni voce (per scegliere una voce libera)

// Unisono
FaseUnisono unisonoFase = UNISONO_NO;     // a che punto è Unisono
uint32_t unisonoInizioMs = 0;             // quando è iniziata l'attesa
uint32_t unisonoAnimazioneMs = 0;         // quando è iniziata l'animazione della galassia
uint32_t unisonoFineMs = 0;               // quando è finito l'ultimo Unisono (0 = mai, niente pausa)
bool unisonoInvitata = false;             // per il futuro stato ESPLOSIONE: vero = la stella può entrare in Unisono anche senza salute
StellaGalassia galassia[UNISONO_MAX_STELLE];   // le altre stelle della galassia
int numeroGalassia = 0;                   // quante ce ne sono
float unisonoAngolo = 0;                  // radianti, a che punto del giro è la galassia
float luceAngolo = 0;                     // radianti, dove punta il braccio di luce
float bracciVisibili = 0;                 // 0 = bracci spenti, 1 = accesi (si accendono durante l'attesa)
float luceVisibile = 0;                   // 0 = braccio di luce spento, 1 = acceso (solo durante la galassia)
// Quello che si disegna (lo calcola la logica, lo usa il disegno)
AspettoStella aspettoUnisono;             // la stella di questo dispositivo, al centro
int unisonoCentroX = 0, unisonoCentroY = 0;   // dove sta, in px
AspettoStella aspettiGalassia[UNISONO_MAX_STELLE];   // le altre stelle
int galassiaX[UNISONO_MAX_STELLE], galassiaY[UNISONO_MAX_STELLE];   // dove stanno, in px

// Stato malata (logica_malata.ino)
bool malata = false;                      // la stella è malata (salute scesa sotto MALATA_SOGLIA_ENTRATA, non ancora tornata a MALATA_SOGLIA_USCITA)
float malataSfumatura = 0;                // 0 = aspetto normale, 1 = stella nera (sale quando si ammala, torna a 0 solo alla fine della guarigione)
float malataRosso = 0;                    // 0 = niente rosso, 1 = fondo e cielo rossi
float malataBianco = 0;                   // 0 = niente bianco, 1 = schermo tutto bianco (durante la guarigione)
float malataGuarigione = 0;               // 0 = appena ammalata (salute sotto MALATA_SOGLIA_ENTRATA), 1 = quasi guarita (salute vicina a MALATA_SOGLIA_USCITA)
bool malataInUscita = false;              // è in corso il passaggio rosso -> bianco -> cielo nero
uint32_t malataGuaritaMs = 0;             // quando è guarita (inizio del passaggio)
uint16_t coloreFondo = NERO;              // colore dello sfondo di questo fotogramma (nero, rosso se è malata, bianco mentre guarisce)
bool malataAccesa = true;                 // nel lampeggio affannoso: la stella è accesa
uint32_t malataProssimoLampoMs = 0;       // a che ora il lampeggio cambia (da acceso a spento o viceversa)
uint32_t statoCasualeMalata = MALATA_SEME;   // numeri "casuali" ripetibili per il ritmo dei lampi

// Tempo e disegno
uint32_t ultimoAggiornamento = 0;
// Numeri "casuali" ripetibili: con lo stesso seme esce sempre lo stesso cielo
uint32_t statoCasuale = SEME_CIELO;
int puntiVisibili = 0;   // quanti punti sono sullo schermo, per il Monitor seriale
