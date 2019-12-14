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
#include <linux/list.h>
#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-lcd.h>
#include <mach/mfp.h>
#include <mach/gpio.h>
#include <linux/input.h>

#include "11th_touch.h"


volatile unsigned int *sys_apbiprst1        = NULL;
volatile unsigned int *adc_xysort0          = NULL;
volatile unsigned int *adc_xysort1          = NULL;
volatile unsigned int *adc_xysort2          = NULL;
volatile unsigned int *adc_xysort3          = NULL;

static struct adc_reg *adc_reg             = NULL;
static struct list_head reg_list;

static void touch2detect(void);
static void detect2touch(void);

struct timer_list check_pen_up_timer;
struct timer_list do_xy_convertion_tiemr;

struct input_dev *input_ts_dev = NULL;

struct tasklet_struct touch_task;
static void adc_menu_start(void);

void trigger_xy_convertion(unsigned long data)
{ 
//  printk("trigger_xy_convertion\n");
  adc_menu_start();//start conversion again here, so that will enter irq hander again
}


unsigned int* register_reg_map_item(char *name, unsigned int map_base_addr, unsigned int map_size)
{
  struct lcd_ioreg_list *lcd_ioreg_list;
  
  lcd_ioreg_list = kzalloc(sizeof(struct lcd_ioreg_list), GFP_KERNEL);
  CHECK_NULL(lcd_ioreg_list);
  
  lcd_ioreg_list->remap_addr = ioremap(map_base_addr, map_size);
  strcpy(lcd_ioreg_list->name, name);
  list_add(&lcd_ioreg_list->list, &reg_list);

  return lcd_ioreg_list->remap_addr;

err:
  return 0;
}

volatile unsigned int* get_remaped_addr_by_name(char *name)
{
  struct list_head *l;
  struct lcd_ioreg_list *reg = NULL;

  list_for_each(l, &reg_list)
  {
    reg = list_entry(l, struct lcd_ioreg_list , list);
    CHECK_NULL(reg);

    if (!strcmp(reg->name, name))
      return reg->remap_addr;
  }

  return 0;
err:
  return 0;
}

int io_remap_regs(void)
{
  sys_apbiprst1    = register_reg_map_item("sys_apbiprst1",    SYS_BA+0x68, 4);
  adc_reg          = (struct adc_reg*)register_reg_map_item("adc_reg", ADC_BA, sizeof(struct adc_reg));
  adc_xysort0      = register_reg_map_item("adc_xysort0",    SYS_BA+0x1f4, 4);
  adc_xysort1      = register_reg_map_item("adc_xysort1",    SYS_BA+0x1f8, 4);
  adc_xysort2      = register_reg_map_item("adc_xysort2",    SYS_BA+0x1fc, 4);
  adc_xysort3      = register_reg_map_item("adc_xysort3",    SYS_BA+0x200, 4);

   
  CHECK_NULL(sys_apbiprst1);
  CHECK_NULL(adc_reg);
  CHECK_NULL(adc_xysort0);
  CHECK_NULL(adc_xysort1);
  CHECK_NULL(adc_xysort2);
  CHECK_NULL(adc_xysort3);
  

return 0;

err:
  return -1;
}

int io_unremap_regs(void)
{
  struct lcd_ioreg_list *reg = NULL;
  struct lcd_ioreg_list *reg1 = NULL;

  list_for_each_entry_safe(reg, reg1, &reg_list, list)
  {
    iounmap(reg->remap_addr);
    list_del(&reg->list);
    kfree(reg);
  }
 
  return 0;
}

static void report_xy_data(int x, int y, int pressure, int btn)
{
  input_report_key(input_ts_dev, BTN_TOUCH, btn);
  input_report_abs(input_ts_dev, ABS_PRESSURE, pressure);
  input_report_abs(input_ts_dev, ABS_X, x);
  input_report_abs(input_ts_dev, ABS_Y, y);
  input_sync(input_ts_dev);
}

static int pen_detected(void)
{
  unsigned int z1, z2;
  unsigned int z1_threshold = 200;

  z1 = (adc_reg->ADC_ZDATA & 0xfff);
  z2 = ((adc_reg->ADC_ZDATA) >> 16 & 0xfff);
//  printk("z1=%d, z2=%d\n", z1, z2);
  
  if (z1 > z1_threshold)
    return 1;
  else
    return 0;

}
static void adc_menu_start(void)
{
  adc_reg->ADC_CTL |= (1<<8);
}


