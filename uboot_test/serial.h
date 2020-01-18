#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "nuc970.h"

#define MAX_SERIAL_BUF_LEN 80

typedef enum MSG_TYPE
{
   MSG_TYPE_UINT32,
   MSG_TYPE_INT32,
   MSG_TYPE_HEX,
   MSG_TYPE_MAX
}MSG_TYPE_t;

void sysPutMsg(INT8 *msg, MSG_TYPE_t type, UINT32 num);
void sysPutHexBuf(UINT32 *data, UINT32 data_len);
void sysPutHexCharBuf(UINT8 *data, UINT32 data_len);

#endif

