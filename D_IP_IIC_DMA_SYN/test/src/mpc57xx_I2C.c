// --------------------------------------------------------------------------
// COPYRIGHT (c) Freescale 2009
// Freescale Confidential Proprietary
//  All Rights Reserved
// --------------------------------------------------------------------------
//!\file    mpc57xx_I2C.c
//!\brief   This file conatins the test case giving the example of use of
//          software interrupts implementation of reset within a test
//
// TYPE: C Source File
// DEPARTMENT: MSG-IDC
// CREATED BY: Aravindan V  (B44571)
// --------------------------------------------------------------------------
// PURPOSE:  Example test cases
// --------------------------------------------------------------------------
// $Log: mpc57xx_I2C.c.rca $
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
//  Revision: 1.6 Mon Sep  7 16:01:24 2015 b44571
//  *** empty comment string ***
// 
//  Revision: 1.5 Fri Aug 14 17:18:34 2015 B44571
//  *** empty comment string ***
// 
//  Revision: 1.4 Fri Jun 12 16:31:05 2015 b44571
//  *** empty comment string ***
// 
//  Revision: 1.3 Fri Jun 12 12:36:52 2015 b44571
//  *** empty comment string ***
// 
//  Revision: 1.2 Fri Jun 12 12:22:03 2015 b44571
//  *** empty comment string ***
// 
//  Revision: 1.1 Fri Sep 19 15:05:04 2014 B44571
//  *** empty comment string ***


#include "common.h"
#include "i2c_soc_config.h"
#include "cgm_api.h"
#include "cgm_defines.h"
//#include "fccu_api.h"
#include "mc_rgm_api.h"
#include "mc_rgm_defines.h"
#include "mc_rgm_soc_config.h"

extern int reset_status;

#define FREQ_OPT1 1
#define FREQ_OPT2 2

#define BAUD_RATE_DIV 0x25

          ////////Function/ISR declarations///////////////
void DMA0_ISR();
void i2c0_stop_mode_isr();
void i2c1_stop_mode_isr();
#if I2C_INSTANCES == 3 || I2C_INSTANCES == 4
void i2c2_stop_mode_isr();
#endif
#if I2C_INSTANCES == 4
void i2c3_stop_mode_isr();
#endif
void i2c_dma_config(uint8_t,uint8_t);
void i2c_rx_int_isr();

        //////////Global Variable declarations///////////
extern I2C_BUFFER i2c_tx_buffer;
extern I2C_BUFFER i2c_rx_buffer;

extern volatile uint8_t master_tx_done,master_rx_done,slave_tx_done,slave_rx_done,\
    master_mode,more_master_var,stop,DMA_recv_enb,check_en,find_slave;
extern uint8_t bus_idle[I2C_INSTANCES],no_ack[I2C_INSTANCES],arb_lost[I2C_INSTANCES];
extern int i2c_short_func_reset;
uint16_t r_err_count,w_err_count;
uint8_t enableRandomModeEntry;

/* Globals for this file only */
extern uint8_t my_buffer[NO_OF_DATA] = { 0x48, 0x32, 0x40, 0x20, 0x4C, 
			          0x6F, 0x6F, 0x70, 0x62, 0x61, 
			          0x63, 0x6B, 0x20, 0x65, 0x78, 
			          0x61, 0x6D, 0x70, 0x6C, 0x65, 0x2E };

uint8_t master_tf_over=0,master_dma_inst,DMA_TX,DMA_RX;
uint8_t tx_byte_count = sizeof(my_buffer),i2c_inst_global=0;
uint32_t t_cnt1,t_cnt2,stm_time; 


void MC_ME_read_write_err_isr()
{
    r_err_count=1;
    w_err_count=1;
}

