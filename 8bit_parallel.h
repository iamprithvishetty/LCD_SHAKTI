#include "platform.h"
#include "gpio.h"

#define RD_MASK 0x00001000 //define RD_MASK HEX VALUE here  READ : GPIO12
#define WR_MASK 0x00000800  //define WR_MASK HEX VALUE here   WRITE : GPIO11
#define CD_MASK 0x00000400  //define CD_MASK HEX VALUE here   COMMAND : GPIO10
#define CS_MASK 0x00000200  //define CS_MASK HEX VALUE here   CHIP SELECT : GPIO9
#define RES_MASK 0x00000100  //define RES_MASK HEX VALUE here   RESET : GPIO8

#define CONTROL_MASK CS_MASK|CD_MASK|WR_MASK|RD_MASK|RES_MASK // CONTROL MASK to only make changes in the control pins
#define SET_CNTL_BITS *GPIO_DIRECTION_CNTRL_REG = *GPIO_DIRECTION_CNTRL_REG | CONTROL_MASK // Setting CONTROL pins to OUTPUT

#define CS_ACTIVE *GPIO_DATA_REG = *GPIO_DATA_REG & ~CS_MASK // CS pin LOW
#define CS_IDLE *GPIO_DATA_REG = *GPIO_DATA_REG | CS_MASK // CS pin HIGH
#define CD_COMMAND *GPIO_DATA_REG = *GPIO_DATA_REG & ~CD_MASK // CD pin LOW
#define CD_DATA *GPIO_DATA_REG = *GPIO_DATA_REG | CD_MASK // CD pin HIGH
#define WR_ACTIVE *GPIO_DATA_REG = *GPIO_DATA_REG & ~WR_MASK // WR pin LOW
#define WR_IDLE *GPIO_DATA_REG = *GPIO_DATA_REG | WR_MASK // WR pin HIGH
#define RD_ACTIVE *GPIO_DATA_REG = *GPIO_DATA_REG & ~RD_MASK // RD pin LOW
#define RD_IDLE *GPIO_DATA_REG = *GPIO_DATA_REG | RD_MASK // RD pin HIGH
#define RES_ACTIVE *GPIO_DATA_REG = *GPIO_DATA_REG | RES_MASK // RES pin HIGH
#define RES_IDLE *GPIO_DATA_REG = *GPIO_DATA_REG & ~RES_MASK // RES pin LOW
#define WR_STROBE {WR_ACTIVE; WR_IDLE;} // WR pin LOW to WR pin HIGH
#define RD_STROBE {RD_ACTIVE; RD_IDLE;} // RD pin LOW to RD pin HIGH

#define DELAY for(int i=0; i<1;i++){/*for(int j=0;j<1;j++){}*/} // DELAY change the 1 to 5 to increase delay

#define DATA_MASK 0x000000FF // D0-D7 Data Pins Mask

#define write8(d) { *GPIO_DATA_REG = (*GPIO_DATA_REG&~DATA_MASK) | (d & DATA_MASK);WR_STROBE } // WRITE D0-D7 with HEX value 'd'
#define read8(dst) { RD_ACTIVE;DELAY;dst=(*GPIO_DATA_REG)&DATA_MASK;RD_IDLE; } // READ D0-D7 data

#define write16(d) { uint8_t h = (d)>>8, l = d; write8(h); write8(l); } // WRITE 16-bit data on D0-D7, 8 bit at a time
#define read16(dst) { uint8_t hi; read8(hi); read8(dst); dst |= (hi << 8); } // READ 16-bit data on D0-D7
#define writeCmd8(x){ CD_COMMAND; write8(x); CD_DATA;  } // WRITE 8-bit Command
#define writeData8(x){  write8(x) } // WRITE 8-bit Data
#define writeCmd16(x){ CD_COMMAND; write16(x); CD_DATA; } // WRITE 16-bit Command
#define writeData16(x){ write16(x) } // WRITE 16-bit Data

#define setWriteDir() {*GPIO_DIRECTION_CNTRL_REG = *GPIO_DIRECTION_CNTRL_REG|DATA_MASK;} // D0-D7 pins to OUTPUT
#define setReadDir() {*GPIO_DIRECTION_CNTRL_REG = *GPIO_DIRECTION_CNTRL_REG & ~DATA_MASK;} // D0-D7 pins to INPUT

// Set value of TFT register: 8-bit address, 8-bit value
#define writeCmdData8(a, d) { CD_COMMAND; write8(a); CD_DATA; write8(d); } 

// Set value of TFT register: 16-bit address, 16-bit value
// See notes at top about macro expansion, hence hi & lo temp vars
#define writeCmdData16(a, d) { \
  uint8_t hi, lo; \
  hi = (a) >> 8; lo = (a); CD_COMMAND; write8(hi); write8(lo); \
  hi = (d) >> 8; lo = (d); CD_DATA   ; write8(hi); write8(lo); }
