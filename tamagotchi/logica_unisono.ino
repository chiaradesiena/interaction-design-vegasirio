// =====================================================================
//  LOGICA UNISONO — quando la radio trova una stella vicina (o arriva una stella finta di prova):
//  attesa di UNISONO_ATTESA_S per altre stelle, poi la galassia per UNISONO_DURATA_S,
//  poi socialità al 100% e pausa di UNISONO_PAUSA_DOPO_S. Legge stelleVicine e bisogni;
//  scrive galassia, aspettoUnisono, aspettiGalassia e le posizioni. Non disegna.
// =====================================================================

// La stella può entrare in Unisono (e lo dice alle altre nel messaggio radio):
// è in salute (o invitata dopo l'esplosione), la pausa è finita e sta nella schermata normale, senza menu né festa.
// Durante l'attesa resta disponibile, così una stella che arriva adesso può unirsi.
bool stellaDisponibile(uint32_t ora) {
  if (unisonoFase == UNISONO_ATTESA) return true;
  if (unisonoFase == UNISONO_ANIMAZIONE) return false;
  if (bisogni.salute < UNISONO_SALUTE_MINIMA && !unisonoInvitata) return false;
  if (unisonoFineMs != 0 && ora - unisonoFineMs < (uint32_t)(UNISONO_PAUSA_DOPO_S * 1000)) return false;
  return schermataCorrente == HOME && !menuAperto && !festaAttiva;
}

// Le stelle finte di prova sono "vicine" dopo UNISONO_FINTE_PRIMA_S secondi dall'accensione
bool finteVicine(uint32_t ora) {
  return UNISONO_STELLE_FINTE > 0 && ora >= (uint32_t)(UNISONO_FINTE_PRIMA_S * 1000);
}

// C'è almeno una stella vera vicina (confermata) e disponibile?
bool stellaVeraPronta() {
  bool pronta = false;
  portENTER_CRITICAL(&lucchettoVicine);
  for (int i = 0; i < numeroStelleVicine; i++)
    if (stellaVicinaConfermata(stelleVicine[i]) && stelleVicine[i].disponibile) pronta = true;
  portEXIT_CRITICAL(&lucchettoVicine);
  return pronta;
}

// Ogni fotogramma nella schermata normale: se la stella è disponibile e ne trova un'altra, parte l'attesa
void controllaIncontroUnisono(uint32_t ora) {
  if (unisonoFase != UNISONO_NO || !stellaDisponibile(ora)) return;
  if (stellaVeraPronta() || finteVicine(ora)) iniziaAttesaUnisono(ora);
}

void iniziaAttesaUnisono(uint32_t ora) {
  unisonoFase = UNISONO_ATTESA;
  unisonoInizioMs = ora;
  numeroGalassia = 0;
  azzeraOrchestra();
  schermataCorrente = UNISONO;
  Serial.println("unisono: incontrata una stella, attesa");
}

// Aggiunge alla galassia una stella (se c'è ancora posto); parte dal punto del cerchio dove poi starà
void aggiungiStellaGalassia(const uint8_t* mac, int indiceFinta, const char* nome, uint16_t colore, uint8_t punte,
                            float vita, float affilatura, const char* ritmo, uint32_t ora) {
  bool finta = indiceFinta >= 0;
  if (numeroGalassia >= UNISONO_MAX_STELLE + UNISONO_MAX_FINTE) return;
  StellaGalassia& s = galassia[numeroGalassia];
  memcpy(s.mac, mac, 6);
  s.finta = finta;
  strncpy(s.nome, nome, sizeof(s.nome) - 1);
  s.nome[sizeof(s.nome) - 1] = 0;
  s.colore = colore;
  s.punte = punte;
  s.vita = vita;
  s.affilatura = affilatura;
  s.arrivoMs = ora;
  s.posto = 1;                                   // arriva in fondo al braccio
  s.luce = 0;
  s.indiceFinta = indiceFinta;
  s.notaCorrente = 0;
  copiaRitmo(s.ritmo, ritmo);
  numeroGalassia++;
  Serial.printf("unisono: si aggiunge %s (%d stelle)\n", s.nome, numeroGalassia + 1);
}

