/**
*   @file     CN0391.cpp
*   @brief    Application file for the CN0391 project
*   @author   Analog Devices Inc.
*
********************************************************************************
* Copyright 2016(c) Analog Devices, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*  - Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  - Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
*  - Neither the name of Analog Devices, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*  - The use of this software may or may not infringe the patent rights
*    of one or more patent holders.  This license does not release you
*    from the requirement that you obtain separate licenses from these
*    patent holders to use this software.
*  - Use of the software either in source or binary form, must be run
*    on or directly connected to an Analog Devices Inc. component.
*
* THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
********************************************************************************/
#include "Arduino.h"
#include "CN0391.h"

#include "RTD.h"
#include "Thermocouple.h"
#include "AD7124.h"

#include "Communication.h"

#include <stdio.h>
#include <math.h>

int32_t _ADCValue0[4], _ADCValue1[4];
float rRtdValue[4], temp0[4], temp1[4];

float cj_Voltage[4], th_Voltage_read[4], th_Voltage[4];

static const unsigned char thermocouple_type[] = { 'T', 'J', 'K', 'E', 'S', 'R', 'N', 'B' };
uint8_t th_types[4];

error_code errFlag[4] = {NO_ERR, NO_ERR, NO_ERR, NO_ERR};

#define ms_delay (1)


/*
Blocking functions:

AD7124_WaitForConvReady
AD7124_WaitForSpiReady
CN0391_read_channel

*/


int32_t CN0391_read_channel(int ch) // blocking function | causes 500 to 1000 millisecond delay.
{
	int32_t data;

	CN0391_enable_channel(ch); 

   convFlag = 1;
	digitalWrite(CS_PIN, LOW);
	CN0391_start_single_conversion(); // cannot be called repeatedly unless flag is enabled. 

	//
	if (AD7124_WaitForConvReady(10000) == -3) { // generates recursive calls to blocking waitforconvready.
			Serial.println("TIMEOUT");
			return 0;
		}
	//
	//delay(60); // adds excessive delay to code: default = 100 | min_working ~ 60

	AD7124_ReadData(&data); // non-blocking
	convFlag = 0;
	digitalWrite(CS_PIN, HIGH);

	CN0391_disable_channel(ch); 
	return data;
}

/*
int32_t CN0391_read_channel(int ch)
{
	// added variables
	static bool channel_state[4] = { LOW };
	static int32_t channel_timer[4] = { millis() };
	constexpr int32_t READ_DELAY = 100; 		// millisec
	
	// original code
	//int32_t data;
	static int32_t data[4] = { 0 }; // default value given as 0

	// states
	//bool state = channel_state[ch];
	//int32_t timer = channel_timer[ch];
	//bool cs_pin_state = digitalRead(CS_PIN);
	Serial.print("Channel: ");
	Serial.println(ch);

	if( channel_state[ch] == LOW ) {
		channel_state[ch] = HIGH;		// block execution until set LOW
		channel_timer[ch] = millis();	// begin timer

		// original code
		CN0391_enable_channel(ch);
		convFlag = 1;
		digitalWrite(CS_PIN, LOW);
		CN0391_start_single_conversion();
		
		// debug
		//Serial.println("entered channel LOW");
	}
		
	// this greatly slows down data reads | recursive 
	//if (AD7124_WaitForConvReady(10000) == -3) { // | default = 10000 | blocking function
	//		Serial.println("TIMEOUT");
	//		return 0;
	//	}
	//
	
	// NOTE: using non-blocking delays to allow more precise control of PWM.

	//delay(100); //-------- lag in reading data | need 100ms minimum delay

	int32_t change_time = millis() - channel_timer[ch];
	
	if( change_time >= READ_DELAY && channel_state[ch] == HIGH ) {
		channel_timer[ch] = millis();
		channel_state[ch] = LOW;

		// original code
		//AD7124_ReadData( &data );
		
		AD7124_ReadData( &data[ch] );
		convFlag = 0;
		digitalWrite(CS_PIN, HIGH);
		CN0391_disable_channel(ch); 
		
		// return data;
	}
	
	return data[ch];
}
*/
float CN0391_data_to_voltage(int32_t data, uint8_t channel) {

	float voltage;

	if(channel == RTD_CHANNEL){
	      voltage = (VREF_EXT*(data - _2_23))/(_2_23 *GAIN_RTD);
	} else{
	      voltage = (VREF_INT*(data - _2_23))/(_2_23*GAIN_TH);
	}

	return voltage;
}

float CN0391_data_to_resistance(int32_t data)
{

   float rRtd;

   rRtd = (R5*(data - _2_23))/(_2_23 *GAIN_RTD);

   return rRtd;
}
void CN0391_enable_channel(int channel)
{
	enum ad7124_registers regNr = static_cast<enum ad7124_registers> (AD7124_Channel_0 + channel); //Select _ADC_Control register
	uint32_t setValue = AD7124_ReadDeviceRegister(regNr);
	setValue |= (uint32_t)AD7124_CH_MAP_REG_CH_ENABLE;  //Enable channel0
	setValue &= 0xFFFF;
	AD7124_WriteDeviceRegister(regNr, setValue);    // Write data to _ADC
	delay(ms_delay);
}

