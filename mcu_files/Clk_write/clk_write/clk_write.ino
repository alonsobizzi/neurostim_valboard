#include <driver/gpio.h>
#include <driver/ledc.h>

#define CLOCK_PIN GPIO_NUM_9   // SCL pin on FeatherS2
#define CLOCK_FREQ 5000000    // 10 MHz

void setup() {
  // Configure GPIO with maximum drive and fast slew rate
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << CLOCK_PIN);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);
  
  // Set maximum drive strength (40mA)
  gpio_set_drive_capability(CLOCK_PIN, GPIO_DRIVE_CAP_3);
  
  // Configure LEDC timer
  ledc_timer_config_t timer_conf = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_1_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = CLOCK_FREQ,
    .clk_cfg = LEDC_USE_APB_CLK
  };
  ledc_timer_config(&timer_conf);
  
  // Configure LEDC channel
  ledc_channel_config_t channel_conf = {
    .gpio_num = CLOCK_PIN,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .duty = 1,
    .hpoint = 0
  };
  ledc_channel_config(&channel_conf);
}

void loop() {
  // Clock runs continuously
}