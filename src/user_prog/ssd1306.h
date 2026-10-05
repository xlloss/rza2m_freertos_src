#ifndef _SSD1306_H_
#define _SSD1306_H_

#include "r_typedefs.h"

/* 
 * 8-bit I2C Slave Addresses for Renesas RIIC Driver:
 *   - SA0=0 (Default on almost all 0.96"/0.91" modules): 7-bit 0x3C << 1 = 0x78
 *   - SA0=1 (Alternate when SA0/D/C is pulled high):      7-bit 0x3D << 1 = 0x7A
 */
#define SSD1306_I2C_ADDR_DEFAULT  (0x78u)
#define SSD1306_I2C_ADDR_ALT      (0x7Au)
#define SSD1306_I2C_ADDR          SSD1306_I2C_ADDR_DEFAULT

/* Panel geometry selection */
typedef enum {
    SSD1306_PANEL_128X32 = 0,  /* 128x32 (0.91 inch, 32 MUX, sequential COM pins) */
    SSD1306_PANEL_128X64 = 1   /* 128x64 (0.96 inch, 64 MUX, alternative COM pins) */
} ssd1306_panel_t;

/* SSD1306 Commands */
#define SSD1306_CMD_SET_CONTRAST           (0x81)
#define SSD1306_CMD_DISPLAY_ALL_ON_RESUME  (0xA4)
#define SSD1306_CMD_DISPLAY_ALL_ON         (0xA5)
#define SSD1306_CMD_NORMAL_DISPLAY         (0xA6)
#define SSD1306_CMD_INVERT_DISPLAY         (0xA7)
#define SSD1306_CMD_DISPLAY_OFF            (0xAE)
#define SSD1306_CMD_DISPLAY_ON             (0xAF)

#define SSD1306_CMD_SET_DISPLAY_OFFSET     (0xD3)
#define SSD1306_CMD_SET_COMPINS            (0xDA)
#define SSD1306_COMPINS_SEQ_DISABLE        (0x02) /* 128x32 sequential COM pins */
#define SSD1306_COMPINS_ALT_DISABLE        (0x12) /* 128x64 alternative COM pins */

#define SSD1306_CMD_SET_VCOM_DETECT        (0xDB)
#define SSD1306_CMD_SET_DISPLAY_CLOCK_DIV  (0xD5)
#define SSD1306_CMD_SET_PRECHARGE          (0xD9)

#define SSD1306_CMD_SET_MULTIPLEX          (0xA8)
#define SSD1306_MULTIPLEX_128X32           (0x1F) /* 32 MUX */
#define SSD1306_MULTIPLEX_128X64           (0x3F) /* 64 MUX */

#define SSD1306_CMD_SET_LOW_COLUMN         (0x00)
#define SSD1306_CMD_SET_HIGH_COLUMN        (0x10)
#define SSD1306_CMD_SET_START_LINE         (0x40)

#define SSD1306_CMD_MEMORY_MODE            (0x20)
#define SSD1306_CMD_COLUMN_ADDR            (0x21)
#define SSD1306_CMD_PAGE_ADDR              (0x22)

#define SSD1306_CMD_COM_SCAN_INC           (0xC0)
#define SSD1306_CMD_COM_SCAN_DEC           (0xC8)

#define SSD1306_CMD_SEG_REMAP              (0xA0)
#define SSD1306_CMD_CHARGE_PUMP            (0x8D)

/* Init and API */
int ssd1306_init(int_t riic_handle);
int ssd1306_init_panel(int_t riic_handle, ssd1306_panel_t panel);
int ssd1306_fill(int_t riic_handle, uint8_t fill_data);
int ssd1306_clear(int_t riic_handle);

/* 16x16 Proportional Font Display Functions (Line 0: Page 0+1, Line 1: Page 2+3) */
int ssd1306_clear_line16(int_t riic_handle, uint8_t line);
int ssd1306_draw_string16(int_t riic_handle, uint8_t start_x, uint8_t line, const char *str);

/* Classic 5x7 Font Display Function */
int ssd1306_draw_string(int_t riic_handle, uint8_t col, uint8_t page, const char *str);

#endif /* _SSD1306_H_ */
