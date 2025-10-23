#include <SPI.h>

// ===== Pin assignments for Adafruit HUZZAH32 ESP32 =====
#define SYNC_PIN 5   // DAC SYNC (acts as Chip Select)
#define LDAC_PIN 4   // DAC LDAC (optional)
#define CLR_PIN  2   // DAC CLR (optional)

// ===== Function to send 32-bit command to DAC =====
void writeToDAC(uint32_t data) {
  digitalWrite(SYNC_PIN, LOW);
  SPI.transfer((data >> 24) & 0xFF);
  SPI.transfer((data >> 16) & 0xFF);
  SPI.transfer((data >> 8)  & 0xFF);
  SPI.transfer(data & 0xFF);
  digitalWrite(SYNC_PIN, HIGH);
}

// ===== Build DAC command frame =====
uint32_t buildDACCommand(uint8_t cmd, uint8_t addr, uint16_t value) {
  uint32_t frame = 0;
  frame |= ((uint32_t)cmd & 0x0F) << 24;   // C3..C0
  frame |= ((uint32_t)addr & 0x0F) << 20;  // A3..A0
  frame |= ((uint32_t)value & 0xFFFF);     // Data (16-bit)
  return frame;
}

void setup() {
  pinMode(SYNC_PIN, OUTPUT);
  pinMode(LDAC_PIN, OUTPUT);
  pinMode(CLR_PIN, OUTPUT);

  digitalWrite(SYNC_PIN, HIGH);
  digitalWrite(LDAC_PIN, HIGH);
  digitalWrite(CLR_PIN, HIGH);

  // Initialize SPI on default VSPI bus
  SPI.begin(18, 19, 23, SYNC_PIN); // SCK=18, MISO=19, MOSI=23
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
  delay(50);

  // Enable internal reference (Command 1000, DB0=1)
  uint32_t refCmd = (0x8 << 24) | 0x1;
  writeToDAC(refCmd);
  delay(10);

  // Example: set DAC A (Address 0x0) to midscale
  uint16_t value = 0x8000;
  uint32_t cmd = buildDACCommand(0x3, 0x0, value);
  writeToDAC(cmd);

  // Toggle LDAC to update output
  digitalWrite(LDAC_PIN, LOW);
  delayMicroseconds(1);
  digitalWrite(LDAC_PIN, HIGH);
}

void loop() {
  // Sweep DAC A output for demo
  for (uint16_t val = 0; val < 0xFFFF; val += 1024) {
    uint32_t cmd = buildDACCommand(0x3, 0x0, val);
    writeToDAC(cmd);
    digitalWrite(LDAC_PIN, LOW);
    delayMicroseconds(1);
    digitalWrite(LDAC_PIN, HIGH);
    delay(20);
  }
}