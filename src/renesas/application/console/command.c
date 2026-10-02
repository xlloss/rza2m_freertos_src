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
 * File Name    : command.c
 * Version      : 1.10
 * Description  : The entry point of the main command handler and associated
 *                commands
 *****************************************************************************/
/******************************************************************************
 * History      : DD.MM.YYYY Version Description
 *              : 04.02.2010 1.00    First Release
 *              : 10.06.2010 1.01    Updated type definitions.
 *              : 09.06.2015 1.10    Updated high-ASCII password for GNU TC
 *                                   Added FAT Stub check and warning
 *****************************************************************************/

/******************************************************************************
 Includes   <System Includes> , "Project Includes"
 *****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "compiler_settings.h"
#include "driver.h"
#include "version.h"
#include "command.h"

#include "r_os_abstraction_api.h"
#include "application_cfg.h"

#include "r_eeprom_sample.h"

/******************************************************************************
Typedef definitions
******************************************************************************/

/******************************************************************************
Macro definitions
******************************************************************************/

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
 Exported global variables and functions (to be accessed by other files)
 *****************************************************************************/

/******************************************************************************
 Private global variables and functions
 *****************************************************************************/

static int16_t cmd_write_eeprom(int iArgCount, char **ppszArgument, pst_comset_t pCom);
static int16_t cmd_read_eeprom(int iArgCount, char **ppszArgument, pst_comset_t pCom);
static int16_t cmd_help(int iArgCount, char **ppszArgument, pst_comset_t pCom);
static int16_t cmd_version(int iArgCount, char **ppszArgument, pst_comset_t pCom);
static int16_t cmd_exit(int iArgCount, char **ppszArgument, pst_comset_t pCom);

/*****************************************************************************
Global Variables
******************************************************************************/

/******************************************************************************
Public Functions
******************************************************************************/

/* Table that associates command letters, function pointer and a little
   description of what the command does */
static st_cmdfnass_t gs_cmd_command[] =
{
    {
        "write",
        cmd_write_eeprom,
        "address size fist_data_value data_count_type<CR> - Write data to EEPROM",
    },
    {
        "read",
        cmd_read_eeprom,
        "address size<CR> - Read data from EEPROM",
    },
    {
        "help",
        cmd_help,
        "<CR> - Show the commamd description",
    },
    {
        "ver",
        cmd_version,
        "<CR> - Show the application version",
    },
    {
        "exit",
        cmd_exit,
        "<CR> - Exit sample program",
    }
};

/* Table that points to the above table and contains the number of entries */
static const st_command_table_t g_cmd_tbl_command =
{
    "General Commands",
    gs_cmd_command,
    ((sizeof(gs_cmd_command)) / sizeof(st_cmdfnass_t))
};

/* Define the command tables required */
static pst_command_table_t g_ppcommands[32] =
{
    /* cast to pst_command_table_t */
    (pst_command_table_t) &g_cmd_tbl_command,

    /* Added space for additional 'sample code' */
};

static int32_t g_num_commands = (sizeof(g_ppcommands)) / sizeof(pst_command_table_t);

/******************************************************************************
 Function Name: cmd_console
 Description:   Function to process the console commands
 Arguments:     IN  pIn - Pointer to the input file stream
                IN  pOut - Pointer to the file stream for echo of command input
                IN  pszPrompt - Pointer to a command prompt
 Return value:  none
 *****************************************************************************/
