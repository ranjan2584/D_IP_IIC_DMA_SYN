#ifndef I2C_SOC_CONFIG_H_
#define I2C_SOC_CONFIG_H_

#include "common.h"
#ifdef CORE_M4_ENABLE
#include "nvic_m4_0_soc_config.h"
#elif defined CORE_A530 || defined CORE_A531 || defined CORE_A532 || defined CORE_A533
#include "gic_a5x_soc_config.h"
#elif defined CORE_A5_ENABLE
#include "gic_a5_0_soc_config.h"
#elif defined CORE_M0_ENABLE
#include "nvic_m0_0_soc_config.h"
#endif
#include "edma_soc_config.h"
#include "mc_me_api.h"
#include "i2c_defines.h"
#include "i2c_api.h"

#define I2C_INSTANCES 3

typedef volatile struct I2C_tag  *I2C_PTR;
typedef struct
{
    I2C_PTR I2C_n;
    char *name;
    uint8_t intr_vec;
    uint8_t tx_dma_src;
    uint8_t rx_dma_src;
    uint8_t pctl_no;

} I2C_STRUCT;

extern I2C_STRUCT i2c_struct[];

/* Structure for storing I2C transfer data */
typedef struct {
    int tx_index;			            /* TX index */
    int rx_index;			            /* RX index */
    volatile int data_present;			        /* Data present flag */
    int16_t length;			        /* Length of the buffer in bytes */
    uint8_t buf[I2C_BUFFER_SIZE];	/* Data buffer */
} I2C_BUFFER;

/////////////////////////////////////
/*Fill Soc specific Macro*/
//i2c0
#if defined CORE_M4_ENABLE
#define I2C0_INTR_VEC       INT_I2C0
#elif defined CORE_A53_ENABLE
#define I2C0_INTR_VEC       INT_I2C_0
#endif
#define I2C0_TX_DMA_SRC     IIC_0_Transmit
#define I2C0_RX_DMA_SRC     IIC_0_Receive
#define I2C0_PCTL_NO        PCTL_IIC0

#define I2C0_SDA_MSCR_OPT0   15
#define I2C0_SDA_IMCR_OPT0   781
#define I2C0_SCL_MSCR_OPT0   16
#define I2C0_SCL_IMCR_OPT0   780

#define I2C0_SDA_MSCR_OPT1   76
#define I2C0_SDA_IMCR_OPT1   
#define I2C0_SCL_MSCR_OPT1   77
#define I2C0_SCL_IMCR_OPT1   

#define I2C0_SDA_MSCR_OPT2   99
#define I2C0_SDA_IMCR_OPT2   781
#define I2C0_SCL_MSCR_OPT2   100
#define I2C0_SCL_IMCR_OPT2   780

//i2c1
#if defined CORE_M4_ENABLE
#define I2C1_INTR_VEC       INT_I2C1
#elif defined CORE_A53_ENABLE
#define I2C1_INTR_VEC       INT_I2C_1
#endif
#define I2C1_TX_DMA_SRC     IIC_1_Transmit
#define I2C1_RX_DMA_SRC     IIC_1_Receive
#define I2C1_PCTL_NO        PCTL_IIC1

#define I2C1_SDA_MSCR_OPT0   17
#define I2C1_SDA_IMCR_OPT0   783
#define I2C1_SCL_MSCR_OPT0   18
#define I2C1_SCL_IMCR_OPT0   782

#define I2C1_SDA_MSCR_OPT1   78
#define I2C1_SDA_IMCR_OPT1   
#define I2C1_SCL_MSCR_OPT1   79
#define I2C1_SCL_IMCR_OPT1   

#define I2C1_SDA_MSCR_OPT2   101
#define I2C1_SDA_IMCR_OPT2   783
#define I2C1_SCL_MSCR_OPT2   102
#define I2C1_SCL_IMCR_OPT2   782

//i2c2
#if defined CORE_M4_ENABLE
#define I2C2_INTR_VEC       INT_I2C2
#elif defined CORE_A53_ENABLE
#define I2C2_INTR_VEC       INT_I2C_2
#endif
#define I2C2_TX_DMA_SRC     IIC_2_Transmit
#define I2C2_RX_DMA_SRC     IIC_2_Receive
#define I2C2_PCTL_NO        PCTL_IIC0

#define I2C2_SDA_MSCR_OPT0   19
#define I2C2_SDA_IMCR_OPT0   785
#define I2C2_SCL_MSCR_OPT0   20
#define I2C2_SCL_IMCR_OPT0   784
/////////////////////////////////////

#endif
