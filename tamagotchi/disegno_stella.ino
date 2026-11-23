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
  if (stileStella == STILE_RAGGI && a.cane <= 0 && a.passoGriglia <= 0) { disegnaStellaRaggi(c, cx, cy, a); return; }   // versione a raggi (disegno_raggi.ino), solo la stella grande
  if (a.luminosita <= 0) return;                      // fase "spenta" del lampeggio: la stella non si vede
  if (!isfinite(a.raggio) || a.raggio <= 0) return;  // grandezza sbagliata (es. molla impazzita): meglio non disegnare che bloccarsi
  bool grigliaCane = a.cane >= SIRIO_CANE_SOGLIA_GRIGLIA;   // Sirio che diventa cane: puntini più fitti (disegno_cane.ino)
  if (a.cane > 0) preparaCane(a);
  const int passoPrincipale = stileStella == STILE_FITTO ? FITTO_PASSO_GRIGLIA : STELLA_PASSO_GRIGLIA;   // versione a puntini fitti (swipe verso l'alto, logica_stile.ino)
  const int passoStella = a.passoGriglia > 0 ? a.passoGriglia : passoPrincipale;   // le stelle piccole usano puntini più fitti
  const int passo = grigliaCane ? SIRIO_CANE_PASSO_GRIGLIA : passoStella;
  const float raggioMax = grigliaCane ? SIRIO_CANE_PUNTINO_MAX_PX : passo / 2.0f;
  int n = (int)(a.raggio * max(STELLA_ALONE, 1.0f) / passo) + 1;
  int nMax = (LARGHEZZA + ALTEZZA) / passo + 1;       // oltre questo i puntini sarebbero comunque fuori dallo schermo
  if (n > nMax) n = nMax;
  for (int i = -n; i <= n; i++) {
    for (int j = -n; j <= n; j++) {
      float bucoPx = a.buco * a.raggio;                // il buco lascia vedere il cielo: qui non si disegna niente
      if (bucoPx > 0 && (float)(i * i + j * j) * passo * passo < bucoPx * bucoPx) continue;
      float densita = densitaStellaOCane(i * passo, j * passo, a);
      int x = cx + i * passo, y = cy - j * passo;
      if (grigliaCane) { disegnaPuntinoCane(c, x, y, passo, densita, a.colore, a.cane >= 1); continue; }   // cane: puntini tutti uguali
      float rf = raggioMax * densita;
      if (rf < 0.25f) continue;                       // troppo piccolo: niente puntino
      c.fillRect(x - passo / 2, y - passo / 2, passo, passo, coloreFondo);   // copre il cielo dietro la stella (col colore dello sfondo: nero, o rosso da malata)
      c.fillCircle(x, y, (int)(rf + 0.5f), a.colore); // raggio 0 = un solo pixel
    }
  }
}
