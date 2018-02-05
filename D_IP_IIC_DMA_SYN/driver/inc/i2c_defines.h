#ifndef I2C_DEFINES_H_
#define I2C_DEFINES_H_


#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* I2C parameters */
#define I2C_BUFFER_SIZE	(50)
#define I2C_TXRX	(2)
#define I2C_TX		(0)
#define	I2C_RX		(1)
#define iPORT		(0x6E)

//#define DEBUG

typedef enum 
{
    e_I2C_MASTER = 0,
    e_I2C_SLAVE  = 1
}i2c_type;

/* Global definition of "enum" for Enable/Disable */
enum 
{
    e_DISABLE =	0,
    e_ENABLE  =	1
};

/* Global definition of "enum" for status bits in registers. */
enum
{
    e_NOT_SET =	0,
    e_SET     =	1
};

/* ===================== */
/* IBAD : I2C Bus Address Register */
/* ===================== */

#define IBAD_ADR_Slave_Address   	0xfe
#define IBAD_ADR_Slave_Address_Shift	0//temp
#define Shift_IBAD_ADR_Slave_Address	0//temp
typedef enum 
{
    e_ADR_Slave_ADR1 = 0x62,
    e_ADR_Slave_ADR2 = 0x60,
    e_ADR_Slave_ADR3 = 0x44
//	...
}ADR_Random_Slave_Addresses;

/* ============================ */
/* IBFD : I2C Bus Frequency Divider Register  */
/* ============================ */
#define IBFD_IBC_Bus_Clk_Rate        0xff
#define IBFD_IBC_Bus_Clk_Rate_Shift  0
#define Shift_IBFD_IBC_Bus_Clk_Rate  0
typedef enum
{
    e_Bit_Rate_4_8KHz_MUL4    = 0xBF,
    e_Bit_Rate_9_7KHz         = 0x33,
    e_Bit_Rate_19_5KHz_MUL1   =	0x3F,	
    e_Bit_Rate_234KHz_MUL1    =	0x25,
    e_Bit_Rate_260KHz_MUL1    =	0x24,
    e_Bit_Rate_334KHz_MUL1    =	0x22,
    e_Bit_Rate_937_5KHz_MUL1  =	0x14,
    e_Bit_Rate_1MHz_MUL1      =	0x13,
    e_Bit_Rate_3_7MHz_MUL1    =	0x00
    
}Bit_Rates_for_Testing;

#define IBFD_IBC_MUL_Bus_Multiplier_Factor         0xC0
#define IBFD_IBC_MUL_Bus_Multiplier_Factor_Shift   6
#define Shift_IBFD_IBC_MUL_Bus_Multiplier_Factor   6
typedef enum
{
    e_MUL_Multiply_by_1	 = 0,
    e_MUL_Multiply_by_2	 = 1,
    e_MUL_Multiply_by_4  = 2
}MUL_Multiplication_Factors;

#define IBFD_IBC_Prescaler_Divider_Value	   0x38
#define IBFD_IBC_Prescaler_Divider_Value_Shift	   3
#define Shift_IBFD_IBC_Prescaler_Divider_Value	   3
typedef enum
{
    e_scl2start_scl2stop_scl2tap_tap2tap_clks_for_Div0	 = 0,
    e_scl2start_scl2stop_scl2tap_tap2tap_clks_for_Div1   = 1,
    e_scl2start_scl2stop_scl2tap_tap2tap_clks_for_Div2	 = 2,
    e_scl2start_scl2stop_scl2tap_tap2tap_clks_for_Div3	 = 3,
    e_scl2start_scl2stop_scl2tap_tap2tap_clks_for_Div4	 = 4,
    e_scl2start_scl2stop_scl2tap_tap2tap_clks_for_Div5	 = 5,
    e_scl2start_scl2stop_scl2tap_tap2tap_clks_for_Div6	 = 6,	
    e_scl2start_scl2stop_scl2tap_tap2tap_clks_for_Div7	 = 7
}Prescaler_Divider_Values;