void cmd_console(FILE* pIn, FILE *pOut, char *pszPrompt)
{
    pst_comset_t com = R_OS_Malloc(sizeof(st_comset_t), R_MEMORY_REGION_DEFAULT);
    int32_t pos;

    g_num_commands = (sizeof(g_ppcommands)) / sizeof(pst_command_table_t);

    /* remove unused console command lists */
    for (pos = (g_num_commands - 1); pos > 0; pos--)
    {
        /* check for null entry */
        if (NULL == g_ppcommands[pos])
        {
            g_num_commands--;
        }
    }

    if (com)
    {
        while (1)
        {
            e_cmderr_t cmd_err = CMD_OK;

            /* Initialise the console for login */
            memset(com, 0, sizeof(st_comset_t));
            com->p_in = pIn;
            com->p_out = pOut;

            /* Process commands until the link is lost or the command returns 
               a code greater than CMD_ERROR_IN_IO */
            while (cmd_err < CMD_USER_EXIT)
            {
                /* cast g_ppcommands to cpst_command_table_t * */
                cmd_err = console(com, (cpst_command_table_t *) g_ppcommands, g_num_commands, pIn, pOut, pszPrompt);
            }

            /* CMD_ERROR_IN_IO is returned when the client disconnects check 
                for an IP stream */
            if ((CMD_ERROR_IN_IO == cmd_err) && 
                /* Cast to an appropriate type */
                (control(R_DEVLINK_FilePtrDescriptor(com->p_in), CTL_STREAM_TCP, NULL) == 0))
            {
                /* Log out */
                cmd_err = CMD_LOG_OUT;
            }

            /* Check for the "exit<CR>" command return code */
            if (CMD_LOG_OUT == cmd_err)
            {
                fprintf(com->p_out, "\r\nBye!\r\n");
                fflush(pOut);

                /* Do "exit" code */
                R_OS_Free((void*) &com);
                return;
            }
        }
    }
}
/******************************************************************************
 End of function cmd_console
 *****************************************************************************/

/******************************************************************************
Function Name: cmd_version
Description:   Command to show the version
Arguments:     IN  iArgCount - The number of arguments in the argument list
               IN  ppszArgument - The argument list
               IN  pCom - Pointer to the command object
Return value:  CMD_OK for success
******************************************************************************/
static int16_t cmd_version(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    show_welcome_msg(pCom->p_out, false);
    return CMD_OK;
}
/******************************************************************************
End of function cmd_version
******************************************************************************/

/******************************************************************************
 Function Name: cmd_write_eeprom
 Description:   Command to write EEPROM
 Arguments:     IN  iArgCount - The number of arguments in the argument list
                IN  ppszArgument - The argument list
                IN  pCom - Pointer to the command object
 Return value:  CMD_OK for success
 *****************************************************************************/
