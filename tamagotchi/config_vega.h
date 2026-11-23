// =====================================================================
//  CONFIGURAZIONE DI VEGA — i numeri e i comportamenti propri di Vega.
//  Viene usata quando in personaggio.h è attiva la riga PERSONAGGIO_VEGA.
// =====================================================================
#pragma once

const char* const NOME_PERSONAGGIO = "Vega";    // compare sul Monitor seriale all'accensione

// Comportamenti (le possibilità sono scritte in tipi.h; si possono scambiare con quelli di Sirio)
const TipoAscolto  ASCOLTO_TIPO  = ASCOLTO_RAGGI;       // voce: i raggi si allungano fino ai bordi (ASCOLTO_PALLINO = come Sirio)
const TipoCrescita CRESCITA_TIPO = CRESCITA_CONTINUA;   // carezza: cresce di continuo (CRESCITA_TRATTI = come Sirio)
const TipoOrbita   ORBITA_TIPO   = ORBITA_CASUALE;      // movimento: orbite casuali con profondità (ORBITA_OTTO = come Sirio)
const TipoFesta    FESTA_TIPO    = FESTA_RIMBALZI;      // energia al 100%: rimbalzi fra alto e basso (FESTA_SALTI_BORDO = come Sirio)
const TipoStatoProprio STATO_PROPRIO_TIPO = PROPRIO_NESSUNO;   // salute alta: per ora nessuna animazione propria (PROPRIO_CANE = come Sirio)

// Cielo
const float MOTION_BLUR                = 0.2;   // lunghezza della scia (0 = niente scia, 2 = scia doppia)

// Stella
const int   STELLA_NUMERO_PUNTE   = 7;          // Vega: stella a 7 punte
const float STELLA_AFFILATURA     = 5;          // più alto = punte più sottili e curve
const float STELLA_SCALA          = 1.0;        // Vega: grandezza sullo schermo rispetto a STELLA_RAGGIO (più alto = più grande; Sirio 1.3); il cielo non cambia
const bool  STILE_FITTO_ATTIVO    = true;       // true = Vega ha anche la versione a puntini fitti (swipe verso l'alto); false = con lo swipe passa subito ai raggi

// Versione a raggi di puntini (disegno_raggi.ino): i numeri sono in config_comune.h (RAGGI_VEGA)
const CarattereRaggi RAGGI_MIEI = RAGGI_VEGA;   // i raggi di questa stella

// Bisogni
const float NUTRIMENTO_INIZIALE = 60;          // nutrimento all'accensione (0-100)
const float SOCIALITA_INIZIALE = 100;           // socialità all'accensione (0-100)
const float SOCIALITA_INTERVALLO_S = 9;         // ogni quanti secondi perde quei punti (Vega: 9 s = più in fretta del nutrimento che usa 15 s; più basso = scende più in fretta)
const uint32_t LAMPEGGIO_PERIODO_MS = 400;      // Vega: un lampeggio completo (acceso + spento) dura tanto, in ms; più basso = più veloce
const float SOCIALITA_VOCE_SOGLIA = 0.60;       // volume del microfono (0-1) da superare perché conti come "qualcuno parla"; più basso = basta una voce più piano

// Richiamo del nutrimento (Vega): quando il nutrimento arriva alla soglia suona "pi-pì, pi-pì", due coppie di note acute, breve + lunga
const uint8_t VOLUME_SUONO = 70;               // non più usato: il volume ora si sceglie a livelli (VOLUME_LIVELLO_INIZIALE)
const int VOLUME_LIVELLO_INIZIALE = 3;          // livello del volume all'accensione (1 = muto ... 10 = massimo; i valori sono in VOLUME_LIVELLI)
const int   SUONO_NUMERO_NOTE = 5;              // quante note ha il suono (devono corrispondere agli elenchi qui sotto)
const float SUONO_NOTE_HZ[SUONO_NUMERO_NOTE] =      // altezza di ogni nota in Hz (più alto = più acuto; 0 = silenzio)
  { 1046.50, 1318.51, 0, 1046.50, 1318.51 };         // pi, pì, pausa, pi, pì
const uint16_t SUONO_DURATE_MS[SUONO_NUMERO_NOTE] = // durata di ogni nota in ms (somma + pause: al massimo 3000)
  { 120, 300, 350, 120, 300 };
const uint16_t SUONO_PAUSA_MS = 30;             // silenzio fra una nota e l'altra, in ms

// Battito della carezza
const float BATTITO_HZ = 659.25;                // Vega: battito medio acuto (più alto = più acuto)

// Melodia di Vega quando, accarezzandola, il nutrimento arriva al 100% (ricavata da basic_pitch_transcription.mid, scala di re)
const int   PIENO_NUMERO_NOTE = 12;             // quante note ha la melodia (devono corrispondere agli elenchi qui sotto)
const float PIENO_NOTE_HZ[PIENO_NUMERO_NOTE] =        // altezza di ogni nota in Hz (0 = silenzio)
  { 739.99, 880.00, 1174.66, 0, 1174.66, 0, 587.33, 880.00, 1174.66, 0, 880.00, 739.99 };   // fa#5 la5 re6 . re6 . re5 la5 re6 . la5 fa#5