#define IBFD_IBC_Bus_Tap_and_Prescale_Value	    0x7
#define IBFD_IBC_Bus_Tap_and_Prescale_Value_Shift   0
#define Shift_IBFD_IBC_Bus_Tap_and_Prescale_Value   0
typedef enum
{
    e_SCL_Tap_SDA_Tap_clks_for_Val0  = 0,
    e_SCL_Tap_SDA_Tap_clks_for_Val1  = 1,
    e_SCL_Tap_SDA_Tap_clks_for_Val2  = 2,
    e_SCL_Tap_SDA_Tap_clks_for_Val3  = 3,
    e_SCL_Tap_SDA_Tap_clks_for_Val4  = 4,
    e_SCL_Tap_SDA_Tap_clks_for_Val5  = 5,
    e_SCL_Tap_SDA_Tap_clks_for_Val6  = 6,
    e_SCL_Tap_SDA_Tap_clks_for_Val7  = 7
}Tap_and_Prescale_Values;

/* ===================== */
/* IBCR : I2C Bus Control Register  */
/* ===================== */

#define IBCR_MDIS_Module_Disable_bit	       0x80
#define IBCR_MDIS_Module_Disable_bit_Shift     7
#define Shift_IBCR_MDIS_Module_Disable_bit     7
typedef enum 
{
    e_MDIS_Module_Enabled     =	0,
    e_MDIS_Reset_n_Disabled   = 1	
}MDIS_Module_Disable_bit_options;

#define IBCR_IBIE_IBUS_Interrupt_Enable 	0x40
#define IBCR_IBIE_IBUS_Interrupt_Enable_Shift   6
#define Shift_IBCR_IBIE_IBUS_Interrupt_Enable   6

#define IBCR_MS_SLn_Master_Slave_Mode_Select         0x20
#define IBCR_MS_SLn_Master_Slave_Mode_Select_Shift   5
#define Shift_IBCR_MS_SLn_Master_Slave_Mode_Select   5
typedef enum 
{
    e_MS_SLn_Slave   =	0,
    e_MS_SLn_Master  =  1	
}MS_SLn_Master_Slave_Mode_Select;

#define IBCR_Tx_Rxn_Transmit_Receive_Mode_Select	0x10
#define IBCR_Tx_Rxn_Transmit_Receive_Mode_Select_Shift	4
#define Shift_IBCR_Tx_Rxn_Transmit_Receive_Mode_Select	4
typedef enum 
{
    e_Tx_Rxn_Receive_Mode   = 0,
    e_Tx_Rxn_Transmit_Mode  = 1
}Tx_Rxn_Transmit_Receive_Mode_Select;

#define IBCR_NOACK_Data_Ack_Disable	     0x08
#define IBCR_NOACK_Data_Ack_Disable_Shift    3
#define Shift_IBCR_NOACK_Data_Ack_Disable    3
typedef enum 
{
    e_NOACK_Ack_Sent	  = 0,
    e_NOACK_Ack_Not_Sent  = 1
}NOACK_Bit_info;

#define IBCR_RSTA_Repeat_Start_Master_Only          0x4
#define IBCR_RSTA_Repeat_Start_Master_Only_Shift    2
#define Shift_IBCR_RSTA_Repeat_Start_Master_Only    2

#define IBCR_DMAEN_DMA_Enable           0x2
#define IBCR_DMAEN_DMA_Enable_Shift	1
#define Shift_IBCR_DMAEN_DMA_Enable	1

/* ===================== */
/* IBSR : I2C Bus Status Register   */
/* ===================== */

#define IBSR_TCF_Transfer_Complete          0x80
#define IBSR_TCF_Transfer_Complete_Shift    7
#define Shift_IBSR_TCF_Transfer_Complete    7
typedef enum 
{
    e_TCF_Transfer_In_Progress  =  0,
    e_TCF_Transfer_Complete     =  1	
}TCF_Bit_info;

#define IBSR_IAAS_Addressed_as_Slave	     0x40
#define IBSR_IAAS_Addressed_as_Slave_Shift   6
#define Shift_IBSR_IAAS_Addressed_as_Slave   6
typedef enum 
{
    e_IAAS_Not_Addressed        =  0,
    e_IAAS_Addressed_as_Slave	=  1
}IAAS_Bit_info;

#define IBSR_IBB_Bus_Busy	  0x20
#define IBSR_IBB_Bus_Busy_Shift	  5
#define Shift_IBSR_IBB_Bus_Busy	  5
typedef enum 
{
    e_IBB_Bus_Idle   =	0,
    e_IBB_Bus_Busy   =  1	
}IBB_Bit_info;

