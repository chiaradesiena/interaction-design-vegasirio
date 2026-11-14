// =====================================================================
//  Tamagotchi Vega — M5Stack CoreS3
//  Schermata HOME: una stella in uno spazio che ruota col giroscopio.
//
//  Ogni giro del loop segue tre tempi: LEGGI -> AGGIORNA -> DISEGNA.
//  - Solo la sezione 4 parla con i sensori.
//  - Le funzioni di disegno (sezioni 6-7) disegnano quello che ricevono,
//    non leggono mai i sensori.
//
//  Scheda:   M5CoreS3      Librerie: M5CoreS3 (+ M5Unified, M5GFX)
//  Monitor seriale: 115200 baud
// =====================================================================

#include <M5CoreS3.h>
#include <math.h>
#include <Preferences.h>   // memoria permanente della scheda (già inclusa nel core ESP32, nessuna libreria da installare)
#include <esp_system.h>    // per sapere perché la scheda è ripartita (reset o riaccensione)

// =====================================================================
//  0. TIPI — tutte le struct ed enum stanno QUI, prima di ogni funzione.
//  Arduino IDE aggiunge da solo le dichiarazioni delle funzioni prima
//  della prima funzione del file: se una struct venisse dopo, darebbe
//  l'errore "does not name a type".
// =====================================================================

// Un punto del cielo: direzione 3D (x, y, z lunghi 1 in tutto), dimensione e colore
struct PuntoCielo {
  float x, y, z;
  uint8_t dimensione;
  uint8_t grigio;          // luminosità di base (0-255)
  float fase;              // a che punto del respiro parte (0-1)
  float periodoMs;         // durata del suo respiro
  uint16_t colore;         // colore di questo fotogramma (cambia col respiro)
  uint16_t coloreScia;     // colore più scuro per il motion blur
};

// Molla: un valore che insegue un obiettivo con un po' di elasticità
// (va oltre, torna indietro e si ferma). Serve per drag ed effetto elastico.
struct Molla {
  float valore;
  float velocita;
};

// Tutto quello che i sensori dicono in questo fotogramma
struct Sensi {
  bool  imuOk;
  float giroX, giroY, giroZ;      // velocità di rotazione in gradi al secondo
  bool  toccato;                  // vero solo nel fotogramma in cui il dito appoggia lo schermo
  int   toccoX, toccoY;           // dove (in px) ha toccato
  bool  premuto;                  // vero finché il dito resta appoggiato allo schermo
  float volumeVoce;               // volume sentito dal microfono, da 0 (silenzio) a 1 (fortissimo)
  float scossa;                   // quanto si sta scuotendo il dispositivo, in g oltre la gravità normale (0 = fermo)
};

// Aspetto della stella: la logica (sezione 5) cambia questi numeri,
// il disegno (sezione 7) li usa. Gli stati futuri cambieranno solo questi.
struct AspettoStella {
  uint16_t colore;
  float raggio;          // px
  int   numeroPunte;
  float vita;            // larghezza fra le punte
  float affilatura;
  float rotazione;       // gradi
  float spostamentoX, spostamentoY;   // px, piccolo spostamento dal centro (tremolio)
  float buco;            // frazione del raggio lasciata vuota al centro (0 = nessun buco)
  float luminosita;      // 1 = stella accesa, 0 = spenta (serve al lampeggio)
};

// I bisogni della stella e la sua salute, tutti da 100 (al massimo) a 0 (per niente)
struct Bisogni {
  float nutrimento;
  float socialita;
  float energia;
  float umore;
  float salute;          // media dei 4 bisogni
};

enum Schermata { HOME };

// =====================================================================
//  1. CONFIGURAZIONE — numeri da tarare
// =====================================================================
const int LARGHEZZA = 320;
const int ALTEZZA   = 240;
const int LUMINOSITA_SCHERMO = 160;             // 0-255
const uint32_t INTERVALLO_STAMPA_MS = 1000;     // ogni quanto stampare gli FPS

// Cielo: i punti stanno su una sfera intorno al dispositivo, se ne vede solo una parte
const int   PUNTI_SULLA_SFERA = 215;            // 215 sulla sfera = circa 60 visibili sullo schermo
const float CAMPO_VISIVO      = 160.0;          // "zoom" in pixel: più alto = meno punti visibili e più grandi le distanze
const int   DIMENSIONE_MIN    = 1;              // px
const int   DIMENSIONE_MAX    = 3;              // px
const int   GRIGIO_MIN        = 85;            // luminosità dei punti più piccoli (0-255)
const uint32_t SEME_CIELO     = 12345;          // cambia questo numero per avere un altro cielo

// Rotazione col giroscopio
const float SENSIBILITA_ROTAZIONE = 1.0;        // 1 = il cielo gira quanto giri il dispositivo; 2 = il doppio
const float ZONA_MORTA_GRADI_S    = 1.5;        // rotazioni più lente di così vengono ignorate (toglie la deriva)
const int   CAMPIONI_CALIBRAZIONE = 100;        // letture all'accensione, a dispositivo fermo (circa 1 s)
// Se un movimento va al contrario, cambia il segno (1 oppure -1) dell'asse corrispondente
const float SEGNO_ASSE_X = 1;                   // inclinare avanti/indietro
const float SEGNO_ASSE_Y = 1;                   // girare a destra/sinistra
const float SEGNO_ASSE_Z = 1;                   // ruotare come un volante

// Elasticità e motion blur del cielo
const float ELASTICO_CIELO_FREQUENZA   = 3.0;   // più alto = cielo più rigido, segue subito il movimento
const float ELASTICO_CIELO_SMORZAMENTO = 0.5;   // 1 = nessun rimbalzo; più basso = rimbalza di più
const float MOTION_BLUR                = 0.2;   // lunghezza della scia (0 = niente scia, 2 = scia doppia)
const float MOTION_BLUR_LUCE           = 0.2;  // luminosità della scia rispetto al punto (0-1)

// Respiro delle stelle di sfondo (ognuna col suo ritmo)
const float RESPIRO_CIELO_AMPIEZZA     = 0.35;  // quanto si affievoliscono (0 = niente, 1 = spariscono)
const uint32_t RESPIRO_CIELO_MIN_MS    = 3000;  // respiro più veloce
const uint32_t RESPIRO_CIELO_MAX_MS    = 7000;  // respiro più lento

// Drag della stella: quando giri il dispositivo resta un po' indietro, poi torna al centro
const float DRAG_STELLA       = 0.08;           // quanto si sposta (più alto = più drag)
const float DRAG_MAX_PX       = 24;             // spostamento massimo dal centro, in px
const float DRAG_FREQUENZA    = 2.0;            // più alto = torna al centro più in fretta
const float DRAG_SMORZAMENTO  = 0.35;           // più basso = oscilla di più prima di fermarsi

// Stella (soggetto) — resa "halftone": fatta di puntini su una griglia
const int   STELLA_NUMERO_PUNTE   = 7;          // Vega: stella a 7 punte
const float STELLA_RAGGIO         = 64;         // px, dal centro alla fine di una punta
const float STELLA_VITA           = 0.3;       // larghezza fra le punte (0 = punte sottilissime, 1 = cerchio)
const float STELLA_AFFILATURA     = 5;          // più alto = punte più sottili e curve
const int   STELLA_PASSO_GRIGLIA  = 8;          // px fra un puntino e l'altro
const float STELLA_ALONE          = 1.3;        // alone di puntini minuscoli fino a 1.3 volte il raggio (1 = niente alone)
const float STELLA_FORZA_ALONE    = 0.1;      // quanto è visibile l'alone (0-1)
const float PULSAZIONE_AMPIEZZA   = 0.10;       // la stella cresce e cala del 6%
const uint32_t PULSAZIONE_PERIODO_MS = 3500;    // durata di un respiro di luce completo
const float STELLA_ROTAZIONE_GRADI_S = 0.5;       // rotazione lenta della stella su sé stessa (0 = ferma)

// Buco al centro della stella: più il nutrimento è basso, più è grande (lascia vedere il cielo dietro)
const float BUCO_SOGLIE[5] = { 91, 80, 60, 40, 20 };               // nutrimento da cui in su: 91 = nessun buco, 80 = livello 1, 60 = 2, 40 = 3, 20 = 4; sotto 20 = livello 5
const float BUCO_RAGGIO[6] = { 0, 0.15, 0.25, 0.35, 0.45, 0.55 };  // grandezza del buco per livello, come frazione del raggio della stella (0.55 = più della metà)
const float BUCO_VELOCITA  = 3;                 // quanto in fretta il buco cambia grandezza (più alto = più rapido, 3 = circa 1 secondo)

