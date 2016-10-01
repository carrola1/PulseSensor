//-------------------------------------------------------------------------------
//  TinyCircuits TinyScreen/TinyScreen+ Basic Example
//  Last Updated 26 January 2016
//  
//  This example shows the basic functionality of the TinyScreen library,
//  including drawing, writing bitmaps, and printing text
//
//  Written by Ben Rose for TinyCircuits, https://tiny-circuits.com
//
//-------------------------------------------------------------------------------

#include <Wire.h>
#include <SPI.h>
#include <TinyScreen.h>

TinyScreen display = TinyScreen(TinyScreenPlus);

void setup(void) {
  display.begin();
  //setBrightness(brightness);//sets main current level, valid levels are 0-15
  display.setBrightness(10);
  Wire.begin();
  SerialUSB.begin(9600);
}

byte x;
byte addr = 0xFF;
int rtn;
void loop() {
  //if (x == 120) {
  //  x = 80;
  //} else {
  //  x = x + 1;
  //}
  Wire.beginTransmission(87);
  Wire.write(addr);
  rtn = Wire.endTransmission(false);
  Wire.requestFrom(87,1,true);
  x = Wire.read();
  SerialUSB.println(x);
  writeText(x);
}

void writeText(int rate){
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
  char HR_text[width];
  sprintf(HR_text, "Heart Rate = %3.3d",rate);
  display.print(HR_text);
  display.setCursor(15,25);
  display.fontColor(TS_8b_Blue,TS_8b_Black);
  if (rate < 100) {
    display.print("Pick it up!");
  } else {
    display.print("Keep it up!");
  }
  delay(500);
}
