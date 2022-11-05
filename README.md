# Morro-x9-AC-Waveform-Generator

The Morro x9 Waveform Generator is a general purpose function generator which can create square, sinusoid, and triangular waveforms at frequencies of 100 Hz, 200 Hz, 300 Hz, 400 Hz, and 500 Hz. The Morro x9 includes a user interface made of a SparkFun COM-14662 passive 3x4 keypad and a Newhaven Display 2 lines x 16 characters LCD display module. The datasheets of these devices can be found in the Appendix. The waveforms are created by sending an array of analog voltages corresponding to each waveform to a MCP4921 12-bit Voltage Output DAC. The waveform computation, device interfacing and initialization, and power are provided and controlled by an STM32L4A6-ZGT6U Microcontroller.

The keypad allows the user to control the Morro x9 output waveform which includes the following functionality: frequency generation from 100 Hz to 500 Hz, selection of each of the three waveforms, and adjustment of the duty cycle of the square wave from 10% to 90% with a default of 50%. When a waveform other than the square wave is selected, the duty cycle resets to 50%. A map of these key commands can be seen in Table 1. 
 
The LCD displays the output waveform type (i.e. SIN, SQU, TRI), the frequency of the output, the last key pressed, and the duty cycle of the square wave. If a waveform other than the square wave is being displayed, the duty cycle will be displayed as 50%.

The output must be viewed using an oscilloscope as there is no waveform display included in this system. The default output upon power up is a 3 Vpeak-to-peak square wave at 100 Hz. 
