void led_on(void)
{
  *((volatile unsigned int *)0xb0000218) = 0x00000008;
  *((volatile unsigned int *)0xb8003040) = 0x00000030;
  *((volatile unsigned int *)0xb8003044) = 0x0;
}
void led_off(void)
{
  *((volatile unsigned int *)0xb0000218) = 0x00000008;
  *((volatile unsigned int *)0xb8003040) = 0x00000030;
  *((volatile unsigned int *)0xb8003044) = 0xffffffff;
}

