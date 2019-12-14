#include <linux/major.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/bitops.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include <asm/setup.h>
#include <asm/pgtable.h>
#include <linux/hdreg.h>


#define  RAMBLOCK_SIZE (1024*1024)

struct gendisk *ramblock_gendisk = NULL;
struct request_queue *ramblock_queue = NULL;
int major = 0;
unsigned char *ramblock_buf = NULL;

DEFINE_SPINLOCK(ramblock_lock);

static int ramblock_getgeo(struct block_device *dev, struct hd_geometry *geo)
{
  geo->heads = 2;
  geo->cylinders = 32;
  geo->sectors = RAMBLOCK_SIZE/geo->heads/geo->cylinders/512;
  return 0;
}

static struct block_device_operations ramblock_operatios =
{
  .owner = THIS_MODULE,
  .getgeo = ramblock_getgeo,
};

static void ramblock_request(struct request_queue *q)
{
  static int r_cnt = 0;
  static int w_cnt = 0;
  
  struct request *req = blk_fetch_request(q);

  printk("enter ramblock_request\n");
  if (NULL == req)
  {    
    printk("enter req is NULL\n");
  }

	while (NULL != req) {
	  unsigned long start = blk_rq_pos(req) << 9;
		unsigned long len  = blk_rq_cur_bytes(req);

		if (rq_data_dir(req) == READ)
		{
		  memcpy(req->buffer, (char *)(ramblock_buf+start), len);
		  printk("r_cnt=%d\n", ++r_cnt);
		}
  	else
  	{
		  memcpy((char *)(ramblock_buf+start), req->buffer, len);
		  printk("w_cnt=%d\n", ++w_cnt);
		}
		
		if (!__blk_end_request_cur(req, 0))
		{
		  req = blk_fetch_request(q);
		}
//		else
//		{
//		  break;
//	  }
//    __blk_end_request_cur(req, 0);
//    req = blk_fetch_request(q);

	}
}

static int ramblock_init(void)
{
  major = register_blkdev(0, "ramblock");
  
  ramblock_gendisk = alloc_disk(16);
  if (NULL == ramblock_gendisk) {
    printk("alloc_disk failed\n");
    return -1;
  }

  ramblock_queue = blk_init_queue(ramblock_request, &ramblock_lock);
  if (NULL == ramblock_queue) {
    printk("blk_init_queue failed\n");
    return -1;
  }  
  
  ramblock_buf = kzalloc(RAMBLOCK_SIZE, GFP_KERNEL);
  if (NULL == ramblock_buf)
  {
    printk("kzalloc failed\n");
    return -1;
  }
  ramblock_gendisk->major = major;
  ramblock_gendisk->first_minor = 0;
  sprintf(ramblock_gendisk->disk_name, "ramblock");
  ramblock_gendisk->fops = &ramblock_operatios;
  ramblock_gendisk->queue = ramblock_queue;

  set_capacity(ramblock_gendisk, RAMBLOCK_SIZE/512);

  add_disk(ramblock_gendisk);
  
  return 0;
}

static void ramblock_exit(void)
{ 
  unregister_blkdev(major, "ramblock");
  del_gendisk(ramblock_gendisk);
  put_disk(ramblock_gendisk);
  blk_cleanup_queue(ramblock_queue);
  kfree(ramblock_buf);
}

module_init(ramblock_init);
module_exit(ramblock_exit);
MODULE_LICENSE("GPL");


