/*
**********************************************************
*Product       MODBUS drivers
*Version       1.0
*Date          10:50, Oct 14, 2010
**********************************************************
*Function:
	1. MODBUS drivers

*Adjust:

*/
/*****************************************************************************/
/* include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "iodef.h"
#include "modbusdrv.h"
//mike-#include "main.h"
#include "16-Bit Flash Programmer.h"
#include "linux/serial.h"
#include "DarfonloggerGlobe.h"
#include "gdefine.h"


//#define SUPPORT_ZIGBEE



extern void writeLog(char *pLog);

/*****************************************************************************/
/* global definitions */
#define MODBUS_DEBUG				3
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

#if(MODBUS_DEBUG>2)
#define DEBUG3(x)	x
#else
#define DEBUG3(x)
#endif




/* CRC lookup table */
const unsigned char CRCtableHi[256]=
{
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40
};
const unsigned char CRCtableLo[256]=
{
	0x00,0xC0,0xC1,0x01,0xC3,0x03,0x02,0xC2,0xC6,0x06,
	0x07,0xC7,0x05,0xC5,0xC4,0x04,0xCC,0x0C,0x0D,0xCD,
	0x0F,0xCF,0xCE,0x0E,0x0A,0xCA,0xCB,0x0B,0xC9,0x09,
	0x08,0xC8,0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,
	0x1E,0xDE,0xDF,0x1F,0xDD,0x1D,0x1C,0xDC,0x14,0xD4,
	0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,
	0x11,0xD1,0xD0,0x10,0xF0,0x30,0x31,0xF1,0x33,0xF3,
	0xF2,0x32,0x36,0xF6,0xF7,0x37,0xF5,0x35,0x34,0xF4,
	0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,
	0x3B,0xFB,0x39,0xF9,0xF8,0x38,0x28,0xE8,0xE9,0x29,
	0xEB,0x2B,0x2A,0xEA,0xEE,0x2E,0x2F,0xEF,0x2D,0xED,
	0xEC,0x2C,0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,
	0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,0xA0,0x60,
	0x61,0xA1,0x63,0xA3,0xA2,0x62,0x66,0xA6,0xA7,0x67,
	0xA5,0x65,0x64,0xA4,0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,
	0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,
	0x78,0xB8,0xB9,0x79,0xBB,0x7B,0x7A,0xBA,0xBE,0x7E,
	0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,0xB4,0x74,0x75,0xB5,
	0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,
	0x70,0xB0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
	0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9C,0x5C,
	0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,
	0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4B,0x8B,
	0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
	0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
	0x43,0x83,0x41,0x81,0x80,0x40
};

/*****************************************************************************/
/* Global variable */
pthread_t tidModbusDriver;
int fdModbus=-1;
int modbusDriverStatus=0;
unsigned int txsize;
unsigned char txbuffer[1544];//MODBUS_TX_BUFFER_SIZE
int TRC_RECEIVED, TRC_TXIDLE, TRC_RXERROR, ZIGBEECMD_RECEIVED,ZIGBEECMD_RESPONSE_COUNT;
bool have_respond = false;
unsigned char respond_buff[4096];

/* for inverter/sensor */
enum
{
	URS_Idle=0,
	URS_Address,
	URS_FunctionCode,
	URS_DataLength,
	URS_Data,
	URS_CRC,
	URS_End
};
unsigned int receiveState;

/* MODBUS protocol */
MODBUS_REPLY_PACKET mrPacket;
unsigned int  mrDcnt, mrCnt, mrLen;
unsigned char waitAddr, waitFCode;
extern int inBootloaderMode;





extern struct s_InverterTable  CurrentUsedglobesetting;

/*****************************************************************************/
/* function definition */

/*****************************************************************************/

/*
 Function: lySleep
 Description: sleep for specified us
 Parameters: delay time in us
 Returns: none
*/
void lySleep(unsigned long us)
{
 unsigned long i, j, k;

 if(us>=10000)
    usleep(us);
 else
 {
  k=us/10;
  k=(k)?k:1;
  for(i=0;i<k;i++)
   for(j=0;j<526;j++);
 }
}

/* function codes */
/*
	Function: CalculateCRC
	Description: calculate CRC
	Parameters: data pointer and data length
	Returns: CRC
*/

unsigned short CalculateCRC(unsigned char *p, unsigned int len)
{
    unsigned short crc;
    unsigned char index;
    unsigned char m, l;
    crc=0xffff;
    while(len--)
    {
        m=(unsigned char)((crc>>8)&0x00ff);
        l=crc&0x00ff;
        index=m ^ *p++;
        m=l ^ CRCtableHi[index];
        l=CRCtableLo[index];
        crc=(((unsigned short)m)<<8)+((unsigned short)l);
    }
    return(crc);
}
#ifdef SUPPORT_ZIGBEE

/*
	Function: ModbusDriver
	Description: MODBUS driver, send/receive data between buffer and physical device
	Parameters: none
	Returns: none
*/






void *HandleZigbeeSystemCMD()
{
    unsigned char buf;
    int ret, fn,i;
    unsigned short crc;
    byte ID[8];
    DEBUG3(printf("HandleZigbeeSystemCMD\n"));


    ret=read(fdModbus, &buf, 1);
    if(ret<=0)
        return;
    fn=buf;
    mrPacket.mData[2]=buf;
    //DEBUG3(printf("fn= %02X\n", mrPacket.mData[2]));
    switch(fn)
    {
        case DFAPP_SYSTEM_FN_GET_MI_NUM:
            for ( i = 3; i <= 15; i++)
            {
                usleep(5000);
                if(read(fdModbus, &(mrPacket.mData[i]), 1)>0)
                {
                    DEBUG3(printf("HandleZigbeeSystemCMD r= %02X\n", mrPacket.mData[i]));
                }
                else
                    break;
            }
            crc=CalculateCRC(&(mrPacket.mData[0]), 16-2);
            mrPacket.crc=((unsigned short)(mrPacket.mData[14]))<<8;
			mrPacket.crc|=(unsigned short)mrPacket.mData[15];

            if(crc==mrPacket.crc)
			{
                if (mrPacket.mData[3] == mrPacket.mData[4])
                {
                    CurrentUsedglobesetting.MINumZigbee = mrPacket.mData[3];
                    DEBUG3(printf("DFAPP_SYSTEM_FN_GET_MI_NUM num= %d\n", CurrentUsedglobesetting.MINumZigbee ));
                    ZIGBEECMD_RECEIVED=1;
                    CurrentUsedglobesetting.CommunicationBoxType=Zigbee;
                }
                else
                {
                    CurrentUsedglobesetting.MINumZigbee = 0;
                    TRC_RXERROR=1;
                }

			}
			else
			{
				CurrentUsedglobesetting.MINumZigbee = 0;
				TRC_RXERROR=1;
			}


		break;
		case DFAPP_SYSTEM_FN_READ_ALL_MI_SN_TABLE:

			for ( i = 3; i <= 15; i++)
            {
                usleep(2000);
                if(read(fdModbus, &(mrPacket.mData[i]), 1)>0)
                {
                    //DEBUG3(printf("HandleZigbeeSystemCMD r= %02X\n", mrPacket.mData[i]));
                    if (i < (16 - 4) && i>3)
                    {
                        ID[i - 4] = mrPacket.mData[i];
                        //DEBUG3(printf("DFAPP_SYSTEM_FN_READ_ALL_MI_SN_TABLE ID= %02X\n", ID[i - 4]));

                    }
                }
                else
                    break;
            }



            crc=CalculateCRC(&(mrPacket.mData[0]), 16-2);
            mrPacket.crc=(unsigned short)(mrPacket.mData[14])<<8;
			mrPacket.crc|=(unsigned short)mrPacket.mData[15];

            if(crc==mrPacket.crc)
			{
			///
			    int i2;
			    DEBUG3(printf("ID="));

        		for(i2=0;i2<8;i2++)
        			DEBUG3(printf("%02x",ID[i2]));
        		DEBUG3(printf("\n"));
            ///
                DEBUG3(printf("ZIGBEECMD_RESPONSE_COUNT=0x%02x\n",ZIGBEECMD_RESPONSE_COUNT));
                AddMItoInveterlistID(ID);
                //AddMItoInveterlistID();
                ZIGBEECMD_RESPONSE_COUNT++;
                if(CurrentUsedglobesetting.MINumZigbee==ZIGBEECMD_RESPONSE_COUNT)
                {
                    WriteAllMITableToROM();
                    ZIGBEECMD_RECEIVED=1;
                    CurrentUsedglobesetting.CommunicationBoxType=Zigbee;
                }
			}
            else
            {
                DEBUG3(printf("DFAPP_SYSTEM_FN_READ_ALL_MI_SN_TABLE checksum fail\n"));
                //sleep(5);
            }

		break;


    }


}

