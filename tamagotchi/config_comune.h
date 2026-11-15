// =====================================================================
//  CONFIGURAZIONE COMUNE — numeri da tarare, uguali per Sirio e Vega.
//  Quello che cambia fra le due stelle sta in config_sirio.h e config_vega.h.
// =====================================================================
#pragma once

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
const float MOLLA_PASSO_MAX_S = 0.01;           // le molle (cielo, drag, carezza) si calcolano a passi di al massimo tanti secondi: evita che "impazziscano" quando gli FPS sono bassi (non serve cambiarlo)

// Elasticità e motion blur del cielo
const float ELASTICO_CIELO_FREQUENZA   = 3.0;   // più alto = cielo più rigido, segue subito il movimento
const float ELASTICO_CIELO_SMORZAMENTO = 0.5;   // 1 = nessun rimbalzo; più basso = rimbalza di più
const bool  CIELO_RIMBALZO             = false; // false = se il giroscopio è fermo il cielo si ferma subito; true = va un po' oltre e torna indietro
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
const float STELLA_RAGGIO         = 64;         // px, dal centro alla fine di una punta
const float STELLA_VITA           = 0.3;       // larghezza fra le punte (0 = punte sottilissime, 1 = cerchio)
const int   STELLA_PASSO_GRIGLIA  = 8;          // px fra un puntino e l'altro
const float STELLA_ALONE          = 1.3;        // alone di puntini minuscoli fino a 1.3 volte il raggio (1 = niente alone)
const float STELLA_FORZA_ALONE    = 0.1;      // quanto è visibile l'alone (0-1)
const float PULSAZIONE_AMPIEZZA   = 0.10;       // la stella cresce e cala del 6%
const uint32_t PULSAZIONE_PERIODO_MS = 3500;    // durata di un respiro di luce completo
const float STELLA_ROTAZIONE_GRADI_S = 0.5;       // rotazione lenta della stella su sé stessa (0 = ferma)

// Buco al centro della stella: più il nutrimento è basso, più è grande (lascia vedere il cielo dietro)
const float BUCO_SOGLIE[5] = { 91, 80, 60, 40, 20 };               // nutrimento da cui in su: 91 = nessun buco, 80 = livello 1, 60 = 2, 40 = 3, 20 = 4; sotto 20 = livello 5
const float BUCO_RAGGIO[6] = { 0, 0.15, 0.25, 0.35, 0.45, 0.55 };  // grandezza del buco per livello, come frazione del raggio della stella (0.55 = più della metà)
const float BUCO_VELOCITA  = 3;                 // quanto in fretta il buco cambia grandezza (più alto = più rapido, 3 = circa 1 secondo)

// Bisogni e salute: ogni bisogno va da 100 (soddisfatto) a 0 (per niente); la salute è la media dei 4
const float BISOGNO_MAX = 100;                  // valore di un bisogno completamente soddisfatto
const float NUTRIMENTO_DECADIMENTO = 1;         // quanti punti percentuali perde il nutrimento a ogni intervallo
const float NUTRIMENTO_INTERVALLO_S = 15;       // ogni quanti secondi perde quei punti (1 punto ogni 15 s = da 100 a 0 in 1500 s, con MOLTIPLICATORE_TEMPO 0.5 circa 50 minuti)
const float MOLTIPLICATORE_TEMPO = 0.5;           // velocità dei bisogni: 1 = normale; 10 = dieci volte più veloci (utile per le prove)
const float SALUTE_SOGLIE[4] = { 80, 60, 40, 20 };   // salute da qui in su: 80 = livello bianco, 60 = azzurro, 40 = giallo, 20 = arancione; sotto 20 = rosso
const float SALUTE_DURATA_COLORE_MS = 1000;     // durata della sfumatura quando cambia il colore (0 = cambio netto)

// Socialità: scende col tempo, risale quando qualcuno parla vicino; sotto la soglia la stella lampeggia (acceso/spento netto)
const float SOCIALITA_DECADIMENTO = 1;          // quanti punti percentuali perde la socialità a ogni intervallo
const float SOCIALITA_SOGLIA_LAMPEGGIO = 50;    // sotto questo valore la stella lampeggia (0-100)
const float SOCIALITA_AUMENTO_PUNTI_S = 5;      // quanti punti di socialità si guadagnano al secondo finché qualcuno parla
const uint32_t SOCIALITA_TENUTA_VOCE_MS = 400;  // la voce conta ancora per tanto, in ms, dopo l'ultimo momento forte (tiene unite le pause fra le parole)
const uint32_t MIC_ATTESA_DOPO_SUONO_MS = 300;  // dopo un suono si aspetta tanto, in ms, prima di riaccendere il microfono (altoparlante e microfono non lavorano insieme)
const size_t   MIC_CAMPIONI = 256;              // quanti campioni ha ogni misura del microfono (256 a 16000 Hz = 16 ms)
const uint32_t MIC_FREQUENZA_HZ = 16000;        // frequenza di campionamento del microfono

