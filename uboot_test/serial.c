#include "nuc970.h"
#include "serial.h"

INT8 buf[MAX_SERIAL_BUF_LEN] = {0};



void sysPutChar(UINT8 ucCh)
{
    volatile int loop;
    while ((inpw(REG_UART0_FSR) & (1<<23))); //waits for TX_FULL bit is clear
    outpw(REG_UART0_THR, ucCh);
}

void sysPutString(INT8 *string)
{
    while (*string != '\0') {
        sysPutChar(*string);
        string++;
    }
}

void sysPutStringLine(INT8 *string)
{
  sysPutString(string);
  sysPutString("\r\n");
}




void sysPutHex(UINT32 num)
{
    int   i = 0;
    UINT8 val = 0;
    UINT8 c;
    
    for (i = 0; i < 8; i++)
    {
        val = (num >> (28 - i * 4)) & 0xf;
        
        if (val < 10)
        {
          c = val + '0';
        }
        else if (val < 16)
        {
          c = 'a' + (val - 10);
        }

        buf[i] = c;
    }
    buf[i] = 0;
    
    sysPutString("0x");
    sysPutString(buf);
}

void sysPutCharHex(UINT8 num)
{
  int   i = 0;
  UINT8 val = 0;
  UINT8 c;
  
  for (i = 0; i < 2; i++)
  {
      val = (num >> (4 - i * 4)) & 0xf;
      
      if (val < 10)
      {
        c = val + '0';
      }
      else if (val < 16)
      {
        c = 'a' + (val - 10);
      }
  
      buf[i] = c;
  }
  buf[i] = 0;
  
  sysPutString("0x");
  sysPutString(buf);

}


void sysPutHexCharBuf(UINT8 *data, UINT32 data_len)
{
  UINT32 i = 0;
  if (NULL == data)
  {
    //error
  }
  else
  {    
    for (i = 0; i < data_len; i++)
    {
      if (i%16 == 0)
      {
        sysPutString("\r\n");
        sysPutHex(i);
        sysPutString(":  ");
      }
      sysPutCharHex(data[i]);
      sysPutString(" ");
    }
    sysPutString("\r\n");
  }
}

void sysPutUint32(UINT32 num)
{
    int   i = 0;
    int   j, k;
    UINT8 val = 0;
    UINT8 c;

    while ( num / 10  > 0)
    {   
      buf[i++] = num % 10 + '0';
      num /= 10;
    }

    buf[i++] = num + '0';
    buf[i] = 0;
 
    i--;

    /* Reverse printed it out will show the right number */
    while (i >= 0) {
        sysPutChar(buf[i]);
        i--;
    }
}

void sysPutInt32(INT32 num)
{
    int   i = 0;
    int   j, k;
    UINT8 val = 0;
    UINT8 c;
    BOOL bIsNegtive = FALSE;

    if (num < 0)
    {
      bIsNegtive = TRUE;
      num = -num;
    }

    while ( num / 10  > 0)
    {   
      buf[i++] = num % 10 + '0';
      num /= 10;
    }

    buf[i++] = num + '0';
    
    if (bIsNegtive)
    {
      buf[i++] = '-';
    }
    
    buf[i] = 0;
 
    i--;

    /* Reverse printed it out will show the right number */
    while (i >= 0) {
        sysPutChar(buf[i]);
        i--;
    }
}



void sysPutMsg(INT8 *msg, MSG_TYPE_t type, UINT32 num)
{
    if (msg)
    {
        sysPutString(msg);

        switch (type)
        {
          case MSG_TYPE_UINT32:
            sysPutUint32(num); 
            break;
          case MSG_TYPE_INT32:
            sysPutInt32(num); 
            break;
          case MSG_TYPE_HEX:
            sysPutHex(num); 
            break;
        }
        sysPutString("\r\n");
    }
}

void init_serial(void)
{
  int i = 0;
  
  /* enable UART0 clock */
  outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | (1<<16));

  /* GPE0, GPE1 */
  outpw(REG_SYS_GPE_MFPL, (inpw(REG_SYS_GPE_MFPL) & 0xffffff00) | 0x99);  // UART0 multi-function

  /* UART0 line configuration for (115200,n,8,1) */
  outpw(REG_UART0_LCR, inpw(REG_UART0_LCR) | 0x3);
  outpw(REG_UART0_BAUD, 0x30000066); /* 12MHz reference clock input, 115200 */    
  
  sysPutMsg("MSG_TYPE_UINT32:", MSG_TYPE_UINT32, 12345);
  sysPutMsg("MSG_TYPE_INT32:", MSG_TYPE_INT32, -7);  
  sysPutMsg("MSG_TYPE_INT32:", MSG_TYPE_HEX, -7); 
  sysPutMsg("MSG_TYPE_HEX:", MSG_TYPE_HEX, 0xfde1234);  

}




