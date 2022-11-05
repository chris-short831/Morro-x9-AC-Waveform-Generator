/*The following functions are used to initialize and send data
 * to the NHD-0216HZ-FSW-FBW-33V3C LCD. Their purposes are described therein.
 */

//Include necessary libraries
#include "main.h"
#include "LCD.h"
#include <stdint.h>
#include <stdio.h>


//Initializes MPU GPIO for LCD
void LCD_GPIO_Init(void)
{
//Configure GPIOC Pins {GPIO interface between LCD and MPU (MPU_Data_Bus)
	//Connect GPIOC to clock
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOCEN);
	//Set MODER Mask = 0
	GPIOC->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 |GPIO_MODER_MODE2
			|GPIO_MODER_MODE3 | GPIO_MODER_MODE4 |GPIO_MODER_MODE5| GPIO_MODER_MODE6);
	//Set MPU_Data_Bus to output mode with MODER[01]
	GPIOC->MODER |= (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0
			|GPIO_MODER_MODE3_0 | GPIO_MODER_MODE4_0 |GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_0);
	GPIOC->MODER &= ~(GPIO_MODER_MODE0_1 | GPIO_MODER_MODE1_1 | GPIO_MODER_MODE2_1
			|GPIO_MODER_MODE3_1 | GPIO_MODER_MODE4_1 | GPIO_MODER_MODE5_1 |GPIO_MODER_MODE6_1);
	//Set the output type to Push-Pull: OT = [0]
    GPIOC->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 | GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3
    		|GPIO_OTYPER_OT4 | GPIO_OTYPER_OT5 | GPIO_OTYPER_OT6);
	//Set MPU_Data_Bus to NO PULL UP/PULL DOWN: PUPD registers = [00]
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD2
			| GPIO_PUPDR_PUPD3 | GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5 | GPIO_PUPDR_PUPD6);
	//Sets OSPEED to 3 which is "Very high Speed"
	GPIOC->OSPEEDR |= ((3 << GPIO_OSPEEDR_OSPEED0_Pos) |
			(3 << GPIO_OSPEEDR_OSPEED1_Pos)|(3 << GPIO_OSPEEDR_OSPEED2_Pos) |
				(3 << GPIO_OSPEEDR_OSPEED3_Pos) | (3 << GPIO_OSPEEDR_OSPEED4_Pos) |
					(3 << GPIO_OSPEEDR_OSPEED5_Pos)|(3 << GPIO_OSPEEDR_OSPEED6_Pos));
}


/*Writes a command to the LCD by sending a
 * byte of data to the LCD in 4-bit TX mode*/
void Write_Command(uint8_t LCDcommand)
{
	//Reset RS for command Register Select and RW for Write
	DATA_OUT->BRR = RS;
	DATA_OUT->BRR = RW;

	//Clear DATA_OUT
	DATA_OUT->BRR = (GPIO_PIN_0| GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3);

	//Set E to enable data transfer
	DATA_OUT->BSRR = E;
	//Delay to account for rise time
	delay_us(1);
	//Output upper nibble
	DATA_OUT->ODR |= ((LCDcommand & 0xF0)>>4);
	//Delay to allow for data transfer
	delay_us(1);
	//Reset E to end data transfer
	DATA_OUT->BRR = E;

	//Delay data clear for address hold
	delay_us(1);
	//Clear DATA_OUT
	DATA_OUT->BRR = (GPIO_PIN_0| GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3);

	//Set E to enable data transfer
	DATA_OUT->BSRR = E;
	//Delay to account for rise time
	delay_us(1);
	//Output lower nibble
	DATA_OUT->ODR |= (LCDcommand & LOWER_NIB);
	//Delay to allow for data transfer
	delay_us(1);
	//Reset E to end data transfer
	DATA_OUT->BRR = E;

	//Delay data clear for address hold
	delay_us(1);
	//Clear DATA_OUT
	DATA_OUT->BRR = (GPIO_PIN_0| GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3);
}

/*Writes data (character code) to the LCD by sending a
 * byte of data to the LCD in 4-bit TX mode*/
