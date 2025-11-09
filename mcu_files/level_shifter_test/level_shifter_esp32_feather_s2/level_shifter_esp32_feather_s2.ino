#include <SPI.h>
#include "DAC_SPI.h"

/*LEVEL SHIFTER CODE
We start with level-shifter U9 and just use the pins that test the clock.
We currently just care about writing.

Pins we care about:
- UC_SER_WRITE_ENABLE --> S1: 23/A9/IO33 --> S2: (6th from bottom) D10/GPIO 10
- UC_SER_CLK_WRITE --> S1: 27/SCL/IO22 --> S2: SCL/GPIO 4
- SER_CLK_WRITE (only on level shifter, not in program)

This should 
1. Set the level shifter voltage to 1.8 V
2. Set the enable pin on the level shifter (check what pin from the layout)
3. Program it to test with a smaller clock

*/
// ===== Set Level Shifter pins =====
#define WRITE_ENABLE_PIN 10
#define CLK_WRITE 4

// ===== Set DAC (AD5628) pins =====
#define SYNC_PIN 5     // GPIO5 (IO5) → DAC_SYNC
#define SCK_PIN  36    // GPIO36 (SCK) → DAC_SCLK
#define SDO_PIN  35    // GPIO35 (SDO) → DAC_DIN

// ===== DAC configuration =====
//const float DAC_VREF = 5.0;
const int   DAC_BITS = 12;
const bool  DAC_REF_INTERNAL = false;

// ===== Target voltages for channels A–H ===== // We only care about 1.8V!
float dacVoltages[8] = {0, 0, 0, 0, 1.8, 0, 0, 0};

// ===== Setting the frequency ====
int CLK_FREQ = 10e6;//Hz
float CLK_PERIOD = 1/CLK_FREQ;


// ===== Now for the actual code (setup etc) ====

void setup() {
  Serial.begin(115200);
  
  // Wait for Serial connection
  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime) < 5000) {
    delay(10);
  }

  delay(1000);  // Extra delay to ensure serial is ready

  configure_spi();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SER_CLK_WRITE_PIN, OUTPUT);
  digitalWrite(SER_CLK_WRITE_PIN, 0);
  digitalWrite(LED_BUILTIN, HIGH);

  // Set DAC output voltages
  set_dac_voltage(DAC_CH_V1P8, 1.8);
  // set_dac_voltage(DAC_CH_V3P3, 3.3);
  // set_dac_voltage(DAC_CH_VBG, 1.366);
  // set_dac_voltage(DAC_CH_VCBREF, 2.0);
  // set_dac_voltage(DAC_CH_VDACREF, 1.0);
  // set_dac_voltage(DAC_CH_VECOM, 1.65);
  // set_dac_voltage(DAC_CH_VPWR, 3.55);
  // set_dac_voltage(DAC_CH_VSUP, 3.55);

  //Set the level-shifter pins
  pinMode(WRITE_ENABLE_PIN, OUTPUT);
  pinMode(CLK_WRITE, OUTPUT);

  // Enable level shifter (A→B active)
  digitalWrite(WRITE_ENABLE_PIN, LOW);



}

void loop() {
  // Continuous DAC updates every 2 seconds
  static unsigned long lastUpdate = 0;
  static unsigned long cycleCount = 0;

digitalWrite(CLK_WRITE, HIGH);
delayMicroseconds(CLK_PERIOD*1e3/2);// Convert to ms
digitalWrite(CLK_WRITE, LOW);
delayMicroseconds(CLK_PERIOD*1e3/2);


}