// Durante l'attesa: le stelle vere confermate e disponibili che non sono ancora nella galassia si aggiungono
void aggiungiStelleVere(uint32_t ora) {
  StellaVicina copia[UNISONO_MAX_STELLE];
  portENTER_CRITICAL(&lucchettoVicine);
  int n = numeroStelleVicine;
  memcpy(copia, stelleVicine, sizeof(copia));
  portEXIT_CRITICAL(&lucchettoVicine);
  for (int i = 0; i < n; i++) {
    const StellaVicina& v = copia[i];
    if (!stellaVicinaConfermata(v) || !v.disponibile) continue;
    bool giaDentro = false;
    for (int j = 0; j < numeroGalassia; j++)
      if (!galassia[j].finta && memcmp(galassia[j].mac, v.mac, 6) == 0) giaDentro = true;
    if (!giaDentro) aggiungiStellaGalassia(v.mac, -1, v.nome, v.colore, v.punte, v.vita, v.affilatura, v.ritmo, ora);
  }
}

// Durante l'attesa: le stelle finte arrivano una alla volta, a UNISONO_FINTE_INTERVALLO_S secondi l'una dall'altra
void aggiungiStelleFinte(uint32_t ora) {
  if (!finteVicine(ora)) return;
  int finte = 0;
  for (int j = 0; j < numeroGalassia; j++) if (galassia[j].finta) finte++;
  int attese = 1 + (int)((ora - unisonoInizioMs) / (UNISONO_FINTE_INTERVALLO_S * 1000));
  int massimo = min(UNISONO_STELLE_FINTE, UNISONO_MAX_FINTE);
  if (attese > massimo) attese = massimo;
  const uint8_t nessunMac[6] = { 0, 0, 0, 0, 0, 0 };
  while (finte < attese) {
    const uint8_t* c = UNISONO_FINTE_COLORI[finte];
    aggiungiStellaGalassia(nessunMac, finte, UNISONO_FINTE_NOMI[finte], rgb(c[0], c[1], c[2]), UNISONO_FINTE_PUNTE[finte],
                           UNISONO_FINTE_VITA[finte], UNISONO_FINTE_AFFILATURA[finte], UNISONO_FINTE_RITMO[finte], ora);
    finte++;
  }
}

// Fine di Unisono: socialità al 100%, si torna alla schermata normale e parte la pausa
void fineUnisono(uint32_t ora) {
  bisogni.socialita = BISOGNO_MAX;
  unisonoFase = UNISONO_NO;
  unisonoFineMs = ora == 0 ? 1 : ora;
  unisonoInvitata = false;
  ripristinaVolumiOrchestra();
  schermataCorrente = HOME;
  Serial.println("unisono: finito, socialita al 100%");
}

// Ogni fotogramma durante Unisono (al posto del solito aggiornaStato): cielo, colore, aspetto e galassia;
// i bisogni restano fermi e tocchi, voce e scosse non contano
void aggiornaUnisono(uint32_t ora, float dt) {
  aggiornaOrientamento(dt);
  aggiornaRespiroCielo(ora);
  aggiornaColoreSalute(ora);
  aggiornaAspettoStella(ora);
  unisonoAngolo = fmodf(unisonoAngolo + 2 * PI * dt / UNISONO_GIRO_S, 2 * PI);
  if (unisonoFase == UNISONO_ATTESA) {
    aggiungiStelleVere(ora);
    aggiungiStelleFinte(ora);
    if (ora - unisonoInizioMs >= (uint32_t)(UNISONO_ATTESA_S * 1000)) {
      unisonoFase = UNISONO_ANIMAZIONE;
      unisonoAnimazioneMs = ora;
      Serial.printf("unisono: galassia di %d stelle\n", numeroGalassia + 1);
    }
  } else if (unisonoFase == UNISONO_ANIMAZIONE && ora - unisonoAnimazioneMs >= (uint32_t)(UNISONO_DURATA_S * 1000)) {
    fineUnisono(ora);
    return;
  }
  calcolaGalassia(ora, dt);
  aggiornaLuceGalassia(ora, dt);
  aggiornaOrchestra(ora);
}

