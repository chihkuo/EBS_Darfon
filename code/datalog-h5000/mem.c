#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "mem.h"
#include "modbusdrv.h"
//#include "16-Bit Flash Programmer.h"

extern int DarfonNewProtocolWritePMBlock(unsigned int Address, char *pBuffer , int SlaveID,eFamily Family);
extern int WriteCommBlockModBus( unsigned int pStartAddress,int functioncode, char *pBuffer , int BytesToWrite, int SlaveID);
int CheckResponse1(unsigned long waittime_s);
extern void lySleep(unsigned long us);


mem_cMemRow pMemory[PM_SIZE+EE_SIZE+CM_SIZE];

char	        m_ConfigurationBuffer[CM_SIZE*3];//mike0116* char	m_ConfigurationBuffer[24];
int            m_ConfigurationBufferIndex;

extern int  oncenum;
extern int  UserCodeSlaveID_For_Bootloader;
extern unsigned int  BootloaderVersion;
extern eFamily  Family;





#define MODBUS_DEBUG				2
#if(MODBUS_DEBUG>0)
#define DEBUG1(x)	x
#else
#define DEBUG1(x)
#endif
#if(MODBUS_DEBUG>1)
#define DEBUG2(x)	x
#else
#define DEBUG2(x)
#endif

void Init_mem_cMemRow(int MemRowindex, eType Type, unsigned int StartAddr, int Row, eFamily Family)
{
    int Size;

    pMemory[MemRowindex].m_RowNumber = Row;
    pMemory[MemRowindex].m_eFamily    = Family;
    pMemory[MemRowindex].m_eType     = Type;
    pMemory[MemRowindex].m_bEmpty    = 1;//mike*m_bEmpty    = TRUE;


    if(pMemory[MemRowindex].m_eType == Program)
    {
        if(pMemory[MemRowindex].m_eFamily == dsPIC30F)
        {
            pMemory[MemRowindex].m_RowSize = PM30F_ROW_SIZE;
        }
        else
        {
            pMemory[MemRowindex].m_RowSize = PM33F_ROW_SIZE;
        }
    }
    else
    {
        pMemory[MemRowindex].m_RowSize = EE30F_ROW_SIZE;
    }

    if(pMemory[MemRowindex].m_eType == Program)
    {
        Size = pMemory[MemRowindex].m_RowSize * 3;
        pMemory[MemRowindex].m_Address = StartAddr + Row * pMemory[MemRowindex].m_RowSize * 2;
    }
    if(pMemory[Row].m_eType == EEProm)
    {
        Size = pMemory[MemRowindex].m_RowSize * 2;
        pMemory[MemRowindex].m_Address = StartAddr + Row * pMemory[MemRowindex].m_RowSize * 2;
    }
    if(pMemory[Row].m_eType == Configuration)
    {
        Size = 3;
        pMemory[MemRowindex].m_Address = StartAddr + Row * 2;
    }

    pMemory[MemRowindex].m_pBuffer   = (char *)malloc(Size);
    memset(pMemory[MemRowindex].m_Data, 0xFFFF, sizeof(unsigned short)*PM33F_ROW_SIZE*2);
}

int InsertData(int MemRowindex, unsigned int Address, char * pData)
{
    if(Address < pMemory[MemRowindex].m_Address)
    {
        return 0;
    }

    if((pMemory[MemRowindex].m_eType == Program) && (Address >= (pMemory[MemRowindex].m_Address + pMemory[MemRowindex].m_RowSize * 2)))
    {
        return 0;
    }

    if((pMemory[MemRowindex].m_eType == EEProm) && (Address >= (pMemory[MemRowindex].m_Address + pMemory[MemRowindex].m_RowSize * 2)))
    {
        return 0;
    }

    if((pMemory[MemRowindex].m_eType == Configuration) && (Address >= (pMemory[MemRowindex].m_Address + 2)))
    {
        return 0;
    }

    pMemory[MemRowindex].m_bEmpty    = 0;

    sscanf(pData, "%4hx", &(pMemory[MemRowindex].m_Data[Address - pMemory[MemRowindex].m_Address]));
    return 1;
}