// Feedback di ascolto: si sceglie con ASCOLTO_TIPO in config_sirio.h / config_vega.h
// ASCOLTO_PALLINO (nato per Sirio): mentre qualcuno parla la stella diventa un pallino fatto di puntini (halftone)
const float SIRIO_ASCOLTO_RAGGIO_PX = 30;       // raggio del pallino durante l'ascolto, in px (più alto = pallino più grande; 64 = grande quanto la stella)
// ASCOLTO_RAGGI (nato per Vega): mentre qualcuno parla il nucleo resta uguale e i raggi si allungano fino ai bordi dello schermo
const float VEGA_ASCOLTO_RAGGIO_PX = 200;       // lunghezza dei raggi durante l'ascolto, in px (200 = arrivano agli angoli dello schermo; 120 = solo ai lati corti; più alto = vanno oltre)
const float ASCOLTO_VELOCITA_SALITA = 6;        // quanto in fretta la stella diventa pallino quando inizia la voce (più alto = più rapido, 6 = circa mezzo secondo)
const float ASCOLTO_VELOCITA_DISCESA = 2.5;     // quanto in fretta torna normale quando la voce finisce (più basso = ritorno più lento)

// Richiamo del nutrimento: quando il nutrimento arriva alla soglia suona (le note sono in config_sirio.h / config_vega.h)
const float NUTRIMENTO_SOGLIA_SUONO = 50;       // il richiamo parte quando il nutrimento scende a questo valore (0-100)
const float RICHIAMO_RIPETIZIONE_S  = 20;       // ogni quanti secondi si ripete finché il nutrimento resta sotto la soglia

// Slider del volume: fila di puntini in basso, si trascina il dito (VOLUME_SUONO è il volume di partenza)
const int   SLIDER_ZONA_ALTEZZA   = 36;         // px, fascia in basso dove il dito controlla il volume
const int   SLIDER_MARGINE_X      = 30;         // px lasciati liberi a sinistra e a destra
const int   SLIDER_PASSO_PUNTINI  = 8;          // px fra un puntino e l'altro
const int   SLIDER_RAGGIO_CURSORE = 6;          // px, grandezza del cursore
const float SLIDER_NOTA_PROVA_HZ  = 880;        // nota di prova quando si toglie il dito (più alto = più acuta)
const uint16_t SLIDER_NOTA_PROVA_MS = 150;      // durata della nota di prova, in ms

