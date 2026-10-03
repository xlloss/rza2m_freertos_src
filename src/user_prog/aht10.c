#include <stdio.h>
#include <string.h>
#include "r_typedefs.h"
#include "r_riic_drv_api.h"
#include "r_os_abstraction_api.h"
#include "aht10.h"

static int aht10_i2c_write(int_t handle, uint8_t *p_buf, uint32_t len)
{
    st_r_drv_riic_transfer_t xfer;
    
    xfer.device_address = AHT10_I2C_ADDR;
    xfer.sub_address_type = RIIC_SUB_ADDR_NONE; /* 關閉 Sub-Address */
    xfer.sub_address = 0;
    xfer.number_of_bytes = len;
    xfer.p_data_buffer = p_buf;
    
    if (DRV_SUCCESS != control(handle, CTL_RIIC_WRITE, &xfer))
    {
        return -1;
    }
    return 0;
}

static int aht10_i2c_read(int_t handle, uint8_t *p_buf, uint32_t len)
{
    st_r_drv_riic_transfer_t xfer;
    
    xfer.device_address = AHT10_I2C_ADDR;
    xfer.sub_address_type = RIIC_SUB_ADDR_NONE; /* 關閉 Sub-Address */
    xfer.sub_address = 0;
    xfer.number_of_bytes = len;
    xfer.p_data_buffer = p_buf;
    
    if (DRV_SUCCESS != control(handle, CTL_RIIC_READ, &xfer))
    {
        return -1;
    }
    return 0;
}

int aht10_soft_reset(int_t riic_handle)
{
    uint8_t cmd = AHT10_CMD_SOFT_RESET;
    
    if (aht10_i2c_write(riic_handle, &cmd, 1) < 0)
    {
        return -1;
    }
    R_OS_TaskSleep(20); /* Soft reset takes <= 20ms */
    return 0;
}

int aht10_init(int_t riic_handle)
{
    uint8_t status = 0;
    uint8_t init_cmd[3] = {AHT10_CMD_INIT, 0x08, 0x00};
    
    R_OS_TaskSleep(25); /* Wait for power on */
    
    if (aht10_i2c_read(riic_handle, &status, 1) < 0)
    {
        return -1;
    }
    
    /* Check calibration enable bit Bit[3] */
    if ((status & AHT10_STATUS_CAL_MASK) == 0)
    {
        if (aht10_i2c_write(riic_handle, init_cmd, 3) < 0)
        {
            return -2;
        }
        R_OS_TaskSleep(15);
    }
    return 0;
}

int aht10_read_data(int_t riic_handle, aht10_data_t *p_data)
{
    uint8_t trig_cmd[3] = {AHT10_CMD_TRIGGER, 0x33, 0x00};
    uint8_t rx_buf[6] = {0};
    uint32_t raw_humi = 0;
    uint32_t raw_temp = 0;
    
    if (NULL == p_data)
    {
        return -1;
    }
    
    /* 1. Trigger measurement */
    if (aht10_i2c_write(riic_handle, trig_cmd, 3) < 0)
    {
        return -2;
    }
    
    /* 2. Conversion delay (>=75ms) */
    R_OS_TaskSleep(80);
    
    /* 3. Read 6 bytes of data */
    if (aht10_i2c_read(riic_handle, rx_buf, 6) < 0)
    {
        return -3;
    }
    
    /* 4. Check Busy status */
    if (rx_buf[0] & AHT10_STATUS_BUSY_MASK)
    {
        return -4;
    }
    
    /* 5. Concatenate 20-bit raw data */
    raw_humi = (((uint32_t)rx_buf[1] << 12) | 
                ((uint32_t)rx_buf[2] << 4)  | 
                ((uint32_t)rx_buf[3] >> 4));
                
    raw_temp = ((((uint32_t)rx_buf[3] & 0x0F) << 16) | 
                ((uint32_t)rx_buf[4] << 8)  | 
                (uint32_t)rx_buf[5]);
                
    /* 6. Calculate physical values */
    p_data->humidity    = ((float)raw_humi * 100.0f) / 1048576.0f;
    p_data->temperature = (((float)raw_temp * 200.0f) / 1048576.0f) - 50.0f;
    
    return 0;
}
