#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
  int fd0;
  unsigned char key_vals[4];
  int count = 0;

  fd0 = open("/dev/key0", O_RDWR);
  if (fd0 < 0)
  {
    printf("can't open ed0\n");
    goto exit;
  }

  while(1)
  {
    read(fd0, key_vals, sizeof(key_vals));
    if(/*!key_vals[0] || !key_vals[1] || */!key_vals[2] || !key_vals[3])
    {
      printf("%04d key pressed: %d %d %d %d\n", count++, key_vals[0], key_vals[1], key_vals[2], key_vals[3]);
    }
  }


exit:
  return 0;
}
