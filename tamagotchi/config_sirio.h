// =====================================================================
//  CONFIGURAZIONE DI SIRIO — i numeri e i comportamenti propri di Sirio.
//  Viene usata quando in personaggio.h è attiva la riga PERSONAGGIO_SIRIO.
// =====================================================================
#pragma once

const char* const NOME_PERSONAGGIO = "Sirio";   // compare sul Monitor seriale all'accensione

// Comportamenti (le possibilità sono scritte in tipi.h; si possono scambiare con quelli di Vega)
const TipoAscolto  ASCOLTO_TIPO  = ASCOLTO_PALLINO;     // voce: la stella diventa un pallino (ASCOLTO_RAGGI = come Vega)
const TipoCrescita CRESCITA_TIPO = CRESCITA_TRATTI;     // carezza: cresce a tratti (CRESCITA_CONTINUA = come Vega)
const TipoOrbita   ORBITA_TIPO   = ORBITA_OTTO;         // movimento: disegna un 8 (ORBITA_CASUALE = come Vega)
const TipoFesta    FESTA_TIPO    = FESTA_SALTI_BORDO;   // energia al 100%: salti lungo il bordo (FESTA_RIMBALZI = come Vega)
const TipoStatoProprio STATO_PROPRIO_TIPO = PROPRIO_NESSUNO;   // salute alta: nessuna animazione propria (PROPRIO_CANE = ogni tanto diventa un cane)

// Cielo
const float MOTION_BLUR                = 0.5;   // lunghezza della scia (0 = niente scia, 2 = scia doppia)

// Stella
const int   STELLA_NUMERO_PUNTE   = 4;          // Sirio: stella a 4 punte
const float STELLA_AFFILATURA     = 2;          // più alto = punte più sottili e curve; 2 = angoli morbidi e arrotondati
const float STELLA_SCALA          = 1.3;        // Sirio: grandezza sullo schermo rispetto a STELLA_RAGGIO (1 = come Vega, 1.3 = 30% più grande); vale per tutte le versioni, il cielo non cambia
const bool  STILE_FITTO_ATTIVO    = false;      // false = Sirio non ha la versione a puntini fitti (con lo swipe passa subito ai raggi); true = ce l'ha

// Versione a raggi di puntini (disegno_raggi.ino): i numeri sono in config_comune.h (RAGGI_SIRIO)
const CarattereRaggi RAGGI_MIEI = RAGGI_SIRIO;  // i raggi di questa stella

// Bisogni
const float NUTRIMENTO_INIZIALE = 40;          // nutrimento all'accensione (0-100)
const float SOCIALITA_INIZIALE = 51;           // socialità all'accensione (0-100)
const float SOCIALITA_INTERVALLO_S = 24;        // ogni quanti secondi perde quei punti (Sirio: 24 s = più piano del nutrimento che usa 15 s; più basso = scende più in fretta)
const uint32_t LAMPEGGIO_PERIODO_MS = 1600;     // Sirio: un lampeggio completo (acceso + spento) dura tanto, in ms; più alto = più lento
const float SOCIALITA_VOCE_SOGLIA = 0.80;       // volume del microfono (0-1) da superare perché conti come "qualcuno parla"; più basso = basta una voce più piano

// Richiamo del nutrimento (Sirio): quando il nutrimento arriva alla soglia suona "tu-tuu-tuuu", tre note gravi che scendono e si allungano
const uint8_t VOLUME_SUONO = 128;               // non più usato: il volume ora si sceglie a livelli (VOLUME_LIVELLO_INIZIALE)
const int VOLUME_LIVELLO_INIZIALE = 6;          // livello del volume all'accensione (1 = muto ... 10 = massimo; i valori sono in VOLUME_LIVELLI)
const int   SUONO_NUMERO_NOTE = 3;              // quante note ha il suono (devono corrispondere agli elenchi qui sotto)
const float SUONO_NOTE_HZ[SUONO_NUMERO_NOTE] =      // altezza di ogni nota in Hz (più basso = più grave; 0 = silenzio)
  { 220.00, 174.61, 146.83 };
const uint16_t SUONO_DURATE_MS[SUONO_NUMERO_NOTE] = // durata di ogni nota in ms (somma + pause: al massimo 3000)
  { 400, 800, 1200 };
const uint16_t SUONO_PAUSA_MS = 50;             // silenzio fra una nota e l'altra, in ms

// Battito della carezza
const float BATTITO_HZ = 196.00;                // Sirio: battito medio basso (più basso = più grave)

// Melodia di Sirio quando, accarezzandola, il nutrimento arriva al 100% (ricavata da basic_pitch_transcription (1).mid, accorciata a meno di 4 secondi)
const int   PIENO_NUMERO_NOTE = 33;             // quante note ha la melodia (devono corrispondere agli elenchi qui sotto)
const float PIENO_NOTE_HZ[PIENO_NUMERO_NOTE] =        // altezza di ogni nota in Hz (0 = silenzio)
  { 0, 277.18, 246.94, 311.13, 329.63, 369.99, 329.63, 0, 440.00, 0, 440.00, 0, 587.33, 0, 554.37, 0, 1046.50,
    880.00, 261.63, 0, 246.94, 311.13, 349.23, 0, 329.63, 415.30, 466.16, 0, 440.00, 0, 554.37, 0, 783.99 };
