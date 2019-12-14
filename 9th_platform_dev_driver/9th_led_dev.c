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
#include <linux/input.h>
#include <linux/platform_device.h>


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
int major;

void *my_dev_id = NULL;

static struct input_dev *btn_input_device = NULL;

struct timer_list timer;

DEFINE_MUTEX(button_lock);

struct key_desc
{
  int key_pin;
  int key_val;
  int input_key;
};

struct key_desc key_table[] = 
{
  {NUC970_PF13, 0x3, KEY_L},//release->0x3, press->0x83
  {NUC970_PF14, 0x4, KEY_ENTER},//release->0x4, press->0x84
};

irqreturn_t myirq_handler(int irq, void *dev_id)
{
  mod_timer(&timer, jiffies+HZ/50);
  my_dev_id = dev_id;
  
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



  return 0;
}
static ssize_t sixth_drv_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
  unsigned int flag;
  flag = file->f_flags;

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

void my_timer_function(unsigned long data)
{
  unsigned int reg_val;
  int shift_num = 0;
  int isKeyDown = 0;
  
  struct key_desc *key_desc = (struct key_desc *)my_dev_id;

  if (NULL == key_desc)
    return 0;
  
  reg_val = *gpfdatain;
  
  shift_num = (key_desc->key_pin == NUC970_PF13) ? 13:14;

  if (reg_val & (1<<shift_num))
  {
    //key up
    input_event(btn_input_device, EV_KEY, key_desc->input_key, 0);
    input_sync(btn_input_device);
  }
  else
  {
    //key down
    input_event(btn_input_device, EV_KEY, key_desc->input_key, 1);
    input_sync(btn_input_device);
  }

  key_val = key_desc->key_val;
  
  ev_press = 1;
  wake_up_interruptible(&button_wq);

  kill_fasync(&button_async, SIGIO, POLLIN);
}

void	led_device_release(struct device *dev)
{
  printk("%s is called\n", __func__);
}


struct resource led_res[] =
{
  {
  .name = "nuc972_led_res",
  .start = 1,
  .end = 222,
  .flags = IORESOURCE_MEM,
  },
};

struct platform_device led_dev =
{
  .name = "nuc972_led",
  .id = -1,
  .num_resources = 1,
	.resource = &led_res,
	.dev={
	  .release = led_device_release,
	},
};


int sixth_drv_init(void)
{
  platform_device_register(&led_dev);
  return 0;
}

void sixth_drv_exit(void)
{  
  platform_device_unregister(&led_dev);
}



module_init(sixth_drv_init);
module_exit(sixth_drv_exit);

MODULE_LICENSE("GPL");


