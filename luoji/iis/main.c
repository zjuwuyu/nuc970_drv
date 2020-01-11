#include <stdio.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"
#include "gpio.h"

extern int __bss_start__;
extern int __bss_end__;
extern void init(void);


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

/* led1: PB4,  led5: PB5*/
void led_init(void)
{
  int ret = 0;
  ret = GPIO_OpenBit(GPIOB, (BIT4 | BIT5), DIR_OUTPUT, NO_PULL_UP);
  GPIO_SetBit(GPIOB, BIT4);
  GPIO_ClrBit(GPIOB, BIT5);
}
void led_on()
{
  
}
int main(void)
{
    init();
    
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    led_init();
    sysInitializeUART();
    sysEnableCache(CACHE_WRITE_BACK);
    
    
    printf("a=%x, b=%d, str=%s\n", 123, 19, "this is iis test");

}
