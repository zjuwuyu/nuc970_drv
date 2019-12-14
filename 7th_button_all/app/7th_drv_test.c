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
    printf("can't open /dev/button0\n");
    goto exit;
  }
  printf("hello\n");  

  while(1)
  {
    read(fd0, &key_val, 1);    
    printf("key=0x%x\n", key_val);  
  }

exit:
  return 0;
}