// Menu delle impostazioni: si apre con uno swipe verso destra, si chiude con uno swipe verso sinistra
const int   MENU_SWIPE_MIN_PX = 80;             // px che il dito deve percorrere perché lo swipe conti (più alto = serve uno swipe più lungo)
const float MENU_SWIPE_ORIZZONTALE = 1.5;       // lo swipe deve essere tante volte più orizzontale che verticale (più alto = deve essere più dritto)
const float MENU_IGNORA_DOPO_CAREZZA_S = 0.5;   // se si stava accarezzando la stella da più di tanti secondi, lo swipe non apre il menu
const float MENU_FREQUENZA = 2.5;               // velocità con cui il menu entra ed esce (più alto = più rapido)
const float MENU_SMORZAMENTO = 0.6;             // elasticità: più basso = rimbalza di più, 1 = arriva senza rimbalzo
const int   MENU_PASSO_GRIGLIA = 16;            // px fra i puntini di fondo del menu (più basso = griglia più fitta)
const int   MENU_RAGGIO_PUNTINI = 1;            // grandezza dei puntini di fondo, in px
const int   MENU_TASTO_MENO_X = 40;             // px dal bordo sinistro, centro del tasto -
const int   MENU_TASTO_PIU_X  = 280;            // px dal bordo sinistro, centro del tasto +
const int   MENU_TASTO_RAGGIO_PX = 16;          // grandezza del cerchio dei tasti, in px
const int   MENU_TASTO_AREA_PX = 30;            // zona sensibile attorno al centro del tasto, in px (più alto = più facile da toccare)
const int   MENU_SEGNO_PX = 7;                  // metà della lunghezza del segno + e -, in px
const uint32_t MENU_LAMPO_TASTO_MS = 150;       // per quanti ms il tasto toccato resta illuminato
const int   MENU_ETICHETTA_SOPRA_PX = 26;       // px fra il nome della riga (es. "volume") e la riga
const int   MENU_PASSO_LIVELLI_PX = 18;         // px fra un puntino del livello e l'altro
const int   MENU_LIVELLO_RAGGIO_MIN_PX = 2;     // grandezza del primo puntino (il più piccolo), in px
const int   MENU_LIVELLO_RAGGIO_MAX_PX = 5;     // grandezza dell'ultimo puntino (il più grande), in px
const int   MENU_VOLUME_Y = 50;                 // px dal bordo alto, altezza della riga del volume
const int   MENU_STELLE_Y = 115;                // px dal bordo alto, altezza della riga delle stelle della costellazione
const int   MENU_PARAMETRI_Y = 175;             // px dal bordo alto, altezza della prima riga dei parametri (salute, nutrimento)
const int   MENU_PARAMETRI_INTERLINEA_PX = 30;  // px fra la prima e la seconda riga dei parametri
const int   MENU_PARAMETRI_COLONNA_SINISTRA_X = 90;   // px dal bordo sinistro, centro della colonna di sinistra (salute, socialita)
const int   MENU_PARAMETRI_COLONNA_DESTRA_X = 230;    // px dal bordo sinistro, centro della colonna di destra (nutrimento, energia)
const float MENU_PARAMETRI_DIMENSIONE_TESTO = 1;      // grandezza del testo dei parametri (1 = piccolo, 2 = doppio: allora allarga le colonne)
// Volume: 10 livelli; il livello 1 è muto, il 10 è il massimo (il livello di partenza è VOLUME_LIVELLO_INIZIALE in config_sirio.h / config_vega.h)
const int   VOLUME_NUMERO_LIVELLI = 10;
const uint8_t VOLUME_LIVELLI[VOLUME_NUMERO_LIVELLI] = { 0, 28, 57, 85, 113, 142, 170, 198, 227, 255 };   // volume dell'altoparlante per ogni livello (0-255)
const float VOLUME_NOTA_PROVA_HZ = 880;         // nota di prova a ogni tocco di + o - (più alto = più acuta)
const uint16_t VOLUME_NOTA_PROVA_MS = 120;      // durata della nota di prova, in ms

// Carezza: tocco continuo con il dito che si muove a destra e a sinistra sulla stella = la si sta nutrendo
const float CAREZZA_DURATA_MIN_S = 1;           // secondi di carezza continua prima che il nutrimento cominci a salire
const float NUTRIMENTO_AUMENTO_PUNTI_S = 8;     // quanti punti di nutrimento al secondo si guadagnano accarezzando
const float CAREZZA_VELOCITA_MIN_PX_S = 10;     // velocità orizzontale minima del dito per contare come carezza (più alto = serve strusciare più in fretta)
const uint16_t CAREZZA_PAUSA_TOLLERATA_MS = 500;   // se la carezza si ferma o il dito si stacca per più di così, i 2 secondi ripartono da zero
const uint16_t CAREZZA_RITORNO_ATTESA_MS = 2000;   // dopo l'ultima carezza la stella resta ingrandita per tanto, poi torna normale
const float CAREZZA_AREA_STELLA = 1.5;          // zona sensibile: 1 = grandezza attuale della stella; più alto = più facile da toccare
const float CAREZZA_OCCUPAZIONE_MAX = 0.9;      // quanto dell'altezza dello schermo occupa al massimo la stella (0-1)
const float CAREZZA_RITORNO_FREQUENZA = 2.2;    // velocità con cui la stella cambia grandezza (più alto = più rapida)
const float TREMOLIO_AMPIEZZA_PX = 2.5;         // tremolio della stella mentre la si accarezza: spostamento massimo in px (0 = niente tremolio)
const float TREMOLIO_FREQUENZA_HZ = 7;          // quanto in fretta trema (più alto = più veloce)
const float TREMOLIO_VELOCITA_CAMBIO = 6;       // quanto in fretta il tremolio parte e si spegne (più alto = più rapido)
const float BATTITO_INTERVALLO_S = 1.5;         // durante la carezza, ogni quanti secondi suona un battito
const uint16_t BATTITO_DURATA_MS = 80;          // durata di ogni battito, in ms (più corto = più leggero)
// Crescita durante la carezza: si sceglie con CRESCITA_TIPO in config_sirio.h / config_vega.h
// CRESCITA_TRATTI (nata per Sirio): la stella fa tre tratti: cresce, si riduce a metà strada, cresce ancora e poi resta
const int   SIRIO_NUMERO_TRATTI = 3;            // quanti tratti ha la sequenza (devono corrispondere agli elenchi qui sotto)
const float SIRIO_DURATE_S[SIRIO_NUMERO_TRATTI] = { 5, 3, 5 };          // durata di ogni tratto, in secondi
const float SIRIO_SCALA[SIRIO_NUMERO_TRATTI]    = { 1.6, 1.3, 1.7 };    // grandezza a fine tratto (1 = normale; 1.6 = +60%; 1.3 = metà strada fra 1.6 e normale)
// CRESCITA_CONTINUA (nata per Vega): più la carezza è continua, più la stella cresce
const float VEGA_DURATA_CRESCITA_S = 7;       // secondi di carezza continua per arrivare alla grandezza massima

