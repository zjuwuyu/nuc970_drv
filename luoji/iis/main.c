#include <stdio.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"

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

int main(void)
{
    char c;

    *(volatile unsigned int *)(CLK_BA+0x18) |= (1<<16); /* Enable UART0 */
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();
    sysprintf("aaaa\n");
    printf("helloworld!");
}
