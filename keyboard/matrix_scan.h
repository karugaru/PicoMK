#ifndef MATRIX_SCAN_H
#define MATRIX_SCAN_H

#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"
#include <stdbool.h>

void matrix_init(void);
void matrix_process(void);

#endif // MATRIX_SCAN_H
