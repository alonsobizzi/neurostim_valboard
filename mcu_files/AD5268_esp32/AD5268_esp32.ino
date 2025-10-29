#include <SPI.h>

// ===== Pin mapping for FeatherS2 to AD5628 =====
#define SYNC_PIN 5     // GPIO5 (IO5) → DAC_SYNC
#define SCK_PIN  36    // GPIO36 (SCK) → DAC_SCLK
#define SDO_PIN  35    // GPIO35 (SDO) → DAC_DIN

// ===== DAC configuration =====
const float DAC_VREF = 5.0;
const int   DAC_BITS = 12;
const bool  DAC_REF_INTERNAL = false;

// ===== Target voltages for channels A–H =====
float dacVoltages[8] = {3.55, 1.2, 5.0, 3.5, 1.8, 1.366, 3.3, 1.65};

// ---------------------------------------------------------------------
// ----------------------  DAC INTERFACE FUNCTIONS  --------------------
// ---------------------------------------------------------------------

uint32_t buildDACCommand(uint8_t cmd, uint8_t addr, uint16_t value) {
  uint32_t frame = 0;
  frame |= ((uint32_t)cmd & 0x0F) << 24;
  frame |= ((uint32_t)addr & 0x0F) << 20;
  frame |= ((uint32_t)value & 0x0FFF) << 8;
  return frame;
}

void writeToDAC(uint32_t data) {
  Serial.print("  -> SYNC going LOW...");
  digitalWrite(SYNC_PIN, LOW);
  Serial.println(" [LOW]");
  delayMicroseconds(50);  // Longer delay for scope triggering
  
  Serial.print("  -> Transferring SPI data: 0x");
  Serial.print(data, HEX);
  Serial.println();
  
  uint8_t byte1 = (data >> 24) & 0xFF;
  uint8_t byte2 = (data >> 16) & 0xFF;
  uint8_t byte3 = (data >> 8) & 0xFF;
  uint8_t byte4 = data & 0xFF;
  
  Serial.print("     Byte 1: 0x");
  Serial.println(byte1, HEX);
  SPI.transfer(byte1);
  
  Serial.print("     Byte 2: 0x");
  Serial.println(byte2, HEX);
  SPI.transfer(byte2);
  
  Serial.print("     Byte 3: 0x");
  Serial.println(byte3, HEX);
  SPI.transfer(byte3);
  
  Serial.print("     Byte 4: 0x");
  Serial.println(byte4, HEX);
  SPI.transfer(byte4);
  
  delayMicroseconds(50);  // Longer delay for scope
  Serial.print("  -> SYNC going HIGH...");
  digitalWrite(SYNC_PIN, HIGH);
  Serial.println(" [HIGH]");
  delayMicroseconds(100);
}

void enableInternalRef(bool enable) {
  Serial.println("\n--- Internal Reference Configuration ---");
  Serial.print("Command: ");
  Serial.println(enable ? "ENABLE (0x1)" : "DISABLE (0x0)");
  
  uint32_t refCmd = buildDACCommand(0x8, 0x0, enable ? 0x1 : 0x0);
  Serial.print("Command frame: 0x");
  Serial.println(refCmd, HEX);
  
  writeToDAC(refCmd);
  delay(5);
  Serial.println("--- Reference Configuration Complete ---\n");
}

uint16_t voltageToCode(float voltage) {
  if (voltage < 0) voltage = 0;
  if (voltage > DAC_VREF) voltage = DAC_VREF;
  uint32_t code = (uint32_t)((voltage / DAC_VREF) * 4095.0 + 0.5);
  if (code > 4095) code = 4095;
  return (uint16_t)code;
}

void setDACVoltage(uint8_t channelIndex, float voltage) {
  if (channelIndex > 7) return;
  
  uint16_t code = voltageToCode(voltage);
  
  Serial.println();
  Serial.print(">>> Setting Channel ");
  Serial.print(char('A' + channelIndex));
  Serial.println(" <<<");
  Serial.print("  Target voltage: ");
  Serial.print(voltage, 3);
  Serial.println(" V");
  Serial.print("  DAC code (dec): ");
  Serial.println(code);
  Serial.print("  DAC code (hex): 0x");
  Serial.println(code, HEX);
  Serial.print("  DAC code (bin): ");
  for (int b = 11; b >= 0; b--) {
    Serial.print((code & (1 << b)) ? '1' : '0');
  }
  Serial.println();
  
  uint32_t cmd = buildDACCommand(0x3, channelIndex, code);
  Serial.print("  Full command: 0x");
  Serial.println(cmd, HEX);
  
  writeToDAC(cmd);
  delay(10);
}

