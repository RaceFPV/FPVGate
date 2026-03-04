/**
 * Waveshare Arduino demo touch driver for CST328 (2.8").
 * For WAVESHARE_ESP32S3_LCD28: uses ESP-IDF I2C master (no Arduino Wire) to avoid driver_ng 259/NACK.
 * Otherwise: uses Wire1, interrupt-driven read, and demo init/reset sequence.
 */
#include "Touch_CST328.h"
#include <cassert>
#include <cstring>

#if defined(WAVESHARE_ESP32S3_LCD28)
#include "esp_idf_version.h"
#if ESP_IDF_VERSION_MAJOR >= 5
#include "driver/i2c_master.h"
#include "esp_err.h"
#define CST328_USE_IDF_I2C 1
static i2c_master_bus_handle_t s_cst328_bus = NULL;
static i2c_master_dev_handle_t s_cst328_dev = NULL;
#define CST328_IDF_TIMEOUT_MS 100
static const i2c_device_config_t s_cst328_dev_cfg = {
  .dev_addr_length = I2C_ADDR_BIT_LEN_7,
  .device_address = CST328_ADDR,
  .scl_speed_hz = I2C_MASTER_FREQ_HZ,
};
static const i2c_master_bus_config_t s_cst328_bus_cfg = {
  .i2c_port = I2C_NUM_1,
  .sda_io_num = (gpio_num_t)CST328_SDA_PIN,
  .scl_io_num = (gpio_num_t)CST328_SCL_PIN,
  .clk_source = I2C_CLK_SRC_DEFAULT,
  .glitch_ignore_cnt = 7,
  .intr_priority = 0,
  .trans_queue_depth = 0,
  .flags = { .enable_internal_pullup = true },
};
// After NACK the CST328 may stop responding; reset it and recreate bus for next poll.
static void recoverIdfBusForNextPoll(void) {
  if (s_cst328_dev) {
    i2c_master_bus_rm_device(s_cst328_dev);
    s_cst328_dev = NULL;
  }
  if (s_cst328_bus) {
    i2c_del_master_bus(s_cst328_bus);
    s_cst328_bus = NULL;
  }
  delay(25);
  CST328_Touch_Reset();  // hardware reset so CST328 responds again
  delay(60);             // let chip come up after reset
  if (i2c_new_master_bus(&s_cst328_bus_cfg, &s_cst328_bus) != ESP_OK) {
#if defined(CST328_DEBUG)
    printf("[CST328] IDF recover: new bus failed\n");
#endif
    return;
  }
  if (i2c_master_bus_add_device(s_cst328_bus, &s_cst328_dev_cfg, &s_cst328_dev) != ESP_OK) {
#if defined(CST328_DEBUG)
    printf("[CST328] IDF recover: add device failed\n");
#endif
    i2c_del_master_bus(s_cst328_bus);
    s_cst328_bus = NULL;
    return;
  }
#if defined(CST328_DEBUG)
  printf("[CST328] IDF recover bus+device for next poll\n");
#endif
}
#endif
#endif
#if !defined(CST328_USE_IDF_I2C)
#include <Wire.h>
#endif

struct CST328_Touch touch_data = {0};

