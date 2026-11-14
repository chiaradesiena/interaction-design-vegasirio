---
paths:
  - "**/*.ino"
  - "**/*.h"
---

\# Codice: hardware, architettura, stile



\## Hardware e toolchain

\- Scheda \*\*M5Stack CoreS3\*\*, schermo 320×240, programmata da \*\*Arduino IDE\*\* (scheda "M5CoreS3", librerie M5CoreS3 + M5Unified + M5GFX). Monitor seriale a 115200 baud.

\- Sensori/uscite disponibili: IMU (accelerometro, giroscopio, bussola), luce ambiente e prossimità (LTR553), microfoni stereo, touch, altoparlante, camera, batteria/RTC, Wi-Fi/BLE/\*\*ESP-NOW\*\* (la radio prevista per l'interazione fra più dispositivi), microSD.

\- Non posso provare sulla board (lo fa Chiara: le dico cosa controllare), ma \*\*devo compilare\*\* prima di consegnare il codice, con l'`arduino-cli` che sta dentro Arduino IDE:

&#x20; - Windows: `%LOCALAPPDATA%\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe`; macOS: `/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli`.

&#x20; - Comando: `arduino-cli compile --fqbn m5stack:esp32:m5stack_cores3 --build-path <cartella temporanea fuori dalla repository> tamagotchi`.

&#x20; - Compila \*\*tutte e due le stelle\*\*: Vega su una copia temporanea della cartella con `personaggio.h` cambiato, senza toccare l'originale.

&#x20; - Se la compilazione fallisce, correggi prima di rispondere. Se `arduino-cli` o la scheda M5Stack non sono installati, dillo e scrivi codice che compili al primo colpo.



Attenzione: Arduino IDE vuole \*\*una cartella per sketch, con lo stesso nome del `.ino` principale\*\*, e compila insieme tutti i `.ino` della cartella. Due sketch indipendenti non possono convivere nella stessa cartella.



\## Architettura (modulare, pensata per essere estesa da un LLM)

Stesso schema di `tamagotchi/`: ogni giro del `loop` è \*\*LEGGI → AGGIORNA → DISEGNA\*\*.

\- Solo il modulo \*\*sensori\*\* parla con l'hardware. La \*\*logica\*\* lavora su dati (`Sensi`, statistiche, stato) e non disegna. Il \*\*disegno\*\* disegna ciò che riceve e non legge mai i sensori.

\- La logica scrive in `AspettoStella` (colore, raggio, punte, vita, affilatura, rotazione, ecc.); il disegno lo legge. \*\*Ogni nuovo stato dell'automa cambia solo questi numeri\*\* (e al massimo aggiunge campi). È il punto di estensione principale.

\- Una schermata = una funzione `schermataXxx(M5Canvas\&)` + una voce nell'`enum Schermata` e nello `switch` del `loop`.



Struttura attuale (lo sketch unico, nato da `Sirio.ino` e `Vega.ino` spostando il codice **senza cambiarne il contenuto**):

```

tamagotchi/                 <- cartella = nome del .ino principale

&#x20; tamagotchi.ino            setup() e loop() soltanto, nient'altro

&#x20; tipi.h                    struct ed enum (prima di tutto)

&#x20; personaggio.h             UNA riga: quale stella caricare (PERSONAGGIO_SIRIO o PERSONAGGIO_VEGA)

&#x20; config.h                  sceglie config_sirio.h o config_vega.h (niente da tarare)

&#x20; config_comune.h           costanti tarabili uguali per le due stelle, raggruppate per tema

&#x20; config_sirio.h, config_vega.h   costanti e comportamenti (`*_TIPO`) propri di ciascuna stella

&#x20; colori.h                  rgb() e palette

&#x20; dati.h                    TUTTE le variabili globali (cosi ogni .ino le trova gia dichiarate)

&#x20; sensori.ino               leggi sensori, calibrazione (IMU, luce, mic, touch…)

&#x20; logica\_\*.ino              un file per tema: statistiche, automa, molle, orientamento

&#x20; disegno\_\*.ino             un file per tema: cielo, stella, effetti

&#x20; schermate.ino             le schermate e lo switch

```

Regole Arduino IDE da rispettare:

\- I `.ino` della cartella sono uniti \*\*in ordine alfabetico\*\*; non dipendere dall'ordine: i tipi stanno in `.h` inclusi esplicitamente.

\- Struct/enum \*\*mai dopo\*\* la prima funzione (altrimenti "does not name a type"), come spiegato in `tamagotchi/tipi.h`.

\- `#include <SD.h>` va prima di `M5CoreS3.h` se si usa la SD.

\- Ogni modulo ha in testa un commento di 2–4 righe: cosa fa, cosa legge, cosa scrive.



\## Stile di programmazione (dedotto da `tamagotchi/`)

\- \*\*Tutto in italiano\*\*: nomi di variabili, funzioni, costanti, struct, commenti, messaggi seriali (senza accenti nelle stringhe mostrate sullo schermo). Eccezioni: parole delle API (`M5Canvas`, `Serial`, `loop`…).

\- Nomi: funzioni e variabili in `camelCase` descrittivo (`aggiornaMolla`, `leggiSensori`, `disegnaCielo`); tipi in `PascalCase` (`PuntoCielo`, `AspettoStella`); \*\*costanti in `MAIUSCOLO\_CON\_UNDERSCORE`\*\*, nome parlante e con unità o prefisso di tema (`DRAG\_FREQUENZA`, `STELLA\_RAGGIO`, `PULSAZIONE\_PERIODO\_MS`, `ZONA\_MORTA\_GRADI\_S`). Colori come costanti (`ROSSO\_STELLA`, `NERO`) tramite `rgb(r,g,b)` RGB565.

\- Niente "numeri magici" nel codice: ogni valore da tarare è una `const` nella sezione CONFIGURAZIONE, con un \*\*commento a destra\*\* che dice cosa fa e cosa succede se sale/scende (es. `// più alto = ...`), con unità (`px`, `ms`, `gradi al secondo`) e intervallo `(0-255)`.

\- Le costanti in config sono raggruppate per tema con una riga di intestazione-commento.

\- Struct piccole e semplici (`Sensi`, `Molla`, `AspettoStella`) per passare dati fra moduli, passate per riferimento (`const T\&` se sola lettura).

\- Funzioni \*\*corte, con un solo compito\*\*, con un commento di una-due righe sopra che spiega il \*perché\* in linguaggio semplice (non il \*come\* riga per riga).

\- Il file è diviso in sezioni numerate con banner `// ===== N. TITOLO =====`: tipi, configurazione, colori, dati, sensori, logica, disegno, schermate, setup/loop. Mantenere questo ordine e questa grafica.

\- Tempo: si usa `millis()` e `dt` in secondi (con tetto 0.1 s), mai `delay()` lunghi nel loop (solo `delay(1)` a fine loop; `delay` ammesso solo in calibrazioni di setup).

\- Disegno: tutto su un `M5Canvas tela` fuori schermo, poi un solo `pushSprite(0,0)` (niente sfarfallio). Fallback a 8 bit se manca memoria.

\- Float espliciti (`1.0f`) nei calcoli dentro le funzioni; `constexpr`/`const` per le costanti; niente `#define` (tranne dove l'esploratore li usa per macro, e in `tamagotchi/personaggio.h` per scegliere la stella da caricare).

\- Numeri casuali ripetibili (generatore con seme, `SEME\_CIELO`): stesso seme = stesso risultato.

\- Se un sensore non risponde, il codice \*\*non si blocca\*\*: valori a zero e prosegue (`imuOk`).

\- Diagnostica: una riga al secondo sul Monitor seriale (FPS e valori chiave), intervallo in costante.

\- Prestazioni: puntare ai \~30+ FPS; evitare allocazioni dinamiche (`String`, `new`) nel loop di disegno; array a dimensione fissa da costante.

