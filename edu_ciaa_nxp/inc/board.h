#include "chip.h"
#include <string.h>
#include <stdio.h>

#define DEBUGINIT() Board_Debug_Init()
#define DEBUGOUT(...) printf(__VA_ARGS__)
#define DEBUGSTR(str) Board_UARTPutSTR(str)
#define DEBUGIN() Board_UARTGetChar()

enum LEDS {LED1, LED2, LED3, LEDR, LEDG, LEDB};
enum TECS {TEC1, TEC2, TEC3, TEC4};

void Board_Init(void);

void Board_Debug_Init(void);

void Board_UARTPutChar(char ch);

int Board_UARTGetChar(void);

void Board_UARTPutSTR(const char *str);

void Board_Leds_Init(void);

void Board_Buttons_Init(void);

void Board_SetupMuxing(void);

void Board_Led_Set(uint8_t LEDName, bool State);

bool Board_Led_Test(uint8_t LEDName);

void Board_Led_Toggle(uint8_t LEDName);

bool Board_Button_Test(uint8_t TECName);

void Board_SystemInit(void);

void Board_SetupClocking(void);
