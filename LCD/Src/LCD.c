/*
 * LCD.c
 *
 *  Created on: Mar 14, 2026
 *      Author: Harish
 */

#include "LCD.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

// ---------------- GPIO Helper Functions ----------------

// Configure a pin as output (push-pull, very high speed, no pull-up/down).
// Enables the RCC clock for the port, then sets MODER, OTYPER, OSPEEDR, and PUPDR.
void GPIO_OUTPUT(GPIO_TypeDef *Port, uint32_t Pin) {
	// --- Enable RCC clock for the given port ---
	if (Port == GPIOA)
		RCC->AHB1ENR |= (1UL << 0);
	else if (Port == GPIOB)
		RCC->AHB1ENR |= (1UL << 1);
	else if (Port == GPIOC)
		RCC->AHB1ENR |= (1UL << 2);
	else if (Port == GPIOD)
		RCC->AHB1ENR |= (1UL << 3);
	else if (Port == GPIOE)
		RCC->AHB1ENR |= (1UL << 4);
	else if (Port == GPIOH)
		RCC->AHB1ENR |= (1UL << 7);

	// --- Configure pin as output mode ---
	Port->MODER &= ~(0x3UL << (2U * Pin)); // Clear mode bits
	Port->MODER |= (0x1UL << (2U * Pin));  // Set mode = 01 (output)

	// --- Output type: push-pull ---
	Port->OTYPER &= ~(0x1UL << Pin);

	// --- Output speed: very high ---
	Port->OSPEEDR &= ~(0x3UL << (2U * Pin));
	Port->OSPEEDR |= (0x3UL << (2U * Pin));

	// --- No pull-up/pull-down ---
	Port->PUPDR &= ~(0x3UL << (2U * Pin));
}

// Drive pin HIGH (logic 1) using BSRR register
void GPIO_HIGH(GPIO_TypeDef *Port, uint32_t Pin) {
	Port->BSRR = (1UL << Pin);
}

// Drive pin LOW (logic 0) using BSRR register
void GPIO_LOW(GPIO_TypeDef *Port, uint32_t Pin) {
	Port->BSRR = (1UL << (Pin + 16U));
}

// Write a single bit value to a pin (helper for mapping bytes to LCD pins)
void SendBitToPortAndPin(GPIO_TypeDef *Port, uint32_t Pin, uint32_t bitTest) {
	if (bitTest)
		GPIO_HIGH(Port, Pin);
	else
		GPIO_LOW(Port, Pin);
}

// ---------------- LCD Helper Functions ----------------

// Map each bit of a byte to LCD data pins D0–D7.
// This drives the parallel bus with either a command or character.
void SendCharacterToLCDDataPins(char character) {
	SendBitToPortAndPin(LCD_D0_Port, LCD_D0_Pin, character & 0b00000001);
	SendBitToPortAndPin(LCD_D1_Port, LCD_D1_Pin, character & 0b00000010);
	SendBitToPortAndPin(LCD_D2_Port, LCD_D2_Pin, character & 0b00000100);
	SendBitToPortAndPin(LCD_D3_Port, LCD_D3_Pin, character & 0b00001000);
	SendBitToPortAndPin(LCD_D4_Port, LCD_D4_Pin, character & 0b00010000);
	SendBitToPortAndPin(LCD_D5_Port, LCD_D5_Pin, character & 0b00100000);
	SendBitToPortAndPin(LCD_D6_Port, LCD_D6_Pin, character & 0b01000000);
	SendBitToPortAndPin(LCD_D7_Port, LCD_D7_Pin, character & 0b10000000);
}

// Initialize all LCD pins (data + control) as outputs
void InitializePinsForLCD(void) {
	// Data pins
	GPIO_OUTPUT(LCD_D0_Port, LCD_D0_Pin);
	GPIO_OUTPUT(LCD_D1_Port, LCD_D1_Pin);
	GPIO_OUTPUT(LCD_D2_Port, LCD_D2_Pin);
	GPIO_OUTPUT(LCD_D3_Port, LCD_D3_Pin);
	GPIO_OUTPUT(LCD_D4_Port, LCD_D4_Pin);
	GPIO_OUTPUT(LCD_D5_Port, LCD_D5_Pin);
	GPIO_OUTPUT(LCD_D6_Port, LCD_D6_Pin);
	GPIO_OUTPUT(LCD_D7_Port, LCD_D7_Pin);

	// Control pins
	GPIO_OUTPUT(LCD_E_Port, LCD_E_Pin);   // Enable
	GPIO_OUTPUT(LCD_RW_Port, LCD_RW_Pin); // Read/Write
	GPIO_OUTPUT(LCD_RS_Port, LCD_RS_Pin); // Register Select
}

