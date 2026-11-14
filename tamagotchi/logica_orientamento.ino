// =====================================================================
//  LOGICA ORIENTAMENTO — come è girato il dispositivo: cielo che ruota col giroscopio,
//  molle, drag della stella e respiro dei punti. Legge "sensi", scrive orientamento e drag.
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

// Se il dispositivo gira di un certo angolo, il cielo (visto da dentro)
// gira dello stesso angolo al contrario: effetto "finestra sullo spazio".
// Il cielo usa la velocità "elastica": parte un po' in ritardo e, quando ti
// fermi, va leggermente oltre e torna indietro (solo se CIELO_RIMBALZO è true).
void aggiornaOrientamento(float dt) {
  memcpy(orientamentoPrecedente, orientamento, sizeof(orientamento));

  // giroscopio fermo: il cielo non si muove affatto (si azzera anche l'elastico)
  if (!CIELO_RIMBALZO && sensi.giroX == 0 && sensi.giroY == 0 && sensi.giroZ == 0) {
    elasticoX = {0, 0}; elasticoY = {0, 0}; elasticoZ = {0, 0};
    return;
  }

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