void CN0391_disable_channel(int channel)
{
	enum ad7124_registers regNr = static_cast<enum ad7124_registers> (AD7124_Channel_0 + channel); //Select _ADC_Control register
	uint32_t setValue = AD7124_ReadDeviceRegister(regNr);
	setValue &= (~(uint32_t) AD7124_CH_MAP_REG_CH_ENABLE);  //Enable channel0
	setValue &= 0xFFFF;
	AD7124_WriteDeviceRegister(regNr, setValue);    // Write data to _ADC
	delay(ms_delay);
}

void CN0391_enable_current_source(int current_source_channel)
{
	enum ad7124_registers regNr = AD7124_IOCon1; //Select _ADC_Control register
	uint32_t setValue = AD7124_ReadDeviceRegister(regNr);
	setValue &= ~(AD7124_IO_CTRL1_REG_IOUT_CH0(0xF));
	setValue |= AD7124_IO_CTRL1_REG_IOUT_CH0(2*current_source_channel + 1);// set IOUT0 current
	setValue &= 0xFFFFFF;
	AD7124_WriteDeviceRegister(regNr, setValue);    // Write data to _ADC
	delay(ms_delay);
}

void CN0391_start_single_conversion()
{
	enum ad7124_registers regNr = AD7124_ADC_Control; //Select _ADC_Control register
	AD7124_WriteDeviceRegister(regNr, 0x0584);    // Write data to _ADC
}

void CN0391_reset() {

	AD7124_Reset();
	Serial.println(F("Reseted AD7124\n"));
}

void CN0391_setup() {

	AD7124_Setup();
}

