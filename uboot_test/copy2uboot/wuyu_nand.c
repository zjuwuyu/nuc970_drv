#include "nand.h"
#include "serial.h"


UINT8 gNandBuf[MAX_NAND_BUF_LEN] = {0};

/* Nand select for CS0 */
void nand_select(BOOL bSelect)
{
  if (TRUE == bSelect)
  {
    outpw(REG_NANDCTL, (inpw(REG_NANDCTL)&(~(1<<25))));
  }
  else
  {      
    outpw(REG_NANDCTL, (inpw(REG_NANDCTL)|(1<<25)));
  }
}
void nand_initialize(void)
{
  UINT32 reg = 0;
  UINT32 i = 0;
  NAND_ID_TYPE sNandId;
  UINT8 test_buf[160];
  UINT8 uStatus;

  /* initial NAND controller, enabler FMI clock and nand clock */
  reg = inpw(REG_CLK_HCLKEN);

  reg |= ((1<<20) | (1<<21));
  outpw(REG_CLK_HCLKEN, reg);

  sysPutMsg("REG_SYS_PWRON:", MSG_TYPE_HEX, inpw(REG_SYS_PWRON));
  

  /* select NAND function pins */
  if (inpw(REG_SYS_PWRON) & 0x08000000)
  {
      /* Set GPI1~15 for NAND */
      outpw(REG_SYS_GPI_MFPL, 0x55555550);
      outpw(REG_SYS_GPI_MFPH, 0x55555555);
  }
  else
  {
      /* Set GPC0~14 for NAND */
      outpw(REG_SYS_GPC_MFPL, 0x55555555);
      outpw(REG_SYS_GPC_MFPH, 0x05555555);
  }

  /* Enabled Nand */
  reg = inpw(REG_FMI_CTL);
  reg |= (1<<3);
  outpw(REG_FMI_CTL, reg);

  /* Config nand timing */
  outpw(REG_NANDTMCTL, 0x20305);
  
  /* Disale CS0, Disable CS1 */
  outpw(REG_NANDCTL, inpw(REG_NANDCTL)| (3<<25));

  nand_select(TRUE);
  
  /* Nand flash is writable now */
  outpw(REG_NANDECTL, 0x1); /* un-lock write protect */

  // NAND Reset
  outpw(REG_NANDCTL, inpw(REG_NANDCTL) | 0x1);    // software reset
  while (inpw(REG_NANDCTL) & 0x1);

  /* reset nand chip command */
  outpw(REG_NANDCMD, 0xff);
  for (i = 0; i < 10; i++);
  
  while (!(inpw(REG_NANDINTSTS) & (1<<18)));
  nand_read_chip_id(&sNandId);

//  uStatus = nand_erase_block(0x100000);
//  uStatus = nand_write_addr(0x100000, test_buf, 16);

//  nand_select(FALSE);
}

inline void wait_chip_ready(void)
{
  while (!(inpw(REG_NANDINTSTS) & (1<<18)));
}
void nand_read_chip_id(NAND_ID_TYPE *pID)
{
  if (pID)
  {
    outpw(REG_NANDCMD, 0x90);
    outpw(REG_NANDADDR, 0x0 | ( 1<<31));

    wait_chip_ready();

    pID->id[0] = inpw(REG_NANDDATA);
    pID->id[1] = inpw(REG_NANDDATA); 
    pID->id[2] = inpw(REG_NANDDATA);
    pID->id[3] = inpw(REG_NANDDATA);
    pID->id[4] = inpw(REG_NANDDATA);

    sysPutMsg("id[0]:", MSG_TYPE_HEX, pID->id[0]);  
    sysPutMsg("id[1]:", MSG_TYPE_HEX, pID->id[1]);  
    sysPutMsg("id[2]:", MSG_TYPE_HEX, pID->id[2]);  
    sysPutMsg("id[3]:", MSG_TYPE_HEX, pID->id[3]);  
    sysPutMsg("id[4]:", MSG_TYPE_HEX, pID->id[4]);  
  }
}

void nand_prepare_read(UINT32 uAddr)
{
  UINT32 uRowAddr = uAddr / 2048;  
  UINT32 uColAddr = uAddr % 2048;

  outpw(REG_NANDCMD, 0x0);

  outpw(REG_NANDADDR, uColAddr & 0xff);  
  outpw(REG_NANDADDR, (uColAddr >> 8) & 0xf); //second cycle MSB 4 bits should be 0 per datasheet
  outpw(REG_NANDADDR, uRowAddr & 0xff);  
  outpw(REG_NANDADDR, ((uRowAddr >> 8) & 0xff)|(1<<31));  // last address should set BIT31 to high 

  outpw(REG_NANDCMD, 0x30);

  wait_chip_ready();
  
}

void nand_read_bytes(UINT8 **buf, UINT32 size)
{
  UINT32 i;
  UINT8 *p = *buf;
  
  for (i = 0; i < size; i++)
  {
    p[i] = inpw(REG_NANDDATA);
  }

  *buf = *buf + size;
}

