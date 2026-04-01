#ifndef PICOMK_PERIPHERAL_H
#define PICOMK_PERIPHERAL_H

#include <pico/stdlib.h>

bool peripheral_init(void);
bool peripheral_require_event_processing(void);
void peripheral_process_events(void);

#endif // PICOMK_PERIPHERAL_H