void CN0391_init() {

	uint32_t setValue;
	int i;
	enum ad7124_registers regNr;
	CN0391_setup();

  // Set thermocouple types for all 4 channels
  th_types[CHANNEL_P1] = TYPE_K;
  th_types[CHANNEL_P2] = TYPE_K;
  th_types[CHANNEL_P3] = TYPE_K;
  th_types[CHANNEL_P4] = TYPE_K;


	delay(ms_delay);


	//Setup 0 -> RTD

	// Set Config_0 0x19
	regNr = AD7124_Config_0;               //Select Config_0 register
	setValue = AD7124_ReadDeviceRegister(regNr);
	setValue |= AD7124_CFG_REG_BIPOLAR;     //Select bipolar operation
	setValue |= AD7124_CFG_REG_BURNOUT(0);  //Burnout current source off
	setValue |= AD7124_CFG_REG_REF_BUFP;
	setValue |= AD7124_CFG_REG_REF_BUFM;
	setValue |= AD7124_CFG_REG_AIN_BUFP;
	setValue |= AD7124_CFG_REG_AINN_BUFM;
	setValue |= AD7124_CFG_REG_REF_SEL(1); //Select REFIN2(+)/REFIN2(-)
	setValue |= AD7124_CFG_REG_PGA(0);  //GAIN1
	setValue &= 0xFFFF;
	AD7124_WriteDeviceRegister(regNr, setValue);   // Write data to _ADC

   // Set AD7124_Filter_0 0x21
   regNr = AD7124_Filter_0;
   setValue = AD7124_ReadDeviceRegister(regNr);
   setValue |= AD7124_FILT_REG_FILTER(2);                     // set SINC3
   setValue |= AD7124_FILT_REG_FS(384);                     //FS = 48 => 50 SPS LOW power
   setValue &= 0xFFFFFF;
   AD7124_WriteDeviceRegister(regNr, setValue);// Write data to _ADC


   for(i = 0; i < 4; i++){
      // Set Channel_0 register 0x09
      regNr = static_cast<enum ad7124_registers>(AD7124_Channel_0 + i);
      setValue = AD7124_ReadDeviceRegister(regNr);
      setValue &= (~(uint32_t)AD7124_CH_MAP_REG_CH_ENABLE); //Disable channel
      setValue |= AD7124_CH_MAP_REG_SETUP(0);             // Select setup0
      setValue |= AD7124_CH_MAP_REG_AINP(2*i + 1);         // Set AIN1+, AIN3+, AIN5+, AIN7+

      if(i == 3){
            setValue |= AD7124_CH_MAP_REG_AINM(2*i - 1);   //AIN5-
      } else{
            setValue |= AD7124_CH_MAP_REG_AINM(2*i + 3);  //AIN3-, AIN5-, AIN7-
      }

      setValue &= 0xFFFF;
      AD7124_WriteDeviceRegister(regNr, setValue);   // Write data to _ADC
      delay(ms_delay);

      }


   // Set IO_Control_1 0x03
   regNr = AD7124_IOCon1;               //Select IO_Control_1 register
   setValue = AD7124_ReadDeviceRegister(regNr);
   setValue |= AD7124_IO_CTRL1_REG_IOUT0(5);// set IOUT0 current to 750uA
   setValue &= 0xFFFFFF;
   AD7124_WriteDeviceRegister(regNr, setValue);// Write data to _ADC


   //Setup 1 -> Thermocouple

  // Set Config_1
   regNr = AD7124_Config_1;
   setValue = AD7124_ReadDeviceRegister(regNr);
   setValue |= AD7124_CFG_REG_BIPOLAR;     //Select bipolar operation
   setValue |= AD7124_CFG_REG_BURNOUT(0);  //Burnout current source off
   setValue |= AD7124_CFG_REG_REF_BUFP;
   setValue |= AD7124_CFG_REG_REF_BUFM;
   setValue |= AD7124_CFG_REG_AIN_BUFP;
   setValue |= AD7124_CFG_REG_AINN_BUFM;
   setValue |= AD7124_CFG_REG_REF_SEL(2); //internal reference
   setValue |= AD7124_CFG_REG_PGA(5);  //GAIN32
   setValue &= 0xFFFF;
   AD7124_WriteDeviceRegister(regNr, setValue);   // Write data to _ADC

   // Set AD7124_Filter_0 0x21
   regNr = AD7124_Filter_1;
   setValue = AD7124_ReadDeviceRegister(regNr);
   setValue |= AD7124_FILT_REG_FILTER(2);                     // set SINC3
   setValue |= AD7124_FILT_REG_FS(384);                     //FS = 48 => 50 SPS
   setValue &= 0xFFFFFF;
   AD7124_WriteDeviceRegister(regNr, setValue);// Write data to _ADC


  //  for(i = 0; i < 4; i++){

  //     regNr = static_cast<enum ad7124_registers>(AD7124_Channel_4 + i);
  //     setValue = AD7124_ReadDeviceRegister(regNr);
  //     setValue &= (~(uint32_t)AD7124_CH_MAP_REG_CH_ENABLE); //Disable channel
  //     setValue |= AD7124_CH_MAP_REG_SETUP(1);             // Select setup0
  //     setValue |= AD7124_CH_MAP_REG_AINP(2*i);         // Set AIN0+, AIN2+, AIN4+, AIN6+
  //     setValue |= AD7124_CH_MAP_REG_AINM(15);         // Set AIN15 as negative
  //     setValue &= 0xFFFF;
  //     AD7124_WriteDeviceRegister(regNr, setValue);   // Write data to _ADC
  //     delay(ms_delay);

  //  }



for(i = 0; i < 4; i++){
    regNr = static_cast<enum ad7124_registers>(AD7124_Channel_4 + i);
    setValue = AD7124_ReadDeviceRegister(regNr);
    setValue &= (~(uint32_t)AD7124_CH_MAP_REG_CH_ENABLE); //Disable channel
    setValue |= AD7124_CH_MAP_REG_SETUP(1); // Use setup1 for thermocouple

    if (i == 3) {
        // 🔁 Swap polarity for CH7 (P1 thermocouple)
        setValue |= AD7124_CH_MAP_REG_AINP(6);  // Positive to AIN15
        setValue |= AD7124_CH_MAP_REG_AINM(15);   // Negative to AIN6
    } else {
        setValue |= AD7124_CH_MAP_REG_AINP(2*i);     // AIN0, AIN2, AIN4
        setValue |= AD7124_CH_MAP_REG_AINM(15);      // AIN15−
    }

    setValue &= 0xFFFF;
    AD7124_WriteDeviceRegister(regNr, setValue);
    delay(ms_delay);
}









// c:\Users\morga\Downloads\ADuCM360_demo_cn0391.bin
	// Set _ADC_Control 0x01
	regNr = AD7124_ADC_Control;            //Select _ADC_Control register
	setValue = AD7124_ReadDeviceRegister(regNr);
	setValue |= AD7124_ADC_CTRL_REG_DATA_STATUS; // set data status bit in order to check on which channel the conversion is
	setValue |= AD7124_ADC_CTRL_REG_REF_EN;
	setValue |= AD7124_ADC_CTRL_REG_POWER_MODE(0);  //set low power mode
	setValue &= 0xFFFF;
	AD7124_WriteDeviceRegister(regNr, setValue);    // Write data to _ADC
	delay(ms_delay);

  th_types[CHANNEL_P1] = P1_TYPE;
  th_types[CHANNEL_P2] = P2_TYPE;
  th_types[CHANNEL_P3] = P3_TYPE;
  th_types[CHANNEL_P4] = P4_TYPE;
}










// update parameters, generates slow down!

void CN0391_set_data(void)
{
   channel_t i;

  for(i = CHANNEL_P1; i <= CHANNEL_P4; i = static_cast<channel_t>(i+1)){

        CN0391_enable_current_source(i);
        
        _ADCValue0[i] = CN0391_read_channel(i);
        //Serial.println( _ADCValue0[i]);
        _ADCValue1[i] = CN0391_read_channel((i + 4));
        //Serial.println( _ADCValue1[i]);
        rRtdValue[i] = CN0391_data_to_resistance(_ADCValue0[i]);
        //Serial.println( rRtdValue[i]);
        CN0391_calc_rtd_temperature(i, &temp0[i]);
        
        CN0391_calc_th_temperature(i, temp0[i], &temp1[i]);
       

   }

}

//-------

