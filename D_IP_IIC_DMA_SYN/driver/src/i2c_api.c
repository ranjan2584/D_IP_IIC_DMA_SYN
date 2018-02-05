#include "i2c_soc_config.h"

I2C_BUFFER i2c_tx_buffer;
I2C_BUFFER i2c_rx_buffer;

volatile uint8_t master_tx_done=0,master_rx_done=0,slave_tx_done=0,slave_rx_done=0,\
    master_mode,more_master_var=0,stop=0,DMA_recv_enb=0,check_en=1,find_slave=0;
int i2c_short_func_reset=0;

uint8_t bus_idle[I2C_INSTANCES] = {0},no_ack[I2C_INSTANCES] = {0},arb_lost[I2C_INSTANCES] = {0};

uint8_t my_buffer[NO_OF_DATA];

I2C_STRUCT i2c_struct[]=
{
    {
	&I2C_0,
	"I2C_0",
	I2C0_INTR_VEC,
	I2C0_TX_DMA_SRC,
	I2C0_RX_DMA_SRC,
	I2C0_PCTL_NO,	

    },

    {
	&I2C_1,
	"I2C_1",
	I2C1_INTR_VEC,
	I2C1_TX_DMA_SRC,
	I2C1_RX_DMA_SRC,
	I2C1_PCTL_NO,

    },

#if I2C_INSTANCES == 3 || I2C_INSTANCES == 4
    {
	&I2C_2,
	"I2C_2",
	I2C2_INTR_VEC,
	I2C2_TX_DMA_SRC,
	I2C2_RX_DMA_SRC,
	I2C2_PCTL_NO,

    },
#endif
#if I2C_INSTANCES == 4
    {
	&I2C_3,
	"I2C_3",

    },
#endif
	
};

void i2c_init(i2c_type i2c_instant_type, uint8_t inst_num,uint8_t baud_rate_div,uint8_t slave_address)
{
    uint8_t ch;

    /*ISR registeration*/
#if defined CORE_A530 || defined CORE_A531 || defined CORE_A532 || defined CORE_A533 
    gic_disable_irq_all_cores(i2c_struct[inst_num].intr_vec);
#endif
    if (inst_num == 0) {
	intc_install_interrupt(i2c_struct[inst_num].intr_vec,&i2c0_isr,PRIORITY_3,get_core_id());
    }
    else if(inst_num == 1){
	intc_install_interrupt(i2c_struct[inst_num].intr_vec,&i2c1_isr,PRIORITY_3,get_core_id());
    }
#if I2C_INSTANCES == 3 || I2C_INSTANCES == 4
    else if(inst_num == 2){
	intc_install_interrupt(i2c_struct[inst_num].intr_vec,&i2c2_isr,PRIORITY_3,get_core_id());
    }
#endif //I2C_INSTANCES == 3 || I2C_INSTANCES == 4
#if I2C_INSTANCES == 4
    else if(inst_num == 3){
	intc_install_interrupt(i2c_struct[inst_num].intr_vec,&i2c3_isr,PRIORITY_3,get_core_id());
    }
#endif //I2C_INSTANCES == 4
	
    if(i2c_instant_type == e_I2C_MASTER) {
	
	/*pin mux select*/
	printf("pin muxing configuration for master device and instance num is %d",inst_num);
	i2c_pin_muxing(inst_num);

	i2c_reg_reset(inst_num);

	/***** Initialize I2C Module *****/
	printf("MASTER:Configuring I2C %d registers",inst_num);
	RegWrF(inst_num,IBFD,IBFD_IBC_Bus_Clk_Rate,baud_rate_div);   
	RegWrF(inst_num,IBCR,IBCR_IBIE_IBUS_Interrupt_Enable,1);
	RegWrF(inst_num,IBCR,IBCR_MDIS_Module_Disable_bit,e_MDIS_Module_Enabled);
	
    }
    else {
	
	printf("pin muxing configuration for slave device and instance num is %d",inst_num);
	i2c_pin_muxing(inst_num);

	i2c_reg_reset(inst_num);
	
	printf("SLAVE:Configuring I2C %d registers",inst_num);
	RegWrF(inst_num,IBFD,IBFD_IBC_Bus_Clk_Rate,baud_rate_div);
	RegWrF(inst_num,IBAD, IBAD_ADR_Slave_Address, slave_address);
	RegWrF(inst_num,IBCR,IBCR_IBIE_IBUS_Interrupt_Enable,1);
	RegWrF(inst_num,IBCR,IBCR_MDIS_Module_Disable_bit, e_MDIS_Module_Enabled);
    } 
}

/******************************************************************************/
/*	Purpose:  	General function for performing I2C master transfers.  Capable
 *				of performing Master TX, Master RX, and Master TX/RX w/ Repeated
 *				Start.
 *				
 *	Arguments:	mode - Valid modes include I2C_TX, I2C_RX, and I2C_TXRX 
 *					(all modes defined in i2c.h)
 *				slave_address - The slave address of the I2C module that needs
 *					to be dealt with.  
 */
/******************************************************************************/

uint8_t i2c_transfer(uint8_t mode,uint8_t master_num,uint8_t slave_address_with_r_w)
{
     int i;
     uint8_t dummy_read,err_cnt=0;

     master_mode = mode;
     master_tx_done = FALSE;

     /* Reset index for TX and RX buffers */
     i2c_tx_buffer.tx_index = 0;
     i2c_rx_buffer.rx_index = 0;
    
     /* Transmit followed by receive using RSTA */
     if (mode == I2C_TXRX)
     {
	 /* Make sure bus is idle */
	 while (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Busy);
	 /* Put module in master TX mode (generates START) */
	 RegWrF(master_num,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);
	 RegWrF(master_num,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);
	 /* Put target address into IBDR */
	 RegWrF(master_num,IBDR, IBDR_Data, slave_address_with_r_w);

	 /* Wait for TX to finish before starting RX */
	 while (!master_tx_done);
	 master_tx_done = FALSE;
	 
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
	     step("TEST PASS:Rpt start Master tx successful");
	 }
	 else {
	     error("TEST FAIL:Rpt start Master tx not successful");
	 }

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

	 printf("sending repeated start condition");
	 master_mode = I2C_RX;
	 /* Set IBCR.RSTA and put in master RX mode */
	 RegWrF(master_num,IBCR, IBCR_RSTA_Repeat_Start_Master_Only, 1);
	 /* Put target address into IBDR and set the R/!W bit */
	 RegWrF(master_num,IBDR, IBDR_Data, e_Slave_Addr1_Read_Transfer);

	 while (!master_rx_done);
	 master_rx_done = FALSE;

	 return 1;
     }
    	/* Single TX or RX */
     else if ( (mode == I2C_TX) | (mode == I2C_RX) )
     {
	 /* Make sure bus is idle */
	 if ( more_master_var == 0) {
	     while (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Busy);
	     /* Put module in master TX mode (generates START) */
	     RegWrF(master_num,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);
	     RegWrF(master_num,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);
	     
	 }/* Put target address into IBDR */
	 
	 RegWrF(master_num,IBDR, IBDR_Data,slave_address_with_r_w);

	 if(check_en) {
	     if(mode==I2C_TX) {
		 while (!master_tx_done);
		 master_tx_done = FALSE;
	     }
	     else {
		 while (!master_rx_done);
		 master_rx_done = FALSE;
	     }
	 }
	 return 1;
     }
     else
	 printf("**ERROR** : Invalid Master transfer mode selected.\n");

}

