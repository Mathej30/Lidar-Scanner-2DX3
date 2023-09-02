#include <stdint.h>
#include <stdio.h>
#include "tm4c1294ncpdt.h"
#include "PLL.h"
#include "SysTick.h"
#include "I2C0.h" //modified version of Valvano i2c0.c  
#include "uart.h"
#include "VL53L1X_api.h"
#include "onboardLEDs.h"
#define dev 0x29
//Jibin Mathew 400303976 mathej30

void PortH_Init(void){															//Stepper Motor Port
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7;					// activate clock for Port H
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R7) == 0){}	// allow time for clock to stabilize
	GPIO_PORTH_DIR_R |= 0x0F;        									// make PH0 to PH3 output
	GPIO_PORTH_AFSEL_R &= ~0xFF;     									// disable alt funct
	GPIO_PORTH_DEN_R |= 0x0F;        									// enable digital I/O on PH0 to PH3
	GPIO_PORTH_AMSEL_R &= ~0xFF;     									// disable analog functionality on PH0 to PH3	

	return;
}
void PortM_Init(void){																			//Buttons 
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11;                 // Activate the clock for Port M
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R11) == 0){};        // Allow time for clock to stabilize 
	GPIO_PORTM_DIR_R = 0b00000000;       								      // Make PM0 inputs
  GPIO_PORTM_DEN_R = 0b00000001;
	GPIO_PORTM_CR_R = 0x01;
	GPIO_PORTM_LOCK_R = 0x0;
	GPIO_PORTM_PUR_R = 0b00001111; 														// Enable the pull-up resistors for PM0
	return;
}

void stepCCW(uint32_t steps, uint32_t wait){ 							//Each rotation is 2048 steps
	
	uint32_t i;
	for(i = 0; i < steps; i++) {
		//depending on state of motor, sets motor to next state going CCW
		uint8_t data = GPIO_PORTH_DATA_R & 0x0F;
		
		if (data == 0b00000011) {
			GPIO_PORTH_DATA_R = 0b00001001;
			//FlashLED1(1);
			SysTick_Wait10ms(wait);
		}
		else if (data == 0b00000110) {
			GPIO_PORTH_DATA_R = 0b00000011;
			//FlashLED2(1);
			SysTick_Wait10ms(wait);
		}
		else if (data == 0b00001001) {
			GPIO_PORTH_DATA_R = 0b00001100;
			FlashLED3(1);
			SysTick_Wait10ms(wait);
		}
		else if (data == 0b00001100) {
			GPIO_PORTH_DATA_R = 0b00000110;
			//FlashLED4(1);
			SysTick_Wait10ms(wait);
		}
		else {													// Setting the default sate for the motor
			GPIO_PORTH_DATA_R = 0b00000011;
			SysTick_Wait10ms(wait);
		}
	}

	return;
}

void step(uint32_t steps, uint32_t wait){							//Each rotation is 2048 steps
	
	uint32_t i;
	for(i = 0; i < steps; i++) {
		uint8_t data = GPIO_PORTH_DATA_R & 0x0F;						
		
		if (data == 0b00000011) {
			GPIO_PORTH_DATA_R = 0b00000110;
			SysTick_Wait10ms(wait);
		}
		else if (data == 0b00000110) {
			GPIO_PORTH_DATA_R = 0b00001100;
			SysTick_Wait10ms(wait);
		}
				else if (data == 0b00001100) {
			GPIO_PORTH_DATA_R = 0b00001001;
			SysTick_Wait10ms(wait);
		}
		else if (data == 0b00001001) {
			GPIO_PORTH_DATA_R = 0b00000011;
			SysTick_Wait10ms(wait);
		}
		else {													// Setting the default sate for the motor
			GPIO_PORTH_DATA_R = 0b00000011;
			SysTick_Wait10ms(wait);
		}
	}

	return;
}

int main(void) {
																						//All LEDs are flashed through the functions in the onboardLEDs file
	int status = 0;
	uint8_t modelID, modelType;
	uint16_t bothType;
	PLL_Init();
	SysTick_Init();
	I2C_Init();
	UART_Init();
	onboardLEDs_Init();
	FlashLED2(1);
	PortH_Init();															//stepper motor
	PortM_Init();															//Button
	
	
	uint8_t byteData, sensorState=0;
  uint16_t wordData;
  uint16_t Distance;
  uint16_t SignalRate;
  uint16_t AmbientRate;
  uint16_t SpadNum; 
  uint8_t RangeStatus;
  uint8_t dataReady;
	int motorState = 0;
	
	status = VL53L1X_GetSensorId(dev, &wordData);													//getting the status of the sensor


	// Booting ToF chip
	while(sensorState==0){
		status = VL53L1X_BootState(dev, &sensorState);
		SysTick_Wait10ms(10);
  }
	FlashAllLEDs();																											//Indeicator for booting the TOF
	
	status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/
	
  /* This function must to be called to initialize the sensor with the default setting  */
  status = VL53L1X_SensorInit(dev);
	Status_Check("SensorInit", status);
	
	status = VL53L1X_StartRanging(dev);   // This function has to be called to enable the ranging
	
	while(1) {

		if((GPIO_PORTM_DATA_R&0b00000001) == 0) {														//Checking to see if the button has been pressed
			while((GPIO_PORTM_DATA_R&0b00000001) == 0){}
				SysTick_Wait(1200*1000);
				motorState = 1;																									//setting the motor state
			}
		
		if(motorState == 1){																								//execution if the moto state is 1
			GPIO_PORTN_DATA_R ^= 0b00000001;																	//toggling the led 
			for(int i = 0; i < 32; i++) {																			//loop to go through all my steps (11.25 deg) --> 32 steps
				FlashLED1(1);																										//flashing my led based on the last degit of the my student number									
					step(64, 1);	
				
				//wait until the ToF sensor's data is ready
				while (dataReady == 0){																
					status = VL53L1X_CheckForDataReady(dev, &dataReady);
							FlashLED3(1);																							//led indicatind data status
							VL53L1_WaitMs(dev, 5);
				}
					dataReady = 0;
				
				status = VL53L1X_GetDistance(dev, &Distance);					//Measured Distance 
				
				FlashLED4(1);																										//led indicating distance collection

				status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/
				
				sprintf(printf_buffer,"%u\r\n",Distance);		//sending the data to UART
				UART_printf(printf_buffer);
				SysTick_Wait10ms(50);
			}
			
			//resetting all the variabels for another rotation
			motorState = 0;															//resetting the motor state
			stepCCW(2048,1);														//calling the counter clocwise funciton
		}
		GPIO_PORTN_DATA_R ^= 0b00000001;						//toggeling the LED that was toggeld to indicate the start of the button press
	}
}