// Bisogni e salute: ogni bisogno va da 100 (soddisfatto) a 0 (per niente); la salute è la media dei 4
const float BISOGNO_MAX = 100;                  // valore di un bisogno completamente soddisfatto
const float NUTRIMENTO_INIZIALE = 60;          // nutrimento all'accensione (0-100)
const float NUTRIMENTO_DECADIMENTO = 1;         // quanti punti percentuali perde il nutrimento a ogni intervallo
const float NUTRIMENTO_INTERVALLO_S = 5;        // ogni quanti secondi perde quei punti (1 punto ogni 5 s = da 100 a 0 in 500 s)
const float MOLTIPLICATORE_TEMPO = 1;           // velocità dei bisogni: 1 = normale; 10 = dieci volte più veloci (utile per le prove)
const float SALUTE_SOGLIE[4] = { 80, 60, 40, 20 };   // salute da qui in su: 80 = livello bianco, 60 = azzurro, 40 = giallo, 20 = arancione; sotto 20 = rosso
const float SALUTE_DURATA_COLORE_MS = 1000;     // durata della sfumatura quando cambia il colore (0 = cambio netto)

// Socialità (Vega): scende col tempo, risale quando qualcuno parla vicino; sotto la soglia la stella lampeggia (acceso/spento netto)
const float SOCIALITA_INIZIALE = 100;           // socialità all'accensione (0-100)
const float SOCIALITA_DECADIMENTO = 1;          // quanti punti percentuali perde la socialità a ogni intervallo
const float SOCIALITA_INTERVALLO_S = 3;         // ogni quanti secondi perde quei punti (Vega: 3 s = più in fretta del nutrimento che usa 5 s; più basso = scende più in fretta)
const float SOCIALITA_SOGLIA_LAMPEGGIO = 50;    // sotto questo valore la stella lampeggia (0-100)
const uint32_t LAMPEGGIO_PERIODO_MS = 400;      // Vega: un lampeggio completo (acceso + spento) dura tanto, in ms; più basso = più veloce
const float SOCIALITA_VOCE_SOGLIA = 0.60;       // volume del microfono (0-1) da superare perché conti come "qualcuno parla"; più basso = basta una voce più piano
const float SOCIALITA_AUMENTO_PUNTI_S = 5;      // quanti punti di socialità si guadagnano al secondo finché qualcuno parla
const uint32_t SOCIALITA_TENUTA_VOCE_MS = 400;  // la voce conta ancora per tanto, in ms, dopo l'ultimo momento forte (tiene unite le pause fra le parole)
const uint32_t MIC_ATTESA_DOPO_SUONO_MS = 300;  // dopo un suono si aspetta tanto, in ms, prima di riaccendere il microfono (altoparlante e microfono non lavorano insieme)
const size_t   MIC_CAMPIONI = 256;              // quanti campioni ha ogni misura del microfono (256 a 16000 Hz = 16 ms)
const uint32_t MIC_FREQUENZA_HZ = 16000;        // frequenza di campionamento del microfono

// Feedback di ascolto (Vega): mentre qualcuno parla il nucleo resta uguale e i raggi si allungano fino ai bordi dello schermo
const float VEGA_ASCOLTO_RAGGIO_PX = 200;       // lunghezza dei raggi durante l'ascolto, in px (200 = arrivano agli angoli dello schermo; 120 = solo ai lati corti; più alto = vanno oltre)
const float ASCOLTO_VELOCITA_SALITA = 6;        // quanto in fretta i raggi si allungano quando inizia la voce (più alto = più rapido, 6 = circa mezzo secondo)
const float ASCOLTO_VELOCITA_DISCESA = 2.5;     // quanto in fretta tornano normali quando la voce finisce (più basso = ritorno più lento)

// Richiamo del nutrimento (Vega): quando il nutrimento arriva alla soglia suona "pi-pì, pi-pì", due coppie di note acute, breve + lunga
const uint8_t VOLUME_SUONO = 70;               // volume dell'altoparlante (0-255)
const float NUTRIMENTO_SOGLIA_SUONO = 50;       // il richiamo parte quando il nutrimento scende a questo valore (0-100)
const float RICHIAMO_RIPETIZIONE_S  = 20;       // ogni quanti secondi si ripete finché il nutrimento resta sotto la soglia
const int   SUONO_NUMERO_NOTE = 5;              // quante note ha il suono (devono corrispondere agli elenchi qui sotto)
const float SUONO_NOTE_HZ[SUONO_NUMERO_NOTE] =      // altezza di ogni nota in Hz (più alto = più acuto; 0 = silenzio)
  { 1046.50, 1318.51, 0, 1046.50, 1318.51 };         // pi, pì, pausa, pi, pì
const uint16_t SUONO_DURATE_MS[SUONO_NUMERO_NOTE] = // durata di ogni nota in ms (somma + pause: al massimo 3000)
  { 120, 300, 350, 120, 300 };
const uint16_t SUONO_PAUSA_MS = 30;             // silenzio fra una nota e l'altra, in ms

// Slider del volume: fila di puntini in basso, si trascina il dito (VOLUME_SUONO è il volume di partenza)
const int   SLIDER_ZONA_ALTEZZA   = 36;         // px, fascia in basso dove il dito controlla il volume
const int   SLIDER_MARGINE_X      = 30;         // px lasciati liberi a sinistra e a destra
const int   SLIDER_PASSO_PUNTINI  = 8;          // px fra un puntino e l'altro
const int   SLIDER_RAGGIO_CURSORE = 6;          // px, grandezza del cursore
const float SLIDER_NOTA_PROVA_HZ  = 880;        // nota di prova quando si toglie il dito (più alto = più acuta)
const uint16_t SLIDER_NOTA_PROVA_MS = 150;      // durata della nota di prova, in ms

// Carezza: tocco continuo con il dito che si muove a destra e a sinistra sulla stella = la si sta nutrendo
const float CAREZZA_DURATA_MIN_S = 1;           // secondi di carezza continua prima che il nutrimento cominci a salire
const float NUTRIMENTO_AUMENTO_PUNTI_S = 8;     // quanti punti di nutrimento al secondo si guadagnano accarezzando
const float CAREZZA_VELOCITA_MIN_PX_S = 10;     // velocità orizzontale minima del dito per contare come carezza (più alto = serve strusciare più in fretta)
const uint16_t CAREZZA_PAUSA_TOLLERATA_MS = 500;   // se la carezza si ferma o il dito si stacca per più di così, i 2 secondi ripartono da zero
const uint16_t CAREZZA_RITORNO_ATTESA_MS = 2000;   // dopo l'ultima carezza la stella resta ingrandita per tanto, poi torna normale
const float CAREZZA_AREA_STELLA = 1.5;          // zona sensibile: 1 = grandezza attuale della stella; più alto = più facile da toccare
const float CAREZZA_OCCUPAZIONE_MAX = 0.9;      // quanto dell'altezza dello schermo occupa al massimo la stella (0-1)
const float CAREZZA_RITORNO_FREQUENZA = 2.2;    // velocità con cui la stella cambia grandezza (più alto = più rapida)
const float TREMOLIO_AMPIEZZA_PX = 2.5;         // tremolio della stella mentre la si accarezza: spostamento massimo in px (0 = niente tremolio)
const float TREMOLIO_FREQUENZA_HZ = 7;          // quanto in fretta trema (più alto = più veloce)
const float TREMOLIO_VELOCITA_CAMBIO = 6;       // quanto in fretta il tremolio parte e si spegne (più alto = più rapido)
const float BATTITO_INTERVALLO_S = 1.5;         // durante la carezza, ogni quanti secondi suona un battito
const uint16_t BATTITO_DURATA_MS = 80;          // durata di ogni battito, in ms (più corto = più leggero)
const float BATTITO_HZ = 659.25;                // Vega: battito medio acuto (più alto = più acuto)

// Melodia di Vega quando, accarezzandola, il nutrimento arriva al 100% (ricavata da basic_pitch_transcription.mid, scala di re)
const float PIENO_RIARMO_SOGLIA = 90;           // dopo la melodia il nutrimento deve scendere sotto questo valore prima che possa suonare di nuovo (0-100)
const uint16_t PIENO_PAUSA_MS = 20;             // silenzio fra una nota e l'altra, in ms
const int   PIENO_NUMERO_NOTE = 12;             // quante note ha la melodia (devono corrispondere agli elenchi qui sotto)
const float PIENO_NOTE_HZ[PIENO_NUMERO_NOTE] =        // altezza di ogni nota in Hz (0 = silenzio)
  { 739.99, 880.00, 1174.66, 0, 1174.66, 0, 587.33, 880.00, 1174.66, 0, 880.00, 739.99 };   // fa#5 la5 re6 . re6 . re5 la5 re6 . la5 fa#5
const uint16_t PIENO_DURATE_MS[PIENO_NUMERO_NOTE] =   // durata di ogni nota in ms (somma + pause: al massimo 3000, ora circa 2600)
  { 120, 120, 240, 80, 150, 70, 130, 130, 320, 120, 220, 700 };
