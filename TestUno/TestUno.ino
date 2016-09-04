#include <Arduino.h>

#if defined(ARDUINO_AVR_UNO)
int x = 2;
#else
int x = 3;
#endif

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println(x);
  delay(1000);
}