void I2C_Reg_Rst_Chk_test(void)
{
#ifdef EN_I2C_Reg_Rst_Chk_test
    
    uint8_t fail_count=0,rxd_char=0,instance_num;

    printf("Starting Reset values comparison...\n\n");

    for(instance_num=0;instance_num < I2C_INSTANCES;instance_num++) {
	
	if(i2c_struct[instance_num].I2C_n->IBAD.R != IBAD_reset) {
	    error("I2C_[%d].IBAD.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBAD.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBAD.R",instance_num);

	if(i2c_struct[instance_num].I2C_n->IBFD.R != IBFD_reset) {
	    error("I2C_[%d].IBFD.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBFD.R );
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBFD.R",instance_num);

	if(i2c_struct[instance_num].I2C_n->IBCR.R != IBCR_reset) {
	    error("I2C_[%d].IBCR.R = 0x%x",instance_num, i2c_struct[instance_num].I2C_n->IBCR.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBCR.R",instance_num);

	if(i2c_struct[instance_num].I2C_n->IBSR.R != IBSR_reset) {
	    error("I2C_[%d].IBSR.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBSR.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBSR.R",instance_num);

	if(i2c_struct[instance_num].I2C_n->IBDR.R != IBDR_reset) {
	    error("I2C_[%d].IBDR.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBDR.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBDR.R",instance_num);

	if(i2c_struct[instance_num].I2C_n->IBIC.R != IBIC_reset) {
	    error("I2C_[%d].IBIC.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBIC.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBIC.R",instance_num);

	if(i2c_struct[instance_num].I2C_n->IBDBG.R != IBDR_reset) {
	    error("I2C_[%d].IBDBG.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBDBG.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBDBG.R",instance_num);

	if(fail_count==0)
	{
	    printf ("Reset value of I2C[%d] registers correct.!\n\r",instance_num);
	}
	else
	{
	    error("I2C[%d] Reset Register value test case Failed and fail count is = %d\n\r",instance_num,fail_count);
	}
    }

#else
    error("NA for this NPI");
#endif

}

void I2C_Reg_RW_test(void)
{
#ifdef EN_I2C_Reg_RW_test
    
    uint8_t rxd_char=0, instance_num, fail_count=0;

#ifdef DEBUG
    printf("Starting Read-Write register access test...\n\n");
#endif
   
   // init_smpu();
    //smpu_enable_all_master_access();

    for(instance_num=0;instance_num < I2C_INSTANCES;instance_num++) {

	i2c_struct[instance_num].I2C_n->IBAD.R  = 0xFF;
	if(i2c_struct[instance_num].I2C_n->IBAD.R  != 0xFE)
	{
	    error("I2C_[%d].IBAD.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBAD.R );
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBAD.R",instance_num);

	// Put IBAD back in its reset state
	i2c_struct[instance_num].I2C_n->IBAD.R = IBAD_reset;

	//***** IBFD *****
	i2c_struct[instance_num].I2C_n->IBFD.R = 0xFF;
	if(i2c_struct[instance_num].I2C_n->IBFD.R!= 0xFF)
	{
	    error("I2C_[%d].IBFD.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBFD.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBFD.R",instance_num);		

	// Put IBFD back in its reset state
	i2c_struct[instance_num].I2C_n->IBFD.R = IBFD_reset;

	//***** IBCR ***** (bit 2 is read-only)
    	i2c_struct[instance_num].I2C_n->IBCR.R  = 0xFF;
	if(i2c_struct[instance_num].I2C_n->IBCR.R  != 0xFB)
	{
	    error("I2C_[%d].IBCR.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBCR.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBCR.R",instance_num);  

    	// Put IBCR back in its reset state
    	i2c_struct[instance_num].I2C_n->IBCR.R = IBCR_reset;


	//***** IBSR *****
	// All bits that are writeable, are not readable as 1s.
	i2c_struct[instance_num].I2C_n->IBSR.R  = 0xFF;
	if(i2c_struct[instance_num].I2C_n->IBSR.R != 0x80)
	{
	    error("I2C_[%d].IBSR.R = 0x%x",instance_num, i2c_struct[instance_num].I2C_n->IBSR.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBSR.R",instance_num);

	//***** IBDR *****
	// All bits that are writeable, are not readable as 1s.

/*	i2c_struct[instance_num].I2C_n->IBDR.R  = 0xFF;
 - 	if(i2c_struct[instance_num].I2C_n->IBDR.R != 0xFF)
 - 	{
 - 	    error("I2C_[%d].IBDR.R = 0x%x",instance_num, i2c_struct[instance_num].I2C_n->IBDR.R);
 - 	    fail_count++;
 - 	}
 - 	else
 - 	    printf("ok I2C_[%d].IBDR.R",instance_num);
 - 	i2c_struct[instance_num].I2C_n->IBDR.R = IBDR_reset;*/

	//***** IBIC *****
	i2c_struct[instance_num].I2C_n->IBCR.R=0x00;
	i2c_struct[instance_num].I2C_n->IBIC.R = 0xFF;
	if(i2c_struct[instance_num].I2C_n->IBIC.R != 0xC0)
	{
	    error("I2C_[%d].IBIC.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBIC.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBIC.R",instance_num);

	// Put IBIC back in its reset state
	i2c_struct[instance_num].I2C_n->IBIC.R = IBIC_reset;
	i2c_struct[instance_num].I2C_n->IBCR.R = IBCR_reset;

	//****IBDBG****
	i2c_struct[instance_num].I2C_n->IBDBG.R = 0xFF;
	if(i2c_struct[instance_num].I2C_n->IBDBG.R != 0x09)
	{
	    error("I2C_[%d].IBDBG.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBDBG.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBDBG.R",instance_num);

        // Put IBDBG back in its reset state
        i2c_struct[instance_num].I2C_n->IBDBG.R = IBDBG_reset;

	if(fail_count==0)
	{
	    printf("I2C[%d] Register Read/Write test case Passed ",instance_num);
	}
	else
	{
	    error("I2C[%d] Register Read/Write test case Failed and fail count is = %d\n\r",instance_num,fail_count);
	}
    }

#else
    error("NA for this NPI");
#endif
        
}

void I2C_Reg_RW_User_Mode_test(void)
{
#ifdef EN_I2C_Reg_RW_User_Mode_test
    
    uint8_t rxd_char=0, instance_num, fail_count=0;

#ifdef DEBUG
    printf("Starting Read-Write register access test in user mode...\n\n");
#endif

    printf("I2C register are not accessible in User Mode");
    printf("Writes are ignored and Read returns value 0");
    
//    intc_init(CORE_0);
//    intc_install_exception(MACHINE_CHECK, I2C_MC_ISR);
    
/*    (*(vuint32_t*)(0xFC00016C)) &= 0xFFF0FFFF; // Configuring LIN2 to be accessible in user mode
 -     (*(vuint32_t*)(0xFC000170)) &= 0xFFFF00FF; // Configuring I2C0 and I2C2 to be accessible in user mode
 -     (*(vuint32_t*)(0xF8000170)) &= 0xFFFF00FF; // Configuring I2C1 and I2C3 to be accessible in user mode*/

    printf("Entering to User Mode");
	
    enter_User_Mode();
    
    for(instance_num=0;instance_num < I2C_INSTANCES;instance_num++) {

	i2c_struct[instance_num].I2C_n->IBAD.R  = 0xFF;
	if(i2c_struct[instance_num].I2C_n->IBAD.R  != 0x00)
	{
	    error("I2C_[%d].IBAD.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBAD.R );
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBAD.R",instance_num);

	// Put IBAD back in its reset state
	i2c_struct[instance_num].I2C_n->IBAD.R = IBAD_reset;

	//***** IBFD *****
	i2c_struct[instance_num].I2C_n->IBFD.R = 0xFF;
	if(i2c_struct[instance_num].I2C_n->IBFD.R != 0x00)
	{
	    error("I2C_[%d].IBFD.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBFD.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBFD.R",instance_num);		

	// Put IBFD back in its reset state
	i2c_struct[instance_num].I2C_n->IBFD.R = IBFD_reset;

	//***** IBCR ***** (bit 2 is read-only)
    	i2c_struct[instance_num].I2C_n->IBCR.R  = 0xFF;
	if(i2c_struct[instance_num].I2C_n->IBCR.R  != 0x00)
	{
	    error("I2C_[%d].IBCR.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBCR.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBCR.R",instance_num);  

    	// Put IBCR back in its reset state
    	i2c_struct[instance_num].I2C_n->IBCR.R = IBCR_reset;


	//***** IBSR *****
	// All bits that are writeable, are not readable as 1s.
	i2c_struct[instance_num].I2C_n->IBSR.R  = 0xFF;
	if(i2c_struct[instance_num].I2C_n->IBSR.R != 0x00)
	{
	    error("I2C_[%d].IBSR.R = 0x%x",instance_num, i2c_struct[instance_num].I2C_n->IBSR.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBSR.R",instance_num);

	//***** IBIC *****
	i2c_struct[instance_num].I2C_n->IBCR.R=0x00;
	i2c_struct[instance_num].I2C_n->IBIC.R = 0xFF;
	if(i2c_struct[instance_num].I2C_n->IBIC.R != 0x00)
	{
	    error("I2C_[%d].IBIC.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBIC.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBIC.R",instance_num);

	// Put IBIC back in its reset state
	i2c_struct[instance_num].I2C_n->IBIC.R = IBIC_reset;
	i2c_struct[instance_num].I2C_n->IBCR.R = IBCR_reset;

	//****IBDBG****
	i2c_struct[instance_num].I2C_n->IBDBG.R = 0xFF;
	if(i2c_struct[instance_num].I2C_n->IBDBG.R != 0x00)
	{
	    error("I2C_[%d].IBDBG.R = 0x%x",instance_num,i2c_struct[instance_num].I2C_n->IBDBG.R);
	    fail_count++;
	}
	else
	    printf("ok I2C_[%d].IBDBG.R",instance_num);

        // Put IBDBG back in its reset state
        i2c_struct[instance_num].I2C_n->IBDBG.R = IBDBG_reset;

	if(fail_count==0)
	{
	    printf("I2C[%d] Register Read/Write test case Passed ",instance_num);
	}
	else
	{
	    error("I2C[%d] Register Read/Write test case Failed and fail count is = %d\n\r",instance_num,fail_count);
	}
    }

    printf("$__TEST_FINISHED__$"); // This will disable the FCCU check

    enter_Supervisor_Mode();

#else
    error("NA for this NPI");
#endif
}

void I2C_find_slave_test(void)
{
    uint16_t slave_add;
    
    i2c_init(e_I2C_MASTER,0,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
    //i2c_init(e_I2C_SLAVE,1,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

    slave_add = i2c_find_slave_add(0);

    if(slave_add == 270) {
	error("no slave ack");
    }
    else {
	printf("slave_add=%x",slave_add);
    }
}

void I2C_Mstr_Tx_test(void)
{
#ifdef EN_I2C_Mstr_Tx_test
    
    int i;
    uint8_t rxd_char,master,master_num=0,slave_num=0,err_cnt=0;

    more_master_var = 0;
    check_en = 1;

    printf("I2C Master Transfer test case");

    while(1)
    {

#if defined(REGRESSION)
	if(master_num == I2C_INSTANCES) {
	    break;
	}
	master_num = master_num + 0;
	slave_num  = master_num + 1;
	
	if(slave_num == I2C_INSTANCES) {
	    slave_num = 0;
	}	
#else
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;
	
	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");
	
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

#endif

	master_tf_over=err_cnt=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	while(master_tf_over == 0)
	{
	    /* Echo received data */
	    if (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr1_Write_Transfer);
	    }
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);	

#if defined(REGRESSION)
	master_num++;
#endif
    }

#else
    error("NA for this NPI");
#endif
}

void I2C_Mstr_Rx_test(void)
{
#ifdef EN_I2C_Mstr_Rx_test
    
    int i;
    uint8_t rxd_char,master_num=0,slave_num=0,err_cnt=0;

    more_master_var = 0;
    check_en = 1;

    printf("I2C Master receive test case");
    
    while(1)
    {
	
#if defined(REGRESSION)
	if(master_num == I2C_INSTANCES) {
	    break;
	}
	master_num = master_num + 0;
	slave_num  = master_num + 1;
	
	if(slave_num == I2C_INSTANCES) {
	    slave_num = 0;
	}	
#else
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;
	
	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");
	
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

#endif

	master_tf_over=err_cnt=0;
	
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	while(master_tf_over == 0)
	{
	    /* Echo received data */
	    if (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		master_tf_over = i2c_transfer(I2C_RX,master_num,e_Slave_Addr1_Read_Transfer);
	    }
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);
#if defined(REGRESSION)
	master_num++;
#endif
    }

#else
    error("NA for this NPI");
#endif
}

void I2C_Rpt_Strt_test(void)
{
#ifdef EN_I2C_Rpt_Strt_test

    int i;
    uint8_t rxd_char,master_num=0,slave_num=0,err_cnt=0;
    
    more_master_var = 0;
    check_en = 1;

    printf("I2C Repeated start test case");
    
    while(1)
    {
	
#if defined(REGRESSION)
	if(master_num == I2C_INSTANCES) {
	    break;
	}
	master_num = master_num + 0;
	slave_num  = master_num + 1;

	if(slave_num == I2C_INSTANCES) {
	    slave_num = 0;
	}	
#else
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

#endif
	master_tf_over=err_cnt=0;
	
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	while(master_tf_over == 0)
	{
	    /* Echo received data */
	    if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		master_tf_over = i2c_transfer(I2C_TXRX,master_num,e_Slave_Addr1_Write_Transfer);
	    }
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);
#if defined(REGRESSION)
	master_num++;
#endif
    }
    
#else
    error("NA for this NPI");
#endif
    
}

void I2C_Data_No_Ack_test(void)
{
#ifdef EN_I2C_Data_No_Ack_test
    
    int i;
    uint8_t rxd_char,master_num=0,slave_num=0,err_cnt=0;
    
    more_master_var = 0;
    check_en = 1;

    printf("I2C No Acknowledgement test case");
    
    while(1)
    {
#if defined(REGRESSION)
	if(master_num == I2C_INSTANCES) {
	    break;
	}
	master_num = master_num + 0;
	slave_num  = master_num + 1;

	if(slave_num == I2C_INSTANCES) {
	    slave_num = 0;
	}	
#else
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

#endif
	master_tf_over=err_cnt=0;
	
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	
	while(master_tf_over == 0)
	{
	    /* Echo received data */
	    if (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		master_tf_over = i2c_transfer(I2C_RX,master_num,e_Slave_Addr1_Read_Transfer);
	    }
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);
#if defined(REGRESSION)
	master_num++;
#endif
    }
    
#else
    error("NA for this NPI");
#endif
}

#if I2C_INSTANCES == 2

void I2C_More_Slave_Rx_test1()
{
#ifdef EN_I2C_More_Slave_Rx_test
    
    uint8_t i,rxd_char,master_num,slave_num1,slave_num2,slave_num3,slave_select;

    more_master_var = 0;

    while(1)
    {
	printf("I2C More slave receive test case");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");
	
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num1 = get_int_with_limit("\n Please select the slave instance no 1 ");

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num1,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	set_int_limits(1,2);
	slave_select = get_int_with_limit("\n select one slave instance from above list for which master has to send data");

	while(master_tf_over == 0)
	{
	    if(slave_select==1) {
		/* Echo received data */
		if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		    master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr1_Write_Transfer);
		}
	    }
	    else {
		if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		    master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr2_Write_Transfer);
		}
	    }

	}

	if(slave_select == 1) {
	    printf("Checking transferred data validity");
	    for (i=0;i<sizeof(my_buffer);i++) {
		if(i2c_tx_buffer.buf[i]== i2c_rx_buffer.buf[i]) {
		    printf("Data=0x%x transfered successfully",i2c_rx_buffer.buf[i]);
		}
		else{
		    error("Data=0x%x transfer failed",i2c_rx_buffer.buf[i]);
		}
		i2c_tx_buffer.buf[i]=0;
		i2c_rx_buffer.buf[i]=0;    
	    }
	}
    }

#else
    error("NA for this NPI");
#endif

}

void I2C_More_Slave_Rx_test2()
{
#ifdef EN_I2C_More_Slave_Rx_test
    
    uint8_t i,rxd_char,slave_num1,slave_num2,slave_num3,slave_select;

    more_master_var = 0;

    while(1)
    {
	printf("I2C More slave Transmit test case");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num2 = get_int_with_limit("\n Please select the slave instance no 2 ");

	i2c_init(e_I2C_SLAVE,slave_num2,BAUD_RATE_DIV,e_ADR_Slave_ADR2);
		
	while (!slave_rx_done)
	{
	    asm("nop");
	}
	slave_rx_done = FALSE;

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]== i2c_rx_buffer.buf[i]) {
		printf("Data=0x%x transfered successfully",i2c_rx_buffer.buf[i]);
	    }
	    else{
		error("Data=0x%x transfer failed",i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
    }
    
#else
    error("NA for this NPI");
#endif
}


void I2C_More_Slave_Tx_test1()
{
#ifdef EN_I2C_More_Slave_Tx_test
    
    uint8_t i,rxd_char,master_num,slave_num1,slave_num2,slave_num3,slave_select;

    more_master_var = 0;

    while(1)
    {
	printf("I2C More slave Transmit test case");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");
	
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num1 = get_int_with_limit("\n Please select the slave instance no 1 ");

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num1,BAUD_RATE_DIV,e_ADR_Slave_ADR1);	

	set_int_limits(1,2);
	slave_select = get_int_with_limit("\n select one slave instance from above list for which master has to send data");

	while(master_tf_over == 0)
	{
	    if(slave_select==1) {
		/* Echo received data */
		if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		    master_tf_over = i2c_transfer(I2C_RX,master_num,e_Slave_Addr1_Read_Transfer);
		}
	    }
	    else  {
		if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		    master_tf_over = i2c_transfer(I2C_RX,master_num,e_Slave_Addr2_Read_Transfer);
		}
	    }
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]== i2c_rx_buffer.buf[i]) {
		printf("Data=0x%x transfered successfully",i2c_rx_buffer.buf[i]);
	    }
	    else{
		error("Data=0x%x transfer failed",i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
    }

#else
    error("NA for this NPI");
#endif

}

void I2C_More_Slave_Tx_test2()
{
#ifdef EN_I2C_More_Slave_Tx_test
    
    uint8_t i,rxd_char,slave_num1,slave_num2,slave_num3,slave_select;

    more_master_var = 0;

    while(1)
    {

	printf("I2C More slave Transmit test case");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num2 = get_int_with_limit("\n Please select the slave instance no 2 ");

	i2c_init(e_I2C_SLAVE,slave_num2,BAUD_RATE_DIV,e_ADR_Slave_ADR2);
	
	while (!slave_tx_done);
	     slave_tx_done = FALSE;
	     
    }
#else
    error("NA for this NPI");
#endif
}

void I2C_Master_Interrupt_test1()
{
#ifdef EN_I2C_Master_Interrupt_test
    
    uint32_t temp,rxd_char=0,i,master_num1,master_num2,master_num3,slave_address;

    more_master_var = 1;
    
    while(1)
    {
	printf("I2C Master side interrupt test case");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);
	
	set_int_limits(0,I2C_INSTANCES-1);
	master_num1 = get_int_with_limit("\n Please select the master1 instance no ");
	
	set_int_limits(0,I2C_INSTANCES-1);
	master_num2 = get_int_with_limit("\n Please select the master2 instance no ");

	i2c_init(e_I2C_MASTER,master_num1,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_MASTER,master_num2,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	printf("Enable Bus Idle Interrupt");
	RegWrF(master_num1,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable,1);
	RegWrF(master_num2,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable,1);
	
	set_user_option("transfer correct slave address from master");
	set_user_option("transfer wrong slave address from master");

	temp = get_user_option_index("Select slave address Options.");
	if(temp == 0) {
	    slave_address = e_Slave_Addr1_Write_Transfer;
	}
	else {
	    slave_address = e_Slave_Addr3_Write_Transfer;
	}

	printf("Generating start condition from master I2C %d registers",master_num1);
	RegWrF(master_num1,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);
	RegWrF(master_num1,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);

	printf("Attempting start condition from other masters when bus is busy"); 
	printf("Generating start condition from master I2C %d registers",master_num2);
	RegWrF(master_num2,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);
	RegWrF(master_num2,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);

	while(stop != 1) {
	     asm("nop");
	}
	stop=0;
	
	if(temp == 0) {
	    while(master_tf_over == 0)
	    {
		master_tf_over = i2c_transfer(I2C_TX,master_num1,slave_address);

	    }
	}
	else {
	    RegWrF(master_num1,IBDR, IBDR_Data,slave_address); //generating noack interrupt

	    while(stop != 1) {
		asm("nop");
	    }
	    stop=0;
	}
    }

#else
    error("NA for this NPI");
#endif
}

void I2C_Master_Interrupt_test2()
{
#ifdef EN_I2C_Master_Interrupt_test
    
    uint32_t temp,rxd_char=0,i,slave_num,slave_address;

    more_master_var = 1;
    
    while(1)
    {
	printf("I2C Master side interrupt test case");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);
	
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");
	
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	printf("Enable Bus Idle Interrupt");
	RegWrF(slave_num,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable,1);
	
	while (!slave_rx_done);
	     slave_rx_done = FALSE;

    }
    
#else
     error("NA for this NPI");
#endif 
}

#elif I2C_INSTANCES == 3 || I2C_INSTANCES == 4

void I2C_More_Slave_Rx_test()
{
#ifdef EN_I2C_More_Slave_Rx_test
    
    uint8_t i,rxd_char,master_num=0,slave_num1=0,slave_num2=0,slave_num3=0,slave_select=0,err_cnt=0;

    more_master_var = 0;
    check_en = 1;

    printf("I2C More slave receive test case");
    
    while(1)
    {

#if defined(REGRESSION)
	if(master_num == I2C_INSTANCES) {
	    break;
	}
	master_num = master_num + 0;
	slave_num1  = master_num + 1;
	if(slave_num1 == I2C_INSTANCES) {
	    slave_num1 = 0;
	}
	slave_num2  = slave_num1 + 1;
	if(slave_num2 == I2C_INSTANCES) {
	    slave_num2 = 0;
	}
#if I2C_INSTANCES == 4
	slave_num3  = slave_num2 + 1;
	if(slave_num3 == I2C_INSTANCES) {
	    slave_num3 = 0;
	}
#endif
	
#else
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;
	
	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");
	
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num1 = get_int_with_limit("\n Please select the slave instance no 1 ");

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num2 = get_int_with_limit("\n Please select the slave instance no 2 ");

#if I2C_INSTANCES == 4
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num3 = get_int_with_limit("\n Please select the slave instance no 3");
#endif

#endif

	master_tf_over=err_cnt=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num1,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num2,BAUD_RATE_DIV,e_ADR_Slave_ADR2);
#if I2C_INSTANCES == 4
	i2c_init(e_I2C_SLAVE,slave_num3,BAUD_RATE_DIV,e_ADR_Slave_ADR3);
#endif
	
#if defined(REGRESSION)
	slave_select = slave_num2;
#else
	printf("slave_num1 is%d",slave_num1);
	printf("slave_num2 is%d",slave_num2);
#if I2C_INSTANCES == 4
	printf("slave_num3 is%d",slave_num3);
#endif
	slave_select =get_int("select one slave instance from above list for which master has to send data");
#endif

	while(master_tf_over == 0)
	{
	    if(slave_select==slave_num1) {
		/* Echo received data */
		if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		    master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr1_Write_Transfer);
		}
	    }
	    else if(slave_select==slave_num2) {
		if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		    master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr2_Write_Transfer);
		}
	    }
#if I2C_INSTANCES == 4
	    else {
		if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		    master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr3_Write_Transfer);
		}
	    }
#endif
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num1);
	i2c_reg_reset(slave_num2);
#if I2C_INSTANCES == 4
	i2c_reg_reset(slave_num3);
#endif
	
#if defined(REGRESSION)
	master_num++;
#endif
    }
    
#else
    error("NA for this NPI");
#endif  
}

void I2C_More_Slave_Tx_test()
{
#ifdef EN_I2C_More_Slave_Tx_test
    
    uint8_t i,rxd_char,master_num=0,slave_num1=0,slave_num2=0,slave_num3=0,slave_select=0,err_cnt=0;

    more_master_var = 0;
    check_en = 1;

    printf("I2C More slave Transmit test case");
    
    while(1)
    {

#if defined(REGRESSION)
	if(master_num == I2C_INSTANCES) {
	    break;
	}
	master_num = master_num + 0;
	slave_num1  = master_num + 1;
	if(slave_num1 == I2C_INSTANCES) {
	    slave_num1 = 0;
	}
	slave_num2  = slave_num1 + 1;
	if(slave_num2 == I2C_INSTANCES) {
	    slave_num2 = 0;
	}
#if I2C_INSTANCES == 4
	slave_num3  = slave_num2 + 1;
	if(slave_num3 == I2C_INSTANCES) {
	    slave_num3 = 0;
	}
#endif

#else
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num1 = get_int_with_limit("\n Please select the slave instance no 1 ");

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num2 = get_int_with_limit("\n Please select the slave instance no 2 ");

#if I2C_INSTANCES == 4
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num3 = get_int_with_limit("\n Please select the slave instance no 3");
#endif

#endif

	master_tf_over=err_cnt=0;
	
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);
	
	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num1,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num2,BAUD_RATE_DIV,e_ADR_Slave_ADR2);
#if I2C_INSTANCES == 4
	i2c_init(e_I2C_SLAVE,slave_num3,BAUD_RATE_DIV,e_ADR_Slave_ADR3);
#endif
	
#if defined(REGRESSION)
	slave_select = slave_num2;
#else
	printf("slave_num1 is%d",slave_num1);
	printf("slave_num2 is%d",slave_num2);
#if I2C_INSTANCES == 4
	printf("slave_num3 is%d",slave_num3);
#endif
	slave_select =get_int("select one slave instance from above list for which master has to send data");
#endif
	
	while(master_tf_over == 0)
	{
	    if(slave_select==slave_num1) {
		/* Echo received data */
		if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		    master_tf_over = i2c_transfer(I2C_RX,master_num,e_Slave_Addr1_Read_Transfer);
		}
	    }
	    else if(slave_select==slave_num2) {
		if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		    master_tf_over = i2c_transfer(I2C_RX,master_num,e_Slave_Addr2_Read_Transfer);
		}
	    }
#if I2C_INSTANCES == 4
	    else {
		if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		    master_tf_over = i2c_transfer(I2C_RX,master_num,e_Slave_Addr3_Read_Transfer);
		}
	    }
#endif
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num1);
	i2c_reg_reset(slave_num2);
#if I2C_INSTANCES == 4
	i2c_reg_reset(slave_num3);
#endif
	
#if defined(REGRESSION)
	master_num++;
#endif
    }

#else
    error("NA for this NPI");
#endif
}

void I2C_Master_Interrupt_test()
{
#ifdef EN_I2C_Master_Interrupt_test
    
    uint32_t temp,rxd_char=0,i,master_num1=0,master_num2=0,master_num3=0,slave_num=0,slave_address=0,err_cnt=0;

    more_master_var = 1;
    check_en = 0;

    printf("I2C Master side interrupt test case");
    
    while(1)
    {
#if defined(REGRESSION)
	if(master_num1 == I2C_INSTANCES) {
	    break;
	}
	master_num1  = master_num1 + 0;
	master_num2  = master_num1 + 1;
	if(master_num2 == I2C_INSTANCES) {
	    master_num2 = 0;
	}
#if I2C_INSTANCES == 4
	master_num3  = master_num2 + 1;
	if(master_num3 == I2C_INSTANCES) {
	    master_num3 = 0;
	}
	slave_num  = master_num3 + 1;
	if(slave_num == I2C_INSTANCES) {
	    slave_num3 = 0;
	}
#else
	slave_num  = master_num2 + 1;
	if(slave_num == I2C_INSTANCES) {
	    slave_num = 0;
	}
#endif	
#else //REGRESSION
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	set_int_limits(0,I2C_INSTANCES-1);
	master_num1 = get_int_with_limit("\n Please select the master1 instance no ");

	set_int_limits(0,I2C_INSTANCES-1);
	master_num2 = get_int_with_limit("\n Please select the master2 instance no ");

#if I2C_INSTANCES == 4
	set_int_limits(0,I2C_INSTANCES-1);
	master_num3 = get_int_with_limit("\n Please select the master3 instance no ");
#endif

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

#endif
	
	master_tf_over=err_cnt=0;
	
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	i2c_init(e_I2C_MASTER,master_num1,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_MASTER,master_num2,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
#if I2C_INSTANCES == 4
	i2c_init(e_I2C_MASTER,master_num3,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
#endif
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

/*	set_user_option("transfer correct slave address from master");
 - 	set_user_option("transfer wrong slave address from master");
 - 
 - 	temp = get_user_option_index("Select slave address Options.");
 - 	if(temp == 0) {
 - 	    slave_address = e_Slave_Addr1_Write_Transfer;
 - 	}
 - 	else {*/
	slave_address = e_Slave_Addr3_Write_Transfer;
	//}
	
	printf("MASTER:Configuring I2C %d registers",master_num1);
	RegWrF(master_num1,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable,1);
	RegWrF(master_num1,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);
	RegWrF(master_num1,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);

	printf("Attempting start condition from other masters when bus is busy"); 
	printf("MASTER:Configuring I2C %d registers",master_num2);
	RegWrF(master_num2,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable,1);
	RegWrF(master_num2,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);
	RegWrF(master_num2,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);
	while(stop!=1) {
	     asm("nop");
	}
	stop=0;

#if I2C_INSTANCES == 4
	printf("MASTER:Configuring I2C %d registers",master_num3);
	RegWrF(master_num3,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable,1);
	RegWrF(master_num3,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);
	RegWrF(master_num3,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);
	while(stop!=1) {
	     asm("nop");
	}
	stop=0;
#endif
	
	while(master_tf_over == 0)
	{
	    master_tf_over = i2c_transfer(I2C_TX,master_num1,slave_address);
	}

	while(stop!=1) {
	    asm("nop");
	}
	stop=0;

	if(arb_lost[master_num2] == 0) {
	    error("Arb lost interrupt not occured at Master2 side when expected");
	}
	else {
	    printf("Arb lost interrupt occured at Master2 side");
	}
	
	if(arb_lost[master_num1] == 0) {
	    printf("Arb lost interrupt not occured at Master1 side");
	}
	else {
	    error("Arb lost interrupt occured at Master1 side");
	}

	if(bus_idle[master_num1] == 0) {
	    error("Bus Idle interrupt not occured at Master1 side when expected");
	}
	else {
	    printf("Bus idle interrupt occured at Master1 side");
	}

	if(no_ack[master_num1] == 0) {
	    error("No_Ack interrupt not occured at Master1 side when expected");
	}
	else {
	    printf("No_Ack interrupt occured at Master1 side");
	}
	
	i2c_reg_reset(master_num1);
	i2c_reg_reset(master_num2);
	i2c_reg_reset(slave_num);
	i2c_reset_variables(master_num1);
	i2c_reset_variables(master_num2);
	i2c_reset_variables(slave_num);

#if defined(REGRESSION)
	master_num1++;
#endif
    }
#else
    error("NA for this NPI");
#endif
}

#endif //defined(RACERUNNER) || defined(CALYPSO6M_1_0)


void I2C_Slave_Interrupt_test(void)
{
#ifdef EN_I2C_Slave_Interrupt_test
    
    int i;
    uint8_t rxd_char,master,master_num=0,slave_num=0,err_cnt=0;

    more_master_var = 0;
    check_en = 1;
	
    printf("I2C Slave side interrupt test case");

    while(1)
    {
#if defined(REGRESSION)
	if(master_num == I2C_INSTANCES) {
	    break;
	}
	master_num = master_num + 0;
	slave_num  = master_num + 1;

	if(slave_num == I2C_INSTANCES) {
	    slave_num = 0;
	}	
#else
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

#endif

	master_tf_over=err_cnt=0;
	
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	printf("Enable Bus Idle Interrupt");
	RegWrF(master_num,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable,1);
	RegWrF(slave_num,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable,1);

	printf("set repeated start bit in slave instance to generate arbitration lost"); 
	RegWrF(slave_num,IBCR,IBCR_RSTA_Repeat_Start_Master_Only,e_SET);

	while(stop!=1) {
	    asm("nop");
	}
	stop=0;

	while(master_tf_over == 0)
	{
	    /* Echo received data */
	    if (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr1_Write_Transfer);
	    }
	}
	
	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	if(arb_lost[slave_num] == 0) {
	    error("Arb lost interrupt not occured when expected");
	}
	else {
	    printf("Arb lost interrupt occured at slave side");
	}

	if(bus_idle[slave_num] == 0) {
	    error("Bus Idle interrupt not occured at slave side when expected");
	}
	else {
	    printf("Bus idle interrupt occured at slave side");
	}
	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);
	i2c_reset_variables(master_num);
	i2c_reset_variables(slave_num);	

#if defined(REGRESSION)
	master_num++;
#endif

    }

#else
    error("NA for this NPI");
#endif
}

/********************************************************************/
/**
 *\brief This is the function which configures the DMA to transmit data through I2C.
 *
 *This function configures DMA channel to transmit data to I2C data register.
 *After that I2C is configured to transmit data through Master and that data is read by the I2C slave.
 */

void I2C_DMA_Mstr_Tx_test(void)
{
#ifdef EN_I2C_DMA_Mstr_Tx_test
    
    uint8_t temp,rxd_char=0,fail=0,i=0,master_num=0,slave_num=0,err_cnt=0;
            
    // tx_buffer stores the data that is to be transferred to iPort
    more_master_var = 0;
    check_en = 1;
    DMA_recv_enb=0;
    DMA_RX=0;
    DMA_TX=1;
    
    printf("Starting DMA TX test");

    while(1)
    {
#if defined(REGRESSION)
	if(master_num == I2C_INSTANCES) {
	    break;
	}
	master_num = master_num + 0;
	slave_num  = master_num + 1;

	if(slave_num == I2C_INSTANCES) {
	    slave_num = 0;
	}	
#else
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

#endif

	master_tf_over=err_cnt=0;
	
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	i2c_dma_config(master_num,DMA_TX);

	master_dma_inst = master_num;

	while(master_tf_over == 0)
	{
	    /* Echo received data */
	    if (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		printf("enabling DMAEN bit in master I2C_CR register"); 
		RegWrF(master_num,IBCR,IBCR_DMAEN_DMA_Enable,e_ENABLE);
		master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr1_Write_Transfer);
	    }
	}
	
	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);

#if defined(REGRESSION)
	master_num++;
#endif	
    }
    
#else
    error("NA for this NPI");
#endif
}

void I2C_DMA_Mstr_Rx_test(void)
{
#ifdef EN_I2C_DMA_Mstr_Rx_test
    
    uint8_t temp,rxd_char=0,fail=0,i=0,master_num=0,slave_num=0,err_cnt=0;

    //tx_buffer stores the data that is to be transferred to iPort
    more_master_var = 0;
    check_en = 1;
    DMA_RX=1;
    DMA_TX=0;
    DMA_recv_enb=1;
    
    edma_init();
    
    printf("Starting DMA RX test...\n\n");

    while(1)
    {
#if defined(REGRESSION)
	if(master_num == I2C_INSTANCES) {
	    break;
	}
	master_num = master_num + 0;
	slave_num  = master_num + 1;

	if(slave_num == I2C_INSTANCES) {
	    slave_num = 0;
	}	
#else
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

#endif

	master_tf_over=err_cnt=0;
	
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_rx_buffer.buf[i] = 0;

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	i2c_dma_config(master_num,DMA_TX);
	
	master_dma_inst = master_num;
	
	while(master_tf_over == 0)
	{	    
	    /* Echo received data */
	    if (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) { 
		master_tf_over = i2c_transfer(I2C_RX,master_num,e_Slave_Addr1_Read_Transfer);
	    }
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);

#if defined(REGRESSION)
	master_num++;
#endif		
    }

#else
    error("NA for this NPI");
#endif
}

void I2C_Mstr_Tx_freq_test(void)
{
#ifdef EN_I2C_Mstr_Tx_freq_test
    
    int i;
    uint8_t rxd_char,master_num,slave_num,baud_rate_div,baud_rate_opt,err_cnt=0;

    more_master_var = 0;
    check_en = 1;

    while(1)
    {
	printf("I2C Master Transfer test case in standard and fast mode");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	//Default SYS6_CLK(I2C baud clk) = 133MHz
/*	random_sys_clock();
 - 
 - 	set_int_limits(1,2);
 - 	baud_rate_opt = get_int_with_limit("\n Please select option 1 for standard mode i2c transfer or select option 2 for fast mode");
 - 	switch(baud_rate_opt) {
 - 	    case FREQ_OPT1:
 - 		printf("110");  //~86Khz
 - 		printf("51");
 - 		printf("106");  //~148Khz
 - 		baud_rate_div = get_int("enter I2C clock divider value from above list for standard mode");
 - 		break;
 - 	    case FREQ_OPT2:
 - 		printf("38");  //~346Khz
 - 		printf("149"); //~377Khz
 - 		printf("37");  //~415Khz
 - 		baud_rate_div = get_int("I2C clock divider value from above list for fast mode");
 - 		break;
 - 	    default:
 - 		printf("Invalid option");
 - 	}*/
	
	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");
	
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no");

	i2c_init(e_I2C_MASTER,master_num,37/*baud_rate_div*/,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,106/*baud_rate_div*/,e_ADR_Slave_ADR1);

	//i2c_struct[master_num].I2C_n->IBDBG.B.GLFLT_EN = 1;
	//i2c_struct[slave_num].I2C_n->IBDBG.B.GLFLT_EN = 1;

	while(master_tf_over == 0)
	{
	    /* Echo received data */
	    if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr1_Write_Transfer);
	    }
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);
    }
    
#else
    error("NA for this NPI");
#endif
}

