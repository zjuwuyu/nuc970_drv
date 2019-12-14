#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

dma_addr_t dma_addr;
signed char *data =  NULL;
struct urb *uk_urb = NULL;


static void uk_irq(struct urb *urb)
{
  int i = 0;
  for (i = 0; i < 8; i++)
  {
    printk("%x ", data[i]);
  }
  printk("\n");
  usb_submit_urb (urb, GFP_ATOMIC);
}

static int uk_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
  struct usb_device *dev = interface_to_usbdev(intf);
  struct usb_host_interface *interface = intf->cur_altsetting;  
  struct usb_endpoint_descriptor *endpoint = &interface->endpoint[0].desc;
  int pipe= usb_rcvintpipe(dev, endpoint->bEndpointAddress);
  
  
  data = usb_alloc_coherent(dev, 8, GFP_KERNEL, &dma_addr);
  uk_urb = usb_alloc_urb(0, GFP_KERNEL);
  
	usb_fill_int_urb(uk_urb, dev, pipe, data, endpoint->wMaxPacketSize, uk_irq, NULL, endpoint->bInterval);
	uk_urb->transfer_dma = dma_addr;
	uk_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
  usb_submit_urb (uk_urb, GFP_ATOMIC);
  
  printk("%s\n", __func__);

  printk("bscUSB = %x\n", dev->descriptor.bcdUSB);  
  printk("VID = %x\n", dev->descriptor.idVendor);
  printk("PID = %x\n", dev->descriptor.idProduct);
  
  return 0;
}

static void uk_disconnect(struct usb_interface *intf)
{  
  printk("%s\n", __func__);
}

static struct usb_device_id uk_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	{ }	/* Terminating entry */
};


static struct usb_driver uk_driver = {
	.name		= "usbmouse",
	.probe		= uk_probe,
	.disconnect	= uk_disconnect,
	.id_table	= uk_id_table,
};


static int uk_init(void)
{
  usb_register(&uk_driver);
  return 0;
}

static void uk_exit(void)
{ 
  usb_deregister(&uk_driver);
}

module_init(uk_init);
module_exit(uk_exit);
MODULE_LICENSE("GPL");


