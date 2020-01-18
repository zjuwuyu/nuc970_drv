#include "setup.h"

static struct tag *params = (struct tag *)0x100;

static void setup_start_tag(void)
{
	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next (params);
}

static void setup_end_tag()
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}

UINT32 strlen(char *p)
{
  int i = 0;
  while(*p != '\0')
  {
    p++;
    i++;
  }
  return i;
}

void strcpy(char *dst, char *src)
{
  while(*src != '\0')
  {
    *dst = *src;
    src++;
    dst++;
  }
  *dst = '\0';
}
static void setup_commandline_tag(char *commandline)
{
	char *p;

	if (!commandline)
		return;

	/* eat leading white space */
	for (p = commandline; *p == ' '; p++);

	/* skip non-existent command lines so the kernel will still
	 * use its default command line.
	 */
	if (*p == '\0')
		return;

	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size =
		(sizeof (struct tag_header) + strlen (p) + 1 + 4) >> 2;

	strcpy (params->u.cmdline.cmdline, p);

	params = tag_next (params);
}

static void setup_memory_tags(void)
{
	params->hdr.tag = ATAG_MEM;
	params->hdr.size = tag_size (tag_mem32);

	params->u.mem.start = 0;
	params->u.mem.size = 64*0x10000;

	params = tag_next (params);
}

static void setup_initrd_tag(u32 initrd_start, u32 initrd_end)
{
	params->hdr.tag = ATAG_INITRD2;
	params->hdr.size = tag_size (tag_initrd);

	params->u.initrd.start = initrd_start;
	params->u.initrd.size = initrd_end - initrd_start;

	params = tag_next (params);
}


void setup_tags(void)
{
  sysPutString("before setup_start_tag\r\n");
  setup_start_tag();
  
  sysPutString("before setup_commandline_tag\r\n");
  setup_commandline_tag("noinitrd root=/dev/mtdblock2 rootfstype=yaffs2 rootflags=inband-tags console=ttyS0,115200n8 rdinit=/sbin/init mem=64M");
  
  sysPutString("before setup_memory_tags\r\n");
  setup_memory_tags();

  sysPutString("before setup_initrd_tag\r\n");
  setup_initrd_tag(0, 0);
  
  sysPutString("before setup_end_tag\r\n");
  setup_end_tag();
  
  sysPutString("end setup_tags\r\n");
}

