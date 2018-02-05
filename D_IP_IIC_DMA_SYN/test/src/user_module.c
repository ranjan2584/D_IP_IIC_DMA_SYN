
// --------------------------------------------------------------------------
// COPYRIGHT (c) Freescale 2009
// Freescale Confidential Proprietary
//  All Rights Reserved
// --------------------------------------------------------------------------
//!\file    user_module.c
//!\brief   This file gives the array containing the test cases to be used by ThirdEye
//
// TYPE: C Source file File
// DEPARTMENT: MSG-IDC
// CREATED BY: Madhusudan Aggarwal  (B17104)
// --------------------------------------------------------------------------
// PURPOSE:  Test case array to be used by ThirdEye
// --------------------------------------------------------------------------
// $Log: user_module.c.rca $
// 
//  Revision: 1.1 Wed Apr  5 16:06:10 2017 B41624
//  *** empty comment string ***
// 
//  Revision: 1.8 Wed Feb  3 14:51:38 2016 B44571
//  *** empty comment string ***
// 
//  Revision: 1.7 Mon Jan 25 15:32:42 2016 B44571
//  *** empty comment string ***
// 
//  Revision: 1.6 Fri Aug 14 17:18:34 2015 B44571
//  *** empty comment string ***
// 


#include "user_module.h"
#include "common.h"
#include "thirdeye.h"
#include "uif.h"
#include "i2c_soc_config.h"

#define conc_test(x) #x "_" STR_CORE_ID

char MOD_NAME[]="D_IP_IIC_DMA_SYN";

