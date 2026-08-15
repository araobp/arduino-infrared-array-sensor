#include <Wire.h>

/*---- Device and register addresses -----------*/
#define AMG8833_DEV_ADDR           0x68
#define AMG8833_RESET_ADDR         0x01
#define AMG8833_FPSC_ADDR          0x02
#define AMG8833_AVE_ADDR           0x07
#define AMG8833_1F_ADDR            0x1F
#define AMG8833_T01L_ADDR          0x80

#define AMG8833_PIXELS_LENGTH      128U
#define AMG8833_PIXELS_LENGTH_HALF 64U
#define AMG8833_READ_REPEAT        (AMG8833_PIXELS_LENGTH / 32)

/*---- UART frame ------------------------------*/
const uint8_t BEGIN = 0xFE;
const uint8_t END   = 0xFF;

/*---- Frame timing ----------------------------*/
const unsigned long FRAME_INTERVAL_MS = 100; // 10 fps (100ms)
unsigned long lastFrameTime = 0;

// Frame buffer
uint8_t raw_pixels[AMG8833_PIXELS_LENGTH];
uint8_t pixels[AMG8833_PIXELS_LENGTH_HALF];

/*---- I2C helper functions --------------------*/

bool i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
  Wire.beginTransmission(dev_addr);
  Wire.write(reg_addr);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  uint8_t read_bytes = Wire.requestFrom((int)dev_addr, (int)len, true);
  if (read_bytes != len) {
    return false;
  }

  for (uint8_t i = 0; i < len; i++) {
    data[i] = (uint8_t)Wire.read();
  }
  return true;
}

bool i2c_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t val) {
  Wire.beginTransmission(dev_addr);
  Wire.write(reg_addr);
  Wire.write(val);
  return (Wire.endTransmission(true) == 0);
}

/*---- Utility functions ---------------------------*/

/**
   Enable/disable moving average
*/
void set_moving_average(bool enable) {
  i2c_write_reg(AMG8833_DEV_ADDR, AMG8833_1F_ADDR, 0x50);
  i2c_write_reg(AMG8833_DEV_ADDR, AMG8833_1F_ADDR, 0x45);
  i2c_write_reg(AMG8833_DEV_ADDR, AMG8833_1F_ADDR, 0x57);
  i2c_write_reg(AMG8833_DEV_ADDR, AMG8833_AVE_ADDR, enable ? 0x20 : 0x00);
  i2c_write_reg(AMG8833_DEV_ADDR, AMG8833_1F_ADDR, 0x00);
}

/**
   Update the frame buffer
*/
bool update_pixels(void) {
  for (uint8_t i = 0; i < AMG8833_READ_REPEAT; i++) {
    if (!i2c_read(AMG8833_DEV_ADDR, AMG8833_T01L_ADDR + i * 32, raw_pixels + i * 32, 32)) {
      return false; // I2C read error
    }
  }

  for (uint8_t i = 0; i < AMG8833_PIXELS_LENGTH_HALF; i++) {
    uint8_t val = raw_pixels[i * 2]; // Ignore MSB of a pair of [LSB, MSB]
    // Clamp at 0xFD so it does not collide with BEGIN (0xFE) or END (0xFF)
    if (val >= BEGIN) {
      val = 0xFD;
    }
    pixels[i] = val;
  }
  return true;
}

/*------ Main routine ---------------------------*/

void setup() {
  Wire.begin();           // I2C bus
  Wire.setClock(400000);  // SCK: 400kHz

  // Begin serial communication
  Serial.begin(115200);

  // Initial reset & 10fps setting
  i2c_write_reg(AMG8833_DEV_ADDR, AMG8833_RESET_ADDR, 0x3F);
  delay(50);
  i2c_write_reg(AMG8833_DEV_ADDR, AMG8833_FPSC_ADDR, 0x00); // 10 FPS mode

  set_moving_average(true);
}

//#define CHAR_FORMAT

void loop() {
  unsigned long now = millis();

  // Send frame at 10 fps (100ms interval)
  if (now - lastFrameTime >= FRAME_INTERVAL_MS) {
    lastFrameTime = now;

    if (update_pixels()) {
#ifdef CHAR_FORMAT
      for (int i = 0; i < AMG8833_PIXELS_LENGTH_HALF; i++) {
        Serial.print(pixels[i]);
        Serial.print(',');
      }
      Serial.println();
#else
      Serial.write(BEGIN);
      Serial.write(pixels, AMG8833_PIXELS_LENGTH_HALF);
      Serial.write(END);
#endif
    }
  }
}
