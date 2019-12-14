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
#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-lcd.h>
#include <mach/mfp.h>
#include <mach/gpio.h>
#include "10th_drv.h"

volatile unsigned int *clk_hclk            = NULL;
volatile unsigned int *clk_divctl1         = NULL;
volatile unsigned int *clk_apllcon         = NULL;
volatile unsigned int *clk_upllcon         = NULL;
volatile unsigned int *gpiog_dir           = NULL; 
volatile unsigned int *gpiog_output        = NULL;
volatile unsigned int *lcd_dccs            = NULL;
volatile unsigned int *lcd_device_ctrl     = NULL;
volatile unsigned int *lcd_crtc_size       = NULL;
volatile unsigned int *lcd_crtc_hr         = NULL;
volatile unsigned int *lcd_crtc_vr         = NULL;
volatile unsigned int *lcd_crtc_dend       = NULL;
volatile unsigned int *lcd_crtc_hsync      = NULL;
volatile unsigned int *lcd_va_baddr0       = NULL;
volatile unsigned int *lcd_va_fbctrl       = NULL;
volatile unsigned int *lcd_va_scale        = NULL;


static struct mfp_reg *mfp_reg             = NULL;
static struct fb_info *nuc972_fbinfo          = NULL;

static struct list_head reg_list;

unsigned int *lcd_virt_base                = NULL;
dma_addr_t    map_dma;
u32 pseudo_palette[16];

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
  clk_apllcon     = register_reg_map_item("clk_apllcon", CLK_BA+0x60, 4);
  clk_upllcon     = register_reg_map_item("clk_upllcon", CLK_BA+0x64, 4);
  clk_divctl1     = register_reg_map_item("clk_divctl1", CLK_BA+0x24, 4);
  gpiog_dir       = register_reg_map_item("gpiog_dir",   GPIO_BA+0x180, 4);
  gpiog_output    = register_reg_map_item("gpiog_output",GPIO_BA+0x184, 4);  
  lcd_dccs        = register_reg_map_item("lcd_dccs",    LCM_BA+0x0, 4);  
  lcd_device_ctrl = register_reg_map_item("lcd_device_ctrl",    LCM_BA+0x4, 4); 
  lcd_crtc_size   = register_reg_map_item("lcd_crtc_size",    LCM_BA+0x10, 4); 
  lcd_crtc_hr     = register_reg_map_item("lcd_crtc_hr",    LCM_BA+0x18, 4);
  lcd_crtc_vr     = register_reg_map_item("lcd_crtc_vr",    LCM_BA+0x20, 4);
  lcd_crtc_dend   = register_reg_map_item("lcd_crtc_dend",    LCM_BA+0x14, 4);
  lcd_crtc_hsync  = register_reg_map_item("lcd_crtc_hsync",    LCM_BA+0x1c, 4);
  lcd_va_baddr0   = register_reg_map_item("lcd_va_baddr0",    LCM_BA+0x24, 4);
  lcd_va_fbctrl   = register_reg_map_item("lcd_va_fbctrl",    LCM_BA+0x2c, 4);
  lcd_va_scale    = register_reg_map_item("lcd_va_scale",    LCM_BA+0x30, 4);

  mfp_reg         = (struct mfp_reg*)register_reg_map_item("mfp_reg", SYS_BA+0x70, sizeof(struct mfp_reg));

  CHECK_NULL(clk_apllcon);
  CHECK_NULL(clk_upllcon);
  CHECK_NULL(clk_divctl1);
  CHECK_NULL(gpiog_dir);
  CHECK_NULL(gpiog_output);
  CHECK_NULL(lcd_dccs);
  CHECK_NULL(lcd_device_ctrl);
  CHECK_NULL(lcd_crtc_size);
  CHECK_NULL(lcd_crtc_hr);
  CHECK_NULL(lcd_crtc_vr);
  CHECK_NULL(lcd_crtc_dend);
  CHECK_NULL(lcd_crtc_hsync);
  CHECK_NULL(lcd_va_baddr0);
  CHECK_NULL(lcd_va_fbctrl);
  CHECK_NULL(lcd_va_scale);
  CHECK_NULL(mfp_reg);

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