void CN0391_display_data(void)
{
   channel_t i;

   for(i = CHANNEL_P1; i <= CHANNEL_P4; i = static_cast<channel_t>(i+1)){

         Serial.print(F("P")); Serial.print(i+1); Serial.print(F("channel (Type ")); Serial.print(thermocouple_type[th_types[i]]); Serial.println(F(")"));
         Serial.print(F("\t_ADC CJ code = ")); Serial.println((int)_ADCValue0[i]);
         Serial.print(F("\tR_rtd = ")); Serial.print(rRtdValue[i]); Serial.println(F(" ohmi"));
         Serial.print(F("\tcj_Temp = ")); Serial.println(temp0[i]);
         Serial.print(F("\tcj_Voltage = ")); Serial.print(cj_Voltage[i]); Serial.println(F(" mV"));
         Serial.print(F("\t_ADC TC code = ")); Serial.println((int)_ADCValue1[i]);
         Serial.print(F("\tth_Voltage_read = ")); Serial.print(th_Voltage_read[i]); Serial.println(F(" mV"));
         Serial.print(F("\tth_Voltage = ")); Serial.print(th_Voltage[i]); Serial.println(F(" mV"));


         if(errFlag[i] == ERR_UNDER_RANGE){
            Serial.println("");
            Serial.println(F("\tWARNING: Linearized temperature value is not available -> exceeds lower limit!!!"));
            Serial.print(F("\t\t Supported temperature range for thermocouple Type ")); Serial.print(thermocouple_type[th_types[i]]); Serial.print(F(" [")); Serial.print(PROGMEM_getAnything(&thTempRange[th_types[i]][0])); Serial.println(PROGMEM_getAnything(&thTempRange[th_types[i]][1])); Serial.println(F("]."));
            errFlag[i] = NO_ERR;
          } else{

                if(errFlag[i] == ERR_OVER_RANGE){
                  Serial.println("");
                  Serial.println(F("\tWARNING: Linearized temperature value is not available -> exceeds upper limit!!!"));
                  Serial.print(F("\t\t Supported temperature range for thermocouple Type ")); Serial.print(thermocouple_type[th_types[i]]); Serial.print(F("is [")); Serial.print(PROGMEM_getAnything(&thTempRange[th_types[i]][0])); Serial.println(PROGMEM_getAnything(&thTempRange[th_types[i]][1])); Serial.println(F("]."));
                  errFlag[i] = NO_ERR;
                }
                else{
                  Serial.print(F("\tth_temp = ")); Serial.print(temp1[i]); Serial.println(F(" °C"));
                }
          }

   }
   Serial.println("");
   Serial.println("");

}

// void CN0391_calc_rtd_temperature(channel_t ch, float *temp)
//  {
//      float rRtd;

//      rRtd = (R5*(_ADCValue0[ch] - _2_23))/(_2_23 *GAIN_RTD);

//       if(rRtd > R0) {

//         *temp = (-COEFF_A + sqrt(COEFF_A_A - COEFF_4B_R0*(R0 - rRtd)))/COEFF_2B;


//       } else {

//          POLY_CALC(*temp, rRtd/10.0, &cjPolyCoeff[0]);
//      }

//  }



void CN0391_calc_rtd_temperature(channel_t ch, float *temp)
{
    // ✅ Enable current source for RTD
    CN0391_enable_current_source(ch);

    // ✅ Read the ADC value from the RTD channel
    int32_t raw = CN0391_read_channel(ch);

    // ✅ Convert to resistance
    float rRtd = CN0391_data_to_resistance(raw);

    // ✅ Convert resistance to temperature
    if (rRtd > R0) {
        *temp = (-COEFF_A + sqrt(COEFF_A_A - COEFF_4B_R0 * (R0 - rRtd))) / COEFF_2B;
    } else {
        POLY_CALC(*temp, rRtd / 10.0, &cjPolyCoeff[0]);
    }
}