void setAllDACVoltages(float vals[8]) {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║  Setting All DAC Channels              ║");
  Serial.println("╚════════════════════════════════════════╝");
  
  for (uint8_t i = 0; i < 8; i++) {
    setDACVoltage(i, vals[i]);
    delay(50);  // Delay between channels for easier scope capture
  }
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║  All Channels Set Complete             ║");
  Serial.println("╚════════════════════════════════════════╝\n");
}

void resetDAC() {
  Serial.println("\n--- DAC Reset ---");
  Serial.println("Sending reset command (0x7)...");
  
  uint32_t resetCmd = buildDACCommand(0x7, 0x0, 0x0);
  Serial.print("Reset command frame: 0x");
  Serial.println(resetCmd, HEX);
  
  writeToDAC(resetCmd);
  delay(10);
  Serial.println("--- DAC Reset Complete ---\n");
}

// Test function to manually toggle SYNC
void testSyncPin() {
  Serial.println("\n");
  Serial.println("╔═══════════════════════════════════════════╗");
  Serial.println("║       SYNC PIN HARDWARE TEST              ║");
  Serial.println("║  Watch GPIO5 on oscilloscope!             ║");
  Serial.println("╚═══════════════════════════════════════════╝");
  Serial.println();
  
  Serial.println("Toggling SYNC pin 10 times (200ms period)...");
  Serial.println("This should be VERY easy to see on scope!");
  Serial.println();
  
  for (int i = 0; i < 10; i++) {
    Serial.print("  Toggle #");
    Serial.print(i + 1);
    Serial.print(": ");
    
    digitalWrite(SYNC_PIN, LOW);
    Serial.print("LOW");
    delay(100);  // 100ms LOW
    
    digitalWrite(SYNC_PIN, HIGH);
    Serial.print(" → HIGH");
    Serial.println();
    delay(100);  // 100ms HIGH
  }
  
  Serial.println();
  Serial.println("╔═══════════════════════════════════════════╗");
  Serial.println("║  SYNC PIN TEST COMPLETE                   ║");
  Serial.println("║  If you didn't see 10 pulses on GPIO5,   ║");
  Serial.println("║  check your oscilloscope connection!     ║");
  Serial.println("╚═══════════════════════════════════════════╝");
  Serial.println();
}

// Test function to toggle SCK manually
void testSCKPin() {
  Serial.println("\n");
  Serial.println("╔═══════════════════════════════════════════╗");
  Serial.println("║       SCK PIN HARDWARE TEST               ║");
  Serial.println("║  Watch GPIO36 on oscilloscope!            ║");
  Serial.println("╚═══════════════════════════════════════════╝");
  Serial.println();
  
  Serial.println("Toggling SCK pin 10 times (200ms period)...");
  Serial.println();
  
  pinMode(SCK_PIN, OUTPUT);
  
  for (int i = 0; i < 10; i++) {
    Serial.print("  Toggle #");
    Serial.print(i + 1);
    Serial.print(": ");
    
    digitalWrite(SCK_PIN, LOW);
    Serial.print("LOW");
    delay(100);
    
    digitalWrite(SCK_PIN, HIGH);
    Serial.print(" → HIGH");
    Serial.println();
    delay(100);
  }
  
  Serial.println();
  Serial.println("╔═══════════════════════════════════════════╗");
  Serial.println("║  SCK PIN TEST COMPLETE                    ║");
  Serial.println("╚═══════════════════════════════════════════╝");
  Serial.println();
}

// Test function to toggle SDO manually
void testSDOPin() {
  Serial.println("\n");
  Serial.println("╔═══════════════════════════════════════════╗");
  Serial.println("║       SDO PIN HARDWARE TEST               ║");
  Serial.println("║  Watch GPIO35 on oscilloscope!            ║");
  Serial.println("╚═══════════════════════════════════════════╝");
  Serial.println();
  
  Serial.println("Toggling SDO pin 10 times (200ms period)...");
  Serial.println();
  
  pinMode(SDO_PIN, OUTPUT);
  
  for (int i = 0; i < 10; i++) {
    Serial.print("  Toggle #");
    Serial.print(i + 1);
    Serial.print(": ");
    
    digitalWrite(SDO_PIN, LOW);
    Serial.print("LOW");
    delay(100);
    
    digitalWrite(SDO_PIN, HIGH);
    Serial.print(" → HIGH");
    Serial.println();
    delay(100);
  }
  
  Serial.println();
  Serial.println("╔═══════════════════════════════════════════╗");
  Serial.println("║  SDO PIN TEST COMPLETE                    ║");
  Serial.println("╚═══════════════════════════════════════════╝");
  Serial.println();
}

