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

// Cielo
const float MOTION_BLUR                = 0.2;   // lunghezza della scia (0 = niente scia, 2 = scia doppia)

// Stella
const int   STELLA_NUMERO_PUNTE   = 7;          // Vega: stella a 7 punte
const float STELLA_AFFILATURA     = 5;          // più alto = punte più sottili e curve

// Bisogni
const float NUTRIMENTO_INIZIALE = 60;          // nutrimento all'accensione (0-100)
const float SOCIALITA_INIZIALE = 100;           // socialità all'accensione (0-100)
const float SOCIALITA_INTERVALLO_S = 9;         // ogni quanti secondi perde quei punti (Vega: 9 s = più in fretta del nutrimento che usa 15 s; più basso = scende più in fretta)
const uint32_t LAMPEGGIO_PERIODO_MS = 400;      // Vega: un lampeggio completo (acceso + spento) dura tanto, in ms; più basso = più veloce
const float SOCIALITA_VOCE_SOGLIA = 0.60;       // volume del microfono (0-1) da superare perché conti come "qualcuno parla"; più basso = basta una voce più piano

// Richiamo del nutrimento (Vega): quando il nutrimento arriva alla soglia suona "pi-pì, pi-pì", due coppie di note acute, breve + lunga
const uint8_t VOLUME_SUONO = 70;               // volume dell'altoparlante (0-255)
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

// Costellazione di Vega: la Lira, 6 stelle: Vega + 5 piccole (epsilon, zeta, delta, Sulafat, Sheliak)
// che si aggiungono una alla volta su un cerchio che gira attorno a Vega
const int COSTELLAZIONE_NUMERO_STELLE = 6;      // quante stelle ha la costellazione, Vega compresa
