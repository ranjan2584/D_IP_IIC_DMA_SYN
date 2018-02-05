#ifndef I2C_API_H_
#define I2C_API_H_

void i2c_init (i2c_type,uint8_t,uint8_t,uint8_t);
uint8_t i2c_transfer(uint8_t mode,uint8_t master_num,uint8_t slave_address_with_r_w);
uint16_t i2c_find_slave_add(uint8_t master_num);
void i2c_pin_muxing(uint8_t device_num);
void i2c_reg_reset(uint8_t device_num);
void i2c_reset_variables(uint8_t device_num);
void i2c0_isr();
void i2c1_isr();
void i2c2_isr();
void i2c3_isr();

#endif