static inline unsigned int chan_to_field(unsigned int chan,
					 struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int nuc970fb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	unsigned int val;
//	printk(KERN_ALERT"wuyu %s is called\n", __func__);

	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		/* true-colour, use pseuo-palette */
		if (regno < 16) {
			u32 *pal = info->pseudo_palette;

			val  = chan_to_field(red, &info->var.red);
			val |= chan_to_field(green, &info->var.green);
			val |= chan_to_field(blue, &info->var.blue);
			pal[regno] = val;
		}
		break;

	default:
		return 1;   /* unknown type */
	}
	return 0;
}


static struct fb_ops nuc972fb_ops = {
	.owner			= THIS_MODULE,
	.fb_setcolreg		= nuc970fb_setcolreg,
	.fb_fillrect		= cfb_fillrect,
	.fb_copyarea		= cfb_copyarea,
	.fb_imageblit		= cfb_imageblit,
};

static int lcd_init(void)
{
  int i     = 0;
  int j     = 0;
  int vtt   = 70+272+36;
  int htt   = 125+480+115;
  int vdend = 272;
  int hdend = 480;
//  int hrs   = 480+115;
//  int hre   = 480+115+64;
  int hrs   = 480+1;
  int hre   = 480+5;
  int vrs   = 272+36;
  int vre   = 272+36+6;
  int hsyncs = 480+115;
  int hsynce = 480+115+64;
  unsigned short *p = NULL;
  
  INIT_LIST_HEAD(&reg_list);

  nuc972_fbinfo = framebuffer_alloc(0, NULL);
  CHECK_NULL(nuc972_fbinfo);
  
  strcpy(nuc972_fbinfo->fix.id, "mylcd");
  //nuc972_fbinfo->fix.smem_start=;
  nuc972_fbinfo->fix.smem_len = 480*272*2;
  nuc972_fbinfo->fix.type = FB_TYPE_PACKED_PIXELS;
  nuc972_fbinfo->fix.visual = FB_VISUAL_TRUECOLOR;
  nuc972_fbinfo->fix.line_length = 480*2;

  nuc972_fbinfo->var.xres           = 480;
  nuc972_fbinfo->var.yres           = 272;
  nuc972_fbinfo->var.xres_virtual   = 480;
  nuc972_fbinfo->var.yres_virtual   = 272;
  nuc972_fbinfo->var.bits_per_pixel = 16;
  nuc972_fbinfo->var.red.offset     = 11;
  nuc972_fbinfo->var.red.length     = 5;
  nuc972_fbinfo->var.green.offset   = 5;
  nuc972_fbinfo->var.green.length   = 6;  
  nuc972_fbinfo->var.blue.offset    = 0;
  nuc972_fbinfo->var.blue.length    = 5;    
  nuc972_fbinfo->var.activate       = FB_ACTIVATE_NOW;
  
  nuc972_fbinfo->fbops              = &nuc972fb_ops;
  //nuc972_fbinfo->pseudo_palette   =;
  //nuc972_fbinfo->screen_base      =;
  nuc972_fbinfo->screen_size        =480*272*2;

  nuc972_fbinfo->pseudo_palette = pseudo_palette;

 
  if (io_remap_regs() < 0)
    printk("io_remap_regs failed\n");

  /*GPA0~15->LCD_DATA0~15*/
  mfp_reg->SYS_GPA_MFPL = 0x22222222;
  mfp_reg->SYS_GPA_MFPH = 0x22222222;

  /*GPD8~15->LCD_DATA16~23*/
  mfp_reg->SYS_GPD_MFPH = 0x22222222;

  /*GPG6->LCD_CLK, GPG7->LCD_HSYNC*/
  mfp_reg->SYS_GPG_MFPL &= ~(0xff<<24);
  mfp_reg->SYS_GPG_MFPL |= (0x22<<24);
  
  /*GPG8->LCD_VSYNC, GPG9->LCD_DEN*/
  mfp_reg->SYS_GPG_MFPH &= ~(0xff<<0);
  mfp_reg->SYS_GPG_MFPH |= (0x22<<0);

  /*GPG3->LCD_BKLIGHT, normal GPIO*/
  mfp_reg->SYS_GPG_MFPL &= ~(0xf<<12);

	clk_prepare(clk_get(NULL, "lcd_hclk"));
	clk_enable(clk_get(NULL, "lcd_hclk"));

//  clk = clk_get(NULL, "lcd_eclk");
//  CHECK_NULL(clk);
//  printk("lcd_eclk = %ld\n", clk_get_rate(clk));
//
//  clk_set_parent(clk_get(NULL, "lcd_eclk_mux"), clk_get(NULL, "lcd_uplldiv"));
//  clk_set_rate( clk_get(NULL, "lcd_eclk"), 20000000);
//  printk("lcd_eclk = %ld\n", clk_get_rate(clk_get(NULL, "lcd_eclk")));

  *gpiog_dir |= (1<<3); //Config GPG3 as output
  *gpiog_output |= (1<<3); //Config GPG3 as output high

  
  *lcd_dccs &= ~(1<<1); //Video Disable
  *lcd_dccs &= ~(1<<3); //Display output Disable


  /* LCD Src from UPLL, UPLL = 300M */
  *clk_divctl1 |= (3<<3);

  /* LCD_SDIV=0, 300/(0+1)=300M */
  *clk_divctl1 &= ~(7<<0);

  /* LCD pixel clock = 300/(1+14)=20MHz */
  *clk_divctl1 &= ~(0xff<<8);
  *clk_divctl1 |= (0xe<<8);

  *lcd_device_ctrl = 0;
  *lcd_device_ctrl &= ~(7<<5);
  *lcd_device_ctrl |= (6<<5);//set device as Sync-based High-color TFT-LCD (RGB565/RGB666/RGB888)
  *lcd_device_ctrl |= (0x3<<24);//RGB color type is 16777216 colors mode
  *lcd_device_ctrl |= (0x1<<26);//Data bus is 16/18-bit
  
//  *lcd_device_ctrl |= (0x1<<23);//Display Data Output Mode Interlace
//  *lcd_device_ctrl |= (0x1<<22);//Sync (Horizontal And Vertical Sync) Interlace
//  *lcd_device_ctrl |= (0x1<<21);//V_POL (Vertical Polarity) High active
//  *lcd_device_ctrl |= (0x1<<20);//H_POL (Horizontal Polarity) High active

  *lcd_crtc_size       = 0;
  *lcd_crtc_hr         = 0;
  *lcd_crtc_vr         = 0;
  *lcd_crtc_dend       = 0;
  *lcd_crtc_hsync      = 0;

  *lcd_crtc_size    |= ((htt<<0) | (vtt<<16));
  *lcd_crtc_dend    |= ((hdend<<0) | (vdend<<16));
  *lcd_crtc_hr      |= ((hrs<<0) | (hre<<16));
  *lcd_crtc_vr      |= ((vrs<<0) | (vre<<16));
  *lcd_crtc_hsync   |= ((hsyncs<<0) | (hsynce<<16));

  lcd_virt_base = (unsigned int*)dma_alloc_writecombine(NULL, 480*272*2+32, &map_dma, GFP_KERNEL); 
  nuc972_fbinfo->screen_base    = lcd_virt_base;
  nuc972_fbinfo->fix.smem_start = (unsigned long)map_dma;

  *lcd_va_baddr0 = (unsigned int)(map_dma);

  *lcd_va_fbctrl = 0xf000f0;//RGB 565
  //*lcd_va_fbctrl = 0x1e001e0;//RGB888
  *lcd_va_fbctrl &= ~(1<<31); //Dual buffer disabled
  *lcd_va_fbctrl &= ~(1<<30); //Starting fetch data from VA_BADDR0

  *lcd_va_scale = 0x04000400;

  *lcd_dccs = 0; //reset display engine
  for (i = 0; i < 10000; i++);
  *lcd_dccs |= (1<<0); //reset display engine
  for (i = 0; i < 10000; i++);
  *lcd_dccs &= ~(1<<0); //reset display engine

  *lcd_dccs &= ~(0x7<<8); 
  *lcd_dccs |= (0x4<<8); //RGB565 format
  //*lcd_dccs |= (0x2<<8); //RGB888 format

  *lcd_dccs |= (1<<1); //Video Enable
  *lcd_dccs |= (1<<3); //Display output enable, normal mode

  register_framebuffer(nuc972_fbinfo);

  p = (unsigned short*)lcd_virt_base;
  for(j = 0; j < 480*272/2; j++)
  {
      p[j] = (0x1f<<11);
  }

  return 0;
err:
  return -1;
}

static void lcd_exit(void)
{
  *gpiog_output &= ~(1<<3); //Config GPG3 as output low, turn off back light
  io_unremap_regs();
  dma_free_writecombine(NULL, 480*272*2, lcd_virt_base, map_dma);
  unregister_framebuffer(nuc972_fbinfo);
}

module_init(lcd_init);
module_exit(lcd_exit);
MODULE_LICENSE("GPL");
