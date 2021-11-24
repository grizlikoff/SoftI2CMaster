// Sketch to explore the luminosity sensor TSL2561 (breakout board by Adafruit)

#define INTTIME TSL2561_TIME_402MS
#define GAIN false

#ifdef __AVR_ATmega328P__
#define SDA_PORT PORTC
#define SDA_PIN 4
#define SCL_PORT PORTC
#define SCL_PIN 5
#define I2C_FASTMODE 1
#else
#define SDA_PORT PORTB
#define SDA_PIN 0
#define SCL_PORT PORTB
#define SCL_PIN 2
#define I2C_FASTMODE 1
#endif

#include <SoftI2CMaster.hpp>
#include "TSL2561Soft.h"

#define ADDR 0x72

//------------------------------------------------------------------------------
unsigned long computeLux(boolean gain, int intTime , unsigned long channel0, unsigned long channel1){
  
  uint16_t clipThreshold;
  unsigned long chScale;
  
  switch (intTime) {
  case TSL2561_TIME_13MS: 
    clipThreshold = TSL2561_CLIPPING_13MS;
    chScale =  TSL2561_LUX_CHSCALE_TINT0;
    break;
  case TSL2561_TIME_101MS: 
    clipThreshold = TSL2561_CLIPPING_101MS;
    chScale = TSL2561_LUX_CHSCALE_TINT1;
    break;
  case TSL2561_TIME_402MS: 
    clipThreshold = TSL2561_CLIPPING_402MS;
    chScale = (1 << TSL2561_LUX_CHSCALE);
    break;
  }
  if (!gain) chScale = chScale << 4;

  /* Return MAX lux if the sensor is saturated */
  if ((channel0 > clipThreshold) || (channel1 > clipThreshold))
  {
#ifdef __AVR_ATmega328P__
    Serial.println(F("Sensor is saturated"));
#endif
    return 32000;
  }

  channel0 = (channel0 * chScale) >> TSL2561_LUX_CHSCALE;
  channel1 = (channel1 * chScale) >> TSL2561_LUX_CHSCALE;
  

  /* Find the ratio of the channel values (Channel1/Channel0) */
  unsigned long ratio1 = 0;
  if (channel0 != 0) ratio1 = (channel1 << (TSL2561_LUX_RATIOSCALE+1)) / channel0;

  /* round the ratio value */
  unsigned long ratio = (ratio1 + 1) >> 1;

  unsigned int b, m;

  if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1T))
    {b=TSL2561_LUX_B1T; m=TSL2561_LUX_M1T;}
  else if (ratio <= TSL2561_LUX_K2T)
    {b=TSL2561_LUX_B2T; m=TSL2561_LUX_M2T;}
  else if (ratio <= TSL2561_LUX_K3T)
    {b=TSL2561_LUX_B3T; m=TSL2561_LUX_M3T;}
  else if (ratio <= TSL2561_LUX_K4T)
    {b=TSL2561_LUX_B4T; m=TSL2561_LUX_M4T;}
  else if (ratio <= TSL2561_LUX_K5T)
    {b=TSL2561_LUX_B5T; m=TSL2561_LUX_M5T;}
  else if (ratio <= TSL2561_LUX_K6T)
    {b=TSL2561_LUX_B6T; m=TSL2561_LUX_M6T;}
  else if (ratio <= TSL2561_LUX_K7T)
    {b=TSL2561_LUX_B7T; m=TSL2561_LUX_M7T;}
  else if (ratio > TSL2561_LUX_K8T)
    {b=TSL2561_LUX_B8T; m=TSL2561_LUX_M8T;}

  unsigned long temp;
  temp = ((channel0 * b) - (channel1 * m));

  /* Do not allow negative lux value */
  if (temp < 0) temp = 0;

  /* Round lsb (2^(LUX_SCALE-1)) */
  temp += (1 << (TSL2561_LUX_LUXSCALE-1));

  /* Strip off fractional portion */
  unsigned long lux = temp >> TSL2561_LUX_LUXSCALE;

  return lux;
}

void setup(void) {

#ifdef __AVR_ATmega328P__
  Serial.begin(19200);
  Serial.println("Initializing ...");
#endif
  i2c_init();

  if (!i2c_start(ADDR | I2C_WRITE)) {
#ifdef __AVR_ATmega328P__
    Serial.println(F("Device does not respond"));
#endif
  }
  if (!i2c_write(0x80)) {
#ifdef __AVR_ATmega328P__
    Serial.println(F("Cannot address reg 0"));
#endif
  }
  if (!i2c_write(0x03)) {
#ifdef __AVR_ATmega328P__
    Serial.println(F("Cannot wake up")); // wake up
#endif
  }
  i2c_stop();
  i2c_start(ADDR | I2C_WRITE);
  if (!i2c_write(0x81)) {
#ifdef __AVR_ATmega328P__
    Serial.println(F("Cannot address reg 1"));
#endif
  }
  if (!i2c_write((GAIN ? 0x10 : 0x00)+INTTIME)) {
#ifdef __AVR_ATmega328P__
      Serial.println(F("Cannot change gain & integration time")); 
#endif
  }
  i2c_stop();
  
}  

void loop (void) {
  unsigned int low0, high0, low1, high1;
  unsigned int chan0, chan1;
  unsigned int lux;

  if (!i2c_start(ADDR | I2C_WRITE)) {
#ifdef __AVR_ATmega328P__
    Serial.println(F("Device does not respond"));
#endif
  }
  if (!i2c_write(0x80)) {
#ifdef __AVR_ATmega328P__
    Serial.println(F("Cannot address reg 0"));
#endif
  }
  if (!i2c_write(0x03)) {
#ifdef __AVR_ATmega328P__
    Serial.println(F("Cannot wake up"));
#endif
  }
  i2c_stop();
  delay(600);
  i2c_start(ADDR | I2C_WRITE);
  i2c_write(0x8C);
  i2c_rep_start(ADDR | I2C_READ);
  low0 = i2c_read(false);
  high0 = i2c_read(true);
  i2c_stop();
  i2c_start(ADDR | I2C_WRITE);
  i2c_write(0x8E);
  i2c_rep_start(ADDR | I2C_READ);
  low1 = i2c_read(false);
  high1 = i2c_read(true);
  i2c_stop();
  i2c_start(ADDR | I2C_WRITE);
  if (!i2c_write(0x80)) {
#ifdef __AVR_ATmega328P__
    Serial.println(F("Cannot address reg 0"));
#endif
  }
  if (!i2c_write(0x00)) {
#ifdef __AVR_ATmega328P__
    Serial.println(F("Cannot wake up"));
#endif
  }
  i2c_stop();
#ifdef __AVR_ATmega328P__
  Serial.print(F("Raw values: chan0="));
  Serial.print(chan0=(low0+(high0<<8)));
  Serial.print(F(" / chan1="));
  Serial.println(chan1=(low1+(high1<<8)));
  lux = computeLux(GAIN,INTTIME,chan0,chan1);
  Serial.print(F("Lux value="));
  Serial.println(lux);
#else
  while (1) ;
#endif
  delay(1000);
}
