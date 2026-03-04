/**
 * CST328 touch controller for Waveshare ESP32-S3-Touch-LCD-2.8.
 * Uses Arduino TwoWire (Wire or Wire1); 16-bit regs, 0x1A. Use Wire1 for 2.8" to avoid conflict.
 */
#ifndef _CST328_H
#define _CST328_H

#include <cstdint>
#include <Wire.h>

#define CST328_REG_READ_NUMBER  0xD005
#define CST328_REG_READ_XY      0xD000
/* Demo puts chip in touch mode after reset (touch_cst328_read_cfg) */
#define CST328_REG_NORMAL_MODE  0xD109

class CST328 {
public:
    CST328(int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin);
    CST328(TwoWire* wire, int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin);

    void begin(void);
    bool getTouch(uint16_t* x, uint16_t* y, uint8_t* gesture);

private:
    TwoWire* _wire;
    int8_t _sda, _scl, _rst, _int;
    uint8_t _addr;
    void recoverBus(void);
    bool i2c_read_reg16(uint16_t reg, uint8_t* data, uint8_t len);
    bool i2c_write_reg16(uint16_t reg, const uint8_t* data, uint8_t len);
};

#endif
