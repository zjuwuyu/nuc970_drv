#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/io.h>
#include <linux/pm.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/slub_def.h>
#include <linux/types.h>
#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-lcd.h>
#include <mach/mfp.h>
#include <mach/gpio.h>
#include "dma_drv.h"

#define BUFFER_SIZE 512*1024



typedef struct dma_reg {
  unsigned long gdma_ctl0;
  unsigned long gdma_srcba0;
  unsigned long gdma_dstba0;
  unsigned long gdma_tcnt0;
  unsigned long gdma_csrca0;
  unsigned long gdma_cdsta0;
  unsigned long gdma_ctcnt0;
  unsigned long gdma_dadr0;
  unsigned long gdma_ctl1;
  unsigned long gdma_srcba1;
  unsigned long gdma_dstba1;
  unsigned long gdma_tcnt1;
  unsigned long gdma_csrca1;
  unsigned long gdma_cdsta1;
  unsigned long gdma_ctcnt1;
  unsigned long gdma_dadr1;
  unsigned long reserved[16];
  unsigned long gdma_buffer0;
  unsigned long gdma_buffer1;
  unsigned long gdma_buffer2;
  unsigned long gdma_buffer3;
  unsigned long gdma_buffer4;
  unsigned long gdma_buffer5;
  unsigned long gdma_buffer6;
  unsigned long gdma_buffer7;
  unsigned long gdma_ints;
}dma_reg_t;

typedef struct nuc970_dma_data {
  struct cdev cdev;
  dev_t dev_id;
  int irq;
  struct clk *clk;
  dma_reg_t *regs;
} nuc970_dma_data_t;


nuc970_dma_data_t *pdma = NULL;
int major = 0;
struct class *nuc970_dma_class = NULL;
struct device *nuc970_dma_device = NULL;


unsigned char *src = NULL;
unsigned int   src_phys;
unsigned char *dst = NULL;
unsigned int   dst_phys;

DECLARE_WAIT_QUEUE_HEAD(dma_wait);
int dma_transfer_finish = 0;

void dma_soft_request()
{
  if (pdma)
    pdma->regs->gdma_ctl0 |= (1<<16);//Software Triggered GDMA Request
  else
    printk("%s failed\n", __func__);
}

long nuc970_dma_ioctl(struct file *file, unsigned int cmd, unsigned long data)
{
  int i = 0;
  switch(cmd) {
    case IOCTL_CMD_WITH_DMA:
      dma_transfer_finish = 0;
      dma_soft_request();
      wait_event_interruptible(dma_wait, dma_transfer_finish);
      if (memcmp(src, dst, BUFFER_SIZE)) {
        printk("buffer copy failed!\n");
      }
      
      break;
    case IOCTL_CMD_NO_DMA:
      for (i = 0; i < BUFFER_SIZE; i++) {
        dst[i] = src[i];        
      }
      if (memcmp(src, dst, BUFFER_SIZE)) {
        printk("buffer copy failed!\n");
      }
      break;
  }

  return 0;
}

static struct file_operations nuc972_dma_ops = {
	.owner			    = THIS_MODULE,
	.unlocked_ioctl = nuc970_dma_ioctl,
};

irqreturn_t dma_irq(int irq, void *data)
{
  int i = 0;
  pdma->regs->gdma_ints |= (1<<8); // clear TC0F flag
  dma_transfer_finish = 1;
  wake_up(&dma_wait);
  return IRQ_HANDLED;
}

static int nuc970_dma_init(void)
{
  int ret = 0;
  int i;
  
  printk("%s enter\n", __func__);
  pdma = (nuc970_dma_data_t*)kzalloc(sizeof(nuc970_dma_data_t), GFP_KERNEL);
  if (NULL == pdma) {
    printk("%s kzalloc nuc970_dma_data_t failed\n", __func__);
    return -1;
  }
  
  src = dma_alloc_writecombine(NULL, BUFFER_SIZE, &src_phys, GFP_KERNEL);
  if (NULL == src) {
    printk("%s dma_alloc_writecombine src failed\n", __func__);
    return -1;
  }

  for (i = 0; i < 1024; i++)
    src[i] = i;

  dst = dma_alloc_writecombine(NULL, BUFFER_SIZE, &dst_phys, GFP_KERNEL);
  if (NULL == dst) {
    printk("%s dma_alloc_writecombine dsr failed\n", __func__);
    return -1;
  }
  
  if (0 == major) {
    ret = alloc_chrdev_region(&pdma->dev_id, 0, 1, "nuc970_dma");
  } else {
    pdma->dev_id = MKDEV(major,0);
    ret = register_chrdev_region(pdma->dev_id, 1,  "nuc970_dma");
  }

  if (ret)
    printk("failed to get devid\n");
    
  cdev_init(&pdma->cdev, &nuc972_dma_ops);
  cdev_add(&pdma->cdev, pdma->dev_id, 1);
  
  nuc970_dma_class = class_create(THIS_MODULE, "nuc970_dma_class");
  if(NULL == nuc970_dma_class) {
    printk("%s class_create failed\n", __func__);
    return -1;
  }

  nuc970_dma_device = device_create(nuc970_dma_class, NULL, pdma->dev_id, NULL, "nuc970_dma");
  if(NULL == nuc970_dma_device) {
    printk("%s device_create failed\n", __func__);
    return -1;
  }

  /* Enable dma clk */
  pdma->clk = clk_get(NULL, "gdma_hclk");
  if (NULL == pdma->clk) {
    printk("failed to get dma clk");
    return -1;
  }

  clk_prepare(pdma->clk);
  clk_enable(pdma->clk);

  pdma->regs = (dma_reg_t *)ioremap(0xB0004000, sizeof(dma_reg_t));
  if (NULL == pdma->regs) {
    printk("ioremap failed\n");
    return -1;
  }

  pdma->regs->gdma_dadr0 |= (1<<2); //use Non-Descriptor-Fetch
  pdma->regs->gdma_ctl0 = ((1<<19) | (1<<17) | (0 << 12) | (1<<11)); // Auto init, Bus Lock, TWS=0, Block mode,

  pdma->regs->gdma_srcba0 = src_phys;
  pdma->regs->gdma_dstba0 = dst_phys;
  pdma->regs->gdma_tcnt0 = BUFFER_SIZE;
  pdma->regs->gdma_ints  = (1<<0);

  if(request_irq(IRQ_GDMA0, dma_irq, 0, "nuc970_dma_irq", NULL)) {
    printk("request_irq failed\n");
    return -1;
  }
  
  pdma->regs->gdma_ctl0 |= (1<<0);//GDMA Enable
  dma_soft_request();//Software Triggered GDMA Request

  
  return ret;
}

static void nuc970_dma_exit(void)
{
  device_destroy(nuc970_dma_class, pdma->dev_id);
  class_destroy(nuc970_dma_class);
  unregister_chrdev_region(pdma->dev_id, 1);

  if(pdma) {
    clk_disable(pdma->clk);

    free_irq(IRQ_GDMA0, NULL);
    
    if (pdma->regs) {
      iounmap(pdma->regs);
    }
    kfree(pdma);
  }
}

module_init(nuc970_dma_init);
module_exit(nuc970_dma_exit);
MODULE_LICENSE("GPL");
