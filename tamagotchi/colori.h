// =====================================================================
//  COLORI — rgb() e palette
// =====================================================================
#pragma once

// Converte un colore RGB normale nel formato RGB565 usato dallo schermo
constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
const uint16_t NERO   = rgb(0, 0, 0);
const uint16_t BIANCO = rgb(255, 255, 255);
const uint16_t ROSSO_STELLA = rgb(255, 0, 0);
const uint16_t AZZURRO_STELLA   = rgb(102, 255, 255);
const uint16_t GIALLO_STELLA    = rgb(255, 255, 51);
const uint16_t ARANCIONE_STELLA = rgb(255, 128, 0);
// Un colore per livello di salute: 100-80 bianco, 79-60 azzurro, 59-40 giallo, 39-20 arancione, 19-0 rosso
const uint16_t COLORI_SALUTE[5] = { BIANCO, AZZURRO_STELLA, GIALLO_STELLA, ARANCIONE_STELLA, ROSSO_STELLA };
const uint16_t GRIGIO_SLIDER = rgb(70, 70, 70);   // puntini dello slider oltre il livello del volume
const uint16_t COLORE_VALORI = rgb(200, 200, 200);   // scritta dei valori in alto a sinistra
const uint16_t GRIGIO_LIVELLO_SPENTO = rgb(70, 70, 70);   // puntini dei livelli del menu oltre il valore scelto
const uint16_t GRIGIO_MENU =rgb(45, 45, 45);        // puntini di fondo del menu
const uint16_t GRIGIO_BORDO_MENU = rgb(110, 110, 110);   // linea sottile del bordo del menu mentre entra o esce
// Stato malata (logica_malata.ino)
const uint16_t MALATA_COLORE_FONDO = rgb(200, 0, 0);   // rosso dello schermo quando la stella è malata
const uint16_t MALATA_COLORE_FONDO_GUARENDO = rgb(110, 0, 0);   // rosso attenuato dello schermo quando è quasi guarita (fra i due cambia piano piano con la salute)
const uint16_t MALATA_COLORE_CIELO = rgb(110, 0, 0);   // rosso scuro dei puntini del cielo più luminosi (quelli più piccoli sono ancora più scuri)
const uint16_t MALATA_COLORE_CIELO_GUARENDO = rgb(60, 0, 0);   // rosso dei puntini del cielo quando è quasi guarita (più scuro del fondo, come sopra)
const uint16_t MALATA_COLORE_STELLA = NERO;            // colore della stella malata
