// =====================================================================
//  Tamagotchi Stella — M5Stack CoreS3
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
  float spostamentoX;    // px, spostamento orizzontale dal centro (animazione Diva)
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
const float MOTION_BLUR                = 0.5;   // lunghezza della scia (0 = niente scia, 2 = scia doppia)
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
const int   STELLA_NUMERO_PUNTE   = 12;
const float STELLA_RAGGIO         = 64;         // px, dal centro alla fine di una punta
const float STELLA_VITA           = 0.3;       // larghezza fra le punte (0 = punte sottilissime, 1 = cerchio)
const float STELLA_AFFILATURA     = 5;          // più alto = punte più sottili e curve
const int   STELLA_PASSO_GRIGLIA  = 8;          // px fra un puntino e l'altro
const float STELLA_ALONE          = 1.3;        // alone di puntini minuscoli fino a 1.3 volte il raggio (1 = niente alone)
const float STELLA_FORZA_ALONE    = 0.1;      // quanto è visibile l'alone (0-1)
const float PULSAZIONE_AMPIEZZA   = 0.10;       // la stella cresce e cala del 6%
const uint32_t PULSAZIONE_PERIODO_MS = 3500;    // durata di un respiro di luce completo
const float STELLA_ROTAZIONE_GRADI_S = 0.5;       // rotazione lenta della stella su sé stessa (0 = ferma)

// Suono: toccando la stella parte una melodia pentatonica "spaziale" (scala di re: re, mi, fa#, la, si)
const uint8_t VOLUME_SUONO       = 128;         // volume dell'altoparlante (0-255)
const float TOCCO_AREA_STELLA    = 1.3;         // zona sensibile: 1 = raggio della stella; più alto = più facile da toccare
const int   MELODIA_NUMERO_NOTE  = 10;          // quante note ha la melodia (devono corrispondere agli elenchi qui sotto)
const float MELODIA_NOTE_HZ[MELODIA_NUMERO_NOTE] =       // altezza di ogni nota in Hz (più alto = più acuta)
  { 293.66, 440.00, 587.33, 739.99, 880.00, 987.77, 880.00, 739.99, 440.00, 293.66 };
const uint16_t MELODIA_DURATE_MS[MELODIA_NUMERO_NOTE] =  // durata di ogni nota in ms (l'ultima è lunga, come un'eco)
  { 80, 80, 80, 80, 100, 170, 100, 100, 120, 410 };
const uint16_t MELODIA_PAUSA_MS  = 20;         // silenzio fra una nota e l'altra, in ms (0 = note legate)

// Animazione Diva: ogni tanto la stella si avvicina per "osservare" lo spettatore, controlla gli angoli e torna
const uint16_t DIVA_INTERVALLO_MIN_S = 45;      // attesa minima fra due animazioni, in secondi
const uint16_t DIVA_INTERVALLO_MAX_S = 60;      // attesa massima, in secondi (ogni volta se ne sceglie una a caso fra min e max)
const int   DIVA_NUMERO_FASI = 7;               // quante fasi ha l'animazione (devono corrispondere agli elenchi qui sotto)
// Fasi in ordine: 1 si avvicina, 2 osserva, 3 va a sinistra, 4 va a destra, 5 torna al centro, 6 si rimpicciolisce, 7 torna normale
const uint16_t DIVA_DURATE_MS[DIVA_NUMERO_FASI] =   // durata di ogni fase in ms (somma = durata totale, ora 6000 = 6 s)
  { 1200, 800, 600, 1200, 600, 1000, 600 };
const float DIVA_SCALA[DIVA_NUMERO_FASI] =          // grandezza della stella a fine fase (1 = normale; 1.8 = 1.8 volte più grande)
  { 1.8, 1.8, 1.9, 1.9, 1.8, 0.7, 1.0 };             // ai lati un po' più grande, per dare prospettiva
const float DIVA_SPOSTAMENTO_PX[DIVA_NUMERO_FASI] = // posizione orizzontale a fine fase, in px dal centro (negativo = sinistra)
  { 0, 0, -50, 50, 0, 0, 0 };

// =====================================================================
//  2. COLORI
// =====================================================================
// Converte un colore RGB normale nel formato RGB565 usato dallo schermo
constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
const uint16_t NERO   = rgb(0, 0, 0);
const uint16_t BIANCO = rgb(255, 255, 255);
const uint16_t ROSSO_STELLA = rgb(255, 55, 65);

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

