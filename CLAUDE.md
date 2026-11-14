# CLAUDE.md — Tamagotchi Stella (M5Stack CoreS3)

Progetto di gruppo per il corso di Interaction Design. Utente: **Chiara**, programmatrice alle prime armi.
Tutto il codice lo scrive Claude; Chiara lo carica sulla board e lo tara.

## Obiettivo
Un Tamagotchi **senza elementi iconografici e senza caratteristiche umanoidi**: la creatura è una **stella astratta** (resa "halftone", fatta di puntini su griglia) in uno spazio di punti che ruota col giroscopio. Lo stato interno si comunica solo con forma, colore, ritmo, movimento, luce (e suono), mai con facce, occhi, barre-icona o simboli.

## Hardware e toolchain
- Scheda **M5Stack CoreS3**, schermo 320×240, programmata da **Arduino IDE** (scheda "M5CoreS3", librerie M5CoreS3 + M5Unified + M5GFX). Monitor seriale a 115200 baud.
- Sensori/uscite disponibili: IMU (accelerometro, giroscopio, bussola), luce ambiente e prossimità (LTR553), microfoni stereo, touch, altoparlante, camera, batteria/RTC, Wi-Fi/BLE/**ESP-NOW** (la radio prevista per l'interazione fra più dispositivi), microSD.
- Non posso compilare né provare sulla board: scrivo codice che deve compilare al primo colpo in Arduino IDE e dico a Chiara cosa controllare sul dispositivo.

## File in questa cartella
| File | Ruolo |
|---|---|
| `stella.ino` | **Riferimento di stile** e base attuale: schermata HOME (cielo + stella). |
| `EsploratoreCoreS3.ino` | Sketch di partenza del professore: una pagina per ogni sensore. **Consultalo** per la sintassi corretta delle API (`CoreS3.Imu`, `.Touch`, `.Mic`, `.Ltr553`, `.Speaker`, ESP-NOW…). Non modificarlo. |
| `Automa a stati finiti_stella.pdf` | Specifica dell'automa (vedi sotto). |
| `moodboard.pdf` | Direzione estetica: stelle a scintilla/punte curve, halftone, puntini su griglia, scie, cielo nero con punti, orbite sottili. |

Attenzione: Arduino IDE vuole **una cartella per sketch, con lo stesso nome del `.ino` principale**, e compila insieme tutti i `.ino` della cartella. Due sketch indipendenti non possono convivere nella stessa cartella.

## Automa a stati (dal PDF)
Statistiche (0–100): `nutrimento (n)`, `socialità (s)`, `energia (e)`, `umore (u)`, `salute`.
- **IN SALUTE**
  - Asse attiva: `ATTIVA` ⇄ `SOLITARIA` (s<50 / s>80), `ATTIVA` ⇄ `SCARICA` (n<50 / n>50), `ATTIVA` ⇄ `STATICA` (e<20 / e>50).
  - Dopo `ATTIVA` si va a `NOTTE` o `GIORNO`. `NOTTE` ⇄ `BRILLARE` (u>80 / u<40). `GIORNO` ⇄ `RICARICA` (u<40 / n>80).
- **IN CRESCITA**: da `NOTTE` con salute=100 → `COSTELLAZIONE`; ritorna dopo "x" secondi.
- **IN DECADENZA**: salute<20 → `MALATA` (torna in salute con salute>60); salute=0 → `ESPLOSIONE` → (invita) `UNISONO` → dopo "x" secondi (o "invita") torna a `RICARICA`.
- Colore della stella in base alla salute: 20% rosa-rosso, 40% arancio, 60% giallo, 80% viola, 100% azzurro chiaro.
- Il PDF è ambiguo su alcune soglie e sui trigger (`invita`): **chiedere a Chiara prima di implementare lo stato interessato**, non inventare.

## Flusso di lavoro (OBBLIGATORIO)
Quando Chiara chiede di programmare qualcosa:
1. **Non scrivere codice subito.** Presenta un **piano a fasi** (breve, numerato): cosa si fa in ogni fase, quali moduli/file si toccano o si creano, cosa Chiara vedrà sul dispositivo a fine fase, eventuali domande aperte.
2. **Aspetta l'approvazione esplicita.** Implementa solo le fasi approvate, una alla volta se così richiesto.
3. Dopo l'implementazione dai la **spiegazione semplice** e l'**elenco costanti** (formato sotto).
4. Se mancano informazioni, fai domande prima del piano. Non fare ipotesi su design, soglie o comportamento della creatura.

Eccezione: correzioni minime di bug già concordate non richiedono un piano completo, basta dire cosa si cambia.

## Formato della risposta dopo ogni implementazione
- **Spiegazione a grandi linee**, in italiano semplice, senza gergo: cosa fa il codice, in 3–8 righe. Se serve un termine tecnico, spiegarlo in una frase (come fanno i commenti di `stella.ino`).
- **Costanti da cambiare**: tabella con nome, valore attuale, cosa succede se si alza/abbassa, modulo/file dove si trova. Sono solo quelle che Chiara ha senso tarare.
- **Cosa controllare sulla board** (cosa dovrebbe vedere; cosa fare se un movimento va al contrario, es. cambiare `SEGNO_ASSE_*`).
- Evidenziare i file modificati e quelli nuovi.

## Regole di modifica
- **Cambia meno codice pregresso possibile.** Aggiungi moduli/funzioni nuovi invece di riscrivere. Non rinominare, non riformattare, non "ripulire" codice che funziona. Modifiche a codice esistente solo dove necessario, e dichiaralo.
- Aggiungere una funzionalità = nuovo file/modulo + poche righe di collegamento (un `case`, una chiamata) nei punti di aggancio.
- Non aggiungere dipendenze o librerie nuove senza chiederlo.
- Mai tarare "a occhio" in silenzio: ogni numero tarabile va in una costante in configurazione.

## Architettura (modulare, pensata per essere estesa da un LLM)
Stesso schema di `stella.ino`: ogni giro del `loop` è **LEGGI → AGGIORNA → DISEGNA**.
- Solo il modulo **sensori** parla con l'hardware. La **logica** lavora su dati (`Sensi`, statistiche, stato) e non disegna. Il **disegno** disegna ciò che riceve e non legge mai i sensori.
- La logica scrive in `AspettoStella` (colore, raggio, punte, vita, affilatura, rotazione, ecc.); il disegno lo legge. **Ogni nuovo stato dell'automa cambia solo questi numeri** (e al massimo aggiunge campi). È il punto di estensione principale.
- Una schermata = una funzione `schermataXxx(M5Canvas&)` + una voce nell'`enum Schermata` e nello `switch` del `loop`.

Struttura target (da confermare nella prima fase di piano; si migra da `stella.ino` spostando il codice **senza cambiarne il contenuto**):
```
tamagotchi/                 <- cartella = nome del .ino principale
  tamagotchi.ino            setup() e loop() soltanto, nient'altro
  tipi.h                    struct ed enum (prima di tutto)
  config.h                  TUTTE le costanti tarabili, raggruppate per tema
  colori.h                  rgb() e palette
  sensori.ino               leggi sensori, calibrazione (IMU, luce, mic, touch…)
  logica_*.ino              un file per tema: statistiche, automa, molle, orientamento
  disegno_*.ino             un file per tema: cielo, stella, effetti
  schermate.ino             le schermate e lo switch
```
Regole Arduino IDE da rispettare:
- I `.ino` della cartella sono uniti **in ordine alfabetico**; non dipendere dall'ordine: i tipi stanno in `.h` inclusi esplicitamente.
- Struct/enum **mai dopo** la prima funzione (altrimenti "does not name a type"), come spiegato in `stella.ino`.
- `#include <SD.h>` va prima di `M5CoreS3.h` se si usa la SD.
- Ogni modulo ha in testa un commento di 2–4 righe: cosa fa, cosa legge, cosa scrive.

## Stile di programmazione (dedotto da `stella.ino`)
- **Tutto in italiano**: nomi di variabili, funzioni, costanti, struct, commenti, messaggi seriali (senza accenti nelle stringhe mostrate sullo schermo). Eccezioni: parole delle API (`M5Canvas`, `Serial`, `loop`…).
- Nomi: funzioni e variabili in `camelCase` descrittivo (`aggiornaMolla`, `leggiSensori`, `disegnaCielo`); tipi in `PascalCase` (`PuntoCielo`, `AspettoStella`); **costanti in `MAIUSCOLO_CON_UNDERSCORE`**, nome parlante e con unità o prefisso di tema (`DRAG_FREQUENZA`, `STELLA_RAGGIO`, `PULSAZIONE_PERIODO_MS`, `ZONA_MORTA_GRADI_S`). Colori come costanti (`ROSSO_STELLA`, `NERO`) tramite `rgb(r,g,b)` RGB565.
- Niente "numeri magici" nel codice: ogni valore da tarare è una `const` nella sezione CONFIGURAZIONE, con un **commento a destra** che dice cosa fa e cosa succede se sale/scende (es. `// più alto = ...`), con unità (`px`, `ms`, `gradi al secondo`) e intervallo `(0-255)`.
- Le costanti in config sono raggruppate per tema con una riga di intestazione-commento.
- Struct piccole e semplici (`Sensi`, `Molla`, `AspettoStella`) per passare dati fra moduli, passate per riferimento (`const T&` se sola lettura).
- Funzioni **corte, con un solo compito**, con un commento di una-due righe sopra che spiega il *perché* in linguaggio semplice (non il *come* riga per riga).
- Il file è diviso in sezioni numerate con banner `// ===== N. TITOLO =====`: tipi, configurazione, colori, dati, sensori, logica, disegno, schermate, setup/loop. Mantenere questo ordine e questa grafica.
- Tempo: si usa `millis()` e `dt` in secondi (con tetto 0.1 s), mai `delay()` lunghi nel loop (solo `delay(1)` a fine loop; `delay` ammesso solo in calibrazioni di setup).
- Disegno: tutto su un `M5Canvas tela` fuori schermo, poi un solo `pushSprite(0,0)` (niente sfarfallio). Fallback a 8 bit se manca memoria.
- Float espliciti (`1.0f`) nei calcoli dentro le funzioni; `constexpr`/`const` per le costanti; niente `#define` (tranne dove l'esploratore li usa per macro).
- Numeri casuali ripetibili (generatore con seme, `SEME_CIELO`): stesso seme = stesso risultato.
- Se un sensore non risponde, il codice **non si blocca**: valori a zero e prosegue (`imuOk`).
- Diagnostica: una riga al secondo sul Monitor seriale (FPS e valori chiave), intervallo in costante.
- Prestazioni: puntare ai ~30+ FPS; evitare allocazioni dinamiche (`String`, `new`) nel loop di disegno; array a dimensione fissa da costante.

## Estetica (vincoli di design)
- Nero profondo, punti bianchi/grigi, stella halftone con colore dello stato; scie e pulsazioni lente; movimento elastico (molle) invece di scatti.
- Vietati: facce, occhi, bocche, braccia, cuori, icone, emoji, barre di stato con simboli. Lo stato si legge da colore, numero di punte, forma, pulsazione, rotazione, densità/luce dei punti, drag.
- Stile della moodboard: scintille a quattro/otto punte, halftone, griglie di puntini, orbite sottili, linee d'aria.

## Domande aperte (da chiarire con Chiara prima di implementare)
- Come variano le statistiche nel tempo (decadimento) e con quali azioni/sensori crescono (tocco, luce, microfono, scossa, vicinanza di un altro dispositivo via ESP-NOW)?
- Valori esatti di soglie e durate ("x" secondi), significato preciso di `invita`, `UNISONO`, `COSTELLAZIONE`.
- Uso dell'audio (suoni della creatura?) e persistenza (salvataggio dello stato dopo lo spegnimento?).
