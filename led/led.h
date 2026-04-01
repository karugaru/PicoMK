#ifndef PICOMK_LED_H
#define PICOMK_LED_H

#include "pico/stdlib.h"

void led_put_pixel(uint32_t pixel_grb);
void led_put_rgb(uint8_t red, uint8_t green, uint8_t blue);
void led_init(uint8_t pin, uint8_t brightness);

#endif // PICOMK_LED_H
