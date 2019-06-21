#include <Wire.h>

/*---- Device and register addresses -----------*/

#define AMG8833_DEV_ADDR 0x68
#define AMG8833_T01L_ADDR 0x80
#define AMG8833_TTHL_ADDR 0x0E
#define AMG8833_1F_ADDR 0x1F
#define AMG8833_AVE_ADDR 0x07
#define AMG8833_THERMISTOR_LENGTH 2U
#define AMG8833_PIXELS_LENGTH 128U
#define AMG8833_PIXELS_LENGTH_HALF 64U

// This is due to the buffer size of Wire lib.
#define AMG8833_READ_REPEAT (AMG8833_PIXELS_LENGTH/32)

/*---- UART frame ------------------------------*/
const uint8_t BEGIN = 0xFE;
const uint8_t END = 0xFF;

/*---- I2C functions ---------------------------*/

void i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
  Wire.beginTransmission(dev_addr);
  Wire.write(reg_addr);
  Wire.endTransmission(false);
  Wire.requestFrom((int)dev_addr, (int)len, true);
  for (int i = 0; i < len; i++) {
    data[i] = (uint8_t)(Wire.read());
  }
}

void i2c_write(uint8_t dev_addr, uint8_t *data, uint8_t len) {
  Wire.beginTransmission(dev_addr);
  for (int i = 0; i < len; i++) {
    Wire.write(data[i]);
  }
  Wire.endTransmission(true);
  delay(100);  // TODO: polling
}

/*---- Utility functions ---------------------------*/

// Frame buffer
uint8_t pixels[AMG8833_PIXELS_LENGTH];

/**
   Enable/disable moving average
*/
void set_moving_average(bool enable) {

  uint8_t enable_sequence[5][2] = {
    {AMG8833_1F_ADDR, 0x50},
    {AMG8833_1F_ADDR, 0x45},
    {AMG8833_1F_ADDR, 0x57},
    {AMG8833_AVE_ADDR, 0x20},
    {AMG8833_1F_ADDR, 0x00}
  };

  uint8_t disable_sequence[5][2] = {
    {AMG8833_1F_ADDR, 0x50},
    {AMG8833_1F_ADDR, 0x45},
    {AMG8833_1F_ADDR, 0x57},
    {AMG8833_AVE_ADDR, 0x00},
    {AMG8833_1F_ADDR, 0x00}
  };

  if (enable) {
    for (int i = 0; i < 5; i++) {
      i2c_write(AMG8833_DEV_ADDR, enable_sequence[i], 2);
    }
  } else {
    for (int i = 0; i < 5; i++) {
      i2c_write(AMG8833_DEV_ADDR, disable_sequence[i], 2);
    }
  }
}

/**
   Update the frame buffer
*/
void update_pixels(void) {

  for (int i = 0; i < AMG8833_READ_REPEAT; i++) {
    i2c_read(AMG8833_DEV_ADDR, AMG8833_T01L_ADDR + i * 32, pixels + i * 32, 32);
  }

  for (int i = 0; i < AMG8833_PIXELS_LENGTH_HALF; i++) {
    pixels[i] = pixels[i * 2]; // Ignore MSB of a pair of [LSB, MSB]
  }

}

/*------ Main routine ---------------------------*/

void setup() {

  Wire.begin();  // I2C bus
  Wire.setClock(400000);  // SCK: 400kHz

  // Begin serial communcation
  Serial.begin(115200);

  set_moving_average(true);

}

//#define CHAR_FORMAT

void loop() {

  update_pixels();

#ifdef CHAR_FORMAT
  for (int i = 0; i < AMG8833_PIXELS_LENGTH_HALF; i++) {
    Serial.print(pixels[i]);
    Serial.print(',');
  }
  delay(3000);
#else
  Serial.write(BEGIN);  
  Serial.write(pixels, AMG8833_PIXELS_LENGTH_HALF);
  Serial.write(END);
  delay(100);
#endif
  
}
