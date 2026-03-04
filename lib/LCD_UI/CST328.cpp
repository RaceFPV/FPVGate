/**
 * CST328 touch driver for Waveshare 2.8. Matches demo init/sequence; uses Wire1 (driver_ng).
 */
#include "CST328.h"
#ifdef CST328_DEBUG
#include <Arduino.h>
#endif

static const uint8_t ADDR_1A = 0x1A;

CST328::CST328(int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin)
    : _wire(&Wire), _sda(sda_pin), _scl(scl_pin), _rst(rst_pin), _int(int_pin), _addr(ADDR_1A) {}

CST328::CST328(TwoWire* wire, int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin)
    : _wire(wire ? wire : &Wire), _sda(sda_pin), _scl(scl_pin), _rst(rst_pin), _int(int_pin), _addr(ADDR_1A) {}

void CST328::recoverBus(void) {
    _wire->end();
    delay(10);
    if (_sda != -1 && _scl != -1)
        _wire->begin(_sda, _scl);
    else
        _wire->begin();
    _wire->setClock(100000);  // 100 kHz after recovery; driver_ng less likely to stick in 259
}

void CST328::begin(void) {
    if (_wire == nullptr) _wire = &Wire;
    if (_int != -1)
        pinMode(_int, INPUT);

    /* Reset: same as demo touch_cst328_reset — 10ms low, 10ms high */
    if (_rst != -1) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, LOW);
        delay(10);
        digitalWrite(_rst, HIGH);
        delay(10);
    }

    /* I2C: Wire1 (1,3), 100 kHz — driver_ng often hits 259 at 400k; 100k is more reliable */
    if (_sda != -1 && _scl != -1)
        _wire->begin(_sda, _scl);
    else
        _wire->begin();
    _wire->setClock(100000);
    _addr = ADDR_1A;

    /* Demo: write NORMAL_MODE to enter touch mode */
    i2c_write_reg16(CST328_REG_NORMAL_MODE, nullptr, 0);
}

bool CST328::i2c_read_reg16(uint16_t reg, uint8_t* data, uint8_t len) {
    for (int attempt = 0; attempt < 2; attempt++) {
        _wire->beginTransmission(_addr);
        _wire->write((uint8_t)(reg >> 8));
        _wire->write((uint8_t)(reg & 0xFF));
        // endTransmission(true) = full STOP so requestFrom() is a separate read transaction.
        // Avoids i2c_master_transmit_receive / i2cWriteReadNonStop which was NACKing (259).
        if (_wire->endTransmission(true) != 0) {
            if (attempt == 0) { recoverBus(); continue; }
#ifdef CST328_DEBUG
            Serial.printf("[CST328] read reg 0x%04X endTransmission failed\n", reg);
#endif
            return false;
        }
        delayMicroseconds(200);  // let controller prepare response
        if (_wire->requestFrom((uint8_t)_addr, len) != len) {
            if (attempt == 0) { recoverBus(); continue; }
#ifdef CST328_DEBUG
            Serial.printf("[CST328] read reg 0x%04X requestFrom failed\n", reg);
#endif
            return false;
        }
        for (uint8_t i = 0; i < len; i++)
            data[i] = _wire->read();
        return true;
    }
    return false;
}

bool CST328::i2c_write_reg16(uint16_t reg, const uint8_t* data, uint8_t len) {
    for (int attempt = 0; attempt < 2; attempt++) {
        _wire->beginTransmission(_addr);
        _wire->write((uint8_t)(reg >> 8));
        _wire->write((uint8_t)(reg & 0xFF));
        if (data)
            for (uint8_t i = 0; i < len; i++)
                _wire->write(data[i]);
        if (_wire->endTransmission() != 0) {
            if (attempt == 0) { recoverBus(); continue; }
#ifdef CST328_DEBUG
            Serial.printf("[CST328] write reg 0x%04X endTransmission failed\n", reg);
#endif
            return false;
        }
        return true;
    }
    return false;
}

bool CST328::getTouch(uint16_t* x, uint16_t* y, uint8_t* gesture) {
    uint8_t buf[28];
    *x = *y = 0;
    *gesture = 0;

    if (!i2c_read_reg16(CST328_REG_READ_NUMBER, buf, 1))
        return false;

    uint8_t touch_cnt = buf[0] & 0x0F;
    if (touch_cnt == 0)
        return false;
    if (touch_cnt > 5) {
        uint8_t clear = 0;
        i2c_write_reg16(CST328_REG_READ_NUMBER, &clear, 1);
        return false;
    }

    if (!i2c_read_reg16(CST328_REG_READ_XY, &buf[1], 27))
        return false;

    *x = (uint16_t)(((uint16_t)buf[2] << 4) + ((buf[4] & 0xF0) >> 4));
    *y = (uint16_t)(((uint16_t)buf[3] << 4) + (buf[4] & 0x0F));

    uint8_t clear = 0;
    i2c_write_reg16(CST328_REG_READ_NUMBER, &clear, 1);
    return true;
}
