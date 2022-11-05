/*
Developer Name: Christopher Short
Class: EE 329 Microcontroller-Based Systems Design
Professor: John Penvenne
Assignment: Project 1 Function Generator
Device: Morro x9 Waveform Generator
Date: 10/30/2022
Peripherals: MCP4921 DAC, Newhaven Display 0216HZ-FSW-FBW-33V3C 2x16 LCD, and a SparkFun COM-14662 passive 3x4 keypad
Development Board: STM32L4A6ZGT6U

Overview: This code interfaces a 12-bit DAC, a 3x4 passive keypad, a 2x16 LCD and the STM32L4A6ZGT6U to create a function generator capable of producing sine, tri, and square waveforms as selected by the user. Each waveform is capable of a frequency of 100 Hz, 200 Hz, 300 Hz, 400 Hz, and 500 Hz. Additionally, the duty cycle of the square wave is adjustable between 10% and 90% in steps of 10%. The program defaults with a 100 Hz square wave with a 50% duty cycle upon power up. 

Keypad function map:
1: Set frequency to 100 Hz
2: Set frequency to 200 Hz
3: Set frequency to 300 Hz
4: Set frequency to 400 Hz
5: Set frequency to 500 Hz
6: Output Sine wave
7: Output Triangle wave
8: Output Square wave
9: Reset to default square wave
*:Decrease duty cycle be 10%
0:Reset duty cycle to 50%
#:Increase duty cycle by 10%
 */
/************************************************************************/

//Include necessary header files and libraries

#include "main.h"
#include "Keypad.h"
#include "Timer.h"
#include "DAC.h"
#include "LCD.h"
#include <math.h>
#include <stdbool.h>

//#defines for Duty Cycle functionality
#define DUTY_CYCLE_50 98
#define DUTY_CYCLE_10 19
#define DUTY_COUNT_INIT 0x35
#define DUTY_CYCLE_MAX 174
#define DUTY_CYCLE_MIN 22

//#defines for waveform generation
#define SINE_MAX_VOLTAGE_INDEX 392
#define TRI_MAX_VOLTAGE_INDEX 350
#define SAW_MAX_VOLTAGE_INDEX 360
#define SQU_MAX_VOLTAGE_INDEX 194
#define SINE_DC_OFFSET 154
#define SINE_AMP 152
#define PI 3.14
#define SIX_V 620
#define THREE_V 308
#define ZERO_V 1

//#define for key de-bounce
#define ONE_MS 100e3

//Create Waveform arrays
uint32_t sineWaveform[420];
uint32_t squareWaveform[420];
uint32_t sawtoothWaveform[420];
uint32_t triangleWaveform[1000];

//Declare program variables
uint32_t key;
uint32_t dutyCycle = DUTY_CYCLE_50;
uint32_t freqIndex = 1;
uint32_t dutyCount = DUTY_COUNT_INIT;
uint32_t keyPress;
float triSlopePos = 3.1;
float triSlopeNeg = 3.18;
float sawSlope = 1.6;

/*Define Sine_Waveform function to build Sine array. This function
 fills an array with the voltage values of a 3V peak-to-peak sinusoid
 with a DC offset of 1.5 V to be sent to the DAC. This arrays is filled with two periods to allow for
 the sine wave to be 100 Hz at the defined sample frequency.*/
void Sine_Waveform(void)
{
  for(uint32_t voltageIndex = 0; voltageIndex < SINE_MAX_VOLTAGE_INDEX; voltageIndex++)
  {
  	  sineWaveform[voltageIndex] = (uint32_t)( SINE_AMP*(sin(2*PI*2*voltageIndex/SINE_MAX_VOLTAGE_INDEX)) + SINE_DC_OFFSET );
  }
}

/* Define Square_Waveform function to build Square array. This function
fills an array with the voltage values of a 100 Hz, 3V peak-to-peak square wave
with a DC offset of 1.5 V to be sent to the DAC. The default duty cycle is 50%. There is a #define DUTY_CYCLE_50
in the main body of the program which is an integer that corresponds to a 50% duty cycle in the for-loop below. To control
the duty cycle, an integer corresponding to 10% of the 50% duty cycle default integer from DUTY_CYCLE_50. */
void Square_Waveform(uint32_t key)
{
	Write_Command(NEW_LINE);
	Write_Data(dutyCount);
	for(uint32_t voltageIndex = 0; voltageIndex < SQU_MAX_VOLTAGE_INDEX; voltageIndex++)
	{
		if( voltageIndex < dutyCycle)
		{
			squareWaveform[voltageIndex] = THREE_V;
		}
		else
		{
			squareWaveform[voltageIndex] = ZERO_V;
		}
	}
}

