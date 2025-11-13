#include <SPI.h>

// Pin definitions
#define DAC_VREF 4.98
#define SYNC_PIN 5            // GPIO5 (IO5) → DAC_SYNC
#define SCK_PIN 36            // GPIO36 (SCK) → DAC_SCLK
#define SDO_PIN 35            // GPIO35 (SDO) → DAC_DIN
#define SER_CLK_WRITE_PIN 9  // A11 GPIO12
#define SER_CLK_WRITE_EN_PIN 3 // P23 on board
#define SER_MON_EN_PIN 38
#define SER_DATA_MON_PIN 8
#define SER_DATA_WRITE_PIN 33
#define SER_CLK_MON_PIN 1
#define RESET_WRITE_PIN 10
#define LS_CH2_EN_PIN 7
#define COMM_EN_PIN 18
#define LS_CH2_DIR_PIN 14
#define LS_CH1_DIR_PIN 12
#define LS_CH1_EN_PIN 6

// DAC CMDs
#define CMD_WRITE_UPDATE 0x03  // Write to and update DAC

// DAC Registers
#define DAC_A 0x00
#define DAC_B 0x01
#define DAC_C 0x02
#define DAC_D 0x03
#define DAC_E 0x04
#define DAC_F 0x05
#define DAC_G 0x06
#define DAC_H 0x07
#define DAC_ALL 0x08

// DAC Channel
#define DAC_CH_VPWR DAC_A
#define DAC_CH_VCBREF DAC_B
#define DAC_CH_VDACREF DAC_C
#define DAC_CH_VSUP DAC_D
#define DAC_CH_V1P8 DAC_E
#define DAC_CH_VBG DAC_F
#define DAC_CH_V3P3 DAC_G
#define DAC_CH_VECOM DAC_H

// Ramp parameters
#define RAMP_TIME_MS 2500
#define RAMP_STEPS 500

// Global constants
SPISettings spi_conf(2000000, MSBFIRST, SPI_MODE1);
hw_timer_t* timer = NULL;  // Instance of a hardware timer
volatile uint32_t clk_count = 200;
volatile uint8_t clk_pin_state = 0;

void configure_spi();
void write_dac(uint8_t cmd, uint8_t address, uint16_t data);
uint16_t voltageToCode(float voltage);
void set_dac_voltage(uint8_t channel, float voltage);
void ramp_dac_voltage(uint8_t channel, float target_voltage, uint32_t ramp_time_ms, uint32_t delay_before_ramp_ms, uint16_t num_steps);

void ARDUINO_ISR_ATTR isr_func() {
  digitalWrite(SER_CLK_WRITE_PIN, clk_pin_state);
  clk_pin_state = !clk_pin_state;
  clk_count--;
}

void setup() {
  configure_spi();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SER_DATA_WRITE_PIN, OUTPUT);
  pinMode(SER_CLK_WRITE_PIN, OUTPUT);
  pinMode(SER_CLK_WRITE_EN_PIN, OUTPUT);
  digitalWrite(SER_CLK_WRITE_EN_PIN, 0); // Enable SERIAL WRITE pins
  
  // digitalWrite(SER_CLK_WRITE_PIN, 0);
  digitalWrite(LED_BUILTIN, HIGH);

  

  pinMode(LS_CH1_EN_PIN, OUTPUT);
  pinMode(LS_CH2_EN_PIN, OUTPUT);
  pinMode(SER_CLK_WRITE_EN_PIN, OUTPUT);
  pinMode(SER_MON_EN_PIN, OUTPUT);
  
  digitalWrite(LS_CH1_EN_PIN, HIGH);
  digitalWrite(LS_CH2_EN_PIN, HIGH);
  digitalWrite(SER_CLK_WRITE_EN_PIN, LOW);
  digitalWrite(SER_MON_EN_PIN, LOW);

  pinMode(LS_CH1_DIR_PIN, OUTPUT);
  pinMode(LS_CH2_DIR_PIN, OUTPUT);
  digitalWrite(LS_CH1_DIR_PIN, LOW);
  digitalWrite(LS_CH2_DIR_PIN, LOW);
  


  // Setup all of the bypass voltages with 10ms ramp and 500 steps
  ramp_dac_voltage(DAC_CH_V1P8, 1.8, RAMP_TIME_MS, 0, RAMP_STEPS);
  ramp_dac_voltage(DAC_CH_V3P3, 3.3, RAMP_TIME_MS, 0, RAMP_STEPS);
  ramp_dac_voltage(DAC_CH_VBG, 1.366, RAMP_TIME_MS, 0, RAMP_STEPS);
  ramp_dac_voltage(DAC_CH_VCBREF, 2, RAMP_TIME_MS, 15, RAMP_STEPS);
  ramp_dac_voltage(DAC_CH_VDACREF, 1, RAMP_TIME_MS, 15, RAMP_STEPS);
  ramp_dac_voltage(DAC_CH_VECOM, 1.65, RAMP_TIME_MS, 15, RAMP_STEPS);
  ramp_dac_voltage(DAC_CH_VPWR, 3.55, RAMP_TIME_MS, 0, RAMP_STEPS);
  ramp_dac_voltage(DAC_CH_VSUP, 3.55, RAMP_TIME_MS, 0, RAMP_STEPS);

  // timer = timerBegin(100000); //timer 1Mhz resolution
  // timerAttachInterrupt(timer, &isr_func);
  // timerAlarm(timer, 1, true, clk_count);
}

void loop() {
  // All voltages are set and stable
  digitalWrite(SER_CLK_WRITE_PIN, 0); 
  delay(10);

  digitalWrite(SER_CLK_WRITE_PIN, 1); 
  delay(10);
}

void configure_spi() {
  SPI.begin(SCK_PIN, -1, SDO_PIN, SYNC_PIN);
  SPI.setHwCs(true);
}

void write_dac(uint8_t cmd, uint8_t address, uint16_t data) {
  uint16_t addr16 = ((uint16_t)address) << 12;
  SPI.beginTransaction(spi_conf);
  SPI.write32(
    ((uint32_t)(0x0F & cmd) << 24) | ((uint32_t)(addr16 | (data & 0x0FFF)) << 8) | (uint32_t)0);
  SPI.endTransaction();
  delayMicroseconds(32);
}

void ramp_dac_voltage(uint8_t channel, float target_voltage, uint32_t ramp_time_ms, uint32_t delay_before_ramp_ms, uint16_t num_steps) {
  // Wait for the initial delay before starting the ramp
  delay(delay_before_ramp_ms);
  uint32_t delay_between_steps_us = (ramp_time_ms * 1000) / num_steps;
  
  for (uint16_t i = 0; i <= num_steps; i++) {
    float current_voltage = (target_voltage / num_steps) * i;
    write_dac(CMD_WRITE_UPDATE, channel, voltageToCode(current_voltage));
    if (i < num_steps) {
      delayMicroseconds(delay_between_steps_us);
    }
  }
}

uint16_t voltageToCode(float voltage) {
  if (voltage < 0) voltage = 0;
  if (voltage > DAC_VREF) voltage = DAC_VREF;
  uint32_t code = (uint32_t)((voltage / DAC_VREF) * 4095.0 + 0.5);
  if (code > 4095) code = 4095;
  return (uint16_t)code;
}

void set_dac_voltage(uint8_t channel, float voltage) {
  write_dac(CMD_WRITE_UPDATE, channel, voltageToCode(voltage));
}