#ifndef G320_H_INCLUDED
#define G320_H_INCLUDED

#include "string.h"
#include "gdefine.h"

#include <time.h>

#define WHITE_LIST_SIZE 2048
#define QUERY_SIZE      4096
#define RESPOND_SIZE    4096
#define LOG_BUF_SIZE    640*500
#define BMS_BUF_SIZE    1024*16
#define BMS_PANAMOD_SIZE 94 //47*2
#define BMS_MODULE_SIZE 112 //56*2

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
extern void MStartTX(int fd);
extern void MClearRX();
extern void MClearTX_Noise(float waittime_s);

extern void MakeReadDataCRC(unsigned char *,int );
extern bool CheckCRC(unsigned char *,int );
extern void DumpMiInfo(unsigned char *, MI_INFO *);
extern void Dumpdata(unsigned char *, MI_INFO *);

extern int ModbusDriverReady();
extern int ModbusDrvInit(void);
extern int ModbusDrvDeinit(int fd);
extern unsigned char respond_buff[RESPOND_SIZE];
extern unsigned char *GetRespond(int fd, int iSize, int delay);
extern int GetQuery(int fd, unsigned char *buf, int buf_size);
extern void CleanRespond();
}

class CG320
{
public:
	CG320();
	virtual ~CG320();

	int     Init(int addr, int com, bool open_com, bool first, int busfd);
	bool    GetDLConfig();
	int     DoReRegister(time_t loop_time);
	int     DoAllRegister(time_t loop_time);
    bool    SaveDeviceList(bool first, bool last);
	void    Start();
	int     GetData(time_t data_time, bool first, bool last);
	void    Pause();
	void    Play();
	void    Stop();

protected:
    void    GetMAC();
    bool    SetPath();

    void    CleanParameter();
    bool    CheckConfig();
    bool    RunTODOList();
    bool    RunWhiteListChanged();
    bool    RunRejoin();
    bool    RunClearAll();
    bool    SendRejoinNetwork();

    int     GetWhiteListCount();
    bool    DumpWhiteListCount(unsigned char *buf);
    bool    GetWhiteListSN();
    bool    DumpWhiteListSN();
    bool    ClearWhiteList();
    bool    WriteWhiteList(int num, unsigned char *listbuf);
    bool    AddWhiteList(int num, unsigned char *listbuf);
    bool    DeleteWhiteList(int num, unsigned char *listbuf);
    int     GetPLCStatus(int index); // idel 0, busy 1, no respond or CRC error 2

    bool    LoadWhiteList();
    bool    LoadWhiteListV3();
    bool    SavePLCWhiteList();
    bool    SaveWhiteList();

    int     WhiteListRegister();
    int     StartRegisterProcess();
    int     AllocateProcess(unsigned char *query, int len);
    bool    GetDevice(int index); // only 2.0 use, 3.0 use GetMiIDInfoV3 to get device num
    bool    ReRegister(int index);
    bool    ReRegisterV3(int index);
    int     WhiteListV3Init();

    bool    GetMiIDInfo(int index);
    bool    GetMiIDInfoV3(int index); // 3.0 get device num too
    void    DumpMiIDInfo(int index, unsigned char *buf);
    bool    GetMiPowerInfo(int index);
    bool    GetMiPowerInfoV3(int index);
    void    DumpMiPowerInfo(unsigned char *buf);

    bool    GetHybridIDData(int index);
    void    DumpHybridIDData(unsigned char *buf);
    bool    SetHybridIDData(int index);
    void    ParserHybridIDFlags1(int flags);
    void    ParserHybridIDFlags2(int flags);
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
    void    DumpHybridRTInfo2(unsigned char *buf);
    void    ParserHybridPVInvErrCOD1(int COD1);
    void    ParserHybridPVInvErrCOD2(int COD2);
    void    ParserHybridPVInvErrCOD3(int COD3);
    void    ParserHybridDDErrCOD(int COD);
    void    ParserHybridDDErrCOD2(int COD2);
    void    ParserHybridIconInfo(int Icon_L, int Icon_H);
    bool    GetHybridBMSInfo(int index);
    void    DumpHybridBMSInfo(unsigned char *buf);
    bool    SetHybridBMSModule(int index);
    bool    GetHybridPanasonicModule(int index);
    bool    SetHybridPanasonicModule(int index);
    bool    GetHybridBMSModule(int index, int module);
    bool    SetBMSFile(int index, int module);

    bool    GetTimezone();
    void    SetTimezone(char *zonename, char *timazone);
    void    GetLocalTime();
    void    GetNTPTime();

    void    SetLogXML();
    bool    WriteLogXML(int index);
    bool    SaveLogXML(bool first, bool last);
    void    SetErrorLogXML();
    bool    WriteErrorLogXML(int index);
    bool    SaveErrorLogXML(bool first, bool last);
    void    SetBMSPath(int index);
    bool    SaveBMS();
    bool    WriteMIListXML();
    void    SetEnvXML();
    bool    SaveEnvXML(bool first, bool last);

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
    int     m_addr;
    int     m_busfd;
    bool    m_first;
    bool    m_last;
    int     m_milist_size;
    int     m_loopstate;
    int     m_loopflag;
    int     m_sys_error;
    int     m_inverter_state;
    bool    m_do_get_TZ;
    struct tm   m_data_st_time;
    struct tm   *m_st_time;
    time_t  m_last_read_time;
    time_t  m_last_register_time;
    time_t  m_last_search_time;
    time_t  m_last_savelog_time;
    time_t  m_current_time;
    int     m_plcver;
    bool    m_save_hb_id_data;
    bool    m_save_hb_rs_info;
    bool    m_save_hb_rrs_info;
    bool    m_save_hb_rt_info;
    bool    m_save_hb_bms_info;

    MI_ID_INFO      m_mi_id_info;
    MI_POWER_INFO   m_mi_power_info;

    HB_ID_DATA      m_hb_id_data;
    HB_ID_FLAGS1    m_hb_id_flags1;
    HB_ID_FLAGS2    m_hb_id_flags2;
    HB_RTC_DATA     m_hb_rtc_data;
    HB_RS_INFO      m_hb_rs_info;
    HB_RRS_INFO     m_hb_rrs_info;
    HB_RT_INFO      m_hb_rt_info;
    HB_PVINV_ERR_COD1   m_hb_pvinv_err_cod1;
    HB_PVINV_ERR_COD2   m_hb_pvinv_err_cod2;
    HB_PVINV_ERR_COD3   m_hb_pvinv_err_cod3;
    HB_DD_ERR_COD   m_hb_dd_err_cod;
    HB_DD_ERR_COD2  m_hb_dd_err_cod2;
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
    char            m_bms_header[8192];
    char            m_env_filename[128];
};

#endif // G320_H_INCLUDED