void wuyu_nand_read(UINT32 uSrcNandAddr, UINT32 uDstBufAddr, UINT32 size)
{
  UINT32 i = 0;
  UINT8 *pDst = (UINT8*)uDstBufAddr;

  UINT32 uColAddr = uSrcNandAddr % PAGE_SIZE;

  if ((uColAddr + size) <= PAGE_SIZE)
  {
    nand_prepare_read(uSrcNandAddr);
    nand_read_bytes(&pDst, size);
  }
  else
  {
    /* read the first less PAGE_SIZE part */
    nand_prepare_read(uSrcNandAddr);
    nand_read_bytes(&pDst, PAGE_SIZE - uColAddr);
    uSrcNandAddr += (PAGE_SIZE - uColAddr);
    size -= (PAGE_SIZE - uColAddr);

    /* read the first less remaining part */
    while (size > PAGE_SIZE)
    {
      nand_prepare_read(uSrcNandAddr);
      nand_read_bytes(&pDst, PAGE_SIZE);
      uSrcNandAddr += PAGE_SIZE;
      size -= PAGE_SIZE;
    }

    if (size > 0)
    {
      nand_prepare_read(uSrcNandAddr);
      nand_read_bytes(&pDst, size);
      uSrcNandAddr += size;
      size -= size;
    }
  }
}


UINT8 nand_write_addr(UINT32 uDstNandAddr, UINT32 uSrcBufAddr, UINT32 size)
{
  UINT32 i         = 0;
  UINT8  uStatus   = 0;
  UINT32 uRowAddr  = uDstNandAddr / 2048;  
  UINT32 uColAddr  = uDstNandAddr % 2048;
  UINT8 *pSrcBuf       = (UINT8*)uSrcBufAddr;

  outpw(REG_NANDCMD, 0x80);

  outpw(REG_NANDADDR, uColAddr & 0xff);  
  outpw(REG_NANDADDR, (uColAddr >> 8) & 0xf); //second cycle MSB 4 bits should be 0 per datasheet
  outpw(REG_NANDADDR, uRowAddr & 0xff);  
  outpw(REG_NANDADDR, ((uRowAddr >> 8) & 0xff)|(1<<31));  // last address should set BIT31 to high 

  for (i = 0; i < size; i++)
  {    
    outpw(REG_NANDDATA, pSrcBuf[i]);
  }
  
  outpw(REG_NANDCMD, 0x10);

  wait_chip_ready();
  

  outpw(REG_NANDCMD, 0x70);
  
  uStatus = inpw(REG_NANDDATA);
  return uStatus;
}

UINT8 nand_erase_block(UINT32 uAddr)
{
  UINT32 i         = 0;
  UINT8  uStatus   = 0;
  UINT32 uRowAddr  = uAddr / 2048;  
  UINT32 uColAddr  = uAddr % 2048;

  outpw(REG_NANDCMD, 0x60);

  outpw(REG_NANDADDR, uRowAddr & 0xff);  
  outpw(REG_NANDADDR, ((uRowAddr >> 8) & 0xff)|(1<<31));  // last address should set BIT31 to high 

  outpw(REG_NANDCMD, 0xD0);

  wait_chip_ready();
  
  outpw(REG_NANDCMD, 0x70);
  
  uStatus = inpw(REG_NANDDATA);
  return uStatus;
}

BOOL nand_is_bad_block_oob(UINT32 uBlockIndex)
{
  UINT32 i          = 0;
  UINT32 uPageIndex = 0;
  BOOL   bRet       = FALSE;
  UINT32 uRowAddr   = uBlockIndex * 64 + 0;
  UINT32 uColAddr    = PAGE_SIZE;

  outpw(REG_NANDCMD, 0x0);

  outpw(REG_NANDADDR, uColAddr & 0xff);  
  outpw(REG_NANDADDR, (uColAddr >> 8) & 0xf); //second cycle MSB 4 bits should be 0 per datasheet
  outpw(REG_NANDADDR, uRowAddr & 0xff);  
  outpw(REG_NANDADDR, ((uRowAddr >> 8) & 0xff)|(1<<31));  // last address should set BIT31 to high 

  outpw(REG_NANDCMD, 0x30);

  wait_chip_ready();


  for (i = 0; i < 64; i++)
  {
    gNandBuf[i] = inpw(REG_NANDDATA);
  }
  
 sysPutHexCharBuf(gNandBuf, 64);

//    if (0xff != inpw(REG_NANDDATA))
//    {
//      bRet = TRUE;
//      return bRet;
//    }
  

  return bRet;
}

BOOL nand_is_bad_block_content(UINT32 uBlockIndex)
{
  UINT32 i          = 0;
  UINT32 uPageIndex = 0;
  
  for (uPageIndex = 0; uPageIndex < 64; uPageIndex++)
  {
    UINT32 uRowAddr = uBlockIndex * 64 + uPageIndex;  
    UINT32 uColAddr = 0;

    outpw(REG_NANDCMD, 0x0);

    outpw(REG_NANDADDR, uColAddr & 0xff);  
    outpw(REG_NANDADDR, (uColAddr >> 8) & 0xf); //second cycle MSB 4 bits should be 0 per datasheet
    outpw(REG_NANDADDR, uRowAddr & 0xff);  
    outpw(REG_NANDADDR, ((uRowAddr >> 8) & 0xff)|(1<<31));  // last address should set BIT31 to high 

    outpw(REG_NANDCMD, 0x30);

    wait_chip_ready();

    for (i = 0; i < PAGE_SIZE; i++)
    {
      gNandBuf[i] = inpw(REG_NANDDATA);
    }

    for (i = 0; i < PAGE_SIZE; i++)
    {      
      if (0xff != gNandBuf[i])
      {
        return TRUE;
      }
    }
    
  }

  return FALSE;
}


