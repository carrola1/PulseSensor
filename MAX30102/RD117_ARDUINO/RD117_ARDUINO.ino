/** \file RD117_LILYPAD.ino ******************************************************
*
* Project: MAXREFDES117#
* Filename: RD117_LILYPAD.ino
* Description: This module contains the Main application for the MAXREFDES117 example program.
*
* Revision History:
*\n 1-18-2016 Rev 01.00 GL Initial release.
*\n
*
* --------------------------------------------------------------------
*
* This code follows the following naming conventions:
*
* char              ch_pmod_value
* char (array)      s_pmod_s_string[16]
* float             f_pmod_value
* int32_t           n_pmod_value
* int32_t (array)   an_pmod_value[16]
* int16_t           w_pmod_value
* int16_t (array)   aw_pmod_value[16]
* uint16_t          uw_pmod_value
* uint16_t (array)  auw_pmod_value[16]
* uint8_t           uch_pmod_value
* uint8_t (array)   auch_pmod_buffer[16]
* uint32_t          un_pmod_value
* int32_t *         pn_pmod_value
*
* ------------------------------------------------------------------------- */
/*******************************************************************************
* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/
/*!\mainpage Main Page
*
* \section intro_sec Introduction
*
* This is the code documentation for the MAXREFDES117# subsystem reference design.
* 
*  The Files page contains the File List page and the Globals page.
* 
*  The Globals page contains the Functions, Variables, and Macros sub-pages.
*
* \image html MAXREFDES117_Block_Diagram.png "MAXREFDES117# System Block Diagram"
* 
* \image html MAXREFDES117_firmware_Flowchart.png "MAXREFDES117# Firmware Flowchart"
*
*/
#include <Arduino.h>
#include "algorithm.h"
#include "max30102.h"
#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>

TinyScreen display = TinyScreen(TinyScreenPlus);

#define MAX_BRIGHTNESS 255

uint32_t aun_ir_buffer[100]; //infrared LED sensor data
uint32_t aun_red_buffer[100];  //red LED sensor data
int32_t n_ir_buffer_length; //data length
int32_t n_spo2;  //SPO2 value
int8_t ch_spo2_valid;  //indicator to show if the SPO2 calculation is valid
int32_t n_heart_rate; //heart rate value
int8_t  ch_hr_valid;  //indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;

int32_t hr_buf[4];
int32_t hrSum;
int32_t hrAvg;
int32_t spo2_buf[4];
int32_t spo2Sum;
int32_t spo2Avg;

// the setup routine runs once when you press reset:
void setup() {
  delay(1000);
  SerialUSB.begin(115200);
  Wire.begin();
  display.begin();
  display.setBrightness(10);  //setBrightness(brightness);//sets main current level, valid levels are 0-15
  maxim_max30102_reset(); //resets the MAX30102
  pinMode(8, INPUT);  //pin D8 connects to the interrupt output pin of the MAX30102
  maxim_max30102_read_reg(REG_INTR_STATUS_1,&uch_dummy);  //Reads/clears the interrupt status register
  maxim_max30102_init();  //initialize the MAX30102
}