const uint16_t PIENO_DURATE_MS[PIENO_NUMERO_NOTE] =   // durata di ogni nota in ms (somma + pause: al massimo 3000, ora circa 2600)
  { 120, 120, 240, 80, 150, 70, 130, 130, 320, 120, 220, 700 };

// Suono della festa di Vega, ricavato da vega_energia.mid (una sola voce per volta: quella più acuta; dura circa 5,2 secondi)
const int   FESTA_NUMERO_NOTE = 6;             // quante note ha il suono (devono corrispondere agli elenchi qui sotto)
const float FESTA_NOTE_HZ[FESTA_NUMERO_NOTE] =        // altezza di ogni nota in Hz (0 = silenzio)
  { 1396.91, 523.25, 0, 2793.83, 1046.50, 1046.50 };
const uint16_t FESTA_DURATE_MS[FESTA_NUMERO_NOTE] =   // durata di ogni nota in ms
  { 1095, 910, 468, 1072, 1118, 456 };

// Costellazione di Vega: la Lira, 6 stelle: Vega (alfa) + 5 piccole, disposte e unite come nel disegno vero
// (il parallelogramma sotto Vega, con delta doppia); si aggiungono una alla volta nell'ordine della tabella e tutta la costellazione
// gira piano attorno a Vega. Più stelle ci sono, più la scena (Vega compresa) rimpicciolisce (logica_costellazione.ino).
const int COSTELLAZIONE_NUMERO_STELLE = 6;      // quante stelle ha la costellazione, Vega compresa (deve essere 1 + le righe della tabella)
const bool COSTELLAZIONE_DISEGNATA = true;      // true = stelle ai posti della tabella qui sotto; false = su un cerchio, unite a catena
const bool COSTELLAZIONE_PALLINI = true;        // true = stelle piccole come pallini con un anello di puntini intorno; false = stelline a puntini (halftone)
const float COSTELLAZIONE_SCALA_PX = 1.5;       // px sullo schermo per ogni unità della tabella, finché la scena non rimpicciolisce (più alto = stelle più lontane da Vega)
// Posto di ogni stella piccola rispetto a Vega, in unità del disegno (x verso destra, y verso il basso), nell'ordine in cui compaiono
const float COSTELLAZIONE_POSIZIONI[COSTELLAZIONE_NUMERO_STELLE - 1][2] = {
  { -28,  33 },    // 1 sotto Vega, dove parte il parallelogramma
  { -28, 121 },    // 2 beta (Sheliak, in basso a destra)
  { -65, 143 },    // 3 gamma (Sulafat, in basso a sinistra)
  { -64,  54 },    // 4 delta, il puntino di sinistra
  { -58,  51 } };  // 5 delta, il puntino di destra
// A quale stella è unita ognuna col suo tratto: 0 = Vega, 1-5 = la stella piccola con quel numero (deve essere già comparsa), -1 = nessun tratto
const int COSTELLAZIONE_COLLEGATA_A[COSTELLAZIONE_NUMERO_STELLE - 1] = { 0, 1, 2, 3, 1 };
// Un secondo tratto, per chiudere i triangoli (-1 = nessuno)
const int COSTELLAZIONE_COLLEGATA_ANCHE_A[COSTELLAZIONE_NUMERO_STELLE - 1] = { -1, -1, -1, -1, -1 };
// Grandezza di ogni stella piccola, come nel disegno vero (1 = grande come le più grandi di Sirio; più basso = più piccola)
const float COSTELLAZIONE_GRANDEZZE[COSTELLAZIONE_NUMERO_STELLE - 1] =
  // sotto Vega  beta  gamma  delta sx  delta dx
  {  0.7,        0.7,  0.7,   0.4,      0.4 };

// Voce di Vega nell'orchestra di Unisono: acuta e veloce, ritmo fitto (tante note brevi).
// Il ritmo ha ORCHESTRA_PASSI = 32 caselle, scritte a gruppi di 8 (un gruppo = un tempo): 1 = suona, 0 = silenzio.
// Una battuta intera (32 caselle) è un giro del braccio di luce. A ogni 1 suona la nota successiva dell'elenco.
// (Una copia di questa voce è anche fra le stelle finte, in config_comune.h: se la cambi qui, cambiala anche lì.)
const char* const ORCHESTRA_RITMO = "10110101" "10101101" "10110101" "11011011";
const int   ORCHESTRA_NUMERO_NOTE = 8;          // quante note ha l'elenco qui sotto
const float ORCHESTRA_NOTE_HZ[ORCHESTRA_NUMERO_NOTE] =   // altezza delle note in Hz (Vega: la, do, mi, re, sol acuti)
  { 880.00, 1046.50, 1318.51, 1174.66, 1567.98, 1318.51, 1174.66, 1046.50 };
const uint16_t ORCHESTRA_DURATA_MS = 70;        // durata di ogni nota, in ms (più basso = note più secche)
