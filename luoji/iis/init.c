#include "nuc970.h"
#include "sys.h"
#include "init.h"


extern int __bss_start__;
extern int __bss_end__;

void clean_bss()
{
  int *bss_start = &__bss_start__;
  int *bss_end  = &__bss_end__;

  while(bss_start < bss_end)
  {
    *bss_start = 0;
    bss_start++; 
  }
}

void led_on()
{
  GPIO_ClrBit(GPIOB, (BIT4|BIT5));
}
void led_off()
{
  GPIO_SetBit(GPIOB, (BIT4|BIT5));
}

/* led1: PB4,  led5: PB5*/
void led_init(void)
{
  GPIO_OpenBit(GPIOB, (BIT4 | BIT5), DIR_OUTPUT, NO_PULL_UP);
  GPIO_SetBit(GPIOB, BIT4);
  GPIO_ClrBit(GPIOB, BIT5);
}

void delay(volatile int i)
{
  volatile  int j = 10000;
  while (i--)
  {
   while (j--);
   j = 10000;
  }
}

void init(void)
{
  /* Disable WDT */
  outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) & ~(1<<0)); 
  
  led_init();
  sysInitializeUART();
  nand_initialize();
}


