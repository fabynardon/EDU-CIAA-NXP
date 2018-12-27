#include "board.h"

#define WRITEFUNC __sys_write
#define READFUNC __sys_readc

const uint32_t ExtRateIn = 0;
const uint32_t OscRateIn = 12000000;

struct CLK_BASE_STATES {
	CHIP_CGU_BASE_CLK_T clk;	/* Base clock */
	CHIP_CGU_CLKIN_T clkin;	/* Base clock source, see UM for allowable souorces per base clock */
	bool autoblock_enab;/* Set to true to enable autoblocking on frequency change */
	bool powerdn;		/* Set to true if the base clock is initially powered down */
};

static const struct CLK_BASE_STATES InitClkStates[] = {
	/* Ethernet Clock base */
	{CLK_BASE_PHY_TX, CLKIN_ENET_TX, true, false},
	{CLK_BASE_PHY_RX, CLKIN_ENET_TX, true, false},
	/* Clocks derived from dividers */
	{CLK_BASE_USB0, CLKIN_IDIVD, true, true}
};

typedef struct {
	uint8_t port;
	uint8_t pin;
} IO_PORT_T;

static const IO_PORT_T gpioLEDBits[] = {{0, 14}, {1, 11}, {1, 12}, {5, 0}, {5, 1}, {5, 2}};
static const IO_PORT_T gpioBUTTONBits[] = {{0, 4}, {0, 8}, {0, 9}, {1, 9}};

static const PINMUX_GRP_T pinmuxing[] =  {	/*Board Leds Configuration*/
																		{2, 10, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | SCU_MODE_FUNC0},
																		{2, 11, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | SCU_MODE_FUNC0},
																		{2, 12, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | SCU_MODE_FUNC0},
																		{2, 0, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | SCU_MODE_FUNC4},
																		{2, 1, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | SCU_MODE_FUNC4},
																		{2, 2, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | SCU_MODE_FUNC4},
																		/*Board Buttons Configuration*/
																		{1, 0, SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0},
																		{1, 1, SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0},
																		{1, 2, SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0},
																		{1, 6, SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0},
																		/*Debug Uart Configuration*/
																		{7, 1, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | SCU_MODE_FUNC6},
																		{7, 2, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | SCU_MODE_FUNC6}};
void Board_Init(){

	//Board_SetupMuxing();
	Board_Debug_Init();
	Board_Leds_Init();
	Board_Buttons_Init();

}

void Board_Debug_Init(void) {

	Chip_UART_Init(LPC_USART2);
	Chip_UART_SetBaudFDR(LPC_USART2, 115200);
	Chip_UART_ConfigData(LPC_USART2, UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS);
	Chip_UART_TXEnable(LPC_USART2);

}

void Board_UARTPutChar(char ch) {

	while ((Chip_UART_ReadLineStatus(LPC_USART2) & UART_LSR_THRE) == 0) {}
	Chip_UART_SendByte(LPC_USART2, (uint8_t) ch);

}

int Board_UARTGetChar(void) {

	if (Chip_UART_ReadLineStatus(LPC_USART2) & UART_LSR_RDR) {
		return (int) Chip_UART_ReadByte(LPC_USART2);
	}
	return -1;

}

void Board_UARTPutSTR(const char *str) {

	while (*str != '\0') {
		Board_UARTPutChar(*str++);
	}

}
void Board_Leds_Init() {

	uint32_t ix;
	for (ix = 0; ix < sizeof(gpioLEDBits) / sizeof(IO_PORT_T); ix++ ) {
		Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, gpioLEDBits[ix].port, gpioLEDBits[ix].pin);
	}

}

void Board_Buttons_Init(){

	uint32_t ix;
	for (ix = 0; ix < sizeof(gpioBUTTONBits) / sizeof(IO_PORT_T); ix++ ) {
		Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, gpioBUTTONBits[ix].port, gpioBUTTONBits[ix].pin);
	}

}


void Board_SetupMuxing(void){

	Chip_SCU_SetPinMuxing(pinmuxing, sizeof(pinmuxing) / sizeof(PINMUX_GRP_T));

}

void Board_Led_Set(uint8_t LEDName, bool State){

	Chip_GPIO_SetPinState(LPC_GPIO_PORT, gpioLEDBits[LEDName].port, gpioLEDBits[LEDName].pin, (bool) State);

}

bool Board_Led_Test(uint8_t LEDName){

	return (bool) Chip_GPIO_GetPinState(LPC_GPIO_PORT, gpioLEDBits[LEDName].port, gpioLEDBits[LEDName].pin);
}

void Board_Led_Toggle(uint8_t LEDName){

	Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, gpioLEDBits[LEDName].port, gpioLEDBits[LEDName].pin);

}

bool Board_Button_Test(uint8_t TECName){

	return (bool) !Chip_GPIO_GetPinState(LPC_GPIO_PORT, gpioBUTTONBits[TECName].port, gpioBUTTONBits[TECName].pin);
}

void Board_SystemInit(void){
	Board_SetupMuxing();
	Board_SetupClocking();
}

void Board_SetupClocking(void){
	int i;

	/* Enable Flash acceleration and setup wait states */
	Chip_CREG_SetFlashAcceleration(MAX_CLOCK_FREQ);

	/* Setup System core frequency to MAX_CLOCK_FREQ */
	Chip_SetupCoreClock(CLKIN_CRYSTAL, MAX_CLOCK_FREQ, true);

	/* Setup system base clocks and initial states. This won't enable and
	   disable individual clocks, but sets up the base clock sources for
	   each individual peripheral clock. */
	for (i = 0; i < (sizeof(InitClkStates) / sizeof(InitClkStates[0])); i++) {
		Chip_Clock_SetBaseClock(InitClkStates[i].clk, InitClkStates[i].clkin,
								InitClkStates[i].autoblock_enab, InitClkStates[i].powerdn);
	}

	/* Reset and enable 32Khz oscillator */
	LPC_CREG->CREG0 &= ~((1 << 3) | (1 << 2));
	LPC_CREG->CREG0 |= (1 << 1) | (1 << 0);
}

int WRITEFUNC(int iFileHandle, char *pcBuffer, int iLength){
	unsigned int i;
	for (i = 0; i < iLength; i++) {
		Board_UARTPutChar(pcBuffer[i]);
	}
	return iLength;
}

int READFUNC(void) {
	int c = Board_UARTGetChar();
	return c;

}
