#include "led.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

static uint8_t brightness = 255;
static PIO pio = NULL;

/**
 * @brief GRB形式のピクセルデータをLEDに送信する。
 * @param pixel_grb GRB形式のピクセルデータ (0x00GGRRBB)
 */
void led_put_pixel(uint32_t pixel_grb) {
  if (pio == NULL)
    return;
  pio_sm_put_blocking(pio, 0, pixel_grb << 8u);
}

/**
 * @brief RGB形式の色データをLEDに送信する。
 * @param red 赤成分 (0-255)
 * @param green 緑成分 (0-255)
 * @param blue 青成分 (0-255)
 */
void led_put_rgb(uint8_t red, uint8_t green, uint8_t blue) {
  if (pio == NULL)
    return;
  red = (red * brightness) / 255;
  green = (green * brightness) / 255;
  blue = (blue * brightness) / 255;
  uint32_t mask = (green << 16) | (red << 8) | (blue << 0);
  led_put_pixel(mask);
}

/**
 * @brief LEDの初期化を行う。
 * @param pin LEDの制御に使用するGPIOピン番号
 * @param bright 明るさ (0-255)
 */
void led_init(uint8_t pin, uint8_t bright) {
  brightness = bright;
  pio = pio0;

  uint offset = pio_add_program(pio, &ws2812_program);
  ws2812_program_init(pio, 0, offset, pin, 800000, true);
}
