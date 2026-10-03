/******************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only
 * intended for use with Renesas products. No other uses are authorized. This
 * software is owned by Renesas Electronics Corporation and is protected under
 * all applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
 * LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
 * TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
 * ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
 * FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
 * ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
 * BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this
 * software and to discontinue the availability of this software. By using this
 * software, you agree to the additional terms and conditions found by
 * accessing the following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2018 Renesas Electronics Corporation. All rights reserved.
 *****************************************************************************/
/******************************************************************************
 * File Name    : main.c
 * Device(s)    : RZ/A2M
 * Tool-Chain   : e2Studio Ver 7.4.0
 *              : GCC ARM Embedded 6.3.1.20170620
 * OS           : None
 * H/W Platform : RZ/A2M Evaluation Board
 * Description  : RZ/A2M Sample Program - Main
 * Operation    :
 * Limitations  :
******************************************************************************/

/******************************************************************************
 Includes   <System Includes> , "Project Includes"
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "r_typedefs.h"
#include "iodefine.h"
#include "r_cpg_drv_api.h"
#include "r_ostm_drv_api.h"
#include "r_scifa_drv_api.h"
#include "r_gpio_drv_api.h"
#include "r_startup_config.h"
#include "compiler_settings.h"
#include "main.h"
#include "r_os_abstraction_api.h"
#include "r_task_priority.h"
#include "command.h"
#include "r_eeprom_sample.h"
#include "aht10.h"
#include "r_riic_drv_api.h"
#include "r_rza2m_riic_lld_api.h"
#include "iodefine.h"

/******************************************************************************
Typedef definitions
******************************************************************************/


/******************************************************************************
Macro definitions
******************************************************************************/
#define MAIN_PRV_LED_ON     (1)
#define MAIN_PRV_LED_OFF    (0)

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/


/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/

/******************************************************************************
Private global variables and functions
******************************************************************************/
static uint32_t gs_main_led_flg;      /* LED lighting/turning off */
static int_t gs_my_gpio_handle;

/* Casting the pointer to a r_gpio_port_pin_t
 * type for setting of GPIO register address */
static st_r_drv_gpio_pin_rw_t gs_p60_hi = { GPIO_PORT_6_PIN_0,
                                            GPIO_LEVEL_HIGH,
                                            GPIO_SUCCESS };

/* Casting the pointer to a r_gpio_port_pin_t
 * type for setting of GPIO register address */
static st_r_drv_gpio_pin_rw_t gs_p60_lo = { GPIO_PORT_6_PIN_0,
                                            GPIO_LEVEL_LOW,
                                            GPIO_SUCCESS };
static const r_gpio_port_pin_t gs_led_pin_list[] =
{
    /* Casting the pointer to a r_gpio_port_pin_t
     * type for setting of GPIO register address */
    GPIO_PORT_6_PIN_0,
};

/* Terminal window escape sequences */
static const char_t * const gsp_clear_screen = "\x1b[2J";
static const char_t * const gsp_cursor_home  = "\x1b[H";

static int_t init_eeprom(void);

/******************************************************************************
* Function Name: os_console_task_t
* Description  : console task
* Arguments    : none
* Return Value : 0
******************************************************************************/
int_t os_console_task_t(void)
{
    /* never exits */

    printf("\r\n");
    printf("Command list:\r\n");
    printf("write - write data to EEPROM\r\n");
    printf("read  - read data from EEPROM\r\n");
    printf("help  - show the command desctiption\r\n");
    printf("ver   - show the version information\r\n");
    printf("exit  - shut down the sample program\r\n");
    printf("\r\n");
    while(1)
    {
        /* ==== Receive command, activate sample software ==== */
        cmd_console(stdin, stdout, "SAMPLE>");

        R_OS_TaskSleep(500);
    }
}
/******************************************************************************
 End of function os_console_task_t
 *****************************************************************************/

void aht10_demo_task(void *p_param)
{
    int_t riic_handle = -1;
    st_riic_config_t riic_cfg;
    aht10_data_t sensor_data;

    /* 1. Open RIIC driver (e.g., Channel 3) */
    riic_handle = open("\\\\.\\riic3", O_RDWR);
    if (riic_handle < 0)
    {
        printf("[ERROR] Failed to open RIIC driver!\r\n");
        while (1) { R_OS_TaskSleep(1000); }
    }

    /* 2. Configure I2C Master parameters */
    memset(&riic_cfg, 0, sizeof(st_riic_config_t));
    riic_cfg.riic_mode                 = RIIC_MODE_MASTER;
    riic_cfg.frequency                 = RIIC_FREQUENCY_100KHZ;
    riic_cfg.duty                      = RIIC_DUTY_50;
    riic_cfg.format                    = RIIC_FORMAT_I2C;
    riic_cfg.noise_filter_stage        = RIIC_FILTER_NOT_USED;
    riic_cfg.timeout                   = RIIC_TIMEOUT_NOT_USED;
    riic_cfg.slave_address_enable[0]   = false;

    if (DRV_SUCCESS != control(riic_handle, CTL_RIIC_SET_CONFIG, &riic_cfg))
    {
        printf("[ERROR] Failed to set RIIC configuration!\r\n");
        close(riic_handle);
        return;
    }

    /* 3. Initialize sensor */
    if (aht10_init(riic_handle) < 0)
    {
        printf("[WARN] AHT10 init failed, trying soft reset...\r\n");
        aht10_soft_reset(riic_handle);
        if (aht10_init(riic_handle) < 0)
        {
            printf("[ERROR] Failed to connect to AHT10, please check wiring and pull-up resistors!\r\n");
            close(riic_handle);
            return;
        }
    }

    printf("AHT10 sensor initialized successfully, starting sampling...\r\n");

    /* 4. Sampling loop (every 2 seconds) */
    while (1)
    {
        int ret = aht10_read_data(riic_handle, &sensor_data);
        if (ret == 0)
        {
            printf("[AHT10] Temp: %.2f °C | Hum: %.2f %%RH\r\n",
                   sensor_data.temperature,
                   sensor_data.humidity);
        }
        else
        {
            printf("[ERROR] Failed to read AHT10 data! Error code: %d\r\n", ret);
        }

        R_OS_TaskSleep(2000);
    }

    close(riic_handle);
}