#if !defined(CST328_USE_IDF_I2C)
// Re-init Wire1 for the *next* poll only (never retry in same transaction — that caused 259).
static void recoverBusForNextPoll(void) {
  Wire1.end();
  delay(50);
  Wire1.begin(CST328_SDA_PIN, CST328_SCL_PIN, I2C_MASTER_FREQ_HZ);
#if defined(CST328_DEBUG)
  printf("[CST328] recoverBus for next poll\n");
#endif
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I2C Read/Write — ESP-IDF path for 2.8", Wire1 path otherwise.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Touch_I2C_Read(uint8_t Driver_addr, uint16_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
#if defined(CST328_USE_IDF_I2C)
  if (!s_cst328_dev) return false;
  uint8_t reg_buf[2] = { (uint8_t)(Reg_addr >> 8), (uint8_t)(Reg_addr & 0xFF) };
  esp_err_t err = i2c_master_transmit_receive(s_cst328_dev, reg_buf, 2, Reg_data, Length, CST328_IDF_TIMEOUT_MS);
#if defined(CST328_DEBUG)
  if (err != ESP_OK) {
    printf("[CST328] IDF Read reg=0x%04X err=0x%x (%s)\n", (unsigned)Reg_addr, (unsigned)err, esp_err_to_name(err));
  }
#endif
  if (err != ESP_OK) {
    recoverIdfBusForNextPoll();
    return false;
  }
  return true;
#else
  Wire1.beginTransmission(Driver_addr);
  Wire1.write((uint8_t)(Reg_addr >> 8));
  Wire1.write((uint8_t)Reg_addr);
  int et = Wire1.endTransmission(true);
#if defined(CST328_DEBUG)
  if (et != 0) {
    printf("[CST328] Read reg=0x%04X endTransmission=%d\n", (unsigned)Reg_addr, et);
  }
#endif
  if (et != 0) {
    recoverBusForNextPoll();
    return false;
  }
  size_t got = Wire1.requestFrom(Driver_addr, Length);
#if defined(CST328_DEBUG)
  if (got != Length) {
    printf("[CST328] Read reg=0x%04X requestFrom wanted=%lu got=%u\n",
           (unsigned)Reg_addr, (unsigned long)Length, (unsigned)got);
  }
#endif
  if (got != Length) {
    recoverBusForNextPoll();
    return false;
  }
  for (uint32_t i = 0; i < Length; i++) {
    *Reg_data++ = Wire1.read();
  }
  return true;
#endif
}

bool Touch_I2C_Write(uint8_t Driver_addr, uint16_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
#if defined(CST328_USE_IDF_I2C)
  if (!s_cst328_dev) return false;
  uint8_t tx_buf[2 + 32];  // reg (2) + payload (max we use is 24)
  if (Length > sizeof(tx_buf) - 2) return false;
  tx_buf[0] = (uint8_t)(Reg_addr >> 8);
  tx_buf[1] = (uint8_t)(Reg_addr & 0xFF);
  if (Reg_data && Length)
    memcpy(tx_buf + 2, Reg_data, Length);
  uint32_t tx_len = 2 + Length;
  esp_err_t err = i2c_master_transmit(s_cst328_dev, tx_buf, tx_len, CST328_IDF_TIMEOUT_MS);
#if defined(CST328_DEBUG)
  if (err != ESP_OK) {
    printf("[CST328] IDF Write reg=0x%04X err=0x%x\n", (unsigned)Reg_addr, (unsigned)err);
  }
#endif
  return (err == ESP_OK);
#else
  Wire1.beginTransmission(Driver_addr);
  Wire1.write((uint8_t)(Reg_addr >> 8));
  Wire1.write((uint8_t)Reg_addr);
  if (Reg_data)
    for (uint32_t i = 0; i < Length; i++) {
      Wire1.write(Reg_data[i]);
    }
  int et = Wire1.endTransmission(true);
#if defined(CST328_DEBUG)
  if (et != 0) {
    printf("[CST328] Write reg=0x%04X endTransmission=%d\n", (unsigned)Reg_addr, et);
  }
#endif
  if (et != 0) return false;
  return true;
#endif
}

