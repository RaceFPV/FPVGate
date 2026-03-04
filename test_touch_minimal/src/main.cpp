/**
 * Minimal touch-only test: Wire1 (SDA=1, SCL=3) + CST328 at 0x1A.
 * No display, no LVGL, no other I2C. Poll getTouch every 100ms and print.
 * Use to isolate I2C 259/NACK: if it fails here, problem is Wire1+CST328+driver_ng;
 * if it's stable here, problem is elsewhere in FPVGate (LVGL, display, tasks).
 */
#include <Arduino.h>
#include <Wire.h>

static const uint8_t ADDR = 0x1A;
static const int SDA = 1, SCL = 3, RST = 2, INT_PIN = 4;
#define REG_READ_NUMBER  0xD005
#define REG_READ_XY      0xD000
#define REG_NORMAL_MODE  0xD109

static TwoWire* w = &Wire1;

static void recoverBus() {
    w->end();
    delay(10);
    w->begin(SDA, SCL);
    w->setClock(100000);
}

static bool i2c_read_reg16(uint16_t reg, uint8_t* data, uint8_t len) {
    for (int attempt = 0; attempt < 2; attempt++) {
        w->beginTransmission(ADDR);
        w->write((uint8_t)(reg >> 8));
        w->write((uint8_t)(reg & 0xFF));
        if (w->endTransmission(false) != 0) {
            if (attempt == 0) { recoverBus(); continue; }
            Serial.printf("[touch] read 0x%04X endTransmission fail\n", reg);
            return false;
        }
        delayMicroseconds(200);
        if (w->requestFrom(ADDR, len) != len) {
            if (attempt == 0) { recoverBus(); continue; }
            Serial.printf("[touch] read 0x%04X requestFrom fail\n", reg);
            return false;
        }
        for (uint8_t i = 0; i < len; i++) data[i] = w->read();
        return true;
    }
    return false;
}

static bool i2c_write_reg16(uint16_t reg, const uint8_t* data, uint8_t len) {
    for (int attempt = 0; attempt < 2; attempt++) {
        w->beginTransmission(ADDR);
        w->write((uint8_t)(reg >> 8));
        w->write((uint8_t)(reg & 0xFF));
        if (data) for (uint8_t i = 0; i < len; i++) w->write(data[i]);
        if (w->endTransmission() != 0) {
            if (attempt == 0) { recoverBus(); continue; }
            return false;
        }
        return true;
    }
    return false;
}

static bool getTouch(uint16_t* x, uint16_t* y) {
    uint8_t buf[28];
    *x = *y = 0;
    if (!i2c_read_reg16(REG_READ_NUMBER, buf, 1)) return false;
    uint8_t cnt = buf[0] & 0x0F;
    if (cnt == 0) return false;
    if (cnt > 5) {
        uint8_t clear = 0;
        i2c_write_reg16(REG_READ_NUMBER, &clear, 1);
        return false;
    }
    if (!i2c_read_reg16(REG_READ_XY, &buf[1], 27)) return false;
    *x = (uint16_t)(((uint16_t)buf[2] << 4) + ((buf[4] & 0xF0) >> 4));
    *y = (uint16_t)(((uint16_t)buf[3] << 4) + (buf[4] & 0x0F));
    uint8_t clear = 0;
    i2c_write_reg16(REG_READ_NUMBER, &clear, 1);
    return true;
}

void setup() {
    Serial.begin(921600);
    delay(2000);
    Serial.println("Touch minimal test: Wire1(1,3) CST328 0x1A, 100ms poll");

    if (RST >= 0) {
        pinMode(RST, OUTPUT);
        digitalWrite(RST, LOW);
        delay(10);
        digitalWrite(RST, HIGH);
        delay(10);
    }
    if (INT_PIN >= 0) pinMode(INT_PIN, INPUT);

    w->begin(SDA, SCL);
    w->setClock(100000);
    i2c_write_reg16(REG_NORMAL_MODE, nullptr, 0);

    Serial.println("Init done. Touch the screen; you should see x,y. Watch for I2C errors.");
}

void loop() {
    uint16_t x, y;
    if (getTouch(&x, &y))
        Serial.printf("touch %u %u\n", x, y);
    delay(100);
}
