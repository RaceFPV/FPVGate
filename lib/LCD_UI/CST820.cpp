#include "CST820.h"

CST820::CST820(int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin)
    : _wire(&Wire), _sda(sda_pin), _scl(scl_pin), _rst(rst_pin), _int(int_pin) {}

CST820::CST820(TwoWire* wire, int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin)
    : _wire(wire ? wire : &Wire), _sda(sda_pin), _scl(scl_pin), _rst(rst_pin), _int(int_pin) {}

void CST820::begin(void)
{
    if (_wire == nullptr) _wire = &Wire;
    // Initialize I2C
    if (_sda != -1 && _scl != -1)
        _wire->begin(_sda, _scl);
    else
        _wire->begin();
    _wire->setClock(100000);  // 100 kHz for stability (CST328 can do up to 1 MHz)

    // Int Pin: input from touch IC (interrupt/data-ready); do not drive it
    if (_int != -1)
        pinMode(_int, INPUT);

    // Reset Pin Configuration
    if (_rst != -1)
    {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, LOW);
        delay(10);
        digitalWrite(_rst, HIGH);
        delay(300);
    }

    // Disable auto low-power mode (CST820; CST328 may NACK - ignore)
    _wire->beginTransmission(I2C_ADDR_CST820);
    _wire->write(0xFE);
    _wire->write(0xFF);
    (void)_wire->endTransmission();
}

bool CST820::getTouch(uint16_t *x, uint16_t *y, uint8_t *gesture)
{
    uint8_t data[4];
    if (i2c_read_continuous(0x03, data, 4) != 0) {
        *x = *y = 0;
        *gesture = None;
        return false;  // I2C failed (e.g. NACK) - no touch
    }
    bool FingerIndex = (bool)i2c_read(0x02);
    *gesture = i2c_read(0x01);
    if (!(*gesture == SlideUp || *gesture == SlideDown))
    {
        *gesture = None;
    }
    *x = ((data[0] & 0x0f) << 8) | data[1];
    *y = ((data[2] & 0x0f) << 8) | data[3];
    return FingerIndex;
}

uint8_t CST820::i2c_read(uint8_t addr)
{
    uint8_t rdData = 0;
    const int maxRetries = 3;
    for (int retry = 0; retry < maxRetries; retry++) {
        _wire->beginTransmission(I2C_ADDR_CST820);
        _wire->write(addr);
        if (_wire->endTransmission(false) != 0) continue;
        if (_wire->requestFrom(I2C_ADDR_CST820, (uint8_t)1) == 1) {
            if (_wire->available()) rdData = _wire->read();
            return rdData;
        }
    }
    return rdData;  // failure: return 0 and avoid endless loop / I2C spam
}

uint8_t CST820::i2c_read_continuous(uint8_t addr, uint8_t *data, uint32_t length)
{
    _wire->beginTransmission(I2C_ADDR_CST820);
    _wire->write(addr);
    if (_wire->endTransmission(true)) return -1;
    if (_wire->requestFrom(I2C_ADDR_CST820, length) != length) return -1;
    for (uint32_t i = 0; i < length; i++)
        *data++ = _wire->read();
    return 0;
}

void CST820::i2c_write(uint8_t addr, uint8_t data)
{
    _wire->beginTransmission(I2C_ADDR_CST820);
    _wire->write(addr);
    _wire->write(data);
    _wire->endTransmission();
}

uint8_t CST820::i2c_write_continuous(uint8_t addr, const uint8_t *data, uint32_t length)
{
    _wire->beginTransmission(I2C_ADDR_CST820);
    _wire->write(addr);
    for (uint32_t i = 0; i < length; i++)
        _wire->write(*data++);
    if (_wire->endTransmission(true)) return -1;
    return 0;
}
