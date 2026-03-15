#include"stm32f401xe.h"
#include"stm32f4xx_hal.h"
#include "LCD.h"

int main(void) {
	HAL_Init();

	LCDInit();
	LCDSendAString("Harish");
	LCDSetCursor(2, 2);
	char msg[] = "STM32F401RE   "; // add spaces for smooth scroll
	int len = strlen(msg);

	LCDSendAString("Harish");   // Line 1 fixed
	while (1U) {
		for (int offset = 0; offset < len; offset++) {
			LCDSetCursor(2, 0);              // Start of line 2
			for (int j = 0; j < 15; j++) {   // Assuming 16x2 LCD
				LCDSendCharacter(msg[(offset + j) % len]);
			}
			HAL_Delay(500);
		}
	}
}