void Write_Data(uint8_t LCDdata)
{
	//Set RS for data register select
	DATA_OUT->BSRR = RS;
	//Clear DATA_OUT
	DATA_OUT->BRR = (GPIO_PIN_0| GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3);

	//Set E to enable data transfer
	DATA_OUT->BSRR = E;
	//Delay to account for rise time
	delay_us(1);
	//Output upper nibble
	DATA_OUT->ODR |= ((LCDdata & 0xF0)>>4);
	//Delay to allow for data transfer
	delay_us(1);
	//Reset E to end data transfer
	DATA_OUT->BRR = E;

	//Delay data clear for address hold
	delay_us(1);
	//Clear DATA_OUT
	DATA_OUT->BRR = (GPIO_PIN_0| GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3);

	//Set E to enable data transfer
	DATA_OUT->BSRR = E;
	//Delay to account for rise time
	delay_us(1);
	//Output lower nibble
	DATA_OUT->ODR |= (LCDdata & LOWER_NIB);
	//Delay to allow for data transfer
	delay_us(1);
	//Reset E to end data transfer
	DATA_OUT->BRR = E;

	//Delay data clear for address hold
	delay_us(1);
	//Clear DATA_OUT
	DATA_OUT->BRR = (GPIO_PIN_0| GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3);
}

/*Writes a string to the LCD by converting the string to
 * chars which are sent to the Write_Data function */
void String_Write(char Lcdstring[])
{
	uint8_t charCount = 0;\
	//Iterate through the string
	while((Lcdstring[charCount] != '\0'))
	{
		Write_Data(Lcdstring[charCount]);
		//Go to the next line after the 16th character of the string
		if(charCount > 15)
		{
			Write_Command(NEW_LINE);
			while((Lcdstring[charCount] != '\0'))
				{
					Write_Data(Lcdstring[charCount]);
					charCount++;
					/*If the string is more than 30 chars long
					 * tell the user there's too many chars to be
					 * displayed*/
					if(charCount > 30)
					{
						Write_Command(CLEAR_DISPLAY);
						delay_us(2e3);
						Write_Command(RETURN_HOME);
						String_Write("Too Many Chars!");
						break;
					}
				}
		}
		charCount++;
	}
}

/*Initializes the LCD by sending the appropriate
 * commands and corresponding timing as specified
 * by the LCD data sheet*/
void Lcd_Init(void)
{
	//Long delay after initial power up
	delay_us(50e3);

	//Reset RS and RW
	DATA_OUT->BRR = RS;
	DATA_OUT->BRR = RW;

	//Function set
	Write_Command(0x30);

	//Function set
	Write_Command(0x28);
	delay_us(40);

	/*Toggle E bit for no apparent reason after first or second command.
	 * Code will not work without this. After experimentation it was seen that
	 * the screen behaved better to the reset if the E toggle is placed after second
	 * function set*/
	DATA_OUT->BSRR = E;
	delay_us(15);
	DATA_OUT->BRR = E;

	//Function set
	Write_Command(0x28);
	delay_us(40);

	//Display ON/OFF
	Write_Command(0x0c);
	delay_us(400);

	//Display Clear
	Write_Command(0x01);
	delay_us(2e3);

	//Entry Mode
	Write_Command(0x06);
	//delay_us(45);
}

/* Configure SysTick Timer for use with delay_us function. This will break
 * break compatibility with HAL_delay() by disabling interrupts to allow for
 * shorter delay timing.
 */
void SysTick_Init(void){
    SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk |	       // enable SysTick Timer
                      SysTick_CTRL_CLKSOURCE_Msk);     // select CPU clock
    SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk);      // disable interrupt,
                                                       // breaks HAL delay function
}

/* Delay function using the SysTick timer to count CPU clock cycles for more
 * precise delay timing. Passing a time of 0 will cause an error and result
 * in the maximum delay. Short delays are limited by the clock speed and will
 * often result in longer delay times than specified. @ 4MHz, a delay of 1us
 * will result in a delay of 10-15 us.
 */
void delay_us(const uint32_t time_us) {
    // set the counts for the specified delay
    SysTick->LOAD = (uint32_t)((time_us * (SystemCoreClock / 1000000)) - 1);
    SysTick->VAL = 0;                                      // clear the timer count
    SysTick->CTRL &= ~(SysTick_CTRL_COUNTFLAG_Msk);        // clear the count flag
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)); // wait for the flag
}


void Initialize_LCD(void)
{
	  Write_Command(RETURN_HOME);
	  String_Write("SQU");
	  Write_Command(SHIFT_FREQ);
	  String_Write("100");
	  Write_Command(SHIFT_Hz);
	  String_Write("Hz");
	  Write_Command(SHIFT_LAST);
	  String_Write("LAST");
	  Write_Command(NEW_LINE);
	  Write_Data(DUTY_COUNT_INIT);
	  String_Write("0%");
	  Write_Command(SHIFT_Duty);
	  String_Write("DUTY");
	  Write_Command(SHIFT_KEY_INIT);
	  String_Write("' '");
}



