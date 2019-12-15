#include <linux/module.h>
#include <linux/printk.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/gfp.h>
#include <linux/io.h>
#include <asm/irq.h>
#include <linux/atomic.h>
#include "zwy_check.h"

static struct net_device *vnet = NULL;

static netdev_tx_t vnet_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
  static int cnt = 0;
  printk("vnet_start_xmit cnt:%d\n", ++cnt);
  return NETDEV_TX_OK;
}

static struct net_device_ops vnet_dev_ops = 
{
  .ndo_start_xmit = vnet_start_xmit,
};

static void vnet_setup(struct net_device *net_device)
{
  printk("enter vnet_setup\n");
}

static int vnet_init(void)
{
  int ret = 0;
  vnet = alloc_netdev(0, "vnet0", vnet_setup);

  vnet->netdev_ops = &vnet_dev_ops;
  CHECK_NULL(vnet, "alloc_netdev failed\n");

  ret = register_netdev(vnet);
  CHECK_RET(ret, "register_netdev failed\n");
  
exit:
  return ret;
}

static void vnet_exit(void)
{
  if (vnet)
    unregister_netdev(vnet);
  if (vnet)
    free_netdev(vnet);
}

module_init(vnet_init);
module_exit(vnet_exit);

MODULE_LICENSE("GPL");

