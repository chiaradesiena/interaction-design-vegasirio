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

enum Schermata { HOME, ANIMAZIONE_COSTELLAZIONE };

// Comportamenti che si possono scegliere in config_sirio.h / config_vega.h (e scambiare fra le due stelle)
enum TipoAscolto  { ASCOLTO_PALLINO, ASCOLTO_RAGGI };        // voce: la stella diventa un pallino / i raggi si allungano fino ai bordi
enum TipoCrescita { CRESCITA_TRATTI, CRESCITA_CONTINUA };    // carezza: cresce a tratti / cresce di continuo
enum TipoOrbita   { ORBITA_OTTO, ORBITA_CASUALE };           // movimento: disegna un 8 / orbite casuali con profondità
enum TipoFesta    { FESTA_SALTI_BORDO, FESTA_RIMBALZI };     // energia al 100%: salti lungo il bordo / rimbalzi fra alto e basso
