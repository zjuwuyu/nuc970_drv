#include <stdio.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"
#include "gpio.h"

extern void init(void);

int main(void)
{
  sysDisableCache();
  sysFlushCache(I_D_CACHE);
  sysEnableCache(CACHE_WRITE_BACK);
  sysInitializeUART();
  init();

  printf("after sysEnableCache a=%x, b=%d, str=%s\n\r", 123, 19, "this is iis test");

  while(1)
  {
    led_on();
    delay(100);
    led_off();
    delay(100);
  }
}
