// =====================================================================
//  LOGICA PAPARAZZI — stato proprio di Vega: con salute alta, ogni tanto e da sola, le stelle di sfondo
//  girano veloci e diventano bianche, il fondo diventa bianco e poi rosa; alla fine tutto al contrario
//  fino al cielo normale. Si sceglie con STATO_PROPRIO_TIPO (config_vega.h).
//  Usa caneLibero() e bisognoMigliora() di logica_cane.ino (stesse regole del cane di Sirio).
//  Legge bisogni e schermata; scrive coloreFondo, cieloFrazione, i colori del cielo e l'orientamento (il giro).
// =====================================================================

// Mescola due colori: k = 0 solo il primo, k = 1 solo il secondo, in mezzo una sfumatura
uint16_t mescolaColori(uint16_t a, uint16_t b, float k) {
  int ra = (a >> 11) & 0x1F, ga = (a >> 5) & 0x3F, ba = a & 0x1F;
  int rb = (b >> 11) & 0x1F, gb = (b >> 5) & 0x3F, bb = b & 0x1F;
  uint16_t r = (uint16_t)(ra + (rb - ra) * k + 0.5f);
  uint16_t g = (uint16_t)(ga + (gb - ga) * k + 0.5f);
  uint16_t bl = (uint16_t)(ba + (bb - ba) * k + 0.5f);
  return (r << 11) | (g << 5) | bl;
}

// Sceglie a caso fra quanti secondi arrivano i prossimi paparazzi (fra VEGA_PAPARAZZI_ATTESA_MIN_S e _MAX_S)
void programmaProssimiPaparazzi(uint32_t ora) {
  float attesaS = VEGA_PAPARAZZI_ATTESA_MIN_S + casuale01() * (VEGA_PAPARAZZI_ATTESA_MAX_S - VEGA_PAPARAZZI_ATTESA_MIN_S);
  prossimiPaparazziMs = ora + (uint32_t)(attesaS * 1000);
}

void iniziaPaparazzi(uint32_t ora) {
  paparazziAttivi = true;
  paparazziInRitorno = false;
  paparazziInizioMs = ora;
  Serial.println("paparazzi: inizio");
}

// Quanti secondi si è lontani dal bordo dell'animazione (dall'inizio o dalla fine, il più vicino dei due).
// Le transizioni dipendono solo da questo numero: così la fine è l'inizio visto al contrario.
// Durante il ritorno veloce (dopo un'interruzione) scende fino a 0 in VEGA_PAPARAZZI_RITORNO_S secondi.
float bordoPaparazzi(uint32_t ora) {
  const float transizione = VEGA_PAPARAZZI_GIRO_S + VEGA_PAPARAZZI_BIANCO_S + VEGA_PAPARAZZI_ROSA_S;
  if (paparazziInRitorno) {
    float f = (ora - paparazziRitornoMs) / 1000.0f / VEGA_PAPARAZZI_RITORNO_S;
    return paparazziBordoRitorno * max(0.0f, 1.0f - f);
  }
  float t = (ora - paparazziInizioMs) / 1000.0f;
  return max(0.0f, min(min(t, VEGA_PAPARAZZI_DURATA_S - t), transizione));
}

// Interruzione: da dove si trova torna indietro in fretta (rosa -> bianco -> stelle che rallentano)
void interrompiPaparazzi(uint32_t ora) {
  paparazziBordoRitorno = bordoPaparazzi(ora);
  paparazziInRitorno = true;
  paparazziRitornoMs = ora;
  Serial.println("paparazzi: interrotti");
}

void finePaparazzi(uint32_t ora) {
  paparazziAttivi = false;
  paparazziInRitorno = false;
  coloreFondo = NERO;
  cieloFrazione = 1.0f;
  programmaProssimiPaparazzi(ora);
  Serial.println("paparazzi: fine");
}

// Vero quando l'animazione (o il ritorno veloce) è arrivata in fondo
bool paparazziFiniti(uint32_t ora) {
  if (paparazziInRitorno) return ora - paparazziRitornoMs >= (uint32_t)(VEGA_PAPARAZZI_RITORNO_S * 1000);
  return ora - paparazziInizioMs >= (uint32_t)(VEGA_PAPARAZZI_DURATA_S * 1000);
}