void my_touch_tasklet(unsigned long data)
{
  unsigned int x, y;
  int z = 200; 
  int i,xdata,ydata;

  x = (adc_reg->ADC_XYDATA & 0xfff);
  y = ((adc_reg->ADC_XYDATA >> 16) & 0xfff);

//  for(i=0;i<=3;i+=1)
//  {
//    xdata = (*(adc_xysort0+i) & 0xfff);
//    ydata = ((*(adc_xysort0+i)>>16) & 0xfff);
//    if(xdata==0 || xdata==0xFFF || ydata==0 || ydata==0xfff ||abs(xdata-x)>50 || abs(ydata-y)>50)
//    {
//      printk("conversion data is failed, xdata=%d, ydata=%d, x=%d, y=%d\n", xdata, ydata, x, y);
//      adc_menu_start();
//      return;
//    }
//  }

  report_xy_data(x, y, 200, 1);
  
  if (!pen_detected())
  {
    printk("my_touch_tasklet pen not detected\n");
    touch2detect();
    report_xy_data(x, y, 0, 0);
  }
  else
  {
    mod_timer(&do_xy_convertion_tiemr, jiffies + msecs_to_jiffies(20));
  }
}

irqreturn_t adc_irq_handler(int irq, void *dev_id)
{ 
  static unsigned int pen_up_count = 0;

  if ((adc_reg->ADC_ISR & (1<<17)) && (adc_reg->ADC_ISR & (1<<2)))
  {
  //  udelay(1);
    adc_reg->ADC_ISR |= ((1<<2) | (1<<17)) ;
    detect2touch();
    adc_menu_start();
    printk("down\n");
  }
  else if (adc_reg->ADC_ISR & (1<<4))
  {
  //  udelay(1);
    adc_reg->ADC_ISR |= (1<<4);
    touch2detect();
    printk("up\n");
  }

 // printk("#0 ADC_ISR=0x%x, ADC_IER=0x%x, ADC_CONF=0x%x\n", adc_reg->ADC_ISR, adc_reg->ADC_IER, adc_reg->ADC_CONF);

  if (adc_reg->ADC_ISR & (1<<0))
  {
    //T_F & Z_F
    if (((adc_reg->ADC_ISR & (1<<8) && adc_reg->ADC_CONF & (1<<0))) &&
        ((adc_reg->ADC_ISR & (1<<9) && adc_reg->ADC_CONF & (1<<1))))
    {
  //    printk("#1 ADC_ISR=0x%x, ADC_IER=0x%x, ADC_CONF=0x%x\n", adc_reg->ADC_ISR, adc_reg->ADC_IER, adc_reg->ADC_CONF);
      adc_reg->ADC_ISR |= ((1<<8) | (1<<9));
      
      adc_reg->ADC_ISR |= (1<<0);

      //tasklet_schedule(&touch_task);
      my_touch_tasklet(0);
    }

 //   printk("#2 ADC_ISR=0x%x, ADC_IER=0x%x, ADC_CONF=0x%x\n", adc_reg->ADC_ISR, adc_reg->ADC_IER, adc_reg->ADC_CONF);

    
 //   printk("#3 ADC_ISR=0x%x, ADC_IER=0x%x, ADC_CONF=0x%x\n", adc_reg->ADC_ISR, adc_reg->ADC_IER, adc_reg->ADC_CONF);
    
  }

  return IRQ_HANDLED;
}

static void touch2detect(void)
{
  adc_reg->ADC_CONF &= ~((1<<0) | (1<<1));//Touch detection function disable, Press measure function disable

  adc_reg->ADC_IER &= ~((1<<2) | (1<<6));//Pen Down Even Interrupt Disable, Pen Up Event Interrupt Disable

  adc_reg->ADC_CTL |= ((1<<0) | (1<<9)); //Power on ADC,Pen Down Event Enable

  udelay(100);  

  adc_reg->ADC_ISR |= ((1<<2) | (1<<4) | (1<<8) | (1<<9)); // Clear Pen Down/Up Event Flag T_F, Z_F flag
  
//  adc_reg->ADC_IER &= ~(1<<0);//Menu Interrupt Disbale
  adc_reg->ADC_IER |= ((1<<2) | (1<<6));//Pen Down Even Interrupt Enable, Pen Up Event Interrupt Enable

}