// Send a command to LCD (RS=0, RW=0)
void LCDSendCommand(uint8_t cmd) {
	GPIO_LOW(LCD_RS_Port, LCD_RS_Pin); // RS=0 → command mode
	GPIO_LOW(LCD_RW_Port, LCD_RW_Pin); // RW=0 → write mode

	SendCharacterToLCDDataPins(cmd);
	PulseEnable();                     // Latch with Enable pulse
}

// Send data (character) to LCD (RS=1, RW=0)
void LCDSendData(uint8_t data) {
	GPIO_HIGH(LCD_RS_Port, LCD_RS_Pin); // RS=1 → data mode
	GPIO_LOW(LCD_RW_Port, LCD_RW_Pin);  // RW=0 → write mode

	SendCharacterToLCDDataPins(data);
	PulseEnable();

	HAL_Delay(1); // Small delay for data write (~43 µs)
}

// Generate Enable pulse to latch command/data.
// LCD captures data/command on the falling edge of E.
void PulseEnable(void) {
	GPIO_HIGH(LCD_E_Port, LCD_E_Pin);   // E=1 → start pulse
	HAL_Delay(1);                       // Hold high (datasheet: ≥450 ns)
	GPIO_LOW(LCD_E_Port, LCD_E_Pin);    // E=0 → end pulse
}

// Send a full string to the LCD
void LCDSendAString(char *msg) {
	for (int i = 0; msg[i] != '\0'; i++) {
		LCDSendData(msg[i]);
	}
}

// Convert integer to string and send to LCD
void LCDSendInt(int value) {
	char buffer[12];              // enough for 32-bit int
	sprintf(buffer, "%d", value);
	LCDSendAString(buffer);
}

// Convert float to string with given precision and send to LCD
void LCDSendFloat(float value, int precision) {
	char buffer[20];
	sprintf(buffer, "%.*f", precision, value);
	LCDSendAString(buffer);
}

// Move cursor to specified line and column
// Uses DDRAM addresses: line 1 = 0x00, line 2 = 0x40
void LCDSetCursor(uint8_t line, uint8_t col) {
	uint8_t address;
	if (line == 1)
		address = 0x00 + col;
	else if (line == 2)
		address = 0x40 + col;
	else
		return; // invalid line

	LCDSendCommand(0x80 | address); // 0x80 = Set DDRAM address
}

// Send a single character to the LCD
void LCDSendCharacter(char character) {
	GPIO_HIGH(LCD_RS_Port, LCD_RS_Pin); // RS=1 → data mode
	GPIO_LOW(LCD_RW_Port, LCD_RW_Pin);  // RW=0 → write mode

	SendCharacterToLCDDataPins(character);
	PulseEnable();
}

// Perform the canonical HD44780 LCD initialization (8-bit mode).
// This follows datasheet timing requirements to guarantee reliable startup.
void LCDInit(void) {
	InitializePinsForLCD();

	// Step 1: Wait after power-up
	HAL_Delay(40); // >30 ms

	// Step 2: Function set (8-bit interface)
	LCDSendCommand(LCD_FUNCTION_SET_8BIT);
	HAL_Delay(5); // >4.1 ms

	// Step 3: Repeat function set
	LCDSendCommand(LCD_FUNCTION_SET_8BIT);
	HAL_Delay(1); // >100 µs

	// Step 4: Final function set
	LCDSendCommand(LCD_FUNCTION_SET_8BIT);

	// Step 5: Display ON/OFF control
	LCDSendCommand(LCD_DISPLAY_ON_CURSOR_ON_BF);
	HAL_Delay(1);

	// Step 6: Clear display
	LCDSendCommand(LCD_CLEAR_DISPLAY);
	HAL_Delay(2); // >1.53 ms

	// Step 7: Entry mode set
	LCDSendCommand(LCD_ENTRY_MODE_INC);
	HAL_Delay(1);
}
