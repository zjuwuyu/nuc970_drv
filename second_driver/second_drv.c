#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <asm/io.h>

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


static struct class *seconddrv_class;
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


static int second_drv_open(struct inode *inode, struct file *file)
{
  *pclken0 |= (1<<3);
  *gpbmfpl &= ~(0xff<<16);
  *gpfmfph &= ~(0xffff<<12);
  *gpbdir |= (0x3 << 4);

  *gpfdir &= ~(0xf<<11);
 
  return 0;
}
static ssize_t second_drv_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
  unsigned char key_vals[KEY_NUM];
  int reg_val;
  int i;

  if (count != sizeof(key_vals))
  {
    printk("%s size not correct\n", __func__);
    return -1;
  }
  reg_val = (*gpfdatain >> 11) & 0xf;
  
  for (i = 0; i < 4; i++)
  {
    if (reg_val & (1<<i))
      key_vals[i] = 1;
    else
      key_vals[i] = 0;
  }

  copy_to_user(buf, key_vals, sizeof(key_vals));

//  for (i = 0; i < 4; i++)
//    printk("key_vals[%d]=%d\n", i, key_vals[i]);
//  printk("key_vals = 0x%x\n", reg_val);


  return sizeof(key_vals);
}
static ssize_t second_drv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
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

static struct file_operations second_drv_ops = {
	.owner = THIS_MODULE,
	.open  = second_drv_open,
	.read  = second_drv_read,
	.write = second_drv_write,
};

int major;
int second_drv_init(void)
{
  int i = 0;
  
  if (major != 0)
    register_chrdev(major, "second_drv", &second_drv_ops);
  else
    major = register_chrdev(0, "second_drv", &second_drv_ops);

  seconddrv_class = class_create(THIS_MODULE, "second_drv");
  if (IS_ERR(seconddrv_class))
    goto error;

  for (i = 0; i < KEY_NUM; i++)
  {
    key_dev[i] = device_create(seconddrv_class, NULL, MKDEV(major, i), NULL, "key%d", i);
    if (IS_ERR(key_dev[i]))
      goto error;
  }
    

  gpbmfpl = (volatile unsigned long *)ioremap(SYS_GPB_MFPL, 16);
  gpbmfph = gpbmfpl + 1;
  gpfmfpl = (volatile unsigned long *)ioremap(SYS_GPF_MFPL, 16);
  gpfmfph = gpbmfpl + 1;

  gpbdir = (volatile unsigned long *)ioremap(GPIOB_DIR, 16);
  gpbdataout = gpbdir + 1;


  
  gpfdir     = (volatile unsigned long *)ioremap(GPIOF_DIR, 16);
  gpfdataout = gpfdir + 1;
  gpfdatain  = gpfdataout + 1;

  pclken0 = (volatile unsigned long *)ioremap(CLK_PCLKEN0, 16);

  
	return 0;

error:
  return -1;
}

void second_drv_exit(void)
{ 
  int i  = 0;
  for (i = 0; i < KEY_NUM; i++)
  {
    device_destroy(seconddrv_class, MKDEV(major, i));
  }
  class_destroy(seconddrv_class);
  unregister_chrdev(major, "second_drv");

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

module_init(second_drv_init);
module_exit(second_drv_exit);

MODULE_LICENSE("GPL");