uint8_t Touch_Init(void) {
  pinMode(CST328_INT_PIN, INPUT);
  pinMode(CST328_RST_PIN, OUTPUT);

#if defined(CST328_USE_IDF_I2C)
#if defined(CST328_DEBUG)
  printf("[CST328] Touch_Init ESP-IDF I2C SDA=%d SCL=%d %u Hz\n",
         CST328_SDA_PIN, CST328_SCL_PIN, (unsigned)I2C_MASTER_FREQ_HZ);
#endif
  esp_err_t err = i2c_new_master_bus(&s_cst328_bus_cfg, &s_cst328_bus);
  if (err != ESP_OK) {
    printf("Touch_Init: i2c_new_master_bus failed 0x%x (%s)\n", (unsigned)err, esp_err_to_name(err));
    return false;
  }
  err = i2c_master_bus_add_device(s_cst328_bus, &s_cst328_dev_cfg, &s_cst328_dev);
  if (err != ESP_OK) {
    printf("Touch_Init: i2c_master_bus_add_device failed 0x%x (%s)\n", (unsigned)err, esp_err_to_name(err));
    i2c_del_master_bus(s_cst328_bus);
    s_cst328_bus = NULL;
    return false;
  }
#else
#if defined(CST328_DEBUG)
  printf("[CST328] Touch_Init start Wire1 SDA=%d SCL=%d %u Hz\n",
         CST328_SDA_PIN, CST328_SCL_PIN, (unsigned)I2C_MASTER_FREQ_HZ);
#endif
  Wire1.begin(CST328_SDA_PIN, CST328_SCL_PIN, I2C_MASTER_FREQ_HZ);
#endif

  CST328_Touch_Reset();
#if defined(CST328_DEBUG)
  printf("[CST328] Touch_Init after reset, reading cfg...\n");
#endif
  uint16_t Verification = CST328_Read_cfg();
  if(!((Verification==0xCACA)?true:false)) {
    printf("Touch initialization failed! (CACA=0x%04X)\r\n", (unsigned)Verification);
  }
#if defined(CST328_DEBUG)
  else {
    printf("[CST328] Touch_Init cfg OK CACA=0x%04X\n", (unsigned)Verification);
  }
#endif
  attachInterrupt(CST328_INT_PIN, Touch_CST328_ISR, interrupt);

  return ((Verification==0xCACA)?true:false);
}
/* Reset controller */
uint8_t CST328_Touch_Reset(void)
{
    digitalWrite(CST328_RST_PIN, HIGH );     // Reset
    delay(50);
    digitalWrite(CST328_RST_PIN, LOW);
    delay(5);
    digitalWrite(CST328_RST_PIN, HIGH );
    delay(50);
    return true;
}
uint16_t CST328_Read_cfg(void) {

  uint8_t buf[24];
  Touch_I2C_Write(CST328_ADDR, HYN_REG_MUT_DEBUG_INFO_MODE, buf, 0);
  Touch_I2C_Read(CST328_ADDR, HYN_REG_MUT_DEBUG_INFO_BOOT_TIME,buf, 4);
  printf("TouchPad_ID:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[0], buf[1], buf[2], buf[3]);
  Touch_I2C_Read(CST328_ADDR, HYN_REG_MUT_DEBUG_INFO_BOOT_TIME, buf, 4);
  printf("TouchPad_X_MAX:%d    TouchPad_Y_MAX:%d \r\n", buf[1]*256+buf[0],buf[3]*256+buf[2]);

  Touch_I2C_Read(CST328_ADDR, HYN_REG_MUT_DEBUG_INFO_TP_NTX, buf, 24);
  printf("D1F4:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[0], buf[1], buf[2], buf[3]);
  printf("D1F8:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[4], buf[5], buf[6], buf[7]);
  printf("D1FC:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[8], buf[9], buf[10], buf[11]);
  printf("D200:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[12], buf[13], buf[14], buf[15]);
  printf("D204:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[16], buf[17], buf[18], buf[19]);
  printf("D208:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[20], buf[21], buf[22], buf[23]);
  printf("CACA Read:0x%04x\r\n", (unsigned int)(((uint16_t)buf[11] << 8) | buf[10]));

  Touch_I2C_Write(CST328_ADDR, HYN_REG_MUT_NORMAL_MODE, buf, 0);
  return (((uint16_t)buf[11] << 8) | buf[10]);
}

// reads sensor and touches
// updates Touch Points, but if not touched, resets all Touch Point Information
// Returns true on success (I2C OK), false if any I2C read/write failed.
uint8_t Touch_Read_Data(void) {
  uint8_t buf[41];
  uint8_t touch_cnt = 0;
  uint8_t clear = 0;
  size_t i = 0,num=0;
  if (!Touch_I2C_Read(CST328_ADDR, ESP_LCD_TOUCH_CST328_READ_Number_REG, buf, 1)) {
#if defined(CST328_DEBUG)
    static uint32_t failCount;
    if (++failCount <= 5 || (failCount % 100 == 0)) {
      printf("[CST328] Touch_Read_Data I2C fail at READ_Number (total fails %lu)\n", (unsigned long)failCount);
    }
#endif
    return false;
  }
  if ((buf[0] & 0x0F) == 0x00) {
    Touch_I2C_Write(CST328_ADDR, ESP_LCD_TOUCH_CST328_READ_Number_REG, &clear, 1);  // No touch data
    return true;
  }
  /* Count of touched points */
  touch_cnt = buf[0] & 0x0F;
  if (touch_cnt > CST328_LCD_TOUCH_MAX_POINTS || touch_cnt == 0) {
    Touch_I2C_Write(CST328_ADDR, ESP_LCD_TOUCH_CST328_READ_Number_REG, &clear, 1);
    return true;
  }
  /* Read all points */
  if (!Touch_I2C_Read(CST328_ADDR, ESP_LCD_TOUCH_CST328_READ_XY_REG, &buf[1], 27)) {
#if defined(CST328_DEBUG)
    printf("[CST328] Touch_Read_Data I2C fail at READ_XY\n");
#endif
    return false;
  }
  /* Clear all */
  if (!Touch_I2C_Write(CST328_ADDR, ESP_LCD_TOUCH_CST328_READ_Number_REG, &clear, 1)) {
#if defined(CST328_DEBUG)
    printf("[CST328] Touch_Read_Data I2C fail at clear\n");
#endif
    return false;
  }
  noInterrupts();
  /* Number of points */
  if (touch_cnt > CST328_LCD_TOUCH_MAX_POINTS)
    touch_cnt = CST328_LCD_TOUCH_MAX_POINTS;
  touch_data.points = (uint8_t)touch_cnt;
  /* Fill all coordinates */
  for (i = 0; i < touch_cnt; i++) {
    if (i > 0) num = 2;
    touch_data.coords[i].x = (uint16_t)(((uint16_t)buf[(i * 5) + 2 + num] << 4) + ((buf[(i * 5) + 4 + num] & 0xF0) >> 4));
    touch_data.coords[i].y = (uint16_t)(((uint16_t)buf[(i * 5) + 3 + num] << 4) + (buf[(i * 5) + 4 + num] & 0x0F));
    touch_data.coords[i].strength = ((uint16_t)buf[(i * 5) + 5 + num]);
  }
  interrupts();
  return true;
}
void Touch_Loop(void){
  if(Touch_interrupts){
    Touch_interrupts = false;
    example_touchpad_read();
  }
}

uint8_t Touch_Get_XY(uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num) {
  assert(x != NULL);
  assert(y != NULL);
  assert(point_num != NULL);
  assert(max_point_num > 0);
  
  noInterrupts();
  /* Count of points */
  if(touch_data.points > max_point_num)
    touch_data.points = max_point_num;
  for (size_t i = 0; i < touch_data.points; i++) {
      x[i] = touch_data.coords[i].x;
      y[i] = touch_data.coords[i].y;
      if (strength) {
          strength[i] = touch_data.coords[i].strength;
      }
  }
  *point_num = touch_data.points;
  /* Invalidate */
  touch_data.points = 0;
  interrupts();
  return (*point_num > 0);
}
void example_touchpad_read(void){
  uint16_t touchpad_x[5] = {0};
  uint16_t touchpad_y[5] = {0};
  uint16_t strength[5]   = {0};
  uint8_t touchpad_cnt = 0;
  Touch_Read_Data();
  uint8_t touchpad_pressed = Touch_Get_XY(touchpad_x, touchpad_y, strength, &touchpad_cnt, CST328_LCD_TOUCH_MAX_POINTS);
  if (touchpad_pressed && touchpad_cnt > 0) {
      printf("Touch : X=%u Y=%u num=%d\r\n", (unsigned)touchpad_x[0], (unsigned)touchpad_y[0], (int)touchpad_cnt);
  }
}
/*!
    @brief  handle interrupts
*/
uint8_t Touch_interrupts;
void IRAM_ATTR Touch_CST328_ISR(void) {
  Touch_interrupts = true;
}
