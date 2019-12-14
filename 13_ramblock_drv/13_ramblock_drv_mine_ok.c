#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>

#define my_ramblock_disk_SIZE (1024*1024)

struct gendisk *my_ramblock_disk = NULL;
struct request_queue *my_ramblock_disk_queue = NULL;
static DEFINE_SPINLOCK(my_ramblock_disk_lock);
int major;
unsigned char *my_ramblock_disk_buf = NULL;

static int my_ramblock_disk_getgeo(struct block_device *dev, struct hd_geometry *g)
{
  g->heads = 2;
  g->cylinders = 32;
  g->sectors = my_ramblock_disk_SIZE/g->heads/g->cylinders/512;
  return 0;
}

static void do_my_ramblock_disk_request(struct request_queue *q)
{
  struct request *req = blk_fetch_request(q);
  static int r_cnt = 0;
  static int w_cnt = 0;

  while(req)
  {
    unsigned long offset = 512*blk_rq_pos(req);
    unsigned long len   = blk_rq_cur_bytes(req);

    if (READ == rq_data_dir(req))
    {
      printk("r_cnt=%d\n", r_cnt++);
      memcpy(req->buffer, my_ramblock_disk_buf+offset, len);
    }
    else
    {
      printk("w_cnt=%d\n", w_cnt++);
      memcpy(my_ramblock_disk_buf+offset, req->buffer, len);
    }

    if (!__blk_end_request_cur(req, 0))
			req = blk_fetch_request(q);
			
  }

}




struct block_device_operations my_ramblock_disk_fops=
{
  .owner = THIS_MODULE,
  .getgeo = my_ramblock_disk_getgeo,
};


static int my_ramblock_init(void)
{
  my_ramblock_disk = alloc_disk(10);
  
  major = register_blkdev(0, "my_ramblock_disk");

  my_ramblock_disk_queue = blk_init_queue(do_my_ramblock_disk_request, &my_ramblock_disk_lock);
  
  my_ramblock_disk->major = major;
  
  my_ramblock_disk->first_minor = 0;
  my_ramblock_disk->fops = &my_ramblock_disk_fops;
  sprintf(my_ramblock_disk->disk_name, "my_ramblock_disk");
  my_ramblock_disk->queue = my_ramblock_disk_queue;
  

  set_capacity(my_ramblock_disk, my_ramblock_disk_SIZE/512);

  my_ramblock_disk_buf =  kzalloc(my_ramblock_disk_SIZE, GFP_KERNEL);
  
  add_disk(my_ramblock_disk);
  
  return 0;
}

static void my_ramblock_exit(void)
{
  unregister_blkdev(major, "my_ramblock_disk");
  del_gendisk(my_ramblock_disk);
  put_disk(my_ramblock_disk);
  blk_cleanup_queue(my_ramblock_disk_queue);
  
  kfree(my_ramblock_disk_buf);
}

module_init(my_ramblock_init);
module_exit(my_ramblock_exit);
MODULE_LICENSE("GPL");


  





