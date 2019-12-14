#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
  int fd0, fd1;
  int val = 1;

  fd0 = open("/dev/led0", O_RDWR);
  if (fd0 < 0)
  {
    printf("can't open ed0\n");
    goto exit;
  }
  fd1 = open("/dev/led1", O_RDWR);
  if (fd1 < 0)
  {
    printf("can't open led1\n");
    goto exit;
  }

  if (argc != 3)
  {
    printf("Usage :\n");
    printf("%s <on|off> <4|5>\n", argv[0]);
    goto exit;
  }

  if (!strcmp(argv[1], "on"))
  {
    val = 1;
  }
  else
  {
    val = 0;
  }

  if (!strcmp(argv[2], "4"))
  {
    write(fd0, &val, 4);
  }
  else
  {
    write(fd1, &val, 4);
  }

exit:
  return 0;
}