// the loop routine runs over and over again forever:
void loop() {
  uint32_t un_min, un_max, un_prev_data, un_brightness;  //variables to calculate the on-board LED brightness that reflects the heartbeats
  int32_t i;
  float f_temp;
  
  un_brightness=0;
  un_min=0x3FFFF;
  un_max=0;
  
  n_ir_buffer_length=100;  //buffer length of 100 stores 4 seconds of samples running at 25sps

  //read the first 100 samples, and determine the signal range
  for(i=0;i<n_ir_buffer_length;i++)
  {
    while(digitalRead(8)==1);  //wait until the interrupt pin asserts
    maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));  //read from MAX30102 FIFO
    
    if(un_min>aun_red_buffer[i])
      un_min=aun_red_buffer[i];  //update signal min
    if(un_max<aun_red_buffer[i])
      un_max=aun_red_buffer[i];  //update signal max
    /*SerialUSB.print(F("red="));
    SerialUSB.print(aun_red_buffer[i], DEC);
    SerialUSB.print(F(", ir="));
    SerialUSB.println(aun_ir_buffer[i], DEC);*/
  }
  un_prev_data=aun_red_buffer[i];
  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 

  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while(1)
  {
    i=0;
    un_min=0x3FFFF;
    un_max=0;

    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for(i=25;i<100;i++)
    {
      aun_red_buffer[i-25]=aun_red_buffer[i];
      aun_ir_buffer[i-25]=aun_ir_buffer[i];

      //update the signal min and max
      if(un_min>aun_red_buffer[i])
        un_min=aun_red_buffer[i];
      if(un_max<aun_red_buffer[i])
        un_max=aun_red_buffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for(i=75;i<100;i++)
    {
      un_prev_data=aun_red_buffer[i-1];
      while(digitalRead(8)==1);
      maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));

      //calculate the brightness of the LED
      if(aun_red_buffer[i]>un_prev_data)
      {
        f_temp=aun_red_buffer[i]-un_prev_data;
        f_temp/=(un_max-un_min);
        f_temp*=MAX_BRIGHTNESS;
        f_temp=un_brightness-f_temp;
        if(f_temp<0)
          un_brightness=0;
        else
          un_brightness=(int)f_temp;
      }
      else
      {
        f_temp=un_prev_data-aun_red_buffer[i];
        f_temp/=(un_max-un_min);
        f_temp*=MAX_BRIGHTNESS;
        un_brightness+=(int)f_temp;
        if(un_brightness>MAX_BRIGHTNESS)
          un_brightness=MAX_BRIGHTNESS;
      }

      //send samples and calculation result to terminal program through UART
      SerialUSB.print(F("red="));
      SerialUSB.print(aun_red_buffer[i], DEC);
      SerialUSB.print(F(", ir="));
      SerialUSB.print(aun_ir_buffer[i], DEC);
      
      SerialUSB.print(F(", HR="));
      SerialUSB.print(n_heart_rate, DEC);
      
      SerialUSB.print(F(", HRvalid="));
      SerialUSB.print(ch_hr_valid, DEC);
      
      SerialUSB.print(F(", SPO2="));
      SerialUSB.print(n_spo2, DEC);

      SerialUSB.print(F(", SPO2Valid="));
      SerialUSB.println(ch_spo2_valid, DEC);

      SerialUSB.println(aun_red_buffer[i], DEC);
    }
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
    
    if (ch_hr_valid == 1) {
      if(n_heart_rate < 180) {
        if(n_heart_rate > 45) {
          hrSum = 0;
          for(i=0;i<3;i++) {
            hr_buf[i] = hr_buf[i+1];
            hrSum = hrSum + hr_buf[i+1];
          }
          hr_buf[3] = n_heart_rate;
          hrSum     = hrSum + n_heart_rate;
        }
      }
    } 
    hrAvg = hrSum>>2;
    
    if (ch_spo2_valid == 1) {
      if(n_spo2 > 59) {
        spo2Sum = 0;
        for(i=0;i<3;i++) {
          spo2_buf[i] = spo2_buf[i+1];
          spo2Sum = spo2Sum + spo2_buf[i+1];
        }
        spo2_buf[3] = n_spo2;
        spo2Sum     = spo2Sum + n_spo2;
      }
    }
    spo2Avg = spo2Sum>>2;   
    writeText(hrAvg, spo2Avg); 
  }
}

void writeText(int rate, int spo2){
  display.clearScreen();
  //setFont sets a font info header from font.h
  //information for generating new fonts is included in font.h
  display.setFont(thinPixel7_10ptFontInfo);
  
  //getPrintWidth(character array);//get the pixel print width of a string
  int width=display.getPrintWidth("Heart Rate = ___");
  //setCursor(x,y);//set text cursor position to (x,y)- in this example, the example string is centered
  display.setCursor(48-(width/2),10);
  //fontColor(text color, background color);//sets text and background color
  display.fontColor(TS_8b_Red,TS_8b_Black);
  char text[width];
  sprintf(text, "Heart Rate = %3.3d",rate);
  display.print(text);
  
  width=display.getPrintWidth("SPO2 = ___");
  //setCursor(x,y);//set text cursor position to (x,y)- in this example, the example string is centered
  display.setCursor(48-(width/2),25);
  //fontColor(text color, background color);//sets text and background color
  display.fontColor(TS_8b_Blue,TS_8b_Black);
  char text2[width];
  sprintf(text2, "SPO2 = %3.3d",spo2);
  display.print(text2);

  width=display.getPrintWidth("Pick it up!");
  display.setCursor(48-(width/2),40);
  display.fontColor(TS_8b_Green,TS_8b_Black);
  if (rate < 90) {
    display.print("Pick it up!");
  } else {
    display.print("Keep it up!");
  }
}
 
