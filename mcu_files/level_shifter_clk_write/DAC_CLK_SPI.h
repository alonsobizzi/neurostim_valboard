#ifndef DAC_CLK_SPI_H
#define DAC_CLK_SPI_H

#include <Arduino.h>
#include <SPI.h>

// ==================== DAC CONSTANTS ==========================
#define DAC_VREF 4.98

// ===== Pin mapping =====
#define SYNC_PIN 5
#define SCK_PIN 36
#define SDO_PIN 35
#define SER_CLK_WRITE_PIN 9
#define SER_CLK_WRITE_EN_PIN 3
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

// ===== DAC Commands =====
#define CMD_WRITE_UPDATE 0x03  // Write to and update DAC

// ===== DAC Registers =====
#define DAC_A 0x00
#define DAC_B 0x01
#define DAC_C 0x02
#define DAC_D 0x03
#define DAC_E 0x04
#define DAC_F 0x05
#define DAC_G 0x06
#define DAC_H 0x07
#define DAC_ALL 0x08

// ===== Logical Channel Names =====
#define DAC_CH_VPWR   DAC_A
#define DAC_CH_VCBREF DAC_B
#define DAC_CH_VDACREF DAC_C
#define DAC_CH_VSUP   DAC_D
#define DAC_CH_V1P8   DAC_E
#define DAC_CH_VBG    DAC_F
#define DAC_CH_V3P3   DAC_G
#define DAC_CH_VECOM  DAC_H




// ===== Global SPI objects =====
extern SPISettings spi_conf;
extern hw_timer_t* timer;
extern volatile uint32_t clk_count;
extern volatile uint8_t clk_pin_state;

// ===== Function prototypes =====
void configure_spi();
void write_dac(uint8_t cmd, uint8_t address, uint16_t data);
uint16_t voltageToCode(float voltage);
void set_dac_voltage(uint8_t channel, float voltage);
void ARDUINO_ISR_ATTR isr_func();



#endif // DAC_SPI_H
