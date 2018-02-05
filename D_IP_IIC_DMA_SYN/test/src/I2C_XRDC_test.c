#include "common.h"
#include "xrdc_soc_config.h"
#include "xrdc_api.h"
#ifdef CORE_M4_ENABLE
#include "nvic_m4_0_soc_config.h"
#endif

extern xrdc_memory_access_config_struct xrdc_memory_access_conf;
extern xrdc_peripheral_access_config_struct xrdc_peripheral_access_conf;
extern xrdc_processID_config_struct xrdc_processID_conf;
extern xrdc_domainID_config_struct xrdc_domainID_conf;
extern xrdc_read_access_fault_flag,xrdc_write_access_fault_flag,xrdc_instruction_access_fault_flag,xrdc_dummy_address ;
vuint16_t err_count;

void I2C_XRDC_test()
{
#ifdef EN_I2C_XRDC_test
     uint32_t slot;
     vuint32_t read_val,write_val,xrdc_instance=0,register_reset_value,register_write_mask_value;
     xrdc_init();
     xrdc_domainID_conf.xrdc_mdac_instance = XRDC_M4;
#if defined CORE_M4_ENABLE
     intc_install_exception(BUS_FAULT,xrdc_bus_fault_handler);
#elif defined CORE_A5_ENABLE
     intc_install_exception(DATA_ABORT,xrdc_bus_fault_handler);
#endif

     for(slot=0;slot<3;slot++) {
     
	 step("XRDC protection check for OCOTP slot %d ",slot);

	 if(slot == 0) {
	     xrdc_peripheral_access_conf.xrdc_pdac_instance = 81;      //Select your respective IP
	     xrdc_dummy_address = (uint32_t )&I2C_0 + 0x1;// register to be written
	     register_reset_value = 0x0; //Reset value of the the register where read write operation is done
	     register_write_mask_value=0xff; // write mask value for the register where read write operation is done
	 }
	 else if(slot == 1) {
	     xrdc_peripheral_access_conf.xrdc_pdac_instance = 184;      //Select your respective IP
	     xrdc_dummy_address = (uint32_t )&I2C_1 + 0x1;// register to be written
	     register_reset_value = 0x0; //Reset value of the the register where read write operation is done
	     register_write_mask_value=0xff; // write mask value for the register where read write operation is done
	 }
	 else {
	     xrdc_peripheral_access_conf.xrdc_pdac_instance = 186;      //Select your respective IP
	     xrdc_dummy_address = (uint32_t )&I2C_2 + 0x1;// register to be written
	     register_reset_value = 0x0; //Reset value of the the register where read write operation is done
	     register_write_mask_value=0xff; // write mask value for the register where read write operation is done
	 }

	 if(xrdc_peripheral_access_conf.xrdc_pdac_instance<127)
	     xrdc_peripheral_access_conf.xrdc_pac_instance =0;
	 else if(xrdc_peripheral_access_conf.xrdc_pdac_instance<255)
	     xrdc_peripheral_access_conf.xrdc_pac_instance =1;
	 else if(xrdc_peripheral_access_conf.xrdc_pdac_instance<383)
	     xrdc_peripheral_access_conf.xrdc_pac_instance =2;

	 /* Check for all type of accesses */
	 for(xrdc_peripheral_access_conf.xrdc_access_control_type = 0;xrdc_peripheral_access_conf.xrdc_access_control_type<3;xrdc_peripheral_access_conf.xrdc_access_control_type++)
	 {
	     read_val = 0xFF;
	     write_val = 0xFF;
	     xrdc_read_access_fault_flag = 0;
	     xrdc_write_access_fault_flag= 0;

	     xrdc_peripheral_access_config(0x0);

	     xrdc_enable(0x0);

	     step("Making read access...at 0x%x",xrdc_dummy_address);
	     read_val = *(uint32_t *)xrdc_dummy_address;
	     step("Making write access...at 0x%x",xrdc_dummy_address);
	     *(uint32_t *)xrdc_dummy_address = write_val;

	     //xrdc_bus_fault_handler();

	     xrdc_disable(0x0);

	     if(xrdc_peripheral_access_conf.xrdc_access_control_type == XRDC_NO_ACCESS)
	     {
		 if(xrdc_read_access_fault_flag)
		     printf("Exception generated in read access when read permission is not granted for XRDC_%d  %d times",xrdc_instance,xrdc_read_access_fault_flag);
		 else
		     error("Exception not generated in read access when read permission is not granted for XRDC_%d  %d times",xrdc_instance,xrdc_read_access_fault_flag);

		 if(xrdc_write_access_fault_flag)
		     printf("Exception generated in write access when write permission is not granted for XRDC_%d  %d times",xrdc_instance,xrdc_write_access_fault_flag);
		 else
		     error("Exception not generated in write access when write permission is not granted for XRDC_%d  %d times",xrdc_instance,xrdc_write_access_fault_flag);

		 if(read_val == *(uint32_t *)xrdc_dummy_address)
		     error("Read access was successfull with data=0x%x",read_val);
		 else
		     printf("Read access was not successfull with data=0x%x",read_val);

		 if(*(uint32_t *)xrdc_dummy_address == write_val)
		     error("Write access was successfull with data=0x%x",*(uint32_t *)xrdc_dummy_address);
		 else
		     printf("Write access was not successfull with data=0x%x",*(uint32_t *)xrdc_dummy_address);
	     }
	     else if(xrdc_peripheral_access_conf.xrdc_access_control_type == XRDC_READ_ONLY)
	     {
		 if(xrdc_read_access_fault_flag)
		     error("Exception generated in read access when read permission is granted for XRDC_%d  %d times",xrdc_instance,xrdc_read_access_fault_flag);
		 else
		     printf("Exception not generated in read access when read permission is granted for XRDC_%d  %d times",xrdc_instance,xrdc_read_access_fault_flag);

		 if(xrdc_write_access_fault_flag)
		     printf("Exception generated in write access when write permission is not granted for XRDC_%d  %d times",xrdc_instance,xrdc_write_access_fault_flag);
		 else
		     error("Exception not generated in write access when write permission is not granted for XRDC_%d  %d times",xrdc_instance,xrdc_write_access_fault_flag);

		 if(read_val == *(uint32_t *)xrdc_dummy_address)
		     printf("Read access was successfull with data=0x%x",read_val);
		 else
		     error("Read access was not successfull with data=0x%x",read_val);

		 if(*(uint32_t *)xrdc_dummy_address == write_val)
		     error("Write access was successfull with data=0x%x",*(uint32_t *)xrdc_dummy_address);
		 else
		     printf("Write access was not successfull with data=0x%x",*(uint32_t *)xrdc_dummy_address);
	     }
	     else
	     {
		 if(xrdc_read_access_fault_flag)
		     error("Exception generated in read access when read permission is granted for XRDC_%d  %d times",xrdc_instance,xrdc_read_access_fault_flag);
		 else
		     printf("Exception not generated in read access when read permission is granted for XRDC_%d  %d times",xrdc_instance,xrdc_read_access_fault_flag);

		 if(xrdc_write_access_fault_flag)
		     error("Exception generated in write access when write permission is granted for XRDC_%d  %d times",xrdc_instance,xrdc_write_access_fault_flag);
		 else
		     printf("Exception not generated in write access when write permission is granted for XRDC_%d  %d times",xrdc_instance,xrdc_write_access_fault_flag);

		 if((read_val & 0xff) ==register_reset_value)
		     printf("Read access was successfull with data=0x%x",read_val);
		 else
		     error("Read access was not successfull with data=0x%x",read_val);

		 if(((*(uint32_t *)xrdc_dummy_address & 0xff) == register_write_mask_value) )
		     printf("Write access was successfull with data=0x%x",*(uint32_t *)xrdc_dummy_address);
		 else
		     error("Write access was not successfull with data=0x%x",*(uint32_t *)xrdc_dummy_address);
	     }
	 }
     }

#else
    error("NA for this NPI");
#endif
}
