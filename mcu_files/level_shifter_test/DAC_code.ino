#include "DAC_SPI.h"

// ===== Global definitions =====
SPISettings spi_conf(2000000, MSBFIRST, SPI_MODE1);
hw_timer_t* timer = NULL;
volatile uint32_t clk_count = 200;
volatile uint8_t clk_pin_state = 0;

// ===== Interrupt routine =====
void ARDUINO_ISR_ATTR isr_func() {
  digitalWrite(SER_CLK_WRITE_PIN, clk_pin_state);
  clk_pin_state = !clk_pin_state;
  clk_count--;
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  configure_spi();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SER_CLK_WRITE_PIN, OUTPUT);
  digitalWrite(SER_CLK_WRITE_PIN, 0);
  digitalWrite(LED_BUILTIN, HIGH);

  // Set DAC output voltages
  set_dac_voltage(DAC_CH_V1P8, 1.8);
  set_dac_voltage(DAC_CH_V3P3, 3.3);
  set_dac_voltage(DAC_CH_VBG, 1.366);
  set_dac_voltage(DAC_CH_VCBREF, 2.0);
  set_dac_voltage(DAC_CH_VDACREF, 1.0);
  set_dac_voltage(DAC_CH_VECOM, 1.65);
  set_dac_voltage(DAC_CH_VPWR, 3.55);
  set_dac_voltage(DAC_CH_VSUP, 3.55);

  // Optional timer setup
  // timer = timerBegin(100000); // timer 1 MHz resolution
  // timerAttachInterrupt(timer, &isr_func);
  // timerAlarm(timer, 1, true, clk_count);
}

// ===== Loop =====
void loop() {
  // Add your repeated operations here
  delay(1000);
}

// ===== SPI Configuration =====
void configure_spi() {
  SPI.begin(SCK_PIN, -1, SDO_PIN, SYNC_PIN);
  SPI.setHwCs(true);
}

// ===== DAC Write Function =====
void write_dac(uint8_t cmd, uint8_t address, uint16_t data) {
  uint16_t addr16 = ((uint16_t)address) << 12;
  SPI.beginTransaction(spi_conf);
  SPI.write32(((uint32_t)(cmd & 0x0F) << 24) |
              ((uint32_t)(addr16 | (data & 0x0FFF)) << 8));
  SPI.endTransaction();
  delayMicroseconds(32);
}

// ===== Voltage Conversion =====
uint16_t voltageToCode(float voltage) {
  if (voltage < 0) voltage = 0;
  if (voltage > DAC_VREF) voltage = DAC_VREF;
  uint32_t code = (uint32_t)((voltage / DAC_VREF) * 4095.0 + 0.5);
  if (code > 4095) code = 4095;
  return (uint16_t)code;
}

// ===== Set DAC Voltage =====
void set_dac_voltage(uint8_t channel, float voltage) {
  write_dac(CMD_WRITE_UPDATE, channel, voltageToCode(voltage));
}
