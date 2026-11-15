// =====================================================================
//  TIPI — tutte le struct ed enum stanno QUI, prima di ogni funzione.
//  Arduino IDE aggiunge da solo le dichiarazioni delle funzioni prima
//  della prima funzione del file: se una struct venisse dopo, darebbe
//  l'errore "does not name a type".
// =====================================================================
#pragma once

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
  bool  cliccato;                 // vero solo nel fotogramma in cui il dito si stacca dopo un tocco breve, senza scorrere
  bool  swipe;                   // vero solo nel fotogramma in cui finisce uno swipe (dito che scorre veloce e si stacca)
  int   swipeDX, swipeDY;         // strada fatta dal dito nello swipe, in px (DX positivo = verso destra)
  float volumeVoce;               // volume sentito dal microfono, da 0 (silenzio) a 1 (fortissimo)
  float scossa;                   // quanto si sta scuotendo il dispositivo, in g oltre la gravità normale (0 = fermo)
  bool  orologioOk;               // l'orologio della board (RTC) risponde
  uint32_t orologioS;             // ora dell'orologio, in secondi dal 1° gennaio 2000 (va avanti anche da spento)
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
  int   passoGriglia;    // px fra un puntino e l'altro (0 = STELLA_PASSO_GRIGLIA; le stelle piccole della costellazione ne usano uno più fitto)
};

// I bisogni della stella e la sua salute, tutti da 100 (al massimo) a 0 (per niente)
struct Bisogni {
  float nutrimento;
  float socialita;
  float energia;
  float umore;
  float salute;          // media dei 4 bisogni
};

// Messaggio radio di Unisono: ogni stella lo manda a tutti quelli vicini (broadcast).
// "packed" = i campi stanno attaccati, senza spazi vuoti, così ogni dispositivo lo legge uguale.
struct __attribute__((packed)) MessaggioUnisono {
  char firma[4];         // "UNIS": serve a riconoscere i messaggi delle nostre stelle
  uint8_t versione;      // cambia se un giorno cambia la forma del messaggio
  char nome[10];         // nome della stella ("Sirio", "Vega", ...)
  uint16_t colore;       // colore della stella adesso (RGB565)
  uint8_t punte;         // numero di punte
  float vita;            // larghezza fra le punte (come STELLA_VITA)
  float affilatura;      // come STELLA_AFFILATURA
  uint8_t disponibile;   // 1 = è in salute e può entrare in Unisono
  uint32_t contatore;    // quanti messaggi ha mandato (serve a riconoscere un riavvio)
  uint32_t accesaDaMs;   // da quanti ms è accesa: il suo orologio, per andare a tempo col "direttore"
  char ritmo[32];        // il suo ritmo di 0 e 1 (lungo ORCHESTRA_PASSI = 32), così le altre la illuminano a tempo
};

// Una stella vicina sentita dalla radio
struct StellaVicina {
  uint8_t mac[6];        // indirizzo della sua radio (come una targa)
  char nome[10];
  uint16_t colore;
  uint8_t punte;
  float vita, affilatura;
  bool disponibile;
  int rssi;             // forza dell'ultimo segnale (-40 = attaccati, -90 = lontanissimi)
  int conferme;          // messaggi forti di fila (sopra UNISONO_RSSI_SOGLIA)
  uint32_t ultimoMs;     // quando è arrivato l'ultimo messaggio (millis)
  uint32_t confermataMs; // quando ha raggiunto UNISONO_CONFERME (0 = non ancora)
  int32_t differenzaMs;  // quanto il suo orologio è avanti rispetto al nostro, in ms
  char ritmo[32];        // il suo ritmo di 0 e 1
};

// Una stella della galassia di Unisono (vera, sentita dalla radio, oppure finta per le prove)
struct StellaGalassia {
  uint8_t mac[6];        // indirizzo della sua radio (le finte hanno tutti zeri)
  bool finta;
  char nome[10];
  uint16_t colore;
  uint8_t punte;
  float vita, affilatura;
  uint32_t arrivoMs;     // quando si è aggiunta alla galassia
  float posto;           // dove sta adesso lungo il suo braccio (0 = inizio, 1 = la stella più lontana); si sposta piano quando ne arriva un'altra
  float luce;            // 1 = appena illuminata (suona, o ci passa il braccio di luce), 0 = normale
  int indiceFinta;       // per le finte: quale delle UNISONO_FINTE_* (-1 = stella vera)
  int notaCorrente;      // per le finte: quale delle sue note suona la prossima volta
  char ritmo[32];        // il suo ritmo di 0 e 1
};

// A che punto è Unisono
enum FaseUnisono { UNISONO_NO,           // vita normale: la stella chiama e aspetta di incontrarne un'altra
                   UNISONO_ATTESA,       // incontrata una stella: per UNISONO_ATTESA_S si aspetta se ne arrivano altre
                   UNISONO_ANIMAZIONE }; // la galassia, per UNISONO_DURATA_S

enum Schermata { HOME, ANIMAZIONE_COSTELLAZIONE, MENU, UNISONO };
enum TastoMenu { TASTO_NESSUNO, TASTO_VOLUME_MENO, TASTO_VOLUME_PIU, TASTO_STELLE_MENO, TASTO_STELLE_PIU };   // i tasti del menu delle impostazioni

// Comportamenti che si possono scegliere in config_sirio.h / config_vega.h (e scambiare fra le due stelle)
enum TipoAscolto  { ASCOLTO_PALLINO, ASCOLTO_RAGGI };        // voce: la stella diventa un pallino / i raggi si allungano fino ai bordi
enum TipoCrescita { CRESCITA_TRATTI, CRESCITA_CONTINUA };    // carezza: cresce a tratti / cresce di continuo
enum TipoOrbita   { ORBITA_OTTO, ORBITA_CASUALE };           // movimento: disegna un 8 / orbite casuali con profondità
enum TipoFesta    { FESTA_SALTI_BORDO, FESTA_RIMBALZI };     // energia al 100%: salti lungo il bordo / rimbalzi fra alto e basso
