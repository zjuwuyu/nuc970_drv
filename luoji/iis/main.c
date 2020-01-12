#include <stdio.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"
#include "pr.h"
#include "init.h"


int main(void)
{
  int i = 0;
  NAND_ID_TYPE sNandId;
  
  sysDisableCache();
  sysFlushCache(I_D_CACHE);
  sysEnableCache(CACHE_WRITE_BACK);
  
  init();
  
  nand_read_chip_id(&sNandId);
  pr_line("Nand ID:");
  for (i = 0; i < sizeof(sNandId.id)/sizeof(sNandId.id[0]); i++)
  {
    pr_info("0x%x ", sNandId.id[i]);
  }

  while(1)
  {
    led_on();
    delay(100);
    led_off();
    delay(100);
  }
}