/*Define Sawtooth_Waveform function to build Sawtooth array. This function fills an array with the values
of a 100 Hz, 3V peak-to-peak Sawtooth wave to be sent to the DAC. This waveform is not assigned to a key press
and if the user desires a sawtooth wave, they must assign it to a key assigned to one of the other 3 waveforms */
void Sawtooth_Waveform(void)
{
	for(uint32_t voltageIndex = 0; voltageIndex < SAW_MAX_VOLTAGE_INDEX; voltageIndex++)
	{
		sawtoothWaveform[voltageIndex] = sawSlope*voltageIndex;
	}
}

/* Define Triangle_Waveform function to build Triangle array. This function fills an array with
 the values of a 100Hz, 3V peak-to-peak, triangle wave to be sent to the DAC. */
void Triangle_Waveform(void)
{
	for(uint32_t voltageIndex = 0; voltageIndex < TRI_MAX_VOLTAGE_INDEX; voltageIndex++)
	{
		if(voltageIndex < 100)
		{
			triangleWaveform[voltageIndex] = triSlopePos*voltageIndex;
		}
		else if(voltageIndex > 90)
		{
		triangleWaveform[voltageIndex] = SIX_V - triSlopeNeg*voltageIndex;
		}
	}
}


// Define states for each waveform to be chosen by a key press. Square wave is the chosen default.
typedef enum
{
	SQUARE_WAVE,
	SINE_WAVE,
	TRIANGLE_WAVE,
	SAWTOOTH_WAVE
}
state;
state currentState = SQUARE_WAVE;

//Declare default functions
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
  //Call default functions
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  SysTick_Init();

  //Call peripheral initialization functions
  Keypad_Init();
  Timer_Init();
  DAC_Init();
  LCD_GPIO_Init();
  Lcd_Init();

  //Call waveform functions to build waveform arrays
  Sine_Waveform();
  Square_Waveform(key);
  Sawtooth_Waveform();
  Triangle_Waveform();

  //Initialize LCD Display with "SQU 100 Hz LAST
  //	  	  	  	  	  	  	 50% DUTY   ' ' "
  Initialize_LCD();

