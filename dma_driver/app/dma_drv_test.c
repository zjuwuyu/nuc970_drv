#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "../dma_drv.h"



int main(int argc, char **argv)
{
  int fd;
  int ret = 0;

  if (argc < 2)
  {
    printf("argc failed\n");
    return -1;
  }
  
  fd = open("/dev/nuc970_dma", O_RDWR);
  if (!fd)
  {
    printf("device open failed\n");
    return -1;
  }
  
  if (!strcmp(argv[1], "nodma"))
  {
    while(1)
    {
      ret = ioctl(fd, IOCTL_CMD_NO_DMA, 0);
    }
  }

  if (!strcmp(argv[1], "dma"))
  {
    while(1)
    {
      ret = ioctl(fd, IOCTL_CMD_WITH_DMA, 0);
    }
  }

  return 0;  
}
