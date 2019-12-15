#ifndef __ZWY_CHECK__
#define __ZWY_CHECK__

#define CHECK_NULL(p,msg)\
  if (NULL==p)\
  {\
    printk("%s\n", msg);\
    goto exit;\
  }
  
#define CHECK_RET(ret,msg)\
  if (0!=ret)\
  {\
    printk("%s\n", msg);\
    goto exit;\
  }

#endif
