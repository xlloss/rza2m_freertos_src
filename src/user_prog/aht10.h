#ifndef AHT10_H
#define AHT10_H

#include <stdint.h>
#include <stdbool.h>
#include "r_typedefs.h"

#define AHT10_I2C_ADDR (0x70u) /* 7-bit 0x38 << 1 */

#define AHT10_CMD_INIT (0xE1u)
#define AHT10_CMD_TRIGGER (0xACu)
#define AHT10_CMD_SOFT_RESET (0xBAu)

#define AHT10_STATUS_BUSY_MASK (0x80u)
#define AHT10_STATUS_CAL_MASK (0x08u)

typedef struct {
    float humidity;       /* Relative humidity (%RH) */
    float temperature;    /* Temperature (°C) */
} aht10_data_t;

int aht10_init(int_t riic_handle);
int aht10_soft_reset(int_t riic_handle);
int aht10_read_data(int_t riic_handle, aht10_data_t *p_data);

#endif /* AHT10_H */