#define IBSR_IBAL_Arbitration_Lost         0x10
#define IBSR_IBAL_Arbitration_Lost_Shift   4
#define Shift_IBSR_IBAL_Arbitration_Lost   4
typedef enum
{
    e_IBAL_Interrupt_Clear  = 1
}IBAL_INTR_Clear_Info;


#define IBSR_SRW_Slave_Read_Write_Mode		0x4
#define IBSR_SRW_Slave_Read_Write_Mode_Shift	2
#define Shift_IBSR_SRW_Slave_Read_Write_Mode	2
typedef enum
{
    e_SRW_Slave_Receive    = 0,	
    e_SRW_Slave_Transmit   = 1
}SRW_bit_info;

#define IBSR_IBIF_Bus_Interrupt_Flag	     0x2
#define IBSR_IBIF_Bus_Interrupt_Flag_Shift   1
#define Shift_IBSR_IBIF_Bus_Interrupt_Flag   1
typedef enum
{
    e_IBIF_Interrupt_Clear  =  1
}IBIF_INTR_Clear_Info;

#define IBSR_RXAK_Receive_Acknowledge		0x1
#define IBSR_RXAK_Receive_Acknowledge_Shift	0,
#define Shift_IBSR_RXAK_Receive_Acknowledge	0
typedef enum	
{
    e_RXAK_Ack_Received	     = 0,	
    e_RXAK_No_Ack_Received   = 1
}RXAK_bit_info;

/* ===================== */
/* IBDR : I2C Bus Data Register     */
/* ===================== */

#define IBDR_Data	   0xff
#define IBDR_Data_Shift	   0
#define Shift_IBDR_Data    0

typedef enum 
{
    e_Slave_Addr1_Read_Transfer      =	0x63,
    e_Slave_Addr1_Write_Transfer     =	0x62,
    e_Slave_Addr2_Read_Transfer	     =	0x61,
    e_Slave_Addr2_Write_Transfer     =	0x60,
    e_Slave_Addr3_Read_Transfer	     =	0x45,
    e_Slave_Addr3_Write_Transfer     =	0x44
}Slave_Addresses_with_RW;

typedef enum 
{
    e_Data_from_Slave	= 0x67
}Data_Sent_By_Slave;

typedef enum
{
    e_Data_1_from_Master   = 0x25,
    e_Data_2_from_Master   = 0x44
}Data_sent_by_Master;

/* ========================== */
/* IBIC : I2C Bus Interrupt Config Register */
/* ========================== */

#define IBIC_BIIE_Bus_Idle_Interrupt_Enable           0x80
#define IBIC_BIIE_Bus_Idle_Interrupt_Enable_Shift     7
#define Shift_IBIC_BIIE_Bus_Idle_Interrupt_Enable     7

#define IBIC_BYTERXIE_BYTERX_Interrupt_Enable         0x40
#define IBIC_BYTERXIE_BYTERX_Interrupt_Enable_Shift     6
#define Shift_IBIC_BYTERXIE_BYTERX_Interrupt_Enable    	6

#define IBDBG_BYTERX_Status         			0x04
#define IBDBG_BYTERX_Status_Shift    			2
#define Shift_IBDBG_BYTERX_Status		    	2

#define IBAD_reset  0x00
#define IBFD_reset  0x00
#define IBCR_reset  0x80
#define IBSR_reset  0x80
#define IBDR_reset  0x00
#define IBIC_reset  0x00
#define IBDBG_reset  0x00

unsigned long I2C_VAL;

#define RegWrF(n,x,y,z)  {I2C_VAL = (READ_INDEX(n,i2c_struct[n].I2C_n->x.R) & ~y)|(z<<Shift_##y);\
WRITE_INDEX(n,i2c_struct[n].I2C_n->x.R,I2C_VAL);}    //{I2C.x.R = ((I2C.x.R  & ~y) | (z << Shift_##y)); } 	// Macro for writing to register 	

#define RegRdF(n,x,y)    ((READ_INDEX(n,i2c_struct[n].I2C_n->x.R) & y))>>Shift_##y
//((I2C.x.R & y) >> Shift_##y

#define RegWr(n,x,y)      {WRITE_INDEX(n,i2c_struct[n].I2C_n->x.R,y);}//{I2C.x.R = y;}

#define RegRd(n,x)        READ_INDEX(n,i2c_struct[n].I2C_n->x.R)
//(I2C.x.R) 

#define NO_OF_DATA 21

#endif
