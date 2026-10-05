#include <stdio.h>
#include <string.h>
#include "r_typedefs.h"
#include "r_riic_drv_api.h"
#include "r_os_abstraction_api.h"
#include "ssd1306.h"
#include "font16x16.h"

/* Active 8-bit I2C address (auto-probed between 0x78 and 0x7A) */
static uint8_t gs_ssd1306_addr = SSD1306_I2C_ADDR_DEFAULT;

/* Active panel configuration (defaults to 128x32) */
static ssd1306_panel_t gs_ssd1306_panel = SSD1306_PANEL_128X32;

/* Line buffer for 16x16 font rendering (2 pages x 128 columns) */
static uint8_t s_line_buf[2][128];

/* Standard 5x7 ASCII Font Table (characters 0x20 ' ' to 0x7E '~') */
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, /* (space) */
    {0x00, 0x00, 0x5F, 0x00, 0x00}, /* ! */
    {0x00, 0x07, 0x00, 0x07, 0x00}, /* " */
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, /* # */
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, /* $ */
    {0x23, 0x13, 0x08, 0x64, 0x62}, /* % */
    {0x36, 0x49, 0x55, 0x22, 0x50}, /* & */
    {0x00, 0x05, 0x03, 0x00, 0x00}, /* ' */
    {0x00, 0x1C, 0x22, 0x41, 0x00}, /* ( */
    {0x00, 0x41, 0x22, 0x1C, 0x00}, /* ) */
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, /* * */
    {0x08, 0x08, 0x3E, 0x08, 0x08}, /* + */
    {0x00, 0x50, 0x30, 0x00, 0x00}, /* , */
    {0x08, 0x08, 0x08, 0x08, 0x08}, /* - */
    {0x00, 0x60, 0x60, 0x00, 0x00}, /* . */
    {0x20, 0x10, 0x08, 0x04, 0x02}, /* / */
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, /* 0 */
    {0x00, 0x42, 0x7F, 0x40, 0x00}, /* 1 */
    {0x42, 0x61, 0x51, 0x49, 0x46}, /* 2 */
    {0x21, 0x41, 0x45, 0x4B, 0x31}, /* 3 */
    {0x18, 0x14, 0x12, 0x7F, 0x10}, /* 4 */
    {0x27, 0x45, 0x45, 0x45, 0x39}, /* 5 */
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, /* 6 */
    {0x01, 0x71, 0x09, 0x05, 0x03}, /* 7 */
    {0x36, 0x49, 0x49, 0x49, 0x36}, /* 8 */
    {0x06, 0x49, 0x49, 0x29, 0x1E}, /* 9 */
    {0x00, 0x36, 0x36, 0x00, 0x00}, /* : */
    {0x00, 0x56, 0x36, 0x00, 0x00}, /* ; */
    {0x00, 0x08, 0x14, 0x22, 0x41}, /* < */
    {0x14, 0x14, 0x14, 0x14, 0x14}, /* = */
    {0x41, 0x22, 0x14, 0x08, 0x00}, /* > */
    {0x02, 0x01, 0x51, 0x09, 0x06}, /* ? */
    {0x32, 0x49, 0x79, 0x41, 0x3E}, /* @ */
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, /* A */
    {0x7F, 0x49, 0x49, 0x49, 0x36}, /* B */
    {0x3E, 0x41, 0x41, 0x41, 0x22}, /* C */
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, /* D */
    {0x7F, 0x49, 0x49, 0x49, 0x41}, /* E */
    {0x7F, 0x09, 0x09, 0x01, 0x01}, /* F */
    {0x3E, 0x41, 0x41, 0x51, 0x32}, /* G */
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, /* H */
    {0x00, 0x41, 0x7F, 0x41, 0x00}, /* I */
    {0x20, 0x40, 0x41, 0x3F, 0x01}, /* J */
    {0x7F, 0x08, 0x14, 0x22, 0x41}, /* K */
    {0x7F, 0x40, 0x40, 0x40, 0x40}, /* L */
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, /* M */
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, /* N */
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, /* O */
    {0x7F, 0x09, 0x09, 0x09, 0x06}, /* P */
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, /* Q */
    {0x7F, 0x09, 0x19, 0x29, 0x46}, /* R */
    {0x46, 0x49, 0x49, 0x49, 0x31}, /* S */
    {0x01, 0x01, 0x7F, 0x01, 0x01}, /* T */
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, /* U */
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, /* V */
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, /* W */
    {0x63, 0x14, 0x08, 0x14, 0x63}, /* X */
    {0x03, 0x04, 0x78, 0x04, 0x03}, /* Y */
    {0x61, 0x51, 0x49, 0x45, 0x43}, /* Z */
    {0x00, 0x7F, 0x41, 0x41, 0x00}, /* [ */
    {0x02, 0x04, 0x08, 0x10, 0x20}, /* \ */
    {0x00, 0x41, 0x41, 0x7F, 0x00}, /* ] */
    {0x04, 0x02, 0x01, 0x02, 0x04}, /* ^ */
    {0x40, 0x40, 0x40, 0x40, 0x40}, /* _ */
    {0x00, 0x01, 0x02, 0x04, 0x00}, /* ` */
    {0x20, 0x54, 0x54, 0x54, 0x78}, /* a */
    {0x7F, 0x48, 0x44, 0x44, 0x38}, /* b */
    {0x38, 0x44, 0x44, 0x44, 0x20}, /* c */
    {0x38, 0x44, 0x44, 0x48, 0x7F}, /* d */
    {0x38, 0x54, 0x54, 0x54, 0x18}, /* e */
    {0x08, 0x7E, 0x09, 0x01, 0x02}, /* f */
    {0x08, 0x14, 0x54, 0x54, 0x3C}, /* g */
    {0x7F, 0x08, 0x04, 0x04, 0x78}, /* h */
    {0x00, 0x44, 0x7D, 0x40, 0x00}, /* i */
    {0x20, 0x40, 0x44, 0x3D, 0x00}, /* j */
    {0x7F, 0x10, 0x28, 0x44, 0x00}, /* k */
    {0x00, 0x41, 0x7F, 0x40, 0x00}, /* l */
    {0x7C, 0x04, 0x18, 0x04, 0x78}, /* m */
    {0x7C, 0x08, 0x04, 0x04, 0x78}, /* n */
    {0x38, 0x44, 0x44, 0x44, 0x38}, /* o */
    {0x7C, 0x14, 0x14, 0x14, 0x08}, /* p */
    {0x08, 0x14, 0x14, 0x18, 0x7C}, /* q */
    {0x7C, 0x08, 0x04, 0x04, 0x08}, /* r */
    {0x48, 0x54, 0x54, 0x54, 0x20}, /* s */
    {0x04, 0x3F, 0x44, 0x40, 0x20}, /* t */
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, /* u */
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, /* v */
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, /* w */
    {0x44, 0x28, 0x10, 0x28, 0x44}, /* x */
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, /* y */
    {0x44, 0x64, 0x54, 0x4C, 0x44}, /* z */
    {0x00, 0x08, 0x36, 0x41, 0x00}, /* { */
    {0x00, 0x00, 0x7F, 0x00, 0x00}, /* | */
    {0x00, 0x41, 0x36, 0x08, 0x00}, /* } */
    {0x08, 0x08, 0x2A, 0x1C, 0x08}  /* ~ */
};