while (1)
  {
	//Always check for presence of a keypress
	keyPress = Key_Press();

	//If a key is pressed, perform corresponding functionality
	if(keyPress == true)
	{
		//Get key
		key = Get_Key();
		//Add 10% to the SQU duty cycle if key pressed is # and the duty cycle is < 90%
		if( ((key == '#') & (dutyCycle < DUTY_CYCLE_MAX) & (currentState == SQUARE_WAVE)) )
		{
			//De-bounce the keypress
			delay_us(ONE_MS);
			//Add 10% to the dutyCycle variable
			dutyCycle += DUTY_CYCLE_10;
			//Add 1 to the dutyCount
			dutyCount++;
			//Display the key press on the LCD
			Write_Command(SHIFT_LAST_KEY);
			String_Write("#");
		}
		//Subtract 10% to the SQU duty cycle if key pressed is * and the duty cycle is > 10%
		else if( ((key == '*') & (dutyCycle > DUTY_CYCLE_MIN) & (currentState == SQUARE_WAVE)) )
		{
			//De-bounce the keypress
			delay_us(ONE_MS);
			//Subtract 10% to the dutyCycle variable
			dutyCycle -= DUTY_CYCLE_10;
			//Subtract 1 from the dutyCount
			dutyCount--;
			//Display the key press on the LCD
			Write_Command(SHIFT_LAST_KEY);
			String_Write("*");
		}
		//Create waveform with chosen duty cycle
		Square_Waveform(key);

		//Set frequency to 100Hz
		if(key == 1)
		{
			freqIndex = 1;
			//Display the key press and  frequency on the LCD
			Write_Command(SHIFT_FREQ);
			String_Write("100");
			//Display the key press on the LCD
			Write_Command(SHIFT_LAST_KEY);
			String_Write("1");
		}

		//Set frequency to 200Hz
		if(key == 2)
		{
			freqIndex = 2;
			//Display the key press and  frequency on the LCD
			Write_Command(SHIFT_FREQ);
			String_Write("200");
			Write_Command(SHIFT_LAST_KEY);
			String_Write("2");
		}

		//Set frequency to 300Hz
		if(key == 3)
		{
			freqIndex = 3;
			//Display the key press and  frequency on the LCD
			Write_Command(SHIFT_FREQ);
			String_Write("300");
			Write_Command(SHIFT_LAST_KEY);
			String_Write("3");
		}

		//Set frequency to 400Hz
		if(key == 4)
		{
			freqIndex = 4;
			//Display the key press and  frequency on the LCD
			Write_Command(SHIFT_FREQ);
			String_Write("400");
			Write_Command(SHIFT_LAST_KEY);
			String_Write("4");
		}

		//Set frequency to 500Hz
		if(key == 5)
		{
			freqIndex = 5;
			//Display the key press and  frequency on the LCD
			Write_Command(SHIFT_FREQ);
			String_Write("500");
			Write_Command(SHIFT_LAST_KEY);
			String_Write("5");
		}

		//Output Sine Wave
		if(key == 6)
		{
			currentState = SINE_WAVE;
			//Display waveform name on LCD
			Write_Command(RETURN_HOME);
			String_Write("SIN");
			Write_Command(SHIFT_LAST_KEY);
			String_Write("6");
			dutyCount = DUTY_COUNT_INIT;
		}

		//Output Triangle wave
		if(key == 7)
		{
			currentState = TRIANGLE_WAVE;
			//Display waveform name on LCD
			Write_Command(RETURN_HOME);
			String_Write("TRI");
			Write_Command(SHIFT_LAST_KEY);
			String_Write("7");
			dutyCount = DUTY_COUNT_INIT;
		}

		//Output Square wave
		if(key == 8)
		{
			currentState = SQUARE_WAVE;
			//Display waveform name on LCD
			Write_Command(RETURN_HOME);
			String_Write("SQU");
			Write_Command(SHIFT_LAST_KEY);
			String_Write("8");
		}

		//Return to default output (SQU at 100 Hz with 50% duty cycle)
		if(key == 9)
		{
			freqIndex = 1;
			dutyCycle = DUTY_CYCLE_50;
			dutyCount = DUTY_COUNT_INIT;
			currentState = SQUARE_WAVE;
			Initialize_LCD();
			Write_Command(SHIFT_LAST_KEY);
			String_Write("9");
		}
		//Reset SQU duty cycle to 50%
		if((key == 0) & (currentState == SQUARE_WAVE))
		{
			dutyCount = DUTY_COUNT_INIT;
			dutyCycle = DUTY_CYCLE_50;
			//Display the keypress on LCD
			Write_Command(SHIFT_LAST_KEY);
			String_Write("0");
		}
	}
  }
}

void TIM2_IRQHandler(void)
	  {
	/*Create static variable to iterate through the voltage
	  values in each array */
	static int voltageIndex = 0;
		  if (TIM2->SR & TIM_SR_UIF)
			{
			  switch(currentState)
			  {
			  	  case SINE_WAVE:

			  		  if(voltageIndex >= SINE_MAX_VOLTAGE_INDEX)
			  		  {
			  			voltageIndex = 0;
			  		  }
			  		  DAC_Write(sineWaveform[voltageIndex]);
			  		  voltageIndex += freqIndex;

			  		  break;
			  	  case SQUARE_WAVE:
			  		  if(voltageIndex >= SQU_MAX_VOLTAGE_INDEX)
			  		  {
			  			voltageIndex = 0;
			  		  }

			  		  DAC_Write(squareWaveform[voltageIndex]);

			  		  voltageIndex += freqIndex;
			  		  break;
			  	  case SAWTOOTH_WAVE:
					  if(voltageIndex >= 195)
					  {
						  voltageIndex = 0;
					  }
					  DAC_Write(sawtoothWaveform[voltageIndex]);
					  voltageIndex += freqIndex;
					  break;
			  	  case TRIANGLE_WAVE:
					  if(voltageIndex >= 195)
					  {
						  voltageIndex = 0;
					  }
			  		  DAC_Write(triangleWaveform[voltageIndex]);
			  		  voltageIndex += freqIndex;
			  		  break;
			  }
			}
		  TIM2->SR &= ~(TIM_SR_UIF);
	  }
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : STLK_RX_Pin STLK_TX_Pin */
  GPIO_InitStruct.Pin = STLK_RX_Pin|STLK_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_SOF_Pin USB_ID_Pin USB_DM_Pin USB_DP_Pin */
  GPIO_InitStruct.Pin = USB_SOF_Pin|USB_ID_Pin|USB_DM_Pin|USB_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