void I2C_Mstr_Rx_freq_test(void)
{
#ifdef EN_I2C_Mstr_Rx_freq_test
    
    int i;
    uint8_t rxd_char,master_num,slave_num,baud_rate_div,baud_rate_opt,err_cnt=0;

    more_master_var = 0;
    check_en = 1;
    
    while(1)
    {
	printf("I2C Master receive test case in standard and fast mode");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;
	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	//Default SYS6_CLK(I2C baud clk) = 133MHz
	random_sys_clock();
	
	set_int_limits(1,2);
	baud_rate_opt = get_int_with_limit("\n Please select option 1 for standard mode i2c transfer or select option 2 for fast mode");
	switch(baud_rate_opt) {
	    case FREQ_OPT1:
		printf("110");  //~86Khz
		printf("51");
		printf("106");  //~148Khz
		baud_rate_div = get_int("enter I2C clock divider value from above list for standard mode");
		break;
	    case FREQ_OPT2:
		printf("38");  //~346Khz
		printf("149"); //~377Khz
		printf("37");  //~415Khz
		baud_rate_div = get_int("I2C clock divider value from above list for fast mode");
		break;
	    default:
		printf("Invalid option");
	}
	
	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");
	
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no 1 ");

	i2c_init(e_I2C_MASTER,master_num,baud_rate_div,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,baud_rate_div,e_ADR_Slave_ADR1);

	while(master_tf_over == 0)
	{
	    /* Echo received data */
	    if (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		master_tf_over = i2c_transfer(I2C_RX,master_num,e_Slave_Addr1_Read_Transfer);
	    }
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);
    }

#else
    error("NA for this NPI");
#endif
}

void I2C_Dbg_Mode_Before_Add_Tx_Master_test()
{
#ifdef EN_I2C_Dbg_Mode_Before_Add_Tx_test
    
     uint32_t temp,rxd_char=0,i,I2C_in_Debug_Mode,master_num;

     while(1)
     {
	printf("I2C debug mode master test case.... ");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	set_int_limits(0,1);
	I2C_in_Debug_Mode = get_int_with_limit("\n Please select 1 for debug mode 0 for ordinary operation ");
 
	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	RegWrF(master_num,IBCR,IBCR_IBIE_IBUS_Interrupt_Enable,0);	

	// Tx/Rx mode select - configure direction of master/slave transfers. Here, I2C configured for Transmit.
	RegWrF(master_num,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);
	
	set_user_option("enter ok icon for start condition and address tansfer");
  	rxd_char=get_user_option_index("Select Options.");

	RegWrF(master_num,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);

	printf("Instance[%d] as Master Sending device address\n",master_num);

	//Set the slave address to be sent
	// Master sends address of slave (0x62) + Write transfer bit (0 = slave receives data from master)

	RegWrF(master_num,IBDR, IBDR_Data, e_Slave_Addr1_Write_Transfer);

	if(I2C_in_Debug_Mode == 0) {
    
	    while (RegRdF(master_num,IBSR, IBSR_RXAK_Receive_Acknowledge) != e_RXAK_Ack_Received);
	    printf("acknowledge received for Slave address...\n\n");

	    set_user_option("Master:No Ack received in debug mode when ip is enabled in debug mode");
	    set_user_option("Master: Ack received in debug mode when ip is enabled in debug mode");
	    rxd_char=get_user_option_index("Select Options.");
	    if(rxd_char==0) {
		error("I2C communication not happening in debug mode when ip is enabled in debug mode");
	    }
	    else {
		printf("I2C communication  happening in debug mode when ip is enabled in debug mode");
	    }
	}
	else {
	    while (RegRdF(master_num,IBSR, IBSR_RXAK_Receive_Acknowledge) != e_RXAK_No_Ack_Received);
	    printf("no acknowledge received for Slave address...\n\n");

	    set_user_option("Master:No Ack received in debug mode");
	    set_user_option("Master: Ack received in debug mode");
	    rxd_char=get_user_option_index("Select Options.");
	    if(rxd_char==0) {
		printf("I2C communication not happening in debug mode when ip is disabled in debug mode");
	    }
	    else {
		error("I2C communication happening in debug mode when ip is disabled in debug mode");
	    }
	    goto END;
	}
	
	// Data sent from master to slave.
	RegWrF(master_num,IBDR,IBDR_Data,e_Data_1_from_Master);
	
	while (RegRdF(master_num,IBSR, IBSR_RXAK_Receive_Acknowledge) != e_RXAK_Ack_Received)
	{
	    asm("nop");
	}

	printf("Slave acknowledge received for 0x%x...\n\n",e_Data_1_from_Master);

	
	printf("Master sends stop signal...");
	if(RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy) == TRUE)
	{
	    printf("Bus is already busy... Sending STOP\n\n");

	    //I2C Bus module is enabled.
	    RegWrF(master_num,IBCR, IBCR_MDIS_Module_Disable_bit,e_MDIS_Module_Enabled);

	    // I2C module configured in Master mode and enabled.
	    RegWrF(master_num,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);//changing to slave

	    RegWrF(master_num,IBCR, IBCR_MDIS_Module_Disable_bit,e_MDIS_Reset_n_Disabled);

	    for(i=0;i<0x1fff;i++); //wait for a while...
	}

END:
	printf("I2C debug mode test performed at master side");

     }

