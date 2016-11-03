
#include "MAX30102.h" 

//I2C i2c(I2C_SDA, I2C_SCL);
DigitalOut myled(LED1);
static Serial pc(SERIAL_TX, SERIAL_RX);

MAX30102 sensor;

int main()
{
    wait(1.0);
    sensor.printRegisters();
    wait(0.5);
    sensor.setLEDs(pw411, i21, i21);
    wait(0.1);
    sensor.setFIFO(avg4);
    wait(0.5);
    sensor.printRegisters();
    wait(0.5);
    sensor.setHRmode();
    
    while(1) {
        int dataRdy = sensor.getDataRdyStatus();
        if (dataRdy == 0x01) {
            sensor.readSensor();
            pc.printf("%d\r\n", sensor.HR);
            myled = 1;
        } else {
            myled = 0;
        }
        wait(0.02);
    }
}