// Fa girare le stelle di sfondo attorno al centro dello schermo. Si aggiunge al giro del giroscopio,
// quindi le stelle lasciano anche la scia (motion blur) come quando si gira la board.
void giraCieloPaparazzi(float gradi) {
  float a = gradi * GRADI_A_RADIANTI;
  float co = cosf(a), si = sinf(a);
  for (int j = 0; j < 3; j++) {
    float x = orientamento[0][j], y = orientamento[1][j];
    orientamento[0][j] = co * x - si * y;
    orientamento[1][j] = si * x + co * y;
  }
  raddrizzaOrientamento();
}

// Le stelle di sfondo (e le loro scie) diventano bianche: k = 0 colore normale, k = 1 tutte bianche.
// Va chiamata dopo aggiornaRespiroCielo, che a ogni fotogramma rimette i colori normali.
void sbiancaCielo(float k) {
  if (k <= 0) return;
  for (int i = 0; i < PUNTI_SULLA_SFERA; i++) {
    cielo[i].colore = mescolaColori(cielo[i].colore, BIANCO, k);
    cielo[i].coloreScia = mescolaColori(cielo[i].coloreScia, BIANCO, k);
  }
}

// Cosa si vede adesso: prima le stelle accelerano e sbiancano, poi il fondo va da nero a bianco
// (le stelle ci si sciolgono dentro), poi da bianco a rosa. Il cielo non si disegna quando il fondo è tutto bianco o rosa.
void calcolaFasePaparazzi(uint32_t ora, float dt) {
  float d = bordoPaparazzi(ora);
  const float fineGiro = VEGA_PAPARAZZI_GIRO_S;
  const float fineBianco = fineGiro + VEGA_PAPARAZZI_BIANCO_S;
  const float fineRosa = fineBianco + VEGA_PAPARAZZI_ROSA_S;
  float giro = frazioneDolce(d, 0.0f, fineGiro);
  float bianco = frazioneDolce(d, fineGiro, fineBianco);
  float rosa = frazioneDolce(d, fineBianco, fineRosa);
  giraCieloPaparazzi(VEGA_PAPARAZZI_GIRO_CIELO_GRADI_S * giro * dt);
  sbiancaCielo(frazioneDolce(d, 0.0f, fineBianco));
  coloreFondo = mescolaColori(mescolaColori(NERO, BIANCO, bianco), VEGA_PAPARAZZI_ROSA, rosa);
  cieloFrazione = bianco >= 1.0f ? 0.0f : 1.0f;
}

// Ogni fotogramma: controlla se si è nello stato proprio, fa arrivare i paparazzi quando è il momento
// e li manda via se qualcuno migliora un bisogno della stella (carezza o scossa abbastanza lunghe).
void aggiornaPaparazzi(uint32_t ora, float dt) {
  if (STATO_PROPRIO_TIPO != PROPRIO_PAPARAZZI) return;
  bool proprio = bisogni.salute >= STATO_PROPRIO_SOGLIA_SALUTE;
  if (proprio && !statoProprio) programmaProssimiPaparazzi(ora);   // appena entrata nello stato proprio: parte l'attesa
  statoProprio = proprio;
  if (schermataCorrente != HOME && schermataCorrente != MENU) {   // è appena partita la costellazione o Unisono: i paparazzi lasciano subito il posto
    if (paparazziAttivi) finePaparazzi(ora);
    return;
  }
  if (!paparazziAttivi) {
    if (statoProprio && (int32_t)(ora - prossimiPaparazziMs) >= 0 && caneLibero()) iniziaPaparazzi(ora);
  } else if (!paparazziInRitorno && bisognoMigliora()) {
    interrompiPaparazzi(ora);
  }
  if (!paparazziAttivi) return;
  if (paparazziFiniti(ora)) { finePaparazzi(ora); return; }
  calcolaFasePaparazzi(ora, dt);
}
