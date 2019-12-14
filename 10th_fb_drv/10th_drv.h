


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

#endif


