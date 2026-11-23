// =====================================================================
//  CONFIGURAZIONE — mette insieme i numeri comuni e quelli della stella
//  scelta in personaggio.h. Qui non c'è niente da tarare.
// =====================================================================
#pragma once

#include "personaggio.h"
#include "config_comune.h"

#if defined(PERSONAGGIO_SIRIO) && defined(PERSONAGGIO_VEGA)
  #error "In personaggio.h sono attive due righe: lasciane una sola (metti // davanti all'altra)"
#elif defined(PERSONAGGIO_SIRIO)
  #include "config_sirio.h"
#elif defined(PERSONAGGIO_VEGA)
  #include "config_vega.h"
#else
  #error "In personaggio.h non e' attiva nessuna stella: togli // davanti a una riga #define"
#endif

#include "brano.h"   // lo spartito della musica di Unisono (le due parti, per tutte e due le stelle)
