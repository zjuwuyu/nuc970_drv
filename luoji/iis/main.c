#include <stdio.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"

int main(void)
{
    char c;

    *(volatile unsigned int *)(CLK_BA+0x18) |= (1<<16); /* Enable UART0 */
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();
    printf("helloworld!");
}
