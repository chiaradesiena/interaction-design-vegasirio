// =====================================================================
//  Tamagotchi Stella (Sirio e Vega) — M5Stack CoreS3
//  Schermata HOME: una stella in uno spazio che ruota col giroscopio.
//
//  QUALE STELLA CARICARE: si sceglie in personaggio.h (una riga).
//
//  Ogni giro del loop segue tre tempi: LEGGI -> AGGIORNA -> DISEGNA.
//  - Solo sensori.ino parla con i sensori.
//  - I file disegno_*.ino disegnano quello che ricevono,
//    non leggono mai i sensori.
//  - Tutti i numeri da tarare sono in config_comune.h, config_sirio.h, config_vega.h.
//
//  Scheda:   M5CoreS3      Librerie: M5CoreS3 (+ M5Unified, M5GFX)
//  Monitor seriale: 115200 baud
// =====================================================================

#include <M5CoreS3.h>
#include <math.h>
#include <Preferences.h>   // memoria permanente della scheda (già inclusa nel core ESP32, nessuna libreria da installare)
#include <esp_system.h>    // per sapere perché la scheda è ripartita (reset o riaccensione)
#include <WiFi.h>          // radio della scheda (già inclusa nel core ESP32): serve a Unisono
#include <esp_now.h>       // ESP-NOW: messaggi diretti fra dispositivi vicini, senza rete Wi-Fi
#include <esp_wifi.h>      // per scegliere il canale della radio

#include "tipi.h"
#include "config.h"
#include "colori.h"
#include "dati.h"

uint32_t ultimaStampa = 0;
int fotogrammi = 0;

void setup() {
  auto cfg = M5.config();
  CoreS3.begin(cfg);
  Serial.begin(115200);
  Serial.printf("personaggio: %s\n", NOME_PERSONAGGIO);
  caricaStato();

  CoreS3.Display.setBrightness(LUMINOSITA_SCHERMO);
  CoreS3.Speaker.setVolume(volumeCorrente);   // livello di partenza: VOLUME_LIVELLO_INIZIALE
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
  avviaRadio();
  ultimoAggiornamento = millis();
  Serial.println("HOME pronta");
}

void loop() {
  CoreS3.update();

  leggiSensori();
  aggiornaRadio();
  aggiornaStato();

  switch (schermataCorrente) {
    case HOME: schermataHome(tela); break;
    case ANIMAZIONE_COSTELLAZIONE: schermataAnimazioneCostellazione(tela); break;
    case MENU: schermataMenu(tela); break;
    case UNISONO: schermataUnisono(tela); break;
  }
  tela.pushSprite(0, 0);

  // FPS = fotogrammi al secondo
  fotogrammi++;
  if (millis() - ultimaStampa >= INTERVALLO_STAMPA_MS) {
    Serial.printf("FPS %d   punti visibili %d   giro x %.0f y %.0f z %.0f\n",
                  fotogrammi, puntiVisibili, sensi.giroX, sensi.giroY, sensi.giroZ);
    Serial.printf("nutrimento %.0f   socialita %.0f   energia %.0f   umore %.0f   salute %.0f   livello %d\n",
                  bisogni.nutrimento, bisogni.socialita, bisogni.energia, bisogni.umore, bisogni.salute, livelloCorrente);
    Serial.printf("carezza %d   tempo continuo %.1f s   grandezza %.2f\n", carezzando, tempoContinuoS, scalaCarezza.valore);
    Serial.printf("volume voce %.2f (soglia %.2f)   voce attiva %d   microfono %d   luminosita %.0f\n",
                  sensi.volumeVoce, SOCIALITA_VOCE_SOGLIA, voceAttiva, micAttivo, luminositaStella);
    Serial.printf("memoria libera %u   minima %u   psram %u   ascolto %.2f   microfono %d\n",
                  (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMinFreeHeap(), (unsigned)ESP.getPsramSize(), ascolto, micAttivo);
    Serial.printf("scossa %.2f g   scuotendo %d   tempo scossa %.1f s   statica %d   attivita %.2f   festa %d\n",
                  sensi.scossa, scossaRecente, tempoScossaS, staticaEnergia, attivita, festaAttiva);
    Serial.printf("costellazione %d/%d   salute al 100%% da %.1f s   prossima stella fra %d min   orologio %d (%u s)\n",
                  costellazioneStelle, COSTELLAZIONE_NUMERO_STELLE, costellazioneTenutaS, minutiAllaProssimaStella(millis()),
                  sensi.orologioOk, (unsigned)sensi.orologioS);
    stampaRadio();
    fotogrammi = 0;
    ultimaStampa = millis();
  }

  delay(1);
}
