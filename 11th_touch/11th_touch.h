#ifndef __10TH_FB_DRV_H__
#define __10TH_FB_DRV_H__

#define CHECK_NULL(_ptr) \
  if(NULL == _ptr) \
  { \
    printk("null pointer, %s, %d\n", __func__, __LINE__);\
    goto err;\
  }

struct lcd_ioreg_list
{
  char name[16];
  unsigned int *remap_addr;
  struct list_head list;
};

#define SYS_BA 0xB0000000
#define ADC_BA 0xB800A000
#define CLK_BA 0xB0000200
#define GPIO_BA 0xB8003000
#define LCM_BA 0xB0008000

struct mfp_reg 
{
  volatile unsigned int SYS_GPA_MFPL;
  volatile unsigned int SYS_GPA_MFPH;
  volatile unsigned int SYS_GPB_MFPL;
  volatile unsigned int SYS_GPB_MFPH;
  volatile unsigned int SYS_GPC_MFPL;
  volatile unsigned int SYS_GPC_MFPH;
  volatile unsigned int SYS_GPD_MFPL;
  volatile unsigned int SYS_GPD_MFPH;
  volatile unsigned int SYS_GPE_MFPL;
  volatile unsigned int SYS_GPE_MFPH;
  volatile unsigned int SYS_GPF_MFPL;
  volatile unsigned int SYS_GPF_MFPH;
  volatile unsigned int SYS_GPG_MFPL;
  volatile unsigned int SYS_GPG_MFPH;
  volatile unsigned int SYS_GPH_MFPL;
  volatile unsigned int SYS_GPH_MFPH;
  volatile unsigned int SYS_GPI_MFPL;
  volatile unsigned int SYS_GPI_MFPH;
  volatile unsigned int SYS_GPJ_MFPL;
};

struct adc_reg{
  volatile unsigned int ADC_CTL;
  volatile unsigned int ADC_CONF;
  volatile unsigned int ADC_IER;
  volatile unsigned int ADC_ISR;
  volatile unsigned int ADC_WKISR;
  volatile unsigned int reserved1;
  volatile unsigned int reserved2;
  volatile unsigned int reserved3;
  volatile unsigned int ADC_XYDATA;
  volatile unsigned int ADC_ZDATA;
  volatile unsigned int ADC_DATA;
  volatile unsigned int ADC_VBATDATA;
  volatile unsigned int ADC_KPDATA;
  volatile unsigned int ADC_SELFDATA;
};
#endif