void leggiSensori() {
  auto tocco = CoreS3.Touch.getDetail();
  sensi.toccato = tocco.wasPressed();
  sensi.toccoX  = tocco.x;
  sensi.toccoY  = tocco.y;

  float gx, gy, gz;
  sensi.imuOk = CoreS3.Imu.isEnabled() && CoreS3.Imu.getGyro(&gx, &gy, &gz);
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

// Animazione Diva: una sequenza a fasi (vedi DIVA_* in configurazione).
// Ogni fase porta grandezza e posizione dal valore di fine della fase prima a quello suo, con partenza e arrivo dolci.
// Qui si calcolano solo divaScala e divaSpostamentoX; li usa aggiornaAspettoStella.
float divaScala = 1.0f;           // moltiplicatore del raggio (1 = normale)
float divaSpostamentoX = 0.0f;    // px dal centro
bool divaInCorso = false;
uint32_t divaInizioMs = 0;        // quando è partita l'animazione in corso
uint32_t prossimaDivaMs = 0;      // quando partirà la prossima

void programmaProssimaDiva(uint32_t ora) {
  prossimaDivaMs = ora + random(DIVA_INTERVALLO_MIN_S * 1000UL, DIVA_INTERVALLO_MAX_S * 1000UL + 1);
}

float partenzaArrivoDolci(float t) {      // da 0-1 a 0-1, lento all'inizio e alla fine
  return t * t * (3.0f - 2.0f * t);
}

void aggiornaDiva(uint32_t ora) {
  if (!divaInCorso) {
    divaScala = 1.0f; divaSpostamentoX = 0.0f;
    if ((int32_t)(ora - prossimaDivaMs) < 0) return;       // non è ancora il momento
    divaInCorso = true;
    divaInizioMs = ora;
  }
  uint32_t t = ora - divaInizioMs;
  float scalaPrima = 1.0f, xPrima = 0.0f;
  for (int i = 0; i < DIVA_NUMERO_FASI; i++) {
    if (t < DIVA_DURATE_MS[i]) {                           // siamo dentro la fase i
      float k = partenzaArrivoDolci(t / (float)DIVA_DURATE_MS[i]);
      divaScala        = scalaPrima + (DIVA_SCALA[i] - scalaPrima) * k;
      divaSpostamentoX = xPrima + (DIVA_SPOSTAMENTO_PX[i] - xPrima) * k;
      return;
    }
    t -= DIVA_DURATE_MS[i];
    scalaPrima = DIVA_SCALA[i];
    xPrima = DIVA_SPOSTAMENTO_PX[i];
  }
  divaInCorso = false;                                     // finita: si torna alla stella normale
  divaScala = 1.0f; divaSpostamentoX = 0.0f;
  programmaProssimaDiva(ora);
}

// Per ora la stella è sempre rossa e "respira" piano.
// Qui, più avanti, ogni stato dell'automa sceglierà colore, punte, ritmo...
void aggiornaAspettoStella(uint32_t ora) {
  float fase = (ora % PULSAZIONE_PERIODO_MS) / (float)PULSAZIONE_PERIODO_MS;
  aspetto.colore      = ROSSO_STELLA;
  aspetto.raggio      = STELLA_RAGGIO * (1 + PULSAZIONE_AMPIEZZA * sinf(2 * PI * fase)) * divaScala;
  aspetto.spostamentoX = divaSpostamentoX;
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

// Melodia: parte quando si tocca la stella e suona una nota alla volta,
// senza fermare il resto (si controlla solo l'orologio a ogni fotogramma)
int notaCorrente = -1;            // -1 = melodia ferma, altrimenti numero della prossima nota
uint32_t prossimaNotaMs = 0;      // a che ora deve partire la prossima nota

// Il tocco è sulla stella? (la stella si sposta col drag, quindi si usa la sua posizione attuale)
bool stellaToccata() {
  float dx = sensi.toccoX - (LARGHEZZA / 2 + dragX.valore);
  float dy = sensi.toccoY - (ALTEZZA / 2 + dragY.valore);
  float r = STELLA_RAGGIO * TOCCO_AREA_STELLA;
  return dx * dx + dy * dy <= r * r;
}

void aggiornaMelodia(uint32_t ora) {
  if (notaCorrente < 0) {                                  // ferma: aspetta un tocco sulla stella
    if (!(sensi.toccato && stellaToccata()) || divaInCorso) return;   // durante la Diva il tocco non fa nulla
    notaCorrente = 0;
    prossimaNotaMs = ora;
  }
  if ((int32_t)(ora - prossimaNotaMs) < 0) return;         // non è ancora il momento
  CoreS3.Speaker.tone(MELODIA_NOTE_HZ[notaCorrente], MELODIA_DURATE_MS[notaCorrente]);
  prossimaNotaMs = ora + MELODIA_DURATE_MS[notaCorrente] + MELODIA_PAUSA_MS;
  notaCorrente++;
  if (notaCorrente >= MELODIA_NUMERO_NOTE) notaCorrente = -1;   // finita
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
  aggiornaDiva(ora);
  aggiornaAspettoStella(ora);
  aggiornaMelodia(ora);
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
  const int passo = STELLA_PASSO_GRIGLIA;
  const float raggioMax = passo / 2.0f;
  int n = (int)(a.raggio * max(STELLA_ALONE, 1.0f) / passo) + 1;
  for (int i = -n; i <= n; i++) {
    for (int j = -n; j <= n; j++) {
      float rf = raggioMax * densitaStella(i * passo, j * passo, a);
      if (rf < 0.25f) continue;                       // troppo piccolo: niente puntino
      int x = cx + i * passo, y = cy - j * passo;
      c.fillRect(x - passo / 2, y - passo / 2, passo, passo, NERO);   // copre il cielo dietro la stella
      c.fillCircle(x, y, (int)(rf + 0.5f), a.colore); // raggio 0 = un solo pixel
    }
  }
}

// =====================================================================
//  8. SCHERMATE
// =====================================================================
void schermataHome(M5Canvas& c) {
  c.fillScreen(NERO);
  disegnaCielo(c, orientamento, orientamentoPrecedente);
  disegnaStella(c, LARGHEZZA / 2 + (int)dragX.valore + (int)aspetto.spostamentoX,ALTEZZA / 2 + (int)dragY.valore, aspetto);
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
  programmaProssimaDiva(millis());
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
    fotogrammi = 0;
    ultimaStampa = millis();
  }

  delay(1);
}