// Quanto le stelle stanno "partendo" alla fine: 0 = ferme al loro posto, 1 = ripartite
float partenzaGalassia(uint32_t ora) {
  if (unisonoFase != UNISONO_ANIMAZIONE) return 0.0f;
  float t = (ora - unisonoAnimazioneMs) / 1000.0f;
  return frazioneDolce(t, UNISONO_DURATA_S - UNISONO_ARRIVO_S, UNISONO_DURATA_S);
}

// Angolo (radianti, sullo schermo) del braccio numero "braccio" alla distanza r dal centro:
// più ci si allontana, più il braccio è girato. Così i puntini disegnano una spirale.
float angoloBraccio(int braccio, float r) {
  return unisonoAngolo + 2 * PI * braccio / UNISONO_BRACCI
         + UNISONO_AVVOLGIMENTO * (r - UNISONO_RAGGIO_INTERNO_PX) / (UNISONO_RAGGIO_STELLE_MAX_PX - UNISONO_RAGGIO_INTERNO_PX);
}

// Dove sta, sullo schermo, il punto del braccio a distanza r dal centro
void puntoBraccio(int braccio, float r, int& x, int& y) {
  float a = angoloBraccio(braccio, r);
  x = unisonoCentroX + (int)(r * cosf(a));
  y = unisonoCentroY + (int)(r * sinf(a));
}

// Un colore più chiaro: k = 0 colore pieno, k = 1 bianco
uint16_t schiarisciColore(uint16_t c, float k) {
  uint16_t r = (c >> 11) & 0x1F, g = (c >> 5) & 0x3F, b = c & 0x1F;
  r += (uint16_t)((0x1F - r) * k + 0.5f);
  g += (uint16_t)((0x3F - g) * k + 0.5f);
  b += (uint16_t)((0x1F - b) * k + 0.5f);
  return (r << 11) | (g << 5) | b;
}

// Il braccio di luce gira; quando passa sopra una stella la illumina (che poi si spegne piano).
// Solo durante la galassia: nell'attesa è spento.
void aggiornaLuceGalassia(uint32_t ora, float dt) {
  float prima = luceAngolo;
  luceAngolo = angoloLuceComune();                                  // un giro per battuta, a tempo col direttore
  float giro = fmodf(luceAngolo - prima + 4 * PI, 2 * PI);          // quanto ha girato in questo fotogramma
  luceMia -= dt / UNISONO_LUCE_SPEGNIMENTO_S;
  if (luceMia < 0) luceMia = 0;
  luceVisibile = unisonoFase == UNISONO_ANIMAZIONE
                 ? frazioneDolce((ora - unisonoAnimazioneMs) / 1000.0f, 0, UNISONO_ARRIVO_S) * (1.0f - partenzaGalassia(ora))
                 : 0.0f;
  for (int i = 0; i < numeroGalassia; i++) {
    StellaGalassia& s = galassia[i];
    s.luce -= dt / UNISONO_LUCE_SPEGNIMENTO_S;
    if (s.luce < 0) s.luce = 0;
    if (!UNISONO_LUCE_ACCENDE_STELLE || luceVisibile < 0.5f) continue;
    float angoloStella = atan2f(galassiaY[i] - unisonoCentroY, galassiaX[i] - unisonoCentroX);
    float distanza = fmodf(angoloStella - prima + 4 * PI, 2 * PI);   // quanto manca alla luce per arrivare alla stella
    if (distanza < giro) illuminaStellaGalassia(i);
  }
}

// Accende una stella della galassia (la usa il braccio di luce; più avanti anche le note dell'orchestra)
void illuminaStellaGalassia(int i) {
  if (i >= 0 && i < numeroGalassia) galassia[i].luce = 1;
}