// Vega: più la carezza è continua, più la stella cresce
const float VEGA_DURATA_CRESCITA_S = 7;       // secondi di carezza continua per arrivare alla grandezza massima

// Memoria dei bisogni: salvataggio nella memoria permanente, così spegnendo e riaccendendo i valori restano
const bool  AZZERA_CON_RESET = true;            // true = il pulsante reset (freccia circolare) riporta i bisogni ai valori dello script; false = il reset li conserva
const bool  AZZERA_CON_TOCCO = false;           // true = col dito appoggiato allo schermo mentre la scheda parte i bisogni tornano ai valori dello script (ripiego se il reset non funziona)
const float SALVATAGGIO_INTERVALLO_S = 30;      // ogni quanti secondi si salva (più basso = si perde meno allo spegnimento, ma la memoria si consuma prima)
const float SALVATAGGIO_VARIAZIONE_MIN = 1;     // si salva solo se almeno un valore è cambiato di tanti punti
const float SALVATAGGIO_ATTESA_MIN_S = 5;       // quando cambia il colore della salute si salva subito, ma non più spesso di così
const uint32_t FIRMA_RIAVVIO = 0x57E11A01;      // numero-segnale lasciato in memoria: serve a riconoscere un reset (non va cambiato)

// Valori dei bisogni scritti in alto a sinistra
const bool  VALORI_MOSTRA = true;               // false = nessuna scritta
const bool  VALORI_MOSTRA_ENERGIA_UMORE = false;   // true = mostra anche l'umore (per ora non cambia; l'energia si vede sempre)
const int   VALORI_X = 6;                       // px dal bordo sinistro
const int   VALORI_Y = 6;                       // px dal bordo alto
const float VALORI_DIMENSIONE_TESTO = 1;        // grandezza del testo (1 = piccolo, 2 = doppio)
const int   VALORI_INTERLINEA_PX = 12;          // px fra una riga e l'altra (alzalo se alzi la grandezza del testo)

// Energia: scende col tempo come il nutrimento e sale scuotendo il dispositivo; con energia bassa la stella sta ferma
const float ENERGIA_INIZIALE = 50;              // energia all'accensione (0-100)
const float ENERGIA_DECADIMENTO = 1;            // quanti punti percentuali perde l'energia a ogni intervallo
const float ENERGIA_INTERVALLO_S = 5;           // ogni quanti secondi perde quei punti (come il nutrimento)
const float ENERGIA_SCOSSA_SOGLIA_G = 0.8;      // quanto forte si deve scuotere (g oltre la gravità normale); più alto = serve scuotere più forte
const float ENERGIA_SCOSSA_DURATA_MIN_S = 2;    // secondi di scuotimento continuo prima che l'energia cominci a salire
const uint16_t ENERGIA_SCOSSA_PAUSA_MS = 300;   // se non si scuote per più di così, i secondi ripartono da zero
const float ENERGIA_AUMENTO_PUNTI_S = 10;       // quanti punti di energia al secondo si guadagnano scuotendo
const float ENERGIA_SOGLIA_STATICA = 10;        // con energia pari o sotto questo valore la stella si ferma al centro (0-100)
const float ENERGIA_SOGLIA_USCITA_STATICA = 20; // per ripartire l'energia deve superare questo valore (più alto = resta ferma più a lungo)
const float ENERGIA_ATTIVITA_VELOCITA = 2;      // quanto in fretta la stella si ferma o riparte (più alto = più rapido)

// Movimento attivo (Vega): orbite casuali con profondità; la stella si rimpicciolisce quando si allontana e si ingrandisce quando si avvicina
const float VEGA_ORBITA_AMPIEZZA_X_PX = 50;    // quanto si sposta a destra e sinistra, in px (più alto = orbita più larga)
const float VEGA_ORBITA_AMPIEZZA_Y_PX = 35;    // quanto si sposta in alto e in basso, in px
const float VEGA_ORBITA_PERIODO_X_S = 13;      // secondi per un giro orizzontale (più alto = più lenta)
const float VEGA_ORBITA_PERIODO_Y_S = 17;      // secondi per un giro verticale (periodi diversi = percorso che non si ripete)
const float VEGA_ORBITA_PERIODO_Z_S = 11;      // secondi per un'andata e ritorno in profondità
const float VEGA_PROFONDITA = 0.25;            // quanto cambia la grandezza (0.25 = da 75% a 125%; 0 = nessuna profondità)
const float ORBITA_SECONDA_ONDA_PESO = 0.25;   // quanto conta la seconda oscillazione che rende l'orbita irregolare (0 = orbita regolare, 0.5 = molto irregolare)
const float ORBITA_SECONDA_ONDA_RAPPORTO = 0.618;   // durata della seconda oscillazione rispetto alla prima (non vicino a 1: si confonderebbe)
const float ORBITA_SECONDA_ONDA_FASE = 1.7;    // sfasamento della seconda oscillazione (radianti)
const float ORBITA_FASI[3] = { 0.0, 2.1, 4.2 };   // sfasamento di x, y e profondità, perché non vadano in sincrono

// Festa quando l'energia arriva al 100% scuotendo (Vega): rimbalza dal bordo alto a quello basso, torna al centro, sta ferma, riparte
const float FESTA_RIARMO_SOGLIA = 90;          // dopo la festa l'energia deve scendere sotto questo valore prima che possa ripartire (0-100)
const float FESTA_FERMO_S = 3;                 // secondi in cui la stella resta ferma al centro dopo il percorso
const int   VEGA_FESTA_RIMBALZI = 5;           // quante volte tocca un bordo (alto, basso, alto, ...)
const float VEGA_FESTA_VELOCITA_PX_S = 600;    // velocità del percorso in px al secondo (più alto = più veloce)
const float VEGA_FESTA_MARGINE_PX = 30;        // distanza del centro della stella dal bordo quando "rimbalza", in px (più basso = più vicino al bordo)

// Suono della festa di Vega, ricavato da vega_energia.mid (una sola voce per volta: quella più acuta; dura circa 5,2 secondi)
const uint16_t FESTA_PAUSA_MS = 20;            // silenzio fra una nota e l'altra, in ms
const int   FESTA_NUMERO_NOTE = 6;             // quante note ha il suono (devono corrispondere agli elenchi qui sotto)
const float FESTA_NOTE_HZ[FESTA_NUMERO_NOTE] =        // altezza di ogni nota in Hz (0 = silenzio)
  { 1396.91, 523.25, 0, 2793.83, 1046.50, 1046.50 };
const uint16_t FESTA_DURATE_MS[FESTA_NUMERO_NOTE] =   // durata di ogni nota in ms
  { 1095, 910, 468, 1072, 1118, 456 };

// =====================================================================
//  2. COLORI
// =====================================================================
// Converte un colore RGB normale nel formato RGB565 usato dallo schermo
constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
const uint16_t NERO   = rgb(0, 0, 0);
const uint16_t BIANCO = rgb(255, 255, 255);
const uint16_t ROSSO_STELLA = rgb(255, 0, 0);
const uint16_t AZZURRO_STELLA   = rgb(102, 255, 255);
const uint16_t GIALLO_STELLA    = rgb(255, 255, 51);
const uint16_t ARANCIONE_STELLA = rgb(255, 128, 0);
// Un colore per livello di salute: 100-80 bianco, 79-60 azzurro, 59-40 giallo, 39-20 arancione, 19-0 rosso
const uint16_t COLORI_SALUTE[5] = { BIANCO, AZZURRO_STELLA, GIALLO_STELLA, ARANCIONE_STELLA, ROSSO_STELLA };
const uint16_t GRIGIO_SLIDER = rgb(70, 70, 70);   // puntini dello slider oltre il livello del volume
const uint16_t COLORE_VALORI = rgb(200, 200, 200);   // scritta dei valori in alto a sinistra

// =====================================================================
//  3. DATI
// =====================================================================
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
bool suonoInCorso = false;                       // c'è un suono (o una melodia) in corso o si sta usando lo slider: il microfono aspetta
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
float profondita = 0;                            // variazione di grandezza dovuta alla profondità (0 = nessuna)
float movimentoX = 0, movimentoY = 0;            // spostamento totale dal centro (orbita + festa), in px
float energiaPrecedente = ENERGIA_INIZIALE;      // energia del fotogramma prima
bool festaAttiva = false;                        // è in corso la festa dell'energia al 100%
bool festaPronta = true;                         // la festa può partire quando l'energia arriva al 100%
uint32_t festaInizioMs = 0;                      // quando è partita la festa
float festaX = 0, festaY = 0;                    // spostamento dovuto al percorso della festa, in px
int festaNotaCorrente = -1;                      // -1 = suono fermo, altrimenti numero della prossima nota
uint32_t festaProssimaNotaMs = 0;                // a che ora deve partire la prossima nota

