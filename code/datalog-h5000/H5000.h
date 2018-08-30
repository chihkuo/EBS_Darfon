#ifndef H5000_H_INCLUDED
#define H5000_H_INCLUDED

#include "string.h"
#include "gdefine.h"

extern "C" {
/* global variables */
#ifndef STR_MOD_PACKET
#define STR_MOD_PACKET
typedef struct MODBUS_REPLY_PACKET
{
	unsigned char mData[1544];//MODBUS_DATA_BUFFER_SIZE
	unsigned short crc;
}MODBUS_REPLY_PACKET;
#endif

typedef struct stHybrid5k_IDData {
    unsigned char Grid_Voltage;
    int Model;
    int SN_Hi;
    int SN_Lo;
    int Year;
    unsigned char Month;
    unsigned char Date;
    int Inverter_Ver;
    int DD_Ver;
    int EEPROM_Ver;
    unsigned char Flags;
}HB_IDDATA;

typedef struct stHybrid5k_IDFlags {
    unsigned char Rule21;
    unsigned char PVParallel;
    unsigned char PVOffGrid;
    unsigned char Heco1;
    unsigned char Heco2;
}HB_IDFLAGS;

typedef struct stHybrid5k_RTCData {
    unsigned char Seconds;
    unsigned char Minutes;
    unsigned char Hours;
    unsigned char Date;
    unsigned char Month;
    int Year;
    unsigned char Day;
}HB_RTCDATA;

typedef struct stHybrid5k_RemoteSettingInfo {
    unsigned char Mode;
    unsigned char StarHour;
    unsigned char StarMin;
    unsigned char EndHour;
    unsigned char EndMin;
    unsigned char MultiModeSetting;
    unsigned char BatteryType;
    unsigned char BatteryCurrent;
    float BatteryShutdownVoltage;
    float BatteryFloatingVoltage;
    unsigned char BatteryReservePercentage;
    unsigned char RampRatePercentage;
    unsigned char VoltDividVAr;
    unsigned int DegreeLeadLag;
}HB_RSINFO;

typedef struct stHybrid5k_RemoteRealtimeSettingInfo {
    unsigned char Charge;
    unsigned char Grid;
}HB_RRSINFO;

typedef struct stHybrid5k_RealTimeInfo {
    int Ppv_A;
    int Ppv_B;
    int Vac_A;
    int Pac_A;
    float VGrid_A;
    int PGrid_A;
    float VBattery;
    float IBattery;
    int Ppv_TodayH;
    float Ppv_TodayL;
    unsigned char Battery_SOC;
}HB_RTINFO;

typedef struct stHybrid5k_BMSInfo {
    int Voltage;
    int Current;
    unsigned char SOC;
    char MaxTemperature;
    int CycleCount;
    int Status;
    int Error;
}HB_BMSINFO;

#define MODBUS_TX_BUFFER_SIZE		1544
extern unsigned int txsize;
extern unsigned char txbuffer[1544];//MODBUS_TX_BUFFER_SIZE
extern int TRC_RECEIVED, TRC_TXIDLE, TRC_RXERROR, ZIGBEECMD_RECEIVED,ZIGBEECMD_RESPONSE_COUNT;
extern MODBUS_REPLY_PACKET mrPacket;
extern unsigned char waitAddr, waitFCode;
extern unsigned int mrDcnt, mrCnt, mrLen;
/* function definition */
unsigned short CalculateCRC(unsigned char *, unsigned int );
void *ModbusDriver(void *);
//void ModbusDriver();
extern void MStartTX();
extern void MClearRX();
extern void MClearTX_Noise(float waittime_s);

extern void MakeReadDataCRC(unsigned char *,int );
extern bool CheckCRC(unsigned char *,int );


extern int ModbusDriverReady();
extern int ModbusDrvInit(void);
extern int ModbusDrvDeinit(void);
extern unsigned char *GetRespond(int iSize, int iTimeout);

}

class CH5000
{
public:
	CH5000();
	virtual ~CH5000();

	void Init();
	void Start();
	void Pause();
	void Play();
	void Stop();

	int StartRegisterProcess();

protected:
    char     m_strSN[18];  // 16 char

    int     m_SlaveID;

    void    SendAllocatedAddress(char *idSN, unsigned int AllocatedAddress);
    int     AssignAddress(char* ID);
    void    WriteAllMIDataToRAM();

    bool    ReRegiser(int index);
    bool    GetIDData(int index);
    void    DumpIDData(byte *buf);
    bool    GetRTCData(int index);
    bool    SetRTCData(int index);
    void    DumpRTCData(byte *buf);
    bool    GetRemoteSettingInfo(int index);
    void    DumpRemoteSettingInfo(byte *buf);
    bool    GetRemoteRealtimeSettingInfo(int index);
    void    DumpRemoteRealtimeSettingInfo(byte *buf);
    bool    GetRealTimeInfo(int index);
    bool    GetBMSInfo(int index);

    char    *wday[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    SNOBJ   arySNobj[256];
    int     m_snCount;
    HB_IDDATA   m_hb_iddata;
    HB_IDFLAGS  m_hb_idflags;
    HB_RTCDATA  m_hb_rtcdata;
    HB_RSINFO   m_hb_rsinfo;
    HB_RRSINFO  m_hb_rrsinfo;
    HB_RTINFO   m_hb_rtinfo;
    HB_BMSINFO  m_hb_bmsinfo;
};

#endif // G320_H_INCLUDED