void *ModbusDriver(void *arg)
{
	unsigned char buf;
	int ret, end, buflen;
	unsigned short crc;
	end=0;
	DEBUG2(printf("MODBUS(Zigbee) driver start\n"));
	modbusDriverStatus=1;
	while(modbusDriverStatus)
	{
		/* rx control */
		if((TRC_RECEIVED==0)&&TRC_TXIDLE)
		{   //printf("rx\n");
			ret=read(fdModbus, &buf, 1);
			if(ret>0)
			{
				/* data read from MODBUS */
				//DEBUG3(printf("R: %02X\n", buf));
                            //DEBUG1(printf("R: %02X,%02X,%02X,%d\n", buf,waitAddr,waitFCode,receiveState));
				/* LED control */
				switch(receiveState)
				{
					case URS_Idle:
						/* skip anything */
                        //DEBUG3(printf("URS_Idle R: %02X\n", buf));
					break;
					case URS_Address:
Chk_URI_Address:
                        //DEBUG3(printf("URS_Address R: %02X\n", buf));
						if(buf==waitAddr)
						{
                            //DEBUG3(printf("buf==waitAddr %d\n",buf));
							mrPacket.mData[0]=buf;
							mrCnt=1;
							receiveState++;
						}
                        else if(buf==0xff)
                        {
                            mrPacket.mData[0]=buf;
                            receiveState++;
                        }
                                            //DEBUG2(printf("wrong waitAddr %d\n",buf));
					break;
					case URS_FunctionCode:
                        //DEBUG3(printf("URS_FunctionCode R: %02X, mrPacket.mData[0]=%02X\n",buf, mrPacket.mData[0]));
                        if(buf==0xfe && mrPacket.mData[0]==0xff)
                        {
                            //DEBUG3(printf("buf==0xfe && mrPacket.mData[0]==0xff\n"));
                            mrPacket.mData[1]=buf;
                            HandleZigbeeSystemCMD();
                            receiveState=URS_Address;
                            break;
                        }

						if(waitFCode==buf)
						{
						 	//DEBUG2(printf("buf==waitFCode %d\n",buf));
							mrPacket.mData[mrCnt]=buf;
							mrCnt++;
							/* check function code */
							if((buf==0xff)||(buf==0x10))
							{
								/* reply in fixed length for function code 0xff, 0x10 */
								mrLen=4;
								mrDcnt=0;
								receiveState=URS_Data;
							}
							//mike+{
							else if(buf==0x05)
							{
								mrLen=4;
								mrDcnt=0;
								receiveState=URS_Data;
							}
                            else if(buf==0x09)
							{
								mrLen=12;
								mrDcnt=0;
								receiveState=URS_Data;
							}
                            else if(buf==0x02)
							{
								mrLen=1540;
								mrDcnt=0;
								receiveState=URS_Data;
							}
                            else if(buf==0x03 && inBootloaderMode==1)
							{
								mrLen=5;
								mrDcnt=0;
								receiveState=URS_Data;
							}
                            else if(buf==0x08)
							{
								mrLen=5;
								mrDcnt=0;
								receiveState=URS_Data;
							}

							else if(buf==COMMAND_READ_PM_PAGE)
							{
								mrLen=52;
								mrDcnt=0;
								receiveState=URS_Data;
							}
							else if(buf==COMMAND_WRITE_PM_PAGE)
							{
								mrLen=5;
								mrDcnt=0;
								receiveState=URS_Data;
							}
							else if(buf==COMMAND_WRITE_PM_FLUSH)
							{
								mrLen=5;
								mrDcnt=0;
								receiveState=URS_Data;
							}
							else if(buf==COMMAND_READ_BOOTLOADER_VERSION)
							{
								mrLen=6;
								mrDcnt=0;
								receiveState=URS_Data;
							}


							//mike+}
							else receiveState++;
						}
						else
						{

                                                //DEBUG2(printf("wrong waitFCode %d\n",buf));
							receiveState=URS_Address;
							goto Chk_URI_Address;
						}
					break;
					case URS_DataLength:
						/* normally set limitation for length */
						buflen=(int)buf;
						if(buflen<MODBUS_DATA_BUFFER_SIZE-3)
                                            mrPacket.mData[mrCnt]=buf;
						else
                                            mrPacket.mData[mrCnt]=0;//mike*mrPacket.mData[mrCnt]=MODBUS_DATA_BUFFER_SIZE-3;
						mrLen=mrPacket.mData[mrCnt];
						mrCnt++;
						mrDcnt=0;
						receiveState++;
					break;
					case URS_Data:
						mrPacket.mData[mrCnt]=buf;
						mrCnt++;
						mrDcnt++;
						if(mrDcnt>=mrLen)
						{
							mrDcnt=0;
							receiveState++;
						}
					break;
					case URS_CRC:
						if(mrDcnt==0)
						{
						       //DEBUG2(printf("mrDcnt==0\n"));
							mrPacket.crc=((unsigned short)buf)<<8;
							mrDcnt++;
						}
						else if(mrDcnt==1)
						{
						       //DEBUG1(printf("mrDcnt==1\n"));
							mrPacket.crc|=(unsigned short)buf;
							/* calculate CRC */
							crc=CalculateCRC(&(mrPacket.mData[0]), mrCnt);
	                         //DEBUG2(printf("crc=%04X\n",crc));
	                         //DEBUG2(printf("mrPacket.crc=%04X\n",mrPacket.crc));
							if(crc==mrPacket.crc)
							{
								/* packet received correctly */
								//mike-SetLedMode(Led_Rs485Ind, Rs485Led_Received);
								TRC_RECEIVED=1;
							}
							else
							{
								/* packet error */
								//printf("CRC error(%d,%d,%04X,%04X)\n", mrPacket.mData[2], mrCnt, mrPacket.crc, crc);
								//mike-SetLedMode(Led_Rs485Ind, Rs485Led_Error);
								TRC_RXERROR=1;
							}
							receiveState=URS_Idle;
						}
					break;
					case URS_End:
						/* process end */
						end=1;
					break;
					default:
						receiveState=URS_Idle;
					break;
				}
			}
		}
		usleep(5000);//mike*lySleep(10000);
	}
	pthread_exit(0);
}