#else
    error("NA for this NPI");
#endif     
}

void I2C_Dbg_Mode_Before_Add_Tx_Slave_test()
{
#ifdef EN_I2C_Dbg_Mode_Before_Add_Tx_test
    
     uint32_t temp,rxd_char=0,i,I2C_in_Debug_Mode,slave_num;

     while(1)
     {
	printf("I2C debug mode slave test case.... ");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	set_int_limits(0,1);
	I2C_in_Debug_Mode = get_int_with_limit("\n Please select 1 for debug mode 0 for ordinary operation ");
 
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

	i2c_init(e_I2C_MASTER,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	RegWrF(slave_num,IBCR,IBCR_IBIE_IBUS_Interrupt_Enable,0);
	
	if(I2C_in_Debug_Mode == 0) {
	    printf("I2C debug mode when ip is enabled in debug mode");
	}
	else {
	    while(RegRdF(slave_num,IBSR,IBSR_IBB_Bus_Busy) == TRUE) {
		asm("nop");
	    }

	    WRITE(i2c_struct[slave_num].I2C_n->IBDBG.R , 1);
     
	    //while(!(READ(i2c_struct[slave_num].I2C_n->IBDBG.R)&0x02));
	}
	
        // Read the data register to know if address sent by Master is received.
	// set break point here
	temp = RegRd(slave_num,IBDR);

	if(I2C_in_Debug_Mode == 0) {
	    while(temp != e_Slave_Addr1_Write_Transfer)
	    {
		temp = RegRd(slave_num,IBDR);
	    }
	}
	else {
	    i=0;
	    while((temp != e_Slave_Addr1_Write_Transfer)&&(i++<=10))
	    {
		temp = RegRd(slave_num,IBDR);
	    }
	    if(i==12)
	    {
		printf("No Address received by slave in debug mode when ip is disabled in debug mode");
	    }
	    else{
		error("Address received by slave in debug mode when ip is disabled in debug mode");
	    }
		
	    i=0;
	    goto END;
	}
		 
	printf("Address received by slave is = %x",temp);

	if (RegRdF(slave_num,IBSR,IBSR_SRW_Slave_Read_Write_Mode)!= e_SRW_Slave_Receive)
	    error("ERROR: Slave Receive status bit NOT set in IBSR.\n");
	
	temp = RegRd(slave_num,IBDR);
	while(temp != e_Data_1_from_Master)
	{
	    temp = RegRd(slave_num,IBDR);
	}
	if(temp == e_Data_1_from_Master)
	{
	    printf("Data received by slave = 0x%x",e_Data_1_from_Master);
	}

END:
	printf("I2C debug test performed at slave side");
	WRITE(i2c_struct[slave_num].I2C_n->IBDBG.R , 0);
	
     }

#else
    error("NA for this NPI");
#endif
}

void I2C_Debug_Mode_During_Trans_Master_test()
{
#ifdef EN_I2C_Debug_Mode_During_Trans_test
    
     uint8_t i,rxd_char,master_num,slave_select;

    more_master_var = 0;
    check_en = 1;

    while(1)
    {
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	
	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR2);

	while(master_tf_over == 0)
	{ 
	    if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr2_Write_Transfer);
	    }
	}
	
    }
    printf("I2C debug mode test performed at master side");

#else
    error("NA for this NPI");
#endif
}

void I2C_Debug_Mode_During_Trans_Slave_test()
{
#ifdef EN_I2C_Debug_Mode_During_Trans_test
    
    uint8_t i,rxd_char,slave_num,slave_select;

    more_master_var = 0;

    while(1)
    {
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no 2 ");

	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR2);
	
	printf("Disable I2C in Debug mode");
	WRITE(i2c_struct[slave_num].I2C_n->IBDBG.R , 1);

	step("after 3 bytes received enter into debug mode");
	step("Check in dbg window this wont halt reception of next bytes and module will enter into debug mode only after received all the data");
	
	while (!slave_rx_done)
	{
	    if(i2c_rx_buffer.rx_index == 3) { //Set break point inside if condition
		asm("nop");         
		asm("nop");
	    }	    
	}
	slave_rx_done = FALSE;

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]== i2c_rx_buffer.buf[i]) {
		printf("Data=0x%x transfered successfully",i2c_rx_buffer.buf[i]);
	    }
	    else{
		error("Data=0x%x transfer failed",i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
    }

#else
    error("NA for this NPI");
#endif
}

//1. Enter Dbg mode in between i2c transfer.
//2. Check in dbg mem window for tx side that i2c is entered into dbg mode after current byte transfer
//   by checking IPG_DEBUG_HALTED, Open rx side window also to receive remaining data.
//3. Clear IBDBG.R to 0, to ensure i2x tx is now not in debug mode and dma is doing tf to tx data reg,
//   So rx also able to receive data by checking i2c memory in dbg window.
//4. Rerun the test case for logging.

void I2C_DMA_Debug_Mode_test(void)
{
#ifdef EN_I2C_DMA_Debug_Mode_test
    
    uint8_t temp,rxd_char=0,fail=0,i=0,master_num,slave_num,channel_num;
    uint8_t I2C_DbgEnDis,dbg_en;
            
    // tx_buffer stores the data that is to be transferred to iPort
    more_master_var = 0;
    DMA_recv_enb=0;
    DMA_RX=0;
    DMA_TX=1;
    
    edma_init();
    
    printf("Starting DMA_Debug_Mode_test");

    while(1)
    {
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	set_user_option("En/Dis module based on frz bit of ip");
	set_user_option("Enable module in Dbg mode");

	dbg_en=get_user_option_index("Select Options.");

#if defined CORE_M4_ENABLE
	if(dbg_en==0) {
	    *(uint32_t *)0x40091008 &= ~(0x1);
	}
	else {
	    *(uint32_t *)0x40091008 = 0x1;
	}
#elif defined CORE_A530
	if(dbg_en==0) {
	    *(uint32_t *)0x40091008 &= ~(0x2);
	}
	else {
	    *(uint32_t *)0x40091008 = 0x2;
	}
#elif defined CORE_A531
	if(dbg_en==0) {
	    *(uint32_t *)0x40091008 &= ~(0x4);
	}
	else {
	    *(uint32_t *)0x40091008 = 0x4;
	}
#elif defined CORE_A532
	if(dbg_en==0) {
	    *(uint32_t *)0x40091008 &= ~(0x8);
	}
	else {
	    *(uint32_t *)0x40091008 = 0x8;
	}
#elif defined CORE_A533
	if(dbg_en==0) {
	    *(uint32_t *)0x40091008 &= ~(0x10);
	}
	else {
	    *(uint32_t *)0x40091008 = 0x10;
	}
#endif

	set_user_option("I2C enable in dbg mode");
	set_user_option("I2C Disable in dbg mode");

	I2C_DbgEnDis=get_user_option_index("Select Options.");

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");
	
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no 1 ");

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	i2c_dma_config(master_num,DMA_TX);
	
	master_dma_inst = master_num;

	printf("Disable I2C in Debug mode");
	WRITE(i2c_struct[master_num].I2C_n->IBDBG.R , I2C_DbgEnDis);
	
	RegWrF(master_num,IBCR,IBCR_DMAEN_DMA_Enable,e_ENABLE);

	master_mode = I2C_TX;

	while (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Busy);
	     /* Put module in master TX mode (generates START) */
	RegWrF(master_num,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);
	RegWrF(master_num,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);
	
	RegWrF(master_num,IBDR, IBDR_Data,e_Slave_Addr1_Write_Transfer);

	while (!slave_rx_done){
	    if(i2c_rx_buffer.rx_index == 3) { 
		asm("nop");         
		asm("nop");
		//WRITE(i2c_struct[master_num].I2C_n->IBDBG.R , 0);
	    }
	}
	slave_rx_done = FALSE;

	printf("Send Stop signal");
	RegWrF(master_num,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]== i2c_rx_buffer.buf[i]) {
		printf("Data=0x%x transfered successfully",i2c_rx_buffer.buf[i]);
	    }
	    else{
		error("Data=0x%x transfer failed",i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
    }

#else
    error("NA for this NPI");
#endif
}

void I2C_Stop_Mode_Master_test()
{
#ifdef EN_I2C_Stop_Mode_test
    
    uint8_t rxd_char,dummy_read,master_num;
    uint32_t temp,i;
  
      printf("I2C Low-Power mode test started !!! \n");

      while(1)
      {
	set_user_option("Test Continue..");
	set_user_option("Test Done..");
	rxd_char=get_user_option_index("Select Options.");

	printf("I2C Master Transfer test case");
	if(rxd_char==1)
	    break;

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");

	printf("pin muxing configuration for master device and instance num is %d",master_num);
	i2c_pin_muxing(master_num);

	i2c_reg_reset(master_num);

	printf("MASTER:Registering i2c%d ISR",master_num); 
	if (master_num == 0) {
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_0,&i2c0_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_0,&i2c0_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}
	else if(master_num == 1){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_1,&i2c1_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_1,&i2c1_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}

#if I2C_INSTANCES == 3 || I2C_INSTANCES == 4
	else if(master_num == 2){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_2,&i2c2_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_2,&i2c2_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}
#endif //I2C_INSTANCES == 3 || I2C_INSTANCES == 4

#if I2C_INSTANCES == 4
	else if(master_num == 3){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_3,&i2c3_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_3,&i2c3_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}
#endif //I2C_INSTANCES == 4

	
	/***** Initialize I2C Module *****/
	printf("MASTER:Configuring I2C %d registers",master_num);
	RegWrF(master_num,IBFD,IBFD_IBC_Bus_Clk_Rate,BAUD_RATE_DIV);   
	RegWrF(master_num,IBCR,IBCR_IBIE_IBUS_Interrupt_Enable,1);
	RegWrF(master_num,IBCR,IBCR_MDIS_Module_Disable_bit,e_MDIS_Module_Enabled);
	
	// Tx/Rx mode select - configure direction of master/slave transfers. Here, I2C configured for Transmit.
	RegWrF(master_num,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);

	RegWrF(master_num,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);

	printf("Instance[%d] as Master Sending device address\n",master_num);

	//Set the slave address to be sent
	// Master sends address of slave (0x62) + Write transfer bit (0 = slave receives data from master)

	RegWrF(master_num,IBDR, IBDR_Data, e_Slave_Addr1_Write_Transfer);
	while(stop!=1) {
	    asm("nop");
	}
	stop=0;
 	    
	while (RegRdF(master_num,IBSR, IBSR_RXAK_Receive_Acknowledge) != e_RXAK_Ack_Received);
	printf("Slave address acknowledge received...\n\n");

	set_user_option("enter ok icon for data1 tansfer");
	rxd_char=get_user_option_index("Select Options.");
	
 	/////////////////////////////////////////////Data transfer///////////////
	printf("sending data1 from master");
	RegWrF(master_num,IBDR , IBDR_Data,e_Data_1_from_Master);

	while(stop!=1) {
	    asm("nop");
	}
	stop=0;

	while (RegRdF(master_num,IBSR, IBSR_RXAK_Receive_Acknowledge) != e_RXAK_Ack_Received);
	printf("Slave data1 acknowledge received by master..\n\n");
		
	set_user_option("enter ok icon to send stop condition");
	rxd_char=get_user_option_index("Select Options.");
	printf("sending stop signal");
	RegWrF(master_num,IBCR,IBCR_MS_SLn_Master_Slave_Mode_Select,e_MS_SLn_Slave);
	
     }

#else
    error("NA for this NPI");
#endif 
}


void I2C_Stop_Mode_Slave_test()
{
#ifdef EN_I2C_Stop_Mode_test
    
     uint8_t rxd_char,slave_num;
     uint32_t temp,i;
  
     printf("I2C Low-Power mode test started !!! \n");

     MC_ME_INIT();
       
     while(1)
     {
	 set_user_option("Test Continue..");
	 set_user_option("Test Done..");
	 rxd_char=get_user_option_index("Select Options.");

	 printf("I2C Slave Transfer test case");

	 if(rxd_char==1)
	     break;

	 set_int_limits(0,I2C_INSTANCES-1);
	 slave_num = get_int_with_limit("\n Please select the slave instance no 1 ");

	 printf("pin muxing configuration for slave device and instance num is %d",slave_num);
	 i2c_pin_muxing(slave_num);

	 i2c_reg_reset(slave_num);

	printf("SLAVE:Registering i2c%d ISR",slave_num); 
	if (slave_num == 0) {
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_0,&i2c0_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_0,&i2c0_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}
	else if(slave_num == 1){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_1,&i2c1_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_1,&i2c1_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}

#if I2C_INSTANCES == 3 || I2C_INSTANCES == 4
	else if(slave_num == 2){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_2,&i2c2_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_2,&i2c2_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}
#endif //I2C_INSTANCES == 3 || I2C_INSTANCES == 4

#if I2C_INSTANCES == 4
	else if(slave_num == 3){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_3,&i2c3_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_3,&i2c3_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}
#endif //I2C_INSTANCES == 4
	
	 printf("SLAVE:Configuring I2C %d registers",slave_num);
	 RegWrF(slave_num,IBFD,IBFD_IBC_Bus_Clk_Rate,BAUD_RATE_DIV);
	 RegWrF(slave_num,IBAD, IBAD_ADR_Slave_Address, e_ADR_Slave_ADR1);
	 RegWrF(slave_num,IBCR,IBCR_IBIE_IBUS_Interrupt_Enable,1);
	 RegWrF(slave_num,IBCR,IBCR_MDIS_Module_Disable_bit, e_MDIS_Module_Enabled);

	 while(stop!=1) {
	     asm("nop");
	 }
	 stop=0;
	 temp = RegRd(slave_num,IBDR);

	 while(temp != e_Slave_Addr1_Write_Transfer)
	 {
	     temp = RegRd(slave_num,IBDR);
	 }

	 printf("Address received by slave is = %x",temp);

	 if (RegRdF(slave_num,IBSR,IBSR_SRW_Slave_Read_Write_Mode)!= e_SRW_Slave_Receive)
	     error("ERROR: Slave Receive status bit NOT set in IBSR.\n");

	 printf("entering into stop mode");
	 if(slave_num==0) {
	     stop_mode_entry(PCTL_I2C_0,1);
	 }
	 else {
	     stop_mode_entry(PCTL_I2C_1,1);
	 }

	 while(stop!=1) {
	     asm("nop");
	 }
	 stop=0;
	 temp = RegRd(slave_num,IBDR);

	 while(temp != e_Data_1_from_Master)
	 {
	     temp = RegRd(slave_num,IBDR);
	 }
	 printf("data received by slave is 0x%x",temp);
     }
#else
    error("NA for this NPI");
#endif 
}

void I2C_Halt_Mode_Master_test()
{
#ifdef EN_I2C_Halt_Mode_test
    
    uint8_t rxd_char,dummy_read,master_num;
    uint32_t temp,i;
  
      printf("I2C Low-Power mode test started");

      while(1)
      {
	set_user_option("Test Continue..");
	set_user_option("Test Done..");
	rxd_char=get_user_option_index("Select Options.");

	printf("I2C Master Transfer test case");
	if(rxd_char==1)
	    break;

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");

	printf("pin muxing configuration for master device and instance num is %d",master_num);
	i2c_pin_muxing(master_num);

	i2c_reg_reset(master_num);

	/*ISR registeration*/
	printf("MASTER:Registering i2c%d ISR",master_num);
	if (master_num == 0) {
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_0,&i2c0_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_0,&i2c0_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}
	else if(master_num == 1){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_1,&i2c1_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_1,&i2c1_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}

#if I2C_INSTANCES == 3 || I2C_INSTANCES == 4
	else if(master_num == 2){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_2,&i2c2_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_2,&i2c2_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}
#endif //I2C_INSTANCES == 3 || I2C_INSTANCES == 4

#if I2C_INSTANCES == 4
	else if(master_num == 3){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_3,&i2c3_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_3,&i2c3_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}
#endif //I2C_INSTANCES == 4

	/***** Initialize I2C Module *****/
	printf("MASTER:Configuring I2C %d registers",master_num);
	RegWrF(master_num,IBFD,IBFD_IBC_Bus_Clk_Rate,BAUD_RATE_DIV);   
	RegWrF(master_num,IBCR,IBCR_IBIE_IBUS_Interrupt_Enable,1);
	RegWrF(master_num,IBCR,IBCR_MDIS_Module_Disable_bit,e_MDIS_Module_Enabled);
	
	// Tx/Rx mode select - configure direction of master/slave transfers. Here, I2C configured for Transmit.
	RegWrF(master_num,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);

	RegWrF(master_num,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);

	printf("Instance[%d] as Master Sending device address\n",master_num);

	//Set the slave address to be sent
	// Master sends address of slave (0x62) + Write transfer bit (0 = slave receives data from master)

	RegWrF(master_num,IBDR, IBDR_Data, e_Slave_Addr1_Write_Transfer);
	while(stop!=1) {
	    asm("nop");
	}
	stop=0;
 	    
	while (RegRdF(master_num,IBSR, IBSR_RXAK_Receive_Acknowledge) != e_RXAK_Ack_Received);
	printf("Slave address acknowledge received...\n\n");

	set_user_option("enter ok icon for data1 tansfer");
	rxd_char=get_user_option_index("Select Options.");
	
 	/////////////////////////////////////////////Data transfer///////////////
	printf("sending data1 from master");
	RegWrF(master_num,IBDR , IBDR_Data,e_Data_1_from_Master);

	while(stop!=1) {
	    asm("nop");
	}
	stop=0;

	while (RegRdF(master_num,IBSR, IBSR_RXAK_Receive_Acknowledge) != e_RXAK_Ack_Received);
	printf("Slave data1 acknowledge received by master..\n\n");
		
	set_user_option("enter ok icon to send stop condition");
	rxd_char=get_user_option_index("Select Options.");
	printf("sending stop signal");
	RegWrF(master_num,IBCR,IBCR_MS_SLn_Master_Slave_Mode_Select,e_MS_SLn_Slave);
	
     }

#else
    error("NA for this NPI");
#endif 
}


void I2C_Halt_Mode_Slave_test()
{
#ifdef EN_I2C_Halt_Mode_test
    
     uint8_t rxd_char,slave_num;
     uint32_t temp,i;
  
     printf("I2C Low-Power mode test started !!! \n");

     MC_ME_INIT();
       
     while(1)
     {
	 set_user_option("Test Continue..");
	 set_user_option("Test Done..");
	 rxd_char=get_user_option_index("Select Options.");

	 printf("I2C Slave Transfer test case");

	 if(rxd_char==1)
	     break;

	 set_int_limits(0,I2C_INSTANCES-1);
	 slave_num = get_int_with_limit("\n Please select the slave instance no 1 ");

	 printf("pin muxing configuration for slave device and instance num is %d",slave_num);
	 i2c_pin_muxing(slave_num);

	 i2c_reg_reset(slave_num);

	 printf("SLAVE:Registering i2c%d ISR",slave_num);
	 if (slave_num == 0) {
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_0,&i2c0_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_0,&i2c0_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}
	else if(slave_num == 1){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_1,&i2c1_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_1,&i2c1_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}

#if I2C_INSTANCES == 3 || I2C_INSTANCES == 4
	else if(slave_num == 2){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_2,&i2c2_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_2,&i2c2_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}
#endif //I2C_INSTANCES == 3 || I2C_INSTANCES == 4

#if I2C_INSTANCES == 4
	else if(slave_num == 3){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C_3,&i2c3_isr,PRIORITY_3,CORE_CM4_0);
#endif
#ifdef CORE_A5_ENABLE
	    intc_install_interrupt(INT_I2C_3,&i2c3_isr,PRIORITY_3,CORE_CA5_0);
#endif
	}
#endif //I2C_INSTANCES == 4

	 printf("SLAVE:Configuring I2C %d registers",slave_num);
	 RegWrF(slave_num,IBFD,IBFD_IBC_Bus_Clk_Rate,BAUD_RATE_DIV);
	 RegWrF(slave_num,IBAD, IBAD_ADR_Slave_Address, e_ADR_Slave_ADR1);
	 RegWrF(slave_num,IBCR,IBCR_IBIE_IBUS_Interrupt_Enable,1);
	 RegWrF(slave_num,IBCR,IBCR_MDIS_Module_Disable_bit, e_MDIS_Module_Enabled);

	 while(stop!=1) {
	     asm("nop");
	 }
	 stop=0;
	 temp = RegRd(slave_num,IBDR);

	 while(temp != e_Slave_Addr1_Write_Transfer)
	 {
	     temp = RegRd(slave_num,IBDR);
	 }

	 printf("Address received by slave is = %x",temp);

	 if (RegRdF(slave_num,IBSR,IBSR_SRW_Slave_Read_Write_Mode)!= e_SRW_Slave_Receive)
	     error("ERROR: Slave Receive status bit NOT set in IBSR.\n");

	 printf("entering into stop mode");
	 if(slave_num==0) {
	     halt_mode_entry(PCTL_I2C_0,1);
	 }
	 else {
	     halt_mode_entry(PCTL_I2C_1,1);
	 }

	 while(stop!=1) {
	     asm("nop");
	 }
	 stop=0;
	 temp = RegRd(slave_num,IBDR);

	 while(temp != e_Data_1_from_Master)
	 {
	     temp = RegRd(slave_num,IBDR);
	 }
	 printf("data received by slave is 0x%x",temp);
     }
#else
    error("NA for this NPI");
#endif 
}

