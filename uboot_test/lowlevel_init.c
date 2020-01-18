#include "nuc970.h"

extern UINT32 __bss_start;
extern UINT32 __bss_end;

void enable_pll(BOOL bEnable)
{
  UINT32 reg;
  
  reg = inpw(REG_CLK_APLLCON);

  if (TRUE == bEnable)
  {
    //clear this bit to enable pll
    reg &= (~(1<<28)); 
    reg |= ((1<<30));
  }
  else
  {
    //clear this bit to enable pll
    reg |= (1<<28); 
    reg &= (~(1<<30));
  }
  
  outpw(REG_CLK_APLLCON, reg);
}
void lowlevel_init(void)
{
  UINT32 reg;

  /* P=1, M=1, N=25, Fvco=300MHz */
  reg = inpw(REG_CLK_APLLCON);
  reg &= (~0xffff);  
  reg |= ((0<<13) | (0<<7) | (24 << 0));
  outpw(REG_CLK_APLLCON, reg);

  enable_pll(TRUE);

  /* wait pll stable */
  while (0 == (inpw(REG_CLK_APLLCON) & (1<<31)));

  /* Select APLL as SYSCLK */
  reg = inpw(REG_CLK_DIVCTL0);
  reg &= ~(0x7<<0); //clear SYSTEM_SDIV
  reg &= ~(0x3<<0x3);
  reg |=  (0x2<<0x3);
  outpw(REG_CLK_DIVCTL0, reg);

  /* setup uart0 clock , 300/(4+1)/(4+1) = 12MHz as input uart0 clock*/
  reg = inpw(REG_CLK_DIVCTL4);
  reg &= ~0x7; 
  reg |= 0x4; 
  reg &= ~(0x3<<0x3);
  reg |=  (0x2<<0x3);
  reg &= ~(7<<5);
  reg |=  (4<<0x5);
  
  outpw(REG_CLK_DIVCTL4, reg);
}


void clean_bss(void)
{
  UINT32 *p = NULL;

  for (p = &__bss_start; p < &__bss_end; p++)
  {
    *p = 0;
  }
}
