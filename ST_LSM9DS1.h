/* 
 * File:   ST_LSM9DS1.h
 * Author: dieterv
 *
 * Created on March 17, 2020, 2:16 PM
 */


/* Datasheet abbreviations:
 G: gyro
 XL: accelerometer
 THS: threshold
 */

/* Enable gyro:
 CTRL_REG1_G (0x10) set to 0b00100000
  Sets ODR(Output Data Rate) to 14.9Hz, no Cutoff
 CTRL_REG3_G (0x12) to 0b10000000
  Sets LP mode, HPF disabled
 * Disable gyro:
 CTRL_REG1_G (0x10) set to 0b00000000 
 
 * Enable ACC:
 CTRL_REG6_XL (0x20) set to 0b00100000
  sets ODR to 10Hz, FS +/-2G, BW by ODR (408Hz at 10Hz)
 
 * Enable Mag:
 set CTRL_REG1_M (0x20) to 0b10000000
  sets Temp_comp on, low power mode, ODR 0,625Hz, Self test off
 Set CTRL_REG3_M (0x22) to 0b 00100001
  sets I2C enable, Low power (0.625Hz), SPI write only, Single conversion mode

 * Disable FIFO (direct read from buffer)
 set FIFO_CTRL (0x2E) to 0b11000000
 sets to continuous mode
 
 * Read data from:
 OUT_X_G (0x18-0x19): gyro X
 OUT_X_G (0x1A-0x1B): gyro Y
 OUT_X_G (0x1C-0x1D): gyro Z
 
 OUT_X_XL (0x28-0x29): Acc X
 OUT_Y_XL (0x2A-0x2B): ACC Y
 OUT_Z_XL (0x2C-0x2D): ACC Z
 
 OUT_TEMP (0x15-0x16): TEMP
 * 
 
 */
#ifndef ST_LSM9DS1_H
#define	ST_LSM9DS1_H

int8_t LSM9_InitReset (void);
int8_t LSM9_GetTemp (int16_t * pTempRaw);



#endif	/* ST_LSM9DS1_H */