#else
void *ModbusDriver(void *arg)
{
	unsigned char buf;
	int ret, end, buflen;
	unsigned short crc;
	end=0;
	DEBUG2(printf("MODBUS driver start\n"));
	modbusDriverStatus=1;
	while(modbusDriverStatus)
	{
		/* rx control */
		usleep(500);
		if((TRC_RECEIVED==0)&&TRC_TXIDLE)
		{   //printf("rx\n");
		    writeLog("into recv mode!!!");
			ret=read(fdModbus, &buf, 1);
			if(ret>0) {
                writeLog("recv something!!");
				/* data read from MODBUS */
                if(receiveState==URS_Idle)
                    DEBUG2(printf("\nR URS_Idle: %02X", buf));
                else
				    DEBUG2(printf("\nR: %02X", buf));
                            //DEBUG1(printf("R: %02X,%02X,%02X,%d\n", buf,waitAddr,waitFCode,receiveState));
				/* LED control */
				switch(receiveState)
				{
					case URS_Idle:
						/* skip anything */
                                            //DEBUG2(printf("URS_Idle R: %02X\n", buf));
					break;
					case URS_Address:
Chk_URI_Address:
						if(buf==waitAddr)
						{
                             			//DEBUG2(printf("buf==waitAddr %d\n",buf));
							mrPacket.mData[0]=buf;
							mrCnt=1;
							receiveState++;
						}
                                          //else
                                            //DEBUG2(printf("wrong waitAddr %d\n",buf));
					break;
					case URS_FunctionCode:
						if(waitFCode==buf)
						{
						 	//DEBUG2(printf("buf==waitFCode %d\n",buf));
							mrPacket.mData[mrCnt]=buf;
							mrCnt++;
							/* check function code */
							if((buf==0xff)||(buf==0x10))
							{
								/* reply in fixed length for function code 0xff, 0x10 */
								mrLen=4;
								mrDcnt=0;
								receiveState=URS_Data;
							}
							//mike+{
							else if(buf==0x05)
							{
								mrLen=4;
								mrDcnt=0;
								receiveState=URS_Data;
							}
                            else if(buf==0x09)
							{
								mrLen=12;
								mrDcnt=0;
								receiveState=URS_Data;
							}
                            else if(buf==0x02)
							{
								mrLen=1540;
								mrDcnt=0;
								receiveState=URS_Data;
							}
                            else if(buf==0x03 && inBootloaderMode==1)
							{
								mrLen=5;
								mrDcnt=0;
								receiveState=URS_Data;
							}
                            else if(buf==0x08)
							{
								mrLen=5;
								mrDcnt=0;
								receiveState=URS_Data;
							}

							else if(buf==COMMAND_READ_PM_PAGE)
							{
								mrLen=52;
								mrDcnt=0;
								receiveState=URS_Data;
							}
							else if(buf==COMMAND_WRITE_PM_PAGE)
							{
								mrLen=5;
								mrDcnt=0;
								receiveState=URS_Data;
							}
							else if(buf==COMMAND_WRITE_PM_FLUSH)
							{
								mrLen=5;
								mrDcnt=0;
								receiveState=URS_Data;
							}
							else if(buf==COMMAND_READ_BOOTLOADER_VERSION)
							{
								mrLen=6;
								mrDcnt=0;
								receiveState=URS_Data;
							}


							//mike+}
							else receiveState++;
						}
						else
						{

                                                //DEBUG2(printf("wrong waitFCode %d\n",buf));
							receiveState=URS_Address;
							goto Chk_URI_Address;
						}
					break;
					case URS_DataLength:
						/* normally set limitation for length */
						buflen=(int)buf;
						if(buflen<MODBUS_DATA_BUFFER_SIZE-3)
                                            mrPacket.mData[mrCnt]=buf;
						else
                                            mrPacket.mData[mrCnt]=0;//mike*mrPacket.mData[mrCnt]=MODBUS_DATA_BUFFER_SIZE-3;
						mrLen=mrPacket.mData[mrCnt];
						mrCnt++;
						mrDcnt=0;
						receiveState++;
					break;
					case URS_Data:
						mrPacket.mData[mrCnt]=buf;
						mrCnt++;
						mrDcnt++;
						if(mrDcnt>=mrLen)
						{
							mrDcnt=0;
							receiveState++;
						}
					break;
					case URS_CRC:
						if(mrDcnt==0)
						{
						       //DEBUG2(printf("mrDcnt==0\n"));
							mrPacket.crc=((unsigned short)buf)<<8;
							mrDcnt++;
						}
						else if(mrDcnt==1)
						{
						       //DEBUG1(printf("mrDcnt==1\n"));
							mrPacket.crc|=(unsigned short)buf;
							/* calculate CRC */
							crc=CalculateCRC(&(mrPacket.mData[0]), mrCnt);
	                         //DEBUG2(printf("crc=%04X\n",crc));
	                         //DEBUG2(printf("mrPacket.crc=%04X\n",mrPacket.crc));
							if(crc==mrPacket.crc)
							{
								/* packet received correctly */
								//mike-SetLedMode(Led_Rs485Ind, Rs485Led_Received);
								TRC_RECEIVED=1;
							}
							else
							{
								/* packet error */
								//printf("CRC error(%d,%d,%04X,%04X)\n", mrPacket.mData[2], mrCnt, mrPacket.crc, crc);
								//mike-SetLedMode(Led_Rs485Ind, Rs485Led_Error);
								TRC_RXERROR=1;
							}
							receiveState=URS_Idle;
						}
					break;
					case URS_End:
						/* process end */
						end=1;
					break;
					default:
						receiveState=URS_Idle;
					break;
				}
			} else {
			    writeLog("recv 0 n nothimg!!");
			}
		}
		usleep(g_global.g_delay2*1000);//mike*lySleep(10000);
	}
	pthread_exit(0);
}
#endif

