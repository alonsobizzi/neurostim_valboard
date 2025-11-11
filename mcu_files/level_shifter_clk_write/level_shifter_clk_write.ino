//#include <SPI.h>
#include "DAC_CLK_SPI.h"
#include <driver/gpio.h>
#include <driver/ledc.h>

// ===== Level Shifter pins =====
#define WRITE_ENABLE_PIN 10
#define CLK_WRITE 9

// ===== DAC (AD5628) pins =====
// #define SYNC_PIN 5
// #define SCK_PIN 36
// #define SDO_PIN 35

// ===== DAC configuration =====
const int DAC_BITS = 12;
const bool DAC_REF_INTERNAL = false;

// ===== Target voltages =====
float dacVoltages[8] = {0, 0, 0, 0, 1.8, 0, 0, 0};

// ===== Clock setup =====
// int CLK_FREQ = 10e6;  // Hz
// float CLK_PERIOD = 1.0 / 10e6; // seconds

// ==== Shrey Clock Code =====
#define CLOCK_PIN GPIO_NUM_9   // SCL pin on FeatherS2
#define CLOCK_FREQ 5000000    // 10 MHz



void setup() {
  Serial.begin(115200);
  Serial.println("\n=== BEGIN SETUP ===");

  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime) < 5000) {
    delay(10);
  }
  delay(1000);
  Serial.println("[INFO] Serial ready.");

  // SPI configuration
  Serial.println("[INFO] Configuring SPI...");
  configure_spi();
  Serial.println("[DONE] SPI configured.");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("[INFO] LED set HIGH (power indicator).");

  // DAC configuration
  Serial.println("[INFO] Setting DAC output voltages...");
  set_dac_voltage(DAC_CH_V1P8, 1.8);
  Serial.println("[DONE] DAC set to 1.8V on V1P8 channel.");

  // Level-shifter pin configuration
  Serial.println("[INFO] Configuring level-shifter pins...");
  pinMode(WRITE_ENABLE_PIN, OUTPUT);
  pinMode(CLK_WRITE, OUTPUT);

  // Enable level shifter (A→B active)
  digitalWrite(WRITE_ENABLE_PIN, LOW);
  Serial.println("[INFO] Level shifter enabled (WRITE_ENABLE_PIN=LOW).");

  // Configure GPIO with maximum drive and fast slew
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << CLK_WRITE);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  esp_err_t gpio_status = gpio_config(&io_conf);
  Serial.printf("[GPIO] Configured CLK_WRITE pin. Status: %d\n", gpio_status);

  // Set drive strength
  gpio_set_drive_capability((gpio_num_t)CLK_WRITE, GPIO_DRIVE_CAP_3);
  Serial.println("[GPIO] Drive capability set to 40mA.");

  // LEDC timer config
  Serial.printf("[INFO] Configuring LEDC timer at %d Hz...\n", CLOCK_FREQ);
  ledc_timer_config_t timer_conf = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_1_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = CLOCK_FREQ,
    .clk_cfg = LEDC_USE_APB_CLK
  };
  esp_err_t timer_status = ledc_timer_config(&timer_conf);
  Serial.printf("[LEDC] Timer configured. Status: %d\n", timer_status);

  // LEDC channel config
  ledc_channel_config_t channel_conf = {
    .gpio_num = CLK_WRITE,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .duty = 1,
    .hpoint = 0
  };

  esp_err_t channel_status = ledc_channel_config(&channel_conf);
  Serial.printf("[LEDC] Channel configured. Status: %d\n", channel_status);

  Serial.println("=== SETUP COMPLETE ===\n");
}

void loop() {
  // Optional: blink LED or check if clock output is active
  // static bool led_state = false;
  // led_state = !led_state;
  // digitalWrite(LED_BUILTIN, led_state);
  // Serial.printf("[DEBUG] Loop alive. LED=%d\n", led_state);
  // delay(1000);
}
