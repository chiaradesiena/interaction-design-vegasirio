// =====================================================================
//  DISEGNO STELLA — disegna la stella halftone (griglia di puntini) leggendo "aspetto".
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
  const int passo = a.passoGriglia > 0 ? a.passoGriglia : STELLA_PASSO_GRIGLIA;   // le stelle piccole usano puntini più fitti
  const float raggioMax = passo / 2.0f;
  if (!isfinite(a.raggio) || a.raggio <= 0) return;  // grandezza sbagliata (es. molla impazzita): meglio non disegnare che bloccarsi
  int n = (int)(a.raggio * max(STELLA_ALONE, 1.0f) / passo) + 1;
  int nMax = (LARGHEZZA + ALTEZZA) / passo + 1;       // oltre questo i puntini sarebbero comunque fuori dallo schermo
  if (n > nMax) n = nMax;
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