static int ssd1306_write_cmd(int_t handle, uint8_t cmd)
{
    st_r_drv_riic_transfer_t xfer;
    uint8_t buf[2];

    buf[0] = 0x00; /* Co = 0, D/C# = 0 (Command) */
    buf[1] = cmd;

    xfer.device_address = gs_ssd1306_addr;
    xfer.sub_address_type = RIIC_SUB_ADDR_NONE;
    xfer.sub_address = 0;
    xfer.number_of_bytes = 2;
    xfer.p_data_buffer = buf;
    
    if (DRV_SUCCESS != control(handle, CTL_RIIC_WRITE, &xfer))
    {
        return -1;
    }
    return 0;
}

static int ssd1306_write_data_buffer(int_t handle, const uint8_t *data, uint32_t len)
{
    st_r_drv_riic_transfer_t xfer;
    static uint8_t buf[129]; /* static to prevent task stack overflow */
    
    if (len > 128)
    {
        len = 128;
    }
    
    buf[0] = 0x40; /* Co = 0, D/C# = 1 (Data) */
    memcpy(&buf[1], data, len);
    
    xfer.device_address = gs_ssd1306_addr;
    xfer.sub_address_type = RIIC_SUB_ADDR_NONE;
    xfer.sub_address = 0;
    xfer.number_of_bytes = len + 1;
    xfer.p_data_buffer = buf;
    
    if (DRV_SUCCESS != control(handle, CTL_RIIC_WRITE, &xfer))
    {
        return -1;
    }
    return 0;
}