/*
	Function: MStartTX
	Description: start transmit data
	Parameters: none
	Returns: none
*/
void MStartTX(int fd)
{
	long i;
	TRC_TXIDLE=0;
	/* LED status control */
	/* send to MODBUS */
	//DEBUG2(PrintTxBuffer());   //mike+
	////writeLog(txbuffer);
	////printf("enter MStartTX!!\n");
	DebugPrint(txbuffer,txsize, "send");
	//i=write(fdModbus, txbuffer, txsize);
	i=write(fd, txbuffer, txsize);
	printf("write to %d, return %ld\n", fd, i);
	//getchar();

    MClearTX_Noise(0.01);//0.3 mike20160407+ // 0.01 chih 20190220
	receiveState=URS_Address;
	TRC_TXIDLE=1;
	TRC_RXERROR=0;


	////writeLog("MStartTX exit");
	//for(i=0;i<MODBUS_TX_BUFFER_SIZE;i++)txbuffer[i]=0; //mike+
	return;
}

/*
	Function: MClearRX
	Description: received data read, clear buffer for next reading
	Parameters: none
	Returns: none
*/
void MClearRX()
{
	//if(receiveState!=0)printf("packet dropped\n");
	waitAddr=0;
       //TRC_TXIDLE=1;//mike+
	receiveState=URS_Idle;
	TRC_RECEIVED=0;
	TRC_RXERROR=0;
       //lySleep(100000);//mike+
       //TRC_TXIDLE=0;//mike+
	//printf("MClearRX\n");
	return;
}

void MClearTX_Noise(float waittime_s)
{

	//DEBUG2(printf("MClearTX_Noise\n"));
    unsigned long i=0;
    unsigned long totalus=waittime_s*1000000;
    unsigned long gap=10000;//10ms 0.01s
    while( i<(totalus/gap)) // <= to <
    {

        receiveState=URS_Idle;
        TRC_RECEIVED=0;
        TRC_TXIDLE=1;
        TRC_RXERROR=0;
        usleep(g_global.g_delay3);//ex 1000000 = 1s
        i++;
    }



}


/*
	Function: ModbusDriverReady
	Description: check MODBUS driver status
	Parameters: none
	Returns: 1 for ready, 0 for not ready
*/
int ModbusDriverReady()
{
	return modbusDriverStatus;
}

/*
	Function: ModbusDrvInit
	Description: initialize MODBUS driver
	Parameters: none
	Returns: 0 for OK, minus for error
*/

#define TIOCSRS485 0x542F



int ModbusDrvInit(void)
{
	unsigned int i;
	unsigned int flags;
	pthread_t tid;
	int fd;
	int ret;
	struct termios T_new;

    DEBUG1(printf("ModbusDrvInit\n"));
	fdModbus=-1;
	/*open tty port*/
	fd = open(RS485_PORT, O_RDWR | O_NOCTTY );
	//fd = open(GSM_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		printf("open %s Failed, errno: %d\n", RS485_PORT, errno);
		return -1;
	}

	/*termios functions use to control asynchronous communications ports*/
	if (tcgetattr(fd, &T_new) != 0)
	{	/*fetch tty state*/
		printf("tcgetattr failed. errno: %d\n", errno);
		close(fd);
		return -2;
	}
	bzero(&T_new, sizeof(T_new));

	/*set 	9600bps, n81, RTS/CTS flow control,
		ignore modem status lines,
		hang up on last close,
		and disable other flags*/
	//T_new.c_cflag = (B9600 | CS8 | CREAD | CLOCAL | CRTSCTS);
	T_new.c_cflag = CLOCAL|CREAD;
	T_new.c_cflag |= B9600;
	T_new.c_cflag &= ~CSIZE;
	T_new.c_cflag |= CS8;
	T_new.c_cflag &=~ CSTOPB;
	T_new.c_cflag &=~ PARENB;

    //T_new.c_cflag &= ~CRTSCTS;//mike20151120+



	T_new.c_oflag = 0;
	T_new.c_iflag = 0;
	T_new.c_lflag = 0;
	if (tcsetattr(fd, TCSANOW, &T_new) != 0)
	{
		printf("tcsetattr failed. errno: %d\n", errno);
		close(fd);
		return -3;
	}
	/* set to non-blocking read */
	fcntl(fd, F_SETFL, FNDELAY);

	/* uart buffer init */
	for(i=0;i<MODBUS_TX_BUFFER_SIZE;i++)
       {
            txbuffer[i]=0;
            //printf("txbuffer[%d]=0\n",i);
       }
	txsize=0;
	TRC_TXIDLE=1;
	TRC_RECEIVED=0;
	waitAddr=0;
	receiveState=URS_Idle;
	fdModbus=fd;
	/* create thread */
	ret=pthread_create(&tid, NULL, ModbusDriver, NULL);
	if(ret!=0)
	{
		printf("cannot creat thread\n");
		return -1;
	}
	tidModbusDriver=tid;
	return 0;
}


