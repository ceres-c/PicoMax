#include "mcc_generated_files/mcc.h"
#define FLASH_READ_ADDR 0xAAA

volatile uint8_t start_barrier = 0;
volatile uint16_t prog_data = 0;
volatile uint8_t read_state = 0;

volatile uint8_t datl = 0;
volatile uint8_t dath = 0;

static void ReadIntHandler() {
    if (read_state == 0) {
        I2C_Write(datl);
        read_state = 1;
    } else {
        I2C_Write(dath);
        read_state = 0;
    }
}

static void AddressIntHandler() {
    uint8_t buf = SSPBUF; // Cleanup buffer
}

void main(void)
{
    // initialize the device
    SYSTEM_Initialize();
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    I2C_Initialize();

    EEDATH = 0;
    EEDATL = 0;

    while(!IO_RA1_GetValue()) asm("nop");   // Wait to toggle high...
    while(IO_RA1_GetValue()) asm("nop");    // ...and low

    I2C_Open(); // From now on we can return data over i2c
    I2C_SlaveSetWriteIntHandler(ReadIntHandler);
    I2C_SlaveSetAddrIntHandler(AddressIntHandler);

    // Read program memory
    EEADRL = (FLASH_READ_ADDR & 0x00FF);
    EEADRH = ((FLASH_READ_ADDR & 0xFF00) >> 8);

    EECON1bits.CFGS = 0;    // Deselect Configuration space
    EECON1bits.EEPGD = 1;   // Select Program Memory

    IO_RA0_SetHigh();   // Keep output pin high during memory read operation
    EECON1bits.RD = 1;      // Initiate Read
    NOP();
    NOP();
    IO_RA0_SetLow();   // Then pull the loop pin down
    datl = EEDATL;
    dath = EEDATH;

    // prog_data = ((uint16_t)((EEDATH << 8) | EEDATL));

    while(1) asm("nop");

    // TODO i2c send

}
/**
 End of File
*/