// Melodia quando, accarezzandola, il nutrimento arriva al 100% (le note sono in config_sirio.h / config_vega.h)
const float PIENO_RIARMO_SOGLIA = 90;           // dopo la melodia il nutrimento deve scendere sotto questo valore prima che possa suonare di nuovo (0-100)
const uint16_t PIENO_PAUSA_MS = 20;             // silenzio fra una nota e l'altra, in ms

// Memoria dei bisogni: salvataggio nella memoria permanente, così spegnendo e riaccendendo i valori restano
const bool  AZZERA_CON_RESET = true;            // true = il pulsante reset (freccia circolare) riporta i bisogni ai valori dello script; false = il reset li conserva
const bool  AZZERA_CON_TOCCO = false;           // true = col dito appoggiato allo schermo mentre la scheda parte i bisogni tornano ai valori dello script (ripiego se il reset non funziona)
const float SALVATAGGIO_INTERVALLO_S = 30;      // ogni quanti secondi si salva (più basso = si perde meno allo spegnimento, ma la memoria si consuma prima)
const float SALVATAGGIO_VARIAZIONE_MIN = 1;     // si salva solo se almeno un valore è cambiato di tanti punti
const float SALVATAGGIO_ATTESA_MIN_S = 5;       // quando cambia il colore della salute si salva subito, ma non più spesso di così
const uint32_t FIRMA_RIAVVIO = 0x57E11A01;      // numero-segnale lasciato in memoria: serve a riconoscere un reset (non va cambiato)

// Valori dei bisogni scritti in alto a sinistra
const bool  VALORI_MOSTRA = true;               // false = nessuna scritta
const bool  VALORI_MOSTRA_ENERGIA_UMORE = false;   // true = mostra anche l'umore (per ora non cambia; l'energia si vede sempre)
const int   VALORI_X = 6;                       // px dal bordo sinistro
const int   VALORI_Y = 6;                       // px dal bordo alto
const float VALORI_DIMENSIONE_TESTO = 1;        // grandezza del testo (1 = piccolo, 2 = doppio)
const int   VALORI_INTERLINEA_PX = 12;          // px fra una riga e l'altra (alzalo se alzi la grandezza del testo)

// Energia: scende col tempo come il nutrimento e sale scuotendo il dispositivo; con energia bassa la stella sta ferma
const float ENERGIA_INIZIALE = 50;              // energia all'accensione (0-100)
const float ENERGIA_DECADIMENTO = 1;            // quanti punti percentuali perde l'energia a ogni intervallo
const float ENERGIA_INTERVALLO_S = 15;          // ogni quanti secondi perde quei punti (come il nutrimento)
const float ENERGIA_SCOSSA_SOGLIA_G = 0.8;      // quanto forte si deve scuotere (g oltre la gravità normale); più alto = serve scuotere più forte
const float ENERGIA_SCOSSA_DURATA_MIN_S = 2;    // secondi di scuotimento continuo prima che l'energia cominci a salire
const uint16_t ENERGIA_SCOSSA_PAUSA_MS = 300;   // se non si scuote per più di così, i secondi ripartono da zero
const float ENERGIA_AUMENTO_PUNTI_S = 10;       // quanti punti di energia al secondo si guadagnano scuotendo
const float ENERGIA_SOGLIA_STATICA = 10;        // con energia pari o sotto questo valore la stella si ferma al centro (0-100)
const float ENERGIA_SOGLIA_USCITA_STATICA = 20; // per ripartire l'energia deve superare questo valore (più alto = resta ferma più a lungo)
const float ENERGIA_ATTIVITA_VELOCITA = 2;      // quanto in fretta la stella si ferma o riparte (più alto = più rapido)