// =====================================================================
//  4. SENSORI
// =====================================================================
// Il giroscopio da fermo non segna mai zero esatto: questo errore (offset)
// si misura all'accensione e poi si sottrae
float offsetX = 0, offsetY = 0, offsetZ = 0;

void calibraGiroscopio() {
  Serial.println("calibrazione: tieni fermo il dispositivo...");
  float sx = 0, sy = 0, sz = 0;
  int letti = 0;
  for (int i = 0; i < CAMPIONI_CALIBRAZIONE; i++) {
    float gx, gy, gz;
    if (CoreS3.Imu.getGyro(&gx, &gy, &gz)) { sx += gx; sy += gy; sz += gz; letti++; }
    delay(10);
  }
  if (letti > 0) { offsetX = sx / letti; offsetY = sy / letti; offsetZ = sz / letti; }
  Serial.printf("offset giroscopio  x %.2f  y %.2f  z %.2f\n", offsetX, offsetY, offsetZ);
}

float zonaMorta(float v) {
  return fabsf(v) < ZONA_MORTA_GRADI_S ? 0 : v;
}

// Microfono e altoparlante usano lo stesso canale audio e non possono lavorare insieme:
// di solito ascolta il microfono; quando serve un suono si passa all'altoparlante e, finito il suono, si torna al microfono.
int16_t bufferMic[MIC_CAMPIONI];         // ultima misura del microfono
bool micAttivo = false;                  // vero = il canale audio è in mano al microfono
bool micHaDati = false;                  // una misura è stata richiesta e va letta
uint32_t fineSuonoMs = 0;                // a che ora finisce l'ultimo suono
float volumeVoceLetto = 0;               // ultimo volume misurato (0-1)

// Tutti i suoni passano di qui (al posto di CoreS3.Speaker.tone): se il microfono è acceso, lo spegne e riaccende l'altoparlante
void suona(float hz, uint32_t durataMs) {
  if (micAttivo) {
    CoreS3.Mic.end();
    CoreS3.Speaker.begin();
    micAttivo = false;
    micHaDati = false;
    volumeVoceLetto = 0;
  }
  CoreS3.Speaker.tone(hz, durataMs);
  fineSuonoMs = millis() + durataMs;
}

// Misura il volume dell'ambiente (0-1, stessa scala della pagina Microfono dell'esploratore) senza fermare il loop
void leggiMicrofono(uint32_t ora) {
  if (!micAttivo) {
    bool libero = !suonoInCorso && !CoreS3.Speaker.isPlaying() && (int32_t)(ora - fineSuonoMs) >= (int32_t)MIC_ATTESA_DOPO_SUONO_MS;
    if (!libero) { sensi.volumeVoce = 0; return; }
    CoreS3.Speaker.end();
    CoreS3.Mic.begin();
    micAttivo = true;
    micHaDati = false;
  }
  if (CoreS3.Mic.isEnabled() && !CoreS3.Mic.isRecording()) {
    if (micHaDati) {                                              // c'è una misura pronta: la trasformo in volume
      double somma = 0;
      for (size_t i = 0; i < MIC_CAMPIONI; i++) somma += (double)bufferMic[i] * bufferMic[i];
      float rms = sqrtf((float)(somma / MIC_CAMPIONI)) / 32768.0f;
      float db = 20.0f * log10f(rms < 1e-5f ? 1e-5f : rms);
      volumeVoceLetto = constrain((db + 60.0f) / 60.0f, 0.0f, 1.0f);
    }
    micHaDati = CoreS3.Mic.record(bufferMic, MIC_CAMPIONI, MIC_FREQUENZA_HZ);   // chiede la misura successiva
  }
  sensi.volumeVoce = volumeVoceLetto;
}

void leggiSensori() {
  auto tocco = CoreS3.Touch.getDetail();
  sensi.toccato = tocco.wasPressed();
  sensi.premuto = tocco.isPressed();
  sensi.toccoX  = tocco.x;
  sensi.toccoY  = tocco.y;
  leggiMicrofono(millis());

  float gx, gy, gz;
  sensi.imuOk = CoreS3.Imu.isEnabled() && CoreS3.Imu.getGyro(&gx, &gy, &gz);
  float ax, ay, az;                                                // scossa = quanto l'accelerazione si allontana dalla sola gravità (1 g)
  sensi.scossa = 0;
  if (sensi.imuOk && CoreS3.Imu.getAccel(&ax, &ay, &az)) sensi.scossa = fabsf(sqrtf(ax * ax + ay * ay + az * az) - 1.0f);
  if (!sensi.imuOk) { sensi.giroX = sensi.giroY = sensi.giroZ = 0; return; }
  sensi.giroX = zonaMorta(gx - offsetX) * SEGNO_ASSE_X;
  sensi.giroY = zonaMorta(gy - offsetY) * SEGNO_ASSE_Y;
  sensi.giroZ = zonaMorta(gz - offsetZ) * SEGNO_ASSE_Z;
}

// =====================================================================
//  5. LOGICA  (più avanti: statistiche e automa a stati)
// =====================================================================

