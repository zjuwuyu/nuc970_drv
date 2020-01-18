#ifndef __NAND_H__
#define __NAND_H__

#include "nuc970.h"

#define PAGE_SIZE 2048
#define MAX_NAND_BUF_LEN 2048


typedef struct
{
  UINT8 id[5];
} NAND_ID_TYPE;


extern void nand_initialize(void);
extern void nand_read_chip_id(NAND_ID_TYPE *pID);
extern void nand_read(UINT32 uSrcAddr, UINT32 uDstAddr, UINT32 size);
extern UINT8 nand_write_addr(UINT32 uDstNandAddr, UINT32 uSrcBufAddr, UINT32 size);
extern UINT8 nand_erase_block(UINT32 uAddr);
extern BOOL nand_is_bad_block_oob(UINT32 uBlockIndex);
extern BOOL nand_is_bad_block_content(UINT32 uBlockIndex);

#endif