// Movimento attivo: si sceglie con ORBITA_TIPO in config_sirio.h / config_vega.h
// ORBITA_OTTO (nata per Sirio): orbita a 8 senza profondità
const float SIRIO_ORBITA_AMPIEZZA_X_PX = 60;   // larghezza dell'8, in px (più alto = 8 più largo)
const float SIRIO_ORBITA_AMPIEZZA_Y_PX = 30;   // altezza dell'8, in px
const float SIRIO_ORBITA_PERIODO_S = 12;       // secondi per disegnare un 8 completo (più alto = più lenta)
// ORBITA_CASUALE (nata per Vega): orbite casuali con profondità; la stella si rimpicciolisce quando si allontana e si ingrandisce quando si avvicina
const float VEGA_ORBITA_AMPIEZZA_X_PX = 50;    // quanto si sposta a destra e sinistra, in px (più alto = orbita più larga)
const float VEGA_ORBITA_AMPIEZZA_Y_PX = 35;    // quanto si sposta in alto e in basso, in px
const float VEGA_ORBITA_PERIODO_X_S = 13;      // secondi per un giro orizzontale (più alto = più lenta)
const float VEGA_ORBITA_PERIODO_Y_S = 17;      // secondi per un giro verticale (periodi diversi = percorso che non si ripete)
const float VEGA_ORBITA_PERIODO_Z_S = 11;      // secondi per un'andata e ritorno in profondità
const float VEGA_PROFONDITA = 0.25;            // quanto cambia la grandezza (0.25 = da 75% a 125%; 0 = nessuna profondità)
const float ORBITA_SECONDA_ONDA_PESO = 0.25;   // quanto conta la seconda oscillazione che rende l'orbita irregolare (0 = orbita regolare, 0.5 = molto irregolare)
const float ORBITA_SECONDA_ONDA_RAPPORTO = 0.618;   // durata della seconda oscillazione rispetto alla prima (non vicino a 1: si confonderebbe)
const float ORBITA_SECONDA_ONDA_FASE = 1.7;    // sfasamento della seconda oscillazione (radianti)
const float ORBITA_FASI[3] = { 0.0, 2.1, 4.2 };   // sfasamento di x, y e profondità, perché non vadano in sincrono

// Festa quando l'energia arriva al 100% scuotendo: percorso, poi torna al centro, sta ferma, riparte. Si sceglie con FESTA_TIPO
const float FESTA_RIARMO_SOGLIA = 90;          // dopo la festa l'energia deve scendere sotto questo valore prima che possa ripartire (0-100)
const float FESTA_FERMO_S = 3;                 // secondi in cui la stella resta ferma al centro dopo il percorso
// FESTA_SALTI_BORDO (nata per Sirio): percorre il bordo con salti lenti
const int   SIRIO_FESTA_GIRI = 3;              // quanti giri del bordo, in senso orario
const int   SIRIO_FESTA_SALTI_PER_GIRO = 3;    // in quanti salti si fa un giro
const float SIRIO_FESTA_SALTO_S = 0.8;         // durata di un salto in secondi (più alto = più lento e più "lunare")
const float SIRIO_FESTA_ALTEZZA_SALTO_PX = 40; // quanto si stacca dal bordo verso l'interno a metà salto, in px (gravità lunare = salti alti e lenti)
const float SIRIO_FESTA_MARGINE_PX = 30;       // distanza del centro della stella dal bordo dello schermo, in px
const float SIRIO_FESTA_TRANSIZIONE_S = 0.6;   // secondi per andare dal centro al bordo, e dal bordo al centro
// FESTA_RIMBALZI (nata per Vega): rimbalza dal bordo alto a quello basso
const int   VEGA_FESTA_RIMBALZI = 5;           // quante volte tocca un bordo (alto, basso, alto, ...)
const float VEGA_FESTA_VELOCITA_PX_S = 600;    // velocità del percorso in px al secondo (più alto = più veloce)
const float VEGA_FESTA_MARGINE_PX = 30;        // distanza del centro della stella dal bordo quando "rimbalza", in px (più basso = più vicino al bordo)

// Suono della festa (le note sono in config_sirio.h / config_vega.h)
const uint16_t FESTA_PAUSA_MS = 20;            // silenzio fra una nota e l'altra, in ms