/***This test case tests in each mode
  if enabling and disabling of a peripheralis possible
  and read write is possible to the registers in case peripheral is eanbled
  but read/write is not possible and generates bus error in case peripheral is disabled****/

void I2C_Enable_Disable_test()
{
#ifdef EN_I2C_Enable_Disable_test
    
    uint8_t runPC=1,inst_num;
    int inst=0,pctl_no;
    mode_t mode;
    mode_config_t mode_config;
    mode_config_sec_cc_t mode_config_sec_cc; 
    int i;
    uint8_t temp;
    
    MC_ME_INIT();
    
#if defined CORE_M4_ENABLE
    intc_install_exception(BUS_FAULT,MC_ME_read_write_err_isr);
#elif defined CORE_A5_ENABLE
    intc_install_exception(DATA_ABORT,MC_ME_read_write_err_isr);
#endif

    for(inst_num=0;inst_num < I2C_INSTANCES;inst_num++) {
	
	//use this flag to enable/disable random mode entry happening inside SWT ISR.
	enableRandomModeEntry=0;

	// Enable Interrupts
	r_err_count=0;
	w_err_count=0;

	mode_config.r= READ_INDEX(inst, mc_me_struct[inst].mc_me->DRUN_MC.R);
	mode_config_sec_cc.r = READ(mc_me_struct[MC_ME_INSTANCE].mc_me->DRUN_SEC_CC_I.R);
	    
	MC_ME_ConfigureMode(inst,MODE_RUN0,mode_config,mode_config_sec_cc);
	MC_ME_ConfigureMode(inst,MODE_RUN1,mode_config,mode_config_sec_cc);
	MC_ME_ConfigureMode(inst,MODE_RUN2,mode_config,mode_config_sec_cc);
	MC_ME_ConfigureMode(inst,MODE_RUN3,mode_config,mode_config_sec_cc);

	/******FOR all modes other than SAFE mode***********/
	for(mode=MODE_RUN0; mode<=MODE_RUN3; mode++)
	{
	    /* Skipping SAFE mode from this loop, because SAFE mode entry requires all modules running on
	       any clock other than IRC, to be switched OFF; Or to be switched to IRC.
	       Therefore, for SAFE mode entry, sysClk and clock of module under test, will have to be configured
	       on IRC before mode switch.
	       It will be taken care of, later. */
	    if(mode != MODE_SAFE)
	    {
		printf("Cheking read/write access in %d", mode);
		//Skipping RUN_PC0 and RUN_PC1, as it is being used for all other module configurations in different APIs.
		for(runPC=2; runPC<=7; runPC++)
		{
		    //initiallize the error counters
		    r_err_count=0;
		    w_err_count=0;
			    
		    if(EnablePeripheralInMode(0, runPC, i2c_struct[inst_num].pctl_no, mode)== ENABLE_PERI_FAIL)
			error("Peripheral could not be enabled");
		    else
		    {
			if(inst_num == 0) {
			    if(MC_ME.PS2.B.S_IIC0 == 1)
				printf("peripheral enabled.");
			}
			else if(inst_num == 1) {
			    if(MC_ME.PS5.B.S_IIC1 == 1)
				printf("peripheral enabled.");
			}
			else if(inst_num == 2) {
			    if(MC_ME.PS5.B.S_IIC2 == 1)
				printf("peripheral enabled.");
			}
			
			i2c_struct[inst_num].I2C_n->IBAD.R= 0x46;
			temp = i2c_struct[inst_num].I2C_n->IBAD.R;
			
			if(temp == 0x46)
			    step("register read write access done correctly");
			else
			    error("register read write access FAILED.");

			if( (r_err_count==0) && (w_err_count==0) )
			    printf("PASS: With peripheral enabled in %d mode, BUS ERROR NOT generated.", mode);
			else
			    error("FAIL: With peripheral enabled in %d mode, BUS ERROR is generated.", mode);

			i2c_struct[inst_num].I2C_n->IBAD.R=0x00;  //default value
		    }

		    if(DisablePeripheralInMode(0, runPC, i2c_struct[inst_num].pctl_no, mode)== DISABLE_PERI_FAIL)	// Disable peripheral for commanded mode
			error("Peripheral could not be disabled");
		    else
		    {
			if(inst_num == 0) {
			    if(MC_ME.PS2.B.S_IIC0 == 0)
				printf("peripheral disabled.");
			}
			else if(inst_num == 1) {
			    if(MC_ME.PS5.B.S_IIC1 == 0)
				printf("peripheral disabled.");
			}
			else if(inst_num == 2) {
			    if(MC_ME.PS5.B.S_IIC2 == 0)
				printf("peripheral disabled.");
			}

			//Access I2C registers and check if bus error occurs.
			i2c_struct[inst_num].I2C_n->IBAD.R= 0x10;
			temp = i2c_struct[inst_num].I2C_n->IBAD.R;
			if(temp == 0x10)
			    error("register read write done correctly, when module is disabled");
			else
			    step("register read write did not work, as expected.");

			if( (r_err_count==1) || (w_err_count==1) )
			{
			    if((r_err_count==1) && (w_err_count==1))  //if both errors occurred
				printf("PASS: With peripheral DISABLED in %d mode, BUS ERROR is generated, as expected.", mode);
			    else if (r_err_count==1)  //if only read access generated error
				error("FAIL: With peripheral DISABLED in %d mode, only READ generates BUS ERROR, as expected.", mode);
			    else if (w_err_count==1)  //if only write access generated error
				error("FAIL: With peripheral DISABLED in %d mode, only WRITE generates BUS ERROR, as expected.", mode);
			}
			else  //if no access generated error
			    error("FAIL: With peripheral DISABLED in %d mode, R/W access does not generate BUS ERROR!! Unexpected.", mode);
		    }
		}
	    }
	}
#if 0
	/*************        FOR SAFE mode   ***********/
	mode = MODE_SAFE;
	disable_peripheral_scsrc(0);

	printf("Cheking read/write access in %d", mode);
	//Skipping RUN_PC0 and RUN_PC1, as it is being used for all other module configurations in different APIs.
	for(runPC=2; runPC<=7; runPC++)
	{
	    //initiallize the error counters
	    r_err_count=0;
	    w_err_count=0;

	    //TO be done by module owner: before switching to SAFE mode, do switch your module to iRC, if IRC is a source clock to your module.
	    //else, only check the "DisablePeripheralInMode" part
	    //cgm_init_clk_div(PER_CLK,IRC_CLK, 0, 0); //clkname,clk_src,divider,fmt_div

	    ///MC_ME.DRUN_MC.R = 0x001300F0; /* Enable XOSC and PLLs - IRC is sysclk */
	    /* RE  enter the drun mode, to update the configuration */
	    MC_ME.MCTL.R = 0x30005AF0;     	         /* Mode & Key */
	    MC_ME.MCTL.R = 0x3000A50F;                /* Mode & Key inverted */
	    while(MC_ME.GS.B.S_MTRANS == 1);		         /* Wait for mode entry to complete */
	    while(MC_ME.GS.B.S_CURRENT_MODE != 0x3);       /* Check DRUN mode has been entered */
	    //while(MC_ME.GS.B.S_SYSCLK != 0x0);

	    //sci_init(cgm_get_clk_freq_in_Khz(LINFLEX_CLK));

	    if(EnablePeripheralInMode(0, runPC, PCTL_I2C_0+instance_num, mode)== ENABLE_PERI_FAIL)
		error("Peripheral could not be enabled");
	    else
	    {
		lin_setting();
		if(instance_num == 0) {
		    if(MC_ME.PS0.B.S_I2C_0 == 1)
			printf("peripheral enabled.");
		}
		else if(instance_num == 1) {
		    if(MC_ME.PS0.B.S_I2C_1 == 1)
			printf("peripheral enabled.");
		}	    

		i2c_struct[instance_num].I2C_n->IBAD.R= 0x46;
		temp = i2c_struct[instance_num].I2C_n->IBAD.R;
		if(temp == 0x46)
		    step("register read write access done correctly");
		else
		    error("register read write access FAILED.");


		if( (r_err_count==0) && (w_err_count==0) )
		    printf("PASS: With peripheral enabled in %d mode, BUS ERROR NOT generated.", mode);

		else
		    error("FAIL: With peripheral enabled in %d mode, BUS ERROR is generated.", mode);

		i2c_struct[instance_num].I2C_n->IBAD.R=0x00;  //default value
	    }

	    //TO be done by module owner: switch your module back to normal clock
	    //cgm_init_clk_div(PER_CLK,PLL0_CLK, 4, 0); //clkname,clk_src,divider,fmt_div

	    if(DisablePeripheralInMode(0, runPC, PCTL_I2C_0+instance_num, mode)== DISABLE_PERI_FAIL)	// Disable peripheral for commanded mode
		error("Peripheral could not be disabled");
	    else
	    {
		if(instance_num == 0) {
		    if(MC_ME.PS0.B.S_I2C_0 == 0)
			printf("peripheral disabled.");
		}
		else if(instance_num == 1) {
		    if(MC_ME.PS0.B.S_I2C_1 == 0)
			printf("peripheral disabled.");
		}

		//Access I2C registers and check if bus error occurs.
		i2c_struct[instance_num].I2C_n->IBAD.R= 0x10;
		temp = i2c_struct[instance_num].I2C_n->IBAD.R;
		if(temp == 0x10)
		    error("register read write done correctly, when module is disabled");
		else
		    step("register read write did not work, as expected.");

		if( (r_err_count==1) || (w_err_count==1) )
		{
		    if((r_err_count==1) && (w_err_count==1))  //if both errors occurred
			printf("PASS: With peripheral DISABLED in %d mode, BUS ERROR is generated, as expected.", mode);
		    else if (r_err_count==1)  //if only read access generated error
			error("FAIL: With peripheral DISABLED in %d mode, only READ generates BUS ERROR, as expected.", mode);
		    else if (w_err_count==1)  //if only write access generated error
			error("FAIL: With peripheral DISABLED in %d mode, only WRITE generates BUS ERROR, as expected.", mode);
		}
		else  //if no access generated error
		    error("FAIL: With peripheral DISABLED in %d mode, R/W access does not generate BUS ERROR!! Unexpected.", mode);
	    }
	}
#endif
	//Peripheral will get back to its original state
	if(EnablePeripheralInMode(0,2, i2c_struct[inst_num].pctl_no, MODE_DRUN)== ENABLE_PERI_FAIL)
	    error("Peripheral could not be enabled in DRUN mode");

	enableRandomModeEntry=1;
    }

#else
    error("NA for this NPI");
#endif
}

