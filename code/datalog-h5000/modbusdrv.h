/*
**********************************************************
*Product       MODBUS drivers
*Version       1.0
*Date          10:50, Oct 14, 2010
**********************************************************
*/


#ifndef __MODBUS_DRV_H__
#define __MODBUS_DRV_H__
/*****************************************************************************/
/* global switches */
/* data type definition */

#define MODBUS_DATA_BUFFER_SIZE		1544



typedef struct MODBUS_REPLY_PACKET
{
	unsigned char mData[1544];//MODBUS_DATA_BUFFER_SIZE
	unsigned short crc;
}MODBUS_REPLY_PACKET;

/* global variables */
#define MODBUS_TX_BUFFER_SIZE		1544
extern unsigned int txsize;
extern unsigned char txbuffer[1544];//MODBUS_TX_BUFFER_SIZE
extern int TRC_RECEIVED, TRC_TXIDLE, TRC_RXERROR, ZIGBEECMD_RECEIVED,ZIGBEECMD_RESPONSE_COUNT;
extern MODBUS_REPLY_PACKET mrPacket;
extern unsigned char waitAddr, waitFCode;
extern unsigned int mrDcnt, mrCnt, mrLen;
/* function definition */
unsigned short CalculateCRC(unsigned char *p, unsigned int len);
void *ModbusDriver(void *);
//void ModbusDriver();
extern void MStartTX();
extern void MClearRX();
extern void MClearTX_Noise(float waittime_s);

int ModbusDriverReady();
int ModbusDrvInit(void);
int ModbusDrvDeinit(void);
unsigned char *GetRespond(int iSize, int iTimeout);
int GetQuery(unsigned char *buf, int buf_size);

#endif /* __MODBUS_DRV_H__ */