// Costellazione: con la salute al 100% per un po' si aggiunge una stella (le stelle sono in config_sirio.h / config_vega.h)
const float COSTELLAZIONE_SALUTE_SOGLIA = 99.5;  // salute da raggiungere (99.5 = la scritta in alto mostra già "100%")
const float COSTELLAZIONE_TENUTA_S = 5;          // secondi in cui la salute deve restare lì
const uint32_t COSTELLAZIONE_PAUSA_S = 3600;   // secondi minimi fra una stella e la prossima (3600 = 1 ora; per le prove si può abbassare, es. 30)
const bool  COSTELLAZIONE_AZZERA_CON_RESET = true;   // true = il reset (come per i bisogni) riporta la costellazione alla sola stella principale
const uint32_t OROLOGIO_INTERVALLO_MS = 1000;    // ogni quanto leggere l'orologio della board (non serve cambiarlo)
const int   OROLOGIO_ANNO_PARTENZA = 2026;       // se l'orologio della board non è mai stato regolato, riparte dal 1° gennaio di quest'anno (non serve l'ora vera)
// Le stelle piccole stanno su un cerchio che gira attorno alla stella principale, alla stessa distanza fra loro,
// unite da tratti sottili: la prima alla principale, ognuna alla precedente; a costellazione completa il cerchio si chiude
const float COSTELLAZIONE_GRANDEZZA = 0.2;       // grandezza delle stelle piccole rispetto alla principale (0.2 = 20%)
const int   COSTELLAZIONE_PASSO_GRIGLIA = 3;     // px fra i puntini delle stelle piccole (più basso = più dettaglio; la principale usa 8)
const float COSTELLAZIONE_RAGGIO_CERCHIO_PX = 95;   // distanza delle stelle piccole dal centro della principale, in px
const float COSTELLAZIONE_GIRO_S = 60;           // secondi per un giro completo del cerchio (più alto = più lento; negativo = gira al contrario)
const float COSTELLAZIONE_LUCE_TRATTO = 0.6;     // luminosità dei tratti rispetto al colore della stella (0-1)
// Animazione quando si aggiunge una stella (10 secondi). Ogni numero è il secondo in cui FINISCE quel passaggio
const float ANIM_COST_FINE_SCOMPARSA_S = 1;      // 0-1 s: la stella si rimpicciolisce e scompare
const float ANIM_COST_FINE_BIANCO_S = 2;         // 1-2 s: lo schermo diventa bianco
const float ANIM_COST_FINE_PUNTI_S = 3;          // 2-3 s: due puntini neri lampeggiano
const float ANIM_COST_FINE_TRATTO_S = 5;         // 3-5 s: fra i due puntini si disegna il tratto sottile
const float ANIM_COST_FINE_NERO_S = 6;           // 5-6 s: lo schermo torna nero
const float ANIM_COST_FINE_CIELO_S = 7;          // 6-7 s: ritornano le stelle di fondo, un punto alla volta
const float ANIM_COST_DURATA_S = 10;             // 7-10 s: ricompare la stella lampeggiando con la stella nuova; poi finisce
const uint32_t ANIM_COST_LAMPEGGIO_PUNTI_MS = 250;   // ogni quanti ms i puntini neri si accendono o si spengono (più basso = più veloce)
const uint32_t ANIM_COST_LAMPEGGIO_STELLA_MS = 500;  // ogni quanti ms la stella si accende o si spegne quando ricompare
const int   ANIM_COST_PUNTO_PX = 3;              // raggio dei puntini neri delle stelle piccole, in px
const int   ANIM_COST_PUNTO_PRINCIPALE_PX = 5;   // raggio del puntino nero della stella principale, in px (più grande: si riconosce)
const float COSTELLAZIONE_VELOCITA_SPARIZIONE = 3;   // quanto in fretta le stelle piccole spariscono e ricompaiono durante la festa (più alto = più rapido)

// Unisono — radio (ESP-NOW): le stelle si chiamano e capiscono chi è vicino
const float UNISONO_CHIAMATA_S = 20;             // ogni quanti secondi la stella manda la sua richiesta di unisono (più basso = si trovano prima)
const uint32_t UNISONO_VERIFICA_MS = 1000;       // quando sente una stella vicina, si chiamano più spesso: ogni tanti ms (più basso = si confermano prima)
const int   UNISONO_RSSI_SOGLIA = -60;           // forza minima del segnale per dire "vicina" (-40 = attaccati, -90 = lontanissimi; più vicino a 0 = bisogna stare più vicini)
const int   UNISONO_CONFERME = 3;                // quanti messaggi forti di fila servono per dire "vicina" (più alto = più sicuro ma più lento)
const float UNISONO_SILENZIO_MAX_S = 45;         // se una stella tace per tanti secondi, la si considera andata via (deve restare più alto di UNISONO_CHIAMATA_S)
const float UNISONO_ATTESA_S = 10;               // dopo l'incontro si aspetta tanto, in secondi, per vedere se arrivano altre stelle
const float UNISONO_SALUTE_MINIMA = 20;          // la stella è "in salute" (disponibile per Unisono) da questa salute in su (0-100)
const int   UNISONO_MAX_STELLE = 8;              // quante stelle vicine si possono ricordare insieme
const uint8_t UNISONO_CANALE_RADIO = 1;          // canale radio: deve essere uguale su tutti i dispositivi (1-13)
const uint8_t UNISONO_VERSIONE_MESSAGGIO = 1;    // non va tarato: cambia solo se cambia la forma del messaggio

