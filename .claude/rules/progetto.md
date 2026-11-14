\# Progetto: Tamagotchi Stella



\## Obiettivo

Un Tamagotchi \*\*senza elementi iconografici e senza caratteristiche umanoidi\*\*: la creatura è una \*\*stella astratta\*\* (resa "halftone", fatta di puntini su griglia) in uno spazio di punti che ruota col giroscopio. Lo stato interno si comunica solo con forma, colore, ritmo, movimento, luce (e suono), mai con facce, occhi, barre-icona o simboli.



\## File in questa cartella

| File | Ruolo |

|---|---|

| `stella.ino` | \*\*Riferimento di stile\*\* e base attuale: schermata HOME (cielo + stella). |

| `EsploratoreCoreS3.ino` | Sketch di partenza del professore: una pagina per ogni sensore. \*\*Consultalo\*\* per la sintassi corretta delle API (`CoreS3.Imu`, `.Touch`, `.Mic`, `.Ltr553`, `.Speaker`, ESP-NOW…). Non modificarlo. |

| `Automa a stati finiti\_stella.pdf` | Specifica dell'automa (vedi sotto). |

| `moodboard.pdf` | Direzione estetica: stelle a scintilla/punte curve, halftone, puntini su griglia, scie, cielo nero con punti, orbite sottili. |

| `.claude/settings.json` | Permessi di Claude Code per i comandi git e `gh`. Non modificarlo senza richiesta della proprietaria. |



\## Automa a stati (dal PDF)

Statistiche (0–100): `nutrimento (n)`, `socialità (s)`, `energia (e)`, `umore (u)`, `salute`.

\- \*\*IN SALUTE\*\*

&#x20; - Asse attiva: `ATTIVA` ⇄ `SOLITARIA` (s<50 / s>80), `ATTIVA` ⇄ `SCARICA` (n<50 / n>50), `ATTIVA` ⇄ `STATICA` (e<20 / e>50).

&#x20; - Dopo `ATTIVA` si va a `NOTTE` o `GIORNO`. `NOTTE` ⇄ `BRILLARE` (u>80 / u<40). `GIORNO` ⇄ `RICARICA` (u<40 / n>80).

\- \*\*IN CRESCITA\*\*: da `NOTTE` con salute=100 → `COSTELLAZIONE`; ritorna dopo "x" secondi.

\- \*\*IN DECADENZA\*\*: salute<20 → `MALATA` (torna in salute con salute>60); salute=0 → `ESPLOSIONE` → (invita) `UNISONO` → dopo "x" secondi (o "invita") torna a `RICARICA`.

\- Colore della stella in base alla salute: 20% rosa-rosso, 40% arancio, 60% giallo, 80% viola, 100% azzurro chiaro.

\- Il PDF è ambiguo su alcune soglie e sui trigger (`invita`): \*\*chiedere a Chiara prima di implementare lo stato interessato\*\*, non inventare.



\## Estetica (vincoli di design)

\- Nero profondo, punti bianchi/grigi, stella halftone con colore dello stato; scie e pulsazioni lente; movimento elastico (molle) invece di scatti.

\- Vietati: facce, occhi, bocche, braccia, cuori, icone, emoji, barre di stato con simboli. Lo stato si legge da colore, numero di punte, forma, pulsazione, rotazione, densità/luce dei punti, drag.

\- Stile della moodboard: scintille a quattro/otto punte, halftone, griglie di puntini, orbite sottili, linee d'aria.



\## Domande aperte (da chiarire con Chiara prima di implementare)

\- Come variano le statistiche nel tempo (decadimento) e con quali azioni/sensori crescono (tocco, luce, microfono, scossa, vicinanza di un altro dispositivo via ESP-NOW)?

\- Valori esatti di soglie e durate ("x" secondi), significato preciso di `invita`, `UNISONO`, `COSTELLAZIONE`.

\- Uso dell'audio (suoni della creatura?) e persistenza (salvataggio dello stato dopo lo spegnimento?).