// Calcola cosa si vede: la propria stella si rimpicciolisce al centro, i bracci si accendono,
// le altre stelle arrivano scivolando lungo i bracci e girano piano; alla fine ripartono e la propria stella torna grande
void calcolaGalassia(uint32_t ora, float dt) {
  float partenza = partenzaGalassia(ora);
  float stringi = frazioneDolce((ora - unisonoInizioMs) / 1000.0f, 0, UNISONO_ARRIVO_S) * (1.0f - partenza);
  float scala = 1.0f + (UNISONO_GRANDEZZA_PROPRIA - 1.0f) * stringi;
  aspettoUnisono = aspetto;
  aspettoUnisono.raggio = aspetto.raggio * scala * (1.0f + UNISONO_LUCE_CRESCITA * luceMia);   // quando suona si gonfia
  aspettoUnisono.colore = schiarisciColore(aspetto.colore, UNISONO_LUCE_BIANCO * luceMia);    // e si schiarisce
  aspettoUnisono.spostamentoX = 0;
  aspettoUnisono.spostamentoY = 0;
  aspettoUnisono.passoGriglia = max(UNISONO_PASSO_MINIMO, (int)(STELLA_PASSO_GRIGLIA * scala + 0.5f));
  unisonoCentroX = LARGHEZZA / 2 + (int)((dragX.valore + aspetto.spostamentoX) * (1.0f - stringi));   // dal punto dov'era al centro
  unisonoCentroY = ALTEZZA / 2 + (int)((dragY.valore + aspetto.spostamentoY) * (1.0f - stringi));

  bracciVisibili = stringi;

  float k = UNISONO_VELOCITA_POSTO * dt;
  if (k > 1.0f) k = 1.0f;
  for (int i = 0; i < numeroGalassia; i++) {
    StellaGalassia& s = galassia[i];
    int braccio = i % UNISONO_BRACCI;
    int sulBraccio = (numeroGalassia - 1 - braccio) / UNISONO_BRACCI + 1;   // quante stelle stanno su questo braccio
    float postoGiusto = (float)(i / UNISONO_BRACCI + 1) / sulBraccio;      // distribuite lungo il braccio, l'ultima in fondo
    s.posto += (postoGiusto - s.posto) * k;                                // si sposta piano verso il suo posto
    float rPosto = UNISONO_RAGGIO_INTERNO_PX + (UNISONO_RAGGIO_STELLE_MAX_PX - UNISONO_RAGGIO_INTERNO_PX) * s.posto;
    float arrivo = frazioneDolce((ora - s.arrivoMs) / 1000.0f, 0, UNISONO_ARRIVO_S) * (1.0f - partenza);
    float r = UNISONO_DISTANZA_ARRIVO_PX + (rPosto - UNISONO_DISTANZA_ARRIVO_PX) * arrivo;   // scivola lungo il braccio
    puntoBraccio(braccio, r, galassiaX[i], galassiaY[i]);
    AspettoStella& a = aspettiGalassia[i];
    a = aspettoUnisono;
    a.colore = schiarisciColore(s.colore, UNISONO_LUCE_BIANCO * s.luce);
    a.raggio = STELLA_RAGGIO * UNISONO_GRANDEZZA_ALTRE * (1.0f + UNISONO_LUCE_CRESCITA * s.luce);
    a.numeroPunte = s.punte;
    a.vita = s.vita;
    a.affilatura = s.affilatura;
    a.buco = 0;
    a.luminosita = 1;
    a.passoGriglia = UNISONO_PASSO_GRIGLIA;
  }
}

// Nomi delle fasi per il Monitor seriale
const char* nomeFaseUnisono() {
  if (unisonoFase == UNISONO_ATTESA) return "attesa";
  if (unisonoFase == UNISONO_ANIMAZIONE) return "galassia";
  return "no";
}

// Secondi che mancano alla fine della pausa dopo Unisono (0 = finita), per il Monitor seriale
int secondiPausaUnisono(uint32_t ora) {
  if (unisonoFineMs == 0) return 0;
  uint32_t passati = (ora - unisonoFineMs) / 1000;
  return passati >= (uint32_t)UNISONO_PAUSA_DOPO_S ? 0 : (int)(UNISONO_PAUSA_DOPO_S - passati);
}