// Unisono — lo stato: attesa di UNISONO_ATTESA_S, galassia per UNISONO_DURATA_S, poi socialità al 100% e pausa
const float UNISONO_DURATA_S = 20;               // secondi di animazione della galassia (dopo l'attesa)
const float UNISONO_PAUSA_DOPO_S = 300;          // secondi dopo la fine di Unisono prima di poterci rientrare (300 = 5 minuti; per le prove si può abbassare, es. 30)
const float UNISONO_ARRIVO_S = 1.5;              // secondi che una stella impiega ad arrivare dal bordo al suo posto (e a ripartire alla fine)
const float UNISONO_GRANDEZZA_PROPRIA = 0.5;     // grandezza della propria stella al centro, rispetto al normale (0.5 = metà)
const float UNISONO_GRANDEZZA_ALTRE = 0.3;       // grandezza delle altre stelle, rispetto a STELLA_RAGGIO (0.3 = 30%)
const int   UNISONO_PASSO_GRIGLIA = 4;           // px fra i puntini delle altre stelle (più basso = più dettaglio)
const int   UNISONO_PASSO_MINIMO = 4;            // px: la propria stella, rimpicciolendo, rende i puntini più fitti fino a qui
const float UNISONO_DISTANZA_ARRIVO_PX = 230;    // da quanto lontano arrivano (fuori dallo schermo), scivolando lungo il braccio, in px
const float UNISONO_GIRO_S = 30;                 // secondi per un giro completo della galassia (più alto = più lento; negativo = al contrario)
const float UNISONO_VELOCITA_POSTO = 2;          // quanto in fretta le stelle si spostano per fare spazio a una nuova (più alto = più rapido)

// Unisono — i bracci della spirale: righe di puntini grigi che partono dalla propria stella e si avvolgono.
// Le altre stelle stanno sui bracci, a turno (la prima sul primo braccio, la seconda sul secondo, ...)
const int   UNISONO_BRACCI = 2;                  // quanti bracci ha la spirale
const float UNISONO_RAGGIO_INTERNO_PX = 28;      // dove iniziano i bracci, in px dal centro
const float UNISONO_RAGGIO_STELLE_MAX_PX = 110;  // la stella più lontana su un braccio sta a tanti px dal centro
const float UNISONO_RAGGIO_BRACCI_PX = 170;      // fin dove si disegnano i bracci, in px dal centro (oltre 160 escono dallo schermo)
const float UNISONO_AVVOLGIMENTO = 2.5;          // quanto si avvolge un braccio fra RAGGIO_INTERNO e RAGGIO_STELLE_MAX, in radianti (più alto = spirale più stretta; negativo = al contrario)
const float UNISONO_BRACCIO_PASSO_PX = 6;        // px fra un puntino e l'altro del braccio (più basso = braccio più pieno, ma più lento da disegnare)
const float UNISONO_BRACCIO_LUCE = 0.55;         // luminosità dei puntini vicino al centro (0-1); verso l'esterno si spengono piano

// Unisono — il braccio di luce: una riga di luce che gira dal centro e, passando, illumina le stelle
const bool  UNISONO_LUCE_ACCENDE_STELLE = false; // true = anche il braccio di luce, passando, illumina le stelle; false = le stelle si illuminano solo quando suonano
const float UNISONO_LUCE_LUNGHEZZA_PX = 160;     // quanto è lungo, in px dal centro
const float UNISONO_LUCE_PASSO_PX = 5;           // px fra un puntino e l'altro della luce
const int   UNISONO_LUCE_SCIA = 6;               // quante righe più deboli lascia dietro di sé (0 = nessuna scia)
const float UNISONO_LUCE_SCIA_GRADI = 4;         // gradi fra una riga della scia e la successiva (più alto = scia più larga)
const float UNISONO_LUCE_FORZA = 0.8;            // luminosità della riga principale (0-1)
const float UNISONO_LUCE_SPEGNIMENTO_S = 0.8;    // secondi in cui una stella illuminata torna normale (più alto = resta accesa di più)
const float UNISONO_LUCE_CRESCITA = 0.5;         // quanto cresce una stella illuminata (0.5 = 50% più grande)
const float UNISONO_LUCE_BIANCO = 0.6;           // quanto diventa bianca una stella illuminata (0 = resta del suo colore, 1 = bianca)

