#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <mach/irqs.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/mutex.h>


#define KEY_NUM 4

#define SYS_BA 0xb0000000
#define SYS_GPB_MFPL (SYS_BA+0x78)
#define SYS_GPB_MFPH (SYS_BA+0x7C)
#define SYS_GPF_MFPL (SYS_BA+0x98)
#define SYS_GPF_MFPH (SYS_BA+0x9C)


#define GPIO_BA 0xb8003000
#define GPIOB_DIR (GPIO_BA+0x40)
#define GPIOB_DATAOUT (GPIO_BA+0x44)

#define GPIOF_DIR (GPIO_BA+0x140)
#define GPIOF_DATAOUT (GPIO_BA+0x144)
#define GPIOF_DATAIN (GPIO_BA+0x148)

#define CLK_BA 0xb0000200
#define CLK_PCLKEN0 (CLK_BA+0x18)


static struct class *sixthdrv_class;
static struct device *key_dev[4];


int major = 0;

volatile unsigned long *gpbmfpl = NULL;
volatile unsigned long *gpbmfph = NULL;

volatile unsigned long *gpfmfpl = NULL;
volatile unsigned long *gpfmfph = NULL;

volatile unsigned long *gpbdir = NULL;
volatile unsigned long *gpbdataout = NULL;

volatile unsigned long *gpfdir = NULL;
volatile unsigned long *gpfdataout = NULL;
volatile unsigned long *gpfdatain = NULL;

volatile unsigned long *pclken0 = NULL;

volatile unsigned char key_val;

DECLARE_WAIT_QUEUE_HEAD(button_wq);

static volatile int ev_press = 0;


static struct fasync_struct *button_async;

atomic_t canopen = ATOMIC_INIT(1);


DEFINE_MUTEX(button_lock);

struct key_desc
{
  int key_pin;
  int key_val;
};

struct key_desc key_table[] = 
{
  {NUC970_PF13, 0x3},//release->0x3, press->0x83
  {NUC970_PF14, 0x4},//release->0x4, press->0x84
};



irqreturn_t myirq_handler(int irq, void *dev_id)
{
  unsigned int reg_val;
  int shift_num = 0;
  
  struct key_desc *key_desc = (struct key_desc *)dev_id;
  
  reg_val = *gpfdatain;
  
  shift_num = (key_desc->key_pin == NUC970_PF13) ? 13:14;

  if (reg_val & (1<<shift_num))
  {
    key_desc->key_val &= ~(1<<7);
  }
  else
  {
    key_desc->key_val |= (1<<7);
  }

  key_val = key_desc->key_val;
  
  printk("irq=%d, key_val=0x%x\n", irq, key_desc->key_val);

  ev_press = 1;
  wake_up_interruptible(&button_wq);

  kill_fasync(&button_async, SIGIO, POLLIN);
  
  return IRQ_HANDLED;
}



static int sixth_drv_open(struct inode *inode, struct file *file)
{  
  int i = 0;
  int ret = 0;
  printk("sixth_drv_open enter");
  unsigned int flag;
  flag = file->f_flags;

  if (flag & O_NONBLOCK)
  {
    if (!mutex_trylock(&button_lock))
    {
      return -EAGAIN;
    }
  }
  else
  {
    mutex_lock(&button_lock);
  }
  
  *pclken0 |= (1<<3);
  *gpbdir |= (0x3 << 4);
//  *gpbmfpl &= ~(0xff<<16);
//  *gpfmfph &= ~(0xffff<<12);

  for (i = 0; i < sizeof(key_table)/sizeof(key_table[0]); i++)
  {
    ret = request_irq(gpio_to_irq(key_table[i].key_pin), myirq_handler, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "buttons", &key_table[i]);
    if (ret)
      printk("request_irq %d failed, ret = %d\n", i, ret);
  }  

  return 0;
}
static ssize_t sixth_drv_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
  unsigned int flag;
  flag = file->f_flags;

  printk("enter read\n");
  if (count != 1)
  {
    printk("%s size not correct\n", __func__);
    return -1;
  }
  
  if (flag & O_NONBLOCK)
  {
    if (!ev_press)
    {
     return -EAGAIN;
    }
  }
  else
  {
    wait_event_interruptible(button_wq, ev_press);
  }
  ev_press = 0;

  copy_to_user(buf, &key_val, 1);

  return 1;
}
static ssize_t sixth_drv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
  int val;
  int minor = MINOR(file->f_inode->i_rdev);
  
  copy_from_user(&val, buf, count);

  if (minor == 0)
  {
    if (1 == val)
    {
      *gpbdataout &= ~(1 << 4);
    }
    else
    {
      *gpbdataout |= (1 << 4);
    }
  }
  else if (minor == 1)
  {
    if (1 == val)
    {
      *gpbdataout &= ~(1 << 5);
    }
    else
    {
      *gpbdataout |= (1 << 5);
    }
  }
  return 0;
}

