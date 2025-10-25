#include <SPI.h>

// ===== Pin mapping (from your schematic) =====
#define SYNC_PIN 4    // DAC_NSYNC  (active low)
#define SCK_PIN  5    // DAC_SCLK
#define MOSI_PIN 18   // DAC_DIN

// ===== DAC configuration =====
const float DAC_VREF = 2.5;   // internal reference voltage
const int   DAC_BITS = 16;    // AD5668 = 16-bit
const bool  DAC_REF_INTERNAL = true;

// ===== Initial voltages for channels A–H =====
float dacVoltages[8] = {0.25, 0.50, 0.75, 1.00, 1.25, 1.50, 1.75, 2.00};

// ---------------------------------------------------------------------
// ----------------------  DAC INTERFACE FUNCTIONS  --------------------
// ---------------------------------------------------------------------
uint32_t buildDACCommand(uint8_t cmd, uint8_t addr, uint16_t value) {
  uint32_t frame = 0;
  frame |= ((uint32_t)cmd & 0x0F) << 24;   // command bits
  frame |= ((uint32_t)addr & 0x0F) << 20;  // address bits (channel)
  frame |= ((uint32_t)value & 0xFFFF);     // 16-bit data
  return frame;
}

void writeToDAC(uint32_t data) {
  digitalWrite(SYNC_PIN, LOW);
  SPI.transfer((data >> 24) & 0xFF);
  SPI.transfer((data >> 16) & 0xFF);
  SPI.transfer((data >> 8) & 0xFF);
  SPI.transfer(data & 0xFF);
  digitalWrite(SYNC_PIN, HIGH);
}

void enableInternalRef(bool enable) {
  uint32_t refCmd = (0x8 << 24) | (enable ? 0x1 : 0x0);
  writeToDAC(refCmd);
  delay(5);
}

uint16_t voltageToCode(float voltage) {
  if (voltage < 0) voltage = 0;
  if (voltage > DAC_VREF * 2.0) voltage = DAC_VREF * 2.0;
  return (uint16_t)((voltage / (DAC_VREF * 2.0)) * ((1UL << DAC_BITS) - 1));
}

void setDACVoltage(uint8_t channelIndex, float voltage) {
  if (channelIndex > 7) return; // 0–7 valid (A–H)
  uint16_t code = voltageToCode(voltage);
  uint32_t cmd = buildDACCommand(0x3, channelIndex, code); // write + update
  writeToDAC(cmd);
}

void setAllDACVoltages(float vals[8]) {
  for (uint8_t i = 0; i < 8; i++)
    setDACVoltage(i, vals[i]);
}

// ---------------------------------------------------------------------
// ----------------------------  SETUP  --------------------------------
// ---------------------------------------------------------------------
void setup() {
  pinMode(SYNC_PIN, OUTPUT);
  digitalWrite(SYNC_PIN, HIGH);

  SPI.begin(SCK_PIN, -1, MOSI_PIN, SYNC_PIN);
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
  delay(50);

  if (DAC_REF_INTERNAL)
    enableInternalRef(true);

  // Apply the initial voltages
  setAllDACVoltages(dacVoltages);
}

// ---------------------------------------------------------------------
// -----------------------------  LOOP  --------------------------------
// ---------------------------------------------------------------------
void loop() {
  // Nothing to do – outputs remain at the set voltages
}