static void detect2touch(void)
{
  adc_reg->ADC_CTL &= ~(1<<9); //Pen Down Event Disable
  adc_reg->ADC_IER &= ~((1<<2) | (1<<6));//Pen Down Even Interrupt Disable, Pen Up Event Interrupt Disable

  adc_reg->ADC_CONF |= ((1<<0) | (1<<1) | (7<<3) | (3<<6) | (1<<22) | (0xff<<24));//Touch detection function enable, Press measure function enable

//  adc_reg->ADC_ISR |= ((1<<0) | (1<<8) | (1<<9)); // Clear flag Press Conversion Finish,Touch Conversion Finish,Menu Complete Flag
  adc_reg->ADC_ISR |= (1<<0);//Clear M_F flag

  adc_reg->ADC_IER |= (1<<0);//Menu Interrupt Enable
}

static int touch_init(void)
{  
  INIT_LIST_HEAD(&reg_list);

  tasklet_init(&touch_task, my_touch_tasklet, 0);
  
  io_remap_regs();
  
  CHECK_NULL(adc_reg);
  
  /* Reset ADC hw module */
  *sys_apbiprst1 |= (1<<24);
  *sys_apbiprst1 &= ~(1<<24);

  clk_prepare(clk_get(NULL, "adc"));
  clk_enable(clk_get(NULL, "adc"));

  /* Clear interrupt flags */
  adc_reg->ADC_ISR |= (1<<2);
  adc_reg->ADC_ISR |= (1<<4);
  
  if (request_irq(18, adc_irq_handler, 0, "adc_irq", NULL))
  {
    printk("request_irq for adc_irq failed\n");
  }

  
  touch2detect();

  input_ts_dev = input_allocate_device();
  if (!input_ts_dev) {
     printk("input_allocate_device failed\n");
     return -1;
  }

//  input_ts_dev->name = "NUC970 TouchScreen(ADC)";
//  input_ts_dev->phys = "nuc970ts/event0";
//  input_ts_dev->id.bustype = BUS_HOST;
//  input_ts_dev->id.vendor  = 0x0005;
//  input_ts_dev->id.product = 0x0001;
//  input_ts_dev->id.version = 0x0100;
//  input_ts_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) | BIT_MASK(EV_SYN);
//  
//  input_ts_dev->absbit[BIT_WORD(ABS_X)] |= BIT_MASK(ABS_X);
//  input_ts_dev->absbit[BIT_WORD(ABS_Y)] |= BIT_MASK(ABS_Y);
//  input_ts_dev->absbit[BIT_WORD(ABS_PRESSURE)] |= BIT_MASK(ABS_PRESSURE);
//  input_ts_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

  set_bit(EV_KEY, input_ts_dev->evbit);
  set_bit(EV_ABS, input_ts_dev->evbit);
  set_bit(EV_SYN, input_ts_dev->evbit);
  
  set_bit(ABS_X, input_ts_dev->absbit);
  set_bit(ABS_Y, input_ts_dev->absbit);
  set_bit(ABS_PRESSURE, input_ts_dev->absbit);

  set_bit(BTN_TOUCH, input_ts_dev->keybit);
  
  input_set_abs_params(input_ts_dev, ABS_X, 0, 0x3ff, 0, 0);
  input_set_abs_params(input_ts_dev, ABS_Y, 0, 0x3ff, 0, 0);
  input_set_abs_params(input_ts_dev, ABS_PRESSURE, 0, 255, 0, 0);

  
  setup_timer(&do_xy_convertion_tiemr, trigger_xy_convertion, NULL);

  if(input_register_device(input_ts_dev))
  {
    printk("input_register_device failed\n");
    return -1;
  }
  return 0;
  
err:
  return -1;
}

static void touch_exit(void)
{
  io_unremap_regs();
  clk_prepare(clk_get(NULL, "adc"));
  clk_disable(clk_get(NULL, "adc"));
  free_irq(18, NULL);
  input_unregister_device(input_ts_dev);
}

module_init(touch_init);
module_exit(touch_exit);
MODULE_LICENSE("GPL");