void I2C_Multi_Core_test(void)
{
#ifdef EN_I2C_Multi_Core_test
    
    int i;
    uint8_t rxd_char,master,master_num=0,slave_num=0,err_cnt=0;

    more_master_var = 0;
    check_en = 1;

    printf("I2C Master Transfer test case");

    while(1)
    {

#if defined(REGRESSION)
	if(master_num == I2C_INSTANCES) {
	    break;
	}
	master_num = master_num + 0;
	slave_num  = master_num + 1;
	
	if(slave_num == I2C_INSTANCES) {
	    slave_num = 0;
	}	
#else
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;
	
	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");
	
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

#endif

	master_tf_over=err_cnt=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	while(master_tf_over == 0)
	{
	    /* Echo received data */
	    if (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr1_Write_Transfer);
	    }
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);	

#if defined(REGRESSION)
	master_num++;
#endif
    }

#else
    error("NA for this NPI");
#endif
}

#ifdef EN_I2C_External_Memory_Access_test
/*This code can write only first 64KB of data out of 128KB of flash because P is always 0 in device addrs*/
/*This code uses only I2C0 for code programming. Other instances are not used */
void i2c_eeprom(uint8_t *image_addr, uint32_t size)
{
    uint8_t tempc[256],temp;
    uint16_t j,addr=0;
    uint8_t addr_msb=0,addr_lsb=0;
    uint32_t data_written=0, wait,err_cnt=0;

    i2c_init(e_I2C_MASTER,0,0x2B,e_ADR_Slave_ADR1);
    RegWrF(0,IBCR,IBCR_IBIE_IBUS_Interrupt_Enable,0);

    for(data_written; data_written < size; data_written += 0x100, image_addr += 0x100, addr += 0x100)
    {  
	i2c_reg_reset(0);
	I2C_0.IBSR.R = 0xff;
	while(I2C_0.IBSR.B.IBB == 0x1);	//Wait for the bus to become free
	I2C_0.IBFD.R = 0x2B;
	I2C_0.IBSR.B.IBIF = 0x1;		//clear IBIF flag
	I2C_0.IBCR.R = 0x10;
	I2C_0.IBCR.B.MSSL = 0x1;		//I2C1 is a master & generates 'start'
	I2C_0.IBDR.R = 0xA0;			//Slave address write i.e. 0xA0

	while(I2C_0.IBSR.B.TCF != 0);
	while(I2C_0.IBSR.B.TCF != 1);
	while(I2C_0.IBSR.B.RXAK == 1); 	//wait for slave Ack

	printf("address tx successfully");
	addr_lsb = (uint8)addr;
	addr_msb = (uint8)(addr>>8);

	I2C_0.IBSR.R = 0xff;
	I2C_0.IBDR.R = addr_msb;

	while(I2C_0.IBSR.B.TCF != 0);
	while(I2C_0.IBSR.B.TCF != 1);
	while(I2C_0.IBSR.B.IBIF != 1);
	while(I2C_0.IBSR.B.RXAK == 1);

	I2C_0.IBSR.R = 0xff;
	I2C_0.IBDR.R = addr_lsb;

	while(I2C_0.IBSR.B.TCF != 0);
	while(I2C_0.IBSR.B.TCF != 1);
	while(I2C_0.IBSR.B.IBIF != 1);
	while(I2C_0.IBSR.B.RXAK == 1);

	printf("flash address tx successfully");
	
	for(j=0;j<256;j++)
	{
	    I2C_0.IBSR.R = 0xff;
	    I2C_0.IBDR.R = *(image_addr + j);		// Transmit data

	    while(I2C_0.IBSR.B.TCF != 0);
	    while(I2C_0.IBSR.B.TCF != 1);
	    while(I2C_0.IBSR.B.IBIF != 1);
	    while(I2C_0.IBSR.B.RXAK == 1);
	}
	I2C_0.IBCR.B.MSSL = 0;               //Sending stop signal
	for(wait=0;wait<200000;wait++);       // wait for some duration after ganarating stop before next start

	printf("flash data tx successfully");
//########## Read from I2C EEPROM ##############
	I2C_0.IBSR.R = 0xff;                   //Clear status
	i2c_reg_reset(0);
	while(I2C_0.IBSR.B.IBB == 0x1);	    //Wait for the bus to become free

	I2C_0.IBFD.R = 0x2B;
	I2C_0.IBSR.B.IBIF = 0x1;		    //clear IBIF flag
	I2C_0.IBCR.R = 0x10;
	I2C_0.IBCR.B.MSSL = 0x1;		    //I2C_0 is a master & generates 'start'
	I2C_0.IBDR.R = 0xA0;	                    //Slave address write i.e. 0xA0

	while(I2C_0.IBSR.B.IBB != 1);            //wait for the bus to become engaged
	while(I2C_0.IBSR.B.IBIF != 1);           //Wait for transfer completion
	while(I2C_0.IBSR.B.RXAK == 1); 	    //wait for slave Ack

	I2C_0.IBSR.R = 0xff;
	I2C_0.IBDR.R = addr_msb;

	while(I2C_0.IBSR.B.TCF != 0);
	while(I2C_0.IBSR.B.TCF != 1);
	while(I2C_0.IBSR.B.IBIF != 1);
	while(I2C_0.IBSR.B.RXAK == 1);

	I2C_0.IBSR.R = 0xff;
	I2C_0.IBDR.R = addr_lsb;

	while(I2C_0.IBSR.B.TCF != 0);
	while(I2C_0.IBSR.B.TCF != 1);
	while(I2C_0.IBSR.B.IBIF != 1);
	while(I2C_0.IBSR.B.RXAK == 1);
	I2C_0.IBCR.B.MSSL = 0;               //Sending stop signal    
	for(wait=0;wait<2000;wait++);       // wait for some duration after ganarating stop before next start

	I2C_0.IBSR.B.IBIF = 0x1;		//clear IBIF flag
	//I2C_0.IBCR.B.TXRX = 0;                   // Config master as Rx
	I2C_0.IBCR.B.MSSL = 0x1;		    //I2C_0 is a master & generates 'start'
//      I2C_0.IBCR.B.RSTA = 0x1;
	I2C_0.IBDR.R = 0xA1;	                    //Slave address write i.e. 0xA0 + 0x1 

	while(I2C_0.IBSR.B.IBB != 1);            //wait for the bus to become engaged
	while(I2C_0.IBSR.B.IBIF != 1);           //Wait for transfer completion
	while(I2C_0.IBSR.B.RXAK == 1); 	    //wait for slave Ack

/*##### Data Cycle #####*/
	I2C_0.IBSR.R = 0xff;
	I2C_0.IBCR.B.TXRX = 0;                   // Config master as Rx
	temp = I2C_0.IBDR.R;                     // Dummy read

	for(j=0;j<256;j++)
	{
	    while(I2C_0.IBSR.B.IBIF != 1);           //Wait for transfer completion
	    while(I2C_0.IBSR.B.TCF != 1);           //Wait for transfer completion
	    if(j == 254)
		I2C_0.IBCR.B.NOACK = 1;	// Set before 2nd last byte Read
	    I2C_0.IBSR.R = 0xff;
	    tempc[j] = I2C_0.IBDR.R;
	}
	I2C_0.IBCR.B.MSSL = 0;               //Sending stop signal
	printf("all the data read successfully");

	printf("Comparing the read data");
	for(j=0;j<256;j++)
	{  
	    if(tempc[j] != *(image_addr+j))
		err_cnt++;
	    //printf("data=%x",tempc[j]);
	}
	if(err_cnt==0) {
	    printf("All the data tx and rx successfully");
	}
	else {
	    error("Tx and Rx data not matched and count=%d",err_cnt);
	}
    } 
}
#endif

void I2C_External_Memory_Access_test(void)
{
#ifdef EN_I2C_External_Memory_Access_test
    
    uint8_t *i2c_data;
    
    for(i2c_data = 0X3E880010; i2c_data < (0X3E880010+(64*1024));i2c_data++) {
	*(uint8_t *)i2c_data = 0x20;
    }
    i2c_eeprom((uint8_t *) 0X3E880010, 64*1024);

#else
    error("NA for this NPI");
#endif
}

void I2C_Mstr_Tx_With_DFS_test(void)
{
#ifdef EN_I2C_Mstr_Tx_With_DFS_test
    
    int i;
    uint8_t rxd_char,master,master_num,slave_num,err_cnt=0;

    more_master_var = 0;
    check_en = 1;

    while(1)
    {
	printf("I2C_Mstr_Tx_With_DFS_test case");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");
	
	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	while(master_tf_over == 0)
	{
	    /* Echo received data */
	    if (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr1_Write_Transfer);
	    }
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);
    }

#else
    error("NA for this NPI");
#endif
}

void I2C_Pin_Mux_test()
{
#ifdef EN_I2C_Pin_Mux_test
    
    int i;
    uint8_t rxd_char,master_num,slave_num,err_cnt=0;

    more_master_var = 0;
    check_en = 1;

    while(1)
    {
	printf("I2C Pin mux test case");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	while(master_tf_over == 0)
	{
	    /* Echo received data */
	    if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr1_Write_Transfer);
	    }
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);
    }

#else
    error("NA for this NPI");
#endif
}

