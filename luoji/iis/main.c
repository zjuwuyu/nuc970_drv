#include <stdio.h>
#include <string.h>
#include "nuc970.h"
#include "sys.h"
#include "pr.h"
#include "init.h"
#include "i2c.h"
#include "i2s.h"
#include "wav.h"


#define BUF_SIZE 1024
#define MUSIC_NAND_OFFSET 0x200000
#define MUSIC_NAND_SIZE (0x100000*2) 

#define MUSIC_RAM_OFFSET 0x200000
#define MUSIC_RAM_SIZE MUSIC_NAND_SIZE




/*---------------------------------------------------------------------------------------------------------*/
/*  Write 9-bit data to 7-bit address register of NAU8822 with I2C0                                        */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_WriteNAU8822(UINT8 u8addr, UINT16 u16data)
{
    UINT8 TxData[2];
    
retry:    
    TxData[0] = (UINT8)((u8addr << 1) | (u16data >> 8));
    TxData[1] = (UINT8)(u16data & 0x00FF);
    
    i2cIoctl(0, I2C_IOC_SET_SUB_ADDRESS, TxData[0], 0);
    if(i2cWrite(0, &TxData[0], 2) != 2)
        goto retry;
}

void I2C_ReadOneByteNAU8822(UINT8 addr)
{
  UINT8 i;
  UINT8 TxData[1];
  UINT8 RxData[2];
  memset(RxData, 0, sizeof(RxData));

retry1:    
  TxData[0] = (UINT8)(addr << 1);

  i2cIoctl(0, I2C_IOC_SET_SUB_ADDRESS, TxData[0], 0);
  if(i2cWrite(0, &TxData[0], 1) != 1)
  {
    pr_line("i2cWrite failed, TxData[0]=0x%x\n", TxData[0]);
    goto retry1; 
  }
retry2:

  if (i2cRead(0, RxData, sizeof(RxData)) != sizeof(RxData))
  {
    pr_line("i2cRead failed, RxData[0]=0x%x, RxData[1]=0x%x\n", RxData[0],RxData[1]);
    goto retry2; 
  }

  for(i = 0; i < sizeof(RxData); i++)
  {
    pr_info("0x%x ", RxData[i]);
  }
    
}


/*---------------------------------------------------------------------------------------------------------*/
/*  NAU8822 Settings with I2C interface                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
void NAU8822_Setup()
{
    pr_line("Configure NAU8822 ...");

    I2C_WriteNAU8822(0,  0x000);   /* Reset all registers */
    delay(10);

    //input source is MIC
    I2C_WriteNAU8822(1,  0x03F);
    I2C_WriteNAU8822(2,  0x1BF);   /* Enable L/R Headphone, ADC Mix/Boost, ADC */
    I2C_WriteNAU8822(3,  0x07F);   /* Enable L/R main mixer, DAC */
    I2C_WriteNAU8822(4,  0x010);   /* 16-bit word length, I2S format, Stereo */
    I2C_WriteNAU8822(5,  0x000);   /* Companding control and loop back mode (all disable) */   
    I2C_WriteNAU8822(10, 0x008);   /* DAC soft mute is disabled, DAC oversampling rate is 128x */
    I2C_WriteNAU8822(14, 0x108);   /* ADC HP filter is disabled, ADC oversampling rate is 128x */
    I2C_WriteNAU8822(15, 0x1EF);   /* ADC left digital volume control */
    I2C_WriteNAU8822(16, 0x1EF);   /* ADC right digital volume control */

    I2C_WriteNAU8822(44, 0x033);   /* LMICN/LMICP is connected to PGA */
    I2C_WriteNAU8822(50, 0x001);   /* Left DAC connected to LMIX */
    I2C_WriteNAU8822(51, 0x001);   /* Right DAC connected to RMIX */

    pr_line("[OK]");
}

int main(void)
{
  int i = 0;
  NAND_ID_TYPE sNandId;
  unsigned char buf[BUF_SIZE];
  Wav *pWavBuf = NULL;
  memset(buf, 0, sizeof(buf));
  
  outpw(REG_CLK_HCLKEN, 0x0527);
	outpw(REG_CLK_PCLKEN0, 0);
	outpw(REG_CLK_PCLKEN1, 0);
	
  sysDisableCache();
  sysFlushCache(I_D_CACHE);
  sysEnableCache(CACHE_WRITE_BACK);
  
  init();
  
  nand_read_chip_id(&sNandId);
  pr_line("Nand ID:");
  for (i = 0; i < sizeof(sNandId.id)/sizeof(sNandId.id[0]); i++)
  {
    pr_info("0x%x ", sNandId.id[i]);
  }
  pr_line("");

  nand_read(MUSIC_NAND_OFFSET, MUSIC_RAM_OFFSET, MUSIC_NAND_SIZE);

  pWavBuf = (Wav*)MUSIC_RAM_OFFSET;
  
  pr_line("ChunkSiez:%d", pWavBuf->riff.ChunkSize); 
  pr_line("Subchunk1Size:%d", pWavBuf->fmt.Subchunk1Size);
  pr_line("ChunBitsPerSamplekSiez:%d", pWavBuf->fmt.BitsPerSample);
  pr_line("NumChannels:%d", pWavBuf->fmt.NumChannels);
  pr_line("SampleRate:%d", pWavBuf->fmt.SampleRate);

  pr_line("Subchunk2Size:%d", pWavBuf->data.Subchunk2Size);

  audio_init1();
 
  /* Configure multi function pins to I2C0 */
  outpw(REG_SYS_GPG_MFPL, inpw(REG_SYS_GPG_MFPL)| 0x88);
	
  // Initialize I2C-0 interface
  i2cInit(0);
  if(i2cOpen(0) != 0)
  {
    pr_info("i2cOpen failed\n");
    return 0;
  }

  // Set slave address is 0x1a
  i2cIoctl(0, I2C_IOC_SET_DEV_ADDRESS, 0x1A, 0);
  // I2C interface speed is 100KHz
  i2cIoctl(0, I2C_IOC_SET_SPEED, 100, 0);

//  I2C_ReadOneByteNAU8822(0x3e);
//  I2C_ReadOneByteNAU8822(0x3f);
  NAU8822_Setup();
  
  I2C_WriteNAU8822(11, 0x1B1);   /* ADC left digital volume control */
  I2C_WriteNAU8822(12, 0x1B1);   /* ADC right digital volume control */

//  i2sIoctl(I2S_SET_RECORD, I2S_START_REC, 0);
  i2sIoctl(I2S_SET_PLAY, I2S_START_PLAY, 0);
  while(1);

exit:
err:
  while(1)
  {
    led_on();
    delay(100);
    led_off();
    delay(100);
  }
}