void CN0391_calc_th_temperature(channel_t ch, float cjTemp, float *_buffer)
{
    float cjVoltage, thVoltage;
    const temp_range thCoeff = PROGMEM_getAnything(&thPolyCoeff[th_types[ch]]);

    // thVoltage = ((VREF_INT*(_ADCValue1[ch] - _2_23))/(_2_23*GAIN_TH)) + TC_OFFSET_VOLTAGE;

    thVoltage = ((float)_ADCValue1[ch] / _2_23 - 1.0) * (VREF_INT / GAIN_TH) + TC_OFFSET_VOLTAGE;


    th_Voltage_read[ch]= thVoltage;

      if(cjTemp <  PROGMEM_getAnything(&cjTempRange[th_types[ch]][1])) {
         POLY_CALC(cjVoltage, cjTemp, thCoeff.neg_temp);
      } else {

        if(cjTemp <=  PROGMEM_getAnything(&cjTempRange[th_types[ch]][2])){
          // Serial.print("Triggering pos_voltage1 conversion for voltage: "); 
          // Serial.println(thVoltage, 6);


          POLY_CALC(cjVoltage, cjTemp, thCoeff.pos_temp1);
          if(th_types[ch] == TYPE_K){
             cjVoltage += COEFF_K_A0*exp(COEFF_K_A1*(cjTemp - COEFF_K_A2)*(cjTemp - COEFF_K_A2));

            //  cjVoltage += 1.1;
           }
        } else{
          POLY_CALC(cjVoltage, cjTemp, thCoeff.pos_temp2);
        }
      }
      cj_Voltage[ch] = cjVoltage;

      thVoltage += cjVoltage;
      th_Voltage[ch] = thVoltage;


      int32_t microvolts = (int32_t)(thVoltage * 1000);
      // Serial.print(F("Corrected Voltage (µV): "));
      // Serial.println(microvolts);



// 🔧 DEBUG: Show actual thermocouple voltage after CJ correction
      // Serial.print("Actual corrected thermocouple voltage: ");
      // Serial.println(thVoltage, 6);

      // 🔒 Clamp extreme/floating values just in case
      if (thVoltage < -10 || thVoltage > 70) {
      // Serial.println("⚠️  Clamped thVoltage due to unexpected value.");
      thVoltage = 0;
}

      

      th_Voltage[ch] = thVoltage;

      // Serial.print("Actual corrected thermocouple voltage: ");
      // Serial.println(thVoltage, 6);


      if(thVoltage < PROGMEM_getAnything(&thVoltageRange[th_types[ch]][0])) {
            // Serial.println("⚠️  Voltage below supported range");
            errFlag[ch] = ERR_UNDER_RANGE;
      } else{
            if(thVoltage < PROGMEM_getAnything(&thVoltageRange[th_types[ch]][1])) {
              // Serial.println("✅ Using neg_voltage coefficients...");
              POLY_CALC(*_buffer, thVoltage, thCoeff.neg_voltage);
            } else {
                if(thVoltage <= PROGMEM_getAnything(&thVoltageRange[th_types[ch]][2])) {
                  // Serial.println("✅ Using pos_voltage1 coefficients...");
                  POLY_CALC(*_buffer, thVoltage, thCoeff.pos_voltage1);
                }else{

                   if((thVoltage <= PROGMEM_getAnything(&thVoltageRange[th_types[ch]][3])) && (thCoeff.pos_voltage2[0] != 1.0f)) {
                    POLY_CALC(*_buffer, thVoltage, thCoeff.pos_voltage2);
                  }else{
                        if(thCoeff.pos_voltage3[0] != 1.0f){
                           if(thVoltage <= PROGMEM_getAnything(&thVoltageRange[th_types[ch]][4])){
                                 POLY_CALC(*_buffer, thVoltage, thCoeff.pos_voltage3);
                           }
                           else{
                                 errFlag[ch] = ERR_OVER_RANGE;
                           }
                        }
                        else{
                              errFlag[ch] = ERR_OVER_RANGE;
                        }
                  }
                }
             }
      }

}

void CN0391_calibration(uint8_t channel)
{

      if(channel == RTD_CHANNEL){

                 CN0391_enable_current_source(1);
                 CN0391_enable_channel(0);
                 CN0391_set_calibration_mode(0x514);
                 while(AD7124_ReadDeviceRegister(AD7124_ADC_Control) != 0x0510);  //wait for idle mode
                 CN0391_disable_channel(0);
      } else{

                  CN0391_enable_channel(4);
                  AD7124_WriteDeviceRegister(AD7124_Offset_1, 0x800000);
                  CN0391_set_calibration_mode(0x518);
                  while(AD7124_ReadDeviceRegister(AD7124_ADC_Control) != 0x0510);  //wait for idle mode
                  CN0391_set_calibration_mode(0x514);
                  while(AD7124_ReadDeviceRegister(AD7124_ADC_Control) != 0x0510);  //wait for idle mode
                  CN0391_disable_channel(4);
      }

      AD7124_WriteDeviceRegister(AD7124_ADC_Control, 0x588);
}

void CN0391_read_reg(void)
{
   enum ad7124_registers regNr;

   for(regNr = AD7124_Status; regNr < AD7124_REG_NO;regNr = static_cast<enum ad7124_registers>(regNr + 1)) {

         AD7124_ReadDeviceRegister(regNr);

   }

}

void CN0391_set_calibration_mode(uint16_t mode)
{
   convFlag = 1;
   digitalWrite(CS_PIN, LOW);

   AD7124_WriteDeviceRegister(AD7124_ADC_Control, mode);

   if (AD7124_WaitForConvReady(100000) == -3) {  // blocking function
        Serial.println("TIMEOUT");

     }
   convFlag = 0;
   digitalWrite(CS_PIN, HIGH);

}


void CN0391_set_power_mode(int mode)
{
   enum ad7124_registers regNr = AD7124_ADC_Control;            //Select ADC_Control register
   uint32_t setValue = AD7124_ReadDeviceRegister(regNr);
   setValue |= AD7124_ADC_CTRL_REG_POWER_MODE(mode);  //set low power mode
   setValue &= 0xFFFF;
   AD7124_WriteDeviceRegister(regNr, setValue);    // Write data to ADC
   delay(ms_delay);
}

















// #include "Arduino.h"
// #include "CN0391.h"
// #include "RTD.h"
// #include "Thermocouple.h"
// #include "AD7124.h"
// #include "Communication.h"
// #include <stdio.h>
// #include <math.h>

// // Global variables
// int32_t _ADCValue0[4], _ADCValue1[4];
// float rRtdValue[4], temp0[4], temp1[4];
// float cj_Voltage[4], th_Voltage_read[4], th_Voltage[4];
// static const unsigned char thermocouple_type[] = { 'T', 'J', 'K', 'E', 'S', 'R', 'N', 'B' };
// uint8_t th_types[4];
// error_code errFlag[4] = { NO_ERR, NO_ERR, NO_ERR, NO_ERR };
// #define ms_delay (1)

// // Function to set thermocouple type for a specific channel
// void CN0391_set_thermocouple_type(uint8_t channel, uint8_t type) {
//     if (channel < 4) {
//         th_types[channel] = type; // Set the thermocouple type for the specified channel
//     }
// }

