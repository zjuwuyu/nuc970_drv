


#ifndef __DMA_DRV_H__
#define __DMA_DRV_H__

#include <linux/ioctl.h>

#define CHECK_NULL(_ptr) \
  if(NULL == _ptr) \
  { \
    printk("null pointer, %s, %d\n", __func__, __LINE__);\
    goto err;\
  }

#define IOCTL_CMD_WITH_DMA _IO('v',1)
#define IOCTL_CMD_NO_DMA   _IO('v',2)


#endif