// ---------------------------------------------------------------------
// ----------------------------  SETUP  --------------------------------
// ---------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  
  // Wait for Serial connection
  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime) < 5000) {
    delay(10);
  }
  
  delay(1000);  // Extra delay to ensure serial is ready
  
  Serial.println("\n\n\n");
  Serial.println("╔═══════════════════════════════════════════╗");
  Serial.println("║                                           ║");
  Serial.println("║     AD5628 DAC - COMPLETE DEBUG MODE     ║");
  Serial.println("║          FeatherS2 ESP32-S2               ║");
  Serial.println("║                                           ║");
  Serial.println("╚═══════════════════════════════════════════╝");
  Serial.println();
  
  Serial.println("Hardware Configuration:");
  Serial.println("┌─────────────────────────────────────────┐");
  Serial.print("│ SYNC (CS):  GPIO");
  Serial.print(SYNC_PIN);
  Serial.println(" (IO5)              │");
  Serial.print("│ SCK:        GPIO");
  Serial.print(SCK_PIN);
  Serial.println(" (Hardware SPI)     │");
  Serial.print("│ SDO (MOSI): GPIO");
  Serial.print(SDO_PIN);
  Serial.println(" (Hardware SPI)     │");
  Serial.println("└─────────────────────────────────────────┘");
  Serial.println();
  
  // Configure SYNC pin first
  Serial.println("Step 1: Configuring SYNC pin...");
  pinMode(SYNC_PIN, OUTPUT);
  digitalWrite(SYNC_PIN, HIGH);
  Serial.println("  ✓ SYNC pin configured as OUTPUT");
  Serial.println("  ✓ SYNC initialized to HIGH");
  Serial.println();
  
  delay(1000);
  
  // Test SYNC pin manually
  testSyncPin();
  delay(2000);
  
  // Test SCK pin manually
  testSCKPin();
  delay(2000);
  
  // Test SDO pin manually
  testSDOPin();
  delay(2000);
  
  // Now initialize SPI
  Serial.println("Step 2: Initializing SPI interface...");
  SPI.begin(SCK_PIN, -1, SDO_PIN, -1);
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
  Serial.println("  ✓ SPI initialized");
  Serial.println("  ✓ Clock: 2 MHz");
  Serial.println("  ✓ Mode: SPI_MODE1 (CPOL=0, CPHA=1)");
  Serial.println("  ✓ Bit order: MSB First");
  Serial.println();
  
  delay(500);
  
  Serial.println("╔═══════════════════════════════════════════╗");
  Serial.println("║  Hardware tests complete!                 ║");
  Serial.println("║  Starting continuous DAC updates...       ║");
  Serial.println("║  Updates every 2 seconds                  ║");
  Serial.println("╚═══════════════════════════════════════════╝");
  Serial.println();
  
  delay(2000);
}

// ---------------------------------------------------------------------
// -----------------------------  LOOP  --------------------------------
// ---------------------------------------------------------------------

void loop() {
  static unsigned long lastUpdate = 0;
  static unsigned long cycleCount = 0;
  
  if (millis() - lastUpdate >= 2000) {  // Every 2 seconds
    cycleCount++;
    
    Serial.println("\n\n");
    Serial.println("████████████████████████████████████████████");
    Serial.print("█████    UPDATE CYCLE #");
    Serial.print(cycleCount);
    Serial.println("               █████");
    Serial.println("████████████████████████████████████████████");
    Serial.println();
    
    // Step 1: Reset DAC
    resetDAC();
    delay(100);
    
    // Step 2: Configure reference
    enableInternalRef(false);
    delay(100);
    
    // Step 3: Set all channels
    setAllDACVoltages(dacVoltages);
    
    Serial.println("\n");
    Serial.println("████████████████████████████████████████████");
    Serial.println("█████    CYCLE COMPLETE                █████");
    Serial.println("█████    SYNC toggled 10 times         █████");
    Serial.println("█████    Watch GPIO5 on oscilloscope!  █████");
    Serial.println("████████████████████████████████████████████");
    Serial.println();
    
    lastUpdate = millis();
  }
  
  delay(10);
}