// // Function to convert ADC data to temperature
// float CN0391_data_to_temperature(int32_t data, uint8_t channel) {
//     float voltage = CN0391_data_to_voltage(data, channel); // Convert ADC data to voltage
//     float temperature = 0.0;

//     // Convert voltage to temperature based on thermocouple type
//     switch (th_types[channel]) {
//         case TYPE_K:
//             temperature = voltage * 25.0; // Example conversion for Type K thermocouple
//             break;
//         case TYPE_J:
//             temperature = voltage * 20.0; // Example conversion for Type J thermocouple
//             break;
//         // Add cases for other thermocouple types as needed
//         default:
//             temperature = 0.0; // Default value
//             break;
//     }

//     return temperature;
// }

// // Function to read data from a specific channel
// int32_t CN0391_read_channel(int ch) {
//     int32_t data;
//     CN0391_enable_channel(ch);
//     convFlag = 1;
//     digitalWrite(CS_PIN, LOW);
//     CN0391_start_single_conversion();

//     if (AD7124_WaitForConvReady(10000) == -3) {
//         Serial.println("TIMEOUT");
//         return 0;
//     }

//     delay(100);
//     AD7124_ReadData(&data);
//     convFlag = 0;
//     digitalWrite(CS_PIN, HIGH);
//     CN0391_disable_channel(ch);
//     return data;
// }

// // Function to convert ADC data to voltage
// float CN0391_data_to_voltage(int32_t data, uint8_t channel) {
//     float voltage;
//     if (channel == RTD_CHANNEL) {
//         voltage = (VREF_EXT * (data - _2_23)) / (_2_23 * GAIN_RTD);
//     } else {
//         voltage = (VREF_INT * (data - _2_23)) / (_2_23 * GAIN_TH);
//     }
//     return voltage;
// }

// // Function to convert ADC data to resistance
// float CN0391_data_to_resistance(int32_t data) {
//     float rRtd;
//     rRtd = (R5 * (data - _2_23)) / (_2_23 * GAIN_RTD);
//     return rRtd;
// }

// // Function to enable a specific channel
// void CN0391_enable_channel(int channel) {
//     enum ad7124_registers regNr = static_cast<enum ad7124_registers>(AD7124_Channel_0 + channel);
//     uint32_t setValue = AD7124_ReadDeviceRegister(regNr);
//     setValue |= (uint32_t)AD7124_CH_MAP_REG_CH_ENABLE;
//     setValue &= 0xFFFF;
//     AD7124_WriteDeviceRegister(regNr, setValue);
//     delay(ms_delay);
// }

// // Function to disable a specific channel
// void CN0391_disable_channel(int channel) {
//     enum ad7124_registers regNr = static_cast<enum ad7124_registers>(AD7124_Channel_0 + channel);
//     uint32_t setValue = AD7124_ReadDeviceRegister(regNr);
//     setValue &= (~(uint32_t)AD7124_CH_MAP_REG_CH_ENABLE);
//     setValue &= 0xFFFF;
//     AD7124_WriteDeviceRegister(regNr, setValue);
//     delay(ms_delay);
// }

// // Function to enable current source
// void CN0391_enable_current_source(int current_source_channel) {
//     enum ad7124_registers regNr = AD7124_IOCon1;
//     uint32_t setValue = AD7124_ReadDeviceRegister(regNr);
//     setValue &= ~(AD7124_IO_CTRL1_REG_IOUT_CH0(0xF));
//     setValue |= AD7124_IO_CTRL1_REG_IOUT_CH0(2 * current_source_channel + 1);
//     setValue &= 0xFFFFFF;
//     AD7124_WriteDeviceRegister(regNr, setValue);
//     delay(ms_delay);
// }

// // Function to start a single conversion
// void CN0391_start_single_conversion() {
//     enum ad7124_registers regNr = AD7124_ADC_Control;
//     AD7124_WriteDeviceRegister(regNr, 0x0584);
// }

// // Function to reset the AD7124
// void CN0391_reset() {
//     AD7124_Reset();
//     Serial.println(F("Reseted AD7124\n"));
// }

// // Function to set up the AD7124
// void CN0391_setup() {
//     AD7124_Setup();
// }

// // Function to initialize the CN0391
// void CN0391_init() {
//     uint32_t setValue;
//     int i;
//     enum ad7124_registers regNr;
//     CN0391_setup();
//     delay(ms_delay);

//     // Setup for RTD channels
//     regNr = AD7124_Config_0;
//     setValue = AD7124_ReadDeviceRegister(regNr);
//     setValue |= AD7124_CFG_REG_BIPOLAR | AD7124_CFG_REG_BURNOUT(0) | AD7124_CFG_REG_REF_BUFP |
//                AD7124_CFG_REG_REF_BUFM | AD7124_CFG_REG_AIN_BUFP | AD7124_CFG_REG_AINN_BUFM |
//                AD7124_CFG_REG_REF_SEL(1) | AD7124_CFG_REG_PGA(0);
//     setValue &= 0xFFFF;
//     AD7124_WriteDeviceRegister(regNr, setValue);

//     regNr = AD7124_Filter_0;
//     setValue = AD7124_ReadDeviceRegister(regNr);
//     setValue |= AD7124_FILT_REG_FILTER(2) | AD7124_FILT_REG_FS(384);
//     setValue &= 0xFFFFFF;
//     AD7124_WriteDeviceRegister(regNr, setValue);

