#include "nuc970.h"
#include "sys.h"
#include "init.h"
#include "pr.h"


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
  led_off();
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


INT32 key_eint_handler(UINT32 status, UINT32 userData)
{
  volatile UINT32 uIper = inpw(REG_AIC_IPER);
  
  pr_info("zwy enter %s,userData=%d, uIper=%d\n", __func__, userData, uIper);
  
  if (0 == userData)
  {
    GPIO_ClrISRBit(GPIOF, BIT11);
  }
  else if (1 == userData)
  {
    GPIO_ClrISRBit(GPIOF, BIT12);
  }
  else if (2 == userData)
  {
    GPIO_ClrISRBit(GPIOF, BIT13);
  }
  else if (3 == userData)
  {
    GPIO_ClrISRBit(GPIOF, BIT14);
  }

}

INT32 key_gpio_int_handler(UINT32 status, UINT32 userData)
{
  volatile UINT32 uIper = inpw(REG_AIC_IPER);
  
  pr_info("zwy enter %s,userData=%d, uIper=%d\n", __func__, userData, uIper);
  
  if (3 == userData)
  {
    GPIO_ClrISRBit(GPIOF, status);
  }
}


void key_eint_init()
{
  outpw(REG_SYS_GPF_MFPH, inpw(REG_SYS_GPF_MFPH)| (0xffff<<12));

  GPIO_OpenBit(GPIOF, BIT11|BIT12|BIT13|BIT14, DIR_INPUT, NO_PULL_UP);
  GPIO_EnableTriggerType(GPIOF, BIT11|BIT12|BIT13|BIT14, FALLING);
  
  GPIO_EnableEINT(NIRQ0, key_eint_handler, 0);
  GPIO_EnableEINT(NIRQ1, key_eint_handler, 1);
  GPIO_EnableEINT(NIRQ2, key_eint_handler, 2);
  GPIO_EnableEINT(NIRQ3, key_eint_handler, 3);
}

void key_gpio_init()
{
  GPIO_OpenBit(GPIOF, BIT14, DIR_INPUT, NO_PULL_UP);
  GPIO_EnableTriggerType(GPIOF, BIT14, FALLING);
  GPIO_EnableDebounce(100000);

  GPIO_EnableInt(GPIOF, key_gpio_int_handler, 3);
}


void init(void)
{
  /* Disable WDT */
  outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) & ~(1<<0)); 
  
  led_init();
  sysInitializeUART();
  nand_initialize();

  key_eint_init();
  //key_gpio_init();
}