static unsigned int sixth_drv_poll (struct file *file, struct poll_table_struct *wait)
{
  unsigned int mask = 0;

  poll_wait(file,&button_wq, wait);

  if (ev_press)
    mask |= (POLLIN | POLLRDNORM);
    
  return mask;
}

static int sixth_drv_fasync (int fd, struct file *file, int on)
{
  printk("%s enter\n", __func__);
  return fasync_helper(fd, file, on, &button_async);
}

static int sixth_drv_close (struct inode *inode, struct file *file)
{
  int i = 0;
  
  atomic_inc(&canopen);
  
  for (i = 0; i < sizeof(key_table)/sizeof(key_table[0]); i++)
  {
    free_irq(gpio_to_irq(key_table[i].key_pin),  &key_table[i]);
  }  

  mutex_unlock(&button_lock);

  return 0;
}


static struct file_operations sixth_drv_ops = {
	.owner = THIS_MODULE,
	.open  = sixth_drv_open,
	.read  = sixth_drv_read,
	.write = sixth_drv_write,
	.poll = sixth_drv_poll,
	.release = sixth_drv_close,
	.fasync = sixth_drv_fasync,
	
};

int major;
int sixth_drv_init(void)
{
  int i = 0;
  
  int *mydata =  NULL;
  mydata = (int *)kzalloc(100, GFP_KERNEL);
  
  if (major != 0)
    register_chrdev(major, "sixth_drv", &sixth_drv_ops);
  else
    major = register_chrdev(0, "sixth_drv", &sixth_drv_ops);

  sixthdrv_class = class_create(THIS_MODULE, "sixth_drv");
  if (IS_ERR(sixthdrv_class))
    goto error;

  for (i = 0; i < KEY_NUM; i++)
  {
    key_dev[i] = device_create(sixthdrv_class, NULL, MKDEV(major, i), NULL, "button%d", i);
    if (IS_ERR(key_dev[i]))
      goto error;
  }
    

  gpbmfpl = (volatile unsigned long *)ioremap(SYS_GPB_MFPL, 16);
  gpbmfph = gpbmfpl + 1;
  gpbdir = (volatile unsigned long *)ioremap(GPIOB_DIR, 16);
  gpbdataout = gpbdir + 1;
  
  gpfmfpl = (volatile unsigned long *)ioremap(SYS_GPF_MFPL, 16);
  gpfmfph = gpbmfpl + 1;
  gpfdir     = (volatile unsigned long *)ioremap(GPIOF_DIR, 16);
  gpfdataout = gpfdir + 1;
  gpfdatain  = gpfdataout + 1;

  pclken0 = (volatile unsigned long *)ioremap(CLK_PCLKEN0, 16);

	return 0;

error:
  return -1;
}

void sixth_drv_exit(void)
{ 
  int i  = 0;
  for (i = 0; i < KEY_NUM; i++)
  {
    device_destroy(sixthdrv_class, MKDEV(major, i));
  }
  class_destroy(sixthdrv_class);
  unregister_chrdev(major, "sixth_drv");

  iounmap(gpbmfpl);
  iounmap(gpbmfph);
  iounmap(gpfmfph);
  iounmap(gpfmfph);
  iounmap(gpbdir);
  iounmap(gpbdataout);
  iounmap(gpfdir);
  iounmap(gpfdataout);
  iounmap(gpfdatain);
  iounmap(pclken0);

}

module_init(sixth_drv_init);
module_exit(sixth_drv_exit);

MODULE_LICENSE("GPL");