// Rimette in ordine la matrice: i piccoli errori di calcolo, sommati,
// la deformerebbero e il cielo si "schiaccerebbe"
void raddrizzaOrientamento() {
  float* a = orientamento[0];
  float* b = orientamento[1];
  float* c = orientamento[2];
  float la = sqrtf(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
  for (int i = 0; i < 3; i++) a[i] /= la;
  float d = a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
  for (int i = 0; i < 3; i++) b[i] -= d * a[i];
  float lb = sqrtf(b[0]*b[0] + b[1]*b[1] + b[2]*b[2]);
  for (int i = 0; i < 3; i++) b[i] /= lb;
  c[0] = a[1]*b[2] - a[2]*b[1];
  c[1] = a[2]*b[0] - a[0]*b[2];
  c[2] = a[0]*b[1] - a[1]*b[0];
}

// Fa avanzare una molla verso "obiettivo".
// frequenza = rigidità (oscillazioni al secondo), smorzamento = quanto frena (1 = niente rimbalzo)
void aggiornaMolla(Molla& m, float obiettivo, float frequenza, float smorzamento, float dt) {
  float w = 2 * PI * frequenza;
  float accelerazione = w * w * (obiettivo - m.valore) - 2 * smorzamento * w * m.velocita;
  m.velocita += accelerazione * dt;
  m.valore   += m.velocita * dt;
}

const float GRADI_A_RADIANTI = PI / 180.0;

// Se il dispositivo gira di un certo angolo, il cielo (visto da dentro)
// gira dello stesso angolo al contrario: effetto "finestra sullo spazio".
// Il cielo usa la velocità "elastica": parte un po' in ritardo e, quando ti
// fermi, va leggermente oltre e torna indietro.
void aggiornaOrientamento(float dt) {
  memcpy(orientamentoPrecedente, orientamento, sizeof(orientamento));

  aggiornaMolla(elasticoX, sensi.giroX, ELASTICO_CIELO_FREQUENZA, ELASTICO_CIELO_SMORZAMENTO, dt);
  aggiornaMolla(elasticoY, sensi.giroY, ELASTICO_CIELO_FREQUENZA, ELASTICO_CIELO_SMORZAMENTO, dt);
  aggiornaMolla(elasticoZ, sensi.giroZ, ELASTICO_CIELO_FREQUENZA, ELASTICO_CIELO_SMORZAMENTO, dt);

  float wx = elasticoX.valore * GRADI_A_RADIANTI * SENSIBILITA_ROTAZIONE * dt;
  float wy = elasticoY.valore * GRADI_A_RADIANTI * SENSIBILITA_ROTAZIONE * dt;
  float wz = elasticoZ.valore * GRADI_A_RADIANTI * SENSIBILITA_ROTAZIONE * dt;
  float angolo = sqrtf(wx*wx + wy*wy + wz*wz);
  if (angolo < 1e-6f) return;

  // rotazione di "-angolo" attorno all'asse (kx, ky, kz)
  float kx = wx / angolo, ky = wy / angolo, kz = wz / angolo;
  float co = cosf(-angolo), si = sinf(-angolo), t = 1 - co;
  float r[3][3] = {
    { co + kx*kx*t,    kx*ky*t - kz*si, kx*kz*t + ky*si },
    { ky*kx*t + kz*si, co + ky*ky*t,    ky*kz*t - kx*si },
    { kz*kx*t - ky*si, kz*ky*t + kx*si, co + kz*kz*t    }
  };

  float nuova[3][3];
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      nuova[i][j] = r[i][0]*orientamento[0][j] + r[i][1]*orientamento[1][j] + r[i][2]*orientamento[2][j];
  memcpy(orientamento, nuova, sizeof(nuova));
  raddrizzaOrientamento();
}

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
uint32_t ultimaVoceMs = 0;               // quando il microfono ha sentito l'ultima volta un suono sopra la soglia
bool voceAttiva = false;                 // qualcuno sta parlando (o ha appena smesso)

void aggiornaSocialita(uint32_t ora, float dt) {
  bisogni.socialita -= SOCIALITA_DECADIMENTO / SOCIALITA_INTERVALLO_S * MOLTIPLICATORE_TEMPO * dt;
  if (sensi.volumeVoce >= SOCIALITA_VOCE_SOGLIA) { voceAttiva = true; ultimaVoceMs = ora; }
  else if (ora - ultimaVoceMs > SOCIALITA_TENUTA_VOCE_MS) voceAttiva = false;
  if (voceAttiva) bisogni.socialita += SOCIALITA_AUMENTO_PUNTI_S * dt;
  if (bisogni.socialita < 0) bisogni.socialita = 0;
  if (bisogni.socialita > BISOGNO_MAX) bisogni.socialita = BISOGNO_MAX;
}

// Ascolto: finché qualcuno parla (voceAttiva) il valore sale verso 1, poi torna piano verso 0.
void aggiornaAscolto(float dt) {
  bool ascoltando = voceAttiva && !festaAttiva;                  // durante la festa l'ascolto non si vede
  float velocita = ascoltando ? ASCOLTO_VELOCITA_SALITA : ASCOLTO_VELOCITA_DISCESA;
  float k = velocita * dt;
  if (k > 1.0f) k = 1.0f;
  ascolto += ((ascoltando ? 1.0f : 0.0f) - ascolto) * k;
}

// Vega: durante l'ascolto i raggi si allungano ma il nucleo (la parte fra un raggio e l'altro) e il buco della fame
// restano della stessa grandezza in px. Si parte dall'aspetto già calcolato e si modifica solo raggio, vita e buco.
void applicaAscolto() {
  float raggioBase = aspetto.raggio;
  float raggioAscolto = max(raggioBase, VEGA_ASCOLTO_RAGGIO_PX);
  float raggioNuovo = raggioBase + (raggioAscolto - raggioBase) * ascolto;
  aspetto.vita = aspetto.vita * raggioBase / raggioNuovo;
  aspetto.buco = aspetto.buco * raggioBase / raggioNuovo;
  aspetto.raggio = raggioNuovo;
}

// Lampeggio: sotto la soglia di socialità la luminosità passa a gradino da 1 a 0 a ritmo costante.
// Agisce solo sulla luce, mentre la fame agisce solo sul buco: i due segnali non si disturbano.
void aggiornaLampeggio(uint32_t ora) {
  bool solitaria = bisogni.socialita < SOCIALITA_SOGLIA_LAMPEGGIO;
  bool acceso = (ora % LAMPEGGIO_PERIODO_MS) < LAMPEGGIO_PERIODO_MS / 2;
  luminositaStella = (!solitaria || acceso || voceAttiva || festaAttiva) ? 1.0f : 0.0f;   // mentre si ascolta o c'è la festa la stella resta accesa
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

// Attività: con energia bassa la stella si ferma al centro; per ripartire deve superare una soglia più alta
// (così non balla fra i due stati). Durante la festa è ferma, perché il percorso è deciso dalla festa.
void aggiornaAttivita(float dt) {
  if (bisogni.energia <= ENERGIA_SOGLIA_STATICA) staticaEnergia = true;
  else if (bisogni.energia > ENERGIA_SOGLIA_USCITA_STATICA) staticaEnergia = false;
  float bersaglio = (staticaEnergia || festaAttiva) ? 0.0f : 1.0f;
  float k = ENERGIA_ATTIVITA_VELOCITA * dt;
  if (k > 1.0f) k = 1.0f;
  attivita += (bersaglio - attivita) * k;
}

// Vega: durata del percorso della festa. Tratti: centro -> bordo, (rimbalzi - 1) traversate, bordo -> centro.
float festaDurataPercorsoS() {
  float y = ALTEZZA / 2.0f - VEGA_FESTA_MARGINE_PX;
  return 2.0f * VEGA_FESTA_RIMBALZI * y / VEGA_FESTA_VELOCITA_PX_S;
}

// Vega: dove si trova la stella (rispetto al centro) t secondi dopo l'inizio della festa. Rimbalza fra bordo alto e basso a velocità costante.
void festaPosizione(float t, float& x, float& y) {
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
  if (arrivata && festaPronta && !festaAttiva) {
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

// Un'oscillazione irregolare fra -1 e 1: due onde con durate che non si sovrappongono mai, così l'orbita non si ripete
float ondaOrbita(float t, float periodoS, float fase) {
  float prima = sinf(2 * PI * t / periodoS + fase);
  float seconda = sinf(2 * PI * t / (periodoS * ORBITA_SECONDA_ONDA_RAPPORTO) + fase + ORBITA_SECONDA_ONDA_FASE);
  return (1.0f - ORBITA_SECONDA_ONDA_PESO) * prima + ORBITA_SECONDA_ONDA_PESO * seconda;
}

// Vega: orbite casuali con profondità. Tutto è moltiplicato per "attivita": a energia bassa o durante la festa torna piano al centro.
void aggiornaMovimento(uint32_t ora) {
  float t = ora / 1000.0f;
  orbitaX = VEGA_ORBITA_AMPIEZZA_X_PX * ondaOrbita(t, VEGA_ORBITA_PERIODO_X_S, ORBITA_FASI[0]) * attivita;
  orbitaY = VEGA_ORBITA_AMPIEZZA_Y_PX * ondaOrbita(t, VEGA_ORBITA_PERIODO_Y_S, ORBITA_FASI[1]) * attivita;
  profondita = VEGA_PROFONDITA * ondaOrbita(t, VEGA_ORBITA_PERIODO_Z_S, ORBITA_FASI[2]) * attivita;
  movimentoX = orbitaX + festaX;
  movimentoY = orbitaY + festaY;
}

// Applica il movimento all'aspetto già calcolato: sposta la stella e ne cambia la grandezza (profondità)
void applicaMovimento() {
  aspetto.spostamentoX += movimentoX;
  aspetto.spostamentoY += movimentoY;
  aspetto.raggio *= 1.0f + profondita;
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

// Slider del volume: si trascina il dito nella fascia in basso; quando lo si toglie suona una nota di prova
int volumeCorrente = VOLUME_SUONO;        // 0-255
bool sliderInUso = false;                 // il dito sta trascinando lo slider

void aggiornaSlider() {
  if (sensi.toccato && sensi.toccoY >= ALTEZZA - SLIDER_ZONA_ALTEZZA) sliderInUso = true;
  if (!sliderInUso) return;
  if (sensi.premuto) {
    float f = (sensi.toccoX - SLIDER_MARGINE_X) / (float)(LARGHEZZA - 2 * SLIDER_MARGINE_X);
    if (f < 0.0f) f = 0.0f;
    if (f > 1.0f) f = 1.0f;
    volumeCorrente = (int)(f * 255);
    CoreS3.Speaker.setVolume(volumeCorrente);
  } else {                                // dito tolto: fine trascinamento e nota di prova
    sliderInUso = false;
    suona(SLIDER_NOTA_PROVA_HZ, SLIDER_NOTA_PROVA_MS);
  }
}

float partenzaArrivoDolci(float t) {      // da 0-1 a 0-1, lento all'inizio e alla fine
  return t * t * (3.0f - 2.0f * t);
}

// Vega: più la carezza è continua, più la stella cresce, fino a occupare CAREZZA_OCCUPAZIONE_MAX dello schermo.
// Finita la carezza (e passata l'attesa) il bersaglio torna 1 e la stella si rimpicciolisce dolcemente.
void aggiornaScalaCarezza(float dt) {
  float scalaMax = CAREZZA_OCCUPAZIONE_MAX * (ALTEZZA / 2.0f) / (STELLA_RAGGIO * (1 + PULSAZIONE_AMPIEZZA));
  float k = tempoCrescitaS / VEGA_DURATA_CRESCITA_S;
  if (k > 1.0f) k = 1.0f;
  float bersaglio = 1.0f + (scalaMax - 1.0f) * partenzaArrivoDolci(k);
  aggiornaMolla(scalaCarezza, bersaglio, CAREZZA_RITORNO_FREQUENZA, 1.0f, dt);
}

// Carezza: dito appoggiato sulla stella che si muove soprattutto in orizzontale.
// Qui si decide se si sta accarezzando e si accumulano i tempi; il nutrimento sale in aggiornaNutrimento.
void aggiornaCarezza(uint32_t ora, float dt) {
  if (sensi.premuto && carezzaPrecPremuto && dt > 0) {            // velocità del dito, un po' lisciata
    float vx = (sensi.toccoX - carezzaPrecX) / dt;
    float vy = (sensi.toccoY - carezzaPrecY) / dt;
    carezzaVelX += (vx - carezzaVelX) * 0.4f;
    carezzaVelY += (vy - carezzaVelY) * 0.4f;
  } else {
    carezzaVelX = 0; carezzaVelY = 0;
  }
  carezzaPrecPremuto = sensi.premuto;
  carezzaPrecX = sensi.toccoX;
  carezzaPrecY = sensi.toccoY;

  float dx = sensi.toccoX - (LARGHEZZA / 2 + dragX.valore + movimentoX);
  float dy = sensi.toccoY - (ALTEZZA / 2 + dragY.valore + movimentoY);
  float r = aspetto.raggio * CAREZZA_AREA_STELLA;
  bool sullaStella = sensi.premuto && !sliderInUso && !festaAttiva && dx * dx + dy * dy <= r * r;   // durante la festa la carezza non conta
  carezzando = sullaStella && fabsf(carezzaVelX) >= CAREZZA_VELOCITA_MIN_PX_S
                           && fabsf(carezzaVelX) >= fabsf(carezzaVelY);

  if (carezzando) {
    ultimaCarezzaMs = ora;
    tempoContinuoS += dt;
    tempoCrescitaS += dt;
  }
  uint32_t daUltima = ora - ultimaCarezzaMs;
  if (daUltima > CAREZZA_PAUSA_TOLLERATA_MS) tempoContinuoS = 0;
  if (daUltima > CAREZZA_RITORNO_ATTESA_MS)  tempoCrescitaS = 0;
  aggiornaScalaCarezza(dt);
  aggiornaTremolio(ora, dt);
}

// Tremolio: mentre la si accarezza la stella vibra appena. Si sommano due oscillazioni a ritmi diversi
// per non farlo sembrare meccanico; parte e si spegne in modo graduale.
void aggiornaTremolio(uint32_t ora, float dt) {
  float bersaglio = (tempoContinuoS > 0) ? 1.0f : 0.0f;
  float k = TREMOLIO_VELOCITA_CAMBIO * dt;
  if (k > 1.0f) k = 1.0f;
  tremolioIntensita += (bersaglio - tremolioIntensita) * k;
  float t = ora / 1000.0f;
  float w = 2 * PI * TREMOLIO_FREQUENZA_HZ;
  float a = TREMOLIO_AMPIEZZA_PX * tremolioIntensita / 1.5f;
  tremolioX = a * (sinf(w * t) + 0.5f * sinf(w * 1.7f * t + 1.3f));
  tremolioY = a * (sinf(w * 1.3f * t + 0.7f) + 0.5f * sinf(w * 2.1f * t));
}

// Richiamo del nutrimento: quando scende alla soglia suona e poi si ripete a intervalli regolari.
// Suona una nota alla volta, controllando solo l'orologio, senza fermare il resto.
int suonoNotaCorrente = -1;               // -1 = fermo, altrimenti numero della prossima nota
uint32_t suonoProssimaNotaMs = 0;         // a che ora deve partire la prossima nota
uint32_t prossimoRichiamoMs = 0;          // a che ora deve ripartire il richiamo
bool nutrimentoSottoSoglia = false;       // il nutrimento è alla soglia o sotto?

void aggiornaSuonoNutrimento(uint32_t ora) {
  bool sotto = bisogni.nutrimento <= NUTRIMENTO_SOGLIA_SUONO;
  if (sotto && !nutrimentoSottoSoglia) prossimoRichiamoMs = ora;          // appena arrivato: subito
  nutrimentoSottoSoglia = sotto;
  if (sotto && suonoNotaCorrente < 0 && (int32_t)(ora - prossimoRichiamoMs) >= 0) {
    suonoNotaCorrente = 0;
    suonoProssimaNotaMs = ora;
    prossimoRichiamoMs = ora + (uint32_t)(RICHIAMO_RIPETIZIONE_S * 1000);
  }
  if (suonoNotaCorrente < 0) return;
  if ((int32_t)(ora - suonoProssimaNotaMs) < 0) return;                  // non è ancora il momento
  if (SUONO_NOTE_HZ[suonoNotaCorrente] > 0)
    suona(SUONO_NOTE_HZ[suonoNotaCorrente], SUONO_DURATE_MS[suonoNotaCorrente]);
  suonoProssimaNotaMs = ora + SUONO_DURATE_MS[suonoNotaCorrente] + SUONO_PAUSA_MS;
  suonoNotaCorrente++;
  if (suonoNotaCorrente >= SUONO_NUMERO_NOTE) suonoNotaCorrente = -1;    // finito
}

// Melodia del nutrimento al 100%: suona una volta quando, accarezzando, il nutrimento arriva al massimo.
// Per risuonare il nutrimento deve prima scendere sotto PIENO_RIARMO_SOGLIA.
int pienoNotaCorrente = -1;               // -1 = ferma, altrimenti numero della prossima nota
uint32_t pienoProssimaNotaMs = 0;         // a che ora deve partire la prossima nota
float nutrimentoPrecedente = NUTRIMENTO_INIZIALE;   // nutrimento del fotogramma prima
bool pienoPronto = true;                  // può suonare quando il nutrimento arriva al 100%

void aggiornaMelodiaPieno(uint32_t ora) {
  if (bisogni.nutrimento < PIENO_RIARMO_SOGLIA) pienoPronto = true;
  bool arrivatoAlMassimo = tempoContinuoS > 0 && nutrimentoPrecedente < BISOGNO_MAX && bisogni.nutrimento >= BISOGNO_MAX;
  nutrimentoPrecedente = bisogni.nutrimento;
  if (arrivatoAlMassimo && pienoPronto) {
    pienoPronto = false;
    pienoNotaCorrente = 0;
    pienoProssimaNotaMs = ora;
    Serial.println("nutrimento al 100%: melodia");
  }
  if (pienoNotaCorrente < 0) return;
  if ((int32_t)(ora - pienoProssimaNotaMs) < 0) return;                   // non è ancora il momento
  if (PIENO_NOTE_HZ[pienoNotaCorrente] > 0)
    suona(PIENO_NOTE_HZ[pienoNotaCorrente], PIENO_DURATE_MS[pienoNotaCorrente]);
  pienoProssimaNotaMs = ora + PIENO_DURATE_MS[pienoNotaCorrente] + PIENO_PAUSA_MS;
  pienoNotaCorrente++;
  if (pienoNotaCorrente >= PIENO_NUMERO_NOTE) pienoNotaCorrente = -1;     // finita
}

// Battito della carezza: finché la si accarezza suona una nota breve ogni BATTITO_INTERVALLO_S secondi
// (non quando sta suonando il richiamo del nutrimento, per non sovrapporli).
bool battitoAttivo = false;               // la carezza è in corso e il battito sta andando
uint32_t prossimoBattitoMs = 0;           // a che ora suona il prossimo battito

void aggiornaSuonoCarezza(uint32_t ora) {
  if (tempoContinuoS <= 0) { battitoAttivo = false; return; }
  if (!battitoAttivo) { battitoAttivo = true; prossimoBattitoMs = ora; }   // inizio carezza: primo battito subito
  if ((int32_t)(ora - prossimoBattitoMs) < 0) return;
  prossimoBattitoMs = ora + (uint32_t)(BATTITO_INTERVALLO_S * 1000);
  if (suonoNotaCorrente < 0 && pienoNotaCorrente < 0 && festaNotaCorrente < 0) suona(BATTITO_HZ, BATTITO_DURATA_MS);
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

// Drag: la stella viene "trascinata" nella stessa direzione in cui scorre il cielo
// (giri a destra -> il cielo scorre a sinistra -> la stella scivola a sinistra),
// poi la molla la riporta al centro con un piccolo rimbalzo.
void aggiornaDragStella(float dt) {
  // velocità con cui il centro del cielo scorre sullo schermo, in px al secondo
  float vx = CAMPO_VISIVO * sensi.giroY * GRADI_A_RADIANTI * SENSIBILITA_ROTAZIONE;
  float vy = CAMPO_VISIVO * sensi.giroX * GRADI_A_RADIANTI * SENSIBILITA_ROTAZIONE;
  float ox = vx * DRAG_STELLA, oy = vy * DRAG_STELLA;
  float lunghezza = sqrtf(ox * ox + oy * oy);
  if (lunghezza > DRAG_MAX_PX) { ox *= DRAG_MAX_PX / lunghezza; oy *= DRAG_MAX_PX / lunghezza; }
  aggiornaMolla(dragX, ox, DRAG_FREQUENZA, DRAG_SMORZAMENTO, dt);
  aggiornaMolla(dragY, oy, DRAG_FREQUENZA, DRAG_SMORZAMENTO, dt);
}

// Respiro dello sfondo: ogni punto si affievolisce e si riaccende col suo ritmo
void aggiornaRespiroCielo(uint32_t ora) {
  for (int i = 0; i < PUNTI_SULLA_SFERA; i++) {
    PuntoCielo& p = cielo[i];
    float onda = 0.5f + 0.5f * sinf(2 * PI * (ora / p.periodoMs + p.fase));   // fra 0 e 1
    uint8_t g = p.grigio * (1 - RESPIRO_CIELO_AMPIEZZA * onda);
    uint8_t gs = g * MOTION_BLUR_LUCE;
    p.colore     = rgb(g, g, g);
    p.coloreScia = rgb(gs, gs, gs);
  }
}

// ---- Memoria dei bisogni
// Un valore è accettabile se è un numero fra 0 e 100 (un NaN o un dato rovinato non passa)
bool bisognoValido(float v) {
  return v >= 0.0f && v <= BISOGNO_MAX;
}

// All'accensione: decide se azzerare (reset o tocco) oppure riprendere i valori salvati.
// Se non c'è nulla di salvato o il dato non è valido, restano i valori dello script.
void caricaStato() {
  memoria.begin("stella", false);
  esp_reset_reason_t motivo = esp_reset_reason();
  // reset = pin esterno, oppure "accensione" ma con la firma ancora in memoria (dopo una vera riaccensione la firma è sparita)
  bool resetPremuto = (motivo == ESP_RST_EXT) || (motivo == ESP_RST_POWERON && firmaRiavvio == FIRMA_RIAVVIO);
  firmaRiavvio = FIRMA_RIAVVIO;
  bool toccoAvvio = false;
  if (AZZERA_CON_TOCCO) {
    for (int i = 0; i < 10 && !toccoAvvio; i++) {    // il tocco ha bisogno di qualche istante per essere letto
      CoreS3.update();
      toccoAvvio = CoreS3.Touch.getDetail().isPressed();
      delay(20);
    }
  }
  Serial.printf("avvio: motivo %d   reset riconosciuto %d   tocco %d\n", (int)motivo, resetPremuto, toccoAvvio);
  if ((AZZERA_CON_RESET && resetPremuto) || toccoAvvio) {
    memoria.remove("bisogni");
    Serial.println("bisogni azzerati: valori dello script");
  } else if (memoria.getBytesLength("bisogni") == sizeof(Bisogni)) {
    Bisogni letti;
    memoria.getBytes("bisogni", &letti, sizeof(Bisogni));
    if (bisognoValido(letti.nutrimento) && bisognoValido(letti.socialita) && bisognoValido(letti.energia)
        && bisognoValido(letti.umore) && bisognoValido(letti.salute)) {
      bisogni = letti;
      Serial.println("bisogni ripresi dalla memoria");
    }
  }
  bisogniSalvati = bisogni;
  livelloSalvato = calcolaLivelloSalute(bisogni.salute);
}

float differenzaMassima(const Bisogni& a, const Bisogni& b) {
  float d = fabsf(a.nutrimento - b.nutrimento);
  d = max(d, fabsf(a.socialita - b.socialita));
  d = max(d, fabsf(a.energia - b.energia));
  d = max(d, fabsf(a.umore - b.umore));
  d = max(d, fabsf(a.salute - b.salute));
  return d;
}

// Salva ogni SALVATAGGIO_INTERVALLO_S secondi (se qualcosa è cambiato) e, più in fretta, quando cambia il colore della salute.
// Non si salva a ogni fotogramma: la memoria permanente sopporta un numero limitato di scritture.
void salvaStato(uint32_t ora) {
  uint32_t dallUltimo = ora - ultimoSalvataggioMs;
  bool periodico = dallUltimo >= (uint32_t)(SALVATAGGIO_INTERVALLO_S * 1000)
                   && differenzaMassima(bisogni, bisogniSalvati) >= SALVATAGGIO_VARIAZIONE_MIN;
  bool cambioColore = livelloCorrente != livelloSalvato && dallUltimo >= (uint32_t)(SALVATAGGIO_ATTESA_MIN_S * 1000);
  if (!periodico && !cambioColore) return;
  memoria.putBytes("bisogni", &bisogni, sizeof(Bisogni));
  bisogniSalvati = bisogni;
  livelloSalvato = livelloCorrente;
  ultimoSalvataggioMs = ora;
}

uint32_t ultimoAggiornamento = 0;

void aggiornaStato() {
  uint32_t ora = millis();
  float dt = (ora - ultimoAggiornamento) / 1000.0;   // secondi dall'ultimo fotogramma
  ultimoAggiornamento = ora;
  if (dt > 0.1) dt = 0.1;                            // evita salti se un fotogramma tarda
  aggiornaOrientamento(dt);
  aggiornaDragStella(dt);
  aggiornaRespiroCielo(ora);
  aggiornaCarezza(ora, dt);
  aggiornaNutrimento(dt);
  aggiornaSocialita(ora, dt);
  aggiornaEnergia(ora, dt);
  aggiornaSalute();
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
  salvaStato(ora);
  suonoInCorso = suonoNotaCorrente >= 0 || pienoNotaCorrente >= 0 || festaNotaCorrente >= 0 || sliderInUso;   // il microfono aspetta finché c'è un suono
}

// =====================================================================
//  6. DISEGNO CIELO
// =====================================================================

// Numeri "casuali" ripetibili: con lo stesso seme esce sempre lo stesso cielo
uint32_t statoCasuale = SEME_CIELO;
float casuale01() {                       // numero fra 0 e 1
  statoCasuale = statoCasuale * 1664525UL + 1013904223UL;
  return (statoCasuale >> 8) / 16777216.0f;
}

void creaCielo() {
  for (int i = 0; i < PUNTI_SULLA_SFERA; i++) {
    // direzione distribuita in modo uniforme sulla sfera
    float z = casuale01() * 2 - 1;
    float angolo = casuale01() * 2 * PI;
    float r = sqrtf(1 - z * z);
    cielo[i].x = r * cosf(angolo);
    cielo[i].y = r * sinf(angolo);
    cielo[i].z = z;

    // i punti grandi sono rari; più grandi = più luminosi
    float t = casuale01() * casuale01();
    int dim = DIMENSIONE_MIN + (int)(t * (DIMENSIONE_MAX - DIMENSIONE_MIN + 1));
    if (dim > DIMENSIONE_MAX) dim = DIMENSIONE_MAX;
    cielo[i].dimensione = dim;
    uint8_t g = GRIGIO_MIN + (255 - GRIGIO_MIN) * (dim - DIMENSIONE_MIN) / (DIMENSIONE_MAX - DIMENSIONE_MIN);
    cielo[i].grigio = g;
    cielo[i].colore = rgb(g, g, g);
  }
  // ritmo del respiro: in un secondo giro, così la disposizione del cielo resta la stessa di prima
  for (int i = 0; i < PUNTI_SULLA_SFERA; i++) {
    cielo[i].fase = casuale01();
    cielo[i].periodoMs = RESPIRO_CIELO_MIN_MS + casuale01() * (RESPIRO_CIELO_MAX_MS - RESPIRO_CIELO_MIN_MS);
  }
}

// Dalla direzione 3D alla posizione sullo schermo; false se il punto è alle spalle
bool proietta(const float m[3][3], const PuntoCielo& p, int& sx, int& sy) {
  float x = m[0][0]*p.x + m[0][1]*p.y + m[0][2]*p.z;
  float y = m[1][0]*p.x + m[1][1]*p.y + m[1][2]*p.z;
  float z = m[2][0]*p.x + m[2][1]*p.y + m[2][2]*p.z;
  if (z >= -0.05f) return false;
  sx = LARGHEZZA / 2 + (int)(CAMPO_VISIVO * x / -z);
  sy = ALTEZZA / 2   - (int)(CAMPO_VISIVO * y / -z);
  return true;
}

// Motion blur: una scia dal punto verso dove si trovava nel fotogramma prima
void disegnaScia(M5Canvas& c, int x, int y, int xPrima, int yPrima, int dim, uint16_t col) {
  float dx = (xPrima - x) * MOTION_BLUR, dy = (yPrima - y) * MOTION_BLUR;
  float lunghezza = sqrtf(dx * dx + dy * dy);
  if (lunghezza < 1.5f) return;                   // quasi fermo: niente scia
  if (lunghezza > 40) { dx *= 40 / lunghezza; dy *= 40 / lunghezza; }
  int x2 = x + (int)dx, y2 = y + (int)dy;
  c.drawLine(x, y, x2, y2, col);
  if (dim >= 2) c.drawLine(x + 1, y, x2 + 1, y2, col);   // punti grandi: scia più spessa
  if (dim >= 3) c.drawLine(x, y + 1, x2, y2 + 1, col);
}

void disegnaPunto(M5Canvas& c, int x, int y, int dim, uint16_t col) {
  if (dim % 2 == 1) c.fillCircle(x, y, dim / 2, col);          // dispari (1, 3, 5 px): tondo
  else              c.fillRect(x - dim / 2, y - dim / 2, dim, dim, col);  // pari (2, 4 px): quadrato
}

// Ogni punto viene girato secondo l'orientamento del dispositivo e poi
// proiettato guardando "dietro lo schermo" (verso -z).
int puntiVisibili = 0;   // quanti punti sono sullo schermo, per il Monitor seriale

void disegnaCielo(M5Canvas& c, const float m[3][3], const float mPrima[3][3]) {
  puntiVisibili = 0;
  for (int i = 0; i < PUNTI_SULLA_SFERA; i++) {
    const PuntoCielo& p = cielo[i];
    int sx, sy, px, py;
    if (!proietta(m, p, sx, sy)) continue;
    if (sx < -45 || sx > LARGHEZZA + 45 || sy < -45 || sy > ALTEZZA + 45) continue;
    if (MOTION_BLUR > 0 && proietta(mPrima, p, px, py))
      disegnaScia(c, sx, sy, px, py, p.dimensione, p.coloreScia);
    disegnaPunto(c, sx, sy, p.dimensione, p.colore);
    puntiVisibili++;
  }
}

// =====================================================================
//  7. DISEGNO STELLA
// =====================================================================

// Quanto un punto (dx, dy) dal centro è "dentro" la stella:
// 1 al centro, 0 sul bordo e fuori. Il bordo è una forma a punte curve.
float densitaStella(float dx, float dy, const AspettoStella& a) {
  float distanza = sqrtf(dx * dx + dy * dy);
  float angolo = atan2f(dy, dx) - a.rotazione * PI / 180.0f;
  float onda = powf(fabsf(cosf(a.numeroPunte * angolo / 2)), a.affilatura);   // 1 sulle punte, 0 fra le punte
  float bordo = a.raggio * (a.vita + (1 - a.vita) * onda);
  float dentro = 1 - distanza / bordo;
  float alone = (1 - distanza / (a.raggio * STELLA_ALONE)) * STELLA_FORZA_ALONE;
  return max(max(dentro, alone), 0.0f);
}

// Griglia di puntini: più il punto è dentro la stella, più il puntino è grande
void disegnaStella(M5Canvas& c, int cx, int cy, const AspettoStella& a) {
  if (a.luminosita <= 0) return;                      // fase "spenta" del lampeggio: la stella non si vede
  const int passo = STELLA_PASSO_GRIGLIA;
  const float raggioMax = passo / 2.0f;
  int n = (int)(a.raggio * max(STELLA_ALONE, 1.0f) / passo) + 1;
  for (int i = -n; i <= n; i++) {
    for (int j = -n; j <= n; j++) {
      float bucoPx = a.buco * a.raggio;                // il buco lascia vedere il cielo: qui non si disegna niente
      if (bucoPx > 0 && (float)(i * i + j * j) * passo * passo < bucoPx * bucoPx) continue;
      float rf = raggioMax * densitaStella(i * passo, j * passo, a);
      if (rf < 0.25f) continue;                       // troppo piccolo: niente puntino
      int x = cx + i * passo, y = cy - j * passo;
      c.fillRect(x - passo / 2, y - passo / 2, passo, passo, NERO);   // copre il cielo dietro la stella
      c.fillCircle(x, y, (int)(rf + 0.5f), a.colore); // raggio 0 = un solo pixel
    }
  }
}

// Slider del volume: una fila di puntini che crescono verso destra (halftone);
// quelli fino al livello del volume sono bianchi, gli altri grigi, e un puntino grande fa da cursore
void disegnaSlider(M5Canvas& c, int volume) {
  const int y = ALTEZZA - SLIDER_ZONA_ALTEZZA / 2;
  const int lunghezza = LARGHEZZA - 2 * SLIDER_MARGINE_X;
  const int n = lunghezza / SLIDER_PASSO_PUNTINI + 1;
  const int xCursore = SLIDER_MARGINE_X + (int)(volume / 255.0f * lunghezza);
  for (int i = 0; i < n; i++) {
    int x = SLIDER_MARGINE_X + i * SLIDER_PASSO_PUNTINI;
    int r = 1 + (int)(2.0f * i / (n - 1) + 0.5f);                 // da 1 a 3 px
    c.fillCircle(x, y, r, x <= xCursore ? BIANCO : GRIGIO_SLIDER);
  }
  c.fillCircle(xCursore, y, SLIDER_RAGGIO_CURSORE, BIANCO);
}

// =====================================================================
//  8. SCHERMATE
// =====================================================================
// Una riga di testo, per esempio "salute: 60%" (senza accenti; niente String, solo un testo di lunghezza fissa)
void disegnaRigaValore(M5Canvas& c, int riga, const char* nome, float valore) {
  char testo[24];
  snprintf(testo, sizeof(testo), "%s: %d%%", nome, (int)(valore + 0.5f));
  c.drawString(testo, VALORI_X, VALORI_Y + riga * VALORI_INTERLINEA_PX);
}

// Valori dei bisogni e della salute in alto a sinistra, in tempo reale
void disegnaValori(M5Canvas& c, const Bisogni& b) {
  if (!VALORI_MOSTRA) return;
  c.setTextColor(COLORE_VALORI);
  c.setTextSize(VALORI_DIMENSIONE_TESTO);
  c.setTextDatum(top_left);
  int riga = 0;
  disegnaRigaValore(c, riga++, "salute", b.salute);
  disegnaRigaValore(c, riga++, "nutrimento", b.nutrimento);
  disegnaRigaValore(c, riga++, "socialita", b.socialita);
  disegnaRigaValore(c, riga++, "energia", b.energia);
  if (VALORI_MOSTRA_ENERGIA_UMORE) disegnaRigaValore(c, riga++, "umore", b.umore);
}

void schermataHome(M5Canvas& c) {
  c.fillScreen(NERO);
  disegnaCielo(c, orientamento, orientamentoPrecedente);
  disegnaStella(c, LARGHEZZA / 2 + (int)(dragX.valore + aspetto.spostamentoX), ALTEZZA / 2 + (int)(dragY.valore + aspetto.spostamentoY), aspetto);
  disegnaSlider(c, volumeCorrente);
  disegnaValori(c, bisogni);
}

// =====================================================================
//  9. SETUP E LOOP
// =====================================================================
uint32_t ultimaStampa = 0;
int fotogrammi = 0;

void setup() {
  auto cfg = M5.config();
  CoreS3.begin(cfg);
  Serial.begin(115200);
  caricaStato();

  CoreS3.Display.setBrightness(LUMINOSITA_SCHERMO);
  CoreS3.Speaker.setVolume(VOLUME_SUONO);
  CoreS3.Display.fillScreen(NERO);

  tela.setColorDepth(16);
  if (!tela.createSprite(LARGHEZZA, ALTEZZA)) {   // se manca memoria si ripiega sugli 8 bit
    tela.setColorDepth(8);
    tela.createSprite(LARGHEZZA, ALTEZZA);
    Serial.println("canvas a 8 bit");
  }

  creaCielo();
  memcpy(orientamentoPrecedente, orientamento, sizeof(orientamento));
  CoreS3.Display.setTextColor(BIANCO);
  CoreS3.Display.drawCenterString("tienimi fermo...", LARGHEZZA / 2, ALTEZZA / 2);
  calibraGiroscopio();
  ultimoAggiornamento = millis();
  Serial.println("HOME pronta");
}

void loop() {
  CoreS3.update();

  leggiSensori();
  aggiornaStato();

  switch (schermataCorrente) {
    case HOME: schermataHome(tela); break;
  }
  tela.pushSprite(0, 0);

  // FPS = fotogrammi al secondo
  fotogrammi++;
  if (millis() - ultimaStampa >= INTERVALLO_STAMPA_MS) {
    Serial.printf("FPS %d   punti visibili %d   giro x %.0f y %.0f z %.0f\n",
                  fotogrammi, puntiVisibili, sensi.giroX, sensi.giroY, sensi.giroZ);
    Serial.printf("nutrimento %.0f   socialita %.0f   energia %.0f   umore %.0f   salute %.0f   livello %d\n",
                  bisogni.nutrimento, bisogni.socialita, bisogni.energia, bisogni.umore, bisogni.salute, livelloCorrente);
    Serial.printf("carezza %d   tempo continuo %.1f s   grandezza %.2f\n", carezzando, tempoContinuoS, scalaCarezza.valore);
    Serial.printf("volume voce %.2f (soglia %.2f)   voce attiva %d   microfono %d   luminosita %.0f\n",
                  sensi.volumeVoce, SOCIALITA_VOCE_SOGLIA, voceAttiva, micAttivo, luminositaStella);
    Serial.printf("memoria libera %u   minima %u   psram %u   ascolto %.2f   microfono %d\n",
                  (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMinFreeHeap(), (unsigned)ESP.getPsramSize(), ascolto, micAttivo);
    Serial.printf("scossa %.2f g   scuotendo %d   tempo scossa %.1f s   statica %d   attivita %.2f   festa %d\n",
                  sensi.scossa, scossaRecente, tempoScossaS, staticaEnergia, attivita, festaAttiva);
    fotogrammi = 0;
    ultimaStampa = millis();
  }

  delay(1);
}
