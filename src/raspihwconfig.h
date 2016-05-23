#ifndef _RASPI_HW_CONFIG_H_
#define _RASPI_HW_CONFIG_H_

typedef enum {
    RHW_SUCCESS                = 0x0,
    RHW_ERROR                  = 0x1,
    RHW_POSTMAN_FAIL           = 0x2,
} RHW_RETURN_TYPE;

typedef enum {
    RHW_SD_CARD = 0x00000000,
    RHW_UART0   = 0x00000001,
    RHW_UART1   = 0x00000002,
    RHW_USB_HCD = 0x00000003,
    RHW_I2C0    = 0x00000004,
    RHW_I2C1    = 0x00000005,
    RHW_I2C2    = 0x00000006,
    RHW_SPI     = 0x00000007,
    RHW_CCP2TX  = 0x00000008,
} RHW_DEVICE;

typedef enum {
    RHW_POWER_ON,
    RHW_POWER_OFF 
} RHW_POWER_STATE;


RHW_RETURN_TYPE rhw_get_mac_address( unsigned char* pOutAddr );
RHW_RETURN_TYPE rhw_set_device_power( RHW_DEVICE dev, RHW_POWER_STATE state );

#endif