int ModbusDrvInit_test1(void)
{
	unsigned int i;
	int ret;
	pthread_t tid;

    int fd;
    struct termios tty_attributes;
    struct serial_rs485 rs485conf;

    if ((fd = open(RS485_PORT,O_RDWR|O_NOCTTY|O_NONBLOCK))<0) {
        fprintf (stderr,"Open error on %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
	else
    {
        tcgetattr(fd,&tty_attributes);

        // c_cflag
        // Enable receiver
        tty_attributes.c_cflag |= CREAD;

        // 8 data bit
        tty_attributes.c_cflag |= CS8;

        // c_iflag
        // Ignore framing errors and parity errors.
        tty_attributes.c_iflag |= IGNPAR;

        // c_lflag
        // DISABLE canonical mode.
        // Disables the special characters EOF, EOL, EOL2,
        // ERASE, KILL, LNEXT, REPRINT, STATUS, and WERASE, and buffers
        // by lines.

        // DISABLE this: Echo input characters.
        tty_attributes.c_lflag &= ~(ICANON);

        tty_attributes.c_lflag &= ~(ECHO);

        // DISABLE this: If ICANON is also set, the ERASE character
        // erases the preceding input
        // character, and WERASE erases the preceding word.
        tty_attributes.c_lflag &= ~(ECHOE);

        // DISABLE this: When any of the characters INTR, QUIT, SUSP,
        // or DSUSP are received, generate the corresponding signal.
        tty_attributes.c_lflag &= ~(ISIG);

        // Minimum number of characters for non-canonical read.
        tty_attributes.c_cc[VMIN]=1;

        // Timeout in deciseconds for non-canonical read.
        tty_attributes.c_cc[VTIME]=0;

        // Set the baud rate
        cfsetospeed(&tty_attributes,B9600);
        cfsetispeed(&tty_attributes,B9600);

        tcsetattr(fd, TCSANOW, &tty_attributes);

        // Set RS485 mode:
        rs485conf.flags |= SER_RS485_ENABLED;
        if (ioctl (fd, TIOCSRS485, &rs485conf) < 0) {
            printf("ioctl error\n");
        }

//TEST{
		 char txBuffer[10];
    	char rxBuffer[10];

		txBuffer[0]='A';
        write(fd,txBuffer,1);

        // Read a char
        if (read(fd,&rxBuffer,1)==1) {
            printf("%c",rxBuffer[0]);
            printf("\n");
        }
//TEST}


		/* uart buffer init */
		for(i=0;i<MODBUS_TX_BUFFER_SIZE;i++)
		{
		    txbuffer[i]=0;
		    //printf("txbuffer[%d]=0\n",i);
		}
		txsize=0;
		TRC_TXIDLE=1;
		TRC_RECEIVED=0;
		waitAddr=0;
		receiveState=URS_Idle;
		fdModbus=fd;
		/* create thread */
		ret=pthread_create(&tid, NULL, ModbusDriver, NULL);
		if(ret!=0)
		{
			printf("cannot creat thread\n");
			return -1;
		}
		tidModbusDriver=tid;
	}

	return 0;
}



int ModbusDrvInit_old_485fail(void)
{
	unsigned int i;
	unsigned int flags;
	pthread_t tid;
	int fd;
	int ret;
	struct termios T_new;

    DEBUG1(printf("ModbusDrvInit\n"));
	fdModbus=-1;
	/*open tty port*/
	fd = open(RS485_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	//fd = open(GSM_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		printf("open %s Failed, errno: %d\n", RS485_PORT, errno);
		return -1;
	}

	/*termios functions use to control asynchronous communications ports*/
	if (tcgetattr(fd, &T_new) != 0)
	{	/*fetch tty state*/
		printf("tcgetattr failed. errno: %d\n", errno);
		close(fd);
		return -2;
	}

	/*set 	9600bps, n81, RTS/CTS flow control,
		ignore modem status lines,
		hang up on last close,
		and disable other flags*/
	T_new.c_cflag = (B9600 | CS8 | CREAD | CLOCAL | CRTSCTS);
	T_new.c_oflag = 0;
	T_new.c_iflag = 0;
	T_new.c_lflag = 0;
	if (tcsetattr(fd, TCSANOW, &T_new) != 0)
	{
		printf("tcsetattr failed. errno: %d\n", errno);
		close(fd);
		return -3;
	}
	/* set to non-blocking read */
	fcntl(fd, F_SETFL, FNDELAY);

	/* uart buffer init */
	for(i=0;i<MODBUS_TX_BUFFER_SIZE;i++)
       {
            txbuffer[i]=0;
            //printf("txbuffer[%d]=0\n",i);
       }
	txsize=0;
	TRC_TXIDLE=1;
	TRC_RECEIVED=0;
	waitAddr=0;
	receiveState=URS_Idle;
	fdModbus=fd;
	/* create thread */
	ret=pthread_create(&tid, NULL, ModbusDriver, NULL);
	if(ret!=0)
	{
		printf("cannot creat thread\n");
		return -1;
	}
	tidModbusDriver=tid;
	return 0;
}

int ModbusDrvInit_232Zigbeetest(void)
{
	unsigned int i;
	unsigned int flags;
	pthread_t tid;
	int fd;
	int ret;
	struct termios T_new;

    DEBUG1(printf("ModbusDrvInit_232Zigbeetest\n"));
	fdModbus=-1;
	/*open tty port*/
	fd = open(RS485_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	//fd = open(GSM_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		printf("open %s Failed, errno: %d\n", RS485_PORT, errno);
		return -1;
	}

	/*termios functions use to control asynchronous communications ports*/
	if (tcgetattr(fd, &T_new) != 0)
	{	/*fetch tty state*/
		printf("tcgetattr failed. errno: %d\n", errno);
		close(fd);
		return -2;
	}

	/*set 	9600bps, n81, RTS/CTS flow control,
		ignore modem status lines,
		hang up on last close,
		and disable other flags*/
	T_new.c_cflag = (B9600 | CS8 | CREAD | CLOCAL);
	T_new.c_oflag = 0;
	T_new.c_iflag = 0;
	T_new.c_lflag = 0;
	if (tcsetattr(fd, TCSANOW, &T_new) != 0)
	{
		printf("tcsetattr failed. errno: %d\n", errno);
		close(fd);
		return -3;
	}
	/* set to non-blocking read */
	fcntl(fd, F_SETFL, FNDELAY);

	/* uart buffer init */
	for(i=0;i<MODBUS_TX_BUFFER_SIZE;i++)
       {
            txbuffer[i]=0;
            //printf("txbuffer[%d]=0\n",i);
       }
	txsize=0;
	TRC_TXIDLE=1;
	TRC_RECEIVED=0;
	waitAddr=0;
	receiveState=URS_Idle;
	fdModbus=fd;
	/* create thread */
	ret=pthread_create(&tid, NULL, ModbusDriver, NULL);
	if(ret!=0)
	{
		printf("cannot creat thread\n");
		return -1;
	}
	tidModbusDriver=tid;
	return 0;
}


int ModbusDrvInit_232_485(void)//mikechiu20160226
{
	unsigned int i;
	unsigned int flags;
	pthread_t tid;
	int fd;
	int ret;
	struct termios T_new;

    DEBUG1(printf("ModbusDrvInit_232Zigbeetest\n"));
	fdModbus=-1;
	/*open tty port*/
	fd = open(RS485_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	//fd = open(GSM_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		printf("open %s Failed, errno: %d\n", RS485_PORT, errno);
		return -1;
	}

	/*termios functions use to control asynchronous communications ports*/
	if (tcgetattr(fd, &T_new) != 0)
	{	/*fetch tty state*/
		printf("tcgetattr failed. errno: %d\n", errno);
		close(fd);
		return -2;
	}

	/*set 	9600bps, n81, RTS/CTS flow control,
		ignore modem status lines,
		hang up on last close,
		and disable other flags*/
	T_new.c_cflag = (B9600 | CS8 | CREAD | CLOCAL);
	T_new.c_oflag = 0;
	T_new.c_iflag = 0;
	T_new.c_lflag = 0;
	if (tcsetattr(fd, TCSANOW, &T_new) != 0)
	{
		printf("tcsetattr failed. errno: %d\n", errno);
		close(fd);
		return -3;
	}
	/* set to non-blocking read */
	fcntl(fd, F_SETFL, FNDELAY);

	/* uart buffer init */
	for(i=0;i<MODBUS_TX_BUFFER_SIZE;i++)
       {
            txbuffer[i]=0;
            //printf("txbuffer[%d]=0\n",i);
       }
	txsize=0;
	TRC_TXIDLE=1;
	TRC_RECEIVED=0;
	waitAddr=0;
	receiveState=URS_Idle;
	fdModbus=fd;
	/* create thread */

	ret=pthread_create(&tid, NULL, ModbusDriver, NULL);
	if(ret!=0)
	{
		printf("cannot creat thread\n");
		return -1;
	}
	tidModbusDriver=tid;
	return 0;
}


/*
	Function: ModbusDrvDeinit
	Description: deinitialize MODBUS driver
	Parameters: none
	Returns: 0 for OK, minus for error
*/
int ModbusDrvDeinit(int fd)
{
	unsigned int cnt;
	int end;
	end=0;
	modbusDriverStatus=0;
	/* wait process end */
	while(end==0)
	{
		usleep(10000);
		cnt++;
		if(cnt>100)end=1;
		else if(receiveState==URS_End)end=1;
		else receiveState=URS_End;
	}
	//if(fdModbus!=-1)close(fdModbus);
	if(fd!=-1)close(fd);
	return 0;
}

int MyModbusDrvInit(char *port, int baud, int data_bits, char parity, int stop_bits)
{
	unsigned int i;
	unsigned int flags;
	pthread_t tid;
	int fd;
	int ret;
	struct termios T_new;
	char   szbuf[128];
	int val;
	int flag=O_RDWR|O_NOCTTY|O_NDELAY;

    writeLog("MyModbusDrvInit enter");
	fdModbus=-1;
	/*open tty port*/
	fd = open(port, flag );
	//fd = open(GSM_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		printf("open %s Failed, errno: %d\n", port, errno);
		//getchar();
		return -1;
	}
	/* set to non-blocking read */
	fcntl(fd, F_SETFL, FNDELAY);

	// val = fcntl(fd, F_GETFL, 0);
     //printf("post-open file status = 0x%x\n", val);

     //fcntl(fd, F_SETFL, 0);
     //val = fcntl(fd, F_GETFL, 0);
     //printf("post-open file status = 0x%x\n", val);
     //getchar();
	/*termios functions use to control asynchronous communications ports*/
	if (tcgetattr(fd, &T_new) != 0)
	{	/*fetch tty state*/
		printf("tcgetattr failed. errno: %d\n", errno);
		close(fd);
		//getchar();
		return -2;
	}

	T_new.c_cflag |= (CLOCAL|CREAD);
    // set baud
    switch (baud)
    {
        case 4800:
            T_new.c_cflag |= B4800;
            //cfsetispeed(&T_new, B4800);
            //cfsetospeed(&T_new, B4800);
            printf("set baud 4800\n");
            break;
        case 9600:
            T_new.c_cflag |= B9600;
            //cfsetispeed(&T_new, B9600);
            //cfsetospeed(&T_new, B9600);
            printf("set baud 9600\n");
            break;
        case 19200:
            T_new.c_cflag |= B19200;
            //cfsetispeed(&T_new, B19200);
            //cfsetospeed(&T_new, B19200);
            printf("set baud 19200\n");
            break;
        case 38400:
            T_new.c_cflag |= B38400;
            //cfsetispeed(&T_new, B38400);
            //cfsetospeed(&T_new, B38400);
            printf("set baud 38400\n");
            break;
        case 57600:
            T_new.c_cflag |= B57600;
            //cfsetispeed(&T_new, B57600);
            //cfsetospeed(&T_new, B57600);
            printf("set baud 57600\n");
            break;
        case 115200:
            T_new.c_cflag |= B115200;
            //cfsetispeed(&T_new, B115200);
            //cfsetospeed(&T_new, B115200);
            printf("set baud 115200\n");
            break;
        default:
            printf("Unsupport baud rate %d\n", baud);
    }

    // set data_bits
    switch (data_bits)
    {
        case 5:
            T_new.c_cflag &= ~CSIZE;
            T_new.c_cflag |= CS5;
            printf("set bits 5\n");
            break;
        case 6:
            T_new.c_cflag &= ~CSIZE;
            T_new.c_cflag |= CS6;
            printf("set bits 6\n");
            break;
        case 7:
            T_new.c_cflag &= ~CSIZE;
            T_new.c_cflag |= CS7;
            printf("set bits 7\n");
            break;
        case 8:
            T_new.c_cflag &= ~CSIZE;
            T_new.c_cflag |= CS8;
            printf("set data bits 8\n");
            break;
        default:
            printf("Unsupport data bits %d\n", data_bits);
    }

    T_new.c_iflag = 0;
    // set parity
    switch (parity)
    {
        case 'N':
            T_new.c_cflag &= ~PARENB;   /* Clear parity enable */
            //T_new.c_iflag &= ~INPCK;    /* Disable parity checking */
            printf("set parity N\n");
            break;
        case 'O':
            T_new.c_cflag |= PARENB;    /* set odd even parity */
            T_new.c_cflag |= PARODD;    /* set odd parity */
            T_new.c_iflag |= INPCK;     /* enable parity checking */
            printf("set parity O\n");
            break;
        case 'E':
            T_new.c_cflag |= PARENB;    /* set odd even parity */
            T_new.c_cflag &= ~PARODD;   /* set even parity*/
            T_new.c_iflag |= INPCK;     /* enable parity checking */
            printf("set parity E\n");
            break;
        default:
            printf("Unsupport parity %c\n", parity);
    }

    // set stop_bits
    switch(stop_bits)
    {
        case 1:
            T_new.c_cflag &= ~CSTOPB;
            printf("set stop bits 1\n");
            break;
        case 2:
            T_new.c_cflag |= CSTOPB;
            printf("set stop bits 2\n");
            break;
        default:
            printf("Unsupport stop bits %d\n", stop_bits);
    }

    /*set 	9600bps, n81, RTS/CTS flow control,
		ignore modem status lines,
		hang up on last close,
		and disable other flags*/
	//T_new.c_cflag = (B9600 | CS8 | CREAD | CLOCAL );
	//T_new.c_iflag = 0;
	T_new.c_oflag = 0;
	T_new.c_lflag = 0;
	if (tcsetattr(fd, TCSANOW, &T_new) != 0)
	{
		printf("tcsetattr failed. errno: %d\n", errno);
		close(fd);
		//getchar();
		return -3;
	}

	/* uart buffer init */
	for(i=0;i<MODBUS_TX_BUFFER_SIZE;i++)
       {
            txbuffer[i]=0;
            //printf("txbuffer[%d]=0\n",i);
       }
	txsize=0;
	TRC_TXIDLE=1;
	TRC_RECEIVED=0;
	waitAddr=0;
	receiveState=URS_Idle;
	fdModbus=fd;
	/* create thread */

	tidModbusDriver=tid;

	printf("MyModbusDrvInit OK, fdModbus=%d",fdModbus);
	//getchar();
	return fdModbus;
}

unsigned char *GetRespond(int fd, int iSize, int delay)
{
    //unsigned char buff[512];
	int i = 0, total_delat = 0, count = 0, delay_time = 200000; // us
	int len = 0, all_len = 0;
	int  iRet=1;
	unsigned char *pbuf, *p;
	//pbuf = (unsigned char *)malloc(iSize+1);
	//p = pbuf;

	memset(respond_buff, 0x00, 4096);
    pbuf = respond_buff;
	while (total_delat < delay) {


//printf("wait recv data need:%d, Recved: %d \n", iSize, len);
//printf("wait Slave Address : 0x%02X, wait Function Code : 0x%02X \n", waitAddr, waitFCode);
        //getchar();
        //usleep(g_global.g_delay1*1000);
        //i=read(fdModbus, p, iSize-len);
        //i=read(fdModbus, buff, 511);
        //i=read(fdModbus, respond_buff, iSize);
        //i=read(fdModbus, respond_buff, 4096); // get all data(if 0x00 start command), clean return data
        i=read(fd, pbuf, 4096); // get all data(if 0x00 start command), clean return data
        if (i==-1) {
            //printf("read error code=%d (%s) \n",errno, strerror(errno));
            have_respond = false;
            usleep(delay_time);
            total_delat += delay_time;
            continue;
        }
        //printf("read %d / %d bytes \n",i, iSize-len);
        printf("read %d / %d bytes \n",i, 4096);
        DebugPrint(pbuf, i, "recv");
        //DebugPrint(p, i, "receive");
        //len += i;
        len = i;
        all_len += len;
        printf("all_len = %d\n", all_len);
        have_respond = true;
        //p+=i;
        //usleep(g_delay2*1000);
        /*if (i==0 && count > iTimeout) {
	       //free(pbuf);
           return NULL;
           break;
        }
        count++;*/

        if ( all_len >= iSize ) {
            DebugPrint(respond_buff, all_len, "Buffer");
            for (i = 0; i < all_len-6; i++) {
                if ( respond_buff[i] == waitAddr && respond_buff[i+1] == waitFCode ) {
                    switch ( respond_buff[i+1] )
                    {
                        case 0x00: // query
                            if ( respond_buff[i+2] == 0x08 ) {
                                if ( CheckCRC(respond_buff+i, 13) ) {
                                    DebugPrint(respond_buff+i, 13, "Query recv");
                                    return respond_buff+i;
                                }
                            }
                            break;
                        case 0x03: // read
                            count = respond_buff[i+2];
                            if ( CheckCRC(respond_buff+i, count+5) ) {
                                DebugPrint(respond_buff+i, count+5, "Read recv");
                                return respond_buff+i;
                            }
                            break;
                        case 0x05: // Enable Priority 3 &  Shutdown System & Entire Calibration Mode
                            if ( CheckCRC(respond_buff+i, 8) ) {
                                switch ( respond_buff[i+3] )
                                {
                                    case 0x00: // Enable Priority 3
                                        DebugPrint(respond_buff+i, 8, "Enable Priority 3 recv");
                                        break;
                                    case 0x01: // Shutdown System
                                        DebugPrint(respond_buff+i, 8, "Shutdown System recv");
                                        break;
                                    case 0x10: // Reboot System
                                        DebugPrint(respond_buff+i, 8, "Reboot System recv");
                                        break;
                                }
                                return respond_buff+i;
                            }
                            break;
                        case 0x10: // write
                        case 0x11: // for fw update
                            if ( CheckCRC(respond_buff+i, 8) ) {
                                DebugPrint(respond_buff+i, 8, "Write recv");
                                return respond_buff+i;
                            }
                            break;
                        case 0x30:
                            if ( respond_buff[i+2] == 0x0F ) {
                                if ( CheckCRC(respond_buff+i, 15) ) {
                                    DebugPrint(respond_buff+i, 15, "White List count recv");
                                    return respond_buff+i;
                                }
                            }
                            break;
                        case 0x31:
                            if ( CheckCRC(respond_buff+i, respond_buff[i+2]) ) {
                                DebugPrint(respond_buff+i, respond_buff[i+2], "White List SN recv");
                                return respond_buff+i;
                            }
                            break;
                        case 0x33:
                            if ( CheckCRC(respond_buff+i, respond_buff[i+2]) ) {
                                DebugPrint(respond_buff+i, respond_buff[i+2], "0x33 Read recv");
                                return respond_buff+i;
                            }
                            break;
                        case 0x40:
                            if ( respond_buff[i+2] == 0x0E ) {
                                if ( CheckCRC(respond_buff+i, 14) ) {
                                    DebugPrint(respond_buff+i, 14, "Clear White List recv");
                                    return respond_buff+i;
                                }
                            }
                            break;
                        case 0x41:
                            if ( respond_buff[i+2] == 0x0E ) {
                                if ( CheckCRC(respond_buff+i, 14) ) {
                                    DebugPrint(respond_buff+i, 14, "Write White List recv");
                                    return respond_buff+i;
                                }
                            }
                            break;
                        case 0x42:
                            if ( respond_buff[i+2] == 0x0E ) {
                                if ( CheckCRC(respond_buff+i, 14) ) {
                                    DebugPrint(respond_buff+i, 14, "Add White List recv");
                                    return respond_buff+i;
                                }
                            }
                            break;
                        case 0x43:
                            if ( respond_buff[i+2] == 0x0E ) {
                                if ( CheckCRC(respond_buff+i, 14) ) {
                                    DebugPrint(respond_buff+i, 14, "Delete White List recv");
                                    return respond_buff+i;
                                }
                            }
                            break;
                        case 0x45:
                            if ( respond_buff[i+2] == 0x0E ) {
                                if ( CheckCRC(respond_buff+i, 14) ) {
                                    DebugPrint(respond_buff+i, 14, "Reboot Specify recv");
                                    return respond_buff+i;
                                }
                            }
                            break;
                        case 0x48:
                            if ( respond_buff[i+2] == 0x0E ) {
                                if ( CheckCRC(respond_buff+i, 14) ) {
                                    DebugPrint(respond_buff+i, 14, "Update FW ver recv");
                                    return respond_buff+i;
                                }
                            }
                            break;
                        case 0x49:
                            if ( respond_buff[i+2] == 0x0E ) {
                                if ( CheckCRC(respond_buff+i, 14) ) {
                                    DebugPrint(respond_buff+i, 14, "Update FW data recv");
                                    return respond_buff+i;
                                }
                            }
                            break;
                        case 0x4B:
                            if ( respond_buff[i+2] == 0x0E ) {
                                if ( CheckCRC(respond_buff+i, 14) ) {
                                    DebugPrint(respond_buff+i, 14, "LBD Re-register recv");
                                    return respond_buff+i;
                                }
                            }
                            break;
                        case 0xFF: // allocate
                            if ( respond_buff[i+2] == 0x00 && respond_buff[i+3] == 0x01 && respond_buff[i+4] == 0x01 && respond_buff[i+5] == 0x06 ) {
                                if ( CheckCRC(respond_buff+i, 8) ) {
                                    DebugPrint(respond_buff+i, 8, "Allocate recv");
                                    return respond_buff+i;
                                }
                            }
                            break;
                    }
                }
            }
        }

        usleep(delay_time);
        total_delat += delay_time;
        pbuf += len;
        continue;
	}
	//DebugPrint(respond_buff, len, "recv");

	if ( all_len > 0 ) {
        i = 0;
        while ( i != -1 ) {
            //i=read(fdModbus, respond_buff, 4096);
            i=read(fd, respond_buff, 4096);
            if ( i!= -1 )
                DebugPrint(respond_buff, i, "Clean");
        }
    } else
        // no response
        printf("NO response!!\n");

	return NULL;
}

int GetQuery(int fd, unsigned char *buf, int buf_size)
{
    int len = 0;

    //len = read(fdModbus, buf, buf_size);
    len = read(fd, buf, buf_size);
    if (len == -1) {
        printf("read error code=%d (%s) \n",errno, strerror(errno));
        have_respond = false;
        return len;
    }
    have_respond = true;
    printf("read %d / %d bytes \n", len, buf_size);
    //DebugPrint(buf, len, "recv");

    return len;
}

unsigned char *GetCyberPowerRespond(int fd, int iSize, int delay)
{
    //unsigned char buff[512];
	int i = 0, err = 0, count = 0;
	int len=0;
	int  iRet=1;
	unsigned char *pbuf, *p;
	//pbuf = (unsigned char *)malloc(iSize+1);
	//p = pbuf;

	memset(respond_buff, 0x00, 4096);
	while (err < 3) {
//printf("wait recv data need:%d, Recved: %d \n", iSize, len);
//printf("wait Slave Address : 0x%02X, wait Function Code : 0x%02X \n", waitAddr, waitFCode);
        //getchar();
        //usleep(g_global.g_delay1*1000);
        //i=read(fdModbus, p, iSize-len);
        //i=read(fdModbus, buff, 511);
        //i=read(fdModbus, respond_buff, iSize);
        i=read(fd, respond_buff, iSize);
        if (i==-1) {
            printf("read error code=%d (%s) \n",errno, strerror(errno));
            have_respond = false;
            return NULL;
            break;
        }
        //printf("read %d / %d bytes \n",i, iSize-len);
        printf("read %d / %d bytes \n",i, 511);
        //DebugPrint(p, i, "receive");
        len += i;
        //DebugPrint(respond_buff, len, "recv");
        have_respond = true;
        //p+=i;
        //usleep(g_delay2*1000);
        /*if (i==0 && count > iTimeout) {
	       //free(pbuf);
           return NULL;
           break;
        }
        count++;*/

        if ( len >= iSize ) {
            for (i = 0; i < iSize-6; i++) {
                if ( respond_buff[i] == waitAddr && respond_buff[i+1] == waitFCode ) {
                    // check function code
                    switch ( respond_buff[i+1] )
                    {
                        case 0x04: // read input registers
                            count = respond_buff[i+2];
                            if ( CheckCRC(respond_buff+i, count+5) ) {
                                DebugPrint(respond_buff+i, count+5, "Read recv");
                                return respond_buff+i;
                            }
                            break;
                        default:
                            printf("Function code %d not found!\n", respond_buff[i+1]);
                    }
                }
            }
        }
        else
            err++;

        usleep(delay);
	}
	//DebugPrint(respond_buff, len, "recv");
	printf("#### GetCyberPowerRespond() clean buf ####\n");
	while ( i != -1 ) {
        //i=read(fdModbus, respond_buff, 511);
        i=read(fd, respond_buff, 511);
        DebugPrint(respond_buff, i, "Clean");
	}
    printf("#### GetCyberPowerRespond() clean OK ####\n");

	return NULL;
}

unsigned char *GetADtekRespond(int fd, int iSize, int delay)
{
    //unsigned char buff[512];
	int i = 0, err = 0, count = 0;
	int len=0;
	int  iRet=1;
	unsigned char *pbuf, *p;
	//pbuf = (unsigned char *)malloc(iSize+1);
	//p = pbuf;

	memset(respond_buff, 0x00, 4096);
	while (err < 3) {
//printf("wait recv data need:%d, Recved: %d \n", iSize, len);
//printf("wait Slave Address : 0x%02X, wait Function Code : 0x%02X \n", waitAddr, waitFCode);
        //getchar();
        //usleep(g_global.g_delay1*1000);
        //i=read(fdModbus, p, iSize-len);
        //i=read(fdModbus, buff, 511);
        //i=read(fdModbus, respond_buff, iSize);
        i=read(fd, respond_buff, iSize);
        if (i==-1) {
            printf("read error code=%d (%s) \n",errno, strerror(errno));
            // for test, set fack data
            //have_respond = false;
            //return NULL;
            //break;
            have_respond = true;
            i = 7;
            if ( txbuffer[3] == 0 ) {
                respond_buff[0] = 0x01;
                respond_buff[1] = 0x03;
                respond_buff[2] = 0x02;
                respond_buff[3] = 0x09;
                respond_buff[4] = 0x29;
                respond_buff[5] = 0x7F;
                respond_buff[6] = 0xCA;
            }
            if ( txbuffer[3] == 8 ) {
                respond_buff[0] = 0x01;
                respond_buff[1] = 0x03;
                respond_buff[2] = 0x02;
                respond_buff[3] = 0x00;
                respond_buff[4] = 0x02;
                respond_buff[5] = 0x39;
                respond_buff[6] = 0x85;
            }

        }
        //printf("read %d / %d bytes \n",i, iSize-len);
        printf("read %d / %d bytes \n",i, 511);
        //DebugPrint(p, i, "receive");
        //len += i;
        len = i;
        //DebugPrint(respond_buff, len, "recv");
        have_respond = true;
        //p+=i;
        //usleep(g_delay2*1000);
        /*if (i==0 && count > iTimeout) {
	       //free(pbuf);
           return NULL;
           break;
        }
        count++;*/

        if ( len >= iSize ) {
            for (i = 0; i < iSize-5; i++) {
                if ( respond_buff[i] == waitAddr && respond_buff[i+1] == waitFCode ) {
                    // check function code
                    switch ( respond_buff[i+1] )
                    {
                        case 0x03: // read input registers
                            count = respond_buff[i+2];
                            if ( CheckCRC(respond_buff+i, count+5) ) {
                                DebugPrint(respond_buff+i, count+5, "Read recv");
                                return respond_buff+i;
                            }
                            break;
                        default:
                            printf("Function code %d not found!\n", respond_buff[i+1]);
                    }
                }
            }
        }
        else
            err++;

        usleep(delay);
	}
	//DebugPrint(respond_buff, len, "recv");
	printf("#### GetADtekRespond() clean buf ####\n");
	while ( i != -1 ) {
        //i=read(fdModbus, respond_buff, 511);
        i=read(fd, respond_buff, 511);
        DebugPrint(respond_buff, i, "Clean");
	}
    printf("#### GetADtekRespond() clean OK ####\n");

	return NULL;
}

void CleanRespond(int fd)
{
    int i = 0;

    printf("#### CleanRespond() start ####\n");
	while ( i != -1 ) {
        //i=read(fdModbus, respond_buff, 4096);
        i=read(fd, respond_buff, 4096);
        if ( i > 0 )
            DebugPrint(respond_buff, i, "Clean");
	}
    printf("#### CleanRespond() clean OK ####\n");
}