UIF_TEST UIF_TEST_CASES[]={

#ifdef REGRESSION

    //{MOD_NAME,"I2C_find_slave_test","I2C find slave address",I2C_find_slave_test},
    
#ifdef EN_I2C_Reg_Rst_Chk_test
    {MOD_NAME,"I2C_Reg_Rst_Chk_test","I2C Register reset check",I2C_Reg_Rst_Chk_test},
#endif
    
#ifdef EN_I2C_Reg_RW_test
    {MOD_NAME,"I2C_Reg_RW_test","I2C Register Access",I2C_Reg_RW_test},
#endif
    
#ifdef EN_I2C_Mstr_Tx_test
    {MOD_NAME,"I2C_Mstr_Tx_test","I2C master_tx ",I2C_Mstr_Tx_test},
#endif
    
#ifdef EN_I2C_Mstr_Rx_test
    {MOD_NAME,"I2C_Mstr_Rx_test","I2C master_rx ",I2C_Mstr_Rx_test},
#endif
    
#ifdef EN_I2C_Rpt_Strt_test
    {MOD_NAME,"I2C_Rpt_Strt_test","checking I2C repeat start functionality",I2C_Rpt_Strt_test},
#endif
    
#ifdef EN_I2C_Data_No_Ack_test
    {MOD_NAME,"I2C_Data_No_Ack_test","checking NOACK feature of I2C ",I2C_Data_No_Ack_test},
#endif
    
#ifdef EN_I2C_Master_Interrupt_test
#if I2C_INSTANCES == 2
    {MOD_NAME,"I2C_Master_Interrupt_test1","checking various master interrupt sources ",I2C_Master_Interrupt_test1},
    {MOD_NAME,"I2C_Master_Interrupt_test2","checking various master interrupt sources ",I2C_Master_Interrupt_test2},
#else
    {MOD_NAME,"I2C_Master_Interrupt_test","checking various master interrupt sources ",I2C_Master_Interrupt_test},
#endif
#endif //EN_I2C_Master_Interrupt_test
    
#ifdef EN_I2C_Slave_Interrupt_test
    {MOD_NAME,"I2C_Slave_Interrupt_test","checking various slave interrupt sources ",I2C_Slave_Interrupt_test},
#endif

#ifdef EN_I2C_More_Slave_Rx_test
#if I2C_INSTANCES == 2
    {MOD_NAME,"I2C_More_Slave_Rx_test1","configure 2 I2C instances as Slave and receive data from master ",I2C_More_Slave_Rx_test1},
    {MOD_NAME,"I2C_More_Slave_Rx_test2","configure 2 I2C instances as Slave and receive data from master ",I2C_More_Slave_Rx_test2},
#else
    {MOD_NAME,"I2C_More_Slave_Rx_test","configure 2 I2C instances as Slave and receive data from master ",I2C_More_Slave_Rx_test},
#endif
#endif //EN_I2C_More_Slave_Rx_test
  
#ifdef EN_I2C_More_Slave_Tx_test
#if I2C_INSTANCES == 2
    {MOD_NAME,"I2C_More_Slave_Tx_test1","configure 2 I2C instances as Slave and transmit data to master ",I2C_More_Slave_Tx_test1},
    {MOD_NAME,"I2C_More_Slave_Tx_test2","configure 2 I2C instances as Slave and transmit data to master ",I2C_More_Slave_Tx_test2},
#else
    {MOD_NAME,"I2C_More_Slave_Tx_test","configure 2 I2C instances as Slave and receive data from master ",I2C_More_Slave_Tx_test},   
#endif
#endif //EN_I2C_More_Slave_Tx_test
    
#ifdef EN_I2C_DMA_Mstr_Tx_test
    {MOD_NAME,"I2C_DMA_Mstr_Tx_test","I2C DMA Master Tx",I2C_DMA_Mstr_Tx_test},
#endif
    
#ifdef EN_I2C_DMA_Mstr_Rx_test
    {MOD_NAME,"I2C_DMA_Mstr_Rx_test","I2C DMA Master Rx",I2C_DMA_Mstr_Rx_test},
#endif
          
#ifdef EN_I2C_Enable_Disable_test
    {MOD_NAME,"I2C_Enable_Disable_test","Checks I2C read write functionality in all possible mode",I2C_Enable_Disable_test},
#endif

#ifdef EN_I2C_XRDC_test
    {MOD_NAME,"I2C_XRDC_test","Checks I2C module registers protection by XRDC",I2C_XRDC_test},
#endif

#ifdef EN_I2C_Byte_Rx_Int_Enable_test
    {MOD_NAME,"I2C_Byte_Rx_Int_Enable_test","checking i2c rx interrupt feature",I2C_Byte_Rx_Int_Enable_test},
#endif

#ifdef EN_I2C_Reg_RW_User_Mode_test
    {MOD_NAME,"I2C_Reg_RW_User_Mode_test","Checks I2C Register Access in user mode",I2C_Reg_RW_User_Mode_test},
#endif
    
#else  //REGRESSION

#ifdef EN_I2C_Mstr_Tx_freq_test
    {MOD_NAME,"I2C_Mstr_Tx_freq_test","configure i2c module clock for various values and transfer data from master ",I2C_Mstr_Tx_freq_test},
#endif
    
#ifdef EN_I2C_Mstr_Rx_freq_test
    {MOD_NAME,"I2C_Mstr_Rx_freq_test","configure i2c module clock for various values and transfer data to master from slave",I2C_Mstr_Rx_freq_test},
#endif
    
#ifdef EN_I2C_Dbg_Mode_Before_Add_Tx_test
    {MOD_NAME,"I2C_Dbg_Mode_Before_Add_Tx_Master_test","Checking I2C operation in debug mode",I2C_Dbg_Mode_Before_Add_Tx_Master_test},
    {MOD_NAME,"I2C_Dbg_Mode_Before_Add_Tx_Slave_test","Checking I2C operation in debug mode",I2C_Dbg_Mode_Before_Add_Tx_Slave_test},
#endif
    
#ifdef EN_I2C_Debug_Mode_During_Trans_test
    {MOD_NAME,"I2C_Debug_Mode_During_Trans_Master_test","Checking I2C operation in debug mode while transfer ongoing",I2C_Debug_Mode_During_Trans_Master_test},
    {MOD_NAME,"I2C_Debug_Mode_During_Trans_Slave_test","Checking I2C operation in debug mode while transfer ongoing",I2C_Debug_Mode_During_Trans_Slave_test},
#endif
    
#ifdef EN_I2C_DMA_Debug_Mode_test
    {MOD_NAME,"I2C_DMA_Debug_Mode_test","Checking I2C operation in debug mode with DMA transfer",I2C_DMA_Debug_Mode_test},
#endif
    
#ifdef EN_I2C_Stop_Mode_test
    {MOD_NAME,"I2C_Stop_Mode_Master_test","checking stop mode effect in i2c transfer",I2C_Stop_Mode_Master_test},
    {MOD_NAME,"I2C_Stop_Mode_Slave_test","checking stop mode effect in i2c transfer",I2C_Stop_Mode_Slave_test},
#endif
    
#ifdef EN_I2C_Halt_Mode_test
    {MOD_NAME,"I2C_Halt_Mode_Master_test","checking halt mode effect in i2c transfer",I2C_Halt_Mode_Master_test},
    {MOD_NAME,"I2C_Halt_Mode_Slave_test","checking halt mode effect in i2c transfer",I2C_Halt_Mode_Slave_test},
#endif
    
#ifdef EN_I2C_Multi_Core_test
    {MOD_NAME,conc_test(I2C_Multi_Core_test),"Checks I2C interrupt for all the cores",I2C_Multi_Core_test},
#endif
    
#ifdef EN_I2C_Mstr_Tx_With_DFS_test
    {MOD_NAME,"I2C_Mstr_Tx_With_DFS_test","Checks I2C master tx functionality with DFS operation ",I2C_Mstr_Tx_With_DFS_test},
#endif
    
#ifdef EN_I2C_Pin_Mux_test
    {MOD_NAME,"I2C_Pin_Mux_test","checking i2c functionality at other available pins ",I2C_Pin_Mux_test},
#endif

#ifdef EN_I2C_short_func_reset_test
    {MOD_NAME,"I2C_short_func_reset_test","checking i2c after short func reset",I2C_short_func_reset_test},
#endif

#ifdef EN_I2C_External_Memory_Access_test
    {MOD_NAME,"I2C_External_memory_Access_test","Checks I2C External flash memory access in faradey board",I2C_External_Memory_Access_test},
#endif
    
#ifdef EN_I2C_Stress_test
    {MOD_NAME,"I2C_Stress_test","checking I2C data transfer with different slave address and baud rate",I2C_Stress_test},
#endif

#endif //REGRESSION   
};


int UIF_TEST_SIZE=sizeof(UIF_TEST_CASES)/sizeof(UIF_TEST);

void user_testcase()
{
    
}
