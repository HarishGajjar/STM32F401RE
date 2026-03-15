/*
 * LCD.h
 *
 *  Created on: Mar 14, 2026
 *      Author: Harish
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "stm32f401xe.h"

// --- LCD data pin mappings ---
// STM32 GPIO pins connected to LCD data lines D0–D7.
// Adjust these definitions if wiring changes.
#define LCD_D0_Pin 5   // LCD D0 -> PA5
#define LCD_D0_Port GPIOA
#define LCD_D1_Pin 6   // LCD D1 -> PA6
#define LCD_D1_Port GPIOA
#define LCD_D2_Pin 7   // LCD D2 -> PA7
#define LCD_D2_Port GPIOA
#define LCD_D3_Pin 6   // LCD D3 -> PB6
#define LCD_D3_Port GPIOB
#define LCD_D4_Pin 7   // LCD D4 -> PC7
#define LCD_D4_Port GPIOC
#define LCD_D5_Pin 9   // LCD D5 -> PA9
#define LCD_D5_Port GPIOA
#define LCD_D6_Pin 11  // LCD D6 -> PA11
#define LCD_D6_Port GPIOA
#define LCD_D7_Pin 12  // LCD D7 -> PA12
#define LCD_D7_Port GPIOA

// --- LCD control pins ---
// Control signals for the LCD interface.
#define LCD_E_Pin 0    // Enable -> PC0 (latches data on falling edge)
#define LCD_E_Port GPIOC
#define LCD_RW_Pin 1   // R/W -> PC1 (0 = write, 1 = read)
#define LCD_RW_Port GPIOC
#define LCD_RS_Pin 2   // RS -> PC2 (0 = command, 1 = data)
#define LCD_RS_Port GPIOC

// --- LCD command macros ---
// Predefined command codes for common LCD operations.
// These values come directly from the HD44780 datasheet.
#define LCD_FUNCTION_SET_8BIT        0x38  // 8-bit interface, 2-line display, 5x8 font
#define LCD_DISPLAY_ON_CURSOR_BLINK  0x0F  // Display ON, cursor ON, blink ON
#define LCD_ENTRY_MODE_INC           0x06  // Entry mode: increment cursor after each write
#define LCD_CLEAR_DISPLAY            0x01  // Clear display and reset cursor
#define LCD_RETURN_HOME              0x02  // Return cursor to home (line 1, col 0)
#define LCD_DISPLAY_ON               0x0C  // Display ON, cursor OFF
#define LCD_DISPLAY_ON_CURSOR_ON_BF  0x0E  // Display ON, cursor ON, blink OFF

// --- LCD shift commands ---
// Commands to shift the entire display left or right.
// Note: HD44780 shifts both rows together, not individually.
#define LCD_SHIFT_DISPLAY_LEFT   0x18  // Shift entire display left
#define LCD_SHIFT_DISPLAY_RIGHT  0x1C  // Shift entire display right

// --- GPIO helper functions ---
// Abstract GPIO operations for LCD control.
// These make the LCD driver portable and easier to read.
void GPIO_OUTPUT(GPIO_TypeDef *Port, uint32_t Pin);   // Configure pin as output
void GPIO_HIGH(GPIO_TypeDef *Port, uint32_t Pin);    // Drive pin HIGH (logic 1)
void GPIO_LOW(GPIO_TypeDef *Port, uint32_t Pin);      // Drive pin LOW (logic 0)
void SendBitToPortAndPin(GPIO_TypeDef *Port, uint32_t Pin, uint32_t bitTest); // Write a single bit to a pin

// --- LCD data/control helpers ---
// Low-level routines to send data/commands and manage LCD timing.
void SendCharacterToLCDDataPins(char character);   // Map byte to LCD D0–D7 pins
void InitializePinsForLCD(void);                 // Initialize all LCD GPIO pins
void PulseEnable(void);                    // Generate Enable pulse (E HIGH→LOW)
void LCDSendCommand(uint8_t cmd);                  // Send command (RS=0, RW=0)
void LCDSendData(uint8_t data);                    // Send data (RS=1, RW=0)

// --- High-level LCD functions ---
// User-friendly functions for printing and cursor control.
void LCDSendAString(char *msg);              // Print a string at current cursor
void LCDSendInt(int value);                        // Print an integer value
void LCDSendFloat(float value, int precision); // Print a float with given precision
void LCDSetCursor(uint8_t line, uint8_t col); // Move cursor to specified line/column
void LCDSendCharacter(char character);             // Print a single character
void LCDInit(void);                       // Initialize LCD (canonical sequence)
#endif /* INC_LCD_H_ */
