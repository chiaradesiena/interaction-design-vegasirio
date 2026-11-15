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
int volumeCorrente = VOLUME_SUONO;        // 0-255
bool sliderInUso = false;                 // il dito sta trascinando lo slider

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

// Tempo e disegno
uint32_t ultimoAggiornamento = 0;
// Numeri "casuali" ripetibili: con lo stesso seme esce sempre lo stesso cielo
uint32_t statoCasuale = SEME_CIELO;
int puntiVisibili = 0;   // quanti punti sono sullo schermo, per il Monitor seriale