int ssd1306_init_panel(int_t riic_handle, ssd1306_panel_t panel)
{
    gs_ssd1306_panel = panel;
    R_OS_TaskSleep(100); /* Wait for power rails to stabilize */

    /* 
     * Auto-probe I2C slave address:
     * Try 0x78 (SA0=0, default) first. If NACK, try 0x7A (SA0=1).
     */
    gs_ssd1306_addr = SSD1306_I2C_ADDR_DEFAULT; /* 0x78 */
    if (ssd1306_write_cmd(riic_handle, SSD1306_CMD_DISPLAY_OFF) < 0)
    {
        gs_ssd1306_addr = SSD1306_I2C_ADDR_ALT; /* 0x7A */
        if (ssd1306_write_cmd(riic_handle, SSD1306_CMD_DISPLAY_OFF) < 0)
        {
            printf("[ERROR] SSD1306 not responding at 0x78 or 0x7A! Please check VCC, GND, SCL, SDA.\r\n");
            return -1;
        }
    }

    printf("[SSD1306] OLED detected at 8-bit addr 0x%02X (7-bit 0x%02X), panel: %s\r\n", 
           gs_ssd1306_addr, gs_ssd1306_addr >> 1,
           (panel == SSD1306_PANEL_128X32) ? "128x32" : "128x64");

    /* Panel initialization sequence */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_DISPLAY_OFF);            /* 0xAE */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_SET_DISPLAY_CLOCK_DIV);  /* 0xD5 */
    ssd1306_write_cmd(riic_handle, 0x80);                                /* Ratio 0x80 */
    
    /* Multiplex ratio: 0x1F (32MUX) for 128x32, 0x3F (64MUX) for 128x64 */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_SET_MULTIPLEX);          /* 0xA8 */
    ssd1306_write_cmd(riic_handle, (panel == SSD1306_PANEL_128X32) ? SSD1306_MULTIPLEX_128X32 : SSD1306_MULTIPLEX_128X64);
    
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_SET_DISPLAY_OFFSET);     /* 0xD3 */
    ssd1306_write_cmd(riic_handle, 0x00);                                /* No offset */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_SET_START_LINE | 0x0);   /* Line #0 (0x40) */

    /* Charge pump configuration */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_CHARGE_PUMP);            /* 0x8D */
    ssd1306_write_cmd(riic_handle, 0x14);                                /* Enable Charge Pump */

    /* Horizontal memory addressing mode */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_MEMORY_MODE);            /* 0x20 */
    ssd1306_write_cmd(riic_handle, 0x00);                                /* 0x00 = Horizontal */

    /* Segment remap & COM scan direction */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_SEG_REMAP | 0x1);        /* 0xA1 = Column 127 mapped to SEG0 */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_COM_SCAN_DEC);           /* 0xC8 = Scan from COM[N-1] to COM0 */

    /* COM Pins hardware config: 0x02 (Sequential) for 128x32, 0x12 (Alternative) for 128x64 */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_SET_COMPINS);            /* 0xDA */
    ssd1306_write_cmd(riic_handle, (panel == SSD1306_PANEL_128X32) ? SSD1306_COMPINS_SEQ_DISABLE : SSD1306_COMPINS_ALT_DISABLE);

    ssd1306_write_cmd(riic_handle, SSD1306_CMD_SET_CONTRAST);           /* 0x81 */
    ssd1306_write_cmd(riic_handle, 0xCF);                                /* Contrast = 0xCF */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_SET_PRECHARGE);          /* 0xD9 */
    ssd1306_write_cmd(riic_handle, 0xF1);                                /* Precharge = 0xF1 */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_SET_VCOM_DETECT);        /* 0xDB */
    ssd1306_write_cmd(riic_handle, 0x40);                                /* VCOMH deselect level = 0x40 */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_DISPLAY_ALL_ON_RESUME);  /* 0xA4 = Output follows RAM */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_NORMAL_DISPLAY);         /* 0xA6 = Normal display */

    /* Clear display memory initially */
    ssd1306_clear(riic_handle);

    /* Turn display ON */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_DISPLAY_ON);             /* 0xAF */

    return 0;
}

int ssd1306_init(int_t riic_handle)
{
    /* Defaults to 128x32 panel as used in reference Linux driver */
    return ssd1306_init_panel(riic_handle, SSD1306_PANEL_128X32);
}

int ssd1306_fill(int_t riic_handle, uint8_t fill_data)
{
    static uint8_t buffer[128];
    uint8_t total_pages = (gs_ssd1306_panel == SSD1306_PANEL_128X32) ? 4 : 8;
    memset(buffer, fill_data, sizeof(buffer));
    
    /* Set page range */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_PAGE_ADDR);
    ssd1306_write_cmd(riic_handle, 0x00);
    ssd1306_write_cmd(riic_handle, total_pages - 1);
    
    /* Set column range 0-127 */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_COLUMN_ADDR);
    ssd1306_write_cmd(riic_handle, 0x00);
    ssd1306_write_cmd(riic_handle, 0x7F);
    
    for (int i = 0; i < total_pages; i++)
    {
        if (ssd1306_write_data_buffer(riic_handle, buffer, 128) < 0)
        {
            return -1;
        }
    }
    
    return 0;
}

int ssd1306_clear(int_t riic_handle)
{
    return ssd1306_fill(riic_handle, 0x00);
}

