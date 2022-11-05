#include "main.h"
#include "Keypad.h"
#include <stdbool.h>
uint32_t Get_Key(void)
{
   //Declare row and column counters and the key variable
   uint32_t rowCounter;
   uint32_t colCounter;
   uint32_t key;
   /* This for-loop cycles outputs a logic 1 for each Row
    (i.e. 0x0001, 0x0010, 0x0100, and 0x1000) and contains a nested
    for-loop which cycles through the columnIDR IDR to determine both the row
    and column pressed to determine the key pressed.
    */
   for(rowCounter = 0; rowCounter < 4; rowCounter++)
   {
	   //Initialize the key value to 0
	   key = 0;
	   //Clear the rowODR (output data register)
	   rowODR->BRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	   //Clear the columnIDR (input data register)
	   columnIDR->BRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);

	   /* Set rowODR equal to binary 1,2,4, or 8 by left shifting a 1-bit
	   by the row count */
	   rowODR->ODR |= (0b1 << rowCounter);

	   /* This for-loop cycles through the columnIDR in the same manner as the rowCounter
	   for loop */
	   for(colCounter = 0; colCounter<3; colCounter++)
	   {
		   //Determine the Column of the key pressed when Row 1 is high
		   if((rowODR->ODR & 0b0001))
		   {
			   if(columnIDR->IDR & (0b1 << colCounter))
			   {
				   key = 0;
				   /*The sum of colCounter and rowCounter here will be 0, 1 or 2
				    so must add 1 to the count to translate to keypad */
				   key = colCounter + rowCounter + 1;
				   return key;
			   }
		   }
		   //Determine the Column of the key pressed when Row 2 is high
		   else if((rowODR->ODR & 0b0010))
		   {
			   if(columnIDR->IDR & (0b1 << colCounter))
			   {
				   key = 0;
				   /*The sum of colCounter and rowCounter here will be 1, 2 or 3
				   	so must add 3 to the count to translate to keypad */
				   key = colCounter + rowCounter + 3;
				   return key;
			   }
		   }
		   //Determine the Column of the key pressed when Row 3 is high
		   else if((rowODR->ODR & 0b0100))
		   {
			   if(columnIDR->IDR & (0b1 << colCounter))
			   {
				   key = 0;
				   /*The sum of colCounter and rowCounter here will be 2, 3 or 4
				   	so must add 5 to the count to translate to keypad */
				   key = colCounter + rowCounter + 5;
				   return key;
			   }
		   }
		   //Determine the Column of the key pressed when Row 4 is high
		   else if((rowODR->ODR & 0b1000))
		   {
			   /* The last row is done explicitly to set the * and # keys equal to
			    their ASCII values in anticipation of future assignments */
			   if(columnIDR->IDR & (0b1 << 0))
			   {
				   key = 0;
				   key = '*';
				   return key;
			   }
			   else if(columnIDR->IDR & (0b1 << 1))
			   {
				   key = 0;
				   return key;
			   }
			   else if(columnIDR->IDR & (0b1 << 2))
			   {
				   key = 0;
				   key = '#';
				   return key;
			   }
		   }
		   else
		   {
			   return -1;
		   }
	   }
   }
   key = -1;
   return key;
}

void Keypad_Init(void)
{
//Configure GPIO D Pins (rowODR)
     //Connect GPIOD to clock
     RCC->AHB2ENR |= (RCC_AHB2ENR_GPIODEN);
     //Set MODER Mask = 0
     GPIOD->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 |GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
     //Set rowODR to output mode with MODER[01]
     GPIOD->MODER |= (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0 | GPIO_MODER_MODE3_0);
     //Set the output type to Push-Pull: OT = [0]
     GPIOD->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 | GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);
     //N pull up/pull down: set PUPD registers to [00]
     GPIOD->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);
     //Set OSPEED to 3 which is "Very high Speed"
     GPIOD->OSPEEDR |= ((3 << GPIO_OSPEEDR_OSPEED0_Pos) |
     (3 << GPIO_OSPEEDR_OSPEED1_Pos)|(3 << GPIO_OSPEEDR_OSPEED2_Pos) |
     (3 << GPIO_OSPEEDR_OSPEED3_Pos));

//Configure GPIOB Pins (columnIDR)
     //Connect GPIOB to clock
     RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN);
     //Set MODER Mask = 0
     GPIOB->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 |GPIO_MODER_MODE2);
     //Set columnIDR to input mode with MODER[00]
     GPIOB->MODER &= ~(GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0);
     GPIOB->MODER &= ~(GPIO_MODER_MODE0_1 | GPIO_MODER_MODE1_1 | GPIO_MODER_MODE2_1);
     //Set the output type to Push-Pull: OT = [0]
     GPIOB->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 | GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);
     //Set columnIDR to PULL DOWN: PUPD registers = [10]
     GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD2);
     GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD0_0 | GPIO_PUPDR_PUPD1_0 | GPIO_PUPDR_PUPD2_0);
     GPIOB->PUPDR |= (GPIO_PUPDR_PUPD0_1 | GPIO_PUPDR_PUPD1_1 | GPIO_PUPDR_PUPD2_1);
     //Sets OSPEED to 3 which is "Very high Speed"
     GPIOB->OSPEEDR |= ((3 << GPIO_OSPEEDR_OSPEED0_Pos) |
     (3 << GPIO_OSPEEDR_OSPEED1_Pos)|(3 << GPIO_OSPEEDR_OSPEED2_Pos) |
     (3 << GPIO_OSPEEDR_OSPEED3_Pos)); //Sets OSPEED to 3? which is "Very high Speed"
}

/*This function determines if a key was pressed by watching the
 columnIDR (input data register). The columnIDR will go high if a key is pressed and thus this function will return
 true if this occurs. */
uint32_t Key_Press(void)
{
	   bool press;
	   //Clear the columnIDR IDR. If the IDR goes high
	   columnIDR->BRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);

	   if(columnIDR->IDR != 0)
	   {
		   press = true;
		   return press;
	   }
	   else
	   {
		   press = false;
		   return press;
	   }
}

