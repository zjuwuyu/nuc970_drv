#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <asm/io.h>

#define SYS_BA 0xb0000000
#define SYS_GPB_MFPL (SYS_BA+0x78)
#define SYS_GPB_MFPH (SYS_BA+0x7C)

#define GPIO_BA 0xb8003000
#define GPIOB_DIR (GPIO_BA+0x40)
#define GPIOB_DATAOUT (GPIO_BA+0x44)

#define CLK_BA 0xb0000200
#define CLK_PCLKEN0 (CLK_BA+0x18)


static struct class *firstdrv_class;
static struct device *led0_dev;
static struct device *led1_dev;

int major = 0;

volatile unsigned long *gpbmfpl = NULL;
volatile unsigned long *gpbmfph = NULL;
volatile unsigned long *gpbdir = NULL;
volatile unsigned long *gpbdataout = NULL;
volatile unsigned long *pclken0 = NULL;


static int first_drv_open(struct inode *inode, struct file *file)
{
  printk("%s is called\n", __func__);

  *pclken0 |= (1<<3);
  *gpbmfpl &= ~(0xff<<16);
  *gpbdir |= (0x3 << 4);
 
  return 0;
}
static ssize_t first_drv_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
  printk("%s is called\n", __func__);
  return 0;
}
static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
  int val;
  int minor = MINOR(file->f_inode->i_rdev);
  
  printk("%s is called, minor=%d\n", __func__, minor);

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

static struct file_operations first_drv_ops = {
	.owner = THIS_MODULE,
	.open  = first_drv_open,
	.read  = first_drv_read,
	.write = first_drv_write,
};

int major;
int first_drv_init(void)
{
  if (major != 0)
    register_chrdev(major, "first_drv", &first_drv_ops);
  else
    major = register_chrdev(0, "first_drv", &first_drv_ops);

  firstdrv_class = class_create(THIS_MODULE, "first_drv");
  if (IS_ERR(firstdrv_class))
    goto error;

  led0_dev = device_create(firstdrv_class, NULL, MKDEV(major, 0), NULL, "led0");
  if (IS_ERR(led0_dev))
    goto error;

  led1_dev = device_create(firstdrv_class, NULL, MKDEV(major, 1), NULL, "led1");
  if (IS_ERR(led1_dev))
    goto error;
    

  gpbmfpl = (volatile unsigned long *)ioremap(SYS_GPB_MFPL, 16);
  gpbmfph = gpbmfpl + 1;

  gpbdir = (volatile unsigned long *)ioremap(GPIOB_DIR, 16);
  gpbdataout = gpbdir + 1;

  pclken0 = (volatile unsigned long *)ioremap(CLK_PCLKEN0, 16);

  
	return 0;

error:
  return -1;
}

void first_drv_exit(void)
{
  device_destroy(firstdrv_class, MKDEV(major, 0));
  device_destroy(firstdrv_class, MKDEV(major, 1));
  class_destroy(firstdrv_class);
  unregister_chrdev(major, "first_drv");
}

module_init(first_drv_init);
module_exit(first_drv_exit);

MODULE_LICENSE("GPL");


