#ifndef PICOMK_MATRIX_SCAN_H
#define PICOMK_MATRIX_SCAN_H

#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"
#include <stdbool.h>

void matrix_init(void);
void matrix_process(void);

#endif // PICOMK_MATRIX_SCAN_H