uint16_t i2c_find_slave_add(uint8_t master_num)
{
    int i;
    uint16_t find_add=0;

    find_slave=1;
    master_mode = I2C_TX;
    master_tx_done = FALSE; 
    
    i2c_tx_buffer.tx_index = 0;
    i2c_tx_buffer.rx_index = 0;
    i2c_tx_buffer.data_present = TRUE;
    i2c_tx_buffer.length = sizeof(my_buffer);
    for (i=0;i<sizeof(my_buffer);i++)
	i2c_tx_buffer.buf[i] = my_buffer[i];
    
    while (RegRdF(master_num,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Busy);
    /* Put module in master TX mode (generates START) */
    RegWrF(master_num,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);

    for(find_add=2;find_add < 255; find_add = find_add+2) {
	RegWrF(master_num,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Master);
	RegWrF(master_num,IBDR, IBDR_Data,find_add);
	while(stop==0) {
	    asm("nop");
	}
	stop=0;
	if(no_ack[master_num] == 1) {
	    no_ack[master_num] = 0;
	    //step("no_ack");
	}
	else {
	    find_slave=0;
	    return find_add;
	}
    }

    find_slave=0;
    return 270;
}

void i2c_pin_muxing(uint8_t device_num)
{
    unsigned short config_select=0,instance_select_over=0;	

#if defined(REGRESSION)
    if(device_num == 0){
	config_select = 0;
    }
    else if(device_num == 0){
	config_select = 0;
    }
    else {
	config_select = 0;	
    }
#else    
    if(device_num == 0) {
	set_user_option("PA/PB");
	set_user_option("PE");
	set_user_option("PG");
    }
    else if(device_num == 1){
	set_user_option("PB");
	set_user_option("PE");
	set_user_option("PG");
    }
    else {
	set_user_option("PB");
    }
    config_select = get_user_option_index("Select betwen PORTS");
#endif

    if(device_num==0) {
	if(config_select == 0) {
	    WRITE_INDEX(I2C0_SDA_MSCR_OPT0,SIUL2.MSCR[I2C0_SDA_MSCR_OPT0].R , 0x00380101); //SDA PA15 //J48-4
	    WRITE_INDEX(I2C0_SCL_MSCR_OPT0,SIUL2.MSCR[I2C0_SCL_MSCR_OPT0].R , 0x00380101); //SCL PB0  //J48-2

	    WRITE_INDEX(I2C0_SDA_IMCR_OPT0,SIUL2.MSCR[I2C0_SDA_IMCR_OPT0].R , 0x2);
	    WRITE_INDEX(I2C0_SCL_IMCR_OPT0,SIUL2.MSCR[I2C0_SCL_IMCR_OPT0].R , 0x2);

	    printf("Attach I2C bus to respective PortA/B pins");
	    printf("PA[15] --> SDA0");
	    printf("PB[0] --> SCL0");
	}
	else if(config_select == 1) {

	    WRITE_INDEX(I2C0_SDA_MSCR_OPT1,SIUL2.MSCR[I2C0_SDA_MSCR_OPT1].R , 0x00380102); //SDA PE12 J14-21
	    WRITE_INDEX(I2C0_SCL_MSCR_OPT1,SIUL2.MSCR[I2C0_SCL_MSCR_OPT1].R , 0x00380102); //SCL PE13 J14-23

	    printf("Attach I2C bus to respective PortE pins");
	    printf("PE[12] --> SDA0");
	    printf("PE[13] --> SCL0");
	}
	else {
	    WRITE_INDEX(I2C0_SDA_MSCR_OPT2,SIUL2.MSCR[I2C0_SDA_MSCR_OPT2].R , 0x00380102);   //SDA PG3 J100-3
	    WRITE_INDEX(I2C0_SCL_MSCR_OPT2,SIUL2.MSCR[I2C0_SCL_MSCR_OPT2].R , 0x00380102); //SCL PG4 J100-5

	    WRITE_INDEX(I2C0_SDA_IMCR_OPT2,SIUL2.MSCR[I2C0_SDA_IMCR_OPT2].R , 0x3);
	    WRITE_INDEX(I2C0_SCL_IMCR_OPT2,SIUL2.MSCR[I2C0_SCL_IMCR_OPT2].R , 0x3);

	    printf("Attach I2C bus to respective PortG pins");
	    printf("PG[3] --> SDA0");
	    printf("PG[4] --> SCL0");
	}
    }
    else if(device_num==1) {
	if(config_select == 0) {

	    WRITE_INDEX(I2C1_SDA_MSCR_OPT0,SIUL2.MSCR[I2C1_SDA_MSCR_OPT0].R , 0x00380101); // SDA1 PB1 //J155-1
	    WRITE_INDEX(I2C1_SCL_MSCR_OPT0,SIUL2.MSCR[I2C1_SCL_MSCR_OPT0].R , 0x00380101); // SCL1 PB2 //J154-1

	    WRITE_INDEX(I2C1_SDA_IMCR_OPT0,SIUL2.MSCR[I2C1_SDA_IMCR_OPT0].R , 0x2); // SDA1 
	    WRITE_INDEX(I2C1_SCL_IMCR_OPT0,SIUL2.MSCR[I2C1_SCL_IMCR_OPT0].R , 0x2); // SCL1 

	    printf("Attach I2C bus to respective PortB pins");
	    printf("PB[1] --> SDA1");
	    printf("PB[2] --> SCL1");
	}
	else if(config_select == 1) {

	    WRITE_INDEX(I2C1_SDA_MSCR_OPT1,SIUL2.MSCR[I2C1_SDA_MSCR_OPT1].R , 0x00380102); // SDA1 PE14 //J14-25
	    WRITE_INDEX(I2C1_SCL_MSCR_OPT1,SIUL2.MSCR[I2C1_SCL_MSCR_OPT1].R , 0x00380102); // SCL1 PE15 //J14-27

	    printf("Attach I2C bus to respective PortE pins");
	    printf("PE[14] --> SDA1");
	    printf("PE[15] --> SCL1");
	}
	else {

	    WRITE_INDEX(I2C1_SDA_MSCR_OPT2,SIUL2.MSCR[I2C1_SDA_MSCR_OPT2].R , 0x00380102);  //SDA1 PG5 J99-1
	    WRITE_INDEX(I2C1_SCL_MSCR_OPT2,SIUL2.MSCR[I2C1_SCL_MSCR_OPT2].R , 0x00380102);  //SCL1 PG6 J99-2

	    WRITE_INDEX(I2C1_SDA_IMCR_OPT2,SIUL2.MSCR[I2C1_SDA_IMCR_OPT2].R , 0x3); // SDA1 
	    WRITE_INDEX(I2C1_SCL_IMCR_OPT2,SIUL2.MSCR[I2C1_SCL_IMCR_OPT2].R , 0x3); // SCL1 

	    printf("Attach I2C bus to respective PortG pins");
	    printf("PG[5] --> SDA1");
	    printf("PG[5] --> SCL1");
	}
    }
    else {
	WRITE_INDEX(I2C2_SDA_MSCR_OPT0,SIUL2.MSCR[I2C2_SDA_MSCR_OPT0].R , 0x00380101);  //SDA2 PB3 J148-2
	WRITE_INDEX(I2C2_SCL_MSCR_OPT0,SIUL2.MSCR[I2C2_SCL_MSCR_OPT0].R , 0x00380101);  //SCL2 PB4 J157-2

	WRITE_INDEX(I2C2_SDA_IMCR_OPT0,SIUL2.MSCR[I2C2_SDA_IMCR_OPT0].R , 0x2); // SDA2 
	WRITE_INDEX(I2C2_SCL_IMCR_OPT0,SIUL2.MSCR[I2C2_SCL_IMCR_OPT0].R , 0x2); // SCL2 

	printf("Attach I2C bus to respective PortB pins");
	printf("PB[3] --> SDA2");
	printf("PB[4] --> SCL2");
    }
}

void i2c_reg_reset(uint8_t device_num)
{
    printf("Resetting I2C %d registers",device_num);
    WRITE(i2c_struct[device_num].I2C_n->IBAD.R , 0);
    WRITE(i2c_struct[device_num].I2C_n->IBFD.R , 0);  
    WRITE(i2c_struct[device_num].I2C_n->IBCR.R , 0x80);  //clear all add....
    WRITE(i2c_struct[device_num].I2C_n->IBSR.R , 0x12);  //clear all add....
    WRITE(i2c_struct[device_num].I2C_n->IBDR.R , 0x00);  //clear all add....
    WRITE(i2c_struct[device_num].I2C_n->IBIC.R , 0x0);  //clear all add....
    WRITE(i2c_struct[device_num].I2C_n->IBDBG.R , 0x4);  //clear all add....
}

void i2c0_isr()
{
     /* Temp variable for dummy reads */
    uint8_t dummy_read;

    #ifdef DEBUG
    printf("entered into i2c0 isr");
    #endif

    /* Clear the I2C Interrupt Flag. */
     WRITE(i2c_struct[0].I2C_n->IBSR.R , (1<<1));

     if(RegRdF(0,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable)==1 && RegRdF(0,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle){
	 bus_idle[0]++; 
	 step("bus idle interrupt occured");
     }

     else {
	 /* Check if this device is in Master or Slave Mode. */
	 if (RegRdF(0,IBCR,IBCR_MS_SLn_Master_Slave_Mode_Select)== e_MS_SLn_Master)
	 {
	     /* Master Mode - Check if this device is in Transmit or Receive Mode. */
	     if (RegRdF(0,IBCR,IBCR_Tx_Rxn_Transmit_Receive_Mode_Select) == e_Tx_Rxn_Transmit_Mode) {
		 /* Master Transmit Mode - Check if last byte was tranmitted. */
		 if ((i2c_tx_buffer.length == 0) && (master_mode != I2C_RX)) {
		     /* Last byte was transmitted - 
			Generate Stop signal by changing to Slave Mode. */
		     /* If TXRX mode (Repeated Start), signal end of TX */
		     if (master_mode == I2C_TXRX)
			 master_tx_done = TRUE;
		     /* Issue STOP */
		     else {
			 RegWrF(0,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);
			 master_tx_done = TRUE;
		     }
		         
                     #ifdef DEBUG
		     printf("Master TX mode: Last Byte\n");
                     #endif
		 }
		 else {
		     /* More bytes to be transmitted - Check if ACK received. */
		     if (RegRdF(0,IBSR,IBSR_RXAK_Receive_Acknowledge) == e_RXAK_No_Ack_Received){
			 /* ACK not received - Generate STOP */
			 RegWrF(0,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);
			 no_ack[0]++;
                         #ifdef DEBUG
			 step("Master TX mode: NAK received by master");
                         #endif
		     }
		     else {
			 /* Check if end of address cycle */
			 if (master_mode == I2C_RX) {
			     RegWrF(0,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);

			     /*Set Transmit Acknowledge Enable bit so no ACK is sent after the only one byte is received, which
			       indicates "end of data" to the slave. */
			     if(i2c_rx_buffer.length == 1) {
				 RegWrF(0,IBCR, IBCR_NOACK_Data_Ack_Disable, e_NOACK_Ack_Not_Sent); //modified
			     }
			     dummy_read = RegRd(0,IBDR);
			     if(DMA_recv_enb == 1) {
				 RegWrF(0,IBCR,IBCR_DMAEN_DMA_Enable,e_ENABLE);
			     }
			     #ifdef DEBUG
			     printf("Master TX mode: End of RX address cycle, switch to RX mode.\n");
                             #endif
			     
			 }
			 /* ACK received, send data */
			 else {
			     if(find_slave == 1) {
				 #ifdef DEBUG
				 step("Got Ack from slave");     
				 #endif
				 RegWrF(0,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);
			     }
			     else {
				 /* Not end of address cycle - Write next byte to MBDR */
				 RegWrF(0,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
				 i2c_tx_buffer.length--;
                                 #ifdef DEBUG
				 printf("Master TX mode: Send Byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
                                 #endif
			     }
			 }
		     }
		 } 
	     }
	     else {
		 /* Master Receive Mode - Check if this is last byte to be read. */	    
		 if (i2c_rx_buffer.length == 1) {
		     printf("I2CR = 0x%02X\n", RegRd(0,IBCR));
		     /* Last byte to be read - 
			Generate Stop signal by changing to Slave Mode. */
		     RegWrF(0,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);

                     #ifdef DEBUG
		     printf("Master RX mode: All data received, send STOP.\n");
                     #endif

		     master_rx_done = TRUE;
		 }
		 else {
		     /* Not last byte to be read - Check if second to last byte. */
		     if (i2c_rx_buffer.length == 2) {
			 printf("I2CR S2L  = 0x%02X\n", RegRd(0,IBCR));
			 /* Second to last byte to be read - Set Transmit Acknowledge Enable
			    bit so no ACK is sent after the next byte is received, which
			    indicates "end of data" to the slave. */
			 RegWrF(0,IBCR, IBCR_NOACK_Data_Ack_Disable, e_NOACK_Ack_Not_Sent);

                         #ifdef DEBUG
			 printf("Master RX mode: Second-to-last byte received, set TXAK.\n");
                         #endif
		     }
		 }

		 /* Store received data in RX buffer */
		 i2c_rx_buffer.buf[i2c_rx_buffer.rx_index++] = RegRd(0,IBDR);
		 i2c_rx_buffer.length--;

                 #ifdef DEBUG
		 printf("Master RX mode: Receive byte - 0x%02X\n",i2c_rx_buffer.buf[i2c_rx_buffer.rx_index-1]);
                 #endif
	     }
	 }
	 else {
	     /* Slave Mode - Check if Arbitration Lost. */
	     if(RegRdF(0,IBSR,IBSR_IBAL_Arbitration_Lost)== TRUE) {
		 arb_lost[0]++;
                 #ifdef DEBUG
		 step("Arbitration Lost.\n");
                 #endif
		 /* Clear IAL bit */
		 RegWrF(0,IBSR, IBSR_IBAL_Arbitration_Lost, e_IBAL_Interrupt_Clear);

		 /* Arbitration Lost - Check if this device is being addressed as slave.
		    (If not, nothing more needs to be done.) */
		 if (RegRdF(0,IBSR,IBSR_IAAS_Addressed_as_Slave) == e_IAAS_Addressed_as_Slave) {
		     /* Addressed as slave - 
			Check if master was reading from slave or writing to slave. */
		     if (RegRdF(0,IBSR,IBSR_SRW_Slave_Read_Write_Mode)== e_SRW_Slave_Transmit) {
			 /* Set tx_index to 0 */
			 i2c_tx_buffer.tx_index = 0;

			 /* Master was reading from slave - Set Transmit Mode. */
			 RegWrF(0,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);

			 /* Write data to MBDR. */
			 RegWrF(0,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
                         #ifdef DEBUG
			 printf("Arbitration Lost: Addressed as slave - TX mode.\n");
                         #endif
		     }
		     else {
			 /* Set rx_index to 0 */
			 i2c_rx_buffer.rx_index = 0;

			 /* Master was writing to slave - Set Receive Mode. */
			 RegWrF(0,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);

			 /* Dummy read from MBDR, to clear the ICF bit. */
			 dummy_read = RegRd(0,IBDR);

                         #ifdef DEBUG
			 printf("Arbitration Lost: Addressed as slave - RX mode.\n");
                         #endif
		     }
		 }	    
	     }
	     else {
		 /* Arbitration Not Lost - Check if data byte is this devices's Slave Address byte. */
		 if(RegRdF(0,IBSR,IBSR_IAAS_Addressed_as_Slave) == e_IAAS_Addressed_as_Slave) {
		     /* Data byte is Slave Address byte - Check Slave Read/Write bit. */
		     if (RegRdF(0,IBSR,IBSR_SRW_Slave_Read_Write_Mode)== e_SRW_Slave_Transmit) {
			 /* Set tx_index to 0 */
			 i2c_tx_buffer.tx_index = 0;

			 /* Master was reading from slave - Set Transmit Mode. */
			 RegWrF(0,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);

			 /* Write data to MBDR. */
			 RegWrF(0,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
                         #ifdef DEBUG
			 printf("Slave TX: First byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
                         #endif
		     }
		     else {
			 /* Master has specified Slave Receive Mode.
			    Set Receive Mode.  (Writing to MBCR clears IAAS.) */

			 /* Set rx_index to 0 */
			 i2c_rx_buffer.rx_index = 0;

			 RegWrF(0,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);
			 /* Read address data from MBDR and store it. */
			 dummy_read = RegRd(0,IBDR);
                         #ifdef DEBUG
			 step("Slave RX: Receive address.\n");
                         #endif
		     }
		 }
		 else {
		     /* Data byte received is not Slave Address byte - 
			Check if this device is in Transmit or Receive Mode. */
		     if (RegRdF(0,IBCR,IBCR_Tx_Rxn_Transmit_Receive_Mode_Select) == e_Tx_Rxn_Transmit_Mode) {
			 /* Last byte received? */
			 if (RegRdF(0,IBSR,IBSR_RXAK_Receive_Acknowledge) == e_RXAK_No_Ack_Received) {
			     RegWrF(0,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);
			     dummy_read = RegRd(0,IBDR);

                             #ifdef DEBUG
			     printf("Slave TX: Last byte has been sent.\n");
                             #endif
			     slave_tx_done = TRUE;

			 }
			 else {
			     /* Write data to MBDR. */
			     RegWrF(0,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
			     i2c_tx_buffer.length--;
			     if(i2c_short_func_reset==1)
			     {
				 if(i2c_tx_buffer.length==5)
				     MC_RGM_soft_func_short(0);
			     }

                             #ifdef DEBUG
			     printf("Slave TX: Send byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
                             #endif
			 }
		     }
		     else {
			 if(i2c_rx_buffer.length == 1) {
			     slave_rx_done = TRUE;
			 }
			 /* Receive Mode - Read data from MBDR and store it. */
			 i2c_rx_buffer.buf[i2c_rx_buffer.rx_index++] = RegRd(0,IBDR);
			 i2c_rx_buffer.length--;
			 i2c_rx_buffer.data_present = TRUE;

                         #ifdef DEBUG
			 printf("Slave RX: Receive byte - 0x%02X\n",i2c_rx_buffer.buf[i2c_rx_buffer.rx_index-1]);
                         #endif
		     }
		 }
	     }

	 }
     }

     stop=1;
     
     #ifdef DEBUG
     printf("exit from i2c0 isr");
     #endif
     
}

void i2c1_isr()
{

     /* Temp variable for dummy reads */
    uint8_t dummy_read;		

    #ifdef DEBUG
    printf("entered into i2c1 isr");
    #endif
    /* Clear the I2C Interrupt Flag. */
     WRITE(i2c_struct[1].I2C_n->IBSR.R , (1<<1));

     if(RegRdF(1,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable)==1 && RegRdF(1,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle){
	 bus_idle[1]++;
	 step("bus idle interrupt occured");
	
     }

     else {
    
	 /* Check if this device is in Master or Slave Mode. */
	 if (RegRdF(1,IBCR,IBCR_MS_SLn_Master_Slave_Mode_Select)== e_MS_SLn_Master)
	 {
	     /* Master Mode - Check if this device is in Transmit or Receive Mode. */
	     if (RegRdF(1,IBCR,IBCR_Tx_Rxn_Transmit_Receive_Mode_Select) == e_Tx_Rxn_Transmit_Mode) {
		 /* Master Transmit Mode - Check if last byte was tranmitted. */
		 if ((i2c_tx_buffer.length == 0) && (master_mode != I2C_RX))	{
		     /* Last byte was transmitted - 
			Generate Stop signal by changing to Slave Mode. */
		     /* If TXRX mode (Repeated Start), signal end of TX */
		     if (master_mode == I2C_TXRX)
			 master_tx_done = TRUE;
		     /* Issue STOP */
		     else {
			 RegWrF(1,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);
			 master_tx_done = TRUE;
		     }
		
                     #ifdef DEBUG
		     printf("Master TX mode: Last Byte\n");
                     #endif
		 }
		 else {
		     /* More bytes to be transmitted - Check if ACK received. */
		     if (RegRdF(1,IBSR,IBSR_RXAK_Receive_Acknowledge) == e_RXAK_No_Ack_Received){
			 /* ACK not received - Generate STOP */
			 RegWrF(1,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);
			 no_ack[1]++;
		         #ifdef DEBUG
			 step("Master TX mode: NAK received by master");
                         #endif
		     }
		     else {
			 /* Check if end of address cycle */
			 if (master_mode == I2C_RX) {
			     RegWrF(1,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);

			     /*Set Transmit Acknowledge Enable bit so no ACK is sent after the only one byte is received, which
			       indicates "end of data" to the slave. */
			     if(i2c_rx_buffer.length == 1) {
				 RegWrF(1,IBCR, IBCR_NOACK_Data_Ack_Disable, e_NOACK_Ack_Not_Sent); //modified
			     }
			     dummy_read = RegRd(1,IBDR);
			     if(DMA_recv_enb == 1) {
				 RegWrF(1,IBCR,IBCR_DMAEN_DMA_Enable,e_ENABLE);
			     }
                             #ifdef DEBUG
			     printf("Master TX mode: End of RX address cycle, switch to RX mode.\n");
                             #endif
			 }
		         /* ACK received, send data */
			 else {
			     if(find_slave == 1) {
				 #ifdef DEBUG
				 step("Got Ack from slave");     
				 #endif
				 RegWrF(1,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);
			     }
			     else {
				 /* Not end of address cycle - Write next byte to MBDR */
				 RegWrF(1,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
				 i2c_tx_buffer.length--;
                                 #ifdef DEBUG
				 printf("Master TX mode: Send Byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
                                 #endif
			     }
			 }
		     }
		 } 
	     }
	     else {
		 /* Master Receive Mode - Check if this is last byte to be read. */	    
		 if (i2c_rx_buffer.length == 1) {
		     printf("I2CR = 0x%02X\n", RegRd(1,IBCR));
		     /* Last byte to be read - 
			Generate Stop signal by changing to Slave Mode. */
		     RegWrF(1,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);

                     #ifdef DEBUG
		     printf("Master RX mode: All data received, send STOP.\n");
                     #endif
		     master_rx_done = TRUE;
		 }
		 else {
		     /* Not last byte to be read - Check if second to last byte. */
		     if (i2c_rx_buffer.length == 2) {
			 printf("I2CR S2L  = 0x%02X\n", RegRd(1,IBCR));
			 /* Second to last byte to be read - Set Transmit Acknowledge Enable
			    bit so no ACK is sent after the next byte is received, which
			    indicates "end of data" to the slave. */
			 RegWrF(1,IBCR, IBCR_NOACK_Data_Ack_Disable, e_NOACK_Ack_Not_Sent);
		    
                         #ifdef DEBUG
			 printf("Master RX mode: Second-to-last byte received, set TXAK.\n");
                         #endif
		     }
		 }

		 /* Store received data in RX buffer */
		 i2c_rx_buffer.buf[i2c_rx_buffer.rx_index++] = RegRd(1,IBDR);
		 i2c_rx_buffer.length--;

                 #ifdef DEBUG
		 printf("Master RX mode: Receive byte - 0x%02X\n",i2c_rx_buffer.buf[i2c_rx_buffer.rx_index-1]);
                 #endif
	     }
	 }
	 else {
	     /* Slave Mode - Check if Arbitration Lost. */
	     if (RegRdF(1,IBSR,IBSR_IBAL_Arbitration_Lost)== TRUE) {
		 arb_lost[1]++;
                 #ifdef DEBUG
		 step("Arbitration Lost.\n");
                 #endif
		 /* Clear IAL bit */
		 RegWrF(1,IBSR, IBSR_IBAL_Arbitration_Lost, e_IBAL_Interrupt_Clear);

		 /* Arbitration Lost - Check if this device is being addressed as slave.
		    (If not, nothing more needs to be done.) */
		 if (RegRdF(1,IBSR,IBSR_IAAS_Addressed_as_Slave) == e_IAAS_Addressed_as_Slave) {
		     /* Addressed as slave - 
			Check if master was reading from slave or writing to slave. */
		     if (RegRdF(1,IBSR,IBSR_SRW_Slave_Read_Write_Mode)== e_SRW_Slave_Transmit) {
			 /* Set tx_index to 0 */
			 i2c_tx_buffer.tx_index = 0;

			 /* Master was reading from slave - Set Transmit Mode. */
			 RegWrF(1,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);

			 /* Write data to MBDR. */
			 RegWrF(1,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
                         #ifdef DEBUG
			 printf("Arbitration Lost: Addressed as slave - TX mode.\n");
                         #endif
		     }
		     else {
			 /* Set rx_index to 0 */
			 i2c_rx_buffer.rx_index = 0;

			 /* Master was writing to slave - Set Receive Mode. */
			 RegWrF(1,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);

			 /* Dummy read from MBDR, to clear the ICF bit. */
			 dummy_read = RegRd(1,IBDR);

                         #ifdef DEBUG
			 printf("Arbitration Lost: Addressed as slave - RX mode.\n");
                         #endif
		     }
		 }	    
	     }
	     else {
		 /* Arbitration Not Lost - Check if data byte is this devices's Slave Address byte. */
		 if (RegRdF(1,IBSR,IBSR_IAAS_Addressed_as_Slave) == e_IAAS_Addressed_as_Slave) {
		     /* Data byte is Slave Address byte - Check Slave Read/Write bit. */
		     if (RegRdF(1,IBSR,IBSR_SRW_Slave_Read_Write_Mode)== e_SRW_Slave_Transmit) {
			 /* Set tx_index to 0 */
			 i2c_tx_buffer.tx_index = 0;

			 /* Master was reading from slave - Set Transmit Mode. */
			 RegWrF(1,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);

			 /* Write data to MBDR. */
			 RegWrF(1,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
                         #ifdef DEBUG
			 printf("Slave TX: First byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
                         #endif
		     }
		     else {
			 /* Master has specified Slave Receive Mode.
			    Set Receive Mode.  (Writing to MBCR clears IAAS.) */

			 /* Set rx_index to 0 */
			 i2c_rx_buffer.rx_index = 0;

			 RegWrF(1,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);
			 /* Read address data from MBDR and store it. */
			 dummy_read = RegRd(1,IBDR);
                         #ifdef DEBUG
			 step("Slave RX: Receive address.\n");
                         #endif
		     }
		 }
		 else {
		     /* Data byte received is not Slave Address byte - 
			Check if this device is in Transmit or Receive Mode. */
		     if (RegRdF(1,IBCR,IBCR_Tx_Rxn_Transmit_Receive_Mode_Select) == e_Tx_Rxn_Transmit_Mode) {
			 /* Last byte received? */
			 if (RegRdF(1,IBSR,IBSR_RXAK_Receive_Acknowledge) == e_RXAK_No_Ack_Received) {
			     RegWrF(1,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);
			     dummy_read = RegRd(1,IBDR);

                             #ifdef DEBUG
			     printf("Slave TX: Last byte has been sent.\n");
                             #endif

			     slave_tx_done = TRUE;
			 }
			 else {
			     /* Write data to MBDR. */
			     RegWrF(1,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
			     i2c_tx_buffer.length--;
			     if(i2c_short_func_reset==1)
			     {
				 if(i2c_tx_buffer.length==5)
				     MC_RGM_soft_func_short(0);
			     }

                             #ifdef DEBUG
			     printf("Slave TX: Send byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
                             #endif

			 }
		     }
		     else {
			 if(i2c_rx_buffer.length == 1) {
			     slave_rx_done = TRUE;
			 }
			 /* Receive Mode - Read data from MBDR and store it. */
			 i2c_rx_buffer.buf[i2c_rx_buffer.rx_index++] = RegRd(1,IBDR);
			 i2c_rx_buffer.length--;
			 i2c_rx_buffer.data_present = TRUE;

                         #ifdef DEBUG
			 printf("Slave RX: Receive byte - 0x%02X\n",i2c_rx_buffer.buf[i2c_rx_buffer.rx_index-1]);
                         #endif
		     }
		 }
	     }
	 }
     }

     stop=1;
     
     #ifdef DEBUG
     printf("exit from i2c1 isr");
     #endif
    
}

#if I2C_INSTANCES == 3 || I2C_INSTANCES == 4
void i2c2_isr()
{
    
    /* Temp variable for dummy reads */
    uint8_t dummy_read;

    #ifdef DEBUG
    printf("entered into i2c2 isr");
    #endif
    /* Clear the I2C Interrupt Flag. */
     WRITE(i2c_struct[2].I2C_n->IBSR.R , (1<<1));

     if(RegRdF(2,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable)==1 && RegRdF(2,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle){
	 bus_idle[2]++;
	step("bus idle interrupt occured");
     }

     else {
    
	 /* Check if this device is in Master or Slave Mode. */
	 if (RegRdF(2,IBCR,IBCR_MS_SLn_Master_Slave_Mode_Select)== e_MS_SLn_Master)
	 {
	     /* Master Mode - Check if this device is in Transmit or Receive Mode. */
	     if (RegRdF(2,IBCR,IBCR_Tx_Rxn_Transmit_Receive_Mode_Select) == e_Tx_Rxn_Transmit_Mode) {
		 /* Master Transmit Mode - Check if last byte was tranmitted. */
		 if ((i2c_tx_buffer.length == 0) && (master_mode != I2C_RX))	{
		     /* Last byte was transmitted - 
			Generate Stop signal by changing to Slave Mode. */
		     /* If TXRX mode (Repeated Start), signal end of TX */
		     if (master_mode == I2C_TXRX)
			 master_tx_done = TRUE;
		     /* Issue STOP */
		     else {
			 RegWrF(2,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);
			 master_tx_done = TRUE;
		     }

                     #ifdef DEBUG
		     printf("Master TX mode: Last Byte\n");
                     #endif
		 }
		 else {
		     /* More bytes to be transmitted - Check if ACK received. */
		     if (RegRdF(2,IBSR,IBSR_RXAK_Receive_Acknowledge) == e_RXAK_No_Ack_Received){
			 /* ACK not received - Generate STOP */
			 RegWrF(2,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);
			 no_ack[2]++;
                         #ifdef DEBUG
			 step("Master TX mode: NAK received by master");
                         #endif
		     }
		     else {
			 /* Check if end of address cycle */
			 if (master_mode == I2C_RX) {
			     RegWrF(2,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);
			     
			     /*Set Transmit Acknowledge Enable bit so no ACK is sent after the only one byte is received, which
			       indicates "end of data" to the slave. */
			     if(i2c_rx_buffer.length == 1) {
				 RegWrF(2,IBCR, IBCR_NOACK_Data_Ack_Disable, e_NOACK_Ack_Not_Sent); //modified
			     }
			     dummy_read = RegRd(2,IBDR);
			     if(DMA_recv_enb == 1) {
				 RegWrF(2,IBCR,IBCR_DMAEN_DMA_Enable,e_ENABLE);
			     }
                             #ifdef DEBUG
			     printf("Master TX mode: End of RX address cycle, switch to RX mode.\n");
                             #endif
			 }
			 /* ACK received, send data */
			 else {
			     if(find_slave == 1) {
				 #ifdef DEBUG
				 step("Got Ack from slave");     
				 #endif
				 RegWrF(2,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);
			     }
			     else {
				 /* Not end of address cycle - Write next byte to MBDR */
				 RegWrF(2,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
				 i2c_tx_buffer.length--;
                                 #ifdef DEBUG
				 printf("Master TX mode: Send Byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
                                 #endif
			     }
			 }
		     }
		 } 
	     }
	     else {
		 /* Master Receive Mode - Check if this is last byte to be read. */	    
		 if (i2c_rx_buffer.length == 1) {
		     printf("I2CR = 0x%02X\n", RegRd(2,IBCR));
		     /* Last byte to be read - 
			Generate Stop signal by changing to Slave Mode. */
		     RegWrF(2,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);

                     #ifdef DEBUG
		     printf("Master RX mode: All data received, send STOP.\n");
                     #endif
		     master_rx_done = TRUE;
		 }
		 else {
		     /* Not last byte to be read - Check if second to last byte. */
		     if (i2c_rx_buffer.length == 2) {
			 printf("I2CR S2L  = 0x%02X\n", RegRd(2,IBCR));
			 /* Second to last byte to be read - Set Transmit Acknowledge Enable
			    bit so no ACK is sent after the next byte is received, which
			    indicates "end of data" to the slave. */
			 RegWrF(2,IBCR, IBCR_NOACK_Data_Ack_Disable, e_NOACK_Ack_Not_Sent);

                         #ifdef DEBUG
			 printf("Master RX mode: Second-to-last byte received, set TXAK.\n");
                         #endif
		     }
		 }

		 /* Store received data in RX buffer */
		 i2c_rx_buffer.buf[i2c_rx_buffer.rx_index++] = RegRd(2,IBDR);
		 i2c_rx_buffer.length--;

                 #ifdef DEBUG
		 printf("Master RX mode: Receive byte - 0x%02X\n",i2c_rx_buffer.buf[i2c_rx_buffer.rx_index-1]);
                 #endif
	     }
	 }
	 else {
	     /* Slave Mode - Check if Arbitration Lost. */
	     if (RegRdF(2,IBSR,IBSR_IBAL_Arbitration_Lost)== TRUE) {
		 arb_lost[2]++;
                 #ifdef DEBUG
		 step("Arbitration Lost.\n");
                 #endif
		 /* Clear IAL bit */
		 RegWrF(2,IBSR, IBSR_IBAL_Arbitration_Lost, e_IBAL_Interrupt_Clear);

		 /* Arbitration Lost - Check if this device is being addressed as slave.
		    (If not, nothing more needs to be done.) */
		 if (RegRdF(2,IBSR,IBSR_IAAS_Addressed_as_Slave) == e_IAAS_Addressed_as_Slave) {
		     /* Addressed as slave - 
			Check if master was reading from slave or writing to slave. */
		     if (RegRdF(2,IBSR,IBSR_SRW_Slave_Read_Write_Mode)== e_SRW_Slave_Transmit) {
			 /* Set tx_index to 0 */
			 i2c_tx_buffer.tx_index = 0;

			 /* Master was reading from slave - Set Transmit Mode. */
			 RegWrF(2,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);

			 /* Write data to MBDR. */
			 RegWrF(2,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
                         #ifdef DEBUG
			 printf("Arbitration Lost: Addressed as slave - TX mode.\n");
                         #endif
		     }
		     else {
			 /* Set rx_index to 0 */
			 i2c_rx_buffer.rx_index = 0;

			 /* Master was writing to slave - Set Receive Mode. */
			 RegWrF(2,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);

			 /* Dummy read from MBDR, to clear the ICF bit. */
			 dummy_read = RegRd(2,IBDR);

                         #ifdef DEBUG
			 printf("Arbitration Lost: Addressed as slave - RX mode.\n");
                         #endif
		     }
		 }	    
	     }
	     else {
		 /* Arbitration Not Lost - Check if data byte is this devices's Slave Address byte. */
		 if (RegRdF(2,IBSR,IBSR_IAAS_Addressed_as_Slave) == e_IAAS_Addressed_as_Slave) {
		     /* Data byte is Slave Address byte - Check Slave Read/Write bit. */
		     if (RegRdF(2,IBSR,IBSR_SRW_Slave_Read_Write_Mode)== e_SRW_Slave_Transmit) {
			 /* Set tx_index to 0 */
			 i2c_tx_buffer.tx_index = 0;

			 /* Master was reading from slave - Set Transmit Mode. */
			 RegWrF(2,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);

			 /* Write data to MBDR. */
			 RegWrF(2,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
                         #ifdef DEBUG
			 printf("Slave TX: First byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
                         #endif
		     }
		     else {
			 /* Master has specified Slave Receive Mode.
			    Set Receive Mode.  (Writing to MBCR clears IAAS.) */

			 /* Set rx_index to 0 */
			 i2c_rx_buffer.rx_index = 0;

			 RegWrF(2,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);
			 /* Read address data from MBDR and store it. */
			 dummy_read = RegRd(2,IBDR);
                         #ifdef DEBUG
			 step("Slave RX: Receive address.\n");
			 #endif
		     }
		 }
		 else {
		     /* Data byte received is not Slave Address byte - 
			Check if this device is in Transmit or Receive Mode. */
		     if (RegRdF(2,IBCR,IBCR_Tx_Rxn_Transmit_Receive_Mode_Select) == e_Tx_Rxn_Transmit_Mode) {
			 /* Last byte received? */
			 if (RegRdF(2,IBSR,IBSR_RXAK_Receive_Acknowledge) == e_RXAK_No_Ack_Received) {
			     RegWrF(2,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);
			     dummy_read = RegRd(2,IBDR);

                             #ifdef DEBUG
			     printf("Slave TX: Last byte has been sent.\n");
                             #endif
			     slave_tx_done = TRUE;
			 }
			 else {
			     /* Write data to MBDR. */
			     RegWrF(2,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
			     i2c_tx_buffer.length--;
			     if(i2c_short_func_reset==1)
			     {
				 if(i2c_tx_buffer.length==5)
				     MC_RGM_soft_func_short(0);
			     }

                             #ifdef DEBUG
			     printf("Slave TX: Send byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
                             #endif
			 }
		     }
		     else {
			 if(i2c_rx_buffer.length == 1) {
			     slave_rx_done = TRUE;
			 }
			 /* Receive Mode - Read data from MBDR and store it. */
			 i2c_rx_buffer.buf[i2c_rx_buffer.rx_index++] = RegRd(2,IBDR);
			 i2c_rx_buffer.length--;
			 i2c_rx_buffer.data_present = TRUE;
			 //dummy_read = RegRd(2,IBDR);

                         #ifdef DEBUG
			 printf("Slave RX: Receive byte - 0x%02X\n",i2c_rx_buffer.buf[i2c_rx_buffer.rx_index-1]);
                         #endif
		     }
		 }
	     }
	 }
     }
    stop= 1;
    #ifdef DEBUG
    printf("exit from i2c2 isr");
    #endif
}
#endif

#if I2C_INSTANCES == 4
void i2c3_isr()
{
    
    /* Temp variable for dummy reads */
    uint8_t dummy_read;		

    #ifdef DEBUG
    printf("entered into i2c3 isr");
    #endif
    /* Clear the I2C Interrupt Flag. */
     WRITE(i2c_struct[3].I2C_n->IBSR.R , (1<<1));

     if(RegRdF(3,IBIC,IBIC_BIIE_Bus_Idle_Interrupt_Enable)==1 && RegRdF(3,IBSR,IBSR_IBB_Bus_Busy)== e_IBB_Bus_Idle){
	 bus_idle[3]++;
	 step("bus idle interrupt occured");
     }

     else {
    
	 /* Check if this device is in Master or Slave Mode. */
	 if (RegRdF(3,IBCR,IBCR_MS_SLn_Master_Slave_Mode_Select)== e_MS_SLn_Master)
	 {
	     /* Master Mode - Check if this device is in Transmit or Receive Mode. */
	     if (RegRdF(3,IBCR,IBCR_Tx_Rxn_Transmit_Receive_Mode_Select) == e_Tx_Rxn_Transmit_Mode) {
		 /* Master Transmit Mode - Check if last byte was tranmitted. */
		 if ((i2c_tx_buffer.length == 0) && (master_mode != I2C_RX))	{
		     /* Last byte was transmitted - 
			Generate Stop signal by changing to Slave Mode. */
		     /* If TXRX mode (Repeated Start), signal end of TX */
		     if (master_mode == I2C_TXRX)
			 master_tx_done = TRUE;
		     /* Issue STOP */
		     else {
			 RegWrF(3,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);
		         master_tx_done = TRUE;
		     }
                     #ifdef DEBUG
		     printf("Master TX mode: Last Byte\n");
                     #endif
		 }
		 else {
		     /* More bytes to be transmitted - Check if ACK received. */
		     if (RegRdF(3,IBSR,IBSR_RXAK_Receive_Acknowledge) == e_RXAK_No_Ack_Received){
			 /* ACK not received - Generate STOP */
			 RegWrF(3,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);
			 no_ack[3]++;
                         #ifdef DEBUG
			 step("Master TX mode: NAK received by master");
                         #endif
		     }
		     else {
			 /* Check if end of address cycle */
			 if (master_mode == I2C_RX) {
			     RegWrF(3,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);
			     
			     /*Set Transmit Acknowledge Enable bit so no ACK is sent after the only one byte is received, which
			       indicates "end of data" to the slave. */
			     if(i2c_rx_buffer.length == 1) {
				 RegWrF(3,IBCR, IBCR_NOACK_Data_Ack_Disable, e_NOACK_Ack_Not_Sent); //modified
			     }
			     dummy_read = RegRd(3,IBDR);
			     if(DMA_recv_enb == 1) {
				 RegWrF(3,IBCR,IBCR_DMAEN_DMA_Enable,e_ENABLE);
			     }
                             #ifdef DEBUG
			     printf("Master TX mode: End of RX address cycle, switch to RX mode.\n");
                             #endif
			 }
			 /* ACK received, send data */
			 else {
			     if(find_slave == 1) {
				 #ifdef DEBUG
				 step("Got Ack from slave");     
				 #endif
				 RegWrF(3,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);
			     }
			     else {
				 /* Not end of address cycle - Write next byte to MBDR */
				 RegWrF(3,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
				 i2c_tx_buffer.length--;
                                 #ifdef DEBUG
				 printf("Master TX mode: Send Byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
                                 #endif
			     }
			 }
		     }
		 } 
	     }
	     else {
		 /* Master Receive Mode - Check if this is last byte to be read. */	    
		 if (i2c_rx_buffer.length == 1) {
                     #ifdef DEBUG
		     printf("I2CR = 0x%02X\n", RegRd(3,IBCR));
                     #endif
		     /* Last byte to be read - 
			Generate Stop signal by changing to Slave Mode. */
		     RegWrF(3,IBCR, IBCR_MS_SLn_Master_Slave_Mode_Select, e_MS_SLn_Slave);

                     #ifdef DEBUG
		     printf("Master RX mode: All data received, send STOP.\n");
                     #endif
		     master_rx_done = TRUE;
		 }
		 else {
		     /* Not last byte to be read - Check if second to last byte. */
		     if (i2c_rx_buffer.length == 2) {
			 #ifdef DEBUG
			 printf("I2CR S2L  = 0x%02X\n", RegRd(3,IBCR));
                         #endif
			 /* Second to last byte to be read - Set Transmit Acknowledge Enable
			    bit so no ACK is sent after the next byte is received, which
			    indicates "end of data" to the slave. */
			 RegWrF(3,IBCR, IBCR_NOACK_Data_Ack_Disable, e_NOACK_Ack_Not_Sent);

                         #ifdef DEBUG
			 printf("Master RX mode: Second-to-last byte received, set TXAK.\n");
                         #endif
		     }
		 }

		 /* Store received data in RX buffer */
		 i2c_rx_buffer.buf[i2c_rx_buffer.rx_index++] = RegRd(3,IBDR);
		 i2c_rx_buffer.length--;

                 #ifdef DEBUG
		 printf("Master RX mode: Receive byte - 0x%02X\n",i2c_rx_buffer.buf[i2c_rx_buffer.rx_index-1]);
                 #endif
	     }
	 }
	 else {
	     /* Slave Mode - Check if Arbitration Lost. */
	     if (RegRdF(3,IBSR,IBSR_IBAL_Arbitration_Lost)== TRUE) {
		 arb_lost[3]++;
                 #ifdef DEBUG
		 step("Arbitration Lost.\n");
                 #endif
		 /* Clear IAL bit */
		 RegWrF(3,IBSR, IBSR_IBAL_Arbitration_Lost, e_IBAL_Interrupt_Clear);

		 /* Arbitration Lost - Check if this device is being addressed as slave.
		    (If not, nothing more needs to be done.) */
		 if (RegRdF(3,IBSR,IBSR_IAAS_Addressed_as_Slave) == e_IAAS_Addressed_as_Slave) {
		     /* Addressed as slave - 
			Check if master was reading from slave or writing to slave. */
		     if (RegRdF(3,IBSR,IBSR_SRW_Slave_Read_Write_Mode)== e_SRW_Slave_Transmit) {
			 /* Set tx_index to 0 */
			 i2c_tx_buffer.tx_index = 0;

			 /* Master was reading from slave - Set Transmit Mode. */
			 RegWrF(3,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);

			 /* Write data to MBDR. */
			 RegWrF(3,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
                         #ifdef DEBUG
			 printf("Arbitration Lost: Addressed as slave - TX mode.\n");
                         #endif
		     }
		     else {
			 /* Set rx_index to 0 */
			 i2c_rx_buffer.rx_index = 0;

			 /* Master was writing to slave - Set Receive Mode. */
			 RegWrF(3,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);

			 /* Dummy read from MBDR, to clear the ICF bit. */
			 dummy_read = RegRd(3,IBDR);

                         #ifdef DEBUG
			 printf("Arbitration Lost: Addressed as slave - RX mode.\n");
                         #endif
		     }
		 }	    
	     }
	     else {
		 /* Arbitration Not Lost - Check if data byte is this devices's Slave Address byte. */
		 if (RegRdF(3,IBSR,IBSR_IAAS_Addressed_as_Slave) == e_IAAS_Addressed_as_Slave) {
		     /* Data byte is Slave Address byte - Check Slave Read/Write bit. */
		     if (RegRdF(3,IBSR,IBSR_SRW_Slave_Read_Write_Mode)== e_SRW_Slave_Transmit) {
			 /* Set tx_index to 0 */
			 i2c_tx_buffer.tx_index = 0;

			 /* Master was reading from slave - Set Transmit Mode. */
			 RegWrF(3,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Transmit_Mode);

			 /* Write data to MBDR. */
			 RegWrF(3,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
                         #ifdef DEBUG
			 printf("Slave TX: First byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
                         #endif
		     }
		     else {
			 /* Master has specified Slave Receive Mode.
			    Set Receive Mode.  (Writing to MBCR clears IAAS.) */

			 /* Set rx_index to 0 */
			 i2c_rx_buffer.rx_index = 0;

			 RegWrF(3,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);
			 /* Read address data from MBDR and store it. */
			 dummy_read = RegRd(3,IBDR);
                         #ifdef DEBUG
			 step("Slave RX: Receive address.\n");
                         #endif
		     }
		 }
		 else {
		     /* Data byte received is not Slave Address byte - 
			Check if this device is in Transmit or Receive Mode. */
		     if (RegRdF(3,IBCR,IBCR_Tx_Rxn_Transmit_Receive_Mode_Select) == e_Tx_Rxn_Transmit_Mode) {
			 /* Last byte received? */
			 if (RegRdF(3,IBSR,IBSR_RXAK_Receive_Acknowledge) == e_RXAK_No_Ack_Received) {
			     RegWrF(3,IBCR, IBCR_Tx_Rxn_Transmit_Receive_Mode_Select, e_Tx_Rxn_Receive_Mode);
			     dummy_read = RegRd(3,IBDR);

                             #ifdef DEBUG
			     printf("Slave TX: Last byte has been sent.\n");
                             #endif
			     slave_tx_done = TRUE;
			 }
			 else {
			     /* Write data to MBDR. */
			     RegWrF(3,IBDR,IBDR_Data,i2c_tx_buffer.buf[i2c_tx_buffer.tx_index++]);
			     i2c_tx_buffer.length--;
			     if(i2c_short_func_reset==1)
			     {
				 if(i2c_tx_buffer.length==5)
				     MC_RGM_soft_func_short(0);
			     }

                             #ifdef DEBUG
			     printf("Slave TX: Send byte - 0x%02X\n",i2c_tx_buffer.buf[i2c_tx_buffer.tx_index-1]);
                             #endif
			 }
		     }
		     else {
			 if(i2c_rx_buffer.length == 1) {
			     slave_rx_done = TRUE;
			 }
			 /* Receive Mode - Read data from MBDR and store it. */
			 i2c_rx_buffer.buf[i2c_rx_buffer.rx_index++] = RegRd(3,IBDR);
			 i2c_rx_buffer.length--;
			 i2c_rx_buffer.data_present = TRUE;
			 //dummy_read = RegRd(2,IBDR);

                         #ifdef DEBUG
			 printf("Slave RX: Receive byte - 0x%02X\n",i2c_rx_buffer.buf[i2c_rx_buffer.rx_index-1]);
                         #endif
		     }
		 }
	     }
	 }
     }
  
    stop= 1;
    #ifdef DEBUG
    printf("exit from i2c3 isr");
    #endif
    
}
#endif

void i2c_reset_variables(uint8_t device_num)
{
    bus_idle[device_num] = 0;
    no_ack[device_num] = 0;
    arb_lost[device_num] = 0;
}