/******************************************************************************
* Function Name: os_main_task_t
* Description  : FreeRTOS main task called by R_OS_KernelInit()
*              : FreeRTOS is now configured and R_OS_Abstraction calls
*              : can be used.
*              : From this point forward no longer use direct_xxx calls.
*              : For example
*              : in place of   direct_open("ostm2", O_RDWR);
*              : use           open(DEVICE_INDENTIFIER "ostm2", O_RDWR);
*              :
* Arguments    : none
* Return Value : 0
******************************************************************************/
int_t os_main_task_t(void)
{
    int_t err;
    st_r_drv_gpio_pin_list_t pin_led;
    char_t data;

    /* For information only
     * Use stdio calls to open drivers once  the kernel is initialised
     *
     * ie
     * int_t ostm3_handle;
     * ostm3_handle = open (DEVICE_INDENTIFIER "ostm2", O_RDWR);
     * close (ostm3_handle);
     */

    gs_my_gpio_handle = open (DEVICE_INDENTIFIER "gpio", O_RDWR);

    /* On error */
    if ( gs_my_gpio_handle < 0 )
    {
        /* stop execute */
        while(1)
        {
            /* Do Nothing */
        }
    }

    /**************************************************
     * Initialise P6_0 pin parameterised in GPIO_SC_TABLE_MANUAL
     **************************************************/
    pin_led.p_pin_list = gs_led_pin_list;
    pin_led.count = (sizeof(gs_led_pin_list)) / (sizeof(gs_led_pin_list[0]));
    err = direct_control(gs_my_gpio_handle,
                            CTL_GPIO_INIT_BY_PIN_LIST,
                            &pin_led);

    /* On error */
    if ( err < 0 )
    {
        /* stop execute */
        while(1)
        {
            /* Do Nothing */
        }
    }

    /* ==== Output banner message ==== */
    printf("%s%s", gsp_clear_screen, gsp_cursor_home);
    show_welcome_msg(stdout, true);

    err = init_eeprom();

    /* On error */
    if ( err < 0 )
    {
        /* stop execute */
        while(1)
        {
            /* Do Nothing */
        }
    }

    /* Create a task to run the console */
    R_OS_TaskCreate("Console", os_console_task_t, NULL,
                                R_OS_ABSTRACTION_DEFAULT_STACK_SIZE,
                                TASK_CONSOLE_TASK_PRI);

    /* Create AHT10 Demo Task */
    R_OS_TaskCreate("AHT10", aht10_demo_task, NULL,
                                R_OS_ABSTRACTION_DEFAULT_STACK_SIZE,
                                TASK_CONSOLE_TASK_PRI);

    while(1)
    {
        /* ==== LED blink ==== */
        gs_main_led_flg ^= 1;

        if (MAIN_PRV_LED_ON == gs_main_led_flg)
        {
            direct_control(gs_my_gpio_handle, CTL_GPIO_PIN_WRITE, &gs_p60_hi);
        }
        else
        {
            direct_control(gs_my_gpio_handle, CTL_GPIO_PIN_WRITE, &gs_p60_lo);
        }

        R_OS_TaskSleep(500);
    }

    return err;
}
/*****************************************************************************
 * End of function os_main_task_t
 *****************************************************************************/

/******************************************************************************
* Function Name: main
* Description  : C Entry point
*              : opens and configures cpg driver
*              : starts the freertos kernel
* Arguments    : none
* Return Value : 0
******************************************************************************/
int_t main(void)
{
    int_t cpg_handle;

    /* configure any drivers that are required before the Kernel initialises */

    /* Initialize the devlink layer */
    R_DEVLINK_Init();

    /* Initialize CPG */
    cpg_handle = direct_open("cpg", 0);
    if ( cpg_handle < 0 )
    {
        /* stop execute */
        while(1)
        {
            /* Do Nothing */
        }
    }

    /* Can close handle if no need to change clock after here */
    direct_close(cpg_handle);

    /* Start FreeRTOS */
    /* R_OS_InitKernel should never return */
    R_OS_KernelInit();
}
/******************************************************************************
 * End of function main
 *****************************************************************************/

/******************************************************************************
* Function Name: init_eeprom
* Description  : Initialize EEPROM
*              : opens and configures RIIC driver
* Arguments    : none
* Return Value : NO_ERROR - EEPROM initialization is succeeded.
*              : ERROR_FAILURE - EEPROM initialization is failed.
******************************************************************************/
static int_t init_eeprom(void)
{
    int_t ret = NO_ERROR;

    ret = sample_riic_eeprom_init();

    if (ERROR_FAILURE == ret)
    {
        printf("EEPROM initialize is failed\r\n");
    }

    return ret;
}

/******************************************************************************
 * End of function init_eeprom
 *****************************************************************************/

/* End of File */
