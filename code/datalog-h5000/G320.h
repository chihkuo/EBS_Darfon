#ifndef G320_H_INCLUDED
#define G320_H_INCLUDED

#include "string.h"
#include "gdefine.h"

#include <time.h>

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

#define WHITE_LIST_SIZE 2048
#define QUERY_SIZE      4096
#define RESPOND_SIZE    4096
#define LOG_BUF_SIZE    640*500
#define BMS_BUF_SIZE    1024*16
#define BMS_PANAMOD_SIZE 94 //47*2
#define BMS_MODULE_SIZE 112 //56*2
#define DEF_PATH        "/tmp/test"
#define BMS_PATH        DEF_PATH"/BMS"
#define XML_PATH        DEF_PATH"/XML"
#define SYSLOG_PATH     DEF_PATH"/SYSLOG"
#define DEVICELIST_PATH "/tmp/DeviceList"
//#define USB_PATH        "/tmp/usb"
#define USB_PATH        "/tmp/run/mountd/sda1"
#define SDCARD_PATH     "/tmp/sdcard"

#define WHITE_LIST_PATH "/usr/home/White-List.txt"
#define TODOLIST_PATH   "/tmp/TODOList"
#define WL_CHANGED_PATH "/tmp/WL_Changed"

#define TIME_SERVER_URL "https://www.worldtimeserver.com/handlers/GetData.ashx?action=GCTData"

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
extern void DumpMiInfo(unsigned char *, MI_INFO *);
extern void Dumpdata(unsigned char *, MI_INFO *);

extern int ModbusDriverReady();
extern int ModbusDrvInit(void);
extern int ModbusDrvDeinit(void);
extern unsigned char respond_buff[RESPOND_SIZE];
extern unsigned char *GetRespond(int iSize, int delay);
extern int GetQuery(unsigned char *buf, int buf_size);
}

class CG320
{
public:
	CG320();
	virtual ~CG320();

	void    Init();
	void    Start();
	void    Pause();
	void    Play();
	void    Stop();

protected:
    void    GetMAC();
    bool    GetDLConfig();
    bool    SetPath();

    void    CleanParameter();
    bool    CheckConfig();
    bool    RunTODOList();
    bool    RunWhiteListChanged();

    bool    GetWhiteListCount();
    void    DumpWhiteListCount(unsigned char *buf);
    bool    GetWhiteListSN();
    bool    DumpWhiteListSN();
    bool    ClearWhiteList();
    bool    WriteWhiteList(int num, unsigned char *listbuf);
    bool    AddWhiteList(int num, unsigned char *listbuf);
    bool    DeleteWhiteList(int num, unsigned char *listbuf);

    bool    LoadWhiteList();
    bool    SavePLCWhiteList();
    bool    SaveWhiteList();
    bool    SaveDeviceList();

    int     WhiteListRegister();
    int     StartRegisterProcess();
    int     AllocateProcess(unsigned char *query, int len);
    bool    GetDevice(int index);
    bool    ReRegiser(int index);

    bool    GetMiIDInfo(int index);
    void    DumpMiIDInfo(unsigned char *buf);
    bool    GetMiPowerInfo(int index);
    void    DumpMiPowerInfo(unsigned char *buf);

    bool    GetHybridIDData(int index);
    void    DumpHybridIDData(unsigned char *buf);
    bool    SetHybridIDData(int index);
    void    ParserHybridIDFlags(int flags);
    bool    GetHybridRTCData(int index);
    void    DumpHybridRTCData(unsigned char *buf);
    bool    SetHybridRTCData(int index);
    bool    GetHybridRSInfo(int index);
    void    DumpHybridRSInfo(unsigned char *buf);
    bool    SetHybridRSInfo(int index);
    bool    GetHybridRRSInfo(int index);
    void    DumpHybridRRSInfo(unsigned char *buf);
    bool    SetHybridRRSInfo(int index);
    bool    GetHybridRTInfo(int index);
    void    DumpHybridRTInfo(unsigned char *buf);
    void    ParserHybridPVInvErrCOD1(int COD1);
    void    ParserHybridPVInvErrCOD2(int COD2);
    void    ParserHybridDDErrCOD(int COD);
    void    ParserHybridIconInfo(int Icon_L, int Icon_H);
    bool    GetHybridBMSInfo(int index);
    void    DumpHybridBMSInfo(unsigned char *buf);
    bool    SetHybridBMSModule(int index);
    bool    GetHybridPanasonicModule(int index);
    bool    SetHybridPanasonicModule(int index);
    bool    GetHybridBMSModule(int index, int module);
    bool    SetBMSFile(int index, int module);

    bool    CheckTimezone();
    bool    GetTimezone();
    void    SetTimezone(char *zonename, char *timazone);
    void    GetLocalTime();
    void    GetNetTime();

    void    SetLogXML();
    bool    WriteLogXML(int index);
    bool    SaveLogXML();
    void    SetErrorLogXML();
    bool    WriteErrorLogXML(int index);
    bool    SaveErrorLogXML();
    void    SetBMSPath(int index);
    bool    SaveBMS();
    bool    WriteMIListXML();

    FILE    *m_sitelogdata_fd;

    int     m_wl_count;
    int     m_wl_checksum;
    int     m_wl_maxid;
    unsigned char m_white_list_buf[WHITE_LIST_SIZE];
    unsigned char m_query_buf[QUERY_SIZE];
    unsigned char m_bms_panamod[BMS_PANAMOD_SIZE];
    unsigned char m_bms_buf[BMS_MODULE_SIZE];
    SNOBJ   arySNobj[253];
    int     m_snCount;
    int     m_loopstate;
    int     m_loopflag;
    struct tm   *m_st_time;
    time_t  m_last_read_time;
    time_t  m_last_register_time;
    time_t  m_last_search_time;
    time_t  m_last_savelog_time;
    time_t  m_current_time;

    MI_ID_INFO      m_mi_id_info;
    MI_POWER_INFO   m_mi_power_info;

    HB_ID_DATA      m_hb_id_data;
    HB_ID_FLAGS     m_hb_id_flags;
    HB_RTC_DATA     m_hb_rtc_data;
    HB_RS_INFO      m_hb_rs_info;
    HB_RRS_INFO     m_hb_rrs_info;
    HB_RT_INFO      m_hb_rt_info;
    HB_PVINV_ERR_COD1   m_hb_pvinv_err_cod1;
    HB_PVINV_ERR_COD2   m_hb_pvinv_err_cod2;
    HB_DD_ERR_COD   m_hb_dd_err_cod;
    HB_ICON_INFO    m_hb_icon_info;
    HB_BMS_INFO     m_hb_bms_info;

    DL_CMD          m_dl_cmd;
    DL_CONFIG       m_dl_config;
    DL_PATH         m_dl_path;

    char            m_log_buf[LOG_BUF_SIZE];
    char            m_log_filename[128];
    char            m_errlog_buf[LOG_BUF_SIZE];
    char            m_errlog_filename[128];
    char            m_bms_mainbuf[BMS_BUF_SIZE];
    char            m_bms_filename[128];
};

#endif // G320_H_INCLUDED