void I2C_Stress_test(void)
{
#ifdef EN_I2C_Stress_test
    
    uint8_t master_num,slave_num,slave_address,slave_receive_address,slave_transmit_address,baud_rate_mul,baud_rate,cnt,inst_arr[3],i,i2c_data,i2c_data_array_size;    

LOOP:
    set_int_limits(0,I2C_INSTANCES-1);
    master_num = get_int_with_limit("Get master instance number");

    set_int_limits(0,I2C_INSTANCES-1);
    slave_num = get_int_with_limit("Get slave instance number");

    if(master_num == slave_num) {
	goto LOOP;
    }
    trace_variable("master_num",master_num);
    trace_variable("slave_num",slave_num);
    
    //random slave address

    set_int_limits(1,127);
    slave_address = get_int_with_limit("select slave address");

    trace_variable("slave_address",slave_address);

    if(slave_address%2==0){
	slave_receive_address = slave_address;
	slave_transmit_address = slave_address+1;
    }
    else
    {
	slave_receive_address = slave_address-1;
	slave_transmit_address = slave_address;
	slave_address = slave_address-1;
    }

    //random baud_rate selection
    set_int_limits(1,4);
    baud_rate_mul = get_int_with_limit("Get baud_rate multiplier value");
    trace_variable("baud_rate_mul",baud_rate_mul);
    if(baud_rate_mul==3) {
	baud_rate_mul=baud_rate_mul+1;
    }

    if( baud_rate_mul == 1) {

	set_int_limits(35,63);
	baud_rate = get_int_with_limit("Get module clock divider value");
    }
    else if ( baud_rate_mul == 2) {

	set_int_limits(91,126);
	baud_rate = get_int_with_limit("Get module clock divider value");
    }
    else {

	set_int_limits(146,191);
	baud_rate = get_int_with_limit("Get module clock divider value");
    }

    trace_variable("IBFD_value",baud_rate);

    set_int_limits(10,21);
    i2c_data_array_size = get_int_with_limit("Get transmit data array size");
    trace_variable("i2c_data_array_size",i2c_data_array_size);

    for (i=0;i<sizeof(my_buffer);i++)
	my_buffer[i] == 0;

    for(i=0; i < i2c_data_array_size;i++) {

	set_int_limits(1,254);
	i2c_data = get_int_with_limit("Get random data to transfer");
	trace_variable("i2c_data ",i2c_data );
	my_buffer[i] = i2c_data;

    }

    more_master_var = 0;
    check_en = 1;
    
    /* Initialize TX buffer structure */
    i2c_tx_buffer.tx_index = 0;
    i2c_tx_buffer.rx_index = 0;
    i2c_tx_buffer.data_present = TRUE;
    i2c_tx_buffer.length = i2c_data_array_size;
    for (i=0;i<i2c_data_array_size;i++)
	i2c_tx_buffer.buf[i] = my_buffer[i];

    /* Initialize RX buffer structure */
    i2c_rx_buffer.tx_index = 0;
    i2c_rx_buffer.rx_index = 0;
    i2c_rx_buffer.data_present = FALSE;
    i2c_rx_buffer.length = i2c_data_array_size;

    i2c_reg_reset(master_num);
    i2c_reg_reset(slave_num);

    /*ISR registeration*/
    printf("MASTER:Registering i2c%d ISR and configuring pin muxing",master_num); 
    if ( master_num == 0) {
#ifdef CORE_M4_ENABLE
	intc_install_interrupt(INT_I2C0,&i2c0_isr,PRIORITY_3,CORE_CM4_0);
#endif
	WRITE_INDEX(I2C0_SDA_MSCR_OPT0,SIUL2.MSCR[I2C0_SDA_MSCR_OPT0].R , 0x00380101); //SDA PA15 //J48-4
	WRITE_INDEX(I2C0_SCL_MSCR_OPT0,SIUL2.MSCR[I2C0_SCL_MSCR_OPT0].R , 0x00380101); //SCL PB0  //J48-2

	WRITE_INDEX(I2C0_SDA_IMCR_OPT0,SIUL2.MSCR[I2C0_SDA_IMCR_OPT0].R , 0x2);
	WRITE_INDEX(I2C0_SCL_IMCR_OPT0,SIUL2.MSCR[I2C0_SCL_IMCR_OPT0].R , 0x2);

	printf("Attach I2C bus to respective PortA/B pins");
	printf("PA[15] --> SDA0");
	printf("PB[0] --> SCL0");
	    
    }
    else if ( master_num == 1) {
#ifdef CORE_M4_ENABLE
	intc_install_interrupt(INT_I2C1,&i2c1_isr,PRIORITY_3,CORE_CM4_0);
#endif
	WRITE_INDEX(I2C1_SDA_MSCR_OPT0,SIUL2.MSCR[I2C1_SDA_MSCR_OPT0].R , 0x00380101); // SDA1 PB1 //J155-1
	WRITE_INDEX(I2C1_SCL_MSCR_OPT0,SIUL2.MSCR[I2C1_SCL_MSCR_OPT0].R , 0x00380101); // SCL1 PB2 //J154-1
	
	WRITE_INDEX(I2C1_SDA_IMCR_OPT0,SIUL2.MSCR[I2C1_SDA_IMCR_OPT0].R , 0x2); // SDA1 
	WRITE_INDEX(I2C1_SCL_IMCR_OPT0,SIUL2.MSCR[I2C1_SCL_IMCR_OPT0].R , 0x2); // SCL1

	printf("Attach I2C bus to respective PortB pins");
	printf("PB[1] --> SDA1");
	printf("PB[2] --> SCL1");
    }
    else {
#ifdef CORE_M4_ENABLE
	intc_install_interrupt(INT_I2C2,&i2c2_isr,PRIORITY_3,CORE_CM4_0);
#endif
	WRITE_INDEX(I2C2_SDA_MSCR_OPT0,SIUL2.MSCR[I2C2_SDA_MSCR_OPT0].R , 0x00380101);  //SDA2 PB3 J148-2
	WRITE_INDEX(I2C2_SCL_MSCR_OPT0,SIUL2.MSCR[I2C2_SCL_MSCR_OPT0].R , 0x00380101);  //SCL2 PB4 J157-2

	WRITE_INDEX(I2C2_SDA_IMCR_OPT0,SIUL2.MSCR[I2C2_SDA_IMCR_OPT0].R , 0x2); // SDA2 
	WRITE_INDEX(I2C2_SCL_IMCR_OPT0,SIUL2.MSCR[I2C2_SCL_IMCR_OPT0].R , 0x2); // SCL2 

	printf("Attach I2C bus to respective PortB pins");
	printf("PB[3] --> SDA2");
	printf("PB[4] --> SCL2");
    }

    printf("SLAVE:Registering i2c%d ISR and configuring pin muxing",slave_num);
    if ( slave_num == 0) {
#ifdef CORE_M4_ENABLE
	intc_install_interrupt(INT_I2C0,&i2c0_isr,PRIORITY_3,CORE_CM4_0);
#endif
	WRITE_INDEX(I2C0_SDA_MSCR_OPT0,SIUL2.MSCR[I2C0_SDA_MSCR_OPT0].R , 0x00380101); //SDA PA15 //J48-4
	WRITE_INDEX(I2C0_SCL_MSCR_OPT0,SIUL2.MSCR[I2C0_SCL_MSCR_OPT0].R , 0x00380101); //SCL PB0  //J48-2

	WRITE_INDEX(I2C0_SDA_IMCR_OPT0,SIUL2.MSCR[I2C0_SDA_IMCR_OPT0].R , 0x2);
	WRITE_INDEX(I2C0_SCL_IMCR_OPT0,SIUL2.MSCR[I2C0_SCL_IMCR_OPT0].R , 0x2);

	printf("Attach I2C bus to respective PortA/B pins");
	printf("PA[15] --> SDA0");
	printf("PB[0] --> SCL0");
    }
    else if (slave_num == 1) {
#ifdef CORE_M4_ENABLE
	intc_install_interrupt(INT_I2C1,&i2c1_isr,PRIORITY_3,CORE_CM4_0);
#endif
	WRITE_INDEX(I2C1_SDA_MSCR_OPT0,SIUL2.MSCR[I2C1_SDA_MSCR_OPT0].R , 0x00380101); // SDA1 PB1 //J155-1
	WRITE_INDEX(I2C1_SCL_MSCR_OPT0,SIUL2.MSCR[I2C1_SCL_MSCR_OPT0].R , 0x00380101); // SCL1 PB2 //J154-1

	WRITE_INDEX(I2C1_SDA_IMCR_OPT0,SIUL2.MSCR[I2C1_SDA_IMCR_OPT0].R , 0x2); // SDA1 
	WRITE_INDEX(I2C1_SCL_IMCR_OPT0,SIUL2.MSCR[I2C1_SCL_IMCR_OPT0].R , 0x2); // SCL1

	printf("Attach I2C bus to respective PortB pins");
	printf("PB[1] --> SDA1");
	printf("PB[2] --> SCL1");
    }	
    else {
#ifdef CORE_M4_ENABLE
	intc_install_interrupt(INT_I2C2,&i2c2_isr,PRIORITY_3,CORE_CM4_0);
#endif
	WRITE_INDEX(I2C2_SDA_MSCR_OPT0,SIUL2.MSCR[I2C2_SDA_MSCR_OPT0].R , 0x00380101);  //SDA2 PB3 J148-2
	WRITE_INDEX(I2C2_SCL_MSCR_OPT0,SIUL2.MSCR[I2C2_SCL_MSCR_OPT0].R , 0x00380101);  //SCL2 PB4 J157-2

	WRITE_INDEX(I2C2_SDA_IMCR_OPT0,SIUL2.MSCR[I2C2_SDA_IMCR_OPT0].R , 0x2); // SDA2 
	WRITE_INDEX(I2C2_SCL_IMCR_OPT0,SIUL2.MSCR[I2C2_SCL_IMCR_OPT0].R , 0x2); // SCL2 

	printf("Attach I2C bus to respective PortB pins");
	printf("PB[3] --> SDA2");
	printf("PB[4] --> SCL2");
    }

    /***** Initialize I2C Module *****/
    printf("MASTER:Configuring I2C %d registers",master_num);
    RegWrF(master_num,IBFD,IBFD_IBC_Bus_Clk_Rate, baud_rate);   
    RegWrF(master_num,IBCR,IBCR_IBIE_IBUS_Interrupt_Enable,1);
    RegWrF(master_num,IBCR,IBCR_MDIS_Module_Disable_bit,e_MDIS_Module_Enabled);
    //RegWrF(master_num,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable,1);


    printf("SLAVE:Configuring I2C %d registers",slave_num);
    RegWrF(slave_num,IBAD, IBAD_ADR_Slave_Address, slave_address);
    RegWrF(slave_num,IBFD,IBFD_IBC_Bus_Clk_Rate, baud_rate);
    RegWrF(slave_num,IBCR,IBCR_IBIE_IBUS_Interrupt_Enable,1);
    RegWrF(slave_num,IBCR,IBCR_MDIS_Module_Disable_bit, e_MDIS_Module_Enabled);

    master_mode = I2C_TXRX;
    master_tx_done = FALSE;

    while (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Busy);
    /* Put module in master TX mode (generates START) */
    RegWrF(master_num,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);
    RegWrF(master_num,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);
    /* Put target address into IBDR */
    RegWrF(master_num,IBDR, IBDR_Data, slave_receive_address);

    while (!master_tx_done);
    master_tx_done = FALSE;

    printf("Check slave received data");
    for (i=0;i<sizeof(my_buffer);i++) {
	if(i2c_tx_buffer.buf[i]== i2c_rx_buffer.buf[i]) {
	    printf("Data=0x%x transfered successfully",i2c_rx_buffer.buf[i]);
	}
	else{
	    error("Data=0x%x transfer failed",i2c_rx_buffer.buf[i]);
	}
	i2c_tx_buffer.buf[i]=0;
	i2c_rx_buffer.buf[i]=0;    
    }

    /* Wait for I2SR[IBB] (bus busy) to be set */
    //while ((RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy))!= e_IBB_Bus_Busy);

    /* Wait for TX to finish before starting RX */
    i2c_tx_buffer.tx_index = 0;
    i2c_tx_buffer.rx_index = 0;
    i2c_tx_buffer.data_present = TRUE;
    i2c_tx_buffer.length = i2c_data_array_size;
    for (i=0 ;i < i2c_data_array_size ; i++)
	i2c_tx_buffer.buf[i] = my_buffer[i];

    /* Initialize RX buffer structure */
    i2c_rx_buffer.tx_index = 0;
    i2c_rx_buffer.rx_index = 0;
    i2c_rx_buffer.data_present = FALSE;
    i2c_rx_buffer.length = i2c_data_array_size;

    printf("sending repeated start condition");
    master_mode = I2C_RX;
    /* Set IBCR.RSTA and put in master RX mode */
    RegWrF(master_num,IBCR, IBCR_RSTA_Repeat_Start_Master_Only, 1);
    /* Put target address into IBDR and set the R/!W bit */
    RegWrF(master_num,IBDR, IBDR_Data, slave_transmit_address);

    while (!master_rx_done);
    master_rx_done = FALSE;

    printf("Check master received data");
    for (i=0;i<sizeof(my_buffer);i++) {
	if(i2c_tx_buffer.buf[i]== i2c_rx_buffer.buf[i]) {
	    printf("Data=0x%x transfered successfully",i2c_rx_buffer.buf[i]);
	}
	else{
	    error("Data=0x%x transfer failed",i2c_rx_buffer.buf[i]);
	}
	i2c_tx_buffer.buf[i]=0;
	i2c_rx_buffer.buf[i]=0;    
    }

    i2c_reg_reset(master_num);
    i2c_reg_reset(slave_num);

#else
    error("NA for this NPI");
#endif
}

void I2C_Byte_Rx_Int_Enable_test()
{
#ifdef EN_I2C_Byte_Rx_Int_Enable_test
    
    int i;
    uint8_t rxd_char,master_num,slave_num,err_cnt=0;

    while(1)
    {
	printf("I2C Pin mux test case");
	set_user_option("Test Continue..");
	set_user_option("Test Done..");

	rxd_char=get_user_option_index("Select Options.");

	if(rxd_char==1)
	    break;

	master_tf_over=0;
	/* Initialize TX buffer structure */
	i2c_tx_buffer.tx_index = 0;
	i2c_tx_buffer.rx_index = 0;
	i2c_tx_buffer.data_present = TRUE;
	i2c_tx_buffer.length = sizeof(my_buffer);
	for (i=0;i<sizeof(my_buffer);i++)
	    i2c_tx_buffer.buf[i] = my_buffer[i];

	/* Initialize RX buffer structure */
	i2c_rx_buffer.tx_index = 0;
	i2c_rx_buffer.rx_index = 0;
	i2c_rx_buffer.data_present = FALSE;
	i2c_rx_buffer.length = sizeof(my_buffer);

	set_int_limits(0,I2C_INSTANCES-1);
	master_num = get_int_with_limit("\n Please select the master instance no ");

	set_int_limits(0,I2C_INSTANCES-1);
	slave_num = get_int_with_limit("\n Please select the slave instance no ");

	i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	i2c_inst_global = slave_num;
	printf("pin muxing configuration for slave device and instance num is %d",slave_num);
	i2c_pin_muxing(slave_num);

	i2c_reg_reset(slave_num);

	if (slave_num == 0) {
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C0,&i2c_rx_int_isr,PRIORITY_3,CORE_CM4_0);
#endif
	}
	else if(slave_num == 1){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C1,&i2c_rx_int_isr,PRIORITY_3,CORE_CM4_0);
#endif
	}
	else if(slave_num == 2){
#ifdef CORE_M4_ENABLE
	    intc_install_interrupt(INT_I2C2,&i2c_rx_int_isr,PRIORITY_3,CORE_CM4_0);
#endif
	}
	
	printf("SLAVE:Configuring I2C %d registers",slave_num);
	RegWrF(slave_num,IBFD,IBFD_IBC_Bus_Clk_Rate,BAUD_RATE_DIV);
	RegWrF(slave_num,IBAD, IBAD_ADR_Slave_Address, e_ADR_Slave_ADR1);
	RegWrF(slave_num,IBCR,IBCR_MDIS_Module_Disable_bit, e_MDIS_Module_Enabled);
	RegWrF(slave_num,IBIC,IBIC_BYTERXIE_BYTERX_Interrupt_Enable,1);

	while(master_tf_over == 0)
	{
	    /* Echo received data */
	    if ( RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		master_tf_over = i2c_transfer(I2C_TX,master_num,e_Slave_Addr1_Write_Transfer);
	    }
	}

	printf("Checking transferred data validity");
	for (i=0;i<sizeof(my_buffer);i++) {
	    if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		err_cnt++;
		error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
	    }
	    i2c_tx_buffer.buf[i]=0;
	    i2c_rx_buffer.buf[i]=0;    
	}
	if(err_cnt == 0) {
	    step("TEST PASS:All the data transfered successfully");
	}
	else {
	    error("TEST FAIL:Data transfer failed");
	}

	i2c_reg_reset(master_num);
	i2c_reg_reset(slave_num);
    }

#else
    error("NA for this NPI");
#endif
}

