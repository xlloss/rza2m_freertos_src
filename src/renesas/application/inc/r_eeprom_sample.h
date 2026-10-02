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
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
* Copyright (C) 2018 Renesas Electronics Corporation. All rights reserved.
******************************************************************************/
/******************************************************************************
* File Name    : r_eeprom_sample.h
* Description  : EEPROM sample header
*                The target EEPROM model is R1EX24128ASAS0A
******************************************************************************/

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include "r_typedefs.h"

#ifndef R_EEPROM_SAMPLE_H
#define R_EEPROM_SAMPLE_H

/******************************************************************************
Macro definitions
******************************************************************************/
#define MAX_SIZE_EEPROM      (16384)     /* 16Kbyte */
#define EEPROM_DEV_ADDRESS   (0xA0)
#define EEPROM_PAGE_SIZE     (64)
#define RIIC_CH_EEPROM_RZA2M (DEVICE_INDENTIFIER "riic3")

#define EEPROM_DUMMY_READ_BUF_SIZE (1)
/* max time to wait for ACK polling */
/* EEPROM write time is 5msec, so wait about 10msec*/
/* RIIC frequency 400KHz = 50000byte / sec,
 * so do dummy reads of 100bytes to wait 10 msec */
#define EEPROM_DUMMY_READ_MAX_TIME (100)

/******************************************************************************
Typedef definitions
******************************************************************************/
typedef enum
{
    ERROR_FAILURE = -1,              // <Generic error code
    NO_ERROR = 0                     // <Success
} e_error_t;

/******************************************************************************
Variable Externs
******************************************************************************/

/******************************************************************************
Functions Prototypes
******************************************************************************/

/* EEPROM control functions */
/******************************************************************************
 * Function Name: sample_riic_eeprom_init
 * Description  : Open RIIC driver for EEPROM control start
 * Arguments    : None
 * Return Value : int_t - error code
 *****************************************************************************/
int_t sample_riic_eeprom_init(void);

/*****************************************************************************
 * Function Name: sample_riic_eeprom_uninit
 * Description  : Close RIIC driver for EEPROM control shutdown
 * Arguments    : None
 * Return Value : int_t - error code
 *****************************************************************************/
int_t sample_riic_eeprom_uninit(void);

/*****************************************************************************
 * Function Name: sample_riic_eeprom_write
 * Description  : Write data to EEPROM memory
 * Arguments    : uint8_t addr - start address to write to EEPROM
 *              : const void *data - pointer to the buffer storing data to be 
 *              : written to EEPROM
 *              : size_t size - write data size
 * Return Value : int_t - error code
 *****************************************************************************/
int_t sample_riic_eeprom_write(uint32_t addr, const void *data, uint32_t size);

/*****************************************************************************
 * Function Name: sample_riic_eeprom_read
 * Description  : Read data from EEPROM memory
 * Arguments    : uint8_t addr - start address to read from EEPROM
 *              : const void *data - pointer of the buffer to store the data 
 *              : read from EEPROM
 *              : size_t size - read data size
 * Return Value : int_t - error code
 *****************************************************************************/
int_t sample_riic_eeprom_read(uint32_t addr, void *data, uint32_t size);

#endif /* R_EEPROM_SAMPLE_H */
