// --------------------------------------------------------------------------
// COPYRIGHT (c) Freescale 2009
// Freescale Confidential Proprietary
//  All Rights Reserved
// --------------------------------------------------------------------------
//!\file    user_module.h
//!\brief   This file contains the defines giving the number of test cases, use of ThirdEye, SW interrupts etc.
//


#ifndef  __USER_MODULE_H_
#define  __USER_MODULE_H_

//#define EN_I2C_Reg_Rst_Chk_test
//#define EN_I2C_Reg_RW_test
//#define EN_I2C_Reg_RW_User_Mode_test
#define EN_I2C_Mstr_Tx_test
//#define EN_I2C_Mstr_Rx_test
//#define EN_I2C_Rpt_Strt_test
//#define EN_I2C_Data_No_Ack_test
//#define EN_I2C_More_Slave_Rx_test
//#define EN_I2C_More_Slave_Tx_test
//#define EN_I2C_Master_Interrupt_test
//#define EN_I2C_Slave_Interrupt_test
//#define EN_I2C_DMA_Mstr_Tx_test
//#define EN_I2C_DMA_Mstr_Rx_test
//#define EN_I2C_External_Memory_Access_test
//#define EN_I2C_Stress_test
//#define EN_I2C_Enable_Disable_test
//#define EN_I2C_XRDC_test
#define EN_I2C_Mstr_Tx_freq_test
//#define EN_I2C_Mstr_Rx_freq_test
//#define EN_I2C_Dbg_Mode_Before_Add_Tx_test
//#define EN_I2C_Debug_Mode_During_Trans_test
#define EN_I2C_DMA_Debug_Mode_test

//#define EN_I2C_Stop_Mode_test
//#define EN_I2C_Halt_Mode_test
//#define EN_I2C_Multi_Core_test
//#define EN_I2C_Mstr_Tx_With_DFS_test
//#define EN_I2C_Pin_Mux_test
//#define EN_I2C_Byte_Rx_Int_Enable_test
//#define EN_I2C_short_func_reset_test

void I2C_find_slave_test(void);
void I2C_Reg_Rst_Chk_test(void);
void I2C_Reg_RW_test(void);
void I2C_Reg_RW_User_Mode_test();
void I2C_Mstr_Tx_test(void);
void I2C_Mstr_Rx_test(void);
void I2C_Rpt_Strt_test(void);
void I2C_Data_No_Ack_test(void);
void I2C_More_Slave_Rx_test1();
void I2C_More_Slave_Rx_test2();
void I2C_More_Slave_Tx_test1();
void I2C_More_Slave_Tx_test2();
void I2C_Master_Interrupt_test1();
void I2C_Master_Interrupt_test2();
void I2C_More_Slave_Rx_test();
void I2C_More_Slave_Tx_test();
void I2C_Master_Interrupt_test();
void I2C_Slave_Interrupt_test();
void I2C_DMA_Mstr_Tx_test(void);
void I2C_DMA_Mstr_Rx_test(void);
void I2C_Mstr_Tx_freq_test();
void I2C_Mstr_Rx_freq_test(void);
void I2C_Dbg_Mode_Before_Add_Tx_Master_test();
void I2C_Dbg_Mode_Before_Add_Tx_Slave_test();
void I2C_Debug_Mode_During_Trans_Master_test();
void I2C_Debug_Mode_During_Trans_Slave_test();
void I2C_DMA_Debug_Mode_test(void);
void I2C_Stop_Mode_Master_test();
void I2C_Stop_Mode_Slave_test();
void I2C_Halt_Mode_Master_test();
void I2C_Halt_Mode_Slave_test();
void I2C_Enable_Disable_test();
void I2C_Multi_Core_test();
void I2C_External_Memory_Access_test(void);
void I2C_Mstr_Tx_With_DFS_test();
void I2C_Pin_Mux_test();
void I2C_Stress_test(void);
void I2C_Byte_Rx_Int_Enable_test();
void I2C_XRDC_test();
void I2C_short_func_reset_test();

#endif