//     for (i = 0; i < 4; i++) {
//         regNr = static_cast<enum ad7124_registers>(AD7124_Channel_0 + i);
//         setValue = AD7124_ReadDeviceRegister(regNr);
//         setValue &= (~(uint32_t)AD7124_CH_MAP_REG_CH_ENABLE);
//         setValue |= AD7124_CH_MAP_REG_SETUP(0) | AD7124_CH_MAP_REG_AINP(2 * i + 1);
//         if (i == 3) {
//             setValue |= AD7124_CH_MAP_REG_AINM(2 * i - 1);
//         } else {
//             setValue |= AD7124_CH_MAP_REG_AINM(2 * i + 3);
//         }
//         setValue &= 0xFFFF;
//         AD7124_WriteDeviceRegister(regNr, setValue);
//         delay(ms_delay);
//     }

//     // Setup for thermocouple channels
//     regNr = AD7124_Config_1;
//     setValue = AD7124_ReadDeviceRegister(regNr);
//     setValue |= AD7124_CFG_REG_BIPOLAR | AD7124_CFG_REG_BURNOUT(0) | AD7124_CFG_REG_REF_BUFP |
//                AD7124_CFG_REG_REF_BUFM | AD7124_CFG_REG_AIN_BUFP | AD7124_CFG_REG_AINN_BUFM |
//                AD7124_CFG_REG_REF_SEL(2) | AD7124_CFG_REG_PGA(5);
//     setValue &= 0xFFFF;
//     AD7124_WriteDeviceRegister(regNr, setValue);

//     regNr = AD7124_Filter_1;
//     setValue = AD7124_ReadDeviceRegister(regNr);
//     setValue |= AD7124_FILT_REG_FILTER(2) | AD7124_FILT_REG_FS(384);
//     setValue &= 0xFFFFFF;
//     AD7124_WriteDeviceRegister(regNr, setValue);

//     for (i = 0; i < 4; i++) {
//         regNr = static_cast<enum ad7124_registers>(AD7124_Channel_4 + i);
//         setValue = AD7124_ReadDeviceRegister(regNr);
//         setValue &= (~(uint32_t)AD7124_CH_MAP_REG_CH_ENABLE);
//         setValue |= AD7124_CH_MAP_REG_SETUP(1) | AD7124_CH_MAP_REG_AINP(2 * i) | AD7124_CH_MAP_REG_AINM(15);
//         setValue &= 0xFFFF;
//         AD7124_WriteDeviceRegister(regNr, setValue);
//         delay(ms_delay);
//     }

//     // Set ADC control
//     regNr = AD7124_ADC_Control;
//     setValue = AD7124_ReadDeviceRegister(regNr);
//     setValue |= AD7124_ADC_CTRL_REG_DATA_STATUS | AD7124_ADC_CTRL_REG_REF_EN | AD7124_ADC_CTRL_REG_POWER_MODE(0);
//     setValue &= 0xFFFF;
//     AD7124_WriteDeviceRegister(regNr, setValue);
//     delay(ms_delay);

//     // Set default thermocouple types
//     th_types[CHANNEL_P1] = P1_TYPE;
//     th_types[CHANNEL_P2] = P2_TYPE;
//     th_types[CHANNEL_P3] = P3_TYPE;
//     th_types[CHANNEL_P4] = P4_TYPE;
// }

// // Function to calculate RTD temperature
// void CN0391_calc_rtd_temperature(channel_t ch, float *temp) {
//     float rRtd;
//     rRtd = (R5 * (_ADCValue0[ch] - _2_23)) / (_2_23 * GAIN_RTD);

//     if (rRtd > R0) {
//         *temp = (-COEFF_A + sqrt(COEFF_A_A - COEFF_4B_R0 * (R0 - rRtd))) / COEFF_2B;
//     } else {
//         POLY_CALC(*temp, rRtd / 10.0, &cjPolyCoeff[0]);
//     }
// }

// // Function to calculate thermocouple temperature
// void CN0391_calc_th_temperature(channel_t ch, float cjTemp, float *_buffer) {
//     float cjVoltage, thVoltage;
//     const temp_range thCoeff = PROGMEM_getAnything(&thPolyCoeff[th_types[ch]]);

//     thVoltage = ((VREF_INT * (_ADCValue1[ch] - _2_23)) / (_2_23 * GAIN_TH)) + TC_OFFSET_VOLTAGE;
//     th_Voltage_read[ch] = thVoltage;

//     if (cjTemp < PROGMEM_getAnything(&cjTempRange[th_types[ch]][1])) {
//         POLY_CALC(cjVoltage, cjTemp, thCoeff.neg_temp);
//     } else if (cjTemp <= PROGMEM_getAnything(&cjTempRange[th_types[ch]][2])) {
//         POLY_CALC(cjVoltage, cjTemp, thCoeff.pos_temp1);
//         if (th_types[ch] == TYPE_K) {
//             cjVoltage += COEFF_K_A0 * exp(COEFF_K_A1 * (cjTemp - COEFF_K_A2) * (cjTemp - COEFF_K_A2));
//         }
//     } else {
//         POLY_CALC(cjVoltage, cjTemp, thCoeff.pos_temp2);
//     }

