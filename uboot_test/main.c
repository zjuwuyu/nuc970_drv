#include "serial.h"
#include "nand.h"

typedef enum
{ 
  UBOOT,
  KERNEL,
  BOOT_TYPE_MAX
} BOOT_TYPE_t;

//BOOT_TYPE_t eBootType = UBOOT;
BOOT_TYPE_t eBootType = KERNEL;


int main(void)
{  
  outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 0x10000);// UART clk
  outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 0x100);  // Timer clk
  outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 0x8);    // GPIO clk

  init_serial();  
  nand_initialize();

  switch (eBootType)
  {
    case UBOOT:
    {
      void (*uboot)(void)= (void (*)(void))0xE00000;
      
      sysPutString("zwy Starting uboot...\r\n");
      nand_read(0x100000, 0xE00000, PAGE_SIZE * 512);
      uboot();
      break;
    }
    case KERNEL:
    {
      void (*theKernel) (int, int, UINT32) = (void (*) (int, int, UINT32))0x8000;
      
      sysPutString("zwy Starting kernel...\r\n");
      nand_read(0x200000+0x40, 0x8000, PAGE_SIZE * 2048);
      setup_tags();
      theKernel(0,0,1);
    }
  }
  
  return 0;
}
