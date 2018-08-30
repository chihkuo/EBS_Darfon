/*
	IO definitions

	1. LCD: 2x16 text LCM
	2. AD0-3:
		i. AD0: temperature input
		ii. AD1: irradiance input
		iii. AD2: NC
		iv. AD3: NC
	3. UART:
		i. UART1(/dev/ttyS1): GSM module
		ii. UART2(/dev/ttyS2): RS485
		iii. UART3(/dev/ttyS3): NC
		iv. UART4(/dev/ttyS4): NC
	4. I2C: NC
	5. SPI: NC
	6. I2S: NC
	7. USB:
		i. USB memory dongle
		ii. external SD reader
		iii. USB WIFI card
		iv. USB 3G/2.5G module
	8. GPI: button inputs
		i. SW0(GPL11/EINT19): reset switch
		ii SW1(GPL12/EINT20): copy switch
		iii. SW2(GPN8/EINT8):
		iv. SW3(GPN12/EINT12):
	9. GPO: LED outputs
		i. LED0(GPK4): system status 0 LED output
		ii. LED1(GPK5): system status 1 LED output
		iii. LED2(GPK6): system status 2 LED output
		iv. LED3(GPK7): system status 3 LED output
		v. LED4(GPE0): status 0 LED output
		vi. LED5(GPN0): status 1 LED output
		vii. LED6(GPN1): status 2 LED output
		viii. LED7(GPN2): digital output 0
		ix. LED8(GPN3): digital output 1
		x. LED9(GPN4): digital output 2
		xi. LED10(GPN5): digital output 3
	9. buzzer: 2KHz buzzer output
*/


#ifndef __IO_DEFINITIONS_H__
#define __IO_DEFINITIONS_H__



//#define RS485_PORT	"/dev/ttyUSB0"
#define RS485_PORT	"/dev/ttyS0"



#endif
