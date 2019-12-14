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

int fd0;
unsigned char key_val;

void mysignal(int sig)
{
  printf("enter mysignal, sig=%d\n", sig);
  read(fd0, &key_val, 1);    
  printf("key=0x%x\n", key_val);  
}
int main(int argc, char **argv)
{
  int count = 0;
  int ret;

  struct pollfd fds[1];
  int oflags;
  
  signal(SIGIO, mysignal);
  


  fd0 = open("/dev/button0", O_RDWR);
  if (fd0 < 0)
  {
    printf("can't open ed0\n");
    goto exit;
  }

  fds[0].fd = fd0;
  fds[0].events = POLLIN;


  fcntl(fd0, F_SETOWN, getpid());
  oflags = fcntl(fd0, F_GETFL);
  oflags |= FASYNC;
  fcntl(fd0, F_SETFL, oflags);
  
  while(1)
  {
    sleep(1000);
  }

  while(1)
  {
    ret = poll(fds, 1, 5000);
    if (!ret)
      printf("time out\n");
    else
    {
      read(fd0, &key_val, 1);    
      printf("key=0x%x\n", key_val);  
    }
  }

exit:
  return 0;
}