//     cj_Voltage[ch] = cjVoltage;
//     thVoltage += cjVoltage;
//     th_Voltage[ch] = thVoltage;

//     if (thVoltage < PROGMEM_getAnything(&thVoltageRange[th_types[ch]][0])) {
//         errFlag[ch] = ERR_UNDER_RANGE;
//     } else if (thVoltage < PROGMEM_getAnything(&thVoltageRange[th_types[ch]][1])) {
//         POLY_CALC(*_buffer, thVoltage, thCoeff.neg_voltage);
//     } else if (thVoltage <= PROGMEM_getAnything(&thVoltageRange[th_types[ch]][2])) {
//         POLY_CALC(*_buffer, thVoltage, thCoeff.pos_voltage1);
//     } else if ((thVoltage <= PROGMEM_getAnything(&thVoltageRange[th_types[ch]][3])) && (thCoeff.pos_voltage2[0] != 1.0f)) {
//         POLY_CALC(*_buffer, thVoltage, thCoeff.pos_voltage2);
//     } else if (thCoeff.pos_voltage3[0] != 1.0f) {
//         if (thVoltage <= PROGMEM_getAnything(&thVoltageRange[th_types[ch]][4])) {
//             POLY_CALC(*_buffer, thVoltage, thCoeff.pos_voltage3);
//         } else {
//             errFlag[ch] = ERR_OVER_RANGE;
//         }
//     } else {
//         errFlag[ch] = ERR_OVER_RANGE;
//     }
// }

// // Function to set data for all channels
// void CN0391_set_data(void) {
//     channel_t i;

//     for (i = CHANNEL_P1; i <= CHANNEL_P4; i = static_cast<channel_t>(i + 1)) {
//         CN0391_enable_current_source(i);
//         _ADCValue0[i] = CN0391_read_channel(i);
//         _ADCValue1[i] = CN0391_read_channel((i + 4));
//         rRtdValue[i] = CN0391_data_to_resistance(_ADCValue0[i]);
//         CN0391_calc_rtd_temperature(i, &temp0[i]);
//         CN0391_calc_th_temperature(i, temp0[i], &temp1[i]);
//     }
// }

// // Function to display data for all channels
// void CN0391_display_data(void) {
//     channel_t i;

//     for (i = CHANNEL_P1; i <= CHANNEL_P4; i = static_cast<channel_t>(i + 1)) {
//         Serial.print(F("P")); Serial.print(i + 1); Serial.print(F(" channel (Type ")); Serial.print(thermocouple_type[th_types[i]]); Serial.println(F(")"));
//         Serial.print(F("\tADC CJ code = ")); Serial.println((int)_ADCValue0[i]);
//         Serial.print(F("\tR_rtd = ")); Serial.print(rRtdValue[i]); Serial.println(F(" ohms"));
//         Serial.print(F("\tcj_Temp = ")); Serial.println(temp0[i]);
//         Serial.print(F("\tcj_Voltage = ")); Serial.print(cj_Voltage[i]); Serial.println(F(" mV"));
//         Serial.print(F("\tADC TC code = ")); Serial.println((int)_ADCValue1[i]);
//         Serial.print(F("\tth_Voltage_read = ")); Serial.print(th_Voltage_read[i]); Serial.println(F(" mV"));
//         Serial.print(F("\tth_Voltage = ")); Serial.print(th_Voltage[i]); Serial.println(F(" mV"));

//         if (errFlag[i] == ERR_UNDER_RANGE) {
//             Serial.println("");
//             Serial.println(F("\tWARNING: Linearized temperature value is not available -> exceeds lower limit!!!"));
//             Serial.print(F("\t\tSupported temperature range for thermocouple Type ")); Serial.print(thermocouple_type[th_types[i]]); Serial.print(F(" [")); Serial.print(PROGMEM_getAnything(&thTempRange[th_types[i]][0])); Serial.println(PROGMEM_getAnything(&thTempRange[th_types[i]][1])); Serial.println(F("]."));
//             errFlag[i] = NO_ERR;
//         } else if (errFlag[i] == ERR_OVER_RANGE) {
//             Serial.println("");
//             Serial.println(F("\tWARNING: Linearized temperature value is not available -> exceeds upper limit!!!"));
//             Serial.print(F("\t\tSupported temperature range for thermocouple Type ")); Serial.print(thermocouple_type[th_types[i]]); Serial.print(F(" [")); Serial.print(PROGMEM_getAnything(&thTempRange[th_types[i]][0])); Serial.println(PROGMEM_getAnything(&thTempRange[th_types[i]][1])); Serial.println(F("]."));
//             errFlag[i] = NO_ERR;
//         } else {
//             Serial.print(F("\tth_temp = ")); Serial.print(temp1[i]); Serial.println(F(" °C"));
//         }
//     }
//     Serial.println("");
//     Serial.println("");
// }

// // Function to perform calibration
// void CN0391_calibration(uint8_t channel) {
//     if (channel == RTD_CHANNEL) {
//         CN0391_enable_current_source(1);
//         CN0391_enable_channel(0);
//         CN0391_set_calibration_mode(0x514);
//         while (AD7124_ReadDeviceRegister(AD7124_ADC_Control) != 0x0510); // Wait for idle mode
//     }
// }