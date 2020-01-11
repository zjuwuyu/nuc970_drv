#include "nuc970.h"
#include "sys.h"
#include "gpio.h"

void init(void)
{
  outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) & ~(1<<0)); // Disable WDT
}