void I2C_short_func_reset_test(void)
{
    int i,reset_status ;
    uint8_t rxd_char,master_num,slave_num,err_cnt=0;

    more_master_var = 0;

    reset_status = get_int_value("test_case_status");
    if(reset_status==0)
    {
	while(1)
	{
	    store_int_value("test_case_status",1);
	    printf("I2C_short_func_reset_test");
	    set_user_option("Test Continue..");
	    set_user_option("Test Done..");

	    rxd_char=get_user_option_index("Select Options.");

	    if(rxd_char==1)
		break;
	    master_tf_over=0;
	    /* Initialize TX buffer structure */
	    i2c_tx_buffer.tx_index = 0;
	    i2c_tx_buffer.rx_index = 0;
	    i2c_tx_buffer.data_present = TRUE;
	    i2c_tx_buffer.length = sizeof(my_buffer);
	    for (i=0;i<sizeof(my_buffer);i++)
		i2c_tx_buffer.buf[i] = my_buffer[i];

	    /* Initialize RX buffer structure */
	    i2c_rx_buffer.tx_index = 0;
	    i2c_rx_buffer.rx_index = 0;
	    i2c_rx_buffer.data_present = FALSE;
	    i2c_rx_buffer.length = sizeof(my_buffer);

	    set_int_limits(0,I2C_INSTANCES-1);
	    master_num = get_int_with_limit("\n Please select the master instance no ");

	    set_int_limits(0,I2C_INSTANCES-1);
	    slave_num = get_int_with_limit("\n Please select the slave instance no ");

	    //i2c_init(e_I2C_MASTER,master_num,0xBE,e_ADR_Slave_ADR1);
	    //i2c_init(e_I2C_SLAVE,slave_num,0xBE,e_ADR_Slave_ADR1);
	    i2c_short_func_reset=1;
	    i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	    i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	    while(master_tf_over == 0)
	    {
		/* Echo received data */
		if (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		    master_tf_over = i2c_transfer(I2C_RX,master_num,e_Slave_Addr1_Read_Transfer);
		}
	    }

	    printf("Checking transferred data validity");
	    for (i=0;i<sizeof(my_buffer);i++) {
		if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		    err_cnt++;
		    error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
		}
		i2c_tx_buffer.buf[i]=0;
		i2c_rx_buffer.buf[i]=0;    
	    }
	    if(err_cnt == 0) {
		step("TEST PASS:All the data transfered successfully");
	    }
	    else {
		error("TEST FAIL:Data transfer failed");
	    }

	    i2c_reg_reset(master_num);
	    i2c_reg_reset(slave_num);	
	} 
    }
    else
    {
	store_int_value("test_case_status",0);
	while(1)
	{
	    printf("I2C Master receive executing after func reset");
	    set_user_option("Test Continue..");
	    set_user_option("Test Done..");
	    i2c_short_func_reset=0;
	    rxd_char=get_user_option_index("Select Options.");

	    if(rxd_char==1)
		break;
	    master_tf_over=0;
	    /* Initialize TX buffer structure */
	    i2c_tx_buffer.tx_index = 0;
	    i2c_tx_buffer.rx_index = 0;
	    i2c_tx_buffer.data_present = TRUE;
	    i2c_tx_buffer.length = sizeof(my_buffer);
	    for (i=0;i<sizeof(my_buffer);i++)
		i2c_tx_buffer.buf[i] = my_buffer[i];

	    /* Initialize RX buffer structure */
	    i2c_rx_buffer.tx_index = 0;
	    i2c_rx_buffer.rx_index = 0;
	    i2c_rx_buffer.data_present = FALSE;
	    i2c_rx_buffer.length = sizeof(my_buffer);

	    set_int_limits(0,I2C_INSTANCES-1);
	    master_num = get_int_with_limit("\n Please select the master instance no ");

	    set_int_limits(0,I2C_INSTANCES-1);
	    slave_num = get_int_with_limit("\n Please select the slave instance no ");

	    i2c_init(e_I2C_MASTER,master_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);
	    i2c_init(e_I2C_SLAVE,slave_num,BAUD_RATE_DIV,e_ADR_Slave_ADR1);

	    //i2c_init(e_I2C_MASTER,master_num,FAST_BAUD_RATE,e_ADR_Slave_ADR1);
	    //i2c_init(e_I2C_SLAVE,slave_num,FAST_BAUD_RATE,e_ADR_Slave_ADR1);

	    while(master_tf_over == 0)
	    {
		/* Echo received data */
		if (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle) {
		    master_tf_over = i2c_transfer(I2C_RX,master_num,e_Slave_Addr1_Read_Transfer);
		}
	    }

	    printf("Checking transferred data validity");
	    for (i=0;i<sizeof(my_buffer);i++) {
		if(i2c_tx_buffer.buf[i]!= i2c_rx_buffer.buf[i]) {
		    err_cnt++;
		    error("Data_%d=0x%x transfer failed",i,i2c_rx_buffer.buf[i]);
		}
		i2c_tx_buffer.buf[i]=0;
		i2c_rx_buffer.buf[i]=0;    
	    }
	    if(err_cnt == 0) {
		step("TEST PASS:All the data transfered successfully");
	    }
	    else {
		error("TEST FAIL:Data transfer failed");
	    }

	    i2c_reg_reset(master_num);
	    i2c_reg_reset(slave_num);	
	}
	FCCU_NCFS_Clear(77);
    }

}

    
void i2c_rx_int_isr()
{
    uint8_t dummy_read;

    printf("entered into i2c_rx_int_isr");
    
    if(RegRdF(i2c_inst_global,IBSR,IBSR_IAAS_Addressed_as_Slave) == e_IAAS_Addressed_as_Slave) {
	/* Data byte is Slave Address byte - Check Slave Read/Write bit. */
	if (RegRdF(i2c_inst_global,IBSR,IBSR_SRW_Slave_Read_Write_Mode)== e_SRW_Slave_Transmit) {
	    /* Set tx_index to 0 */
	    i2c_tx_buffer.tx_index = 0;

	    /* Master was reading from slave - Set Transmit Mode. */
	    RegWrF(i2c_inst_global,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);

	    /* Write data to MBDR. */
	    RegWrF(i2c_inst_global,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
            #ifdef DEBUG
	    printf("Slave TX: First byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
            #endif
	}
	else {
	    /* Master has specified Slave Receive Mode.
	       Set Receive Mode.  (Writing to MBCR clears IAAS.) */

	    /* Set rx_index to 0 */
	    i2c_rx_buffer.rx_index = 0;

	    RegWrF(i2c_inst_global,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);
	    /* Read address data from MBDR and store it. */
	    dummy_read = RegRd(i2c_inst_global,IBDR);
            #ifdef DEBUG
	    step("Slave RX: Receive address.\n");
            #endif
	}
    }
    else {
	if (RegRdF(i2c_inst_global,IBDBG,IBDBG_BYTERX_Status) == 1) {

	    RegWrF(i2c_inst_global,IBDBG,IBDBG_BYTERX_Status,1);
	
	    if(i2c_rx_buffer.length == 1) {
		slave_rx_done = TRUE;
	    }
	    /* Receive Mode - Read data from MBDR and store it. */
	    i2c_rx_buffer.buf[i2c_rx_buffer.rx_index++] = RegRd(i2c_inst_global,IBDR);
	    i2c_rx_buffer.length--;
	    i2c_rx_buffer.data_present = TRUE;

            #ifdef DEBUG
	    printf("Slave RX: Receive byte - 0x%02X\n",i2c_rx_buffer.buf[i2c_rx_buffer.rx_index-1]);
            #endif
	}
    }

    printf("exit from i2c_rx_int_isr");
}

#ifdef EN_I2C_Stop_Mode_test

void i2c0_stop_mode_isr()
{

    while(MC_ME.GS.B.S_MTRANS)
	asm("nop");
    printf("entered I2C0_ISR");

    WRITE(i2c_struct[0].I2C_n->IBSR.R ,(1<<1));

   
    if(RegRdF(0,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable)==1){
	if(RegRdF(0,IBSR,IBSR_IBB_Bus_Busy)== 0) {
	    printf("bus idle interrupt occured");
	}
    }
    else {
	if(RegRdF(0,IBSR,IBSR_IBAL_Arbitration_Lost)==1){
	WRITE(i2c_struct[0].I2C_n->IBSR.R ,(1<<4));
	printf("arbitration lost occured");
	RegRd(0,IBCR);
	}
	else{
	    if(RegRdF(0,IBSR,IBSR_TCF_Transfer_Complete)==1) {
		printf("Transfer complete bit is set");
	    }
	    if(RegRdF(0,IBSR,IBSR_IAAS_Addressed_as_Slave)==1) {
		printf("addressed as slave bit is set");
	    }
	}
    }
    printf("exit from I2C0_ISR");
    stop= 1;

    printf("previous mode0x%x",MC_ME.DMTS.R);
}

void i2c1_stop_mode_isr()
{

    while(MC_ME.GS.B.S_MTRANS)
	asm("nop");
    
    printf("entered I2C1_ISR");
    
   WRITE(i2c_struct[1].I2C_n->IBSR.R , (1<<1));
   
   
    if(RegRdF(1,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable)==1){
	if(RegRdF(1,IBSR,IBSR_IBB_Bus_Busy)== 0) {
	    printf("bus idle interrupt occured");
	}
    }
    else {
	if(RegRdF(1,IBSR,IBSR_IBAL_Arbitration_Lost)==1){
	WRITE(i2c_struct[1].I2C_n->IBSR.R ,(1<<4));
	printf("arbitration lost occured");
	RegRd(1,IBCR);
	}
	else{
	    if(RegRdF(1,IBSR,IBSR_TCF_Transfer_Complete)==1) {
		printf("Transfer complete bit is set");
	    }
	    if(RegRdF(1,IBSR,IBSR_IAAS_Addressed_as_Slave)==1) {
		printf("addressed as slave bit is set");
	    }
	}
    }
    printf("exit from I2C1_ISR");
    stop= 1;

    printf("previous mode0x%x",MC_ME.DMTS.R);  
}

#if I2C_INSTANCES == 3 || I2C_INSTANCES == 4
void i2c2_stop_mode_isr()
{

    while(MC_ME.GS.B.S_MTRANS)
	asm("nop");
    printf("entered I2C2_ISR");

    WRITE(i2c_struct[2].I2C_n->IBSR.R , (1<<1));

   
    if(RegRdF(2,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable)==1){
	if(RegRdF(2,IBSR,IBSR_IBB_Bus_Busy)== 0) {
	    printf("bus idle interrupt occured");
	}
    }
    else {
	if(RegRdF(2,IBSR,IBSR_IBAL_Arbitration_Lost)==1){
	WRITE(i2c_struct[2].I2C_n->IBSR.R , (1<<4));
	printf("arbitration lost occured");
	RegRd(2,IBCR);
	}
	else{
	    if(RegRdF(2,IBSR,IBSR_TCF_Transfer_Complete)==1) {
		printf("Transfer complete bit is set");
	    }
	    if(RegRdF(2,IBSR,IBSR_IAAS_Addressed_as_Slave)==1) {
		printf("addressed as slave bit is set");
	    }
	}
    }
   
    printf("exit from I2C2_ISR");
    stop = 1;

    printf("previous mode0x%x",MC_ME.DMTS.R);
}
#endif

#if I2C_INSTANCES == 4
void i2c3_stop_mode_isr()
{
    while(MC_ME.GS.B.S_MTRANS)
	asm("nop");
    printf("entered I2C3_ISR");

    WRITE(i2c_struct[3].I2C_n->IBSR.R , (1<<1));

   
    if(RegRdF(3,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable)==1){
	if(RegRdF(3,IBSR,IBSR_IBB_Bus_Busy)== 0) {
	    printf("bus idle interrupt occured");
	}
    }
    else {
	if(RegRdF(3,IBSR,IBSR_IBAL_Arbitration_Lost)==1){
	WRITE(i2c_struct[3].I2C_n->IBSR.R , (1<<4));
	printf("arbitration lost occured");
	RegRd(3,IBCR);
	}
	else{
	    if(RegRdF(3,IBSR,IBSR_TCF_Transfer_Complete)==1) {
		printf("Transfer complete bit is set");
	    }
	    if(RegRdF(3,IBSR,IBSR_IAAS_Addressed_as_Slave)==1) {
		printf("addressed as slave bit is set");
	    }
	}
    }
   
    printf("exit from I2C3_ISR");
    stop = 1;

    printf("previous mode0x%x",MC_ME.DMTS.R);
}
#endif //CALYPSO6M_1_0

#endif //EN_I2C_Stop_Mode_test

#if defined(EN_I2C_DMA_Mstr_Tx_test) || defined(EN_I2C_DMA_Mstr_Rx_test) || defined(EN_I2C_DMA_Debug_Mode_test)
void DMA0_ISR()
{
    printf("entered into dma isr");
    edma_struct[EDMA_INSTANCE_0].edma->INT.R = edma_struct[EDMA_INSTANCE_0].edma->INT.R;
    
    i2c_struct[master_dma_inst].I2C_n->IBCR.B.DMAEN = e_DISABLE;
    
    if (DMA_TX == 1) {
	i2c_tx_buffer.tx_index = tx_byte_count-1;//cha
	i2c_tx_buffer.length = 1;//cha
    }
    else {
	i2c_rx_buffer.rx_index =  tx_byte_count-2;
	i2c_rx_buffer.length = 2;
    }
    printf("exit from dma isr");			   
}

void i2c_dma_config(uint8_t master_num,uint8_t dma_tx)
{
    uint8_t dma_src,channel_num;
	
    printf("Registering DMA transfer complete interrupt");

    if(master_num == 0) {
#if defined CORE_M4_ENABLE
	intc_install_interrupt(INT_DMA_0_TRANSFER_COMPLETE_0_15 ,&DMA0_ISR,PRIORITY_3,CORE_CM4_0);
#elif defined CORE_A530 || defined CORE_A531 || defined CORE_A532 || defined CORE_A533 
	gic_disable_irq_all_cores(INT_DMA0_TRANS_COMP_0_15);
	intc_install_interrupt(INT_DMA0_TRANS_COMP_0_15,&DMA0_ISR,PRIORITY_3,get_core_id());
#endif
	if (dma_tx == 1) {
	    dma_src = i2c_struct[master_num].tx_dma_src;
	    channel_num = EDMA_CH0;
	}
	else {
	    dma_src = i2c_struct[master_num].rx_dma_src;
	    channel_num = EDMA_CH1;
	}
    }
    else if(master_num == 1) {
#if defined CORE_M4_ENABLE
	intc_install_interrupt(INT_DMA_0_TRNSFER_COMPLETE_16_31 ,&DMA0_ISR,PRIORITY_3,CORE_CM4_0);
#elif defined CORE_A530 || defined CORE_A531 || defined CORE_A532 || defined CORE_A533 
	gic_disable_irq_all_cores(INT_DMA0_TRANS_COMP_16_31);
	intc_install_interrupt(INT_DMA0_TRANS_COMP_16_31,&DMA0_ISR,PRIORITY_3,get_core_id());
#endif
	if (dma_tx == 1) {
	    dma_src = i2c_struct[master_num].tx_dma_src;
	    channel_num = EDMA_CH16;
	}
	else {
	    dma_src = i2c_struct[master_num].rx_dma_src;
	    channel_num = EDMA_CH17;
	}
    }
    else if(master_num == 2){
#if defined CORE_M4_ENABLE
	intc_install_interrupt(INT_DMA_0_TRNSFER_COMPLETE_16_31 ,&DMA0_ISR,PRIORITY_3,CORE_CM4_0);
#elif defined CORE_A530 || defined CORE_A531 || defined CORE_A532 || defined CORE_A533 
	gic_disable_irq_all_cores(INT_DMA0_TRANS_COMP_16_31);
	intc_install_interrupt(INT_DMA0_TRANS_COMP_16_31,&DMA0_ISR,PRIORITY_3,get_core_id());
#endif
	if (dma_tx == 1) {
	    dma_src = i2c_struct[master_num].tx_dma_src;
	    channel_num = EDMA_CH16;
	}
	else {
	    dma_src = i2c_struct[master_num].rx_dma_src;
	    channel_num = EDMA_CH17;
	}
    }
    
    //////////////////////////////////Master DMA Config Starts/////////////////////////////////////////////
    edma_init();
    edma_reset_register();
    edma_reset_tcd();
 
    if (dma_tx == 1) {
	edma_settcd (EDMA_INSTANCE_0,dma_src,channel_num,(vuint32_t)&my_buffer[0],(vuint32_t)&(i2c_struct[master_num].I2C_n->IBDR.R), EDMA_SOFF1, EDMA_DOFF0,
		EDMA_SSIZE0,EDMA_DSIZE0,0,0,1,tx_byte_count-1 );
	WRITE_INDEX_2(EDMA_INSTANCE_0,channel_num,edma_struct[EDMA_INSTANCE_0].edma->TCD[channel_num].CSR.B.INTMAJOR ,1);
    }
    else {
	edma_settcd (EDMA_INSTANCE_0,dma_src,channel_num,(vuint32_t) &(i2c_struct[master_num].I2C_n->IBDR.R),(vuint32_t) &i2c_rx_buffer.buf[0], EDMA_SOFF0, EDMA_DOFF1,
		EDMA_SSIZE0,EDMA_DSIZE0,0,0,1,tx_byte_count-2);
	WRITE_INDEX_2(EDMA_INSTANCE_0,channel_num,edma_struct[EDMA_INSTANCE_0].edma->TCD[channel_num].CSR.B.INTMAJOR ,1);
    }
}

#endif