// Unisono — stella finta, solo per provare con un dispositivo. Normalmente è spenta (0): Sirio e Vega vanno in Unisono
// solo quando si trovano davvero vicine (radio), e nella galassia compaiono solo le stelle vere.
// Per una prova con la sola Sirio si può mettere 1: compare una Vega finta (sulla board di Vega non ha senso usarla).
const int   UNISONO_MAX_FINTE = 1;               // non va tarato: quante stelle finte sono descritte qui sotto
const int   UNISONO_STELLE_FINTE = 0;            // quante stelle finte usare (0 = nessuna: si usano solo le stelle vere)
const float UNISONO_FINTE_PRIMA_S = 30;          // la stella finta "arriva vicino" tanti secondi dopo l'accensione
const float UNISONO_FINTE_INTERVALLO_S = 4;      // se ce ne fossero più d'una, arriverebbero a tanti secondi l'una dall'altra
const char* const UNISONO_FINTE_NOMI[UNISONO_MAX_FINTE] = { "Vega" };
const uint8_t UNISONO_FINTE_COLORI[UNISONO_MAX_FINTE][3] =   // colore della stella finta (rosso, verde, blu: 0-255)
  { {150, 230, 255} };
const uint8_t UNISONO_FINTE_PUNTE[UNISONO_MAX_FINTE] = { 7 };
const float UNISONO_FINTE_VITA[UNISONO_MAX_FINTE] = { 0.3 };
const float UNISONO_FINTE_AFFILATURA[UNISONO_MAX_FINTE] = { 5 };
// La voce della stella finta: copia della voce di config_vega.h (ORCHESTRA_RITMO e ORCHESTRA_NOTE_HZ)
const char* const UNISONO_FINTE_RITMO[UNISONO_MAX_FINTE] = {
  "10110101" "10101101" "10110101" "11011011" };
const int   UNISONO_FINTE_MAX_NOTE = 8;          // non va tarato: posti per le note della stella finta
const int   UNISONO_FINTE_NUMERO_NOTE[UNISONO_MAX_FINTE] = { 8 };   // quante note usa davvero
const float UNISONO_FINTE_NOTE_HZ[UNISONO_MAX_FINTE][UNISONO_FINTE_MAX_NOTE] = {
  { 880.00, 1046.50, 1318.51, 1174.66, 1567.98, 1318.51, 1174.66, 1046.50 } };
const uint16_t UNISONO_FINTE_DURATA_MS[UNISONO_MAX_FINTE] = { 70 };   // durata delle note della stella finta, in ms
const bool  UNISONO_FINTE_SUONANO = true;        // true = con un solo dispositivo si sentono anche le stelle finte, ognuna sul suo canale; false = si illuminano ma restano mute
const int   ORCHESTRA_CANALE_MIO = 0;            // non va tarato: canale dell'altoparlante (0-7) per la propria voce; la stella finta usa il canale dopo
const uint8_t ORCHESTRA_VOLUME_MIO = 255;        // volume della propria voce (0-255), rispetto al volume scelto nel menu
const uint8_t ORCHESTRA_VOLUME_FINTE = 170;      // volume delle stelle finte (0-255): più basso = la propria stella si sente di più

// Unisono — l'orchestra: tutte le stelle vanno a tempo con un "direttore" (la stella con l'indirizzo radio più basso).
// Ogni dispositivo suona solo la voce della propria stella (l'altoparlante fa una nota alla volta): l'orchestra nasce nella stanza
const int   ORCHESTRA_PASSI = 32;                // non va tarato: caselle di un ritmo (i ritmi in config_sirio.h / config_vega.h sono lunghi così)
const uint32_t ORCHESTRA_PASSO_MS = 125;         // durata di una casella del ritmo, in ms (125 x 32 = una battuta di 4 s = un giro del braccio di luce; più alto = musica più lenta)
const float ORCHESTRA_FINE_PRIMA_S = 1;          // l'orchestra smette tanti secondi prima della fine della galassia, mentre le stelle ripartono

// Conversioni fisse (non vanno tarate)
const float GRADI_A_RADIANTI = PI / 180.0;
