#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/blkdev.h>
#include <linux/delay.h>

  
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-gcr.h>
#include <mach/regs-timer.h>
#include <mach/regs-fmi.h>
#include <linux/dma-mapping.h>


#define CHECK_NULL(_ptr_) \
  if (NULL == _ptr_) {\
    printk("null pointer at line: %d", __LINE__);\
    goto exit;}
    
typedef struct nuc970_fmi_reg_info
{
  volatile unsigned long FMI_CTL;
  volatile unsigned long FMI_INTEN; 
  volatile unsigned long FMI_INTSTS; 
}nuc970_fmi_reg_info_t;

typedef struct nuc970_nand_reg_info
{
  volatile unsigned long FMI_NANDCTL;
  volatile unsigned long FMI_NANDTMCTL;
  volatile unsigned long FMI_NANDINTEN; 
  volatile unsigned long FMI_NANDINTSTS;
  volatile unsigned long FMI_NANDCMD;
  volatile unsigned long FMI_NANDADDR;
  volatile unsigned long FMI_NANDDATA;
  volatile unsigned long FMI_NANDRACTL;
  volatile unsigned long FMI_NANDECTL;
}nuc970_nand_reg_info_t;


struct clk *fmi_clk                      = NULL;
struct clk *nand_clk                     = NULL;
static struct nand_chip *nuc970_nand     = NULL;
static struct mtd_info *nuc970_mtd       = NULL;
static nuc970_fmi_reg_info_t *fmi_regs   = NULL;
static nuc970_nand_reg_info_t *nand_regs = NULL;


static void nuc970_select_chip(struct mtd_info *mtd, int chipnr)
{
  if (-1 == chipnr)
  {
    //dis-select chip
    nand_regs->FMI_NANDCTL |= (1<<25);
  }
  else
  {
    //select chip    
    nand_regs->FMI_NANDCTL &= ~(1<<25);
  }
}

static void nuc970_cmd_ctrl(struct mtd_info *mtd, int data, unsigned int ctrl)
{
  if (ctrl & NAND_CLE)
  {
    //send command
    nand_regs->FMI_NANDCMD = data;
  }
  else
  {
    //send address
    nand_regs->FMI_NANDADDR= data;
  }
}
static int nuc970_dev_ready(struct mtd_info *mtd)
{ 
  return (nand_regs->FMI_NANDINTSTS & (1<<18)) > 0; 
}

static struct mtd_partition	nuc970_mtd_parts[] = {
	{
		.name =	"u-boot",
		.offset	= 0,
		.size =	2 *	1024 * 1024,
	},
	{
		.name =	"Kernel",
		.size =	20 * 1024 *	1024,
		.offset	= MTDPART_OFS_APPEND,
	},
	{
		.name =	"user",
		.offset	= MTDPART_OFS_APPEND,
		.size =	MTDPART_SIZ_FULL
	}
};

static int nand_init()
{ 
  fmi_clk  = clk_get(NULL, "fmi_hclk");
  nand_clk = clk_get(NULL, "nand_hclk");
  CHECK_NULL(fmi_clk);
  CHECK_NULL(nand_clk);
  clk_enable(fmi_clk);
  clk_enable(nand_clk);
  nuc970_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
  CHECK_NULL(nuc970_nand);
  nuc970_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
  CHECK_NULL(nuc970_mtd);

  fmi_regs  = ioremap(0xB000D800, 16);
  nand_regs = ioremap(0xB000D8A0, 48);

  nuc970_nand->IO_ADDR_R   = &nand_regs->FMI_NANDDATA;//DATA virtual address;
  nuc970_nand->IO_ADDR_W   = &nand_regs->FMI_NANDDATA;//DATA virtual address;
  nuc970_nand->select_chip = nuc970_select_chip;
  nuc970_nand->cmd_ctrl    = nuc970_cmd_ctrl;
  nuc970_nand->dev_ready   = nuc970_dev_ready;
  nuc970_nand->ecc.mode    = NAND_ECC_SOFT;
  
  nuc970_mtd->owner = THIS_MODULE;
  nuc970_mtd->priv = nuc970_nand;

  /* Enable nand */
  fmi_regs->FMI_CTL |= (1<<3);

  /* Initialize timing control register */
  nand_regs->FMI_NANDTMCTL = 0x20305;

  /* Disbale nand write-protect and nand is writable */
  nand_regs->FMI_NANDECTL  |= (1<<0);

  /* Do reset */
  fmi_regs->FMI_CTL |= (1<<0);
  udelay(10);

  if(nand_scan(nuc970_mtd, 1))
  {
    printk("nand_scan failed\n");
    goto exit;
  }

//  add_mtd_device(nuc970_mtd);
//  add_mtd_partitions(nuc970_mtd, nuc970_mtd_parts, 4);
  
  mtd_add_partition(nuc970_mtd, "u-boot", 0, 2*1024*1024);
  mtd_add_partition(nuc970_mtd, "Kernel", 2*1024*1024, 20*1024*1024);
  mtd_add_partition(nuc970_mtd, "root", 22*1024*1024, 80*1024*1024);

exit:
  return 0;
}

static void nand_exit(void)
{ 
  /* Disable nand */
//  fmi_regs->FMI_CTL &= ~(1<<3);
//
//  fmi_clk = clk_get(NULL, "fmi_hclk");
//  nand_clk = clk_get(NULL, "nand_hclk");
//  clk_disable(nand_clk);
//  clk_disable(fmi_clk);

  if (fmi_regs)
    iounmap(fmi_regs);

  if (nand_regs)
    iounmap(nand_regs);
    
  if (nuc970_nand)
    kfree(nuc970_nand);

  if (nuc970_mtd)
    kfree(nuc970_mtd);

}


module_init(nand_init);
module_exit(nand_exit);
MODULE_LICENSE("GPL");
