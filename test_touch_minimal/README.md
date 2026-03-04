# Minimal touch test (CST328 on Wire1)

Isolates touch I2C: only Wire1 (SDA=1, SCL=3) and CST328 at 0x1A. No display, no LVGL.

**Run from this folder:**
```bash
cd test_touch_minimal
pio run -e touch_minimal
pio run -e touch_minimal -t upload
pio device monitor -b 921600
```

**Interpretation:**
- **259/NACK appears here too** → Issue is Wire1 + driver_ng + CST328 on this board (bus/speed/timing).
- **Touch stable here, fails in full FPVGate** → Issue is interaction with LVGL, display, or other tasks (contention or rate).

After the test you can try **50 kHz** in the main CST328 driver (`setClock(50000)`) to see if lower speed helps.