int ssd1306_clear_line16(int_t riic_handle, uint8_t line)
{
    uint8_t page_upper = line * 2;
    uint8_t page_lower = page_upper + 1;
    static uint8_t blank[128];

    memset(blank, 0x00, sizeof(blank));

    ssd1306_write_cmd(riic_handle, SSD1306_CMD_PAGE_ADDR);
    ssd1306_write_cmd(riic_handle, page_upper);
    ssd1306_write_cmd(riic_handle, page_lower);

    ssd1306_write_cmd(riic_handle, SSD1306_CMD_COLUMN_ADDR);
    ssd1306_write_cmd(riic_handle, 0x00);
    ssd1306_write_cmd(riic_handle, 0x7F);

    ssd1306_write_data_buffer(riic_handle, blank, 128);
    ssd1306_write_data_buffer(riic_handle, blank, 128);

    return 0;
}

int ssd1306_draw_string16(int_t riic_handle, uint8_t start_x, uint8_t line, const char *str)
{
    uint8_t max_lines = (gs_ssd1306_panel == SSD1306_PANEL_128X32) ? 2 : 4;
    if (line >= max_lines || str == NULL)
    {
        return -1;
    }

    uint8_t page_upper = line * 2;
    uint8_t page_lower = page_upper + 1;

    /* Clear local 2-page line buffer */
    memset(s_line_buf, 0, sizeof(s_line_buf));
    uint8_t cur_x = start_x;

    while (*str && cur_x < 128)
    {
        char c = *str++;
        uint8_t char_w = 0;
        const uint8_t *glyph = ssd1306_get_glyph16(c, &char_w);

        if (glyph == NULL)
        {
            cur_x += char_w;
            continue;
        }

        /* 
         * Left-edge auto trim (from linux ssd1306_char_drv_v2.0):
         * Find first column with non-zero pixel data.
         */
        int left_edge = 15;
        for (int i = 0; i < 16; i++)
        {
            if (glyph[i] != 0 || glyph[16 + i] != 0)
            {
                left_edge = i;
                break;
            }
        }

        /* Render glyph into 2-page buffer with proportional width */
        for (int i = 0; i < char_w; i++)
        {
            if (cur_x + i >= 128)
            {
                break;
            }
            if (left_edge + i < 16)
            {
                s_line_buf[0][cur_x + i] |= glyph[left_edge + i];      /* Upper 8 pixels */
                s_line_buf[1][cur_x + i] |= glyph[16 + left_edge + i]; /* Lower 8 pixels */
            }
        }

        cur_x += char_w;
    }

    /* Send upper page to OLED */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_PAGE_ADDR);
    ssd1306_write_cmd(riic_handle, page_upper);
    ssd1306_write_cmd(riic_handle, page_upper);

    ssd1306_write_cmd(riic_handle, SSD1306_CMD_COLUMN_ADDR);
    ssd1306_write_cmd(riic_handle, 0x00);
    ssd1306_write_cmd(riic_handle, 0x7F);

    if (ssd1306_write_data_buffer(riic_handle, s_line_buf[0], 128) < 0)
    {
        return -1;
    }

    /* Send lower page to OLED */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_PAGE_ADDR);
    ssd1306_write_cmd(riic_handle, page_lower);
    ssd1306_write_cmd(riic_handle, page_lower);

    ssd1306_write_cmd(riic_handle, SSD1306_CMD_COLUMN_ADDR);
    ssd1306_write_cmd(riic_handle, 0x00);
    ssd1306_write_cmd(riic_handle, 0x7F);

    if (ssd1306_write_data_buffer(riic_handle, s_line_buf[1], 128) < 0)
    {
        return -1;
    }

    return 0;
}

int ssd1306_draw_string(int_t riic_handle, uint8_t col, uint8_t page, const char *str)
{
    if (page > 7 || col > 127 || str == NULL)
    {
        return -1;
    }

    /* Set target page */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_PAGE_ADDR);
    ssd1306_write_cmd(riic_handle, page);
    ssd1306_write_cmd(riic_handle, 0x07);

    /* Set target column */
    ssd1306_write_cmd(riic_handle, SSD1306_CMD_COLUMN_ADDR);
    ssd1306_write_cmd(riic_handle, col);
    ssd1306_write_cmd(riic_handle, 0x7F);

    while (*str)
    {
        char c = *str++;
        uint8_t char_buf[6];

        if (c < 32 || c > 126)
        {
            c = ' ';
        }

        /* 5 bytes glyph + 1 byte spacing */
        memcpy(&char_buf[0], font5x7[c - 32], 5);
        char_buf[5] = 0x00;

        if (ssd1306_write_data_buffer(riic_handle, char_buf, 6) < 0)
        {
            return -1;
        }

        col += 6;
        if (col > 122)
        {
            break; /* prevent wrap around */
        }
    }

    return 0;
}