const uint16_t PIENO_DURATE_MS[PIENO_NUMERO_NOTE] =   // durata di ogni nota in ms (somma + pause: al massimo 4000, ora circa 3900)
  { 26, 108, 40, 78, 78, 116, 78, 33, 85, 108, 93, 34, 71, 48, 64, 553, 40,
    78, 85, 26, 71, 71, 124, 32, 78, 71, 124, 40, 85, 199, 71, 342, 79 };

// Suono della festa di Sirio, ricavato da sirio_energia.mid (una sola voce per volta: quella più acuta; dura circa 1,9 secondi)
const int   FESTA_NUMERO_NOTE = 5;             // quante note ha il suono (devono corrispondere agli elenchi qui sotto)
const float FESTA_NOTE_HZ[FESTA_NUMERO_NOTE] =        // altezza di ogni nota in Hz (0 = silenzio)
  { 116.54, 0, 116.54, 1396.91, 116.54 };
const uint16_t FESTA_DURATE_MS[FESTA_NUMERO_NOTE] =   // durata di ogni nota in ms
  { 677, 328, 490, 142, 120 };

// Costellazione di Sirio: il Cane Maggiore, 10 stelle: Sirio + 9 piccole, disposte come nel disegno vero
// e unite come nel disegno; si aggiungono una alla volta nell'ordine della tabella e tutta la costellazione gira piano attorno a Sirio.
// Più stelle ci sono, più la scena (Sirio compreso) rimpicciolisce, così stanno tutte nello schermo (logica_costellazione.ino).
const int COSTELLAZIONE_NUMERO_STELLE = 10;     // quante stelle ha la costellazione, Sirio compresa (deve essere 1 + le righe della tabella)
const bool COSTELLAZIONE_DISEGNATA = true;      // true = stelle ai posti della tabella qui sotto; false = su un cerchio, unite a catena (come Vega)
const bool COSTELLAZIONE_PALLINI = true;        // true = stelle piccole come pallini con un anello di puntini intorno; false = stelline a puntini (halftone)
const float COSTELLAZIONE_SCALA_PX = 1.5;       // px sullo schermo per ogni unità della tabella, finché la scena non rimpicciolisce (più alto = stelle più lontane da Sirio)
// Posto di ogni stella piccola rispetto a Sirio, in unità del disegno (x verso destra, y verso il basso), nell'ordine in cui compaiono
const float COSTELLAZIONE_POSIZIONI[COSTELLAZIONE_NUMERO_STELLE - 1][2] = {
  { -58, -21 },    // 1 gamma (in alto a sinistra)
  { -12, -73 },    // 2 theta (in cima)
  {  18,  27 },    // 3 incrocio del ramo di destra
  {  72,  34 },    // 4 beta (Mirzam, a destra)
  {  29, 102 },    // 5 fondo del ramo di destra
  { -72,  94 },    // 6 a metà del ramo lungo
  { -95, 127 },    // 7 delta (Wezen)
  { -70, 165 },    // 8 epsilon (Adhara)
  {-150, 163 } };  // 9 eta (Aludra, la più lontana)
// A quale stella è unita ognuna col suo tratto: 0 = Sirio, 1-9 = la stella piccola con quel numero (deve essere già comparsa) , -1 = nessun tratto
const int COSTELLAZIONE_COLLEGATA_A[COSTELLAZIONE_NUMERO_STELLE - 1] = { 0, 0, 0, 3, 3, 0, 6, 7, 7 };
// Un secondo tratto, per chiudere i triangoli (-1 = nessuno): theta si unisce anche a gamma
const int COSTELLAZIONE_COLLEGATA_ANCHE_A[COSTELLAZIONE_NUMERO_STELLE - 1] = { -1, 1, -1, -1, -1, -1, -1, -1, -1 };
// Grandezza di ogni stella piccola, come nel disegno vero (1 = le più grandi: beta, epsilon, eta; più basso = più piccola). Vale per i pallini e i puntini neri dell'animazione
const float COSTELLAZIONE_GRANDEZZE[COSTELLAZIONE_NUMERO_STELLE - 1] =
  // gamma theta incrocio beta fondo  metà delta epsilon eta
  {  0.6,  0.6,  0.4,    1.0, 0.4,  0.6, 0.8,  1.0,    1.0 };

// Voce di Sirio nell'orchestra di Unisono: grave e lenta, ritmo largo (poche note lunghe).
// Il ritmo ha ORCHESTRA_PASSI = 32 caselle, scritte a gruppi di 8 (un gruppo = un tempo): 1 = suona, 0 = silenzio.
// Una battuta intera (32 caselle) è un giro del braccio di luce. A ogni 1 suona la nota successiva dell'elenco.
const char* const ORCHESTRA_RITMO = "10000000" "00000000" "10000000" "00100000";
const int   ORCHESTRA_NUMERO_NOTE = 3;          // quante note ha l'elenco qui sotto
const float ORCHESTRA_NOTE_HZ[ORCHESTRA_NUMERO_NOTE] =   // altezza delle note in Hz (Sirio: la, mi, sol gravi)
  { 220.00, 164.81, 196.00 };
const uint16_t ORCHESTRA_DURATA_MS = 900;       // durata di ogni nota, in ms (più alto = note più lunghe e legate)
