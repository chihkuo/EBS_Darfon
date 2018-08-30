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

void FormatData(int MemRowindex)
{
    int Count;
    if(pMemory[MemRowindex].m_bEmpty == 1)
    {
        return;
    }
    int a=0;//mike+
    if(pMemory[MemRowindex].m_eType == Program)
    {
        for(Count = 0; Count < pMemory[MemRowindex].m_RowSize; Count += 1)
        {
        pMemory[MemRowindex].m_pBuffer[0 + Count * 3] = (pMemory[MemRowindex].m_Data[Count * 2]     >> 8) & 0xFF;
        pMemory[MemRowindex].m_pBuffer[1 + Count * 3] = (pMemory[MemRowindex].m_Data[Count * 2])          & 0xFF;
        pMemory[MemRowindex].m_pBuffer[2 + Count * 3] = (pMemory[MemRowindex].m_Data[Count * 2 + 1] >> 8) & 0xFF;
        }
    }
    else if(pMemory[MemRowindex].m_eType == Configuration)
    {
        pMemory[MemRowindex].m_pBuffer[0] = (pMemory[MemRowindex].m_Data[0]  >> 8) & 0xFF;
        pMemory[MemRowindex].m_pBuffer[1] = (pMemory[MemRowindex].m_Data[0])       & 0xFF;
        pMemory[MemRowindex].m_pBuffer[2] = (pMemory[MemRowindex].m_Data[1]  >> 8) & 0xFF;
    }
    else
    {
        for(Count = 0; Count < pMemory[MemRowindex].m_RowSize; Count++)
        {
        pMemory[MemRowindex].m_pBuffer[0 + Count * 2] = (pMemory[MemRowindex].m_Data[Count * 2] >> 8) & 0xFF;
        pMemory[MemRowindex].m_pBuffer[1 + Count * 2] = (pMemory[MemRowindex].m_Data[Count * 2])      & 0xFF;
        }
    }
}

 int SendData(int MemRowindex)
{
    char Buffer[4] = {0,0,0,0};
    char CheckACKBuffer[9] = {0,0,0,0,0,0,0,0,0};
    char ACK=0x06;

    if((pMemory[MemRowindex].m_bEmpty == 1) && (pMemory[MemRowindex].m_eType != Configuration))
    {
        return 1;
    }

    pMemory[MemRowindex].IfCheckAck=1;
    int errorcount=0;
    clock_t start,end;
    start=clock();
    int errorlimit=5;
	if(BootloaderVersion>0)
		errorlimit=3;

    while(CheckACKBuffer[6] != ACK && pMemory[MemRowindex].IfCheckAck==1)
    {
        pMemory[MemRowindex].IfCheckAck=1;
        if(pMemory[MemRowindex].m_eType == Program)
        {
        	DEBUG1(printf("Write PM 0x%04X\n",pMemory[MemRowindex].m_Address));
        	if(BootloaderVersion==0)
			{
	            Buffer[0] = COMMAND_WRITE_PM;
	            Buffer[1] = (pMemory[MemRowindex].m_Address)       & 0xFF;
	            Buffer[2] = (pMemory[MemRowindex].m_Address >> 8)  & 0xFF;
	            Buffer[3] = (pMemory[MemRowindex].m_Address >> 16) & 0xFF;

	            if( pMemory[MemRowindex].m_Address  == ((unsigned int)0x400))
	            {
	                printf("\n[Error]:Should Not Writing Bootloader, Please Check Hex File\n");
	                return 0;
	            }


	            WriteCommBlockModBus( pMemory[MemRowindex].m_Address, COMMAND_WRITE_PM, pMemory[MemRowindex].m_pBuffer, pMemory[MemRowindex].m_RowSize * 3, UserCodeSlaveID_For_Bootloader);

        	}
			else
			{
				//Family=dsPIC33F;
				if(DarfonNewProtocolWritePMBlock(pMemory[MemRowindex].m_Address, pMemory[MemRowindex].m_pBuffer , UserCodeSlaveID_For_Bootloader, dsPIC33F))
				{
					DEBUG1(printf("\nGot ACK\n"));
					return 0;
				}
			}

			printf("Write PM 0x%04X\n",pMemory[MemRowindex].m_Address);
        }
        else if(pMemory[MemRowindex].m_eType == EEProm)
        {
            Buffer[0] = COMMAND_WRITE_EE;
            Buffer[1] = (pMemory[MemRowindex].m_Address)       & 0xFF;
            Buffer[2] = (pMemory[MemRowindex].m_Address >> 8)  & 0xFF;
            Buffer[3] = (pMemory[MemRowindex].m_Address >> 16) & 0xFF;
        }
        else if((pMemory[MemRowindex].m_eType == Configuration) && (pMemory[MemRowindex].m_RowNumber == 0))
        {
            Buffer[0] = COMMAND_WRITE_CM;
            Buffer[1] = (char)(pMemory[MemRowindex].m_bEmpty)& 0xFF;
            Buffer[2] = pMemory[MemRowindex].m_pBuffer[0];
            Buffer[3] = pMemory[MemRowindex].m_pBuffer[1];

            m_ConfigurationBufferIndex=0;
            m_ConfigurationBuffer[m_ConfigurationBufferIndex+0] = (char)(pMemory[MemRowindex].m_bEmpty)& 0xFF;
            m_ConfigurationBuffer[m_ConfigurationBufferIndex+1] = pMemory[MemRowindex].m_pBuffer[0];
            m_ConfigurationBuffer[m_ConfigurationBufferIndex+2] = pMemory[MemRowindex].m_pBuffer[1];
            m_ConfigurationBufferIndex+=3;
            pMemory[MemRowindex].IfCheckAck=0;
        }
        else if((pMemory[MemRowindex].m_eType == Configuration) && (pMemory[MemRowindex].m_RowNumber != 0))
        {
            if((pMemory[MemRowindex].m_eFamily == dsPIC30F) && (pMemory[MemRowindex].m_RowNumber == 7))
            {
                return 0;
            }

            Buffer[0] = (char)(pMemory[MemRowindex].m_bEmpty)& 0xFF;
            Buffer[1] = pMemory[MemRowindex].m_pBuffer[0];
            Buffer[2] = pMemory[MemRowindex].m_pBuffer[1];

            m_ConfigurationBuffer[m_ConfigurationBufferIndex+0] = (char)(pMemory[MemRowindex].m_bEmpty)& 0xFF;
            m_ConfigurationBuffer[m_ConfigurationBufferIndex+1] = pMemory[MemRowindex].m_pBuffer[0];
            m_ConfigurationBuffer[m_ConfigurationBufferIndex+2] = pMemory[MemRowindex].m_pBuffer[1];
            if(m_ConfigurationBufferIndex<=21)
                m_ConfigurationBufferIndex+=3;
            pMemory[MemRowindex].IfCheckAck=0;
        }
        else
        {
            printf("\n[Error]:Unknown memory type");
            return 0;
        }




        if(pMemory[MemRowindex].IfCheckAck)
        {
            CheckACKBuffer[0] = 1;
            if(CheckResponse1(2)==0)
            {
                printf("\nNo ACK\n");
                if(errorcount>=errorlimit)
                {
                    printf("\n[Error]:SendData Fail[SendData]\n");
                    return 0;
                }
                CheckACKBuffer[6] =0x00;
                errorcount++;
				if(BootloaderVersion==0)
				{
	                if(oncenum>20)
	                    oncenum-=5;
				}

                MClearRX();
            }
            else
            {
                memset(CheckACKBuffer, 0, 9);
                memcpy(CheckACKBuffer, mrPacket.mData, 9);
                DEBUG1(printf("\nGot ACK\n"));
                errorcount=0;
            }
        }
    }
    end=clock();
    DEBUG1(printf("\nwrite row %d cost time=%d\n",MemRowindex,((end-start)/1000000)));
    return 1;
}

int CheckResponse1(unsigned long waittime_s)
{

    unsigned long i=0;
    unsigned long totalus=waittime_s*1000000;
    unsigned long gap=10000;//10ms 0.01s
    while( TRC_RECEIVED==0 && i<=(totalus/gap))
    {
        if(TRC_RXERROR==1)
            break;
        lySleep(gap);//0.01s
        i++;
    }
    float time;
    time=(0.01)*i;
    if(TRC_RXERROR==1)
        DEBUG2(printf("CheckResponse Error %.2f s\n",time));
    if(TRC_RECEIVED==0)
        DEBUG2(printf("CheckResponse No Response %.2f s\n",time));
    else
        DEBUG2(printf("CheckResponse success,cost  %.2f s\n",time));

    return TRC_RECEIVED;
}


