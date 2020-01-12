#ifndef __PR_H__
#define __PR_H__

//#define pr_info(fmt,...) printf("(file: %s, func: %s, line: %d)"fmt"\r\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define pr_line(fmt,...) printf(fmt"\r\n", ##__VA_ARGS__)
#define pr_info(fmt,...) printf(fmt, ##__VA_ARGS__)
#define pr_debug(fmt,...) printf("(file:%s, func:%s, line:%d)"fmt"\r\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#endif