static int16_t cmd_write_eeprom(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    uint32_t address;
    uint32_t len;
    uint32_t first_data;
    uint32_t count_type;
    e_cmderr_t internal_error = CMD_OK;
    uint8_t write_data[MAX_BUF_SIZE_EEPROM_SAMPLE];
    uint8_t write_value;
    uint32_t buf_len;

    address = EEPROM_DEFAULT_ADDRESS;
    len = EEPROM_DEFAULT_SIZE;
    first_data = EEPROM_DEFAULT_FIRST_DATA;
    count_type = EEPROM_DEFAULT_COUNT_TYPE;
    switch (iArgCount)
    {
        case 0 :
        {
            internal_error = CMD_ERROR_IN_IO;
            break;
        }
        case 1:
        {
            /* Intentionally empty braces. */
            break;
        }
        case 2:
        {
            /* get address parameter */
            if (sscanf(ppszArgument[1], "%x", &address) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            break;
        }
        case 3:
        {
            /* get address parameter */
            if (sscanf(ppszArgument[1], "%x", &address) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            
            /* get size parameter */
            if (sscanf(ppszArgument[2], "%x", &len) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            break;
        }
        case 4:
        {
            /* get address parameter */
            if (sscanf(ppszArgument[1], "%x", &address) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            
            /* get size parameter */
            if (sscanf(ppszArgument[2], "%x", &len) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            
            /* get first data value */
            if (sscanf(ppszArgument[3], "%x", &first_data) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            break;
        }
        case 5 :
        {
            /* get address parameter */
            if (sscanf(ppszArgument[1], "%x", &address) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            
            /* get size parameter */
            if (sscanf(ppszArgument[2], "%x", &len) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            
            /* get first data value */
            if (sscanf(ppszArgument[3], "%x", &first_data) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            
            /* get count type */
            if (sscanf(ppszArgument[4], "%d", &count_type) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            break;
        }
        default :
        {
            /* too many arguments */
            internal_error = CMD_ERROR_IN_IO;
            break;
        }
    }
        
    if (CMD_OK == internal_error)
    {
        /* check each parameters */
        /* check EEPROM access size */
        if ((MAX_BUF_SIZE_EEPROM_SAMPLE < len) || (0 == len))
        {
            internal_error = CMD_ERROR_IN_IO;
        }
        
        /* check access range within the range of EEPROM */
        if (MAX_SIZE_EEPROM < (address + len))
        {
            internal_error = CMD_ERROR_IN_IO;
        }
        
        /* check the first data is 1byte data */
        if (MAX_FIRST_DATA_EEPROM_SAMPLE < first_data)
        {
            internal_error = CMD_ERROR_IN_IO;
        }
        
        /* check the count type */
        if (DATA_COUNT_TYPE_MAX <= count_type)
        {
            internal_error = CMD_ERROR_IN_IO;
        }
    }
    
    if (CMD_OK == internal_error)
    {
        /* create write data */
        for (buf_len = 0; buf_len < MAX_BUF_SIZE_EEPROM_SAMPLE; buf_len++)
        {
            write_data[buf_len] = 0;
        }
        
        write_value = first_data;
        for (buf_len = 0; buf_len < len; buf_len++)
        {
            write_data[buf_len] = write_value;
            
            if (DATA_COUNT_TYPE_INCREMENT == count_type)
            {
                write_value++;
            }
            else if (DATA_COUNT_TYPE_DECREMENT == count_type)
            {
                write_value--;
            }
            else
            {
                /* nothing done, keep write data value */
            }
        }
        
        internal_error = sample_riic_eeprom_write(address, write_data, len);
        if (ERROR_FAILURE == internal_error)
        {   
            fprintf(pCom->p_out, "EEPROM write error\r\n");
        }
    }
    else
    {
        fprintf(pCom->p_out, "Command format error\r\n");
    }

    return CMD_OK;
}
/******************************************************************************
 End of function cmd_write_eeprom
 *****************************************************************************/

/******************************************************************************
 Function Name: cmd_read_eeprom
 Description:   Command to read EEPROM
 Arguments:     IN  iArgCount - The number of arguments in the argument list
                IN  ppszArgument - The argument list
                IN  pCom - Pointer to the command object
 Return value:  CMD_OK for success
 *****************************************************************************/
static int16_t cmd_read_eeprom(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    uint32_t address;
    uint32_t len;
    e_cmderr_t internal_error = CMD_OK;
    uint8_t read_data[MAX_BUF_SIZE_EEPROM_SAMPLE];
    uint32_t buf_len;

    address = EEPROM_DEFAULT_ADDRESS;
    len = EEPROM_DEFAULT_SIZE;
    switch (iArgCount)
    {
        case 0 :
        {
            internal_error = CMD_ERROR_IN_IO;
            break;
        }
        case 1:
        {
            /* Intentionally empty braces. */
            break;
        }
        case 2:
        {
            /* get address parameter */
            if (sscanf(ppszArgument[1], "%x", &address) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            break;
        }
        case 3 :
        {
            /* get address parameter */
            if (sscanf(ppszArgument[1], "%x", &address) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            
            /* get size parameter */
            if (sscanf(ppszArgument[2], "%x", &len) == (-1))
            {
                internal_error = CMD_ERROR_IN_IO;
            }
            break;
        }
        default :
        {
            /* too many arguments */
            internal_error = CMD_ERROR_IN_IO;
            break;
        }
    }
        
    if (CMD_OK == internal_error)
    {
        /* check each parameters */
        /* check EEPROM access size */
        if ((MAX_BUF_SIZE_EEPROM_SAMPLE < len) || (0 == len))
        {
            internal_error = CMD_ERROR_IN_IO;
        }
        
        /* check access range within the range of EEPROM */
        if (MAX_SIZE_EEPROM < (address + len))
        {
            internal_error = CMD_ERROR_IN_IO;
        }
    }
    
    if (CMD_OK == internal_error)
    {
        /* flush read data buffer */
        for (buf_len = 0; buf_len < MAX_BUF_SIZE_EEPROM_SAMPLE; buf_len++)
        {
            read_data[buf_len] = 0;
        }
        
        internal_error = sample_riic_eeprom_read(address, read_data, len);
        if (ERROR_FAILURE == internal_error)
        {   
            fprintf(pCom->p_out, "EEPROM read error\r\n");
        }
        else
        {
            /* output read data */
            for (buf_len = 0; buf_len < len; buf_len++)
            {
                if (0 == (buf_len % 16))
                {
                    fprintf(pCom->p_out, "\r\n");
                }
                fprintf(pCom->p_out, " 0x%02x", read_data[buf_len]);
            }
        }
    }
    else
    {
        fprintf(pCom->p_out, "Command format error\r\n");
    }

    return CMD_OK;
}
/******************************************************************************
 End of function cmd_read_eeprom
 *****************************************************************************/

/******************************************************************************
Function Name: cmd_help
Description:   Command to show the help
Arguments:     IN  iArgCount - The number of arguments in the argument list
               IN  ppszArgument - The argument list
               IN  pCom - Pointer to the command object
Return value:  CMD_OK for success
******************************************************************************/
static int16_t cmd_help(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    fprintf(pCom->p_out, "\r\n");
    fprintf(pCom->p_out, "[write command]\r\n");
    fprintf(pCom->p_out, "command format:\r\n");
    fprintf(pCom->p_out, "  write 'address' 'size' 'data' 'type'\r\n");
    fprintf(pCom->p_out, "argument:\r\n");
    fprintf(pCom->p_out, 
        "  'address' - write destination address on EEPROM (range of (address + size):0x0000 - 0x3FFF)\r\n");
    fprintf(pCom->p_out, "  'size' - write data size (range:0x01 - 0x100)\r\n");
    fprintf(pCom->p_out, 
        "  'data' - first data value of increment data or decrement data (range:byte data (0x00 - 0xFF)\r\n");
    fprintf(pCom->p_out, "  'type' - type of data to write (0:increment, 1:decrement 2:fill same value)\r\n");
    fprintf(pCom->p_out, "  note1:Each argument is handled as a hex value with or without '0x'\r\n");
    fprintf(pCom->p_out, "  note2:If each argument is not set, the following values are automatically set\r\n");
    fprintf(pCom->p_out, "        However, if there are too many arguments, it will return a format error\r\n");
    fprintf(pCom->p_out, "  address:0x0000, size:0x40, first_data_value:0x00, data_type:increment\r\n");
    fprintf(pCom->p_out, "command example:\r\n");
    fprintf(pCom->p_out, "  write 0x1000 0x100 0xFF 1:\r\n");
    
    fprintf(pCom->p_out, "\r\n");
    fprintf(pCom->p_out, "[read command]\r\n");
    fprintf(pCom->p_out, "command format:\r\n");
    fprintf(pCom->p_out, "  read 'address' 'size' \r\n");
    fprintf(pCom->p_out, "argument:\r\n");
    fprintf(pCom->p_out, 
            "  'address' - read source address on EEPROM (range of (address + size):0x0000 - 0x3FFF)\r\n");
    fprintf(pCom->p_out, "  'size' - read data size (range:0x01 - 0x100)\r\n");
    fprintf(pCom->p_out, "  note1:Each argument is handled as a hex value with or without '0x'\r\n");
    fprintf(pCom->p_out, "  note2:If each argument is not set, the following values are automatically set\r\n");
    fprintf(pCom->p_out, "        However, if there are too many arguments, it will return a format error\r\n");
    fprintf(pCom->p_out, "  address:0x0000, size:0x40\r\n");
    fprintf(pCom->p_out, "command example:\r\n");
    fprintf(pCom->p_out, "  write 0x1000 0x100:\r\n");
    
    fprintf(pCom->p_out, "\r\n");
    fprintf(pCom->p_out, "[ver command]\r\n");
    fprintf(pCom->p_out, "Show the version information\r\n");
    
    fprintf(pCom->p_out, "\r\n");
    fprintf(pCom->p_out, "[help command]\r\n");
    fprintf(pCom->p_out, "This command is currently executing\r\n");
    
    fprintf(pCom->p_out, "\r\n");
    fprintf(pCom->p_out, "[exit command]\r\n");
    fprintf(pCom->p_out, "Shut down the sample program\r\n");
    
    return CMD_OK;
}
/******************************************************************************
End of function cmd_help
******************************************************************************/

/******************************************************************************
Function Name: cmd_exit
Description:   Command to exit cmd_console loop
Arguments:     IN  iArgCount - The number of arguments in the argument list
               IN  ppszArgument - The argument list
               IN  pCom - Pointer to the command object
Return value:  CMD_LOG_OUT always return
******************************************************************************/
static int16_t cmd_exit(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int_t ret = NO_ERROR;
    
    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;
    
    ret = sample_riic_eeprom_uninit();

    if (ERROR_FAILURE == ret)
    {
        fprintf(pCom->p_out, "EEPROM uninitialize is failed\r\n");
    }

    return CMD_LOG_OUT;
}
/******************************************************************************
End of function cmd_exit
******************************************************************************/

/******************************************************************************
 End  Of File
 *****************************************************************************/
