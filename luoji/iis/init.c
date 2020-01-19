#include "nuc970.h"
#include "sys.h"
#include "init.h"
#include "pr.h"
#include "i2s.h"

extern int __bss_start__;
extern int __bss_end__;

#define BUF_LENGTH  16*1024
#define BUF_HALF_LENGTH 8*1024

#ifdef __ICCARM__
#pragma data_alignment = 32
static uint32_t u32PlayBuf[BUF_LENGTH];
static uint32_t u32RecBuf[BUF_LENGTH];
#else
static uint32_t u32PlayBuf[BUF_LENGTH] __attribute__((aligned(32)));;
static uint32_t u32RecBuf[BUF_LENGTH] __attribute__((aligned(32)));;
#endif

static uint32_t volatile u32BuffIdx=0;
static uint32_t *pbuf, *rbuf;

void play_callback(uint32_t u32Sn)
{
	if(u32Sn == 1)
	{
		/* First half of buffer can be copied from Rx buffer */
		u32BuffIdx = 0;
	}
	else
	{
		/* Last half of buffer can be copied from Rx buffer */
		u32BuffIdx = BUF_HALF_LENGTH;
	}
}

void rec_callback(uint32_t u32Sn)
{
	if(u32Sn == 1)
	{
		/* Copy data form Rx buffer to Tx buffer */
		memcpy((void *)(pbuf + u32BuffIdx), (void *)rbuf, BUF_HALF_LENGTH*sizeof(uint32_t));
	}
	else
	{
		/* Copy data form Rx buffer to Tx buffer */
		memcpy((void *)(pbuf + u32BuffIdx), (void *)(rbuf + BUF_HALF_LENGTH), BUF_HALF_LENGTH*sizeof(uint32_t));
	}
}

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

  pr_info("zwy enter %s,userData=%d\n", __func__, userData);

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


/* I2S init */
void audio_init(void)
{
  /* Configure multi function pins to I2S */
  outpw(REG_SYS_GPG_MFPH, (inpw(REG_SYS_GPG_MFPH) & ~0x0FFFFF00) | 0x08888800);
  // Initialize I2S interface
  i2sInit();
  if(i2sOpen() != 0)
    return 0;
     
  // Select I2S function
  i2sIoctl(I2S_SELECT_BLOCK, I2S_BLOCK_I2S, 0);
  // Select 16-bit data width
  i2sIoctl(I2S_SELECT_BIT, I2S_BIT_WIDTH_16, 0);

  // Set DMA interrupt selection to half of DMA buffer
  i2sIoctl(I2S_SET_PLAY_DMA_INT_SEL, I2S_DMA_INT_HALF, 0);
  i2sIoctl(I2S_SET_REC_DMA_INT_SEL, I2S_DMA_INT_HALF, 0);

  // Set to stereo 
  i2sIoctl(I2S_SET_CHANNEL, I2S_PLAY, I2S_CHANNEL_P_I2S_TWO);
  i2sIoctl(I2S_SET_CHANNEL, I2S_REC, I2S_CHANNEL_R_I2S_TWO);

  // Set DMA buffer address
  i2sIoctl(I2S_SET_DMA_ADDRESS, I2S_PLAY, (uint32_t)u32PlayBuf);
  i2sIoctl(I2S_SET_DMA_ADDRESS, I2S_REC, (uint32_t)u32RecBuf);

  // Put to non cacheable region
  pbuf = (uint32_t *)((uint32_t)u32PlayBuf | (uint32_t)0x80000000);
  rbuf = (uint32_t *)((uint32_t)u32RecBuf | (uint32_t)0x80000000);

  // Set DMA buffer length
  i2sIoctl(I2S_SET_DMA_LENGTH, I2S_PLAY, sizeof(u32PlayBuf));
  i2sIoctl(I2S_SET_DMA_LENGTH, I2S_REC, sizeof(u32RecBuf));

  // Select I2S format
  i2sIoctl(I2S_SET_I2S_FORMAT, I2S_FORMAT_I2S, 0);

  //12.288MHz ==> APLL=98.4MHz / 8 = 12.3MHz

  //APLL is 98.4MHz
  outpw(REG_CLK_APLLCON, 0xC0008028);

  // Select APLL as I2S source and divider is (7+1)
  outpw(REG_CLK_DIVCTL1, (inpw(REG_CLK_DIVCTL1) & ~0x001f0000) | (0x2 << 19) | (0x7 << 24));

  // Set sampleing rate is 16k, data width is 16-bit, stereo
  i2sSetSampleRate(12300000, 16000, 16, 2);

  // Set as master
  i2sIoctl(I2S_SET_MODE, I2S_MODE_MASTER, 0);


  // Set play and record call-back functions
  i2sIoctl(I2S_SET_I2S_CALLBACKFUN, I2S_PLAY, (uint32_t)&play_callback);
  i2sIoctl(I2S_SET_I2S_CALLBACKFUN, I2S_REC, (uint32_t)&rec_callback); 
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

  audio_init();
}


