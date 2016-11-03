#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#include <Wire.h>

/* 
 ***************************************************************************************
 *
 *  Functions for I2C Communication
 *
 ***************************************************************************************
 */

void i2c_write (uint8_t i2c_addr, uint8_t register_addr, char* buffer, uint8_t Nbyte )
{
    int ret;
    char *tmp_buffer;
        
    tmp_buffer = (char*)malloc(sizeof(char)*(Nbyte+1));
        
    /* First, send device address. Then, send data and STOP condition */
    tmp_buffer[0] = register_addr;
    memcpy(tmp_buffer+1, buffer, Nbyte);

    //ret = i2c.write(i2c_addr, tmp_buffer, Nbyte+1, false);

    Wire.beginTransmission(i2c_addr);
    Wire.write(tmp_buffer);
    Wire.endTransmission();
    
    return;
}

static int i2c_read (uint8_t i2c_addr, uint8_t register_addr, char* buffer, uint8_t Nbyte )
{
    int ret;
    
    /* Send device address, with no STOP condition */
    ret = i2c.write(i2c_addr, (const char*)&register_addr, 1, true);
    if(!ret) {
        /* Read data, with STOP condition  */
        ret = i2c.read((i2c_addr|0x01), buffer, Nbyte, false);
    }

    return ret;
}

/* 
 ***************************************************************************************
 *
 *  Functions for ....
 *
 ***************************************************************************************
 */
 #endif