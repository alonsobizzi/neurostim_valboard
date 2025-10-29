#include <SPI.h>

#define DAC_VREF 4.98
#define SYNC_PIN 5  // GPIO5 (IO5) → DAC_SYNC
#define SCK_PIN 36  // GPIO36 (SCK) → DAC_SCLK
#define SDO_PIN 35  // GPIO35 (SDO) → DAC_DIN

// DAC CMDs
#define CMD_WRITE_UPDATE 0x03 // Write to and update DAC

// DAC Registers
#define DAC_A   0x00
#define DAC_B   0x01
#define DAC_C   0x02
#define DAC_D   0x03
#define DAC_E   0x04
#define DAC_F   0x05
#define DAC_G   0x06
#define DAC_H   0x07
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

SPISettings spi_conf(2000000, MSBFIRST, SPI_MODE1);

void configure_spi();
void write_dac(uint8_t cmd, uint8_t address, uint16_t data);
uint16_t voltageToCode(float voltage);
void set_dac_voltage(uint8_t channel, float voltage);

void setup() {
  configure_spi();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Setup all of the bypasses
  set_dac_voltage(DAC_CH_V1P8, 1.8);
  set_dac_voltage(DAC_CH_V3P3, 3.3);
  set_dac_voltage(DAC_CH_VBG, 1.366);
  set_dac_voltage(DAC_CH_VCBREF, 2);
  set_dac_voltage(DAC_CH_VDACREF, 1);
  set_dac_voltage(DAC_CH_VECOM, 1.65);
  set_dac_voltage(DAC_CH_VPWR, 3.55);
  set_dac_voltage(DAC_CH_VSUP, 3.55);
}
float dac_vout = 0.0;
uint8_t dac_channel = 0;
void loop() {

}

void configure_spi() {
  SPI.begin(SCK_PIN, -1, SDO_PIN, SYNC_PIN);
  SPI.setHwCs(true);
}

void write_dac(uint8_t cmd, uint8_t address, uint16_t data) {
  uint16_t addr16 = ((uint16_t) address) << 12;
  SPI.beginTransaction(spi_conf);
  SPI.write32(
    ((uint32_t)(0x0F & cmd) << 24) |
    ((uint32_t)(addr16 | (data & 0x0FFF)) << 8) |
    (uint32_t)0
  );
  SPI.endTransaction();
  delayMicroseconds(32);
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