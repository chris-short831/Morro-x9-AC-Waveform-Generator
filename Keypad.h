/*
 * Keypad.h
 *
 *  Created on: Oct 28, 2022
 *      Author: chris
 */

#ifndef INC_KEYPAD_H_
#define INC_KEYPAD_H_

#define rowODR GPIOD
#define columnIDR GPIOB

uint32_t Get_Key(void);
void Keypad_Init(void);
uint32_t Key_Press(void);



#endif /* INC_KEYPAD_H_ */
