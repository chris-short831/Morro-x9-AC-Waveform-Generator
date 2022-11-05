#include "main.h"
#include "DAC.h"



void DAC_Init(void)
{
  //DAC Config
	//GPIO Configuration
//Connect GPIOA to clock
RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
//Set PA1, PA4, PA7 to Alternate Function mode
GPIOA->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5    | GPIO_MODER_MODE7);
GPIOA->MODER |= (GPIO_MODER_MODE4_1 | GPIO_MODER_MODE5_1  | GPIO_MODER_MODE7_1);
//Set output type to Push-Pull
GPIOA->OTYPER &= ~(GPIO_OTYPER_OT4 | GPIO_OTYPER_OT5  | GPIO_OTYPER_OT7);
//Set GPIOA no Pull-up/Pull-down
GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5  | GPIO_PUPDR_PUPD7);

//Set GPIOA Output Speed
GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED4 | GPIO_OSPEEDR_OSPEED5  | GPIO_OSPEEDR_OSPEED7);

  //Alternate function configuration
GPIOA->AFR[0] &= ~( GPIO_AFRL_AFSEL4_Pos
	  | GPIO_AFRL_AFSEL5_Pos
	  | GPIO_AFRL_AFSEL7_Pos);
GPIOA->AFR[0] |= (GPIO_AF5_SPI1 << GPIO_AFRL_AFSEL4_Pos
	  | GPIO_AF5_SPI1 << GPIO_AFRL_AFSEL5_Pos
	  | GPIO_AF5_SPI1 << GPIO_AFRL_AFSEL7_Pos);

//SPI1 Configuration
//Connect SPI1 to the Peripheral Clock
RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
/*Set SPI to 16-bit mode, enable ~Chip Select (~Slave Select) pulse, and
enable Chip Select Output (Slave Select Output) */
SPI1->CR2 |= ( (0xF << SPI_CR2_DS_Pos) | SPI_CR2_NSSP | SPI_CR2_SSOE);
//Enable SPI1 and configure for controller (Master) mode
SPI1->CR1 |= (SPI_CR1_SPE | SPI_CR1_MSTR);
}

void DAC_Write(uint32_t inputVoltage)
{
	uint32_t digitalVoltage;
	//The - 4 was determined to help calibrate the system through trial and error
	digitalVoltage = (4096 * inputVoltage)/330 - 4;
	//If the TXFIFO is empty, send the 16-bit data string
	if(SPI1->SR & SPI_SR_TXE)
	{
		uint32_t data = 0x3000 | digitalVoltage;
		SPI1->DR = data;
	}
}
