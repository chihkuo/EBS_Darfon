#include "datalog.h"
#include "G320.h"
#include "sys_error.h"
#include <unistd.h>
#include <sys/stat.h>

#define DEF_PATH        "/tmp/test"
#define BMS_PATH        DEF_PATH"/BMS"
#define XML_PATH        DEF_PATH"/XML"
#define SYSLOG_PATH     DEF_PATH"/SYSLOG"
#define DEVICELIST_TMP "/tmp/tmpDeviceList"
#define DEVICELIST_PATH "/tmp/DeviceList"
#define DEV_XML_PATH        "/tmp/XML_PATH"
//#define USB_PATH        "/tmp/usb"
//#define USB_PATH        "/tmp/run/mountd/sda1"
#define USB_PATH        "/mnt"
#define USB_DEV         "/dev/sda1"
#define SDCARD_PATH     "/tmp/sdcard"

#define WHITE_LIST_PATH "/usr/home/White-List.txt"
#define WHITE_LIST_V3_PATH "/usr/home/White-List_V3.txt"
#define TODOLIST_PATH   "/tmp/TODOList"
#define WL_CHANGED_PATH "/tmp/WL_Changed"

//#define TIMEZONE_URL    "http://ip-api.com/json"
#define TIMEZONE_URL    "https://worldtimeapi.org/api/ip"
//#define TIME_OFFSET_URL "http://svn.fonosfera.org/fon-ng/trunk/luci/modules/admin-fon/root/etc/timezones.db"
#define TIME_OFFSET_URL "https://raw.githubusercontent.com/openwrt/luci/master/modules/luci-base/luasrc/sys/zoneinfo/tzdata.lua"
//#define KEY             "O10936IZHJTQ"
#define TIME_SERVER_URL "https://www.worldtimeserver.com/handlers/GetData.ashx?action=GCTData"

#define OFFLINE_SECOND_MI 1200
#define OFFLINE_SECOND_HB 1200

#define START_ADDRESS_ID        0x01
#define START_ADDRESS_RTC       0x40
#define START_ADDRESS_RSINFO    0x90
#define START_ADDRESS_RRSINFO   0xA0
#define START_ADDRESS_RTINFO    0xB0
#define COUNT_ID                30
#define COUNT_RTC               32
#define COUNT_RSINFO            32
#define COUNT_RRSINFO           32
#define COUNT_RTINFO            160

#define HB2_START_ADDRESS_ID        0x01
#define HB2_START_ADDRESS_RSINFO    0xB0
#define HB2_START_ADDRESS_RSINFO2   0xC0
#define HB2_START_ADDRESS_RTINFO    0x100
#define HB2_START_ADDRESS_CEVALUE   0x160
#define HB2_START_ADDRESS_DPINFO    0x1A0
#define HB2_START_ADDRESS_RTC       0x1D0
#define HB2_START_ADDRESS_BMSINFO   0x200
#define HB2_COUNT_ID                30
#define HB2_COUNT_RSINFO            128
#define HB2_COUNT_RTINFO            96
#define HB2_COUNT_CEVALUE           128
#define HB2_COUNT_DPINFO            32
#define HB2_COUNT_RTC               16
#define HB2_COUNT_BMSINFO           64

extern "C"
{
    #include "../common/SaveLog.h"
}

extern "C" {
    extern int     MyModbusDrvInit(char *port, int baud, int data_bits, char parity, int stop_bits);
    //extern void    RemoveAllRegister(int times);
    //extern int    MyStartRegisterProcess(byte *psn);
    extern int     ModbusDrvDeinit(int fd);
    extern void    RemoveRegisterQuery(int fd, byte byAddr);
    extern int    MySyncOffLineQuery(int fd, byte addr, byte MOD, byte buf[], int buf_size);
    extern int   MyAssignAddress(int fd, unsigned char *ID, unsigned char Addr);
    extern int  MyOffLineQuery(int fd, unsigned char addr, unsigned char  buf[], int buf_size);
    //extern int SendForceCoil(byte SlaveID, byte StartAddress, unsigned int Data);
    extern bool have_respond;
    extern unsigned char respond_buff[];
}

CG320::CG320()
{
    memset(m_white_list_buf, 0x00, WHITE_LIST_SIZE);
    memset(m_query_buf, 0x00, QUERY_SIZE);
    memset(m_bms_panamod, 0x00, BMS_PANAMOD_SIZE);
    memset(m_bms_buf, 0x00, BMS_MODULE_SIZE);
    m_sitelogdata_fd = NULL;
    m_wl_count = 0;
    m_wl_checksum = 0;
    m_wl_maxid = 0;
    m_snCount = 0;
    m_addr = 1;
    m_first = true;
    m_last = true;
    m_milist_size = 0;
    m_loopstate = 0;
    m_loopflag = 0;
    m_sys_error = 0;
    m_inverter_state = 0;
    m_do_get_TZ = false;
    m_do_set_RTC = false;
    m_data_st_time = {0};
    m_st_time = NULL;
    m_last_read_time = 0;
    m_last_register_time = 0;
    m_last_search_time = 0;
    m_last_savelog_time = 0;
    m_current_time = 0;
    m_plcver = 0;
    m_mi_id_info = {0};
    m_mi_power_info = {0};
    m_hb_id_data = {0};
    m_hb_id_flags1 = {0};
    m_hb_id_flags2 = {0};
    m_hb_rtc_data = {0};
    m_hb_rs_info = {0};
    m_hb_rrs_info = {0};
    m_hb_rt_info = {0};
    m_hb_bms_info = {0};
    m_hb_bms_ver = 0;
    m_hb_pvinv_err_cod1 = {0};
    m_hb_pvinv_err_cod2 = {0};
    m_hb_pvinv_err_cod3 = {0};
    m_hb_dd_err_cod = {0};
    m_hb_dd_err_cod2 = {0};
    m_hb_icon_info = {0};
    m_dl_cmd = {0};
    m_dl_config = {0};
    m_dl_path = {0};
    m_save_hb_id_data = false;
    m_save_hb_rtc_data = false;
    m_save_hb_rs_info= false;
    m_save_hb_rrs_info= false;
    m_save_hb_rt_info= false;
    m_save_hb_bms_info= false;
    read_size = 100;
    m_hb2_id_data = {0};
    m_hb2_id_flags = {0};
    m_hb2_rs_info = {0};
    m_hb2_rs_f_flags = {0};
    m_hb2_rs_s_flags = {0};
    m_hb2_rt_info = {0};
    m_hb2_rt_m_flags = {0};
    m_hb2_rt_f_flags = {0};
    m_hb2_pvinv_err_cod1 = {0};
    m_hb2_pvinv_err_cod2 = {0};
    m_hb2_pvinv_err_cod3 = {0};
    m_hb2_dd_err_cod1 = {0};
    m_hb2_dd_err_cod2 = {0};
    m_hb2_ce_value = {0};
    m_hb2_dp_info = {0};
    m_hb2_icon_info = {0};
    m_hb2_rtc_data = {0};
    m_hb2_bms_info = {0};
    m_save_hb2_id_data = false;
    m_save_hb2_rs_info = false;
    m_save_hb2_rt_info = false;
    m_save_hb2_ce_value = false;
    m_save_hb2_dp_info = false;
    m_save_hb2_rtc_info = false;
    m_save_hb2_bms_info = false;
    memset(m_log_buf, 0x00, LOG_BUF_SIZE);
    memset(m_log_filename, 0x00, 128);
    memset(m_errlog_buf, 0x00, LOG_BUF_SIZE);
    memset(m_errlog_filename, 0x00, 128);
    memset(m_bms_mainbuf, 0x00, BMS_BUF_SIZE);
    memset(m_bms_filename, 0x00, 128);
    memset(m_env_filename, 0x00, 128);

    int i = 0;
    for (i=0; i<253; i++) {
        arySNobj[i].m_Addr=i+m_addr; // address range 1 ~ 253
        memset(arySNobj[i].m_Sn, 0x00, 17);
        arySNobj[i].m_Device = -2;
        arySNobj[i].m_Model = 0;
        arySNobj[i].m_Err = 0;
        arySNobj[i].m_state = 0;
        arySNobj[i].m_FWver = 0;
        arySNobj[i].m_ok_time = 0;
        //printf("i=%u, addr=%d\n",i,arySNobj[i].m_Addr );
    }

    printf("create bms header\n");
    char buf[128] = {0};
    memset(m_bms_header, 0x00, 8192);
    strcpy(m_bms_header, "time,from/end address");
    for (i = 0x238; i < 0x5B8; i++ ) {
        memset(buf, 0, 128);
        sprintf(buf, ",0x%03X", i);
        strcat(m_bms_header, buf);
    }
    strcat(m_bms_header, "\n");
    printf("end\n");

    sprintf(buf, "rm %s", DEVICELIST_PATH);
    system(buf);
}

CG320::~CG320()
{
    ModbusDrvDeinit(m_busfd);
}

int CG320::Init(int addr, int com, bool open_com, bool first, int busfd)
{
    int i = 0, idc = 0;
    char buf[256] = {0};
    char *port = NULL;
    char szbuf[32] = {0};
    char inverter_parity = 0;
    int ret = 0, result = 0;

    printf("#### G320 Init Start ####\n");
    // set addr
    m_addr = addr;
    for (i=0; i<253; i++) {
        arySNobj[i].m_Addr=(unsigned char)(i+m_addr); // address range 1 ~ 253
        if ( arySNobj[i].m_Addr > 253 )
            arySNobj[i].m_Addr = 253;
        memset(arySNobj[i].m_Sn, 0x00, 17);
        arySNobj[i].m_Device = -2;
        arySNobj[i].m_Model = 0;
        arySNobj[i].m_Err = 0;
        arySNobj[i].m_state = 0;
        arySNobj[i].m_FWver = 0;
        arySNobj[i].m_ok_time = 0;
        //printf("i=%u, addr=%d\n",i,arySNobj[i].m_Addr );
    }

    // set com port
    m_dl_config.m_inverter_port = com;

    GetMAC();
    GetDLConfig();

    // set save file path
    GetLocalTime();
    SetPath();
    OpenLog(m_dl_path.m_syslog_path, m_st_time);

    // get time zone
    if ( first ) {
        GetTimezone();
        usleep(1000000);
    }

    if ( open_com ) {
        port = szPort[com-1]; // COM1~4 <==> /dev/ttyS0~3 or ttyUSB0~3
        sprintf(szbuf,"port = %s \n",port);
        printf(szbuf);

        if ( strstr(m_dl_config.m_inverter_parity, "Odd") )
            inverter_parity = 'O';
        else if ( strstr(m_dl_config.m_inverter_parity, "Even") )
            inverter_parity = 'E';
        else
            inverter_parity = 'N';

        m_busfd = MyModbusDrvInit(port, m_dl_config.m_inverter_baud, m_dl_config.m_inverter_data_bits, inverter_parity, m_dl_config.m_inverter_stop_bits);
        if ( m_busfd < 0 ) {
            ret = -1;
            SaveLog((char *)"DataLogger Init() : MyModbusDrvInit() fail", m_st_time);
        } else {
            ret = m_busfd;
            sprintf(buf, "DataLogger Init() : MyModbusDrvInit() get fd %d", ret);
            SaveLog(buf, m_st_time);
        }
    } else {
        m_busfd = busfd;
    }

/*    unsigned char testbuf_tmp[] = {
                                0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0FE,
                                0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0FA,
//                                0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0FC,
                                };
    WriteWhiteList(sizeof(testbuf_tmp)/8, testbuf_tmp);
    usleep(5000000);
*/    //getchar();

    m_wl_count = 0;
    m_wl_checksum = 0;
    // check plc version
    result = GetWhiteListCount();
    if ( result >= 0 )
        m_plcver = 3;
    else
        m_plcver = 2;
    //m_plcver = 2; // for test
    printf("m_plcver = %d\n", m_plcver);
    // get white list & register
    if ( m_plcver == 3 ) { // 3.0
        if ( GetWhiteListSN() ) {
            WhiteListV3Init();
            LoadWhiteListV3(); // get m_ok_time
            WriteMIListXML();
        }
    } else if ( m_plcver == 2 ) { // 2.0
        // load DL white list, if exit run register
        if ( LoadWhiteList() ) {
            WhiteListRegister();
        }

        idc = StartRegisterProcess();
        if ( idc ) {
            printf("StartRegisterProcess success find %d new invert\n", idc);
            sprintf(buf, "DataLogger Start() : StartRegisterProcess() return %d", idc);
            SaveLog(buf, m_st_time);
            SaveWhiteList();
        }
    }

    CloseLog();

    printf("################ m_snCount = %d #################\n", m_snCount);
    for (i=0; i<m_snCount; i++)
        printf("i = %d, addr = %d, dev = %d, sn = %s\n", i, arySNobj[i].m_Addr, arySNobj[i].m_Device, arySNobj[i].m_Sn);
    printf("################################################\n");

    //GetWhiteListCount();
    //if ( m_wl_count > 0 )
    //    GetWhiteListSN();
    //usleep(2000000);
    //getchar();

    //ClearWhiteList();
    //usleep(2000000);
    //getchar();

/*    unsigned char testbuf_write[30*8] = {0};
    for (int i = 0; i < 2; i++) {
        testbuf_write[0 + i*8] = 0x00;
        testbuf_write[1 + i*8] = 0x05;
        testbuf_write[2 + i*8] = 0x00;
        testbuf_write[3 + i*8] = 0x01;
        testbuf_write[4 + i*8] = 0x00;
        testbuf_write[5 + i*8] = 0x00;
        testbuf_write[6 + i*8] = 0x00;
        testbuf_write[7 + i*8] = 0xFE - i;
    }
    unsigned char testbuf_tmp[] = {
                                0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0FE,
                                0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0FA,
//                                0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0FC,
                                };
    WriteWhiteList(sizeof(testbuf_tmp)/8, testbuf_tmp);
    usleep(2000000);
    getchar();
*/
    //AddWhiteList(sizeof(testbuf_tmp)/8, testbuf_tmp);
    //usleep(2000000);
    //getchar();

    //GetWhiteListCount();
    //if ( m_wl_count > 0 )
    //    GetWhiteListSN();
    //usleep(2000000);
    //getchar();

    //DeleteWhiteList(sizeof(testbuf_tmp)/8, testbuf_tmp);
    //usleep(2000000);
    //getchar();

    //GetWhiteListCount();
    //if ( m_wl_count > 0 ) {
    //    GetWhiteListSN();
    //    WhiteListRegister();
    //}

    printf("\n##### G320 Init End #####\n");

    //char ch;
    //printf("press enter key to next loop~\n");
    //while ((ch = getchar()) != '\n' && ch != EOF);

    return ret;
}

int CG320::DoReRegister(time_t loop_time)
{
    int i = 0, ret = 0;
    bool result = false;
    time_t current_time;
    struct tm *log_time;
    current_time = time(NULL);
    log_time = localtime(&current_time);

    printf("==== ReRegister part start ====\n");

    //m_current_time = loop_time;
    //m_last_register_time = m_current_time;
    //m_st_time = localtime(&m_current_time);
    SetPath();
    //CheckConfig();
    //GetDLConfig();

    OpenLog(m_dl_path.m_syslog_path, log_time);

    for ( i = 0; i < m_snCount; i++) {
        if ( !arySNobj[i].m_state ) {
            if ( m_plcver == 3 ) // MI PLC V3.0
                result = ReRegisterV3(i);
            else
                result = ReRegister(i); // MI PLC V2.0 & Htbrid

            // get time
            current_time = time(NULL);
            m_st_time = localtime(&current_time);
            // get data time not 0 min. current time is 0 min.
            if ( m_data_st_time.tm_min != 0 )
                if ( m_st_time->tm_min == 0 )
                    return -1;

            if ( result ) {
                ret++;
                if ( m_plcver == 3 ) {
                    if ( arySNobj[i].m_FWver == 0 )
                        GetMiIDInfoV3(i); // MI PLC V3.0
                } else {
                    if ( arySNobj[i].m_Device >= 0x0A ) // Hybrid
                        ;//SetHybridRTCData(i);
                    else
                        GetMiIDInfo(i); // MI PLC V2.0
                }

                // get time
                current_time = time(NULL);
                m_st_time = localtime(&current_time);
                // get data time not 0 min. current time is 0 min.
                if ( m_data_st_time.tm_min != 0 )
                    if ( m_st_time->tm_min == 0 )
                        return -1;
            }
        }
    }

    CloseLog();

    printf("===== ReRegister part end =====\n");

    return ret;
}

int CG320::DoAllRegister(time_t loop_time)
{
    int i = 0, ret = 0;
    char buf[256] = {0};
    time_t current_time;
    struct tm *log_time;
    current_time = time(NULL);
    log_time = localtime(&current_time);

    printf("==== AllRegister part start ====\n");

    //m_current_time = loop_time;
    //m_last_search_time = m_current_time;
    //m_st_time = localtime(&m_current_time);
    SetPath();
    //CheckConfig();
    //GetDLConfig();

    OpenLog(m_dl_path.m_syslog_path, log_time);

    // get white list & register
    if ( m_plcver == 3 ) { // 3.0
        ret = GetWhiteListCount();

        // get time
        current_time = time(NULL);
        m_st_time = localtime(&current_time);
        // get data time not 0 min. current time is 0 min.
        if ( m_data_st_time.tm_min != 0 )
            if ( m_st_time->tm_min == 0 )
                return -1;

        if ( ret > 0 ) {
            if ( m_snCount != m_wl_count ) {
                if ( GetWhiteListSN() ) {
                    WhiteListV3Init();
                    LoadWhiteListV3();
                    WriteMIListXML();
                } else {

                    // get time
                    current_time = time(NULL);
                    m_st_time = localtime(&current_time);
                    // get data time not 0 min. current time is 0 min.
                    if ( m_data_st_time.tm_min != 0 )
                        if ( m_st_time->tm_min == 0 )
                            return -1;
                }
            }
        }
    } else if ( m_plcver == 2 ) { // 2.0
        ret = StartRegisterProcess();
        if ( ret == -1 ) {
            return -1;
        }

        if ( ret ) {
            printf("Add %d new device to list\n", ret);
            sprintf(buf, "DataLogger Start() : StartRegisterProcess() return %d", ret);
            SaveLog(buf, log_time);
            for (i = 0; i < m_snCount; i++) {
                if ( arySNobj[i].m_Device == -1 )
                    GetDevice(i);
            }
            SaveWhiteList();
            WriteMIListXML();
        }
    }

    CloseLog();

    printf("===== AllRegister part end =====\n");

    return ret;
}

void CG320::Start()
{
    printf("#### G320 Start() Start ####\n");

    char    buf[256] = {0};
    int     idc = 0, i = 0;
    long    register_interval = 0, search_interval = 0, savelog_imterval = 0;
    struct stat st;
    bool    dosave = true;

    //GetLocalTime();
    //SaveLog((char *)"DataLogger Start() : start", m_st_time);
    printf("\n================================\n");
    printf("StartRegisterProcess() Start!\n");
    printf("================================\n");
    //idc = StartRegisterProcess();
    if ( idc ) {
        printf("StartRegisterProcess success find %d new invert\n", idc);
        sprintf(buf, "DataLogger Start() : StartRegisterProcess() return %d", idc);
        //SaveLog(buf, m_st_time);
        //SaveWhiteList();
    }
    printf("================================\n");
    printf("StartRegisterProcess() End!\n");
    m_last_register_time = time(NULL);
    m_last_search_time = m_last_register_time;
    printf("================================\n");

    printf("################ m_snCount = %d #################\n", m_snCount);
    for (i=0; i<m_snCount; i++)
        printf("i = %d, addr = %d, dev = %d, sn = %s\n", i, arySNobj[i].m_Addr, arySNobj[i].m_Device, arySNobj[i].m_Sn);
    printf("################################################\n");

    while (1) {
        // get local time
        //m_current_time = time(NULL);
        //m_st_time = localtime(&m_current_time);
        // set path
        SetPath();
        SetLogXML();
        SetErrorLogXML();
        // read part
        if ( m_st_time->tm_min % m_dl_config.m_sample_time == 0 ) {
            m_last_read_time = m_current_time;
            //printf("#### Debug : read start time : %ld ####\n", m_last_read_time);
            if ( (m_loopstate != 0) && (m_snCount > 0) ) {
                // before read, get write command
                RunTODOList();
                RunWhiteListChanged();

                // init about XML file
                memset(m_log_buf, 0x00, LOG_BUF_SIZE);
                memset(m_errlog_buf, 0x00, LOG_BUF_SIZE);
            }

            // read data loop
            for (i=0; i<m_snCount; i++) {
                if ( m_loopstate != 0 ) {
                    if ( stat(m_log_filename, &st) == 0 ) {
                        //printf("======== %s exist! ========\n", m_log_filename);
                        break;
                    }
                }
                printf("#### i = %d ####\n", i);
                if ( !arySNobj[i].m_state ) {   // offline
                    continue;                   // read part skip
                }
                if( arySNobj[i].m_Err < 3 ) {
                    if ( arySNobj[i].m_Device == -1 ) { // unknown device, first time to do
                        if ( GetDevice(i) )
                            dosave = true;
                        else
                            arySNobj[i].m_Err++;
                    } else if ( arySNobj[i].m_Device < 0x0A ) { // 0x00 ~ 0x09 ==> MI, 0x0A ~ 0xFFFF ==> Hybrid
                    // MI part
                        CleanParameter();

                        if ( GetMiPowerInfo(i) )
                            arySNobj[i].m_Err = 0;
                        else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                        }

                        //WriteLogXML(i);
                        if ( m_mi_power_info.Error_Code1 || m_mi_power_info.Error_Code2 ||
                            (m_sys_error && (m_st_time->tm_hour%2 == 0) && (m_st_time->tm_min == 0)) ) {
                            WriteErrorLogXML(i);
                        }
                        dosave = true;

                    } else {
                    // Hybrid part
                        CleanParameter();

                        // first set rtc time
                        if ( m_loopstate == 1 )
                            SetHybridRTCData(i);

                        if ( GetHybridIDData(i) )
                            arySNobj[i].m_Err = 0;
                        else {
                            arySNobj[i].m_Err++;
                            m_loopflag++;
                        }

                        if ( GetHybridRTCData(i) )
                            arySNobj[i].m_Err = 0;
                        else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                        }

                        if ( GetHybridRSInfo(i) )
                            arySNobj[i].m_Err = 0;
                        else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                        }

                        if ( GetHybridRRSInfo(i) )
                            arySNobj[i].m_Err = 0;
                        else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                        }

                        if ( GetHybridRTInfo(i) )
                            arySNobj[i].m_Err = 0;
                        else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                        }

                        SetBMSPath(i);
                        if ( GetHybridBMSInfo(i) ) {
                            arySNobj[i].m_Err = 0;
                            SetHybridBMSModule(i);
                            SaveBMS();
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                        }

                        //WriteLogXML(i);
                        if ( m_hb_rt_info.Error_Code || m_hb_rt_info.PV_Inv_Error_COD1_Record || m_hb_rt_info.PV_Inv_Error_COD2_Record || m_hb_rt_info.DD_Error_COD_Record ||
                            (m_sys_error && (m_st_time->tm_hour%2 == 0) && (m_st_time->tm_min == 0)) ) {
                            WriteErrorLogXML(i);
                        }
                        dosave = true;
                    }
                } else {
                    printf("Addr %d Error 3 times, call ReRegiser() later\n", arySNobj[i].m_Addr);
                    // set state to offline
                    arySNobj[i].m_state = 0;
                    //ReRegiser(i);
                }

                printf("Debug : index %d, m_Err = %d, m_loopflag = %d\n", i, arySNobj[i].m_Err, m_loopflag);
                m_loopflag = 0;
            }
            if ( (m_loopstate != 0) && (m_snCount > 0) ) {
                if ( stat(m_log_filename, &st) != 0 ) {
                    //SaveLogXML();
                    //SaveErrorLogXML();
                    dosave = true;
                }
            }

            if ( m_loopstate == 0 ) { // init : device get OK
                m_loopstate = 1;
                if ( m_snCount > 0 ) { // MI or Hybrid checked : save white list
                    //SaveLog((char *)"DataLogger Start() : run WriteMIListXML()", m_st_time);
                    //WriteMIListXML();
                }
            }
            else if ( m_loopstate == 1 ) {
                m_loopstate = 2;
                // find new device
                if ( m_wl_count < m_snCount ) {
                    //SaveLog((char *)"DataLogger Start() : run SaveWhiteList()", m_st_time);
                    //SaveWhiteList();
                }
            } else { // m_loopstate > 2 to do other thing, undefined
                if ( m_snCount > 0 )
                    ;
            }

            //m_current_time = time(NULL);
            //printf("#### Debug : read end time : %ld ####\n", m_current_time);
            printf("#### Debug : read span time : %ld ####\n", m_current_time - m_last_read_time);
        }
        if ( dosave ) {
            //SaveDeviceList();
            dosave = false;
        }

        // loop setting, delay, time ...
        usleep(1000000); // 1s
        //m_current_time = time(NULL);
        register_interval = m_current_time - m_last_register_time;
        search_interval = m_current_time - m_last_search_time;
        savelog_imterval = m_current_time - m_last_savelog_time;
        if ( m_current_time%10 == 0 ) {
            printf("######### check time #########\n");
            printf("m_current_time       = %ld\n", m_current_time);
            printf("m_last_read_time     = %ld\n", m_last_read_time);
            printf("m_last_register_time = %ld\n", m_last_register_time);
            printf("m_last_search_time   = %ld\n", m_last_search_time);
            printf("register_interval    = %ld\n", register_interval);
            printf("search_interval      = %ld\n", search_interval);
            system("date");
            printf("##############################\n");
        }

        // if interval changed
        //CheckConfig();
        GetDLConfig();

        // ReRegister part
        if ( register_interval >= 600 ) {
            printf("==== ReRegister part start ====\n");
            //m_current_time = time(NULL);
            m_last_register_time = m_current_time;
            for ( i = 0; i < m_snCount; i++) {
                if ( !arySNobj[i].m_state ) {
                    if ( ReRegister(i) ) {
                        if ( !GetDevice(i) )
                            arySNobj[i].m_Err++;
                        else {
                            // ReRegiser OK, GetDevice OK, know MI or Hybrid
                            //WriteMIListXML();
                            if ( arySNobj[i].m_Device >= 0x0A )
                                SetHybridRTCData(i);
                        }
                    }
                }
            }
            printf("===== ReRegister part end =====\n");
        }

        // Search new device
        if ( search_interval >= 3600 ) {
            printf("==== Search part start ====\n");
            // sync time from NTP
            GetNTPTime();
            // run StartRegisterProcess
            //m_current_time = time(NULL);
            m_last_search_time = m_current_time;
            //idc = StartRegisterProcess();
            if ( idc ) {
                printf("Add %d new device to list\n", idc);
                sprintf(buf, "DataLogger Start() : StartRegisterProcess() return %d", idc);
                //SaveLog(buf, m_st_time);
                for (i = 1; i <= idc; i++)
                    GetDevice(m_snCount-i);
                //SaveWhiteList();
                //WriteMIListXML();
            }
            printf("===== Search part end =====\n");
        }

        // save syslog
        if ( savelog_imterval >= m_dl_config.m_sample_time*60 ) {
            printf("==== save syslog part start ====\n");
            //m_current_time = time(NULL);
            m_last_savelog_time = m_current_time;
            CloseLog();
            system("sync");

            // get timezone if before not succss
            if ( m_do_get_TZ )
                GetTimezone();

            //GetLocalTime();
            OpenLog(m_dl_path.m_syslog_path, m_st_time);
            printf("===== save syslog part end =====\n");
        }

        //system("date");
        //printf("press enter key to next loop~\n");
        //char    ch;
        //while ((ch = getchar()) != '\n' && ch != EOF);
    }

    printf("#### G320 start() End ####\n");
    getchar();
}

int CG320::GetData(time_t data_time, bool first, bool last)
{
    int     i = 0, j = 0, isbusy = 0;
    struct  stat st;
    bool    dosave = true;

    time_t current_time;

    m_first = first;
    m_last = last;

    //m_current_time = time(NULL);
    m_current_time = data_time;
    m_st_time = localtime(&m_current_time);
    m_data_st_time.tm_year = m_st_time->tm_year;
	m_data_st_time.tm_mon = m_st_time->tm_mon;
	m_data_st_time.tm_mday = m_st_time->tm_mday;
	m_data_st_time.tm_hour = m_st_time->tm_hour;
	m_data_st_time.tm_min = m_st_time->tm_min;
	m_data_st_time.tm_sec = m_st_time->tm_sec;
    // set path
    SetPath();
    SetLogXML();
    SetErrorLogXML();
    SetEnvXML();
    //CheckConfig();
    GetDLConfig();

    OpenLog(m_dl_path.m_syslog_path, m_st_time);

    // mark m_snCount, do it if no device
    if ( (m_loopstate != 0) /*&& (m_snCount > 0)*/ ) {
        // before read, get write command
        RunTODOList();
        RunWhiteListChanged();

        // init about XML file
        memset(m_log_buf, 0x00, LOG_BUF_SIZE);
        memset(m_errlog_buf, 0x00, LOG_BUF_SIZE);
    }

    if ( m_plcver == 3 ) {
        // 3.0 part
        for (i = 0; i < m_snCount; i++) {
            // get time
            current_time = time(NULL);
            m_st_time = localtime(&current_time);
            // get data time not 0 min. current time is 0 min.
            if ( m_data_st_time.tm_min != 0 )
                if ( m_st_time->tm_min == 0 )
                    return -1;

            // power data exit
            if ( m_loopstate != 0 ) {
                if ( stat(m_log_filename, &st) == 0 ) {
                    printf("======== %s exist! ========\n", m_log_filename);
                    break;
                }
            }
            printf("#### i = %d ####\n", i);
            if( arySNobj[i].m_Err < 3 ) {
                // get time
                current_time = time(NULL);
                m_st_time = localtime(&current_time);
                // get data time not 0 min. current time is 0 min.
                if ( m_data_st_time.tm_min != 0 )
                    if ( m_st_time->tm_min == 0 )
                        return -1;

                // clean
                CleanParameter();

                // check PLC status
                do {
                    isbusy = GetPLCStatus(i);
                    if ( isbusy ) { // busy
                        printf("sleep 60 sec.\n");
                        // sleep 60s & check time
                        for (j = 0; j < 60; j++) {
                            // sleep 1s
                            usleep(1000000); // 1s
                            // get time
                            current_time = time(NULL);
                            m_st_time = localtime(&current_time);
                            // get data time not 0 min. current time is 0 min.
                            if ( m_data_st_time.tm_min != 0 )
                                if ( m_st_time->tm_min == 0 )
                                    return -1;
                        }
                        // if no respond, continue to get data
                        if ( isbusy == 2 )
                            break;
                    } else { // idle
                        printf("sleep 1 sec.\n");
                        usleep(1000000); // 1s
                    }
                } while ( isbusy );

                // get time
                current_time = time(NULL);
                m_st_time = localtime(&current_time);
                // get data time not 0 min. current time is 0 min.
                if ( m_data_st_time.tm_min != 0 )
                    if ( m_st_time->tm_min == 0 )
                        return -1;

                // check fwver if not get
                if ( arySNobj[i].m_FWver == 0 ) {
                    if ( GetMiIDInfoV3(i) )
                        arySNobj[i].m_Err = 0;
                    else {
                        if ( m_loopflag == 0 )
                            arySNobj[i].m_Err++;
                        m_loopflag++;
                    }

                    // get time
                    current_time = time(NULL);
                    m_st_time = localtime(&current_time);
                    // get data time not 0 min. current time is 0 min.
                    if ( m_data_st_time.tm_min != 0 )
                        if ( m_st_time->tm_min == 0 )
                            return -1;

                    // set m_loopflag not 0, ignore power data
                    if ( m_loopflag == 0 )
                        m_loopflag = 10;
                    WriteLogXML(i);
                    if ( m_mi_power_info.Error_Code1 || m_mi_power_info.Error_Code2 || m_inverter_state ||
                        (m_sys_error && (m_data_st_time.tm_hour%2 == 0) && (m_data_st_time.tm_min == 0)) ) {
                        WriteErrorLogXML(i);
                    }

                } else {
                    // get power data
                    if ( GetMiPowerInfoV3(i) )
                        arySNobj[i].m_Err = 0;
                    else {
                        if ( m_loopflag == 0 )
                            arySNobj[i].m_Err++;
                        m_loopflag++;
                    }

                    // get time
                    current_time = time(NULL);
                    m_st_time = localtime(&current_time);
                    // get data time not 0 min. current time is 0 min.
                    if ( m_data_st_time.tm_min != 0 )
                        if ( m_st_time->tm_min == 0 )
                            return -1;

                    WriteLogXML(i);
                    if ( m_mi_power_info.Error_Code1 || m_mi_power_info.Error_Code2 || m_inverter_state ||
                        (m_sys_error && (m_data_st_time.tm_hour%2 == 0) && (m_data_st_time.tm_min == 0)) ) {
                        WriteErrorLogXML(i);
                    }
                }

                dosave = true;

            } else {
                printf("Addr %d SN %s Error 3 times, call ReRegisterV3() later\n", arySNobj[i].m_Addr, arySNobj[i].m_Sn);
                // set state to offline
                arySNobj[i].m_state = 0; // offline
                //ReRegiserV3(i);
            }
            printf("Debug : index %d, m_Err = %d, m_loopflag = %d\n", i, arySNobj[i].m_Err, m_loopflag);
            m_loopflag = 0;
        }
    } else if ( m_plcver == 2 ) {
        // 2.0 part, MI or Hybrid possible
        for (i = 0; i < m_snCount; i++) {
            // get time
            current_time = time(NULL);
            m_st_time = localtime(&current_time);
            // get data time not 0 min. current time is 0 min.
            if ( m_data_st_time.tm_min != 0 )
                if ( m_st_time->tm_min == 0 )
                    return -1;

            // power data exit
            if ( m_loopstate != 0 ) {
                if ( stat(m_log_filename, &st) == 0 ) {
                    printf("======== %s exist! ========\n", m_log_filename);
                    break;
                }
            }
            printf("#### i = %d ####\n", i);
            // check state
            //if ( !arySNobj[i].m_state ) {   // offline
            //    continue;                   // read part skip
            //}
            if( arySNobj[i].m_Err < 3 ) {
                // get time
                current_time = time(NULL);
                m_st_time = localtime(&current_time);
                // get data time not 0 min. current time is 0 min.
                if ( m_data_st_time.tm_min != 0 )
                    if ( m_st_time->tm_min == 0 )
                        return -1;

                if ( arySNobj[i].m_Device == -1 ) { // unknown device, first time to do
                    if ( GetDevice(i) ) {
                        dosave = true;
                        if ( arySNobj[i].m_Device < 0x0A ) // 0x00 ~ 0x09 ==> MI, 0x0A ~ 0xFFFF ==> Hybrid
                            GetMiIDInfo(i); // MI get fwver
                    } else
                        arySNobj[i].m_Err++;

                    // get time
                    current_time = time(NULL);
                    m_st_time = localtime(&current_time);
                    // get data time not 0 min. current time is 0 min.
                    if ( m_data_st_time.tm_min != 0 )
                        if ( m_st_time->tm_min == 0 )
                            return -1;

                } else if ( arySNobj[i].m_Device < 0x0A ) { // 0x00 ~ 0x09 ==> MI, 0x0A ~ 0xFFFF ==> Hybrid
                // MI part
                    // clean
                    CleanParameter();

                    // check fwver if not get
                    if ( arySNobj[i].m_FWver == 0 )
                        GetMiIDInfo(i);

                    // get time
                    current_time = time(NULL);
                    m_st_time = localtime(&current_time);
                    // get data time not 0 min. current time is 0 min.
                    if ( m_data_st_time.tm_min != 0 )
                        if ( m_st_time->tm_min == 0 )
                            return -1;

                    // get power data
                    if ( GetMiPowerInfo(i) )
                        arySNobj[i].m_Err = 0;
                    else {
                        if ( m_loopflag == 0 )
                            arySNobj[i].m_Err++;
                        m_loopflag++;
                    }

                    // get time
                    current_time = time(NULL);
                    m_st_time = localtime(&current_time);
                    // get data time not 0 min. current time is 0 min.
                    if ( m_data_st_time.tm_min != 0 )
                        if ( m_st_time->tm_min == 0 )
                            return -1;

                    WriteLogXML(i);
                    if ( m_mi_power_info.Error_Code1 || m_mi_power_info.Error_Code2 || m_inverter_state ||
                        (m_sys_error && (m_data_st_time.tm_hour%2 == 0) && (m_data_st_time.tm_min == 0)) ) {
                        WriteErrorLogXML(i);
                    }
                    dosave = true;

                } else {
                // Hybrid part
                    // clean, all function done, to modify.
                    CleanParameter();

                    if ( arySNobj[i].m_Model < 16 ) { // H5000/5001
                        printf("H5000/5001 series\n");

                        // first set rtc time, and one day set one time
                        if ( m_loopstate == 1 ) {
                            GetHybridBMSVer(i);
                            if ( m_do_set_RTC )
                                SetHybridRTCData(i);
                        } else if ( m_loopstate == 2 ) {
                            if ( m_hb_bms_ver == 0 ) {
                                GetHybridBMSVer(i);
                            } else if ( (m_data_st_time.tm_hour%2 == 0) && (m_data_st_time.tm_min == 0) ) {
                                GetHybridBMSVer(i);
                            }

                            if ( (m_data_st_time.tm_hour == 0) && (m_data_st_time.tm_min == 0) ) {
                                GetHybridBMSVer(i);
                                if ( m_do_set_RTC )
                                    SetHybridRTCData(i);
                            }
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);
                        // get data time not 0 min. current time is 0 min.
                        //if ( m_data_st_time.tm_min != 0 )
                        //    if ( m_st_time->tm_min == 0 )
                        //        return -1;

                        // get power data
                        if ( GetHybridIDData(i) ) {
                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            m_save_hb_id_data = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb_id_data = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);
                        // get data time not 0 min. current time is 0 min.
                        //if ( m_data_st_time.tm_min != 0 )
                        //    if ( m_st_time->tm_min == 0 )
                        //        return -1;

                        // get RTC data
                        if ( GetHybridRTCData(i) ) {
                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            m_save_hb_rtc_data = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb_rtc_data = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);
                        // get data time not 0 min. current time is 0 min.
                        //if ( m_data_st_time.tm_min != 0 )
                        //    if ( m_st_time->tm_min == 0 )
                        //        return -1;

                        if ( GetHybridRSInfo(i) ) {
                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            m_save_hb_rs_info = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb_rs_info = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);
                        // get data time not 0 min. current time is 0 min.
                        //if ( m_data_st_time.tm_min != 0 )
                        //    if ( m_st_time->tm_min == 0 )
                        //        return -1;

                        if ( GetHybridRRSInfo(i) ) {
                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            m_save_hb_rrs_info = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb_rrs_info = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);
                        // get data time not 0 min. current time is 0 min.
                        //if ( m_data_st_time.tm_min != 0 )
                        //    if ( m_st_time->tm_min == 0 )
                        //        return -1;

                        // for test only
                        //GetHybridData(i, START_ADDRESS_ID, COUNT_ID);
                        //GetHybridData(i, START_ADDRESS_RTC, COUNT_RTC);
                        //GetHybridData(i, START_ADDRESS_RSINFO, COUNT_RSINFO);
                        //GetHybridData(i, START_ADDRESS_RRSINFO, COUNT_RRSINFO);
                        //if ( GetHybridRTInfo2(i) ) {
                        if ( GetHybridData(i, START_ADDRESS_RTINFO, COUNT_RTINFO) ) {
                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            m_save_hb_rt_info = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb_rt_info = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);
                        // get data time not 0 min. current time is 0 min.
                        //if ( m_data_st_time.tm_min != 0 )
                        //    if ( m_st_time->tm_min == 0 )
                        //        return -1;

                        SetBMSPath(i);
                        if ( GetHybridBMSInfo(i) ) {
                            // get time
                            //current_time = time(NULL);
                            //m_st_time = localtime(&current_time);
                            // get data time not 0 min. current time is 0 min.
                            //if ( m_data_st_time.tm_min != 0 )
                            //    if ( m_st_time->tm_min == 0 )
                            //        return -1;

                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            //SetHybridBMSModule(i);
                            //SaveBMS();
                            m_save_hb_bms_info = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb_bms_info = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);
                        // get data time not 0 min. current time is 0 min.
                        //if ( m_data_st_time.tm_min != 0 )
                        //    if ( m_st_time->tm_min == 0 )
                        //        return -1;

                        WriteLogXML(i);
                        if ( m_hb_rt_info.Error_Code || m_hb_rt_info.PV_Inv_Error_COD1_Record || m_hb_rt_info.PV_Inv_Error_COD2_Record || m_hb_rt_info.PV_Inv_Error_COD3_Record ||
                             m_hb_rt_info.DD_Error_COD_Record || m_hb_rt_info.DD_Error_COD2_Record || m_inverter_state ||
                            (m_sys_error && (m_data_st_time.tm_hour%2 == 0) && (m_data_st_time.tm_min == 0)) ) {
                            WriteErrorLogXML(i);
                        }
                        dosave = true;
                    } else { // H5500/9600
                        printf("H5500/9600 series\n");

                        //GetHybridData(i, HB2_START_ADDRESS_RTC, HB2_COUNT_RTC);
                        // first set rtc time, and one day set one time
                        if ( m_loopstate == 1 ) {
                            GetHybridBMSVer(i);
                            if ( m_do_set_RTC )
                                SetHybrid2RTCData(i);
                        } else if ( m_loopstate == 2 ) {
                            if ( m_hb_bms_ver == 0 ) {
                                GetHybridBMSVer(i);
                            } else if ( (m_data_st_time.tm_hour%2 == 0) && (m_data_st_time.tm_min == 0) ) {
                                GetHybridBMSVer(i);
                            }

                            if ( (m_data_st_time.tm_hour == 0) && (m_data_st_time.tm_min == 0) ) {
                                GetHybridBMSVer(i);
                                if ( m_do_set_RTC )
                                    SetHybrid2RTCData(i);
                            }
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);

                        // get power data
                        //if ( GetHybrid2IDData(i) ) {
                        if ( GetHybridData(i, HB2_START_ADDRESS_ID, HB2_COUNT_ID) ) {
                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            m_save_hb2_id_data = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb2_id_data = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);

                        if ( GetHybridData(i, HB2_START_ADDRESS_RSINFO, HB2_COUNT_RSINFO) ) {
                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            m_save_hb2_rs_info = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb2_rs_info = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);

                        if ( GetHybridData(i, HB2_START_ADDRESS_RTINFO, HB2_COUNT_RTINFO) ) {
                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            m_save_hb2_rt_info = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb2_rt_info = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);

                        if ( GetHybridData(i, HB2_START_ADDRESS_CEVALUE, HB2_COUNT_CEVALUE) ) {
                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            m_save_hb2_ce_value = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb2_ce_value = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);

                        if ( GetHybridData(i, HB2_START_ADDRESS_DPINFO, HB2_COUNT_DPINFO) ) {
                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            m_save_hb2_dp_info = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb2_dp_info = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);

                        if ( GetHybridData(i, HB2_START_ADDRESS_RTC, HB2_COUNT_RTC) ) {
                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            m_save_hb2_rtc_info = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb2_rtc_info = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);

                        SetBMSPath(i);
                        if ( GetHybridData(i, HB2_START_ADDRESS_BMSINFO, HB2_COUNT_BMSINFO) ) {
                            arySNobj[i].m_Err = 0;
                            arySNobj[i].m_state = 1;
                            //SetHybridBMSModule(i);
                            //SaveBMS();
                            m_save_hb2_bms_info = true;
                        } else {
                            if ( m_loopflag == 0 )
                                arySNobj[i].m_Err++;
                            m_loopflag++;
                            m_save_hb2_bms_info = false;
                        }

                        // get time
                        current_time = time(NULL);
                        m_st_time = localtime(&current_time);

                        WriteLogXML(i);
                        if ( m_hb2_rt_info.Error_Code || m_hb2_rt_info.PV_Inv_Error_COD1_Record || m_hb2_rt_info.PV_Inv_Error_COD2_Record || m_hb2_rt_info.PV_Inv_Error_COD3_Record ||
                             m_hb2_rt_info.DD_Error_COD1_Record || m_hb2_rt_info.DD_Error_COD2_Record || m_inverter_state ||
                            (m_sys_error && (m_data_st_time.tm_hour%2 == 0) && (m_data_st_time.tm_min == 0)) ) {
                            WriteErrorLogXML(i);
                        }
                        dosave = true;
                    }
                }
            } else {
                printf("Addr %d SN %s Error 3 times, call ReRegister() later\n", arySNobj[i].m_Addr, arySNobj[i].m_Sn);
                // set state to offline
                arySNobj[i].m_state = 0; // offline
                //ReRegiser(i);
                if ( current_time - arySNobj[i].m_ok_time >= OFFLINE_SECOND_HB ) {
                    printf("Last m_ok_time more then %d sec.\n", OFFLINE_SECOND_HB);
                    ReRegister(i);
                }
            }
            printf("Debug : index %d, m_Err = %d, m_loopflag = %d\n", i, arySNobj[i].m_Err, m_loopflag);
            m_loopflag = 0;
        }

    }

//    if ( m_plcver == 3 ) {
        if ( stat(m_log_filename, &st) != 0 ) {
            SaveLogXML(first, last);
            SaveErrorLogXML(first, last);
            SaveEnvXML(first, last);
            dosave = true;
        }
//    } else if ( m_plcver == 2 ){
//        if ( (m_loopstate != 0) /*&& (m_snCount > 0)*/ ) {
/*            if ( stat(m_log_filename, &st) != 0 ) {
                SaveLogXML(first, last);
                SaveErrorLogXML(first, last);
                SaveEnvXML(first, last);
                dosave = true;
            }
        } else { // m_loopstate == 0
            //memset(m_log_buf, 0x00, LOG_BUF_SIZE);
            //memset(m_errlog_buf, 0x00, LOG_BUF_SIZE);
            SaveLogXML(first, last);
            SaveErrorLogXML(first, last);
            SaveEnvXML(first, last);
        }
    }*/


    if ( m_loopstate == 0 ) { // init : device get OK
        m_loopstate = 1;
        //if ( m_snCount > 0 ) { // MI or Hybrid checked : save white list
        //    SaveLog((char *)"DataLogger Start() : run WriteMIListXML()", m_st_time);
        //    WriteMIListXML();
        //}
    } else if ( m_loopstate == 1 ) {
        m_loopstate = 2;
        // find new device
        //if ( m_wl_count < m_snCount ) {
        //    SaveLog((char *)"DataLogger Start() : run SaveWhiteList()", m_st_time);
        //    SaveWhiteList();
        //}
    } else { // m_loopstate > 2 to do other thing, undefined
        if ( m_snCount > 0 )
            ;
    }

    if ( m_snCount > 0 ) {
        //SaveLog((char *)"DataLogger GetData() : run WriteMIListXML()", m_st_time);
        WriteMIListXML();
        SaveWhiteList();
    }

    if ( dosave ) {
        SaveDeviceList(first, last);
        dosave = false;
    }

    //CloseLog(); // cancel, move to main loop execute at loop end
    //system("sync");

    // get timezone if before not succss
    if ( m_do_get_TZ )
        GetTimezone();

    return 0;
}

void CG320::Pause()
{

}
void CG320::Play()
{

}
void CG320::Stop()
{

}

void CG320::GetMAC()
{
    FILE *fd = NULL;

    // get MAC address
    fd = popen("uci get network.lan_dev.macaddr", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(g_dlData.g_macaddr, 18, fd);
    pclose(fd);

    printf("MAC = %s\n", g_dlData.g_macaddr);

    return;
}

bool CG320::GetDLConfig()
{
    printf("#### GetDLConfig Start ####\n");

    char buf[32] = {0};
    char cmd[128] = {0};
    FILE *pFile = NULL;

    // get sms_server
    pFile = popen("uci get dlsetting.@sms[0].sms_server", "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(m_dl_config.m_sms_server, 128, pFile);
    pclose(pFile);
    m_dl_config.m_sms_server[strlen(m_dl_config.m_sms_server)-1] = 0; // clean \n
    printf("SMS Server = %s\n", m_dl_config.m_sms_server);
    // get sms server port
    pFile = popen("uci get dlsetting.@sms[0].sms_port", "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &m_dl_config.m_sms_port);
    printf("SMS Port = %d\n", m_dl_config.m_sms_port);

    // get sample_time
    /*pFile = popen("uci get dlsetting.@sms[0].sample_time", "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &m_dl_config.m_sample_time);
    printf("Sample time (Min.) = %d\n", m_dl_config.m_sample_time);*/
    // get MI mi_delay_time
    pFile = popen("uci get dlsetting.@sms[0].delay_time_1", "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &m_dl_config.m_delay_time_1);
    printf("MI delay time (us.) = %d\n", m_dl_config.m_delay_time_1);
    // get Hybrid hb_delay_time
    pFile = popen("uci get dlsetting.@sms[0].delay_time_2", "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &m_dl_config.m_delay_time_2);
    printf("Hybrid delay time (us.) = %d\n", m_dl_config.m_delay_time_2);

    // cancel, set it in init()
    // get serial port
/*    pFile = popen("uci get dlsetting.@comport[0].inverter_port", "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "COM%d", &m_dl_config.m_inverter_port);
    printf("Serial Port = %d\n", m_dl_config.m_inverter_port);*/
    // get baud
    sprintf(cmd, "uci get dlsetting.@comport[0].com%d_baud", m_dl_config.m_inverter_port);
    pFile = popen(cmd, "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &m_dl_config.m_inverter_baud);
    printf("Baud rate = %d\n", m_dl_config.m_inverter_baud);
    // get data bits
    sprintf(cmd, "uci get dlsetting.@comport[0].com%d_data_bits", m_dl_config.m_inverter_port);
    pFile = popen(cmd, "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &m_dl_config.m_inverter_data_bits);
    printf("Data bits = %d\n", m_dl_config.m_inverter_data_bits);
    // get parity
    sprintf(cmd, "uci get dlsetting.@comport[0].com%d_parity", m_dl_config.m_inverter_port);
    pFile = popen(cmd, "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(m_dl_config.m_inverter_parity, 8, pFile);
    pclose(pFile);
    m_dl_config.m_inverter_parity[strlen(m_dl_config.m_inverter_parity)-1] = 0; // clean \n
    printf("Parity = %s\n", m_dl_config.m_inverter_parity);
    // get stop bits
    sprintf(cmd, "uci get dlsetting.@comport[0].com%d_stop_bits", m_dl_config.m_inverter_port);
    pFile = popen(cmd, "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &m_dl_config.m_inverter_stop_bits);
    printf("Stop bits = %d\n", m_dl_config.m_inverter_stop_bits);

    printf("##### GetDLConfig End #####\n");
    return true;
}

bool CG320::SetPath()
{
    //printf("#### SetPath Start ####\n");

    char buf[256] = {0};
    char tmpbuf[256] = {0};
    struct stat st;
    bool mk_tmp_dir = false;

    // set root path (XML & BMS & SYSLOG in the same dir.)
    if ( stat(USB_DEV, &st) == 0 ) { /*linux usb storage detect*/
        strcpy(m_dl_path.m_root_path, USB_PATH); // set usb
        m_sys_error  &= ~SYS_0001_No_USB;
        mk_tmp_dir = true;
    } else if ( stat(SDCARD_PATH, &st) == 0 ) {
        strcpy(m_dl_path.m_root_path, SDCARD_PATH); // set sdcard
        m_sys_error  &= ~SYS_0004_No_SD;
        mk_tmp_dir = true;
    } else {
        strcpy(m_dl_path.m_root_path, DEF_PATH); // set default path
        m_sys_error  |= SYS_0001_No_USB;
        m_sys_error  |= SYS_0004_No_SD;
        mk_tmp_dir = false;
    }

    // set XML path
    sprintf(m_dl_path.m_xml_path, "%s/XML", m_dl_path.m_root_path);
    if ( stat(m_dl_path.m_xml_path, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", m_dl_path.m_xml_path);
        if ( mkdir(m_dl_path.m_xml_path, 0755) == -1 )
            printf("mkdir %s fail!\n", m_dl_path.m_xml_path);
        else
            printf("mkdir %s OK\n", m_dl_path.m_xml_path);
    }

    // set log path
    sprintf(buf, "%s/LOG", m_dl_path.m_xml_path);
    if ( stat(buf, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", buf);
        if ( mkdir(buf, 0755) == -1 )
            printf("mkdir %s fail!\n", buf);
        else
            printf("mkdir %s OK\n", buf);
    }

    // set log date path
    memset(buf, 0, 256);
    sprintf(buf, "%s/LOG/%4d%02d%02d", m_dl_path.m_xml_path, 1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday);
    strcpy(m_dl_path.m_log_path, buf);
    if ( stat(m_dl_path.m_log_path, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", m_dl_path.m_log_path);
        if ( mkdir(m_dl_path.m_log_path, 0755) == -1 )
            printf("mkdir %s fail!\n", m_dl_path.m_log_path);
        else
            printf("mkdir %s OK\n", m_dl_path.m_log_path);
    }

    // set errlog path
    memset(buf, 0, 256);
    sprintf(buf, "%s/ERRLOG", m_dl_path.m_xml_path);
    if ( stat(buf, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", buf);
        if ( mkdir(buf, 0755) == -1 )
            printf("mkdir %s fail!\n", buf);
        else
            printf("mkdir %s OK\n", buf);
    }

    // set errlog date path
    memset(buf, 0, 256);
    sprintf(buf, "%s/ERRLOG/%4d%02d%02d", m_dl_path.m_xml_path, 1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday);
    strcpy(m_dl_path.m_errlog_path, buf);
    if ( stat(m_dl_path.m_errlog_path, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", m_dl_path.m_errlog_path);
        if ( mkdir(m_dl_path.m_errlog_path, 0755) == -1 )
            printf("mkdir %s fail!\n", m_dl_path.m_errlog_path);
        else
            printf("mkdir %s OK\n", m_dl_path.m_errlog_path);
    }

    // set env path
    memset(buf, 0, 256);
    sprintf(buf, "%s/ENV", m_dl_path.m_xml_path);
    if ( stat(buf, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", buf);
        if ( mkdir(buf, 0755) == -1 )
            printf("mkdir %s fail!\n", buf);
        else
            printf("mkdir %s OK\n", buf);
    }

    // set env date path
    memset(buf, 0, 256);
    sprintf(buf, "%s/ENV/%4d%02d%02d", m_dl_path.m_xml_path, 1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday);
    strcpy(m_dl_path.m_env_path, buf);
    if ( stat(m_dl_path.m_env_path, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", m_dl_path.m_env_path);
        if ( mkdir(m_dl_path.m_env_path, 0755) == -1 )
            printf("mkdir %s fail!\n", m_dl_path.m_env_path);
        else
            printf("mkdir %s OK\n", m_dl_path.m_env_path);
    }

    // set BMS path
    sprintf(buf, "%s/BMS", m_dl_path.m_root_path);
    if ( stat(buf, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", buf);
        if ( mkdir(buf, 0755) == -1 )
            printf("mkdir %s fail!\n", buf);
        else
            printf("mkdir %s OK\n", buf);
    }

    // set bms date path
    /*memset(buf, 0, 256);
    sprintf(buf, "%s/BMS/%4d%02d%02d", m_dl_path.m_root_path, 1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday);
    strcpy(m_dl_path.m_bms_path, buf);
    if ( stat(m_dl_path.m_bms_path, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", m_dl_path.m_bms_path);
        if ( mkdir(m_dl_path.m_bms_path, 0755) == -1 )
            printf("mkdir %s fail!\n", m_dl_path.m_bms_path);
        else
            printf("mkdir %s OK\n", m_dl_path.m_bms_path);
    }*/

    // set SYSLOG path
    sprintf(m_dl_path.m_syslog_path, "%s/SYSLOG", m_dl_path.m_root_path);
    if ( stat(m_dl_path.m_syslog_path, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", m_dl_path.m_syslog_path);
        if ( mkdir(m_dl_path.m_syslog_path, 0755) == -1 )
            printf("mkdir %s fail!\n", m_dl_path.m_syslog_path);
        else
            printf("mkdir %s OK\n", m_dl_path.m_syslog_path);
    }

    //printf("m_xml_path = %s\n", m_dl_path.m_xml_path);
    //printf("m_log_path = %s\n", m_dl_path.m_log_path);
    //printf("m_errlog_path = %s\n", m_dl_path.m_errlog_path);
    //printf("m_env_path    = %s\n", m_dl_path.m_env_path);
    //printf("m_bms_path = %s\n", m_dl_path.m_bms_path);
    //printf("m_syslog_path = %s\n", m_dl_path.m_syslog_path);

    if ( mk_tmp_dir ) {
        // create /tmp XML dir
        sprintf(tmpbuf, "%s/XML", DEF_PATH);
        if ( stat(tmpbuf, &st) == -1 ) {
            printf("%s not exist, run mkdir!\n", tmpbuf);
            if ( mkdir(tmpbuf, 0755) == -1 )
                printf("mkdir %s fail!\n", tmpbuf);
            else
                printf("mkdir %s OK\n", tmpbuf);
        }

        // create /tmp LOG dir
        sprintf(tmpbuf, "%s/XML/LOG", DEF_PATH);
        if ( stat(tmpbuf, &st) == -1 ) {
            printf("%s not exist, run mkdir!\n", tmpbuf);
            if ( mkdir(tmpbuf, 0755) == -1 )
                printf("mkdir %s fail!\n", tmpbuf);
            else
                printf("mkdir %s OK\n", tmpbuf);
        }
        // create /tmp LOG date dir
        sprintf(tmpbuf, "%s/XML/LOG/%4d%02d%02d", DEF_PATH, 1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday);
        if ( stat(tmpbuf, &st) == -1 ) {
            printf("%s not exist, run mkdir!\n", tmpbuf);
            if ( mkdir(tmpbuf, 0755) == -1 )
                printf("mkdir %s fail!\n", tmpbuf);
            else
                printf("mkdir %s OK\n", tmpbuf);
        }

        // create /tmp ERRLOG dir
        sprintf(tmpbuf, "%s/XML/ERRLOG", DEF_PATH);
        if ( stat(tmpbuf, &st) == -1 ) {
            printf("%s not exist, run mkdir!\n", tmpbuf);
            if ( mkdir(tmpbuf, 0755) == -1 )
                printf("mkdir %s fail!\n", tmpbuf);
            else
                printf("mkdir %s OK\n", tmpbuf);
        }
        // create /tmp ERRLOG date dir
        sprintf(tmpbuf, "%s/XML/ERRLOG/%4d%02d%02d", DEF_PATH, 1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday);
        if ( stat(tmpbuf, &st) == -1 ) {
            printf("%s not exist, run mkdir!\n", tmpbuf);
            if ( mkdir(tmpbuf, 0755) == -1 )
                printf("mkdir %s fail!\n", tmpbuf);
            else
                printf("mkdir %s OK\n", tmpbuf);
        }

        // create /tmp BMS dir
        sprintf(tmpbuf, "%s/BMS", DEF_PATH);
        if ( stat(tmpbuf, &st) == -1 ) {
            printf("%s not exist, run mkdir!\n", tmpbuf);
            if ( mkdir(tmpbuf, 0755) == -1 )
                printf("mkdir %s fail!\n", tmpbuf);
            else
                printf("mkdir %s OK\n", tmpbuf);
        }
        // create /tmp BMS date dir
        /*sprintf(tmpbuf, "%s/BMS/%4d%02d%02d", DEF_PATH, 1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday);
        if ( stat(tmpbuf, &st) == -1 ) {
            printf("%s not exist, run mkdir!\n", tmpbuf);
            if ( mkdir(tmpbuf, 0755) == -1 )
                printf("mkdir %s fail!\n", tmpbuf);
            else
                printf("mkdir %s OK\n", tmpbuf);
        }*/

        // create /tmp SYSLOG dir
        sprintf(tmpbuf, "%s/SYSLOG", DEF_PATH);
        if ( stat(tmpbuf, &st) == -1 ) {
            printf("%s not exist, run mkdir!\n", tmpbuf);
            if ( mkdir(tmpbuf, 0755) == -1 )
                printf("mkdir %s fail!\n", tmpbuf);
            else
                printf("mkdir %s OK\n", tmpbuf);
        }
    }
    //printf("##### SetPath End #####\n");
    return true;
}

void CG320::CleanParameter()
{
    memset(m_bms_panamod, 0x00, BMS_PANAMOD_SIZE);
    memset(m_bms_buf, 0x00, BMS_MODULE_SIZE);
    memset(m_bms_mainbuf, 0x00, BMS_BUF_SIZE);
    memset(m_bms_filename, 0x00, 128);
    m_mi_id_info = {0};
    m_mi_power_info = {0};
    m_hb_id_data = {0};
    m_hb_id_flags1 = {0};
    m_hb_id_flags2 = {0};
    m_hb_rtc_data = {0};
    m_hb_rs_info = {0};
    m_hb_rrs_info = {0};
    m_hb_rt_info = {0};
    m_hb_bms_info = {0};
    m_hb_pvinv_err_cod1 = {0};
    m_hb_pvinv_err_cod2 = {0};
    m_hb_pvinv_err_cod3 = {0};
    m_hb_dd_err_cod = {0};
    m_hb_dd_err_cod2 = {0};
    m_hb_icon_info = {0};
    m_save_hb_id_data = false;
    m_save_hb_rtc_data = false;
    m_save_hb_rs_info = false;
    m_save_hb_rrs_info = false;
    m_save_hb_rt_info = false;
    m_save_hb_bms_info = false;
    m_inverter_state = 0;
    m_hb2_id_data = {0};
    m_hb2_id_flags = {0};
    m_hb2_rs_info = {0};
    m_hb2_rs_f_flags = {0};
    m_hb2_rs_s_flags = {0};
    m_hb2_rt_info = {0};
    m_hb2_rt_m_flags = {0};
    m_hb2_rt_f_flags = {0};
    m_hb2_pvinv_err_cod1 = {0};
    m_hb2_pvinv_err_cod2 = {0};
    m_hb2_pvinv_err_cod3 = {0};
    m_hb2_dd_err_cod1 = {0};
    m_hb2_dd_err_cod2 = {0};
    m_hb2_ce_value = {0};
    m_hb2_dp_info = {0};
    m_hb2_icon_info = {0};
    m_hb2_rtc_data = {0};
    m_hb2_bms_info = {0};
    m_save_hb2_id_data = false;
    m_save_hb2_rs_info = false;
    m_save_hb2_rt_info = false;
    m_save_hb2_ce_value = false;
    m_save_hb2_dp_info = false;
    m_save_hb2_rtc_info = false;
    m_save_hb2_bms_info = false;

    return;
}

bool CG320::CheckConfig()
{
    //printf("#### CheckConfig start ####\n");

    char buf[32] = {0};
    FILE *fd = NULL;
    int tmp = 0;

    fd = popen("uci get dlsetting.@sms[0].sample_time", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
    } else {
        fgets(buf, 32, fd);
        pclose(fd);

        sscanf(buf, "%d", &tmp);
        //printf("tmp sample_time = %d\n", tmp);

        if ( m_dl_config.m_sample_time == tmp )
            ;//printf("same sample_time\n");
        else
            m_dl_config.m_sample_time = tmp;
    }

    fd = popen("uci get dlsetting.@sms[0].delay_time_1", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
    } else {
        fgets(buf, 32, fd);
        pclose(fd);

        sscanf(buf, "%d", &tmp);
        //printf("tmp delay_time = %d\n", tmp);

        if ( m_dl_config.m_delay_time_1 == tmp )
            ;//printf("same delay\n");
        else
            m_dl_config.m_delay_time_1 = tmp;
    }

    fd = popen("uci get dlsetting.@sms[0].delay_time_2", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
    } else {
        fgets(buf, 32, fd);
        pclose(fd);

        sscanf(buf, "%d", &tmp);
        //printf("tmp delay_time = %d\n", tmp);

        if ( m_dl_config.m_delay_time_2 == tmp )
            ;//printf("same delay\n");
        else
            m_dl_config.m_delay_time_2 = tmp;
    }

    //printf("#### CheckConfig end ####\n");

    return true;
}

bool CG320::RunTODOList()
{
    char buf[128] = {0};
    char data[128] = {0};
    int i = 0, flag = 0;
    FILE *fd = NULL;

    fd = fopen(TODOLIST_PATH, "rb");
    if ( fd == NULL ) {
        //printf("RunTODOList() open %s fail!\n", TODOLIST_PATH);
        return false;
    }
    printf("\n#### RunTODOList start ####\n");
    while ( fgets(buf, 128, fd) != NULL ) {
        if ( strlen(buf) == 0 )
            break;
        sscanf(buf, "%s %d %d %s", m_dl_cmd.m_Sn, &m_dl_cmd.m_addr, &m_dl_cmd.m_count, data);
        printf("Get SN = %s, addr = %d, count = %d, data = %s\n", m_dl_cmd.m_Sn, m_dl_cmd.m_addr, m_dl_cmd.m_count, data);
        for (i = 0; i < m_dl_cmd.m_count; i++) {
            sscanf(data+4*i, "%04X", &m_dl_cmd.m_data[i]);
            printf("data[%d] = %04X\n", i, m_dl_cmd.m_data[i]);
        }

        for (i = 0; i < m_snCount; i++) {
            if ( !strncmp(arySNobj[i].m_Sn, m_dl_cmd.m_Sn, 16) ) {
                // m_state = 1 if online
                if ( arySNobj[i].m_state ) {
                    // check device type
                    if ( arySNobj[i].m_Device >= 0x0A ) {
                        // Hybrid
                        if ( arySNobj[i].m_Model < 16 ) { // H5000/5001
                            printf("RunTODOList H5000/5001 part\n");
                            switch (m_dl_cmd.m_addr)
                            {
                                case 0x01:
                                    m_hb_id_data.Grid_Voltage = m_dl_cmd.m_data[0];
                                    m_hb_id_data.Model = m_dl_cmd.m_data[1] & 0x00FF;
                                    m_hb_id_data.HW_Ver = (m_dl_cmd.m_data[1] >> 8) & 0xFF;
                                    m_hb_id_data.SN_Hi = m_dl_cmd.m_data[2];
                                    m_hb_id_data.SN_Lo = m_dl_cmd.m_data[3];
                                    m_hb_id_data.Year = m_dl_cmd.m_data[4];
                                    m_hb_id_data.Month = m_dl_cmd.m_data[5];
                                    m_hb_id_data.Date = m_dl_cmd.m_data[6];
                                    m_hb_id_data.Inverter_Ver = m_dl_cmd.m_data[7];
                                    m_hb_id_data.DD_Ver = m_dl_cmd.m_data[8];
                                    m_hb_id_data.EEPROM_Ver = m_dl_cmd.m_data[9];
                                    m_hb_id_data.Display_Ver = m_dl_cmd.m_data[10];
                                    m_hb_id_data.Flags1 = m_dl_cmd.m_data[12];
                                    m_hb_id_data.Flags2 = m_dl_cmd.m_data[13];
                                    SaveLog((char *)"DataLogger RunTODOList() : run SetHybridIDData()", m_st_time);
                                    SetHybridIDData(i);
                                    break;
                                case 0x90:
                                    m_hb_rs_info.Mode = m_dl_cmd.m_data[0];
                                    m_hb_rs_info.StarHour = m_dl_cmd.m_data[1];
                                    m_hb_rs_info.StarMin = m_dl_cmd.m_data[2];
                                    m_hb_rs_info.EndHour = m_dl_cmd.m_data[3];
                                    m_hb_rs_info.EndMin = m_dl_cmd.m_data[4];
                                    m_hb_rs_info.MultiModuleSetting = m_dl_cmd.m_data[5];
                                    m_hb_rs_info.BatteryType = m_dl_cmd.m_data[6];
                                    m_hb_rs_info.BatteryCurrent = m_dl_cmd.m_data[7];
                                    m_hb_rs_info.BatteryShutdownVoltage = m_dl_cmd.m_data[8];
                                    m_hb_rs_info.BatteryFloatingVoltage = m_dl_cmd.m_data[9];
                                    m_hb_rs_info.BatteryReservePercentage = m_dl_cmd.m_data[10];
                                    m_hb_rs_info.PeakShavingPower = m_dl_cmd.m_data[11];
                                    m_hb_rs_info.StartFrequency = m_dl_cmd.m_data[12];
                                    m_hb_rs_info.EndFrequency = m_dl_cmd.m_data[13];
                                    m_hb_rs_info.FeedinPower = m_dl_cmd.m_data[14];
                                    SaveLog((char *)"DataLogger RunTODOList() : run SetHybridRSInfo()", m_st_time);
                                    SetHybridRSInfo(i);
                                    break;
                                case 0xA0:
                                    // now not work (read ok, write fail)
                                    m_hb_rrs_info.ChargeSetting = m_dl_cmd.m_data[0];
                                    m_hb_rrs_info.ChargePower = m_dl_cmd.m_data[1];
                                    m_hb_rrs_info.DischargePower = m_dl_cmd.m_data[2];
                                    m_hb_rrs_info.RampRatePercentage = m_dl_cmd.m_data[3];
                                    m_hb_rrs_info.DegreeLeadLag = m_dl_cmd.m_data[4];
                                    m_hb_rrs_info.Volt_VAr = m_dl_cmd.m_data[5];
                                    m_hb_rrs_info.AC_Coupling_Power = m_dl_cmd.m_data[6];
                                    m_hb_rrs_info.SunSpec_Write_All = m_dl_cmd.m_data[7];
                                    m_hb_rrs_info.Remote_Control = m_dl_cmd.m_data[8];
                                    if ( m_hb_rrs_info.Remote_Control & 0x0001 )
                                        flag = 1;
                                    else
                                        flag = 0;
                                    SaveLog((char *)"DataLogger RunTODOList() : run SetHybridRRSInfo()", m_st_time);
                                    SetHybridRRSInfo(i);
                                    if ( flag ) {
                                        printf("sleep 10 sec. wait inverter restart\n");
                                        usleep(10000000);
                                        printf("run ReRegister %d\n", i);
                                        ReRegister(i);
                                    }

                                    break;
                            }
                        } else { // H5500/9600
                            printf("RunTODOList H5500/9600 part\n");
                            switch (m_dl_cmd.m_addr)
                            {
                                case HB2_START_ADDRESS_ID:
                                    m_hb2_id_data.Grid_Voltage = m_dl_cmd.m_data[0];
                                    m_hb2_id_data.Model = m_dl_cmd.m_data[1];
                                    m_hb2_id_data.SN_Hi = m_dl_cmd.m_data[2];
                                    m_hb2_id_data.SN_Lo = m_dl_cmd.m_data[3];
                                    m_hb2_id_data.Year = m_dl_cmd.m_data[4];
                                    m_hb2_id_data.Month = m_dl_cmd.m_data[5];
                                    m_hb2_id_data.Date = m_dl_cmd.m_data[6];
                                    m_hb2_id_data.Inverter_Ver = m_dl_cmd.m_data[7];
                                    m_hb2_id_data.DD_Ver = m_dl_cmd.m_data[8];
                                    m_hb2_id_data.Parameter_Ver = m_dl_cmd.m_data[9];
                                    m_hb2_id_data.Display_Ver = m_dl_cmd.m_data[10];
                                    m_hb2_id_data.HW_Ver = m_dl_cmd.m_data[11];
                                    m_hb2_id_data.Safety_Control = m_dl_cmd.m_data[12];
                                    SaveLog((char *)"DataLogger RunTODOList() : run SetHybrid2IDData()", m_st_time);
                                    SetHybrid2IDData(i);
                                    break;
                                case HB2_START_ADDRESS_RSINFO:
                                    m_hb2_rs_info.Mode = m_dl_cmd.m_data[0];
                                    m_hb2_rs_info.BatteryPolic = m_dl_cmd.m_data[1];
                                    m_hb2_rs_info.MultiModuleSetting = m_dl_cmd.m_data[2];
                                    m_hb2_rs_info.BatteryType = m_dl_cmd.m_data[3];
                                    m_hb2_rs_info.ChargePower = m_dl_cmd.m_data[4];
                                    m_hb2_rs_info.DischargePower = m_dl_cmd.m_data[5];
                                    m_hb2_rs_info.FeedinPower = m_dl_cmd.m_data[6];
                                    m_hb2_rs_info.BatteryPowerRating = m_dl_cmd.m_data[7];
                                    m_hb2_rs_info.MaxBatteryChargingCurrent = m_dl_cmd.m_data[8];
                                    m_hb2_rs_info.BatteryShutdownVoltage = m_dl_cmd.m_data[9];
                                    m_hb2_rs_info.BatteryAbsorptionChargingVoltage = m_dl_cmd.m_data[10];
                                    m_hb2_rs_info.BatteryReservePercentage = m_dl_cmd.m_data[11];
                                    m_hb2_rs_info.GridTiedMaxSOC = m_dl_cmd.m_data[12];
                                    m_hb2_rs_info.PeakShavingPower = m_dl_cmd.m_data[13];
                                    m_hb2_rs_info.StartFrequency = m_dl_cmd.m_data[14];
                                    m_hb2_rs_info.EndFrequency = m_dl_cmd.m_data[15];
                                    SaveLog((char *)"DataLogger RunTODOList() : run SetHybrid2RSInfo1()", m_st_time);
                                    SetHybrid2RSInfo1(i);
                                    break;
                                case HB2_START_ADDRESS_RSINFO2:
                                    m_hb2_rs_info.RampRatePercentage = m_dl_cmd.m_data[0];
                                    m_hb2_rs_info.DegreeLeadLag = m_dl_cmd.m_data[1];
                                    m_hb2_rs_info.PowerFactorControl = m_dl_cmd.m_data[2];
                                    m_hb2_rs_info.TMP_C3 = m_dl_cmd.m_data[3];
                                    m_hb2_rs_info.SpecifiedModbusSlaveID = m_dl_cmd.m_data[4];
                                    m_hb2_rs_info.FunctionControl1 = m_dl_cmd.m_data[5];
                                    m_hb2_rs_info.FunctionControl2 = m_dl_cmd.m_data[6];
                                    m_hb2_rs_info.SunSpecWritable = m_dl_cmd.m_data[7];
                                    m_hb2_rs_info.SoftwareControl = m_dl_cmd.m_data[8];
                                    if ( m_hb2_rs_info.SoftwareControl & 0x0001 )
                                        flag = 1;
                                    else
                                        flag = 0;
                                    SaveLog((char *)"DataLogger RunTODOList() : run SetHybrid2RSInfo2()", m_st_time);
                                    SetHybrid2RSInfo2(i);
                                    if ( flag ) {
                                        printf("sleep 10 sec. wait inverter restart\n");
                                        usleep(10000000);
                                        printf("run ReRegister %d\n", i);
                                        ReRegister(i);
                                    }
                                    break;
                            }
                        }
                    } else {
                        // MI
                    }
                } else {
                    printf("device[%d] %s offline, do nothing!\n", i, arySNobj[i].m_Sn);
                }

                break;
            } else {
                // not match
            }
        }
    }
    fclose(fd);
    remove(TODOLIST_PATH);

    printf("\n#### RunTODOList end ####\n");

    return true;
}

bool CG320::RunWhiteListChanged()
{
    char buf[128] = {0};
    char sn[17] = {0};
    char type[4] = {0};
    unsigned int num[8] = {0};
    unsigned char tmp[8] = {0};
    FILE *fd = NULL;
    int i = 0;
    bool match = false;
    bool save = false;
    struct tm *log_time;
    time_t current_time = 0;

    current_time = time(NULL);
    log_time = localtime(&current_time);

    fd = fopen(WL_CHANGED_PATH, "rb");
    if ( fd == NULL ) {
        //printf("RunWhiteListChanged() open %s fail!\n", WL_CHANGED_PATH);
        return false;
    }
    printf("\n#### RunWhiteListChanged start ####\n");
    while ( fgets(buf, 128, fd) != NULL ) {
        if ( strlen(buf) == 0 )
            break;
        memset(sn, 0x00, 17);
        memset(type, 0x00, 4);
        //sscanf(buf, "%s %s", sn, type);
        //printf("Get SN = %s, TYPE = %s\n", sn, type);
        // get type
        sscanf(buf, "%s", type);
        printf("type = %s\n", type);
        if ( (strstr(type, "ADD") != NULL) || (strstr(type, "DEL") != NULL) ) {
            // ADD or DEL, get sn
        	sscanf(buf+4, "%s", sn);
            printf("sn = %s\n", sn);
        }

        if ( !strcmp(type, "DEL") ) {
            for ( i = 0; i < m_snCount; i++ ) {
                // find match SN
                if ( !strcmp(arySNobj[i].m_Sn, sn) ) {
                    // device off line, mark, online delete too
                    //if ( arySNobj[i].m_state == 0 ) {
                        printf("Clean index %d data\n", i);
                        SendRejoinNetwork();
                        printf("sleep 5 sec.\n");
                        usleep(5000000);
                        SendRejoinNetwork();
                        printf("sleep 5 sec.\n");
                        usleep(5000000);
                        SendRejoinNetwork();
                        printf("sleep 5 sec.\n");
                        usleep(5000000);
                        //RemoveRegisterQuery(m_busfd, arySNobj[i].m_Addr);
                        //usleep(500000); // 0.5s
                        memset(arySNobj[i].m_Sn, 0x00, 17);
                        arySNobj[i].m_Device = -2;
                        arySNobj[i].m_Err = 0;
                        arySNobj[i].m_FWver = 0;
                        arySNobj[i].m_ok_time = 0;
                        //cnt++;
                        // delete SN to PLC box
                        sscanf(sn, "%02X%02X%02X%02X%02X%02X%02X%02X", &num[0], &num[1], &num[2], &num[3], &num[4], &num[5], &num[6], &num[7]);
                        SaveLog((char *)"DataLogger RunWhiteListChanged() : run DeleteWhiteList()", log_time);
                        tmp[0] = (unsigned char)num[0];
                        tmp[1] = (unsigned char)num[1];
                        tmp[2] = (unsigned char)num[2];
                        tmp[3] = (unsigned char)num[3];
                        tmp[4] = (unsigned char)num[4];
                        tmp[5] = (unsigned char)num[5];
                        tmp[6] = (unsigned char)num[6];
                        tmp[7] = (unsigned char)num[7];
                        DeleteWhiteList(1, tmp);
                        usleep(1000000); // 1s
                        save = true;
                    //}
                }
            }
            //m_snCount -= cnt;
            //m_wl_count -= cnt;
            //m_wl_checksum = 0;
        } else if ( !strcmp(type, "ADD") ) {
            match = false;
            for ( i = 0; i < m_snCount; i++ ) {
                // find match SN
                if ( !strcmp(arySNobj[i].m_Sn, sn) ) {
                    match = true;
                    break;
                }
            }
            if ( match ) {
                printf("%s already in list!\n", sn);
                continue;
            }
            else {
                for ( i = 0; i < m_snCount; i++ ) {
                    if ( strlen(arySNobj[i].m_Sn) == 0 )
                        break;
                }

                strcpy(arySNobj[i].m_Sn, sn);
                arySNobj[i].m_Device = -2;
                arySNobj[i].m_Err = 0;
                arySNobj[i].m_state = 0;
                arySNobj[i].m_FWver = 0;
                arySNobj[i].m_ok_time = 0;
                save = true;

                sscanf(sn, "%02X%02X%02X%02X%02X%02X%02X%02X", &num[0], &num[1], &num[2], &num[3], &num[4], &num[5], &num[6], &num[7]);
                // add SN to PLC box
                SaveLog((char *)"DataLogger RunWhiteListChanged() : run AddWhiteList()", log_time);
                tmp[0] = (unsigned char)num[0];
                tmp[1] = (unsigned char)num[1];
                tmp[2] = (unsigned char)num[2];
                tmp[3] = (unsigned char)num[3];
                tmp[4] = (unsigned char)num[4];
                tmp[5] = (unsigned char)num[5];
                tmp[6] = (unsigned char)num[6];
                tmp[7] = (unsigned char)num[7];
                AddWhiteList(1, tmp);
                usleep(1000000); // 1s

                RemoveRegisterQuery(m_busfd, arySNobj[i].m_Addr);
                usleep(500000);  // 0.5s
                if ( MyAssignAddress(m_busfd, tmp, arySNobj[i].m_Addr) )
                {
                    printf("#### MyAssignAddress(%d) OK! ####\n", arySNobj[i].m_Addr);
                    arySNobj[i].m_Device = -1;
                    arySNobj[i].m_state = 1;
                    arySNobj[i].m_ok_time = time(NULL);
                    GetDevice(i);
                }
                else
                    printf("#### MyAssignAddress(%d) fail! ####\n", arySNobj[i].m_Addr);

                if ( i == m_snCount )
                    m_snCount++;
            }
        } else if ( !strcmp(type, "Rejoin") ) {
            printf("run Rejoin\n");
            SaveLog((char *)"DataLogger RunWhiteListChanged() : run RunRejoin()", log_time);
            RunRejoin();
        } else if ( !strcmp(type, "ClearAll") ) {
            printf("run ClearAll\n");
            SaveLog((char *)"DataLogger RunWhiteListChanged() : run RunClearAll()", log_time);
            RunClearAll();
            printf("RunClearAll end, restart this program.\n");
            fclose(fd);
            remove(WL_CHANGED_PATH);
            current_time = time(NULL);
            log_time = localtime(&current_time);
            SaveLog((char *)"DataLogger RunWhiteListChanged() : RunClearAll() end, stop program", log_time);
            CloseLog();
            usleep(1000000); // 1s
            system("sync; sync; killall -9 dlg320.exe; sync; sync");
        }
    }
    fclose(fd);
    remove(WL_CHANGED_PATH);

    if ( save ) {
        SaveWhiteList();
        //SaveDeviceList(); //cancel, do it in GetData() function
        WriteMIListXML();
    }

    printf("\n##### RunWhiteListChanged end #####\n");

    return true;
}

bool CG320::RunRejoin()
{
    printf("#### RunRejoin start ####\n");

    int err = 0;
    byte *lpdata = NULL;
    struct tm *log_time;
    time_t current_time = 0;

    unsigned char cmd[]={0x01, 0x4E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    MakeReadDataCRC(cmd,15);

    MClearRX();
    txsize=15;
    waitAddr = 0x01;
    waitFCode = 0x4E;

    while ( err < 2 ) {
        memcpy(txbuffer, cmd, 15);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_1);

        current_time = time(NULL);
        log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 14, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### RunRejoin OK ####\n");
            SaveLog((char *)"DataLogger RunRejoin() : OK", log_time);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### RunRejoin CRC Error ####\n");
                SaveLog((char *)"DataLogger RunRejoin() : CRC Error", log_time);
            }
            else {
                printf("#### RunRejoin No Response ####\n");
                SaveLog((char *)"DataLogger RunRejoin() : No Response", log_time);
            }
            err++;
        }
    }

    return false;
}

bool CG320::RunClearAll()
{
    printf("#### RunClearAll start ####\n");

    int isbusy = 0;
    // LBD Device Rejoin the Network 0x4A
    SendRejoinNetwork();
    printf("sleep 5 sec.\n");
    usleep(5000000);
    SendRejoinNetwork();
    printf("sleep 5 sec.\n");
    usleep(5000000);
    SendRejoinNetwork();
    printf("sleep 5 sec.\n");
    usleep(5000000);

    // Clear the White-List 0x40
    ClearWhiteList();
    printf("sleep 5 sec.\n");
    usleep(5000000);

    // Control LBS White-List 0x4E
    RunRejoin();
    printf("sleep 15 min.\n");
    usleep(60000000*15); // 60 * 15 sec. = 15 min.

    // check status
    while (1) {
        printf("check PLC status\n");
        isbusy = GetPLCStatus(0);
        if ( isbusy == 0 ) {
            printf("idle, sleep 1 sec.\n");
            usleep(1000000); // 1s
            break;
        } else if ( isbusy == 1 ) {
            printf("busy, sleep 60 sec.\n");
            usleep(60000000); // 60s
        } else {
            printf("suppose busy, sleep 60 sec.\n");
            usleep(60000000); // 60s
            break;
        }
    }

    printf("##### RunClearAll end #####\n");

    return true;
}

bool CG320::SendRejoinNetwork()
{
    printf("#### SendRejoinNetwork start ####\n");

    int err = 0;
    byte *lpdata = NULL;
    struct tm *log_time;
    time_t current_time = 0;

    unsigned char cmd[]={0x01, 0x4A, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00};
    MakeReadDataCRC(cmd,15);

    MClearRX();
    txsize=15;
    waitAddr = 0x01;
    waitFCode = 0x4A;

    while ( err < 2 ) {
        memcpy(txbuffer, cmd, 15);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_1);

        current_time = time(NULL);
        log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 14, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### SendRejoinNetwork OK ####\n");
            SaveLog((char *)"DataLogger SendRejoinNetwork() : OK", log_time);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### SendRejoinNetwork CRC Error ####\n");
                SaveLog((char *)"DataLogger SendRejoinNetwork() : CRC Error", log_time);
            }
            else {
                printf("#### SendRejoinNetwork No Response ####\n");
                SaveLog((char *)"DataLogger SendRejoinNetwork() : No Response", log_time);
            }
            err++;
        }
    }

    return false;
}

int CG320::GetWhiteListCount()
{
    printf("\n#### GetWhiteListCount start ####\n");

    //m_wl_count = 0;
    //m_wl_checksum = 0;

    int err = 0;
    byte *lpdata = NULL;

    unsigned char szWLcount[]={0x01, 0x30, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00};
    MakeReadDataCRC(szWLcount,14);

    MClearRX();
    txsize=14;
    waitAddr = 0x01;
    waitFCode = 0x30;

    while ( err < 2 ) {
        memcpy(txbuffer, szWLcount, 14);
        MStartTX(m_busfd);
        //usleep(100000); // 0.1s

        lpdata = GetRespond(m_busfd, 15, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### GetWhiteListCount OK ####\n");
            SaveLog((char *)"DataLogger GetWhiteListCount() : OK", m_st_time);
            SaveLog((char *)"DataLogger Get PLC version 3.0", m_st_time);
            // get 0x01 0x30 0x0F 0x00 0x00 0x00 0x00 0x00 0x04 countH countL csH csL crcH crcL, so buff+9 is countH
            if ( DumpWhiteListCount(lpdata+9) )
                return m_wl_count;
            else
                return 0;
        } else {
            if ( have_respond == true ) {
                printf("#### GetWhiteListCount CRC Error ####\n");
                SaveLog((char *)"DataLogger GetWhiteListCount() : CRC Error", m_st_time);
            }
            else {
                printf("#### GetWhiteListCount No Response ####\n");
                SaveLog((char *)"DataLogger GetWhiteListCount() : No Response", m_st_time);
            }
            err++;
        }
    }

    return -1;
}

bool CG320::DumpWhiteListCount(unsigned char *buf)
{
    int tmp_count = 0;
    int tmp_checksum = 0;

    tmp_count = (*(buf) << 8) + *(buf+1);
    tmp_checksum = (*(buf+2) << 8) + *(buf+3);

    printf("#### Dump White List count ####\n");
    printf("tmp_count    = %d\n", tmp_count);
    printf("tmp_checksum = 0x%04X\n", tmp_checksum);
    printf("###############################\n");

    if ( tmp_count == 0 )
        return false;

    if ( m_wl_count == 0 ) { // initial
        m_wl_count = tmp_count;
        m_wl_checksum = tmp_checksum;
        printf("set m_wl_count = %d\n", m_wl_count);
        printf("set m_wl_checksum = %04X\n", m_wl_checksum);
        return true;
    } else if ( m_wl_count != tmp_count ) { // count change
        m_wl_count = tmp_count;
        m_wl_checksum = tmp_checksum;
        printf("set m_wl_count = %d\n", m_wl_count);
        printf("set m_wl_checksum = %04X\n", m_wl_checksum);
        return true;
    }

    return false;
}

bool CG320::GetWhiteListSN()
{
    printf("#### GetWhiteListSN start ####\n");

    memset(m_white_list_buf, 0x00, WHITE_LIST_SIZE);

    int i = 0, err = 0, offset = 0, errcnt = 0, j;
    int start_addr = 0;
    int num_of_data = 0;
    int range = 10;
    byte *lpdata = NULL;
    unsigned char szWLSN[]={0x01, 0x31, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};

    time_t current_time;

    for ( i = 0; i < m_wl_count; i+=range ) {
        start_addr = i;
        if ( i+range < m_wl_count )
            num_of_data = range;
        else
            num_of_data = m_wl_count - i;

        szWLSN[8] = (unsigned char)((start_addr >> 8) & 0xFF);
        szWLSN[9] = (unsigned char)(start_addr & 0xFF);
        szWLSN[10] = (unsigned char)((num_of_data >> 8) & 0xFF);
        szWLSN[11] = (unsigned char)(num_of_data & 0xFF);

        MakeReadDataCRC(szWLSN,14);

        MClearRX();
        txsize=14;
        waitAddr = 0x01;
        waitFCode = 0x31;

        while ( err < 3 ) {
            memcpy(txbuffer, szWLSN, 14);
            MStartTX(m_busfd);
            //usleep(1000000); // 1s

            lpdata = GetRespond(m_busfd, 11 + 8*num_of_data, m_dl_config.m_delay_time_1);

            // get time
            current_time = time(NULL);
            m_st_time = localtime(&current_time);
            // get data time not 0 min. current time is 0 min.
            if ( m_data_st_time.tm_min != 0 )
                if ( m_st_time->tm_min == 0 )
                    return false;

            if ( lpdata ) {
                printf("#### GetWhiteListSN index %d, data %d OK ####\n", i, num_of_data);
                printf("copy %d data to white list buf\n", 8*num_of_data);
                memcpy(m_white_list_buf + offset, lpdata+9, 8*num_of_data);
                printf("=================== white list buf ===================");
                for ( j = 0; j < 8*num_of_data; j++ ) {
                    if ( j%8 == 0 )
                        printf("\nSN %d : ", i + j/8 + 1);
                    printf("0x%02X ", m_white_list_buf[j + offset]);
                }
                printf("\n======================================================\n");
                offset += 8*num_of_data;
                printf("offset = %d\n", offset);
                err = 0;
                break;
            } else {
                if ( have_respond == true )
                    printf("#### GetWhiteListSN %d CRC Error ####\n", i);
                else
                    printf("#### GetWhiteListSN %d No Response ####\n", i);
                err++;
            }
        }
        if ( err == 3 )
            errcnt++;
        err = 0;
    }

    //if ( !errcnt ) {
    if ( DumpWhiteListSN() ) {
        printf("#### GetWhiteListSN OK ####\n");
        SaveLog((char *)"DataLogger GetWhiteListSN() : OK", m_st_time);
        return true;
    } else {
        printf("#### GetWhiteListSN Check Sum Error ####\n");
        SaveLog((char *)"DataLogger GetWhiteListSN() : Check Sum Error", m_st_time);
        return false;
    }
}

bool CG320::DumpWhiteListSN()
{
    int checksum = 0, i = 0;
    char buf[256] = {0};

    printf("############# Dump White List SN #############\n");
    printf("m_wl_count = %d\n", m_wl_count);
    for (i = 0; i < m_wl_count; i++) {
        printf("SN %3d = 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n", i+1,
               m_white_list_buf[0 + i*8], m_white_list_buf[1 + i*8], m_white_list_buf[2 + i*8], m_white_list_buf[3 + i*8],
               m_white_list_buf[4 + i*8], m_white_list_buf[5 + i*8], m_white_list_buf[6 + i*8], m_white_list_buf[7 + i*8]);
        sprintf(buf, "DataLogger DumpWhiteListSN() : SN %3d = 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X", i+1,
               m_white_list_buf[0 + i*8], m_white_list_buf[1 + i*8], m_white_list_buf[2 + i*8], m_white_list_buf[3 + i*8],
               m_white_list_buf[4 + i*8], m_white_list_buf[5 + i*8], m_white_list_buf[6 + i*8], m_white_list_buf[7 + i*8]);
        SaveLog(buf, m_st_time);
    }
    for (i = 0; i < 8*m_wl_count; i++)
        checksum += m_white_list_buf[i];
    printf("m_wl_checksum = 0x%04X, checksum = 0x%04X\n", m_wl_checksum, checksum);
    printf("##############################################\n");

    if ( m_wl_checksum == checksum )
        return true;
    else
        return false;
}

bool CG320::ClearWhiteList()
{
    printf("#### ClearWhiteList start ####\n");

    int err = 0;
    byte *lpdata = NULL;
    struct tm *log_time;
    time_t current_time = 0;

    unsigned char szClearWL[]={0x01, 0x40, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    MakeReadDataCRC(szClearWL,15);

    MClearRX();
    txsize=15;
    waitAddr = 0x01;
    waitFCode = 0x40;

    while ( err < 2 ) {
        memcpy(txbuffer, szClearWL, 15);
        MStartTX(m_busfd);
        //usleep(1000000); // 1s

        current_time = time(NULL);
        log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 14, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### ClearWhiteList OK ####\n");
            SaveLog((char *)"DataLogger ClearWhiteList() : OK", log_time);
            // get 0x01 0x40 0x0E 0x00 0x00 0x00 0x00 0x00 0xFF 0x00 0x00 0x00 crcH crcL
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### ClearWhiteList CRC Error ####\n");
                SaveLog((char *)"DataLogger ClearWhiteList() : CRC Error", log_time);
            }
            else {
                printf("#### ClearWhiteList No Response ####\n");
                SaveLog((char *)"DataLogger ClearWhiteList() : No Response", log_time);
            }
            err++;
        }
    }

    return false;
}

bool CG320::WriteWhiteList(int num, unsigned char *listbuf)
{
    printf("#### WriteWhiteList start ####\n");

    int i = 0, err = 0, offset = 0, errcnt = 0;
    int start_addr = 0;
    int num_of_data = 0;
    int range = 10;
    byte *lpdata = NULL;
    unsigned char szWriteWL[256]={0};

    szWriteWL[0] = 0x01;
    szWriteWL[1] = 0x41;
    szWriteWL[3] = 0x00;
    szWriteWL[4] = 0x00;
    szWriteWL[5] = 0x00;
    szWriteWL[6] = 0x00;
    szWriteWL[7] = 0x00;

    for ( i = 0; i < num; i+=range ) {
        if ( i+range < num )
            num_of_data = range;
        else
            num_of_data = num - i;

        szWriteWL[8] = (unsigned char)((start_addr >> 8) & 0xFF);
        szWriteWL[9] = (unsigned char)(start_addr & 0xFF);
        szWriteWL[10] = (unsigned char)((num_of_data >> 8) & 0xFF);
        szWriteWL[11] = (unsigned char)(num_of_data & 0xFF);
        szWriteWL[12] = (unsigned char)(num_of_data*8);
        szWriteWL[2] = szWriteWL[12] + 0x0F;
        memcpy(szWriteWL+13, listbuf+offset, num_of_data*8);
        offset += num_of_data*8;
        MakeReadDataCRC(szWriteWL, szWriteWL[2]);

        MClearRX();
        txsize = szWriteWL[2];
        waitAddr = 0x01;
        waitFCode = 0x41;

        while ( err < 3 ) {
            memcpy(txbuffer, szWriteWL, szWriteWL[2]);
            MStartTX(m_busfd);
            usleep(1000000); // 1s

            if ( err == 0 )
                lpdata = GetRespond(m_busfd, 14, 100000); // 0.1s
            else if ( err == 1 )
                lpdata = GetRespond(m_busfd, 14, 500000); // 0.5s
            else
                lpdata = GetRespond(m_busfd, 14, 1000000); // 1s
            if ( lpdata ) {
                printf("#### WriteWhiteList index %d, data %d OK ####\n", i, num_of_data);
                SaveLog((char *)"DataLogger WriteWhiteList() : OK", m_st_time);
                // get 0x01 0x41 0x0E 0x00 0x00 0x00 0x00 0x00 0x?? 0x?? 0x?? 0x?? crcH crcL
                start_addr += range;
                err = 0;
                break;
            } else {
                if ( have_respond == true ) {
                    printf("#### WriteWhiteList CRC Error ####\n");
                    SaveLog((char *)"DataLogger WriteWhiteList() : CRC Error", m_st_time);
                }
                else{
                    printf("#### WriteWhiteList No Response ####\n");
                    SaveLog((char *)"DataLogger WriteWhiteList() : No Response", m_st_time);
                }
                err++;
            }
        }
        if ( err == 3 )
            errcnt++;
        err = 0;
    }

    if ( errcnt ) {
        printf("#### WriteWhiteList Error ####\n");
        return false;
    } else {
        printf("#### WriteWhiteList OK ####\n");
        return true;
    }
}

bool CG320::AddWhiteList(int num, unsigned char *listbuf)
{
    printf("#### AddWhiteList start ####\n");

    int i = 0, err = 0, offset = 0, errcnt = 0;
    int num_of_data = 0;
    int range = 10;
    byte *lpdata = NULL;
    unsigned char szAddWL[256]={0};

    szAddWL[0] = 0x01;
    szAddWL[1] = 0x42;
    szAddWL[3] = 0x00;
    szAddWL[4] = 0x00;
    szAddWL[5] = 0x00;
    szAddWL[6] = 0x00;
    szAddWL[7] = 0x00;
    szAddWL[8] = 0x00;
    szAddWL[9] = 0x00;

    for ( i = 0; i < num; i+=range ) {
        if ( i+range < num )
            num_of_data = range;
        else
            num_of_data = num - i;

        szAddWL[10] = (unsigned char)((num_of_data >> 8) & 0xFF);
        szAddWL[11] = (unsigned char)(num_of_data & 0xFF);
        szAddWL[12] = (unsigned char)(num_of_data*8);
        szAddWL[2] = szAddWL[12] + 0x0F;
        memcpy(szAddWL+13, listbuf+offset, num_of_data*8);
        offset += num_of_data*8;
        MakeReadDataCRC(szAddWL, szAddWL[2]);

        MClearRX();
        txsize = szAddWL[2];
        waitAddr = 0x01;
        waitFCode = 0x42;

        while ( err < 3 ) {
            memcpy(txbuffer, szAddWL, szAddWL[2]);
            MStartTX(m_busfd);
            usleep(1000000); // 1s

            if ( err == 0 )
                lpdata = GetRespond(m_busfd, 14, 100000); // 0.1s
            else if ( err == 1 )
                lpdata = GetRespond(m_busfd, 14, 500000); // 0.5s
            else
                lpdata = GetRespond(m_busfd, 14, 1000000); // 1s
            if ( lpdata ) {
                printf("#### AddWhiteList index %d, data %d OK ####\n", i, num_of_data);
                SaveLog((char *)"DataLogger AddWhiteList() : OK", m_st_time);
                // get 0x01 0x42 0x0E 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x?? 0x?? crcH crcL
                err = 0;
                break;
            } else {
                if ( have_respond == true ) {
                    printf("#### AddWhiteList CRC Error ####\n");
                    SaveLog((char *)"DataLogger AddWhiteList() : CRC Error", m_st_time);
                }
                else {
                    printf("#### AddWhiteList No Response ####\n");
                    SaveLog((char *)"DataLogger AddWhiteList() : No Response", m_st_time);
                }
                err++;
            }
        }
        if ( err == 3 )
            errcnt++;
        err = 0;
    }

    if ( errcnt ) {
        printf("#### AddWhiteList Error ####\n");
        return false;
    } else {
        printf("#### AddWhiteList OK ####\n");
        return true;
    }
}

bool CG320::LoadWhiteList()
{
    printf("\n#### LoadWhiteList start ####\n");

    FILE *pFile;
    char buf[256] = {0};
    int num = 0;
    unsigned int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0, tmp6 = 0, tmp7 = 0, tmp8 = 0;

    m_wl_count = 0;
    m_wl_checksum = 0;
    m_wl_maxid = 0;

    pFile = fopen(WHITE_LIST_PATH, "r");
    if ( pFile == NULL ) {
        printf("#### LoadWhiteList open file Fail ####\n");
        SaveLog((char *)"DataLogger LoadWhiteList() : fopen white list fail", m_st_time);
        return false;
    }

    fgets(buf, 256, pFile);
    sscanf(buf, "count=%d", &m_wl_count);
    fgets(buf, 256, pFile);
    sscanf(buf, "checksum=%X", &m_wl_checksum);
    fgets(buf, 256, pFile);
    sscanf(buf, "maxid=%d", &m_wl_maxid);

    printf("Get : m_wl_count = 0x%02X, m_wl_checksum = 0x%04X, m_wl_maxid = %d\n", m_wl_count, m_wl_checksum, m_wl_maxid);
    sprintf(buf, "DataLogger LoadWhiteList() : m_wl_count = 0x%02X, m_wl_checksum = 0x%04X, m_wl_maxid = %d", m_wl_count, m_wl_checksum, m_wl_maxid);
    SaveLog(buf, m_st_time);

    for (num = 0; num < m_wl_count; num++) {
        fgets(buf, 256, pFile);
        sscanf(buf, "%02X%02X%02X%02X%02X%02X%02X%02X %08X %08X %010ld", &tmp1, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6, &tmp7, &tmp8, &arySNobj[num].m_Device, &arySNobj[num].m_Model, &arySNobj[num].m_ok_time);
        m_white_list_buf[0 + num*8] = (unsigned char)tmp1;
        m_white_list_buf[1 + num*8] = (unsigned char)tmp2;
        m_white_list_buf[2 + num*8] = (unsigned char)tmp3;
        m_white_list_buf[3 + num*8] = (unsigned char)tmp4;
        m_white_list_buf[4 + num*8] = (unsigned char)tmp5;
        m_white_list_buf[5 + num*8] = (unsigned char)tmp6;
        m_white_list_buf[6 + num*8] = (unsigned char)tmp7;
        m_white_list_buf[7 + num*8] = (unsigned char)tmp8;
        // debug info.
        printf("[%d] Model = 0x%02X\n", num, arySNobj[num].m_Model);
    }
    fclose(pFile);

    if ( DumpWhiteListSN() ) {
        printf("#### LoadWhiteList OK ####\n");
        SaveLog((char *)"DataLogger LoadWhiteList() : OK", m_st_time);
        return true;
    } else {
        printf("#### LoadWhiteList Check Sum Error ####\n");
        SaveLog((char *)"DataLogger LoadWhiteList() : Check Sum Error", m_st_time);
        return false;
    }
}

bool CG320::LoadWhiteListV3()
{
    printf("\n#### LoadWhiteListV3 start ####\n");

    FILE *pFile;
    char buf[256] = {0}, sntmp[17] = {0};
    int num = 0, devtmp = 0, i = 0;
    time_t oktmp = 0;

    m_wl_count = 0;
    m_wl_checksum = 0;
    m_wl_maxid = 0;

    pFile = fopen(WHITE_LIST_V3_PATH, "r");
    if ( pFile == NULL ) {
        printf("#### LoadWhiteListV3 open file Fail ####\n");
        SaveLog((char *)"DataLogger LoadWhiteListV3() : fopen white list fail", m_st_time);
        return false;
    }

    fgets(buf, 256, pFile);
    sscanf(buf, "count=%d", &m_wl_count);
    fgets(buf, 256, pFile);
    sscanf(buf, "checksum=%X", &m_wl_checksum);
    fgets(buf, 256, pFile);
    sscanf(buf, "maxid=%d", &m_wl_maxid);

    printf("Get : m_wl_count = 0x%02X, m_wl_checksum = 0x%04X, m_wl_maxid = %d\n", m_wl_count, m_wl_checksum, m_wl_maxid);
    sprintf(buf, "DataLogger LoadWhiteListV3() : m_wl_count = 0x%02X, m_wl_checksum = 0x%04X, m_wl_maxid = %d", m_wl_count, m_wl_checksum, m_wl_maxid);
    SaveLog(buf, m_st_time);

    for (num = 0; num < m_wl_count; num++) {
        fgets(buf, 256, pFile);
        sscanf(buf, "%16s %08X %010ld", sntmp, &devtmp, &oktmp);
        // check SN in loop list
        for (i = 0; i < m_snCount; i++) {
            if ( !strncmp(sntmp, arySNobj[i].m_Sn, 16) ) {
                // match SN, set m_Device & m_ok_time
                arySNobj[i].m_Device = devtmp;
                arySNobj[i].m_ok_time = oktmp;
                break;
            }
        }
    }
    fclose(pFile);

    printf("#### Dump White List V3 ####\n");
    for (i = 0; i < m_snCount; i++) {
        printf("Index %d, SN = %s, device = %08X, ok_time = %010ld\n", arySNobj[i].m_Addr, arySNobj[i].m_Sn, arySNobj[i].m_Device, arySNobj[i].m_ok_time);
    }
    printf("############################\n");

    printf("#### LoadWhiteListV3 OK ####\n");
    SaveLog((char *)"DataLogger LoadWhiteListV3() : OK", m_st_time);
    return true;
}

bool CG320::SavePLCWhiteList()
{
    printf("#### SavePLCWhiteList start ####\n");

    FILE *pFile;
    int num = 0;
    char buf[32] = {0};

    pFile = fopen(WHITE_LIST_PATH, "w");
    if ( pFile == NULL ) {
        printf("#### SavePLCWhiteList open file Fail ####\n");
        SaveLog((char *)"DataLogger SavePLCWhiteList() : fopen fail", m_st_time);
        return false;
    }

    sprintf(buf, "count=%d\n", m_wl_count);
    fputs(buf, pFile);
    sprintf(buf, "checksum=0x%04X\n", m_wl_checksum);
    fputs(buf, pFile);
    sprintf(buf, "maxid=%d\n", m_wl_maxid);
    fputs(buf, pFile);

    for (num = 0; num < m_wl_count; num++) {
        sprintf(buf, "%02X%02X%02X%02X%02X%02X%02X%02X\n",
                m_white_list_buf[0+num*8], m_white_list_buf[1+num*8], m_white_list_buf[2+num*8], m_white_list_buf[3+num*8],
                m_white_list_buf[4+num*8], m_white_list_buf[5+num*8], m_white_list_buf[6+num*8], m_white_list_buf[7+num*8]);
        fputs(buf, pFile);
    }
    fclose(pFile);

    printf("#### SavePLCWhiteList OK ####\n");
    SaveLog((char *)"DataLogger SavePLCWhiteList() : OK", m_st_time);

    return true;
}

bool CG320::SaveWhiteList()
{
    FILE *pFile;
    int i = 0, checksum = 0, cnt = 0;
    char buf[256] = {0};
    unsigned int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0, tmp6 = 0, tmp7 = 0, tmp8 = 0;

    if ( m_plcver == 3 )
        pFile = fopen(WHITE_LIST_V3_PATH, "w");
    else if ( m_plcver == 2 )
        pFile = fopen(WHITE_LIST_PATH, "w");
    if ( pFile == NULL ) {
        printf("#### SaveWhiteList open file Fail ####\n");
        SaveLog((char *)"DataLogger SaveWhiteList() : fopen fail", m_st_time);
        return false;
    }

    for ( i = 0; i < m_snCount; i++) {
        if ( strlen(arySNobj[i].m_Sn) )
            cnt++;
    }
    sprintf(buf, "count=%d\n", cnt);
    fputs(buf, pFile);

    for ( i = 0; i < m_snCount; i++) {
        if ( strlen(arySNobj[i].m_Sn) ) {
            sscanf(arySNobj[i].m_Sn, "%02X%02X%02X%02X%02X%02X%02X%02X", &tmp1, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6, &tmp7, &tmp8);
            checksum += (tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8);
        }
    }
    sprintf(buf, "checksum=0x%04X\n", checksum);
    fputs(buf, pFile);
    sprintf(buf, "maxid=%d\n", m_wl_maxid);
    fputs(buf, pFile);

    for ( i = 0; i < m_snCount; i++) {
        if ( strlen(arySNobj[i].m_Sn) ) {
            sprintf(buf, "%s %08X %08X %010ld\n", arySNobj[i].m_Sn, arySNobj[i].m_Device, arySNobj[i].m_Model, arySNobj[i].m_ok_time);
            fputs(buf, pFile);
        }
    }
    fclose(pFile);

    printf("#### SaveWhiteList OK ####\n");

    //SaveLog((char *)"DataLogger SaveWhiteList() : OK", m_st_time);

    return true;
}

bool CG320::SaveDeviceList(bool first, bool last)
{
    FILE *pFile;
    char buf[LOG_BUF_SIZE] = {0}, device[32] = {0};
    int i = 0, offset = 0, model = 0;
    char *index_tmp = NULL, *index_start = NULL, *index_end = NULL;

    printf("#### SaveDeviceList Start ####\n");

    if ( first )
        pFile = fopen(DEVICELIST_TMP, "w");
    else
        pFile = fopen(DEVICELIST_TMP, "a");
    if ( pFile == NULL ) {
        printf("#### SaveDeviceList open file Fail ####\n");
        SaveLog((char *)"DataLogger SaveDeviceList() : fopen fail", m_st_time);
        return false;
    }

    for ( i = 0; i < m_snCount; i++) {
        if ( strlen(arySNobj[i].m_Sn) ) {
            if ( arySNobj[i].m_Device < 0 )
                strcpy(device, "Darfon_unknown"); // unknown
            else if ( arySNobj[i].m_Device < 0x0A )
                strcpy(device, "Darfon-MI"); // MI
            else
                strcpy(device, "Darfon-Hybrid"); // Hybrid
            // addr fixed 3 digit, state fixed 1 digit, sn
            sprintf(buf, "%03d <SN>%s</SN> <STATE>%d</STATE> <DEVICE>%s</DEVICE> ", arySNobj[i].m_Addr, arySNobj[i].m_Sn, arySNobj[i].m_state, device);

            if (arySNobj[i].m_state) {
                // set log data
                index_tmp = strstr(m_log_buf, arySNobj[i].m_Sn+1);
                if ( index_tmp != NULL ) {
                    if ( index_tmp - m_log_buf > 80)
                        index_start = strstr(index_tmp-80, "<record dev_id=");
                    else
                        index_start = strstr(m_log_buf, "<record dev_id=");

                    index_end = strstr(index_start, "</record>");
                    offset = index_end - index_start + 9;
                    strcat(buf, "<ENERGY>");
                    strncat(buf, index_start, offset);

                    if ( arySNobj[i].m_Device < 0x0A ) {
                        sscanf(arySNobj[i].m_Sn+4, "%04X", &model); // get 5-8 digit from SN
                        if ( model >= 3 ) { // G640, put B part
                            index_tmp = strstr(index_end, arySNobj[i].m_Sn+1);
                            if ( index_tmp != NULL ) {
                                index_start = strstr(index_tmp-80, "<record dev_id=");
                                index_end = strstr(index_start, "</record>");
                                offset = index_end - index_start + 9;
                                strncat(buf, index_start, offset);
                            }
                        }
                    }
                    strcat(buf, "</ENERGY>");

                } else {
                    printf("%d log index_tmp = NULL!\n", arySNobj[i].m_Addr);
                    strcat(buf, "Empty");
                }

                // set error log data
                index_tmp = strstr(m_errlog_buf, arySNobj[i].m_Sn);
                if ( index_tmp != NULL ) {
                    if ( index_tmp - m_errlog_buf > 80)
                        index_start = strstr(index_tmp-80, "<record dev_id=");
                    else
                        index_start = strstr(m_errlog_buf, "<record dev_id=");

                    index_end = strstr(index_start, "</record>");
                    offset = index_end - index_start + 9;
                    strcat(buf, "<ERROR>");
                    strncat(buf, index_start, offset);

                    if ( arySNobj[i].m_Device < 0x0A ) {
                        sscanf(arySNobj[i].m_Sn+4, "%04X", &model); // get 5-8 digit from SN
                        if ( model >= 3 ) { // G640, put B part
                            index_tmp = strstr(index_end, arySNobj[i].m_Sn);
                            if ( index_tmp != NULL ) {
                                index_start = strstr(index_tmp-80, "<record dev_id=");
                                index_end = strstr(index_start, "</record>");
                                offset = index_end - index_start + 9;
                                strncat(buf, index_start, offset);
                            }
                        }
                    }
                    strcat(buf, "</ERROR>");

                } else {
                    printf("%d errlog index_tmp = NULL!\n", arySNobj[i].m_Addr);
                    strcat(buf, "Empty");
                }
            } else
                strcat(buf, "Empty Empty");

            fputs(buf, pFile);
            fputc('\n', pFile);
        }
    }
    fclose(pFile);

    if ( last ) {
        sprintf(buf, "cp -f %s %s", DEVICELIST_TMP, DEVICELIST_PATH);
        system(buf);
        //system("sync");
    }

    printf("#### SaveDeviceList OK ####\n");
    //SaveLog((char *)"DataLogger SaveDeviceList() : OK", m_st_time);

    return true;
}

bool CG320::DeleteWhiteList(int num, unsigned char *listbuf)
{
    printf("#### DeleteWhiteList start ####\n");

    int i = 0, err = 0, offset = 0, errcnt = 0;
    int num_of_data = 0;
    int range = 10;
    byte *lpdata = NULL;
    unsigned char szDeleteWL[256]={0};

    szDeleteWL[0] = 0x01;
    szDeleteWL[1] = 0x43;
    szDeleteWL[3] = 0x00;
    szDeleteWL[4] = 0x00;
    szDeleteWL[5] = 0x00;
    szDeleteWL[6] = 0x00;
    szDeleteWL[7] = 0x00;
    szDeleteWL[8] = 0x00;
    szDeleteWL[9] = 0x00;

    for ( i = 0; i < num; i+=range ) {
        if ( i+range < num )
            num_of_data = range;
        else
            num_of_data = num - i;

        szDeleteWL[10] = (unsigned char)((num_of_data >> 8) & 0xFF);
        szDeleteWL[11] = (unsigned char)(num_of_data & 0xFF);
        szDeleteWL[12] = (unsigned char)(num_of_data*8);
        szDeleteWL[2] = szDeleteWL[12] + 0x0F;
        memcpy(szDeleteWL+13, listbuf+offset, num_of_data*8);
        offset += num_of_data*8;
        MakeReadDataCRC(szDeleteWL, szDeleteWL[2]);

        MClearRX();
        txsize = szDeleteWL[2];
        waitAddr = 0x01;
        waitFCode = 0x43;

        while ( err < 3 ) {
            memcpy(txbuffer, szDeleteWL, szDeleteWL[2]);
            MStartTX(m_busfd);
            usleep(1000000); // 1s

            if ( err == 0 )
                lpdata = GetRespond(m_busfd, 14, 100000); // 0.1s
            else if ( err == 1 )
                lpdata = GetRespond(m_busfd, 14, 500000); // 0.5s
            else
                lpdata = GetRespond(m_busfd, 14, 1000000); // 1s
            if ( lpdata ) {
                printf("#### DeleteWhiteList index %d, data %d OK ####\n", i, num_of_data);
                SaveLog((char *)"DataLogger DeleteWhiteList() : OK", m_st_time);
                // get 0x01 0x43 0x0E 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x?? 0x?? crcH crcL
                err = 0;
                break;
            } else {
                if ( have_respond == true ) {
                    printf("#### DeleteWhiteList CRC Error ####\n");
                    SaveLog((char *)"DataLogger DeleteWhiteList() : CRC Error", m_st_time);
                }
                else {
                    printf("#### DeleteWhiteList No Response ####\n");
                    SaveLog((char *)"DataLogger DeleteWhiteList() : No Response", m_st_time);
                }
                err++;
            }
        }
        if ( err == 3 )
            errcnt++;
        err = 0;
    }

    if ( errcnt ) {
        printf("#### DeleteWhiteList Error ####\n");
        return false;
    } else {
        printf("#### DeleteWhiteList OK ####\n");
        return true;
    }
}

int CG320::GetPLCStatus(int index)
{
    printf("#### GetPLCStatus start ####\n");

    int err = 0;
    byte *lpdata = NULL;
    char strbuf[1024] = {0};
    time_t current_time;
    struct tm *log_time;

    unsigned char szPLCStatus[]={0x01, 0x32, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00};
    MakeReadDataCRC(szPLCStatus,14);

    MClearRX();
    txsize=14;
    waitAddr = 0x01;
    waitFCode = 0x32;

    while ( err < 2 ) {
        memcpy(txbuffer, szPLCStatus, 14);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_1);

        current_time = time(NULL);
        log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 21, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### GetPLCStatus(%d) OK : Address = %d, SN = %s ####\n", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
            //SaveLog((char *)"DataLogger GetPLCStatus() : OK", log_time);
            sprintf(strbuf, "DataLogger GetPLCStatus(%d) OK : Address = %d, SN = %s", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
            SaveLog(strbuf, log_time);
            //DumpPLCStatus(lpdata+9); if need, then add this function
            if ( *(lpdata+9) & 0x20 ) { // check the bit5 ( 7 ~ 0 ) in Data1(Hi) byte
                // busy
                printf("#### Busy! ####\n");
                SaveLog((char *)"DataLogger GetPLCStatus() : Busy!", log_time);
                return 1;
            } else {
                // idle
                printf("#### Idle ####\n");
                SaveLog((char *)"DataLogger GetPLCStatus() : Idle", log_time);
                return 0;
            }
        } else {
            if ( have_respond == true ) {
                printf("#### GetPLCStatus(%d) CRC Error : Address = %d, SN = %s ####\n", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
                //SaveLog((char *)"DataLogger GetPLCStatus() : CRC Error", log_time);
                sprintf(strbuf, "DataLogger GetPLCStatus(%d) CRC Error : Address = %d, SN = %s", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
                SaveLog(strbuf, log_time);
            }
            else {
                printf("#### GetPLCStatus(%d) No Response : Address = %d, SN = %s ####\n", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
                //SaveLog((char *)"DataLogger GetPLCStatus() : No Response", log_time);
                sprintf(strbuf, "DataLogger GetPLCStatus(%d) No Response : Address = %d, SN = %s", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
                SaveLog(strbuf, log_time);
            }
            err++;
        }
    }

    printf("#### Suppose Busy ####\n");
    SaveLog((char *)"DataLogger GetPLCStatus() : Suppose Busy", log_time);

    return 2;
}

int CG320::WhiteListRegister()
{
    char buf[256] = {0};

    printf("#### WhiteListRegister start ####\n");

    int i = 0, ok = 0;

    /*if (m_snCount==0) {
        for (i=0; i<253; i++) {
            arySNobj[i].m_Addr=i+1; // address range 1 ~ 253
            memset(arySNobj[i].m_Sn, 0x00, 17);
            arySNobj[i].m_Device = -2;
            arySNobj[i].m_Err = 0;
            //printf("i=%u, addr=%d\n",i,arySNobj[i].m_Addr );
        }
	}*/

	if (m_snCount==0) {
        RemoveRegisterQuery(m_busfd, 0);
        CleanRespond(m_busfd);
        usleep(500000);
        RemoveRegisterQuery(m_busfd, 0);
        CleanRespond(m_busfd);
        usleep(500000);
        RemoveRegisterQuery(m_busfd, 0);
        CleanRespond(m_busfd);
        usleep(500000);
	}

    for ( i = 0; i < m_wl_count; i++) {
        printf("\n#### arySNobj[%d] parameter ####\n", i);
        printf("m_snCount = %d\n", m_snCount);
        printf("m_Addr = %d\n", arySNobj[m_snCount].m_Addr);
        sprintf(arySNobj[m_snCount].m_Sn, "%02X%02X%02X%02X%02X%02X%02X%02X",
                m_white_list_buf[0+i*8], m_white_list_buf[1+i*8], m_white_list_buf[2+i*8], m_white_list_buf[3+i*8],
                m_white_list_buf[4+i*8], m_white_list_buf[5+i*8], m_white_list_buf[6+i*8], m_white_list_buf[7+i*8]);
        printf("m_Sn = %s\n", arySNobj[m_snCount].m_Sn);
        printf("################################\n");

        if ( MyAssignAddress(m_busfd, m_white_list_buf + i*8,  arySNobj[m_snCount].m_Addr) )
        {
            printf("=================================\n");
            printf("#### MyAssignAddress(%d) OK! ####\n", arySNobj[m_snCount].m_Addr);
            printf("=================================\n");
            //arySNobj[m_snCount].m_Device = -1; // set it when call LoadWhiteList before
            arySNobj[m_snCount].m_Err = 0;
            arySNobj[m_snCount].m_state = 1;
            arySNobj[m_snCount].m_FWver = 0;
            arySNobj[m_snCount].m_ok_time = time(NULL);
            //m_snCount++;
            ok++;
        }
        else
            printf("#### MyAssignAddress(%d) fail! ####\n", arySNobj[m_snCount].m_Addr);

        m_snCount++;
    }

    printf("m_snCount = %d, OK = %d\n", m_snCount, ok);
    sprintf(buf, "DataLogger WhiteListRegister() end. m_snCount = %d, OK = %d", m_snCount, ok);
    SaveLog(buf, m_st_time);
    printf("##### WhiteListRegister end #####\n");

    return m_snCount;
}

int CG320::StartRegisterProcess()
{
    int DefaultMODValue = 10;
    int i, ret = 0, cnt = 0, total = 0;
	bool Conflict = false;

	time_t current_time;

	/*if (m_snCount==0) {
        for (i=0; i<253; i++) {
            arySNobj[i].m_Addr=i+1; // address range 1 ~ 253
            memset(arySNobj[i].m_Sn, 0x00, 17);
            arySNobj[i].m_Device = -2;
            arySNobj[i].m_Err = 0;
            //printf("i=%u, addr=%d\n",i,arySNobj[i].m_Addr );
        }
	}*/

	char byMOD = DefaultMODValue;
	//m_snCount = 0;

	if (m_snCount==0) {
        RemoveRegisterQuery(m_busfd, 0);
        CleanRespond(m_busfd);
        usleep(500000);
        RemoveRegisterQuery(m_busfd, 0);
        CleanRespond(m_busfd);
        usleep(500000);
        RemoveRegisterQuery(m_busfd, 0);
        CleanRespond(m_busfd);
        usleep(500000);
	}

    SaveLog((char *)"DataLogger StartRegisterProcess() : run", m_st_time);
    while ( 1 ) {
        ret = MySyncOffLineQuery(m_busfd, 0x00, (byte)byMOD, m_query_buf, QUERY_SIZE);
        if ( ret > 0) {
            printf("#### MySyncOffLineQuery return %d ####\n", ret);
            printf("========================================= Debug date value =========================================");
            for ( i = 0; i < ret; i++ ) {
                if ( i%16 == 0 )
                    printf("\n 0x%04X ~ 0x%04X : ", i, i+15);
                printf("0x%02X ", m_query_buf[i]);
            }
            printf("\n====================================================================================================\n");
            goto Allocate_address;
        } else {
            while ( m_snCount<253 && byMOD>=0 ) {
                ret = MyOffLineQuery(m_busfd, 0x00, m_query_buf, QUERY_SIZE);

                // get time
                current_time = time(NULL);
                m_st_time = localtime(&current_time);
                // get data time not 0 min. current time is 0 min.
                if ( m_data_st_time.tm_min != 0 )
                    if ( m_st_time->tm_min == 0 )
                        return -1;

                if ( ret != -1 ) {
                    printf("#### MyOffLineQuery return %d ####\n", ret);
                    printf("========================================= Debug date value =========================================");
                    for ( i = 0; i < ret; i++ ) {
                        if ( i%16 == 0 )
                            printf("\n 0x%04X ~ 0x%04X : ", i, i+15);
                        printf("0x%02X ", m_query_buf[i]);
                    }
                    printf("\n====================================================================================================\n");
                    if ( !CheckCRC(m_query_buf, 13) ) {
                        printf("#### Conflict! ####\n");
                        Conflict = true;
                        continue;
                    } else {
Allocate_address:
                        cnt = AllocateProcess(m_query_buf, ret);
                        if ( cnt == -1 )
                            return -1;
                        printf("#### AllocateProcess success = %d ####\n", cnt);
                        total += cnt;
                    }
                    usleep(1000000); // 1s
                }
                else
                    printf("#### MyOffLineQuery(%d) No response! ####\n", arySNobj[m_snCount].m_Addr);

                byMOD--;
                printf("================ MOD=%d ================\n",byMOD);
            }
        }
        if (Conflict) {
                Conflict = false;
                if (DefaultMODValue == 20)
                    DefaultMODValue = 40;
                else if (DefaultMODValue == 40)
                    DefaultMODValue = 80;
                else if (DefaultMODValue == 80)
                    break;
                byMOD = DefaultMODValue;
        }
        else
            break;
    }

    return total;
}

int CG320::AllocateProcess(unsigned char *query, int len)
{
    int i = 0, j = 0, index = 0, cnt = 0;
    char sn_tmp[17] = {0};

    time_t current_time;

    SaveLog((char *)"DataLogger AllocateProcess() : run", m_st_time);
    for (i = 0; i < len-12; i++) {
        if ( query[i] == 0x00 && query[i+1] == 0x00 && query[i+2] == 0x08 ) {
            if ( CheckCRC(&query[i], 13) ) {
                printf("#### Send allocate address parameter ####\n");
                // get SN
                sprintf(sn_tmp, "%02X%02X%02X%02X%02X%02X%02X%02X", query[i+3], query[i+4],
                        query[i+5], query[i+6], query[i+7], query[i+8], query[i+9], query[i+10]);
                // check SN
                for (j = 0; j < m_snCount; j++) {
                    if ( !strncmp(arySNobj[j].m_Sn, sn_tmp, 16) ) {
                        // already have device
                        index = j;
                        break;
                    } else
                        index++;
                }
                // if new device
                if ( index == m_snCount )
                    memcpy(arySNobj[index].m_Sn, sn_tmp, 17);

                printf("m_snCount = %d\n", m_snCount);
                printf("index = %d\n", index);
                printf("m_Addr = %d\n", arySNobj[index].m_Addr);
                printf("m_Sn = %s\n", arySNobj[index].m_Sn);
                printf("#########################################\n");

                // get time
                current_time = time(NULL);
                m_st_time = localtime(&current_time);
                // get data time not 0 min. current time is 0 min.
                if ( m_data_st_time.tm_min != 0 )
                    if ( m_st_time->tm_min == 0 )
                        return -1;

                if ( MyAssignAddress(m_busfd, &query[i+3],  arySNobj[index].m_Addr) )
                {
                    printf("=================================\n");
                    printf("#### MyAssignAddress(%d) OK! ####\n", arySNobj[index].m_Addr);
                    printf("=================================\n");
                    arySNobj[index].m_Device = -1;
                    arySNobj[index].m_Err = 0;
                    arySNobj[index].m_state = 1;
                    arySNobj[index].m_FWver = 0;
                    arySNobj[index].m_ok_time = time(NULL);
                    if ( index == m_snCount ) {
                        m_snCount++;
                    }
                    cnt++;
                    i+=12;
                }
                else
                    printf("#### MyAssignAddress(%d) fail! ####\n", arySNobj[m_snCount].m_Addr);
            }
        }
    }

    return cnt;
}

bool CG320::ReRegister(int index)
{
    // send remove the slave address, send MySyncOffLineQuery, send MyAssignAddress
    unsigned int tmp[8] = {0};
    unsigned char buffer[9] = {0};
    //int MOD = 20, ret = 0, i;
    int i = 0;
    char buf[256] = {0};

    sprintf(buf, "DataLogger ReRegiser() : addr %d run", arySNobj[index].m_Addr);
    SaveLog(buf, m_st_time);
    printf("#### ReRegiser start ####\n");
    printf("#### Remove %d Query ####\n", arySNobj[index].m_Addr);
    RemoveRegisterQuery(m_busfd, arySNobj[index].m_Addr);
    usleep(500000);
    RemoveRegisterQuery(m_busfd, arySNobj[index].m_Addr);
    usleep(500000);
    RemoveRegisterQuery(m_busfd, arySNobj[index].m_Addr);
    usleep(500000);

    printf("ReRegiser SN = %s\n", arySNobj[index].m_Sn);
    for (i = 0; i < 8; i++) {
        sscanf(arySNobj[index].m_Sn+2*i, "%02X", &tmp[i]);
        buffer[i] = (unsigned char)tmp[i];
        //printf("buffer[%d] = %02X\n", i, buffer[i]);
    }

    //for (i = 0; i < 3; i++) {
        if ( MyAssignAddress(m_busfd, buffer, arySNobj[index].m_Addr) )
        {
            sprintf(buf, "DataLogger ReRegiser() : addr %d OK", arySNobj[index].m_Addr);
            SaveLog(buf, m_st_time);
            printf("=================================\n");
            printf("#### ReRegiser(%d) OK! ####\n", arySNobj[index].m_Addr);
            printf("=================================\n");
            //arySNobj[index].m_Device = -1; // device not change
            arySNobj[index].m_Err = 0;
            arySNobj[index].m_state = 1;
            arySNobj[index].m_ok_time = time(NULL);
            return true;
        }
        else {
            printf("#### ReRegiser(%d) fail! ####\n", arySNobj[index].m_Addr);
            sprintf(buf, "DataLogger ReRegiser() : addr %d fail", arySNobj[index].m_Addr);
            SaveLog(buf, m_st_time);
        }
    //}

    return false;
}

bool CG320::ReRegisterV3(int index)
{
    // send remove the slave address, send MySyncOffLineQuery, send MyAssignAddress
    unsigned int tmp[8] = {0};
    int i = 0, err = 0;
    char buf[256] = {0};
    byte *lpdata = NULL;

    time_t current_time;
    struct tm *log_time;
    current_time = time(NULL);
    log_time = localtime(&current_time);

    printf("#### ReRegisterV3 start ####\n");
    sprintf(buf, "DataLogger ReRegisterV3() : addr %d run", arySNobj[index].m_Addr);
    SaveLog(buf, log_time);
    printf("#### ReRegiser start ####\n");
    //printf("#### Remove %d Query ####\n", arySNobj[index].m_Addr);
    //RemoveRegisterQuery(m_busfd, arySNobj[index].m_Addr);
    //usleep(500000);

    printf("ReRegiser SN = %s\n", arySNobj[index].m_Sn);
    for (i = 0; i < 8; i++) {
        sscanf(arySNobj[index].m_Sn+2*i, "%02X", &tmp[i]);
        //printf("buffer[%d] = %02X\n", i, tmp[i]);
    }

    unsigned char szRegV3[]={0x01, 0x4B, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00};
    // set model & SN
    szRegV3[3] = (unsigned char)tmp[3];
    szRegV3[4] = (unsigned char)tmp[4];
    szRegV3[5] = (unsigned char)tmp[5];
    szRegV3[6] = (unsigned char)tmp[6];
    szRegV3[7] = (unsigned char)tmp[7];
    MakeReadDataCRC(szRegV3,15);

    MClearRX();
    txsize=15;
    waitAddr = 0x01;
    waitFCode = 0x4B;

    while ( err < 2 ) {
        memcpy(txbuffer, szRegV3, 15);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_1);

        current_time = time(NULL);
        log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 14, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("=================================\n");
            printf("#### ReRegisterV3(%d) OK! ####\n", arySNobj[index].m_Addr);
            printf("=================================\n");
            sprintf(buf, "DataLogger ReRegisterV3() : addr %d OK", arySNobj[index].m_Addr);
            SaveLog(buf, log_time);

            arySNobj[index].m_Err = 0;
            arySNobj[index].m_state = 1;
            arySNobj[index].m_ok_time = time(NULL);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### ReRegisterV3 CRC Error ####\n");
                SaveLog((char *)"DataLogger ReRegisterV3() : CRC Error", log_time);
            }
            else {
                printf("#### ReRegisterV3 No Response ####\n");
                SaveLog((char *)"DataLogger ReRegisterV3() : No Response", log_time);
            }
            szRegV3[1] = 0x4D;
            MakeReadDataCRC(szRegV3,15);
            err++;
        }
    }

    return false;
}

int CG320::WhiteListV3Init()
{
    char buf[256] = {0};
    int i = 0;

    printf("#### WhiteListV3Init start ####\n");

    // init arySNobj array
    m_snCount = 0;
    for (i = 0; i < m_wl_count; i++) {
        printf("\n#### arySNobj[%d] parameter ####\n", i);
        printf("m_snCount = %d\n", m_snCount);
        printf("m_Addr = %d\n", arySNobj[m_snCount].m_Addr);
        sprintf(arySNobj[m_snCount].m_Sn, "%02X%02X%02X%02X%02X%02X%02X%02X",
                m_white_list_buf[0+i*8], m_white_list_buf[1+i*8], m_white_list_buf[2+i*8], m_white_list_buf[3+i*8],
                m_white_list_buf[4+i*8], m_white_list_buf[5+i*8], m_white_list_buf[6+i*8], m_white_list_buf[7+i*8]);
        printf("m_Sn = %s\n", arySNobj[m_snCount].m_Sn);
        printf("################################\n");

        arySNobj[m_snCount].m_Device = 0; // must MI
        arySNobj[m_snCount].m_Err = 0;
        arySNobj[m_snCount].m_state = 0;
        arySNobj[m_snCount].m_FWver = 0;
        arySNobj[m_snCount].m_ok_time = 0;
        m_snCount++;
    }

    printf("\nm_snCount = %d\n", m_snCount);
    sprintf(buf, "DataLogger WhiteListV3Init() end. m_snCount = %d", m_snCount);
    SaveLog(buf, m_st_time);

    printf("#### WhiteListV3Init start ####\n");

    return m_snCount;
}

bool CG320::GetDevice(int index)
{
    printf("#### GetDevice start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    struct tm *log_time;
    // check ok time
    time_t current_time = 0;
    current_time = time(NULL);
    if ( current_time - arySNobj[index].m_ok_time >= OFFLINE_SECOND_MI ) {
        printf("Last m_ok_time more then 1200 sec.\n");
        if ( !ReRegister(index) )
            return false;
    }

    char buf[256] = {0};
    unsigned char szDevice[] = {0x00, 0x03, 0x00, 0x02, 0x00, 0x07, 0x00, 0x00}; // default 0x00, 0x03, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00
    szDevice[0] = arySNobj[index].m_Addr;
    MakeReadDataCRC(szDevice,8);

    MClearRX();
    txsize=8;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szDevice, 8);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_1);

        current_time = time(NULL);
        log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 19, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### GetDevice OK ####\n");
            arySNobj[index].m_Device = (*(lpdata+15) << 8) + *(lpdata+16);
            arySNobj[index].m_ok_time = time(NULL);
            if ( arySNobj[index].m_Device < 0x0A ) {
                printf("#### Address %d, Device 0x%04X ==> MI ####\n", arySNobj[index].m_Addr, arySNobj[index].m_Device);
                sprintf(buf, "DataLogger GetDevice() : Address %d, Device 0x%04X ==> MI", arySNobj[index].m_Addr, arySNobj[index].m_Device);
                SaveLog(buf, log_time);
            }
            else {
                arySNobj[index].m_Model = *(lpdata+4);

                printf("#### Address %d, Device 0x%04X ==> Hybrid, Model %d ####\n", arySNobj[index].m_Addr, arySNobj[index].m_Device, arySNobj[index].m_Model);
                sprintf(buf, "DataLogger GetDevice() : Address %d, Device 0x%04X ==> Hybrid, Model %d", arySNobj[index].m_Addr, arySNobj[index].m_Device, arySNobj[index].m_Model);
                SaveLog(buf, log_time);
            }
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetDevice CRC Error ####\n");
                SaveLog((char *)"DataLogger GetDevice() : CRC Error", log_time);
            }
            else {
                printf("#### GetDevice No Response ####\n");
                SaveLog((char *)"DataLogger GetDevice() : No Response", log_time);
            }
            err++;
        }
    }

    return false;
}

bool CG320::GetMiIDInfo(int index)
{
    printf("#### GetMiIDInfo start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    struct tm *log_time;
    // check ok time
    time_t current_time = 0;
    current_time = time(NULL);
    if ( current_time - arySNobj[index].m_ok_time >= OFFLINE_SECOND_MI ) {
        printf("Last m_ok_time more then 1200 sec.\n");
        if ( !ReRegister(index) )
            return false;
    }

    unsigned char szMIIDinfo[]={0x00, 0x03, 0x00, 0x01, 0x00, 0x0A, 0x00, 0x00};

    while ( err < 3 ) {
        // set buf
        // slave id
        szMIIDinfo[0]=arySNobj[index].m_Addr;
        // function code
        szMIIDinfo[1]=0x03;
        // start address 2 byte
        szMIIDinfo[2]=0x00;
        szMIIDinfo[3]=0x01;
        // no, of data 2 byte
        szMIIDinfo[4]=0x00;
        szMIIDinfo[5]=0x0A;
        // crc
        MakeReadDataCRC(szMIIDinfo,8);

        MClearRX();
        txsize=8;
        waitAddr = arySNobj[index].m_Addr;
        waitFCode = 0x03;
        memcpy(txbuffer, szMIIDinfo, 8);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_1);

        current_time = time(NULL);
        log_time = localtime(&current_time);

        //lpdata = GetRespond(m_busfd, 25, m_dl_config.m_delay_time_1);
        lpdata = GetRespond(m_busfd, 25, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetMiIDInfo OK ####\n");
            //SaveLog((char *)"DataLogger GetMiIDInfo() : OK", log_time);
            arySNobj[index].m_ok_time = time(NULL);
            DumpMiIDInfo(index, lpdata+3);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetMiIDInfo CRC Error ####\n");
                SaveLog((char *)"DataLogger GetMiIDInfo() : CRC Error", log_time);
            }
            else {
                printf("#### GetMiIDInfo No Response ####\n");
                SaveLog((char *)"DataLogger GetMiIDInfo() : No Response", log_time);
                SaveLog((char *)"DataLogger GetMiIDInfo() : run reregister()", log_time);
                ReRegister(index);
            }
            err++;
        }
    }

    return false;
}

bool CG320::GetMiIDInfoV3(int index)
{
    printf("#### GetMiIDInfoV3 start ####\n");

    int err = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0;
    byte *lpdata = NULL;

    struct tm *log_time;
    // check ok time
    time_t current_time = 0;
    current_time = time(NULL);
    if ( current_time - arySNobj[index].m_ok_time >= OFFLINE_SECOND_MI ) {
        printf("Last m_ok_time more then 1200 sec.\n");
        if ( !ReRegisterV3(index) )
            return false;
    }

    unsigned char szMIIDinfoV3[]={0x01, 0x33, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x00, 0x00};
    sscanf(arySNobj[index].m_Sn+6, "%02X%02X%02X%02X%02X", &tmp1, &tmp2, &tmp3, &tmp4, &tmp5);
    // set model & SN
    szMIIDinfoV3[3] = (unsigned char)tmp1;
    szMIIDinfoV3[4] = (unsigned char)tmp2;
    szMIIDinfoV3[5] = (unsigned char)tmp3;
    szMIIDinfoV3[6] = (unsigned char)tmp4;
    szMIIDinfoV3[7] = (unsigned char)tmp5;
    MakeReadDataCRC(szMIIDinfoV3,14);

    MClearRX();
    txsize=14;
    waitAddr = 0x01;
    waitFCode = 0x33;

    while ( err < 2 ) {
        memcpy(txbuffer, szMIIDinfoV3, 14);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_1);

        current_time = time(NULL);
        log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 31, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### GetMiIDInfoV3 OK ####\n");
            //SaveLog((char *)"DataLogger GetMiIDInfoV3() : OK", log_time);
            arySNobj[index].m_ok_time = time(NULL);
            arySNobj[index].m_state = 1; // online
            DumpMiIDInfo(index, lpdata+9);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetMiIDInfoV3 CRC Error ####\n");
                SaveLog((char *)"DataLogger GetMiIDInfoV3() : CRC Error", log_time);
            }
            else {
                printf("#### GetMiIDInfoV3 No Response ####\n");
                SaveLog((char *)"DataLogger GetMiIDInfoV3() : No Response", log_time);
            }
            szMIIDinfoV3[1] = 0x36;
            MakeReadDataCRC(szMIIDinfoV3,14);
            err++;
        }
    }

    return false;
}

void CG320::DumpMiIDInfo(int index, unsigned char *buf)
{
    m_mi_id_info.Customer = (*(buf) << 8) + *(buf+1);
    m_mi_id_info.Model = (*(buf+2) << 8) + *(buf+3);
    m_mi_id_info.SN_Hi = (*(buf+4) << 8) + *(buf+5);
    m_mi_id_info.SN_Lo = (*(buf+6) << 8) + *(buf+7);
    m_mi_id_info.Year = (*(buf+8) << 8) + *(buf+9);
    m_mi_id_info.Month = (*(buf+10) << 8) + *(buf+11);
    m_mi_id_info.Date = (*(buf+12) << 8) + *(buf+13);
    m_mi_id_info.Device = (*(buf+14) << 8) + *(buf+15);
    m_mi_id_info.FWVER = (*(buf+18) << 8) + *(buf+19);

    printf("#### Dump MI ID Info ####\n");
    printf("Customer = %d\n", m_mi_id_info.Customer);
    printf("Model    = %d ==> MI-OnGrid ", m_mi_id_info.Model);
    switch (m_mi_id_info.Model)
    {
        case 0:
            printf("G240V/G300\n");
            break;
        case 1:
            printf("G320\n");
            break;
        case 2:
            printf("G321\n");
            break;
        case 3:
            printf("G640\n");
            break;
        case 4:
            printf("G642\n");
            break;
        case 5:
            printf("G640R\n");
            break;
    }
    printf("SN Hi    = 0x%04X\n", m_mi_id_info.SN_Hi);
    printf("SN Lo    = 0x%04X\n", m_mi_id_info.SN_Lo);
    printf("Years    = %d\n", m_mi_id_info.Year);
    printf("Month    = %02d\n", m_mi_id_info.Month);
    printf("Data     = %02d\n", m_mi_id_info.Date);
    printf("Device   = 0x%04X ==> ", m_mi_id_info.Device);
    arySNobj[index].m_Device = m_mi_id_info.Device;
    if ( m_mi_id_info.Device < 0x0A )
        printf("MI\n");
    else
        printf("Hybrid\n");
    arySNobj[index].m_FWver = m_mi_id_info.FWVER;
    printf("FW VER   = 0x%04X ==> %04d\n", arySNobj[index].m_FWver, arySNobj[index].m_FWver);

    printf("##########################\n");
}

bool CG320::GetMiPowerInfo(int index)
{
    printf("#### GetMiPowerInfo start ####\n");

    int err = 0;
    byte *lpdata = NULL;
    char strbuf[1024] = {0};

    struct tm *log_time;
    // check ok time
    time_t current_time = 0;
    current_time = time(NULL);
    if ( current_time - arySNobj[index].m_ok_time >= OFFLINE_SECOND_MI ) {
        printf("Last m_ok_time more then 1200 sec.\n");
        if ( !ReRegister(index) )
            return false;
    }

    unsigned char szMIPowerinfo[]={0x00, 0x03, 0x02, 0x00, 0x00, 0x21, 0x00, 0x00};

    while ( err < 3 ) {
        // set buf
        // slave id
        szMIPowerinfo[0]=arySNobj[index].m_Addr;
        // function code
        szMIPowerinfo[1]=0x03;
        // start address 2 byte
        szMIPowerinfo[2]=0x02;
        szMIPowerinfo[3]=0x00;
        // no, of data 2 byte
        szMIPowerinfo[4]=0x00;
        szMIPowerinfo[5]=0x21;
        // crc
        MakeReadDataCRC(szMIPowerinfo,8);

        MClearRX();
        txsize=8;
        waitAddr = arySNobj[index].m_Addr;
        waitFCode = 0x03;
        memcpy(txbuffer, szMIPowerinfo, 8);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_1);

        current_time = time(NULL);
		log_time = localtime(&current_time);

        //lpdata = GetRespond(m_busfd, 71, m_dl_config.m_delay_time_1);
        lpdata = GetRespond(m_busfd, 71, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetMiPowerInfo(%d) OK : Address = %d, SN = %s ####\n", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
            //SaveLog((char *)"DataLogger GetMiPowerInfo() : OK", log_time);
            sprintf(strbuf, "DataLogger GetMiPowerInfo(%d) OK : Address = %d, SN = %s", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
            //SaveLog(strbuf, log_time);
            arySNobj[index].m_ok_time = time(NULL);
            DumpMiPowerInfo(lpdata+3);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetMiPowerInfo(%d) CRC Error : Address = %d, SN = %s ####\n", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
                //SaveLog((char *)"DataLogger GetMiPowerInfo() : CRC Error", log_time);
                sprintf(strbuf, "DataLogger GetMiPowerInfo(%d) CRC Error : Address = %d, SN = %s", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
                SaveLog(strbuf, log_time);
            }
            else {
                printf("#### GetMiPowerInfo(%d) No Response : Address = %d, SN = %s ####\n", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
                //SaveLog((char *)"DataLogger GetMiPowerInfo() : No Response", log_time);
                sprintf(strbuf, "DataLogger GetMiPowerInfo(%d) No Response : Address = %d, SN = %s", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
                SaveLog(strbuf, log_time);
                SaveLog((char *)"DataLogger GetMiPowerInfo() : run reregister()", log_time);
                ReRegister(index);
            }
            err++;
        }
    }

    return false;
}

bool CG320::GetMiPowerInfoV3(int index)
{
    printf("#### GetMiPowerInfoV3 start ####\n");

    int err = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0, i = 0;
    byte *lpdata = NULL;
    char strbuf[1024] = {0}, datatmp[32] = {0};

    struct tm *log_time;
    // check ok time
    time_t current_time = 0;
    current_time = time(NULL);
    if ( current_time - arySNobj[index].m_ok_time >= OFFLINE_SECOND_MI ) {
        printf("Last m_ok_time more then 1200 sec.\n");
        if ( !ReRegisterV3(index) )
            return false;
    }

    unsigned char szMIPowerinfoV3[]={0x01, 0x33, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x21, 0x00, 0x00};
    sscanf(arySNobj[index].m_Sn+6, "%02X%02X%02X%02X%02X", &tmp1, &tmp2, &tmp3, &tmp4, &tmp5);
    // set model & SN
    szMIPowerinfoV3[3] = (unsigned char)tmp1;
    szMIPowerinfoV3[4] = (unsigned char)tmp2;
    szMIPowerinfoV3[5] = (unsigned char)tmp3;
    szMIPowerinfoV3[6] = (unsigned char)tmp4;
    szMIPowerinfoV3[7] = (unsigned char)tmp5;
    MakeReadDataCRC(szMIPowerinfoV3,14);

	// for debug
	current_time = time(NULL);
	log_time = localtime(&current_time);
	memset(strbuf, 0, 1024);
	sprintf(strbuf, "GetMiPowerInfoV3(%d) send : 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X",
		index, szMIPowerinfoV3[0], szMIPowerinfoV3[1], szMIPowerinfoV3[2], szMIPowerinfoV3[3], szMIPowerinfoV3[4], szMIPowerinfoV3[5], szMIPowerinfoV3[6],
		szMIPowerinfoV3[7], szMIPowerinfoV3[8], szMIPowerinfoV3[9], szMIPowerinfoV3[10], szMIPowerinfoV3[11], szMIPowerinfoV3[12], szMIPowerinfoV3[13]);
	//SaveLog(strbuf, log_time);

    MClearRX();
    txsize=14;
    waitAddr = 0x01;
    waitFCode = 0x33;

    while ( err < 2 ) {
        memcpy(txbuffer, szMIPowerinfoV3, 14);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_1);

        current_time = time(NULL);
        log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 77, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### GetMiPowerInfoV3(%d) OK : Address = %d, SN = %s ####\n", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
            //SaveLog((char *)"DataLogger GetMiPowerInfoV3() : OK", log_time);
            sprintf(strbuf, "DataLogger GetMiPowerInfoV3(%d) OK : Address = %d, SN = %s", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
            //SaveLog(strbuf, log_time);
            arySNobj[index].m_ok_time = time(NULL);
            arySNobj[index].m_state = 1; // online

	    // for debug
            memset(strbuf, 0, 1024);
            strcpy(strbuf, "Get Data : ");
            for (i = 0; i < 77; i++) {
                sprintf(datatmp, "0x%02X, ", lpdata[i]);
                strcat(strbuf, datatmp);
            }
            //SaveLog(strbuf, log_time);

            DumpMiPowerInfo(lpdata+9);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetMiPowerInfoV3(%d) CRC Error : Address = %d, SN = %s ####\n", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
                //SaveLog((char *)"DataLogger GetMiPowerInfoV3() : CRC Error", log_time);
                sprintf(strbuf, "DataLogger GetMiPowerInfoV3(%d) CRC Error : Address = %d, SN = %s", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
                SaveLog(strbuf, log_time);
            }
            else {
                printf("#### GetMiPowerInfoV3(%d) No Response : Address = %d, SN = %s ####\n", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
                //SaveLog((char *)"DataLogger GetMiPowerInfoV3() : No Response", log_time);
                sprintf(strbuf, "DataLogger GetMiPowerInfoV3(%d) No Response : Address = %d, SN = %s", index, arySNobj[index].m_Addr, arySNobj[index].m_Sn);
                SaveLog(strbuf, log_time);
            }
            szMIPowerinfoV3[1] = 0x36;
            MakeReadDataCRC(szMIPowerinfoV3,14);
            err++;
        }
    }

    return false;
}

void CG320::DumpMiPowerInfo(unsigned char *buf)
{
    m_mi_power_info.Temperature = (*(buf) << 8) + *(buf+1);
    m_mi_power_info.Date = (*(buf+2) << 8) + *(buf+3);
    m_mi_power_info.Hour = (*(buf+4) << 8) + *(buf+5);
    m_mi_power_info.Minute = (*(buf+6) << 8) + *(buf+7);
    m_mi_power_info.Ch1_EacH = (*(buf+8) << 8) + *(buf+9);
    m_mi_power_info.Ch1_EacL = (*(buf+10) << 8) + *(buf+11);
    m_mi_power_info.Ch2_EacH = (*(buf+14) << 8) + *(buf+15);
    m_mi_power_info.Ch2_EacL = (*(buf+16) << 8) + *(buf+17);
    m_mi_power_info.Total_EacH = (*(buf+20) << 8) + *(buf+21);
    m_mi_power_info.Total_EacL = (*(buf+22) << 8) + *(buf+23);
    m_mi_power_info.Ch1_Pac = (*(buf+26) << 8) + *(buf+27);
    m_mi_power_info.Ch2_Pac = (*(buf+28) << 8) + *(buf+29);
    m_mi_power_info.Ch1_Vpv = (*(buf+30) << 8) + *(buf+31);
    m_mi_power_info.Ch1_Ipv = (*(buf+32) << 8) + *(buf+33);
    m_mi_power_info.Ch1_Ppv = (*(buf+34) << 8) + *(buf+35);
    m_mi_power_info.Vac = (*(buf+38) << 8) + *(buf+39);
    m_mi_power_info.Total_Iac = (*(buf+40) << 8) + *(buf+41);
    m_mi_power_info.Total_Pac = (*(buf+42) << 8) + *(buf+43);
    m_mi_power_info.Fac = (*(buf+46) << 8) + *(buf+47);
    m_mi_power_info.Error_Code1 = (*(buf+48) << 8) + *(buf+49);
    m_mi_power_info.Error_Code2 = (*(buf+50) << 8) + *(buf+51);
    m_mi_power_info.Pre1_Code1 = (*(buf+52) << 8) + *(buf+53);
    m_mi_power_info.Pre1_Code2 = (*(buf+54) << 8) + *(buf+55);
    m_mi_power_info.Pre2_Code1 = (*(buf+56) << 8) + *(buf+57);
    m_mi_power_info.Pre2_Code2 = (*(buf+58) << 8) + *(buf+59);
    m_mi_power_info.Ch2_Vpv = (*(buf+60) << 8) + *(buf+61);
    m_mi_power_info.Ch2_Ipv = (*(buf+62) << 8) + *(buf+63);
    m_mi_power_info.Ch2_Ppv = (*(buf+64) << 8) + *(buf+65);

    printf("#### Dump MI Power Info ####\n");
    printf("Temperature = %03.1f\n", ((float)m_mi_power_info.Temperature)/10);
    printf("Date        = %d\n", m_mi_power_info.Date);
    printf("Hour        = %d\n", m_mi_power_info.Hour);
    printf("Minute      = %d\n", m_mi_power_info.Minute);
    printf("Ch1_EacH    = %d\n", m_mi_power_info.Ch1_EacH);
    printf("Ch1_EacL    = %d\n", m_mi_power_info.Ch1_EacL);
    printf("Ch1_Eac     = %04.2f kWh\n", m_mi_power_info.Ch1_EacH*100 + ((float)m_mi_power_info.Ch1_EacL)*0.01);
    printf("Ch2_EacH    = %d\n", m_mi_power_info.Ch2_EacH);
    printf("Ch2_EacL    = %d\n", m_mi_power_info.Ch2_EacL);
    printf("Ch2_Eac     = %04.2f kWh\n", m_mi_power_info.Ch2_EacH*100 + ((float)m_mi_power_info.Ch2_EacL)*0.01);
    printf("Total_EacH  = %d\n", m_mi_power_info.Total_EacH);
    printf("Total_EacL  = %d\n", m_mi_power_info.Total_EacL);
    printf("Total_Eac   = %04.2f kWh\n", m_mi_power_info.Total_EacH*100 + ((float)m_mi_power_info.Total_EacL)*0.01);
    printf("Ch1_Pac     = %03.1f W\n", ((float)m_mi_power_info.Ch1_Pac)*0.1);
    printf("Ch2_Pac     = %03.1f W\n", ((float)m_mi_power_info.Ch2_Pac)*0.1);
    printf("Ch1_Vpv     = %03.1f V\n", ((float)m_mi_power_info.Ch1_Vpv)*0.1);
    printf("Ch1_Ipv     = %04.2f A\n", ((float)m_mi_power_info.Ch1_Ipv)*0.01);
    printf("Ch1_Ppv     = %03.1f W\n", ((float)m_mi_power_info.Ch1_Ppv)*0.1);
    printf("Vac         = %03.1f V\n", ((float)m_mi_power_info.Vac)*0.1);
    printf("Total_Iac   = %05.3f A\n", ((float)m_mi_power_info.Total_Iac)*0.001);
    printf("Total_Pac   = %03.1f W\n", ((float)m_mi_power_info.Total_Pac)*0.1);
    printf("Fac         = %04.2f Hz\n", ((float)m_mi_power_info.Fac)*0.01);
    printf("Error_Code1 = 0x%04X\n", m_mi_power_info.Error_Code1);
    printf("Error_Code2 = 0x%04X\n", m_mi_power_info.Error_Code2);
    printf("Pre1_Code1  = 0x%04X\n", m_mi_power_info.Pre1_Code1);
    printf("Pre1_Code2  = 0x%04X\n", m_mi_power_info.Pre1_Code2);
    printf("Pre2_Code1  = 0x%04X\n", m_mi_power_info.Pre2_Code1);
    printf("Pre2_Code2  = 0x%04X\n", m_mi_power_info.Pre2_Code2);
    printf("Ch2_Vpv     = %03.1f V\n", ((float)m_mi_power_info.Ch2_Vpv)*0.1);
    printf("Ch2_Ipv     = %04.2f A\n", ((float)m_mi_power_info.Ch2_Ipv)*0.01);
    printf("Ch2_Ppv     = %03.1f W\n", ((float)m_mi_power_info.Ch2_Ppv)*0.1);
    printf("############################\n");
}

bool CG320::GetHybridIDData(int index)
{
    printf("#### GetHybridIDData start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    struct tm *log_time;
    // check ok time
    time_t current_time = 0;
    current_time = time(NULL);
    if ( current_time - arySNobj[index].m_ok_time >= OFFLINE_SECOND_HB ) {
        printf("Last m_ok_time more then 1200 sec.\n");
        ReRegister(index);
    }

    unsigned char szHBIDdata[]={0x00, 0x03, 0x00, 0x01, 0x00, 0x0E, 0x00, 0x00};

    while ( err < 3 ) {
        // set buf
        // slave id
        szHBIDdata[0]=arySNobj[index].m_Addr;
        // function code
        szHBIDdata[1]=0x03;
        // start address 2 byte
        szHBIDdata[2]=0x00;
        szHBIDdata[3]=0x01;
        // no, of data 2 byte
        szHBIDdata[4]=0x00;
        szHBIDdata[5]=0x0E;
        // crc
        MakeReadDataCRC(szHBIDdata,8);

        MClearRX();
        txsize=8;
        waitAddr = arySNobj[index].m_Addr;
        waitFCode = 0x03;
        memcpy(txbuffer, szHBIDdata, 8);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        current_time = time(NULL);
		log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 33, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybridIDData OK ####\n");
            //SaveLog((char *)"DataLogger GetHybridIDData() : OK", log_time);
            DumpHybridIDData(lpdata+3);
            arySNobj[index].m_ok_time = time(NULL);
            ParserHybridIDFlags1(m_hb_id_data.Flags1);
            ParserHybridIDFlags2(m_hb_id_data.Flags2);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybridIDData CRC Error ####\n");
                SaveLog((char *)"DataLogger GetHybridIDData() : CRC Error", log_time);
            }
            else {
                printf("#### GetHybridIDData No Response ####\n");
                SaveLog((char *)"DataLogger GetHybridIDData() : No Response", log_time);
                //SaveLog((char *)"DataLogger GetHybridIDData() : run reregister()", log_time);
                //ReRegister(index);
            }
            err++;
        }
    }

    return false;
}

void CG320::DumpHybridIDData(unsigned char *buf)
{
    m_hb_id_data.Grid_Voltage = (*(buf) << 8) + *(buf+1);
    m_hb_id_data.HW_Ver = *(buf+2);
    m_hb_id_data.Model = *(buf+3);
    //m_hb_id_data.Model = (*(buf+2) << 8) + *(buf+3);
    m_hb_id_data.SN_Hi = (*(buf+4) << 8) + *(buf+5);
    m_hb_id_data.SN_Lo = (*(buf+6) << 8) + *(buf+7);
    m_hb_id_data.Year = (*(buf+8) << 8) + *(buf+9);
    m_hb_id_data.Month = (*(buf+10) << 8) + *(buf+11);
    m_hb_id_data.Date = (*(buf+12) << 8) + *(buf+13);
    m_hb_id_data.Inverter_Ver = (*(buf+14) << 8) + *(buf+15);
    m_hb_id_data.DD_Ver = (*(buf+16) << 8) + *(buf+17);
    m_hb_id_data.EEPROM_Ver = (*(buf+18) << 8) + *(buf+19);
    m_hb_id_data.Display_Ver = (*(buf+20) << 8) + *(buf+21);
    m_hb_id_data.Flags1 = (*(buf+24) << 8) + *(buf+25);
    m_hb_id_data.Flags2 = (*(buf+26) << 8) + *(buf+27);

/*    printf("#### Dump Hybrid ID Data ####\n");
    printf("Grid_Voltage = %d ==> ", m_hb_id_data.Grid_Voltage);
    switch (m_hb_id_data.Grid_Voltage)
    {
        case 0:
            printf("240V\n");
            break;
        case 1:
            printf("230V\n");
            break;
        case 2:
            printf("220V\n");
            break;
        case 3:
            printf("208V\n");
            break;
        case 4:
            printf("full range\n");
            break;
    }
    printf("Model        = %d ==> ", m_hb_id_data.Model);
    switch (m_hb_id_data.Model)
    {
        case 1:
            printf("H5000\n");
            break;
        case 2:
            printf("H5001\n");
            break;
        case 3:
            printf("HB5\n");
            break;
        case 4:
            printf("HB51\n");
            break;
        case 5:
            printf("H5000 for P\n");
            break;
        case 6:
            printf("H5001 for P\n");
            break;
        case 7:
            printf("H5001 Stacking\n");
            break;
        default:
            printf("Other\n");
    }
    printf("HW_Ver       = 0x%02X\n", m_hb_id_data.HW_Ver);
    printf("SN_Hi        = 0x%04X\n", m_hb_id_data.SN_Hi);
    printf("SN_Lo        = 0x%04X\n", m_hb_id_data.SN_Lo);
    printf("Year         = %d\n", m_hb_id_data.Year);
    printf("Month        = %02d\n", m_hb_id_data.Month);
    printf("Date         = %02d\n", m_hb_id_data.Date);
    printf("Inverter_Ver = 0x%04X ==> ", m_hb_id_data.Inverter_Ver);
    if ( m_hb_id_data.Inverter_Ver < 0x0A )
        printf("MI\n");
    else
        printf("Hybrid\n");
    printf("DD_Ver       = %d\n", m_hb_id_data.DD_Ver);
    printf("EEPROM_Ver   = %d\n", m_hb_id_data.EEPROM_Ver);
    printf("Display_Ver  = %d\n", m_hb_id_data.Display_Ver);
    printf("Flags1        = 0x%02X ==> \n", m_hb_id_data.Flags1);
    printf("Flags2        = 0x%02X ==> \n", m_hb_id_data.Flags2);
    printf("#############################\n");*/
}

bool CG320::SetHybridIDData(int index)
{
    printf("#### SetHybridIDData Start ####\n");

    int err = 0;
    unsigned short crc;
    byte *lpdata = NULL;

    unsigned char szIDData[39]={0};
    szIDData[0] = arySNobj[index].m_Addr;
    szIDData[1] = 0x10; // function code
    szIDData[2] = 0x00;
    szIDData[3] = 0x01; // star address
    szIDData[4] = 0x00;
    szIDData[5] = 0x0F; // number of data
    szIDData[6] = 0x1E; // bytes
    // data 0x01 ~ 0x0B, & flags at 0x0D, 0x0E
    szIDData[7] = 0x00;
    szIDData[8] = (unsigned char)m_hb_id_data.Grid_Voltage;
    szIDData[9] = (unsigned char)m_hb_id_data.HW_Ver;
    szIDData[10] = (unsigned char)m_hb_id_data.Model;
    szIDData[11] = (unsigned char)((m_hb_id_data.SN_Hi >> 8) & 0x00FF);
    szIDData[12] = (unsigned char)(m_hb_id_data.SN_Hi & 0x00FF);
    szIDData[13] = (unsigned char)((m_hb_id_data.SN_Lo >> 8) & 0x00FF);
    szIDData[14] = (unsigned char)(m_hb_id_data.SN_Lo & 0x00FF);
    szIDData[15] = (unsigned char)((m_hb_id_data.Year >> 8) & 0x00FF);
    szIDData[16] = (unsigned char)(m_hb_id_data.Year & 0x00FF);
    szIDData[17] = 0x00;
    szIDData[18] = (unsigned char)m_hb_id_data.Month;
    szIDData[19] = 0x00;
    szIDData[20] = (unsigned char)m_hb_id_data.Date;
    szIDData[21] = (unsigned char)((m_hb_id_data.Inverter_Ver >> 8) & 0x00FF);
    szIDData[22] = (unsigned char)(m_hb_id_data.Inverter_Ver & 0xFF);
    szIDData[23] = (unsigned char)((m_hb_id_data.DD_Ver >> 8) & 0x00FF);
    szIDData[24] = (unsigned char)(m_hb_id_data.DD_Ver & 0xFF);
    szIDData[25] = (unsigned char)((m_hb_id_data.EEPROM_Ver >> 8) & 0x00FF);
    szIDData[26] = (unsigned char)(m_hb_id_data.EEPROM_Ver & 0xFF);
    szIDData[27] = (unsigned char)((m_hb_id_data.Display_Ver >> 8) & 0x00FF);
    szIDData[28] = (unsigned char)(m_hb_id_data.Display_Ver & 0xFF);
    // zero 0x0C
    szIDData[29] = 0x00;
    szIDData[30] = 0x00;
    // flags1 0x0D
    szIDData[31] = (unsigned char)((m_hb_id_data.Flags1 >> 8) & 0x00FF);
    szIDData[32] = (unsigned char)(m_hb_id_data.Flags1 & 0xFF);
    // flags2 0x0E
    szIDData[33] = (unsigned char)((m_hb_id_data.Flags2 >> 8) & 0x00FF);
    szIDData[34] = (unsigned char)(m_hb_id_data.Flags2 & 0xFF);
    // data crc 0x0F
    crc = CalculateCRC(szIDData+7, 28);
    szIDData[35] = (unsigned char) (crc >> 8); // data crc hi
    szIDData[36] = (unsigned char) (crc & 0xFF); // data crc lo
    szIDData[37] = 0x00; // cmd crc hi
    szIDData[38] = 0x00; // cmd crc lo
    MakeReadDataCRC(szIDData,39);
    MClearRX();
    txsize=39;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x10;

    while ( err < 3 ) {
        memcpy(txbuffer, szIDData, 39);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        lpdata = GetRespond(m_busfd, 8, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 8) ) {
                printf("#### SetHybridIDData OK ####\n");
                SaveLog((char *)"DataLogger SetHybridIDData() : OK", m_st_time);
                arySNobj[index].m_ok_time = time(NULL);
                //free(lpdata);
                return true;
            } else {
                printf("#### SetHybridIDData CRC Error ####\n");
                SaveLog((char *)"DataLogger SetHybridIDData() : CRC Error", m_st_time);
                err++;
            }
            //free(lpdata);
        } else {
            printf("#### SetHybridIDData No Response ####\n");
            SaveLog((char *)"DataLogger SetHybridIDData() : No Response", m_st_time);
            err++;
        }

        usleep(1000000);
    }

    return false;
}

void CG320::ParserHybridIDFlags1(int flags)
{
    int tmp = flags;

    m_hb_id_flags1.B0B1_External_Sensor = tmp & 0x03;
    //tmp>>=2;

/*    printf("#### Parser Hybrid ID Flags1 ####\n");
    printf("Bit0Bit1 : External Sensor = %d\n", m_hb_id_flags1.B0B1_External_Sensor);
    printf("################################\n");*/
}

void CG320::ParserHybridIDFlags2(int flags)
{
    int tmp = flags;

    m_hb_id_flags2.B0_Rule21 = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B1_PVParallel = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B2_PVOffGrid = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B3_Heco1 = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B4_Heco2 = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B5_ACCoupling = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B6_FreControl = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B7_ArcDetection = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B8_PREPA = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B9_Self_Supply = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B10_Charge_only_from_PV = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B11_Dominion = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B12_SunSpec = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B13_MA = tmp & 0x01;
    tmp>>=1;
    m_hb_id_flags2.B14_ZeroExport = tmp & 0x01;

/*    printf("#### Parser Hybrid ID Flags2 ####\n");
    printf("Bit0 : Rule21              = %d\n", m_hb_id_flags2.B0_Rule21);
    printf("Bit1 : PV Parallel         = %d\n", m_hb_id_flags2.B1_PVParallel);
    printf("Bit2 : PV Off Grid         = %d\n", m_hb_id_flags2.B2_PVOffGrid);
    printf("Bit3 : Heco1               = %d\n", m_hb_id_flags2.B3_Heco1);
    printf("Bit4 : Heco2               = %d\n", m_hb_id_flags2.B4_Heco2);
    printf("Bit5 : AC Coupling         = %d\n", m_hb_id_flags2.B5_ACCoupling);
    printf("Bit6 : Fre Control         = %d\n", m_hb_id_flags2.B6_FreControl);
    printf("Bit7 : Arc Detection       = %d\n", m_hb_id_flags2.B7_ArcDetection);
    printf("Bit8 : PREPA               = %d\n", m_hb_id_flags2.B8_PREPA);
    printf("Bit9 : Self Supply         = %d\n", m_hb_id_flags2.B9_Self_Supply);
    printf("Bit10: Charge only from PV = %d\n", m_hb_id_flags2.B10_Charge_only_from_PV);
    printf("Bit11: Dominion            = %d\n", m_hb_id_flags2.B11_Dominion);
    printf("Bit12: Sun Spec            = %d\n", m_hb_id_flags2.B12_SunSpec);
    printf("Bit13: MA                  = %d\n", m_hb_id_flags2.B13_MA);
    printf("Bit14: Zero Export         = %d\n", m_hb_id_flags2.B14_ZeroExport);
    printf("################################\n");*/
}

bool CG320::GetHybridRTCData(int index)
{
    printf("#### GetHybridRTCData start ####\n");

    int err = 0;
    byte *lpdata = NULL;
    time_t current_time;
    struct tm *log_time;

    unsigned char szHBRTCdata[]={0x00, 0x03, 0x00, 0x40, 0x00, 0x06, 0x00, 0x00};
    szHBRTCdata[0]=arySNobj[index].m_Addr;
    MakeReadDataCRC(szHBRTCdata,8);

    MClearRX();
    txsize=8;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBRTCdata, 8);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        current_time = time(NULL);
		log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 17, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybridRTCData OK ####\n");
            //SaveLog((char *)"DataLogger GetHybridRTCData() : OK", log_time);
            arySNobj[index].m_ok_time = time(NULL);
            DumpHybridRTCData(lpdata+3);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybridRTCData CRC Error ####\n");
                SaveLog((char *)"DataLogger GetHybridRTCData() : CRC Error", log_time);
            }
            else {
                printf("#### GetHybridRTCData No Response ####\n");
                SaveLog((char *)"DataLogger GetHybridRTCData() : No Response", log_time);
            }
            err++;
        }
    }

    return false;
}

void CG320::DumpHybridRTCData(unsigned char *buf)
{
    m_hb_rtc_data.Second = (*(buf) << 8) + *(buf+1);
    m_hb_rtc_data.Minute = (*(buf+2) << 8) + *(buf+3);
    m_hb_rtc_data.Hour = (*(buf+4) << 8) + *(buf+5);
    m_hb_rtc_data.Date = (*(buf+6) << 8) + *(buf+7);
    m_hb_rtc_data.Month = (*(buf+8) << 8) + *(buf+9);
    m_hb_rtc_data.Year = (*(buf+10) << 8) + *(buf+11);

/*    printf("#### Dump Hybrid RTC Data ####\n");
    printf("Second = %d\n", m_hb_rtc_data.Second);
    printf("Minute = %d\n", m_hb_rtc_data.Minute);
    printf("Hour   = %d\n", m_hb_rtc_data.Hour);
    printf("Date   = %d\n", m_hb_rtc_data.Date); //
    printf("Month  = %d\n", m_hb_rtc_data.Month);
    printf("Year   = %d\n", m_hb_rtc_data.Year);
    printf("##############################\n");
    printf("rtc time : %4d/%02d/%02d ", m_hb_rtc_data.Year, m_hb_rtc_data.Month, m_hb_rtc_data.Date);
    printf("%02d:%02d:%02d\n", m_hb_rtc_data.Hour, m_hb_rtc_data.Minute, m_hb_rtc_data.Second);
    printf("##############################\n");*/
}

bool CG320::SetHybridRTCData(int index)
{
    time_t  current_time;
    struct tm   *st_time = NULL;

    //printf("#### SetHybridRTCData Start ####\n");
    current_time = time(NULL);
    st_time = localtime(&current_time);

    // check ok time
    if ( current_time - arySNobj[index].m_ok_time >= OFFLINE_SECOND_HB ) {
        printf("Last m_ok_time more then 1200 sec.\n");
        ReRegister(index);
    }

    m_hb_rtc_data.Second = st_time->tm_sec;
    m_hb_rtc_data.Minute = st_time->tm_min;
    m_hb_rtc_data.Hour = st_time->tm_hour;
    m_hb_rtc_data.Date = st_time->tm_mday;
    m_hb_rtc_data.Month = 1 + st_time->tm_mon; // ptm->tm_mon 0~11, m_hb_rtc_data.Month 1~12
    m_hb_rtc_data.Year = 1900 + st_time->tm_year;
    //printf("RTC timebuf : %4d/%02d/%02d ", m_hb_rtc_data.Year, m_hb_rtc_data.Month, m_hb_rtc_data.Date);
    //printf("%02d:%02d:%02d\n", m_hb_rtc_data.Hour, m_hb_rtc_data.Minute, m_hb_rtc_data.Second);
    //printf("#######################################\n");

    int err = 0;
    unsigned short crc;
    byte *lpdata = NULL;

    unsigned char szRTCData[41]={0};

    while ( err < 3 ) {
        // set buf
        szRTCData[0] = arySNobj[index].m_Addr;
        szRTCData[1] = 0x10; // function code
        szRTCData[2] = 0x00;
        szRTCData[3] = 0x40; // star address
        szRTCData[4] = 0x00;
        szRTCData[5] = 0x10; // number of data
        szRTCData[6] = 0x20; // bytes
        // data 0x40 ~ 0x45
        szRTCData[7] = 0x00;
        szRTCData[8] = (unsigned char)m_hb_rtc_data.Second;
        szRTCData[9] = 0x00;
        szRTCData[10] = (unsigned char)m_hb_rtc_data.Minute;
        szRTCData[11] = 0x00;
        szRTCData[12] = (unsigned char)m_hb_rtc_data.Hour;
        szRTCData[13] = 0x00;
        szRTCData[14] = (unsigned char)m_hb_rtc_data.Date;
        szRTCData[15] = 0x00;
        szRTCData[16] = (unsigned char)m_hb_rtc_data.Month;
        szRTCData[17] = (unsigned char)((m_hb_rtc_data.Year >> 8) & 0xFF);
        szRTCData[18] = (unsigned char)(m_hb_rtc_data.Year & 0xFF);
        // zero 0x46 ~ 0x4E
        szRTCData[19] = 0x00;
        szRTCData[20] = 0x00;
        szRTCData[21] = 0x00;
        szRTCData[22] = 0x00;
        szRTCData[23] = 0x00;
        szRTCData[24] = 0x00;
        szRTCData[25] = 0x00;
        szRTCData[26] = 0x00;
        szRTCData[27] = 0x00;
        szRTCData[28] = 0x00;
        szRTCData[29] = 0x00;
        szRTCData[30] = 0x00;
        szRTCData[31] = 0x00;
        szRTCData[32] = 0x00;
        szRTCData[33] = 0x00;
        szRTCData[34] = 0x00;
        szRTCData[35] = 0x00;
        szRTCData[36] = 0x00;
        // data crc 0x4F
        crc = CalculateCRC(szRTCData+7, 30);
        szRTCData[37] = (unsigned char) (crc >> 8); // data crc hi
        szRTCData[38] = (unsigned char) (crc & 0xFF); // data crc lo
        szRTCData[39] = 0x00; // cmd crc hi
        szRTCData[40] = 0x00; // cmd crc lo
        MakeReadDataCRC(szRTCData,41);
        MClearRX();
        txsize=41;
        waitAddr = arySNobj[index].m_Addr;
        waitFCode = 0x10;
        memcpy(txbuffer, szRTCData, 41);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        lpdata = GetRespond(m_busfd, 8, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 8) ) {
                printf("#### SetHybridRTCData OK ####\n");
                SaveLog((char *)"DataLogger SetHybridRTCData() : OK", st_time);
                arySNobj[index].m_ok_time = time(NULL);
                //free(lpdata);
                return true;
            } else {
                printf("#### SetHybridRTCData CRC Error ####\n");
                SaveLog((char *)"DataLogger SetHybridRTCData() : CRC Error", st_time);
                err++;
            }
            //free(lpdata);
        } else {
            printf("#### SetHybridRTCData No Response ####\n");
            SaveLog((char *)"DataLogger SetHybridRTCData() : No Response", st_time);
            //SaveLog((char *)"DataLogger SetHybridRTCData() : run reregister()", st_time);
            //ReRegister(index);
            err++;
        }

        usleep(1000000);
    }

    return false;
}

bool CG320::GetHybridRSInfo(int index)
{
    printf("#### GetHybridRSInfo start ####\n");

    int err = 0;
    byte *lpdata = NULL;
    time_t current_time;
	struct tm *log_time;

    unsigned char szHBRSinfo[]={0x00, 0x03, 0x00, 0x90, 0x00, 0x0F, 0x00, 0x00};
    szHBRSinfo[0]=arySNobj[index].m_Addr;
    MakeReadDataCRC(szHBRSinfo,8);

    MClearRX();
    txsize=8;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBRSinfo, 8);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        current_time = time(NULL);
		log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 35, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybridRSInfo OK ####\n");
            //SaveLog((char *)"DataLogger GetHybridRSInfo() : OK", log_time);
            arySNobj[index].m_ok_time = time(NULL);
            DumpHybridRSInfo(lpdata+3);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybridRSInfo CRC Error ####\n");
                SaveLog((char *)"DataLogger GetHybridRSInfo() : CRC Error", log_time);
            }
            else {
                printf("#### GetHybridRSInfo No Response ####\n");
                SaveLog((char *)"DataLogger GetHybridRSInfo() : No Response", log_time);
            }
            err++;
        }
    }

    return false;
}

void CG320::DumpHybridRSInfo(unsigned char *buf)
{
    m_hb_rs_info.Mode = (*(buf) << 8) + *(buf+1);
    m_hb_rs_info.StarHour = (*(buf+2) << 8) + *(buf+3);
    m_hb_rs_info.StarMin = (*(buf+4) << 8) + *(buf+5);
    m_hb_rs_info.EndHour = (*(buf+6) << 8) + *(buf+7);
    m_hb_rs_info.EndMin = (*(buf+8) << 8) + *(buf+9);
    m_hb_rs_info.MultiModuleSetting = (*(buf+10) << 8) + *(buf+11);
    m_hb_rs_info.BatteryType = (*(buf+12) << 8) + *(buf+13);
    m_hb_rs_info.BatteryCurrent = (*(buf+14) << 8) + *(buf+15);
    m_hb_rs_info.BatteryShutdownVoltage = (*(buf+16) << 8) + *(buf+17);
    m_hb_rs_info.BatteryFloatingVoltage = (*(buf+18) << 8) + *(buf+19);
    m_hb_rs_info.BatteryReservePercentage = (*(buf+20) << 8) + *(buf+21);
    m_hb_rs_info.PeakShavingPower = (*(buf+22) << 8) + *(buf+23);
    m_hb_rs_info.StartFrequency = (*(buf+24) << 8) + *(buf+25);
    m_hb_rs_info.EndFrequency = (*(buf+26) << 8) + *(buf+27);
    m_hb_rs_info.FeedinPower = (*(buf+28) << 8) + *(buf+29);

/*    printf("#### Dump Hybrid RS Info ####\n");
    printf("Mode = %d ==> ", m_hb_rs_info.Mode);
    switch (m_hb_rs_info.Mode)
    {
        case 0:
            printf("Back up\n");
            break;
        case 1:
            printf("Residential\n");
            break;
        case 2:
            printf("Back up without feed in\n");
            break;
        case 3:
            printf("Residential without feed in\n");
            break;
        case 4:
            printf("TOU without battery feed in\n");
            break;
        case 5:
            printf("TOU with battery feed in\n");
            break;
        case 6:
            printf("String inverter\n");
            break;
        case 7:
            printf("Remote control\n");
            break;
    }
    printf("StarHour = %d\n", m_hb_rs_info.StarHour);
    printf("StarMin = %d\n", m_hb_rs_info.StarMin);
    printf("EndHour = %d\n", m_hb_rs_info.EndHour);
    printf("EndMin = %d\n", m_hb_rs_info.EndMin);
    printf("Multi Module Setting = %d ==> ", m_hb_rs_info.MultiModuleSetting);
    switch (m_hb_rs_info.MultiModuleSetting)
    {
        case 0:
            printf("Single\n");
            break;
        case 1:
            printf("Parallel\n");
            break;
        case 2:
            printf("Three phase\n");
            break;
    }
    printf("Battery Type = %d ==> ", m_hb_rs_info.BatteryType);
    switch (m_hb_rs_info.BatteryType)
    {
        case 0:
            printf("None (Default)\n");
            break;
        case 1:
            printf("Lead-Acid\n");
            break;
        case 2:
            printf("Gloden Crown\n");
            break;
        case 3:
            printf("Darfon\n");
            break;
        case 4:
            printf("Panasonic\n");
            break;
    }
    printf("Battery Current = %d A\n", m_hb_rs_info.BatteryCurrent);
    printf("Battery Shutdown Voltage = %03.1f V\n", ((float)m_hb_rs_info.BatteryShutdownVoltage)/10);
    printf("Battery Floating Voltage = %03.1f V\n", ((float)m_hb_rs_info.BatteryFloatingVoltage)/10);
    printf("Battery Reserve Percentage = %d%%\n", m_hb_rs_info.BatteryReservePercentage);
    printf("Peak Shaving Power = %d W\n", m_hb_rs_info.PeakShavingPower*100);
    printf("Start Frequency = %03.1f Hz\n", (float)m_hb_rs_info.StartFrequency);
    printf("End Frequency = %03.1f Hz\n", (float)m_hb_rs_info.EndFrequency);
    printf("Feed-in Power = %d W\n", m_hb_rs_info.FeedinPower*100);
    printf("#############################\n");*/
}

bool CG320::SetHybridRSInfo(int index)
{
    printf("#### SetHybridRSInfo Start ####\n");

    int err = 0;
    unsigned short crc;
    byte *lpdata = NULL;

    unsigned char szRSInfo[41]={0};
    szRSInfo[0] = arySNobj[index].m_Addr;
    szRSInfo[1] = 0x10; // function code
    szRSInfo[2] = 0x00;
    szRSInfo[3] = 0x90; // star address
    szRSInfo[4] = 0x00;
    szRSInfo[5] = 0x10; // number of data
    szRSInfo[6] = 0x20; // bytes
    // data 0x90 ~ 0x9E
    szRSInfo[7] = 0x00;
    szRSInfo[8] = (unsigned char)m_hb_rs_info.Mode;
    szRSInfo[9] = 0x00;
    szRSInfo[10] = (unsigned char)m_hb_rs_info.StarHour;
    szRSInfo[11] = 0x00;
    szRSInfo[12] = (unsigned char)m_hb_rs_info.StarMin;
    szRSInfo[13] = 0x00;
    szRSInfo[14] = (unsigned char)m_hb_rs_info.EndHour;
    szRSInfo[15] = 0x00;
    szRSInfo[16] = (unsigned char)m_hb_rs_info.EndMin;
    szRSInfo[17] = 0x00;
    szRSInfo[18] = (unsigned char)m_hb_rs_info.MultiModuleSetting;
    szRSInfo[19] = 0x00;
    szRSInfo[20] = (unsigned char)m_hb_rs_info.BatteryType;
    szRSInfo[21] = 0x00;
    szRSInfo[22] = (unsigned char)m_hb_rs_info.BatteryCurrent;
    szRSInfo[23] = (unsigned char)((m_hb_rs_info.BatteryShutdownVoltage >> 8) & 0xFF);
    szRSInfo[24] = (unsigned char)(m_hb_rs_info.BatteryShutdownVoltage & 0xFF);
    szRSInfo[25] = (unsigned char)((m_hb_rs_info.BatteryFloatingVoltage >> 8) & 0xFF);
    szRSInfo[26] = (unsigned char)(m_hb_rs_info.BatteryFloatingVoltage & 0xFF);
    szRSInfo[27] = 0x00;
    szRSInfo[28] = (unsigned char)m_hb_rs_info.BatteryReservePercentage;
    szRSInfo[29] = (unsigned char)((m_hb_rs_info.PeakShavingPower >> 8) & 0xFF);
    szRSInfo[30] = (unsigned char)(m_hb_rs_info.PeakShavingPower & 0xFF);
    szRSInfo[31] = (unsigned char)((m_hb_rs_info.StartFrequency >> 8) & 0xFF);
    szRSInfo[32] = (unsigned char)(m_hb_rs_info.StartFrequency & 0xFF);
    szRSInfo[33] = (unsigned char)((m_hb_rs_info.EndFrequency >> 8) & 0xFF);
    szRSInfo[34] = (unsigned char)(m_hb_rs_info.EndFrequency & 0xFF);
    szRSInfo[35] = (unsigned char)((m_hb_rs_info.FeedinPower >> 8) & 0xFF);
    szRSInfo[36] = (unsigned char)(m_hb_rs_info.FeedinPower & 0xFF);
    // data crc 0x9F
    crc = CalculateCRC(szRSInfo+7, 30);
    szRSInfo[37] = (unsigned char) (crc >> 8); // data crc hi
    szRSInfo[38] = (unsigned char) (crc & 0xFF); // data crc lo
    szRSInfo[39] = 0x00; // cmd crc hi
    szRSInfo[40] = 0x00; // cmd crc lo
    MakeReadDataCRC(szRSInfo,41);
    MClearRX();
    txsize=41;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x10;

    while ( err < 3 ) {
        memcpy(txbuffer, szRSInfo, 41);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        lpdata = GetRespond(m_busfd, 8, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 8) ) {
                printf("#### SetHybridRSInfo OK ####\n");
                SaveLog((char *)"DataLogger SetHybridRSInfo() : OK", m_st_time);
                arySNobj[index].m_ok_time = time(NULL);
                //free(lpdata);
                return true;
            } else {
                printf("#### SetHybridRSInfo CRC Error ####\n");
                SaveLog((char *)"DataLogger SetHybridRSInfo() : CRC Error", m_st_time);
                err++;
            }
            //free(lpdata);
        } else {
            printf("#### SetHybridRSInfo No Response ####\n");
            SaveLog((char *)"DataLogger SetHybridRSInfo() : No Response", m_st_time);
            err++;
        }

        usleep(1000000);
    }

    return false;
}

bool CG320::GetHybridRRSInfo(int index)
{
    printf("#### GetHybridRRSInfo start ####\n");

    int err = 0;
    byte *lpdata = NULL;
    time_t current_time;
	struct tm *log_time;

    unsigned char szHBRSinfo[]={0x00, 0x03, 0x00, 0xA0, 0x00, 0x09, 0x00, 0x00};
    szHBRSinfo[0]=arySNobj[index].m_Addr;
    MakeReadDataCRC(szHBRSinfo,8);

    MClearRX();
    txsize=8;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBRSinfo, 8);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        current_time = time(NULL);
		log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 23, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybridRRSInfo OK ####\n");
            //SaveLog((char *)"DataLogger GetHybridRRSInfo() : OK", log_time);
            arySNobj[index].m_ok_time = time(NULL);
            DumpHybridRRSInfo(lpdata+3);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybridRRSInfo CRC Error ####\n");
                SaveLog((char *)"DataLogger GetHybridRRSInfo() : CRC Error", log_time);
            }
            else {
                printf("#### GetHybridRRSInfo No Response ####\n");
                SaveLog((char *)"DataLogger GetHybridRRSInfo() : No Response", log_time);
            }
            err++;
        }
    }

    return false;
}

void CG320::DumpHybridRRSInfo(unsigned char *buf)
{
    m_hb_rrs_info.ChargeSetting = (*(buf) << 8) + *(buf+1);
    m_hb_rrs_info.ChargePower = (*(buf+2) << 8) + *(buf+3);
    m_hb_rrs_info.DischargePower = (*(buf+4) << 8) + *(buf+5);
    m_hb_rrs_info.RampRatePercentage = (*(buf+6) << 8) + *(buf+7);
    m_hb_rrs_info.DegreeLeadLag = (*(buf+8) << 8) + *(buf+9);
    m_hb_rrs_info.Volt_VAr = (*(buf+10) << 8) + *(buf+11);
    m_hb_rrs_info.AC_Coupling_Power = (*(buf+12) << 8) + *(buf+13);
    m_hb_rrs_info.SunSpec_Write_All = (*(buf+14) << 8) + *(buf+15);
    m_hb_rrs_info.Remote_Control = (*(buf+16) << 8) + *(buf+17);

/*    printf("#### Dump Hybrid RRS Info ####\n");
    printf("Charge = %d ==> ", m_hb_rrs_info.ChargeSetting);
    switch (m_hb_rrs_info.ChargeSetting)
    {
        case 0:
            printf("Charge\n");
            break;
        case 1:
            printf("Discharge\n");
            break;
    }
    printf("Charge Power = %d W\n", m_hb_rrs_info.ChargePower*100);
    printf("Discharge Power = %d W\n", m_hb_rrs_info.DischargePower*100);
    printf("Ramp Rate Percentage = %d %%\n", m_hb_rrs_info.RampRatePercentage);
    printf("Degree Lead/Lag = %d\n ==> A = ", m_hb_rrs_info.DegreeLeadLag);
    switch (m_hb_rrs_info.DegreeLeadLag/256)
    {
        case 0:
            printf("0 : Disable");
            break;
        case 1:
            printf("1 : Lead");
            break;
        case 2:
            printf("2 : Lag");
            break;
    }
    printf("\n ==> B = %02d\n", m_hb_rrs_info.DegreeLeadLag%100);
    printf("Volt/VAr Q(V) = %d ==>", m_hb_rrs_info.Volt_VAr);
    switch (m_hb_rrs_info.Volt_VAr)
    {
        case 0:
            printf("Specified Power Factor(SPF)\n");
            break;
        case 1:
            printf("Most aggressive\n");
            break;
        case 2:
            printf("Average\n");
            break;
        case 3:
            printf("Least aggressive\n");
            break;
        default:
            printf("Other\n");
    }
    printf("AC Coupling Power = %d *100W\n", m_hb_rrs_info.AC_Coupling_Power);
    printf("SunSpec Write All = %d\n", m_hb_rrs_info.SunSpec_Write_All);
    printf("Remote Control = %d\n", m_hb_rrs_info.Remote_Control);*/

    printf("##############################\n");
}

bool CG320::SetHybridRRSInfo(int index)
{
    printf("#### SetHybridRRSInfo Start ####\n");

    int err = 0;
    unsigned short crc;
    byte *lpdata = NULL;

    unsigned char szRRSInfo[41]={0};
    szRRSInfo[0] = arySNobj[index].m_Addr;
    szRRSInfo[1] = 0x10; // function code
    szRRSInfo[2] = 0x00;
    szRRSInfo[3] = 0xA0; // star address
    szRRSInfo[4] = 0x00;
    szRRSInfo[5] = 0x10; // number of data
    szRRSInfo[6] = 0x20; // bytes
    // data 0xA0 ~ 0xA8
    szRRSInfo[7] = 0x00;
    szRRSInfo[8] = (unsigned char)m_hb_rrs_info.ChargeSetting;
    szRRSInfo[9] = (unsigned char)((m_hb_rrs_info.ChargePower >> 8) & 0xFF);
    szRRSInfo[10] = (unsigned char)(m_hb_rrs_info.ChargePower & 0xFF);
    szRRSInfo[11] = (unsigned char)((m_hb_rrs_info.DischargePower >> 8) & 0xFF);
    szRRSInfo[12] = (unsigned char)(m_hb_rrs_info.DischargePower & 0xFF);
    szRRSInfo[13] = 0x00;
    szRRSInfo[14] = (unsigned char)m_hb_rrs_info.RampRatePercentage;
    szRRSInfo[15] = (unsigned char)((m_hb_rrs_info.DegreeLeadLag >> 8) & 0xFF);
    szRRSInfo[16] = (unsigned char)(m_hb_rrs_info.DegreeLeadLag & 0xFF);
    szRRSInfo[17] = (unsigned char)((m_hb_rrs_info.Volt_VAr >> 8) & 0xFF);
    szRRSInfo[18] = (unsigned char)(m_hb_rrs_info.Volt_VAr & 0xFF);
    szRRSInfo[19] = (unsigned char)((m_hb_rrs_info.AC_Coupling_Power >> 8) & 0xFF);
    szRRSInfo[20] = (unsigned char)(m_hb_rrs_info.AC_Coupling_Power & 0xFF);
    szRRSInfo[21] = (unsigned char)((m_hb_rrs_info.SunSpec_Write_All >> 8) & 0xFF);
    szRRSInfo[22] = (unsigned char)(m_hb_rrs_info.SunSpec_Write_All & 0xFF);
    szRRSInfo[23] = (unsigned char)((m_hb_rrs_info.Remote_Control >> 8) & 0xFF);;
    szRRSInfo[24] = (unsigned char)(m_hb_rrs_info.Remote_Control & 0xFF);;
    // zero 0xA9 ~ 0xAE
    szRRSInfo[25] = 0x00;
    szRRSInfo[26] = 0x00;
    szRRSInfo[27] = 0x00;
    szRRSInfo[28] = 0x00;
    szRRSInfo[29] = 0x00;
    szRRSInfo[30] = 0x00;
    szRRSInfo[31] = 0x00;
    szRRSInfo[32] = 0x00;
    szRRSInfo[33] = 0x00;
    szRRSInfo[34] = 0x00;
    szRRSInfo[35] = 0x00;
    szRRSInfo[36] = 0x00;
    // data crc 0xAF
    crc = CalculateCRC(szRRSInfo+7, 30);
    szRRSInfo[37] = (unsigned char) (crc >> 8); // data crc hi
    szRRSInfo[38] = (unsigned char) (crc & 0xff); // data crc lo
    szRRSInfo[39] = 0x00; // cmd crc hi
    szRRSInfo[40] = 0x00; // cmd crc lo
    MakeReadDataCRC(szRRSInfo,41);
    MClearRX();
    txsize=41;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x10;

    while ( err < 3 ) {
        memcpy(txbuffer, szRRSInfo, 41);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        lpdata = GetRespond(m_busfd, 8, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 8) ) {
                printf("#### SetHybridRRSInfo OK ####\n");
                SaveLog((char *)"DataLogger SetHybridRRSInfo() : OK", m_st_time);
                arySNobj[index].m_ok_time = time(NULL);
                //free(lpdata);
                return true;
            } else {
                printf("#### SetHybridRRSInfo CRC Error ####\n");
                SaveLog((char *)"DataLogger SetHybridRRSInfo() : CRC Error", m_st_time);
                err++;
            }
            //free(lpdata);
        } else {
            printf("#### SetHybridRRSInfo No Response ####\n");
            SaveLog((char *)"DataLogger SetHybridRRSInfo() : No Response", m_st_time);
            err++;
        }

        usleep(1000000);
    }

    return false;
}

bool CG320::GetHybridRTInfo(int index)
{
    printf("#### GetHybridRTInfo start ####\n");

    int err = 0;
    //int flag = 0;
    byte *lpdata = NULL;
    time_t current_time;
	struct tm *log_time;

    unsigned char szHBRTinfo[]={0x00, 0x03, 0x00, 0xB0, 0x00, 0x50, 0x00, 0x00};
    szHBRTinfo[0]=arySNobj[index].m_Addr;
    MakeReadDataCRC(szHBRTinfo,8);

    MClearRX();
    txsize=8;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBRTinfo, 8);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        current_time = time(NULL);
		log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 165, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybridRTInfo OK ####\n");
            //SaveLog((char *)"DataLogger GetHybridRTInfo() : OK", log_time);
            arySNobj[index].m_ok_time = time(NULL);
            DumpHybridRTInfo(lpdata+3);
            ParserHybridPVInvErrCOD1(m_hb_rt_info.PV_Inv_Error_COD1_Record);
            //ParserHybridPVInvErrCOD1(m_hb_rt_info.PV_Inv_Error_COD1);
            ParserHybridPVInvErrCOD2(m_hb_rt_info.PV_Inv_Error_COD2_Record);
            //ParserHybridPVInvErrCOD2(m_hb_rt_info.PV_Inv_Error_COD2);
            ParserHybridDDErrCOD(m_hb_rt_info.DD_Error_COD_Record);
            //ParserHybridDDErrCOD(m_hb_rt_info.DD_Error_COD);
            ParserHybridIconInfo(m_hb_rt_info.Hybrid_IconL, m_hb_rt_info.Hybrid_IconH);
            ParserHybridPVInvErrCOD3(m_hb_rt_info.PV_Inv_Error_COD3_Record);
            ParserHybridDDErrCOD2(m_hb_rt_info.DD_Error_COD2_Record);
            //flag = 1;
            //break;
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybridRTInfo CRC Error ####\n");
                SaveLog((char *)"DataLogger GetHybridRTInfo() : CRC Error", log_time);
            }
            else {
                printf("#### GetHybridRTInfo No Response ####\n");
                SaveLog((char *)"DataLogger GetHybridRTInfo() : No Response", log_time);
            }
            err++;
        }
    }
/*
    if ( flag == 0 )
        return false;

    //szHBRTinfo[]={0x00, 0x03, 0x00, 0xB0, 0x00, 0x30, 0x00, 0x00};
    szHBRTinfo[3] = 0xE0; // addr
    szHBRTinfo[5] = 0x20; // no. of data
    MakeReadDataCRC(szHBRTinfo,8);

    MClearRX();
    txsize=8;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBRTinfo, 8);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        current_time = time(NULL);
		log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 69, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybridRTInfo2 OK ####\n");
            //SaveLog((char *)"DataLogger GetHybridRTInfo2() : OK", log_time);
            arySNobj[index].m_ok_time = time(NULL);
            DumpHybridRTInfo2(lpdata+3);
            ParserHybridPVInvErrCOD3(m_hb_rt_info.PV_Inv_Error_COD3_Record);
            ParserHybridDDErrCOD2(m_hb_rt_info.DD_Error_COD2_Record);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybridRTInfo2 CRC Error ####\n");
                SaveLog((char *)"DataLogger GetHybridRTInfo2() : CRC Error", log_time);
            }
            else {
                printf("#### GetHybridRTInfo2 No Response ####\n");
                SaveLog((char *)"DataLogger GetHybridRTInfo2() : No Response", log_time);
            }
            err++;
        }
    }
*/
    return false;
}

bool CG320::GetHybridData(int index, int star_address, int data_count)
{
    printf("#### GetHybridData start ####, star_address = 0x%x, data_count = %d\n", star_address, data_count);

    int err = 0, read_data_size = 0, read_count = 0, address = 0;
    //int flag = 0;
    byte *lpdata = NULL;
    char msg[256] = {0};
    unsigned char data_buf[1024] = {0};
    //unsigned char szHBData[]={0x00, 0x03, 0x00, (unsigned char)star_address, 0x00, (unsigned char)data_count, 0x00, 0x00};
    unsigned char szHBData[]={0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char addrh = 0, addrl = 0;

    szHBData[0]=arySNobj[index].m_Addr;
    address = star_address;
    addrh = (unsigned char)((address>>8) & 0xFF);
    szHBData[2] = addrh;
    addrl = (unsigned char)(address & 0xFF);
    szHBData[3] = addrl;

    time_t current_time;
	struct tm *log_time;

    while ( (err < 3) && (read_count < data_count) ) {
        //check remaining size
        if ( read_size < (data_count - read_count) )
            read_data_size = read_size;
        else
            read_data_size = data_count - read_count;
        printf("read_size = %d\n", read_size);
        printf("read_count = %d\n", read_count);
        printf("address = 0x%04X\n", address);
        printf("read_data_size = %d\n", read_data_size);
        //szHBData[3] = star_address + read_count/2;
        szHBData[5] = read_data_size/2;
        MakeReadDataCRC(szHBData,8);

        MClearRX();
        txsize=8;
        waitAddr = arySNobj[index].m_Addr;
        waitFCode = 0x03;

        memcpy(txbuffer, szHBData, 8);
        MStartTX(m_busfd);

        current_time = time(NULL);
		log_time = localtime(&current_time);

		lpdata = GetRespond(m_busfd, read_data_size+5, m_dl_config.m_delay_time_2);
		if ( lpdata ) {
            memcpy(data_buf+read_count, lpdata+3, read_data_size);
            read_count += read_data_size;
            printf("#### GetHybridData OK ####, read_count = %d\n", read_count);
            address += read_count/2;
            addrh = (unsigned char)((address>>8) & 0xFF);
            szHBData[2] = addrh;
            addrl = (unsigned char)(address & 0xFF);
            szHBData[3] = addrl;
            arySNobj[index].m_ok_time = time(NULL);

            if ( read_count == data_count ) {
                if ( arySNobj[index].m_Model < 16 ) { // H5000/5001
                    switch ( star_address )
                    {
                        case START_ADDRESS_ID:
                            DumpHybridIDData(data_buf);
                            ParserHybridIDFlags1(m_hb_id_data.Flags1);
                            ParserHybridIDFlags2(m_hb_id_data.Flags2);
                            break;
                        case START_ADDRESS_RTC:
                            DumpHybridRTCData(data_buf);
                            break;
                        case START_ADDRESS_RSINFO:
                            DumpHybridRSInfo(data_buf);
                            break;
                        case START_ADDRESS_RRSINFO:
                            DumpHybridRRSInfo(data_buf);
                            break;
                        case START_ADDRESS_RTINFO:
                            DumpHybridRTInfo(data_buf);
                            ParserHybridPVInvErrCOD1(m_hb_rt_info.PV_Inv_Error_COD1_Record);
                            ParserHybridPVInvErrCOD2(m_hb_rt_info.PV_Inv_Error_COD2_Record);
                            ParserHybridDDErrCOD(m_hb_rt_info.DD_Error_COD_Record);
                            ParserHybridIconInfo(m_hb_rt_info.Hybrid_IconL, m_hb_rt_info.Hybrid_IconH);
                            ParserHybridPVInvErrCOD3(m_hb_rt_info.PV_Inv_Error_COD3_Record);
                            ParserHybridDDErrCOD2(m_hb_rt_info.DD_Error_COD2_Record);
                            break;
                        default:
                            printf("wrong address\n");
                    }
                } else { // H5500/9600
                    switch ( star_address )
                    {
                        case HB2_START_ADDRESS_ID:
                            DumpHybrid2IDData(data_buf);
                            ParserHybrid2IDFlags(m_hb2_id_data.Safety_Control);
                            break;
                        case HB2_START_ADDRESS_RSINFO:
                            DumpHybrid2RSInfo(data_buf);
                            ParserHybrid2RSFunctionFlags(m_hb2_rs_info.FunctionControl1);
                            ParserHybrid2RSSoftwareFlags(m_hb2_rs_info.SoftwareControl);
                            break;
                        //case START_ADDRESS_RRSINFO:
                        //    DumpHybridRRSInfo(data_buf);
                        //    break;
                        case HB2_START_ADDRESS_RTINFO:
                            DumpHybrid2RTInfo(data_buf);
                            ParserHybrid2RTModuleFlags(m_hb2_rt_info.Module_Status);
                            ParserHybrid2RTFunctionFlags(m_hb2_rt_info.Function_Status);
                            ParserHybrid2PVInvErrCOD1(m_hb2_rt_info.PV_Inv_Error_COD1_Record);
                            ParserHybrid2PVInvErrCOD2(m_hb2_rt_info.PV_Inv_Error_COD2_Record);
                            ParserHybrid2PVInvErrCOD3(m_hb2_rt_info.PV_Inv_Error_COD3_Record);
                            ParserHybrid2DDErrCOD1(m_hb2_rt_info.DD_Error_COD1_Record);
                            ParserHybrid2DDErrCOD2(m_hb2_rt_info.DD_Error_COD2_Record);
                            //ParserHybridIconInfo(m_hb_rt_info.Hybrid_IconL, m_hb_rt_info.Hybrid_IconH);
                            break;
                        case HB2_START_ADDRESS_CEVALUE:
                            DumpHybrid2CEValue(data_buf);
                            break;
                        case HB2_START_ADDRESS_DPINFO:
                            DumpHybrid2DPInfo(data_buf);
                            ParserHybrid2IconInfo(m_hb2_dp_info.Hybrid_IconL, m_hb2_dp_info.Hybrid_IconH);
                            break;
                        case HB2_START_ADDRESS_RTC:
                            DumpHybrid2RTCData(data_buf);
                            break;
                        case HB2_START_ADDRESS_BMSINFO:
                            DumpHybrid2BMSInfo(data_buf);
                            break;
                        default:
                            printf("wrong address\n");
                    }
                }
                sprintf(msg, "DataLogger GetHybridData OK : Address = 0x%04X", star_address);
                printf(msg);
                printf("\n");
                SaveLog(msg, log_time);

                return true;
            }
        } else {
            if ( have_respond == true ) {
                sprintf(msg, "DataLogger GetHybridData CRC Error : Address = 0x%04X", address);
                printf(msg);
                printf("\n");
                SaveLog(msg, log_time);
            }
            else {
                sprintf(msg, "DataLogger GetHybridData No Response : Address = 0x%04X", address);
                printf(msg);
                printf("\n");
                SaveLog(msg, log_time);
            }
            err++;
            if (err == 3) {
                if ( read_size > 20 ) {
                    read_size -= 20;
                    printf("set read_size = %d\n", read_size);
                    err = 0;
                } else {
                    break;
                }
            }
        }
    }

    return false;
}

void CG320::DumpHybridRTInfo(unsigned char *buf)
{
    m_hb_rt_info.Inv_Temp = (*(buf) << 8) + *(buf+1);
    m_hb_rt_info.PV1_Temp = (*(buf+2) << 8) + *(buf+3);
    m_hb_rt_info.PV2_Temp = (*(buf+4) << 8) + *(buf+5);
    m_hb_rt_info.DD_Temp = (*(buf+6) << 8) + *(buf+7);
    m_hb_rt_info.PV1_Voltage = (*(buf+8) << 8) + *(buf+9);
    m_hb_rt_info.PV1_Current = (*(buf+10) << 8) + *(buf+11);
    m_hb_rt_info.PV1_Power = (*(buf+12) << 8) + *(buf+13);
    m_hb_rt_info.PV2_Voltage = (*(buf+14) << 8) + *(buf+15);
    m_hb_rt_info.PV2_Current = (*(buf+16) << 8) + *(buf+17);
    m_hb_rt_info.PV2_Power = (*(buf+18) << 8) + *(buf+19);
    m_hb_rt_info.Load_Voltage = (*(buf+20) << 8) + *(buf+21);
    m_hb_rt_info.Load_Current = (*(buf+22) << 8) + *(buf+23);
    m_hb_rt_info.Load_Power = (*(buf+24) << 8) + *(buf+25);
    m_hb_rt_info.Grid_Voltage = (*(buf+26) << 8) + *(buf+27);
    m_hb_rt_info.Grid_Current = (*(buf+28) << 8) + *(buf+29);
    m_hb_rt_info.Grid_Power = (*(buf+30) << 8) + *(buf+31);
    m_hb_rt_info.Battery_Voltage = (*(buf+32) << 8) + *(buf+33);
    m_hb_rt_info.Battery_Current = (*(buf+34) << 8) + *(buf+35);
    m_hb_rt_info.Bus_Voltage = (*(buf+36) << 8) + *(buf+37);
    m_hb_rt_info.Bus_Current = (*(buf+38) << 8) + *(buf+39);
    m_hb_rt_info.PV_Total_Power = (*(buf+40) << 8) + *(buf+41);
    m_hb_rt_info.PV_Today_EnergyH = (*(buf+42) << 8) + *(buf+43);
    m_hb_rt_info.PV_Today_EnergyL = (*(buf+44) << 8) + *(buf+45);
    m_hb_rt_info.PV_Total_EnergyH = (*(buf+46) << 8) + *(buf+47);
    m_hb_rt_info.PV_Total_EnergyL = (*(buf+48) << 8) + *(buf+49);
    m_hb_rt_info.Bat_Total_EnergyH = (*(buf+50) << 8) + *(buf+51);
    m_hb_rt_info.Bat_Total_EnergyL = (*(buf+52) << 8) + *(buf+53);
    m_hb_rt_info.Load_Total_EnergyH = (*(buf+54) << 8) + *(buf+55);
    m_hb_rt_info.Load_Total_EnergyL = (*(buf+56) << 8) + *(buf+57);
    m_hb_rt_info.GridFeed_TotalH = (*(buf+58) << 8) + *(buf+59);
    m_hb_rt_info.GridFeed_TotalL = (*(buf+60) << 8) + *(buf+61);
    m_hb_rt_info.GridCharge_TotalH = (*(buf+62) << 8) + *(buf+63);
    m_hb_rt_info.GridCharge_TotalL = (*(buf+64) << 8) + *(buf+65);
    m_hb_rt_info.External_Power = (*(buf+66) << 8) + *(buf+67);
    m_hb_rt_info.Sys_State = (*(buf+68) << 8) + *(buf+69);
    m_hb_rt_info.PV_Inv_Error_COD1_Record = (*(buf+70) << 8) + *(buf+71);
    m_hb_rt_info.PV_Inv_Error_COD2_Record = (*(buf+72) << 8) + *(buf+73);
    m_hb_rt_info.DD_Error_COD_Record = (*(buf+74) << 8) + *(buf+75);
    m_hb_rt_info.PV_Inv_Error_COD1 = (*(buf+76) << 8) + *(buf+77);
    m_hb_rt_info.PV_Inv_Error_COD2 = (*(buf+78) << 8) + *(buf+79);
    m_hb_rt_info.DD_Error_COD = (*(buf+80) << 8) + *(buf+81);
    m_hb_rt_info.Hybrid_IconL = (*(buf+82) << 8) + *(buf+83);
    m_hb_rt_info.Hybrid_IconH = (*(buf+84) << 8) + *(buf+85);
    m_hb_rt_info.Error_Code = (*(buf+86) << 8) + *(buf+87);
    m_hb_rt_info.Battery_SOC = (*(buf+88) << 8) + *(buf+89);
    m_hb_rt_info.Invert_Frequency = (*(buf+90) << 8) + *(buf+91);
    m_hb_rt_info.Grid_Frequency = (*(buf+92) << 8) + *(buf+93);
    m_hb_rt_info.PBat = (*(buf+94) << 8) + *(buf+95);
    m_hb_rt_info.PV_Inv_Error_COD3_Record = (*(buf+96) << 8) + *(buf+97);
    m_hb_rt_info.DD_Error_COD2_Record = (*(buf+98) << 8) + *(buf+99);

/*    printf("#### Dump Hybrid RT Info ####\n");
    printf("Inv_Temp = %03.1f C\n", ((float)m_hb_rt_info.Inv_Temp)/10);
    printf("PV1_Temp = %03.1f C\n", ((float)m_hb_rt_info.PV1_Temp)/10);
    printf("PV2_Temp = %03.1f C\n", ((float)m_hb_rt_info.PV2_Temp)/10);
    printf("DD_Temp = %03.1f C\n", ((float)m_hb_rt_info.DD_Temp)/10);
    printf("PV1_Voltage = %d V\n", m_hb_rt_info.PV1_Voltage);
    printf("PV1_Current = %04.2f A\n", ((float)m_hb_rt_info.PV1_Current)/100);
    printf("PV1_Power = %d W\n", m_hb_rt_info.PV1_Power);
    printf("PV2_Voltage = %d V\n", m_hb_rt_info.PV2_Voltage);
    printf("PV2_Current = %04.2f A\n", ((float)m_hb_rt_info.PV2_Current)/100);
    printf("PV2_Power = %d W\n", m_hb_rt_info.PV2_Power);
    printf("Load Voltage = %d V\n", m_hb_rt_info.Load_Voltage);
    printf("Load Current = %04.2f A\n", ((float)m_hb_rt_info.Load_Current)/100);
    printf("Load Power = %d W\n", m_hb_rt_info.Load_Power);
    printf("Grid Voltage = %d V\n", m_hb_rt_info.Grid_Voltage);
    printf("Grid Current = %04.2f A\n", ((float)m_hb_rt_info.Grid_Current)/100);
    printf("Grid Power = %d W\n", m_hb_rt_info.Grid_Power);
    printf("Battery Voltage = %03.1f V\n", ((float)m_hb_rt_info.Battery_Voltage)/10);
    printf("Battery Current = %03.1f A\n", ((float)m_hb_rt_info.Battery_Current)/10);
    printf("Bus Voltage = %03.1f V\n", ((float)m_hb_rt_info.Bus_Voltage)/10);
    printf("Bus Current = %03.1f A\n", ((float)m_hb_rt_info.Bus_Current)/10);
    printf("PV Total Power = %d W\n", m_hb_rt_info.PV_Total_Power);
    printf("PV Today EnergyH = %d\n", m_hb_rt_info.PV_Today_EnergyH);
    printf("PV Today EnergyL = %d\n", m_hb_rt_info.PV_Today_EnergyL);
    printf("PV Today Energy = %04.2f kWHr\n", m_hb_rt_info.PV_Today_EnergyH*100 + ((float)m_hb_rt_info.PV_Today_EnergyL)*0.01);
    printf("PV Total EnergyH = %d\n", m_hb_rt_info.PV_Total_EnergyH);
    printf("PV Total EnergyL = %d\n", m_hb_rt_info.PV_Total_EnergyL);
    printf("PV Total Energy = %04.2f kWHr\n", m_hb_rt_info.PV_Total_EnergyH*100 + ((float)m_hb_rt_info.PV_Total_EnergyL)*0.01);
    printf("Bat Total EnergyH = %d\n", m_hb_rt_info.Bat_Total_EnergyH);
    printf("Bat Total EnergyL = %d\n", m_hb_rt_info.Bat_Total_EnergyL);
    printf("Bat Total Energy = %04.2f kWHr\n", m_hb_rt_info.Bat_Total_EnergyH*100 + ((float)m_hb_rt_info.Bat_Total_EnergyL)*0.01);
    printf("Load Total EnergyH = %d\n", m_hb_rt_info.Load_Total_EnergyH);
    printf("Load Total EnergyL = %d\n", m_hb_rt_info.Load_Total_EnergyL);
    printf("Load Total Energy = %04.2f kWHr\n", m_hb_rt_info.Load_Total_EnergyH*100 + ((float)m_hb_rt_info.Load_Total_EnergyL)*0.01);
    printf("GridFeed_TotalH = %d\n", m_hb_rt_info.GridFeed_TotalH);
    printf("GridFeed_TotalL = %d\n", m_hb_rt_info.GridFeed_TotalL);
    printf("GridFeed_Total = %04.2f kWHr\n", m_hb_rt_info.GridFeed_TotalH*100 + ((float)m_hb_rt_info.GridFeed_TotalL)*0.01);
    printf("GridCharge_TotalH = %d\n", m_hb_rt_info.GridCharge_TotalH);
    printf("GridCharge_TotalL = %d\n", m_hb_rt_info.GridCharge_TotalL);
    printf("GridCharge_Total = %04.2f kWHr\n", m_hb_rt_info.GridCharge_TotalH*100 + ((float)m_hb_rt_info.GridCharge_TotalL)*0.01);
    printf("External_Power = %d W\n", m_hb_rt_info.External_Power*100);
    printf("Sys_State = %x\n", m_hb_rt_info.Sys_State);
    printf("PV_Inv_Error_COD1_Record = 0x%04X\n", m_hb_rt_info.PV_Inv_Error_COD1_Record);
    printf("PV_Inv_Error_COD2_Record = 0x%04X\n", m_hb_rt_info.PV_Inv_Error_COD2_Record);
    printf("DD_Error_COD_Record = 0x%04X\n", m_hb_rt_info.DD_Error_COD_Record);
    printf("PV_Inv_Error_COD1 = 0x%04X\n", m_hb_rt_info.PV_Inv_Error_COD1);
    printf("PV_Inv_Error_COD2 = 0x%04X\n", m_hb_rt_info.PV_Inv_Error_COD2);
    printf("DD_Error_COD = 0x%04X\n", m_hb_rt_info.DD_Error_COD);
    printf("Hybrid_IconL = 0x%04X\n", m_hb_rt_info.Hybrid_IconL);
    printf("Hybrid_IconH = 0x%04X\n", m_hb_rt_info.Hybrid_IconH);
    printf("Error_Code = 0x%04X\n", m_hb_rt_info.Error_Code);
    printf("Battery_SOC = %d %%\n", m_hb_rt_info.Battery_SOC);
    printf("Invert Frequency = %03.1f Hz\n", ((float)m_hb_rt_info.Invert_Frequency)/10);
    printf("Grid Frequency = %03.1f Hz\n", ((float)m_hb_rt_info.Grid_Frequency)/10);
    printf("PBat = %d W\n", m_hb_rt_info.PBat);
    printf("PV_Inv_Error_COD3_Record = 0x%04X\n", m_hb_rt_info.PV_Inv_Error_COD3_Record);
    printf("DD_Error_COD2_Record = 0x%04X\n", m_hb_rt_info.DD_Error_COD2_Record);
    printf("#############################\n");*/
}

void CG320::DumpHybridRTInfo2(unsigned char *buf)
{
    m_hb_rt_info.PV_Inv_Error_COD3_Record = (*(buf) << 8) + *(buf+1);
    m_hb_rt_info.DD_Error_COD2_Record = (*(buf+2) << 8) + *(buf+3);

/*    printf("#### Dump Hybrid RT Info ####\n");
    printf("PV_Inv_Error_COD3_Record = 0x%04X\n", m_hb_rt_info.PV_Inv_Error_COD3_Record);
    printf("DD_Error_COD2_Record = 0x%04X\n", m_hb_rt_info.DD_Error_COD2_Record);
    printf("#############################\n");*/
}

void CG320::ParserHybridPVInvErrCOD1(int COD1)
{
    int tmp = COD1;
    m_hb_pvinv_err_cod1.B0_Fac_HL = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B1_CanBus_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B2_Islanding = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B3_Vac_H = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B4_Vac_L = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B5_Fac_H = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B6_Fac_L = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B7_Fac_LL = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B8_Vac_OCP = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B9_Vac_HL = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B10_Vac_LL = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B11_OPP = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B12_Iac_H = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B13_Ipv_H = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B14_ADCINT_OVF = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod1.B15_Vbus_H = tmp & 0x0001;

/*    printf("#### Parser Hybrid PV Inverter Error Code 1 ####\n");
    printf("Bit0  : Fac_HL = %d\n", m_hb_pvinv_err_cod1.B0_Fac_HL);
    printf("Bit1  : CanBus_Fault = %d\n", m_hb_pvinv_err_cod1.B1_CanBus_Fault);
    printf("Bit2  : Islanding = %d\n", m_hb_pvinv_err_cod1.B2_Islanding);
    printf("Bit3  : Vac_H = %d\n", m_hb_pvinv_err_cod1.B3_Vac_H);
    printf("Bit4  : Vac_L = %d\n", m_hb_pvinv_err_cod1.B4_Vac_L);
    printf("Bit5  : Fac_H = %d\n", m_hb_pvinv_err_cod1.B5_Fac_H);
    printf("Bit6  : Fac_L = %d\n", m_hb_pvinv_err_cod1.B6_Fac_L);
    printf("Bit7  : Fac_LL = %d\n", m_hb_pvinv_err_cod1.B7_Fac_LL);
    printf("Bit8  : Vac_OCP = %d\n", m_hb_pvinv_err_cod1.B8_Vac_OCP);
    printf("Bit9  : Vac_HL = %d\n", m_hb_pvinv_err_cod1.B9_Vac_HL);
    printf("Bit10 : Vac_LL = %d\n", m_hb_pvinv_err_cod1.B10_Vac_LL);
    printf("Bit11 : OPP = %d\n", m_hb_pvinv_err_cod1.B11_OPP);
    printf("Bit12 : Iac_H = %d\n", m_hb_pvinv_err_cod1.B12_Iac_H);
    printf("Bit13 : Ipv_H = %d\n", m_hb_pvinv_err_cod1.B13_Ipv_H);
    printf("Bit14 : ADCINT_OVF = %d\n", m_hb_pvinv_err_cod1.B14_ADCINT_OVF);
    printf("Bit15 : Vbus_H = %d\n", m_hb_pvinv_err_cod1.B15_Vbus_H);
    printf("################################################\n");*/
}

void CG320::ParserHybridPVInvErrCOD2(int COD2)
{
    int tmp = COD2;
    m_hb_pvinv_err_cod2.B0_Arc = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B1_Vac_Relay_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B2_Ipv1_Short = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B3_Ipv2_Short = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B4_Vac_Short = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B5_CT_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B6_PV_Over_Power = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B7_NO_GRID = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B8_PV_Input_High = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B9_INV_Overload = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B10_RCMU_30 = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B11_RCMU_60 = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B12_RCMU_150 = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B13_RCMU_300 = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B14_RCMU_Test_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod2.B15_Vac_LM = tmp & 0x0001;

/*    printf("#### Parser Hybrid PV Inverter Error Code 2 ####\n");
    printf("Bit0  : Arc = %d\n", m_hb_pvinv_err_cod2.B0_Arc);
    printf("Bit1  : Vac_Relay_Fault = %d\n", m_hb_pvinv_err_cod2.B1_Vac_Relay_Fault);
    printf("Bit2  : Ipv1_Short = %d\n", m_hb_pvinv_err_cod2.B2_Ipv1_Short);
    printf("Bit3  : Ipv2_Short = %d\n", m_hb_pvinv_err_cod2.B3_Ipv2_Short);
    printf("Bit4  : Vac_Short = %d\n", m_hb_pvinv_err_cod2.B4_Vac_Short);
    printf("Bit5  : CT_Fault = %d\n", m_hb_pvinv_err_cod2.B5_CT_Fault);
    printf("Bit6  : PV_Over_Power = %d\n", m_hb_pvinv_err_cod2.B6_PV_Over_Power);
    printf("Bit7  : NO_GRID = %d\n", m_hb_pvinv_err_cod2.B7_NO_GRID);
    printf("Bit8  : PV_Input_High = %d\n", m_hb_pvinv_err_cod2.B8_PV_Input_High);
    printf("Bit9  : INV_Overload = %d\n", m_hb_pvinv_err_cod2.B9_INV_Overload);
    printf("Bit10 : RCMU_30 = %d\n", m_hb_pvinv_err_cod2.B10_RCMU_30);
    printf("Bit11 : RCMU_60 = %d\n", m_hb_pvinv_err_cod2.B11_RCMU_60);
    printf("Bit12 : RCMU_150 = %d\n", m_hb_pvinv_err_cod2.B12_RCMU_150);
    printf("Bit13 : RCMU_300 = %d\n", m_hb_pvinv_err_cod2.B13_RCMU_300);
    printf("Bit14 : RCMU_Test_Fault = %d\n", m_hb_pvinv_err_cod2.B14_RCMU_Test_Fault);
    printf("Bit15 : Vac_LM = %d\n", m_hb_pvinv_err_cod2.B15_Vac_LM);
    printf("################################################\n");*/
}

void CG320::ParserHybridPVInvErrCOD3(int COD3)
{
    int tmp = COD3;
    m_hb_pvinv_err_cod3.B0_External_PV_OPP = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod3.B1_Model123_Reconnected_Delay = tmp & 0x0001;
    tmp>>=1;
    m_hb_pvinv_err_cod3.B2_Peak_Shaving_Over_Power = tmp & 0x0001;

/*    printf("#### Parser Hybrid PV Inverter Error Code 3 ####\n");
    printf("Bit0  : External_PV_OPP = %d\n", m_hb_pvinv_err_cod3.B0_External_PV_OPP);
    printf("Bit1  : Model123_Reconnected_Delay = %d\n", m_hb_pvinv_err_cod3.B1_Model123_Reconnected_Delay);
    printf("Bit2  : Peak_Shaving_Over_Power = %d\n", m_hb_pvinv_err_cod3.B2_Peak_Shaving_Over_Power);
    printf("################################################\n");*/
}

void CG320::ParserHybridDDErrCOD(int COD)
{
    int tmp = COD;
    m_hb_dd_err_cod.B0_Vbat_H = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B1_Vbat_L = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B2_Vbus_H = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B3_Vbus_L = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B4_Ibus_H = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B5_Ibat_H = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B6_Charger_T = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B7_Code = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B8_Vbat_Drop = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B9_INV_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B10_GND_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B11_No_bat = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B12_BMS_Comute_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B13_BMS_Over_Current = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B14_Restart = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod.B15_Bat_Setting_Fault = tmp & 0x0001;

/*    printf("#### Parser Hybrid DD Error Code ####\n");
    printf("Bit0  : Vbat_H = %d\n", m_hb_dd_err_cod.B0_Vbat_H);
    printf("Bit1  : Vbat_L = %d\n", m_hb_dd_err_cod.B1_Vbat_L);
    printf("Bit2  : Vbus_H = %d\n", m_hb_dd_err_cod.B2_Vbus_H);
    printf("Bit3  : Vbus_L = %d\n", m_hb_dd_err_cod.B3_Vbus_L);
    printf("Bit4  : Ibus_H = %d\n", m_hb_dd_err_cod.B4_Ibus_H);
    printf("Bit5  : Ibat_H = %d\n", m_hb_dd_err_cod.B5_Ibat_H);
    printf("Bit6  : Charger_T = %d\n", m_hb_dd_err_cod.B6_Charger_T);
    printf("Bit7  : Code = %d\n", m_hb_dd_err_cod.B7_Code);
    printf("Bit8  : Vbat_Drop = %d\n", m_hb_dd_err_cod.B8_Vbat_Drop);
    printf("Bit9  : INV_Fault = %d\n", m_hb_dd_err_cod.B9_INV_Fault);
    printf("Bit10 : GND_Fault = %d\n", m_hb_dd_err_cod.B10_GND_Fault);
    printf("Bit11 : No_bat = %d\n", m_hb_dd_err_cod.B11_No_bat);
    printf("Bit12 : BMS_Comute_Fault = %d\n", m_hb_dd_err_cod.B12_BMS_Comute_Fault);
    printf("Bit13 : BMS_Over_Current = %d\n", m_hb_dd_err_cod.B13_BMS_Over_Current);
    printf("Bit14 : Restart = %d\n", m_hb_dd_err_cod.B14_Restart);
    printf("Bit15 : Bat_Setting_Fault = %d\n", m_hb_dd_err_cod.B15_Bat_Setting_Fault);
    printf("#####################################\n");*/
}

void CG320::ParserHybridDDErrCOD2(int COD2)
{
    int tmp = COD2;
    m_hb_dd_err_cod2.B0_EEProm_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod2.B1_Communi_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod2.B2_OT_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod2.B3_Fan_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod2.B4_Low_Battery = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod2.B5_Relay_state = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod2.B6_Off_Grid_Operation = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod2.B7_InvEnable_flag = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod2.B8_Bypass_flag = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod2.B9_DD_en = tmp & 0x0001;
    tmp>>=1;
    m_hb_dd_err_cod2.B10_PVEnable_flag = tmp & 0x0001;

/*    printf("#### Parser Hybrid DD Error Code 2 ####\n");
    printf("Bit0  : EEProm_Fault = %d\n", m_hb_dd_err_cod2.B0_EEProm_Fault);
    printf("Bit1  : Communi_Fault = %d\n", m_hb_dd_err_cod2.B1_Communi_Fault);
    printf("Bit2  : OT_Fault = %d\n", m_hb_dd_err_cod2.B2_OT_Fault);
    printf("Bit3  : Fan_Fault = %d\n", m_hb_dd_err_cod2.B3_Fan_Fault);
    printf("Bit4  : Low_Battery = %d\n", m_hb_dd_err_cod2.B4_Low_Battery);
    printf("Bit5  : Relay_state = %d\n", m_hb_dd_err_cod2.B5_Relay_state);
    printf("Bit6  : Off_Grid_Operation = %d\n", m_hb_dd_err_cod2.B6_Off_Grid_Operation);
    printf("Bit7  : InvEnable_flag = %d\n", m_hb_dd_err_cod2.B7_InvEnable_flag);
    printf("Bit8  : Bypass_flag = %d\n", m_hb_dd_err_cod2.B8_Bypass_flag);
    printf("Bit9  : DD_en = %d\n", m_hb_dd_err_cod2.B9_DD_en);
    printf("Bit10 : PVEnable_flag = %d\n", m_hb_dd_err_cod2.B10_PVEnable_flag);
    printf("#####################################\n");*/
}

void CG320::ParserHybridIconInfo(int Icon_L, int Icon_H)
{
    int tmp = Icon_L;
    m_hb_icon_info.B0_PV = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B1_MPPT = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B2_Battery = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B3_Inverter = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B4_Grid = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B5_Load = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B6_OverLoad = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B7_Error = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B8_Warning = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B9_PC = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B10_BatCharge = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B11_BatDischarge = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B12_FeedingGrid = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B13_PFCMode = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B14_GridCharge = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B15_GridDischarge = tmp & 0x0001;

    tmp = Icon_H;
    m_hb_icon_info.B16_18_INVFlag = tmp & 0x0007;
    tmp>>=3;
    m_hb_icon_info.B19_GeneratorMode = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B20_Master_Slave = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B21_SettingOK = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B22_24_BatType = tmp & 0x0007;
    tmp>>=3;
    m_hb_icon_info.B25_26_MultiINV = tmp & 0x0003;
    tmp>>=2;
    m_hb_icon_info.B27_LoadCharge = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B28_LoadDischarge = tmp & 0x0001;
    tmp>>=1;
    m_hb_icon_info.B29_30_LeadLag = tmp & 0x0003;
    tmp>>=2;

/*    printf("#### Parser Hybrid Icon ####\n");
    printf("Bit0     : PV = %d\n", m_hb_icon_info.B0_PV);
    printf("Bit1     : MPPT = %d\n", m_hb_icon_info.B1_MPPT);
    printf("Bit2     : Battery = %d\n", m_hb_icon_info.B2_Battery);
    printf("Bit3     : Inverter = %d\n", m_hb_icon_info.B3_Inverter);
    printf("Bit4     : Grid = %d\n", m_hb_icon_info.B4_Grid);
    printf("Bit5     : Load = %d\n", m_hb_icon_info.B5_Load);
    printf("Bit6     : OverLoad = %d\n", m_hb_icon_info.B6_OverLoad);
    printf("Bit7     : Error = %d\n", m_hb_icon_info.B7_Error);
    printf("Bit8     : Warning = %d\n", m_hb_icon_info.B8_Warning);
    printf("Bit9     : PC = %d\n", m_hb_icon_info.B9_PC);
    printf("Bit10    : Bat Charge = %d\n", m_hb_icon_info.B10_BatCharge);
    printf("Bit11    : Bat Discharge = %d\n", m_hb_icon_info.B11_BatDischarge);
    printf("Bit12    : Feeding Grid = %d\n", m_hb_icon_info.B12_FeedingGrid);
    printf("Bit13    : PFC Mode = %d\n", m_hb_icon_info.B13_PFCMode);
    printf("Bit14    : Grid Charge = %d\n", m_hb_icon_info.B14_GridCharge);
    printf("Bit15    : Grid Discharge = %d\n", m_hb_icon_info.B15_GridDischarge);
    printf("Bit16-18 : INV Flag = %d\n", m_hb_icon_info.B16_18_INVFlag);
    printf("Bit19    : Generator Mode = %d\n", m_hb_icon_info.B19_GeneratorMode);
    printf("Bit20    : Master Slave = %d\n", m_hb_icon_info.B20_Master_Slave);
    printf("Bit21    : Setting OK = %d\n", m_hb_icon_info.B21_SettingOK);
    printf("Bit22-24 : Bat Type = %d\n", m_hb_icon_info.B22_24_BatType);
    printf("Bit25-26 : Multi-INV = %d\n", m_hb_icon_info.B25_26_MultiINV);
    printf("Bit27    : Load Charge = %d\n", m_hb_icon_info.B27_LoadCharge);
    printf("Bit28    : Load Discharge = %d\n", m_hb_icon_info.B28_LoadDischarge);
    printf("Bit29-30 : Lead Lag = %d\n", m_hb_icon_info.B29_30_LeadLag);
    printf("############################\n");*/
}

bool CG320::GetHybridBMSInfo(int index)
{
    printf("#### GetHybridBMSInfo start ####\n");

    int err = 0;
    byte *lpdata = NULL;
    time_t current_time;
	struct tm *log_time;

    unsigned char szHBBMSinfo[]={0x00, 0x03, 0x02, 0x00, 0x00, 0x0C, 0x00, 0x00};
    szHBBMSinfo[0]=arySNobj[index].m_Addr;
    MakeReadDataCRC(szHBBMSinfo,8);

    MClearRX();
    txsize=8;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBBMSinfo, 8);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        current_time = time(NULL);
		log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 29, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybridBMSInfo OK ####\n");
            //SaveLog((char *)"DataLogger GetHybridBMSInfo() : OK", log_time);
            arySNobj[index].m_ok_time = time(NULL);
            DumpHybridBMSInfo(lpdata+3);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybridBMSInfo CRC Error ####\n");
                SaveLog((char *)"DataLogger GetHybridBMSInfo() : CRC Error", log_time);
            }
            else {
                printf("#### GetHybridBMSInfo No Response ####\n");
                SaveLog((char *)"DataLogger GetHybridBMSInfo() : No Response", log_time);
            }
            err++;
        }
    }

    return false;
}

bool CG320::GetHybridBMSVer(int index)
{
    printf("#### GetHybridBMSVer start ####\n");

    int err = 0;
    byte *lpdata = NULL;
    time_t current_time;
	struct tm *log_time;

    unsigned char szHBBMSver[]={0x00, 0x03, 0x02, 0x71, 0x00, 0x01, 0x00, 0x00};
    szHBBMSver[0]=arySNobj[index].m_Addr;
    if ( arySNobj[index].m_Model < 16 )
        szHBBMSver[3] = 0x71;
    else
        szHBBMSver[3] = 0x14;
    MakeReadDataCRC(szHBBMSver,8);

    MClearRX();
    txsize=8;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBBMSver, 8);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        current_time = time(NULL);
		log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 7, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybridBMSVer OK ####\n");
            SaveLog((char *)"DataLogger GetHybridBMSVer() : OK", log_time);
            arySNobj[index].m_ok_time = time(NULL);
            m_hb_bms_ver = (*(lpdata+3) << 8) + *(lpdata+4);
            printf("m_hb_bms_ver = 0x%04X\n", m_hb_bms_ver);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybridBMSVer CRC Error ####\n");
                SaveLog((char *)"DataLogger GetHybridBMSVer() : CRC Error", log_time);
            }
            else {
                printf("#### GetHybridBMSVer No Response ####\n");
                SaveLog((char *)"DataLogger GetHybridBMSVer() : No Response", log_time);
            }
            err++;
        }
    }

    return false;
}

void CG320::DumpHybridBMSInfo(unsigned char *buf)
{
    m_hb_bms_info.Voltage = (*(buf) << 8) + *(buf+1);
    m_hb_bms_info.Current = (*(buf+2) << 8) + *(buf+3);
    m_hb_bms_info.SOC = (*(buf+4) << 8) + *(buf+5);
    m_hb_bms_info.MaxTemperature = (*(buf+6) << 8) + *(buf+7);
    m_hb_bms_info.CycleCount = (*(buf+8) << 8) + *(buf+9);
    m_hb_bms_info.Status = (*(buf+10) << 8) + *(buf+11);
    m_hb_bms_info.Error = (*(buf+12) << 8) + *(buf+13);
    m_hb_bms_info.Number = (*(buf+14) << 8) + *(buf+15);
    m_hb_bms_info.BMS_Info = (*(buf+16) << 8) + *(buf+17);
    m_hb_bms_info.BMS_Max_Cell = (*(buf+18) << 8) + *(buf+19);
    m_hb_bms_info.BMS_Min_Cell = (*(buf+20) << 8) + *(buf+21);
    m_hb_bms_info.BMS_BaudRate = (*(buf+22) << 8) + *(buf+23);

/*    printf("#### Dump Hybrid BMS Info ####\n");
    printf("Voltage           = %d mV\n", m_hb_bms_info.Voltage*10);
    printf("Current           = %d mA\n", m_hb_bms_info.Current*10);
    printf("SOC               = %d %%\n", m_hb_bms_info.SOC);
    printf("MaxTemperature    = %d C\n", m_hb_bms_info.MaxTemperature);
    printf("Cycle Count       = %d\n", m_hb_bms_info.CycleCount);
    printf("Status            = %x\n", m_hb_bms_info.Status);
    printf("Error             = 0x%04X\n", m_hb_bms_info.Error);
    printf("Module Number     = %d\n", m_hb_bms_info.Number);
    printf("BMS Info          = %d\n", m_hb_bms_info.BMS_Info);
    printf("BMS Max Cell      = %d mV\n", m_hb_bms_info.BMS_Max_Cell);
    printf("BMS Min Cell      = %d mV\n", m_hb_bms_info.BMS_Min_Cell);
    printf("BMS BaudRate      = %d bps\n", m_hb_bms_info.BMS_BaudRate);
    printf("##############################\n");*/
}

bool CG320::SetHybridBMSModule(int index)
{
    int i = 0;
    bool ret = true;

    // save Panasonic module
    //if ( GetHybridPanasonicModule(index) )
    //    SetHybridPanasonicModule(index);

    // save all 16 module
    for (i = 0; i < 16; i++) {
        if ( GetHybridBMSModule(index, i) ) {
            if ( !SetBMSFile(index, i) )
                ret = false;
            usleep(100000);
        }
    }

    return ret;
}

// not use now
bool CG320::GetHybridPanasonicModule(int index)
{
    printf("#### GetHybridPanasonicModule start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    unsigned char szHBPANAMOD[]={0x00, 0x03, 0x02, 0x09, 0x00, 0x2F, 0x00, 0x00}; // get 0x209 ~ 0x237, total 47 = 0x2F
    szHBPANAMOD[0]=arySNobj[index].m_Addr;
    MakeReadDataCRC(szHBPANAMOD,8);

    MClearRX();
    txsize=8;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBPANAMOD, 8);
        MStartTX(m_busfd);
        /*if ( err == 0 )
            usleep(m_dl_config.m_delay_time_2);
        else if ( err == 1 )
            usleep(m_dl_config.m_delay_time_2*5);
        else
            usleep(m_dl_config.m_delay_time_2*10);*/

        lpdata = GetRespond(m_busfd, 99, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybridPanasonicModule OK ####\n");
            //SaveLog("DataLogger GetHybridPanasonicModule() : OK", m_st_time);
            arySNobj[index].m_ok_time = time(NULL);
            memcpy(m_bms_panamod, lpdata+3, BMS_PANAMOD_SIZE);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybridPanasonicModule CRC Error ####\n");
                SaveLog((char *)"DataLogger GetHybridPanasonicModule() : CRC Error", m_st_time);
            }
            else {
                printf("#### GetHybridPanasonicModule No Response ####\n");
                SaveLog((char *)"DataLogger GetHybridPanasonicModule() : No Response", m_st_time);
            }
            err++;
        }
    }

    return false;
}

bool CG320::SetHybridPanasonicModule(int index)
{
    printf("#### SetHybridPanasonicModule start ####\n");

    char buf[256] = {0};
    int i = 0;
    //struct stat st;

    /*if ( stat(m_bms_filename, &st) == -1 ) {
        // set header
        strcpy(m_bms_mainbuf, "time,from/end address");
        for ( i = 0x209; i < 0x5B8; i++ ) {
            memset(buf, 0, 256);
            sprintf(buf, ",0x%03X", i);
            strcat(m_bms_mainbuf, buf);
        }
        strcat(m_bms_mainbuf, "\n");
    }*/

    // because all data = 0, for test, change value //
    /*m_bms_panamod[0] = '0';
    m_bms_panamod[1] = 's';
    m_bms_panamod[2] = 't';
    m_bms_panamod[3] = 'a';
    m_bms_panamod[4] = 'r';
    m_bms_panamod[5] = 't';
    m_bms_panamod[90] = '0';
    m_bms_panamod[91] = 'e';
    m_bms_panamod[92] = 'n';
    m_bms_panamod[93] = 'd';*/
    //////////////////////////////////////////////////
    memset(buf, 0, 256);
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:00,0x209-0x5B7", 1900+m_data_st_time.tm_year, 1+m_data_st_time.tm_mon, m_data_st_time.tm_mday,
                    m_data_st_time.tm_hour, m_data_st_time.tm_min);
    strcpy(m_bms_mainbuf, buf);
    for ( i = 0; i < BMS_PANAMOD_SIZE; i+=2 ) {
        sprintf(buf, ",0x%02X%02X", m_bms_panamod[i], m_bms_panamod[i+1]);
        strcat(m_bms_mainbuf, buf);
    }

    printf("##### SetHybridPanasonicModule end #####\n");
    //SaveLog("DataLogger SetHybridPanasonicModule() : OK", m_st_time);

    return true;
}

// read data Max size is 100, module data size is 112, so read twice which module data
bool CG320::GetHybridBMSModule(int index, int module)
{
    int err = 0, addr = 0;
    unsigned char addrHi = 0, addrLo = 0;
    byte *lpdata = NULL;
    char buf[256] = {0};
    //unsigned char testbuf[256] = {0};
    //for(int i=0; i<256; i++)
    //    testbuf[i] = (unsigned char)i;
    time_t current_time;
	struct tm *log_time;

    addr = 0x238 + module * 0x38;
    addrHi = (addr >> 8) & 0x00FF;
    addrLo = addr & 0x00FF;

    unsigned char szHBBMSmodule[]={0x00, 0x03, 0x02, 0x00, 0x00, 0x1C, 0x00, 0x00};
    szHBBMSmodule[0]=arySNobj[index].m_Addr;
    szHBBMSmodule[2]=addrHi;
    szHBBMSmodule[3]=addrLo;
    MakeReadDataCRC(szHBBMSmodule,8);

    MClearRX();
    txsize=8;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBBMSmodule, 8);
        MStartTX(m_busfd);
        /*if ( err == 0 )
            usleep(m_dl_config.m_delay_time_2);
        else if ( err == 1 )
            usleep(m_dl_config.m_delay_time_2*5);
        else
            usleep(m_dl_config.m_delay_time_2*10);*/

        current_time = time(NULL);
		log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 61, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybridBMSModule first half OK ####\n");
            //sprintf(buf, "DataLogger GetHybridBMSModule() : addr %d, module %d, first half OK", arySNobj[index].m_Addr, module+1);
            //SaveLog(buf, log_time);
            //lpdata = testbuf + module;
            arySNobj[index].m_ok_time = time(NULL);
            memcpy(m_bms_buf, lpdata+3, BMS_MODULE_SIZE/2);
            break;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybridBMSModule first half CRC Error ####\n");
                sprintf(buf, "DataLogger GetHybridBMSModule() : addr %d, module %d, first half CRC Error", arySNobj[index].m_Addr, module+1);
                SaveLog(buf, log_time);
            }
            else {
                printf("#### GetHybridBMSModule first half No Response ####\n");
                sprintf(buf, "DataLogger GetHybridBMSModule() : addr %d, module %d, first half No Response", arySNobj[index].m_Addr, module+1);
                SaveLog(buf, log_time);
            }
            err++;
        }
    }

    if ( err == 3 )
        return false;

    err = 0;

    addr = 0x238 + module * 0x38 + 0x1C;
    addrHi = (addr >> 8) & 0x00FF;
    addrLo = addr & 0x00FF;
    szHBBMSmodule[2]=addrHi;
    szHBBMSmodule[3]=addrLo;
    MakeReadDataCRC(szHBBMSmodule,8);

    MClearRX();
    txsize=8;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBBMSmodule, 8);
        MStartTX(m_busfd);
        /*if ( err == 0 )
            usleep(m_dl_config.m_delay_time_2);
        else if ( err == 1 )
            usleep(m_dl_config.m_delay_time_2*5);
        else
            usleep(m_dl_config.m_delay_time_2*10);*/

        current_time = time(NULL);
		log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 61, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybridBMSModule last half OK ####\n");
            //sprintf(buf, "DataLogger GetHybridBMSModule() : addr %d, module %d, last half OK", arySNobj[index].m_Addr, module+1);
            //SaveLog(buf, log_time);
            //lpdata = testbuf + module + BMS_MODULE_SIZE/2;
            memcpy(m_bms_buf + BMS_MODULE_SIZE/2, lpdata+3, BMS_MODULE_SIZE/2);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybridBMSModule last half CRC Error ####\n");
                sprintf(buf, "DataLogger GetHybridBMSModule() : addr %d, module %d, last half CRC Error", arySNobj[index].m_Addr, module+1);
                SaveLog(buf, log_time);
            }
            else {
                printf("#### GetHybridBMSModule last half No Response ####\n");
                sprintf(buf, "DataLogger GetHybridBMSModule() : addr %d, module %d, last half No Response", arySNobj[index].m_Addr, module+1);
                SaveLog(buf, log_time);
            }
            err++;
        }
    }

    return false;
}

bool CG320::SetBMSFile(int index, int module)
{
    char buf[256] = {0};
    int i = 0;

    // because all data = 0, for test, change value //
    /*m_bms_buf[0] = module + 65;
    m_bms_buf[1] = 's';
    m_bms_buf[2] = 't';
    m_bms_buf[3] = 'a';
    m_bms_buf[4] = 'r';
    m_bms_buf[5] = 't';
    m_bms_buf[108] = module + 65;
    m_bms_buf[109] = 'e';
    m_bms_buf[110] = 'n';
    m_bms_buf[111] = 'd';*/
    //////////////////////////////////////////////////

    // add header
    if ( module == 0 ) {
        memset(buf, 0, 256);
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:00,0x238-0x5B7", 1900+m_data_st_time.tm_year, 1+m_data_st_time.tm_mon, m_data_st_time.tm_mday,
                        m_data_st_time.tm_hour, m_data_st_time.tm_min);
        strcpy(m_bms_mainbuf, buf);
    }

    for ( i = 0; i < BMS_MODULE_SIZE; i+=2 ) {
        sprintf(buf, ",0x%02X%02X", m_bms_buf[i], m_bms_buf[i+1]);
        strcat(m_bms_mainbuf, buf);
    }
    if ( module == 15 )
        strcat(m_bms_mainbuf, "\n");

    printf("#### SetBMSFile index%d Module%d OK ####\n", index, module+1);
    //sprintf(buf, "DataLogger SetBMSFile() : addr %d, module %d, OK", arySNobj[index].m_Addr, module+1);
    //SaveLog(buf, m_st_time);

    return true;
}

bool CG320::GetHybrid2IDData(int index)
{
    printf("#### GetHybrid2IDData start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    struct tm *log_time;
    // check ok time
    time_t current_time = 0;
    current_time = time(NULL);
    if ( current_time - arySNobj[index].m_ok_time >= OFFLINE_SECOND_HB ) {
        printf("Last m_ok_time more then 1200 sec.\n");
        ReRegister(index);
    }

    unsigned char szHBIDdata[]={0x00, 0x03, 0x00, 0x01, 0x00, 0x0D, 0x00, 0x00};

    while ( err < 3 ) {
        // set buf
        // slave id
        szHBIDdata[0]=arySNobj[index].m_Addr;
        // function code
        szHBIDdata[1]=0x03;
        // start address 2 byte
        szHBIDdata[2]=0x00;
        szHBIDdata[3]=0x01;
        // no, of data 2 byte
        szHBIDdata[4]=0x00;
        szHBIDdata[5]=0x0D;
        // crc
        MakeReadDataCRC(szHBIDdata,8);

        MClearRX();
        txsize=8;
        waitAddr = arySNobj[index].m_Addr;
        waitFCode = 0x03;
        memcpy(txbuffer, szHBIDdata, 8);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        current_time = time(NULL);
		log_time = localtime(&current_time);

        lpdata = GetRespond(m_busfd, 31, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybrid2IDData OK ####\n");
            //SaveLog((char *)"DataLogger GetHybrid2IDData() : OK", log_time);
            DumpHybrid2IDData(lpdata+3);
            arySNobj[index].m_ok_time = time(NULL);
            ParserHybrid2IDFlags(m_hb2_id_data.Safety_Control);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybrid2IDData CRC Error ####\n");
                SaveLog((char *)"DataLogger GetHybrid2IDData() : CRC Error", log_time);
            }
            else {
                printf("#### GetHybrid2IDData No Response ####\n");
                SaveLog((char *)"DataLogger GetHybrid2IDData() : No Response", log_time);
                //SaveLog((char *)"DataLogger GetHybrid2IDData() : run reregister()", log_time);
                //ReRegister(index);
            }
            err++;
        }
    }

    return false;
}

void CG320::DumpHybrid2IDData(unsigned char *buf)
{
    m_hb2_id_data.Grid_Voltage = (*(buf) << 8) + *(buf+1);
    m_hb2_id_data.Model = (*(buf+2) << 8) + *(buf+3);
    m_hb2_id_data.SN_Hi = (*(buf+4) << 8) + *(buf+5);
    m_hb2_id_data.SN_Lo = (*(buf+6) << 8) + *(buf+7);
    m_hb2_id_data.Year = (*(buf+8) << 8) + *(buf+9);
    m_hb2_id_data.Month = (*(buf+10) << 8) + *(buf+11);
    m_hb2_id_data.Date = (*(buf+12) << 8) + *(buf+13);
    m_hb2_id_data.Inverter_Ver = (*(buf+14) << 8) + *(buf+15);
    m_hb2_id_data.DD_Ver = (*(buf+16) << 8) + *(buf+17);
    m_hb2_id_data.Parameter_Ver = (*(buf+18) << 8) + *(buf+19);
    m_hb2_id_data.Display_Ver = (*(buf+20) << 8) + *(buf+21);
    m_hb2_id_data.HW_Ver = (*(buf+22) << 8) + *(buf+23);
    m_hb2_id_data.Safety_Control = (*(buf+24) << 8) + *(buf+25);

    printf("#### Dump Hybrid2 ID Data ####\n");
    printf("Grid_Voltage   = %d ==> ", m_hb2_id_data.Grid_Voltage);
    switch (m_hb2_id_data.Grid_Voltage)
    {
        case 0:
            printf("240V\n");
            break;
        case 1:
            printf("230V\n");
            break;
        case 2:
            printf("220V\n");
            break;
        case 3:
            printf("208V\n");
            break;
        case 4:
            printf("208V ~ 240V\n");
            break;
        case 5:
            printf("200V\n");
            break;
        default:
            printf("other\n");
            break;
    }
    printf("Model          = %d ==> ", m_hb2_id_data.Model);
    switch (m_hb_id_data.Model)
    {
        case 1:
            printf("H5000\n");
            break;
        case 2:
            printf("H5001\n");
            break;
        case 3:
            printf("HB5\n");
            break;
        case 4:
            printf("HB51\n");
            break;
        case 5:
            printf("H5001 for P\n");
            break;
        case 6:
            printf("HB51 for P\n");
            break;
        case 7:
            printf("H5001 STK\n");
            break;
        case 8:
            printf("HB51 STK\n");
            break;
        case 9:
            printf("H5001 for E\n");
            break;
        case 10:
            printf("HB51 for E\n");
            break;
        case 16:
            printf("H5500\n");
            break;
        case 17:
            printf("H9600\n");
            break;
        default:
            printf("Other\n");
    }
    printf("SN_Hi          = 0x%04X\n", m_hb2_id_data.SN_Hi);
    printf("SN_Lo          = 0x%04X\n", m_hb2_id_data.SN_Lo);
    printf("Year           = %d\n", m_hb2_id_data.Year);
    printf("Month          = %02d\n", m_hb2_id_data.Month);
    printf("Date           = %02d\n", m_hb2_id_data.Date);
    printf("Inverter_Ver   = 0x%04X\n", m_hb2_id_data.Inverter_Ver);
    printf("DD_Ver         = %d\n", m_hb2_id_data.DD_Ver);
    printf("Parameter_Ver  = %d\n", m_hb2_id_data.Parameter_Ver);
    printf("Display_Ver    = %d\n", m_hb2_id_data.Display_Ver);
    printf("HW_Ver         = 0x%02X\n", m_hb2_id_data.HW_Ver);
    printf("Safety_Control = 0x%02X\n", m_hb2_id_data.Safety_Control);
    printf("#############################\n");
}

bool CG320::SetHybrid2IDData(int index)
{
    printf("#### SetHybrid2IDData Start ####\n");

    int err = 0;
    unsigned short crc;
    byte *lpdata = NULL;

    unsigned char szIDData[39]={0};
    szIDData[0] = arySNobj[index].m_Addr;
    szIDData[1] = 0x10; // function code
    szIDData[2] = 0x00;
    szIDData[3] = 0x01; // star address
    szIDData[4] = 0x00;
    szIDData[5] = 0x0F; // number of data
    szIDData[6] = 0x1E; // bytes
    // data 0x01 ~ 0x0B, & flags at 0x0D, 0x0E
    szIDData[7] = 0x00;
    szIDData[8] = (unsigned char)m_hb2_id_data.Grid_Voltage;
    szIDData[9] = 0x00; //(unsigned char)m_hb_id_data.HW_Ver;
    szIDData[10] = (unsigned char)m_hb2_id_data.Model;
    szIDData[11] = (unsigned char)((m_hb2_id_data.SN_Hi >> 8) & 0x00FF);
    szIDData[12] = (unsigned char)(m_hb2_id_data.SN_Hi & 0x00FF);
    szIDData[13] = (unsigned char)((m_hb2_id_data.SN_Lo >> 8) & 0x00FF);
    szIDData[14] = (unsigned char)(m_hb2_id_data.SN_Lo & 0x00FF);
    szIDData[15] = (unsigned char)((m_hb2_id_data.Year >> 8) & 0x00FF);
    szIDData[16] = (unsigned char)(m_hb2_id_data.Year & 0x00FF);
    szIDData[17] = 0x00;
    szIDData[18] = (unsigned char)m_hb2_id_data.Month;
    szIDData[19] = 0x00;
    szIDData[20] = (unsigned char)m_hb2_id_data.Date;
    szIDData[21] = (unsigned char)((m_hb2_id_data.Inverter_Ver >> 8) & 0x00FF);
    szIDData[22] = (unsigned char)(m_hb2_id_data.Inverter_Ver & 0xFF);
    szIDData[23] = (unsigned char)((m_hb2_id_data.DD_Ver >> 8) & 0x00FF);
    szIDData[24] = (unsigned char)(m_hb2_id_data.DD_Ver & 0xFF);
    szIDData[25] = (unsigned char)((m_hb2_id_data.Parameter_Ver >> 8) & 0x00FF);
    szIDData[26] = (unsigned char)(m_hb2_id_data.Parameter_Ver & 0xFF);
    szIDData[27] = (unsigned char)((m_hb2_id_data.Display_Ver >> 8) & 0x00FF);
    szIDData[28] = (unsigned char)(m_hb2_id_data.Display_Ver & 0xFF);
    szIDData[29] = (unsigned char)((m_hb2_id_data.HW_Ver >> 8) & 0x00FF);
    szIDData[30] = (unsigned char)(m_hb2_id_data.HW_Ver & 0xFF);
    // Safety_Control flag 0x0D
    szIDData[31] = (unsigned char)((m_hb2_id_data.Safety_Control >> 8) & 0x00FF);
    szIDData[32] = (unsigned char)(m_hb2_id_data.Safety_Control & 0xFF);
    // empty
    szIDData[33] = 0x00;
    szIDData[34] = 0x00;
    // data crc 0x0F
    crc = CalculateCRC(szIDData+7, 28);
    szIDData[35] = (unsigned char) (crc >> 8); // data crc hi
    szIDData[36] = (unsigned char) (crc & 0xFF); // data crc lo
    szIDData[37] = 0x00; // cmd crc hi
    szIDData[38] = 0x00; // cmd crc lo
    MakeReadDataCRC(szIDData,39);
    MClearRX();
    txsize=39;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x10;

    while ( err < 3 ) {
        memcpy(txbuffer, szIDData, 39);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        lpdata = GetRespond(m_busfd, 8, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 8) ) {
                printf("#### SetHybrid2IDData OK ####\n");
                SaveLog((char *)"DataLogger SetHybrid2IDData() : OK", m_st_time);
                arySNobj[index].m_ok_time = time(NULL);
                //free(lpdata);
                return true;
            } else {
                printf("#### SetHybrid2IDData CRC Error ####\n");
                SaveLog((char *)"DataLogger SetHybrid2IDData() : CRC Error", m_st_time);
                err++;
            }
            //free(lpdata);
        } else {
            printf("#### SetHybrid2IDData No Response ####\n");
            SaveLog((char *)"DataLogger SetHybrid2IDData() : No Response", m_st_time);
            err++;
        }

        usleep(1000000);
    }

    return false;
}

void CG320::ParserHybrid2IDFlags(int flags)
{
    int tmp = flags;

    m_hb2_id_flags.B0_Rule21 = tmp & 0x01;
    tmp>>=1;
    m_hb2_id_flags.B1_SunSpec = tmp & 0x01;
    tmp>>=1;
    m_hb2_id_flags.B2_Heco1 = tmp & 0x01;
    tmp>>=1;
    m_hb2_id_flags.B3_Heco2 = tmp & 0x01;
    tmp>>=1;
    m_hb2_id_flags.B4_PREPA = tmp & 0x01;
    tmp>>=1;
    m_hb2_id_flags.B5_Dominion = tmp & 0x01;
    tmp>>=1;
    m_hb2_id_flags.B6_MA = tmp & 0x01;

    printf("#### Parser Hybrid2 ID Flags2 ####\n");
    printf("Bit0 : Rule21              = %d\n", m_hb2_id_flags.B0_Rule21);
    printf("Bit1 : SunSpec             = %d\n", m_hb2_id_flags.B1_SunSpec);
    printf("Bit2 : Heco1               = %d\n", m_hb2_id_flags.B2_Heco1);
    printf("Bit3 : Heco2               = %d\n", m_hb2_id_flags.B3_Heco2);
    printf("Bit4 : PREPA               = %d\n", m_hb2_id_flags.B4_PREPA);
    printf("Bit5 : Dominion            = %d\n", m_hb2_id_flags.B5_Dominion);
    printf("Bit6 : MA                  = %d\n", m_hb2_id_flags.B6_MA);
    printf("################################\n");
}

void CG320::DumpHybrid2RSInfo(unsigned char *buf)
{
// 0x00B0
    m_hb2_rs_info.Mode = (*(buf) << 8) + *(buf+1);
    m_hb2_rs_info.BatteryPolic = (*(buf+2) << 8) + *(buf+3);
    m_hb2_rs_info.MultiModuleSetting = (*(buf+4) << 8) + *(buf+5);
    m_hb2_rs_info.BatteryType = (*(buf+6) << 8) + *(buf+7);
    m_hb2_rs_info.ChargePower = (*(buf+8) << 8) + *(buf+9);
    m_hb2_rs_info.DischargePower = (*(buf+10) << 8) + *(buf+11);
    m_hb2_rs_info.FeedinPower = (*(buf+12) << 8) + *(buf+13);
    m_hb2_rs_info.BatteryPowerRating = (*(buf+14) << 8) + *(buf+15);
    m_hb2_rs_info.MaxBatteryChargingCurrent = (*(buf+16) << 8) + *(buf+17);
    m_hb2_rs_info.BatteryShutdownVoltage = (*(buf+18) << 8) + *(buf+19);
    m_hb2_rs_info.BatteryAbsorptionChargingVoltage = (*(buf+20) << 8) + *(buf+21);
    m_hb2_rs_info.BatteryReservePercentage = (*(buf+22) << 8) + *(buf+23);
    m_hb2_rs_info.GridTiedMaxSOC = (*(buf+24) << 8) + *(buf+25);
    m_hb2_rs_info.PeakShavingPower = (*(buf+26) << 8) + *(buf+27);
    m_hb2_rs_info.StartFrequency = (*(buf+28) << 8) + *(buf+29);
    m_hb2_rs_info.EndFrequency = (*(buf+30) << 8) + *(buf+31);
// 0x00C0
    m_hb2_rs_info.RampRatePercentage = (*(buf+32) << 8) + *(buf+33);
    m_hb2_rs_info.DegreeLeadLag = (*(buf+34) << 8) + *(buf+35);
    m_hb2_rs_info.PowerFactorControl = (*(buf+36) << 8) + *(buf+37);
    // 0x00C3 empty
    m_hb2_rs_info.TMP_C3 = (*(buf+38) << 8) + *(buf+39);
    m_hb2_rs_info.SpecifiedModbusSlaveID = (*(buf+40) << 8) + *(buf+41);
    m_hb2_rs_info.FunctionControl1 = (*(buf+42) << 8) + *(buf+43);
    m_hb2_rs_info.FunctionControl2 = (*(buf+44) << 8) + *(buf+45);
    m_hb2_rs_info.SunSpecWritable = (*(buf+46) << 8) + *(buf+47);
    m_hb2_rs_info.SoftwareControl = (*(buf+48) << 8) + *(buf+49);
    // 0x00C9 ~ 0x00CF empty
// 0x00D0
    m_hb2_rs_info.TOU_Season1_Operation_Mode = (*(buf+64) << 8) + *(buf+65);
    m_hb2_rs_info.TOU_OtherSeason_Operation_Mode = (*(buf+66) << 8) + *(buf+67);
    m_hb2_rs_info.TOU_Season1_Start_Month = (*(buf+68) << 8) + *(buf+69);
    m_hb2_rs_info.TOU_Season1_Start_Day = (*(buf+70) << 8) + *(buf+71);
    m_hb2_rs_info.TOU_Season1_End_Month = (*(buf+72) << 8) + *(buf+73);
    m_hb2_rs_info.TOU_Season1_End_Day = (*(buf+74) << 8) + *(buf+75);
    m_hb2_rs_info.Peak_Period_Start_Hour1_of_Season1 = (*(buf+76) << 8) + *(buf+77);
    m_hb2_rs_info.Peak_Period_Start_Minute1_of_Season1 = (*(buf+78) << 8) + *(buf+79);
    m_hb2_rs_info.Peak_Period_End_Hour1_of_Season1 = (*(buf+80) << 8) + *(buf+81);
    m_hb2_rs_info.Peak_Period_End_Minute1_of_Season1 = (*(buf+82) << 8) + *(buf+83);
    m_hb2_rs_info.Peak_Period_Start_Hour2_of_Season1 = (*(buf+84) << 8) + *(buf+85);
    m_hb2_rs_info.Peak_Period_Start_Minute2_of_Season1 = (*(buf+86) << 8) + *(buf+87);
    m_hb2_rs_info.Peak_Period_End_Hour2_of_Season1 = (*(buf+88) << 8) + *(buf+89);
    m_hb2_rs_info.Peak_Period_End_Minute2_of_Season1 = (*(buf+90) << 8) + *(buf+91);
    m_hb2_rs_info.Peak_Period_Start_Hour1_of_OtherSeason = (*(buf+92) << 8) + *(buf+93);
    m_hb2_rs_info.Peak_Period_Start_Minute1_of_OtherSeason = (*(buf+94) << 8) + *(buf+95);
// 0x00E0
    m_hb2_rs_info.Peak_Period_End_Hour1_of_OtherSeason = (*(buf+96) << 8) + *(buf+97);
    m_hb2_rs_info.Peak_Period_End_Minute1_of_OtherSeason = (*(buf+98) << 8) + *(buf+99);
    m_hb2_rs_info.Peak_Period_Start_Hour2_of_OtherSeason = (*(buf+100) << 8) + *(buf+101);
    m_hb2_rs_info.Peak_Period_Start_Minute2_of_OtherSeason = (*(buf+102) << 8) + *(buf+103);
    m_hb2_rs_info.Peak_Period_End_Hour2_of_OtherSeason = (*(buf+104) << 8) + *(buf+105);
    m_hb2_rs_info.Peak_Period_End_Minute2_of_OtherSeason = (*(buf+106) << 8) + *(buf+107);

    printf("#### Dump Hybrid2 RS Info ####\n");
// 0x00B0
    printf("Mode = %d ==> ", m_hb2_rs_info.Mode);
    switch (m_hb2_rs_info.Mode)
    {
        case 0:
            printf("Backup Mode\n");
            break;
        case 1:
            printf("Saving AC Mode\n");
            break;
        case 2:
            printf("TOU Mode\n");
            break;
        case 3:
            printf("Remote Control Mode\n");
            break;
        case 4:
            printf("Off-Grid Mode\n");
            break;
        case 5:
            printf("Calibrarion Mode\n");
            break;
    }
    printf("Battery Polic = %d ==> ", m_hb2_rs_info.BatteryPolic);
    switch (m_hb2_rs_info.BatteryPolic)
    {
        case 0:
            printf("Standby\n");
            break;
        case 1:
            printf("Charge\n");
            break;
        case 2:
            printf("Discharge\n");
            break;
    }
    printf("Multi Module Setting = %d ==> ", m_hb2_rs_info.MultiModuleSetting);
    switch (m_hb2_rs_info.MultiModuleSetting)
    {
        case 0:
            printf("Single\n");
            break;
        case 1:
            printf("Parallel\n");
            break;
        case 2:
            printf("Three phase\n");
            break;
    }
    printf("Battery Type = %d ==> ", m_hb2_rs_info.BatteryType);
    switch (m_hb2_rs_info.BatteryType)
    {
        case 0:
            printf("No communication\n");
            break;
        case 1:
            printf("Darfon HV\n");
            break;
    }
    printf("Charge Power = %d W\n", m_hb2_rs_info.ChargePower*100);
    printf("Discharge Power = %d W\n", m_hb2_rs_info.DischargePower*100);
    printf("Feed-in Power = %d W\n", m_hb2_rs_info.FeedinPower*100);
    printf("Battery Power Rating = %d W\n", m_hb2_rs_info.BatteryPowerRating*100);
    printf("Max Battery Charging Current = %03.1f A\n", (float)m_hb2_rs_info.MaxBatteryChargingCurrent/10);
    printf("Battery Shutdown Voltage = %03.1f V\n", ((float)m_hb2_rs_info.BatteryShutdownVoltage)/10);
    printf("Battery Absorption Charging Voltage = %03.1f V\n", ((float)m_hb2_rs_info.BatteryAbsorptionChargingVoltage)/10);
    printf("Battery Reserve Percentage = %d%%\n", m_hb2_rs_info.BatteryReservePercentage);
    printf("Grid Tied Max SOC = %d%%\n", m_hb2_rs_info.GridTiedMaxSOC);
    printf("Peak Shaving Power = %d W\n", m_hb2_rs_info.PeakShavingPower*100);
    printf("Start Frequency = %03.1f Hz\n", (float)m_hb2_rs_info.StartFrequency/10);
    printf("End Frequency = %03.1f Hz\n", (float)m_hb2_rs_info.EndFrequency/10);

// 0x00C0
    printf("Ramp Rate Percentage = %03.1f%%\n", (float)m_hb2_rs_info.RampRatePercentage/10);
    printf("Degree LeadLag = %d ==> ", m_hb2_rs_info.DegreeLeadLag);
    switch (m_hb2_rs_info.DegreeLeadLag/256)
    {
        case 0:
            printf("Disable\n");
            break;
        case 1:
            printf("Lead\n");
            break;
        case 2:
            printf("Lag\n");
            break;
    }
    printf("TMP_C3 = %d\n", m_hb2_rs_info.TMP_C3);
    printf("Power Factor Control = %d\n", m_hb2_rs_info.PowerFactorControl);
    printf("Specified Modbus SlaveID = 0x%04X\n", m_hb2_rs_info.SpecifiedModbusSlaveID);
    printf("Function Control 1 = 0x%04X\n", m_hb2_rs_info.FunctionControl1);
    printf("Function Control 2 = 0x%04X\n", m_hb2_rs_info.FunctionControl2);
    printf("SunSpec Writable = 0x%04X\n", m_hb2_rs_info.SunSpecWritable);
    printf("Software Control = 0x%04X\n", m_hb2_rs_info.SoftwareControl);

// 0x00D0
    printf("TOU_Season1_Operation_Mode = 0x%04X\n", m_hb2_rs_info.TOU_Season1_Operation_Mode);
    printf("TOU_OtherSeason_Operation_Mode = 0x%04X\n", m_hb2_rs_info.TOU_OtherSeason_Operation_Mode);
    printf("TOU_Season1_Start_Month = %d\n", m_hb2_rs_info.TOU_Season1_Start_Month);
    printf("TOU_Season1_Start_Day = %d\n", m_hb2_rs_info.TOU_Season1_Start_Day);
    printf("TOU_Season1_End_Month = %d\n", m_hb2_rs_info.TOU_Season1_End_Month);
    printf("TOU_Season1_End_Day = %d\n", m_hb2_rs_info.TOU_Season1_End_Day);
    printf("Peak_Period_Start_Hour1_of_Season1 = %d\n", m_hb2_rs_info.Peak_Period_Start_Hour1_of_Season1);
    printf("Peak_Period_Start_Minute1_of_Season1 = %d\n", m_hb2_rs_info.Peak_Period_Start_Minute1_of_Season1);
    printf("Peak_Period_End_Hour1_of_Season1 = %d\n", m_hb2_rs_info.Peak_Period_End_Hour1_of_Season1);
    printf("Peak_Period_End_Minute1_of_Season1 = %d\n", m_hb2_rs_info.Peak_Period_End_Minute1_of_Season1);
    printf("Peak_Period_Start_Hour2_of_Season1 = %d\n", m_hb2_rs_info.Peak_Period_Start_Hour2_of_Season1);
    printf("Peak_Period_Start_Minute2_of_Season1 = %d\n", m_hb2_rs_info.Peak_Period_Start_Minute2_of_Season1);
    printf("Peak_Period_End_Hour2_of_Season1 = %d\n", m_hb2_rs_info.Peak_Period_End_Hour2_of_Season1);
    printf("Peak_Period_End_Minute2_of_Season1 = %d\n", m_hb2_rs_info.Peak_Period_End_Minute2_of_Season1);
    printf("Peak_Period_Start_Hour1_of_OtherSeason = %d\n", m_hb2_rs_info.Peak_Period_Start_Hour1_of_OtherSeason);
    printf("Peak_Period_Start_Minute1_of_OtherSeason = %d\n", m_hb2_rs_info.Peak_Period_Start_Minute1_of_OtherSeason);

// 0x00E0
    printf("Peak_Period_End_Hour1_of_OtherSeason = %d\n", m_hb2_rs_info.Peak_Period_End_Hour1_of_OtherSeason);
    printf("Peak_Period_End_Minute1_of_OtherSeason = %d\n", m_hb2_rs_info.Peak_Period_End_Minute1_of_OtherSeason);
    printf("Peak_Period_Start_Hour2_of_OtherSeason = %d\n", m_hb2_rs_info.Peak_Period_Start_Hour2_of_OtherSeason);
    printf("Peak_Period_Start_Minute2_of_OtherSeason = %d\n", m_hb2_rs_info.Peak_Period_Start_Minute2_of_OtherSeason);
    printf("Peak_Period_End_Hour2_of_OtherSeason = %d\n", m_hb2_rs_info.Peak_Period_End_Hour2_of_OtherSeason);
    printf("Peak_Period_End_Minute2_of_OtherSeason = %d\n", m_hb2_rs_info.Peak_Period_End_Minute2_of_OtherSeason);

    printf("#############################\n");
}

bool CG320::SetHybrid2RSInfo1(int index)
{
    printf("#### SetHybrid2RSInfo1 Start ####\n");

    int err = 0;
    //unsigned short crc;
    byte *lpdata = NULL;

    unsigned char szRSInfo[41]={0};
    szRSInfo[0] = arySNobj[index].m_Addr;
    szRSInfo[1] = 0x10; // function code
    szRSInfo[2] = 0x00;
    szRSInfo[3] = 0xB0; // star address
    szRSInfo[4] = 0x00;
    szRSInfo[5] = 0x10; // number of data
    szRSInfo[6] = 0x20; // bytes
    // data 0xB0 ~ 0xBF
    szRSInfo[7] = 0x00;
    szRSInfo[8] = (unsigned char)m_hb2_rs_info.Mode;
    szRSInfo[9] = 0x00;
    szRSInfo[10] = (unsigned char)m_hb2_rs_info.BatteryPolic;
    szRSInfo[11] = 0x00;
    szRSInfo[12] = (unsigned char)m_hb2_rs_info.MultiModuleSetting;
    szRSInfo[13] = 0x00;
    szRSInfo[14] = (unsigned char)m_hb2_rs_info.BatteryType;
    szRSInfo[15] = (unsigned char)((m_hb2_rs_info.ChargePower >> 8) & 0xFF);
    szRSInfo[16] = (unsigned char)(m_hb2_rs_info.ChargePower & 0xFF);
    szRSInfo[17] = (unsigned char)((m_hb2_rs_info.DischargePower >> 8) & 0xFF);
    szRSInfo[18] = (unsigned char)(m_hb2_rs_info.DischargePower & 0xFF);
    szRSInfo[19] = (unsigned char)((m_hb2_rs_info.FeedinPower >> 8) & 0xFF);
    szRSInfo[20] = (unsigned char)(m_hb2_rs_info.FeedinPower & 0xFF);
    szRSInfo[21] = (unsigned char)((m_hb2_rs_info.BatteryPowerRating >> 8) & 0xFF);
    szRSInfo[22] = (unsigned char)(m_hb2_rs_info.BatteryPowerRating & 0xFF);
    szRSInfo[23] = (unsigned char)((m_hb2_rs_info.MaxBatteryChargingCurrent >> 8) & 0xFF);
    szRSInfo[24] = (unsigned char)(m_hb2_rs_info.MaxBatteryChargingCurrent & 0xFF);
    szRSInfo[25] = (unsigned char)((m_hb2_rs_info.BatteryShutdownVoltage >> 8) & 0xFF);
    szRSInfo[26] = (unsigned char)(m_hb2_rs_info.BatteryShutdownVoltage & 0xFF);
    szRSInfo[27] = (unsigned char)((m_hb2_rs_info.BatteryAbsorptionChargingVoltage >> 8) & 0xFF);
    szRSInfo[28] = (unsigned char)(m_hb2_rs_info.BatteryAbsorptionChargingVoltage & 0xFF);
    szRSInfo[29] = (unsigned char)((m_hb2_rs_info.BatteryReservePercentage >> 8) & 0xFF);
    szRSInfo[30] = (unsigned char)(m_hb2_rs_info.BatteryReservePercentage & 0xFF);
    szRSInfo[31] = (unsigned char)((m_hb2_rs_info.GridTiedMaxSOC >> 8) & 0xFF);
    szRSInfo[32] = (unsigned char)(m_hb2_rs_info.GridTiedMaxSOC & 0xFF);
    szRSInfo[33] = (unsigned char)((m_hb2_rs_info.PeakShavingPower >> 8) & 0xFF);
    szRSInfo[34] = (unsigned char)(m_hb2_rs_info.PeakShavingPower & 0xFF);
    szRSInfo[35] = (unsigned char)((m_hb2_rs_info.StartFrequency >> 8) & 0xFF);
    szRSInfo[36] = (unsigned char)(m_hb2_rs_info.StartFrequency & 0xFF);
    szRSInfo[37] = (unsigned char)((m_hb2_rs_info.EndFrequency >> 8) & 0xFF);
    szRSInfo[38] = (unsigned char)(m_hb2_rs_info.EndFrequency & 0xFF);
    // data crc 0x9F
    //crc = CalculateCRC(szRSInfo+7, 30);
    //szRSInfo[37] = (unsigned char) (crc >> 8); // data crc hi
    //szRSInfo[38] = (unsigned char) (crc & 0xFF); // data crc lo
    szRSInfo[39] = 0x00; // cmd crc hi
    szRSInfo[40] = 0x00; // cmd crc lo
    MakeReadDataCRC(szRSInfo,41);
    MClearRX();
    txsize=41;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x10;

    while ( err < 3 ) {
        memcpy(txbuffer, szRSInfo, 41);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        lpdata = GetRespond(m_busfd, 8, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 8) ) {
                printf("#### SetHybrid2RSInfo1 OK ####\n");
                SaveLog((char *)"DataLogger SetHybrid2RSInfo1() : OK", m_st_time);
                arySNobj[index].m_ok_time = time(NULL);
                //free(lpdata);
                return true;
            } else {
                printf("#### SetHybrid2RSInfo1 CRC Error ####\n");
                SaveLog((char *)"DataLogger SetHybrid2RSInfo1() : CRC Error", m_st_time);
                err++;
            }
            //free(lpdata);
        } else {
            printf("#### SetHybrid2RSInfo1 No Response ####\n");
            SaveLog((char *)"DataLogger SetHybrid2RSInfo1() : No Response", m_st_time);
            err++;
        }

        usleep(1000000);
    }

    return false;
}

bool CG320::SetHybrid2RSInfo2(int index)
{
    printf("#### SetHybrid2RSInfo2 Start ####\n");

    int err = 0;
    //unsigned short crc;
    byte *lpdata = NULL;

    unsigned char szRSInfo[27]={0};
    szRSInfo[0] = arySNobj[index].m_Addr;
    szRSInfo[1] = 0x10; // function code
    szRSInfo[2] = 0x00;
    szRSInfo[3] = 0xC0; // star address
    szRSInfo[4] = 0x00;
    szRSInfo[5] = 0x09; // number of data
    szRSInfo[6] = 0x12; // bytes
    // data 0xC0 ~ 0xC8
    szRSInfo[7] = (unsigned char)((m_hb2_rs_info.RampRatePercentage >> 8) & 0xFF);
    szRSInfo[8] = (unsigned char)(m_hb2_rs_info.RampRatePercentage & 0xFF);
    szRSInfo[9] = (unsigned char)((m_hb2_rs_info.DegreeLeadLag >> 8) & 0xFF);
    szRSInfo[10] = (unsigned char)(m_hb2_rs_info.DegreeLeadLag & 0xFF);
    szRSInfo[11] = 0x00;
    szRSInfo[12] = (unsigned char)m_hb2_rs_info.PowerFactorControl;
    szRSInfo[13] = (unsigned char)((m_hb2_rs_info.TMP_C3 >> 8) & 0xFF);
    szRSInfo[14] = (unsigned char)(m_hb2_rs_info.TMP_C3 & 0xFF);
    szRSInfo[15] = (unsigned char)((m_hb2_rs_info.SpecifiedModbusSlaveID >> 8) & 0xFF);
    szRSInfo[16] = (unsigned char)(m_hb2_rs_info.SpecifiedModbusSlaveID & 0xFF);
    szRSInfo[17] = (unsigned char)((m_hb2_rs_info.FunctionControl1 >> 8) & 0xFF);
    szRSInfo[18] = (unsigned char)(m_hb2_rs_info.FunctionControl1 & 0xFF);
    szRSInfo[19] = (unsigned char)((m_hb2_rs_info.FunctionControl2 >> 8) & 0xFF);
    szRSInfo[20] = (unsigned char)(m_hb2_rs_info.FunctionControl2 & 0xFF);
    szRSInfo[21] = (unsigned char)((m_hb2_rs_info.SunSpecWritable >> 8) & 0xFF);
    szRSInfo[22] = (unsigned char)(m_hb2_rs_info.SunSpecWritable & 0xFF);
    szRSInfo[23] = (unsigned char)((m_hb2_rs_info.SoftwareControl >> 8) & 0xFF);
    szRSInfo[24] = (unsigned char)(m_hb2_rs_info.SoftwareControl & 0xFF);
/*    szRSInfo[25] = 0x00;
    szRSInfo[26] = 0x00;
    szRSInfo[27] = 0x00;
    szRSInfo[28] = 0x00;
    szRSInfo[29] = 0x00;
    szRSInfo[30] = 0x00;
    szRSInfo[31] = 0x00;
    szRSInfo[32] = 0x00;
    szRSInfo[33] = 0x00;
    szRSInfo[34] = 0x00;
    szRSInfo[35] = 0x00;
    szRSInfo[36] = 0x00;
    szRSInfo[37] = 0x00;
    szRSInfo[38] = 0x00;
    // data crc 0x9F
    crc = CalculateCRC(szRSInfo+7, 30);
    szRSInfo[37] = (unsigned char) (crc >> 8); // data crc hi
    szRSInfo[38] = (unsigned char) (crc & 0xFF); // data crc lo
*/    szRSInfo[25] = 0x00; // cmd crc hi
    szRSInfo[26] = 0x00; // cmd crc lo
    MakeReadDataCRC(szRSInfo,27);
    MClearRX();
    txsize=27;
    waitAddr = arySNobj[index].m_Addr;
    waitFCode = 0x10;

    while ( err < 3 ) {
        memcpy(txbuffer, szRSInfo, 27);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        lpdata = GetRespond(m_busfd, 8, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 8) ) {
                printf("#### SetHybrid2RSInfo2 OK ####\n");
                SaveLog((char *)"DataLogger SetHybrid2RSInfo2() : OK", m_st_time);
                arySNobj[index].m_ok_time = time(NULL);
                //free(lpdata);
                return true;
            } else {
                printf("#### SetHybrid2RSInfo2 CRC Error ####\n");
                SaveLog((char *)"DataLogger SetHybrid2RSInfo2() : CRC Error", m_st_time);
                err++;
            }
            //free(lpdata);
        } else {
            printf("#### SetHybrid2RSInfo2 No Response ####\n");
            SaveLog((char *)"DataLogger SetHybrid2RSInfo2() : No Response", m_st_time);
            err++;
        }

        usleep(1000000);
    }

    return false;
}

void CG320::ParserHybrid2RSFunctionFlags(int flags)
{
    int tmp = flags;

    m_hb2_rs_f_flags.B0_PV_12Parallel = tmp & 0x01;
    tmp>>=1;
    m_hb2_rs_f_flags.B1_PV_Off_Grid = tmp & 0x01;
    tmp>>=1;
    m_hb2_rs_f_flags.B2_AC_Coupling = tmp & 0x01;
    tmp>>=1;
    m_hb2_rs_f_flags.B3_Frequency_Control = tmp & 0x01;
    tmp>>=1;
    m_hb2_rs_f_flags.B4_Arc_detection = tmp & 0x01;
    tmp>>=4;
    m_hb2_rs_f_flags.B8_9_External_Sensor = tmp & 0x0003;
    tmp>>=2;
    m_hb2_rs_f_flags.B10_Generator_charge_battery = tmp & 0x01;
    tmp>>=1;
    m_hb2_rs_f_flags.B11_Battery_Calibration_Enable = tmp & 0x01;
    tmp>>=1;
    m_hb2_rs_f_flags.B12_PV_34Parallel = tmp & 0x01;

    printf("#### Parser Hybrid2 RS Function Flags ####\n");
    printf("Bit0  : PV 1/2 Parallel            = %d\n", m_hb2_rs_f_flags.B0_PV_12Parallel);
    printf("Bit1  : PV Off Grid                = %d\n", m_hb2_rs_f_flags.B1_PV_Off_Grid);
    printf("Bit2  : AC Coupling                = %d\n", m_hb2_rs_f_flags.B2_AC_Coupling);
    printf("Bit3  : Frequency Control          = %d\n", m_hb2_rs_f_flags.B3_Frequency_Control);
    printf("Bit4  : Arc detection              = %d\n", m_hb2_rs_f_flags.B4_Arc_detection);
    printf("Bit8&9: External Sensor            = %d\n", m_hb2_rs_f_flags.B8_9_External_Sensor);
    printf("Bit10 : Generator charge battery   = %d\n", m_hb2_rs_f_flags.B10_Generator_charge_battery);
    printf("Bit11 : Battery Calibration Enable = %d\n", m_hb2_rs_f_flags.B11_Battery_Calibration_Enable);
    printf("Bit12 : PV 3/4 Parallel            = %d\n", m_hb2_rs_f_flags.B12_PV_34Parallel);
    printf("################################\n");
}

void CG320::ParserHybrid2RSSoftwareFlags(int flags)
{
    int tmp = flags;

    m_hb2_rs_s_flags.B0_System_Reset = tmp & 0x01;
    tmp>>=1;
    m_hb2_rs_s_flags.B1_Off_Grid_Enable = tmp & 0x01;
    tmp>>=1;
    m_hb2_rs_s_flags.B2_Enable_Disable = tmp & 0x01;

    printf("#### Parser Hybrid2 RS Software Flags ####\n");
    printf("Bit0 : System Reset            = %d\n", m_hb2_rs_s_flags.B0_System_Reset);
    printf("Bit1 : Off Grid Enable         = %d\n", m_hb2_rs_s_flags.B1_Off_Grid_Enable);
    printf("Bit2 : Enable/Disable          = %d\n", m_hb2_rs_s_flags.B2_Enable_Disable);
    printf("################################\n");
}

void CG320::DumpHybrid2RTInfo(unsigned char *buf)
{
// 0x0100
    m_hb2_rt_info.Inv_Temp = (*(buf) << 8) + *(buf+1);
    m_hb2_rt_info.Charger_Temp = (*(buf+2) << 8) + *(buf+3);
    m_hb2_rt_info.Environment_Temp = (*(buf+4) << 8) + *(buf+5);
    m_hb2_rt_info.DD_Temp = (*(buf+6) << 8) + *(buf+7);
    m_hb2_rt_info.PV1_Voltage = (*(buf+8) << 8) + *(buf+9);
    m_hb2_rt_info.PV1_Current = (*(buf+10) << 8) + *(buf+11);
    m_hb2_rt_info.PV1_Power = (*(buf+12) << 8) + *(buf+13);
    m_hb2_rt_info.PV2_Voltage = (*(buf+14) << 8) + *(buf+15);
    m_hb2_rt_info.PV2_Current = (*(buf+16) << 8) + *(buf+17);
    m_hb2_rt_info.PV2_Power = (*(buf+18) << 8) + *(buf+19);
    m_hb2_rt_info.Load_Voltage = (*(buf+20) << 8) + *(buf+21);
    m_hb2_rt_info.Load_Current = (*(buf+22) << 8) + *(buf+23);
    m_hb2_rt_info.Load_Power = (*(buf+24) << 8) + *(buf+25);
    m_hb2_rt_info.Grid_Voltage = (*(buf+26) << 8) + *(buf+27);
    m_hb2_rt_info.Grid_Current = (*(buf+28) << 8) + *(buf+29);
    m_hb2_rt_info.Grid_Power = (*(buf+30) << 8) + *(buf+31);
// 0x0110
    m_hb2_rt_info.Battery_Voltage = (*(buf+32) << 8) + *(buf+33);
    m_hb2_rt_info.Battery_Current = (*(buf+34) << 8) + *(buf+35);
    m_hb2_rt_info.Battery_Power = (*(buf+36) << 8) + *(buf+37);
    m_hb2_rt_info.Bus_Voltage = (*(buf+38) << 8) + *(buf+39);
    m_hb2_rt_info.Bus_Current = (*(buf+40) << 8) + *(buf+41);
    m_hb2_rt_info.PV3_Voltage = (*(buf+42) << 8) + *(buf+43);
    m_hb2_rt_info.PV3_Current = (*(buf+44) << 8) + *(buf+45);
    m_hb2_rt_info.PV3_Power = (*(buf+46) << 8) + *(buf+47);
    m_hb2_rt_info.PV4_Voltage = (*(buf+48) << 8) + *(buf+49);
    m_hb2_rt_info.PV4_Current = (*(buf+50) << 8) + *(buf+51);
    m_hb2_rt_info.PV4_Power = (*(buf+52) << 8) + *(buf+53);
    m_hb2_rt_info.PV_Total_Power = (*(buf+54) << 8) + *(buf+55);
    m_hb2_rt_info.Battery_SOC = (*(buf+56) << 8) + *(buf+57);
    // 0x011D empty
    m_hb2_rt_info.Sys_State = (*(buf+60) << 8) + *(buf+61);
    m_hb2_rt_info.Module_Status = (*(buf+62) << 8) + *(buf+63);
// 0x0120
    m_hb2_rt_info.Function_Status = (*(buf+64) << 8) + *(buf+65);
    m_hb2_rt_info.PV_Inv_Error_COD1_Record = (*(buf+66) << 8) + *(buf+67);
    m_hb2_rt_info.PV_Inv_Error_COD2_Record = (*(buf+68) << 8) + *(buf+69);
    m_hb2_rt_info.PV_Inv_Error_COD3_Record = (*(buf+70) << 8) + *(buf+71);
    m_hb2_rt_info.DD_Error_COD1_Record = (*(buf+72) << 8) + *(buf+73);
    m_hb2_rt_info.DD_Error_COD2_Record = (*(buf+74) << 8) + *(buf+75);
    m_hb2_rt_info.PV_Inv_Error_COD2 = (*(buf+76) << 8) + *(buf+77);
    m_hb2_rt_info.DD_Error_COD = (*(buf+78) << 8) + *(buf+79);
    m_hb2_rt_info.Error_Code = (*(buf+80) << 8) + *(buf+81);
    m_hb2_rt_info.Invert_Frequency = (*(buf+82) << 8) + *(buf+83);
    m_hb2_rt_info.Grid_Frequency = (*(buf+84) << 8) + *(buf+85);
    m_hb2_rt_info.Total_Load_Power = (*(buf+86) << 8) + *(buf+87);
    m_hb2_rt_info.Total_Load_Current = (*(buf+88) << 8) + *(buf+89);
    m_hb2_rt_info.CT_Power = (*(buf+90) << 8) + *(buf+91);
    m_hb2_rt_info.CT_Current = (*(buf+92) << 8) + *(buf+93);

    printf("#### Dump Hybrid2 RT Info ####\n");
// 0x0100
    printf("Inv_Temp = %03.1f C\n", ((float)m_hb2_rt_info.Inv_Temp)/10);
    printf("Charger_Temp = %03.1f C\n", ((float)m_hb2_rt_info.Charger_Temp)/10);
    printf("Environment_Temp = %03.1f C\n", ((float)m_hb2_rt_info.Environment_Temp)/10);
    printf("DD_Temp = %03.1f C\n", ((float)m_hb2_rt_info.DD_Temp)/10);
    printf("PV1_Voltage = %d V\n", m_hb2_rt_info.PV1_Voltage);
    printf("PV1_Current = %04.2f A\n", ((float)m_hb2_rt_info.PV1_Current)/100);
    printf("PV1_Power = %d W\n", m_hb2_rt_info.PV1_Power);
    printf("PV2_Voltage = %d V\n", m_hb2_rt_info.PV2_Voltage);
    printf("PV2_Current = %04.2f A\n", ((float)m_hb2_rt_info.PV2_Current)/100);
    printf("PV2_Power = %d W\n", m_hb2_rt_info.PV2_Power);
    printf("Load Voltage = %d V\n", m_hb2_rt_info.Load_Voltage);
    printf("Load Current = %04.2f A\n", ((float)m_hb2_rt_info.Load_Current)/100);
    printf("Load Power = %d W\n", m_hb2_rt_info.Load_Power);
    printf("Grid Voltage = %d V\n", m_hb2_rt_info.Grid_Voltage);
    printf("Grid Current = %04.2f A\n", ((float)m_hb2_rt_info.Grid_Current)/100);
    printf("Grid Power = %d W\n", m_hb2_rt_info.Grid_Power);
// 0x0110
    printf("Battery Voltage = %03.1f V\n", ((float)m_hb2_rt_info.Battery_Voltage)/10);
    printf("Battery Current = %03.1f A\n", ((float)m_hb2_rt_info.Battery_Current)/10);
    printf("Battery_Power = %d W\n", m_hb2_rt_info.Battery_Power);
    printf("Bus Voltage = %03.1f V\n", ((float)m_hb2_rt_info.Bus_Voltage)/10);
    printf("Bus Current = %03.1f A\n", ((float)m_hb2_rt_info.Bus_Current)/10);
    printf("PV3_Voltage = %d V\n", m_hb2_rt_info.PV3_Voltage);
    printf("PV3_Current = %04.2f A\n", ((float)m_hb2_rt_info.PV3_Current)/100);
    printf("PV3_Power = %d W\n", m_hb2_rt_info.PV3_Power);
    printf("PV4_Voltage = %d V\n", m_hb2_rt_info.PV4_Voltage);
    printf("PV4_Current = %04.2f A\n", ((float)m_hb2_rt_info.PV4_Current)/100);
    printf("PV4_Power = %d W\n", m_hb2_rt_info.PV4_Power);
    printf("PV Total Power = %d W\n", m_hb2_rt_info.PV_Total_Power);
    printf("Battery_SOC = %d %%\n", m_hb2_rt_info.Battery_SOC);
    printf("Sys_State = 0x%04X\n", m_hb2_rt_info.Sys_State);
    printf("Module_Status = 0x%04X\n", m_hb2_rt_info.Module_Status);
// 0x0120
    printf("Function_Status = 0x%04X\n", m_hb2_rt_info.Function_Status);
    printf("PV_Inv_Error_COD1_Record = 0x%04X\n", m_hb2_rt_info.PV_Inv_Error_COD1_Record);
    printf("PV_Inv_Error_COD2_Record = 0x%04X\n", m_hb2_rt_info.PV_Inv_Error_COD2_Record);
    printf("PV_Inv_Error_COD3_Record = 0x%04X\n", m_hb2_rt_info.PV_Inv_Error_COD3_Record);
    printf("DD_Error_COD1_Record = 0x%04X\n", m_hb2_rt_info.DD_Error_COD1_Record);
    printf("DD_Error_COD2_Record = 0x%04X\n", m_hb2_rt_info.DD_Error_COD2_Record);
    //printf("PV_Inv_Error_COD1 = 0x%04X\n", m_hb2_rt_info.PV_Inv_Error_COD1);
    printf("PV_Inv_Error_COD2 = 0x%04X\n", m_hb2_rt_info.PV_Inv_Error_COD2);
    printf("DD_Error_COD = 0x%04X\n", m_hb2_rt_info.DD_Error_COD);
    printf("Error_Code = 0x%04X\n", m_hb2_rt_info.Error_Code);
    printf("Invert Frequency = %03.1f Hz\n", ((float)m_hb2_rt_info.Invert_Frequency)/10);
    printf("Grid Frequency = %03.1f Hz\n", ((float)m_hb2_rt_info.Grid_Frequency)/10);
    printf("Total_Load_Power = %d W\n", m_hb2_rt_info.Total_Load_Power);
    printf("Total_Load_Current = %04.2f A\n", ((float)m_hb2_rt_info.Total_Load_Current)/100);
    printf("CT_Power = %d W\n", m_hb2_rt_info.CT_Power);
    printf("CT_Current = %04.2f A\n", ((float)m_hb2_rt_info.CT_Current)/100);
    printf("#############################\n");
}

void CG320::ParserHybrid2RTModuleFlags(int flags)
{
    int tmp = flags;

    m_hb2_rt_m_flags.B0_Relay_status = tmp & 0x01;
    tmp>>=1;
    m_hb2_rt_m_flags.B1_Software_offgrid_flag = tmp & 0x01;
    tmp>>=1;
    m_hb2_rt_m_flags.B2_InvEnable_flag = tmp & 0x01;
    tmp>>=1;
    m_hb2_rt_m_flags.B3_Bypass_flag = tmp & 0x01;
    tmp>>=1;
    m_hb2_rt_m_flags.B4_DD_en = tmp & 0x01;
    tmp>>=1;
    m_hb2_rt_m_flags.B5_PVEnable_flag = tmp & 0x0001;
    tmp>>=1;
    m_hb2_rt_m_flags.B6_Generator_state = tmp & 0x0001;

    printf("#### Parser Hybrid2 RT Module Flags ####\n");
    printf("Bit0 : Relay_status             = %d\n", m_hb2_rt_m_flags.B0_Relay_status);
    printf("Bit1 : Software_offgrid_flag    = %d\n", m_hb2_rt_m_flags.B1_Software_offgrid_flag);
    printf("Bit2 : InvEnable_flag           = %d\n", m_hb2_rt_m_flags.B2_InvEnable_flag);
    printf("Bit3 : Bypass_flag              = %d\n", m_hb2_rt_m_flags.B3_Bypass_flag);
    printf("Bit4 : DD_en                    = %d\n", m_hb2_rt_m_flags.B4_DD_en);
    printf("Bit5 : PVEnable_flag            = %d\n", m_hb2_rt_m_flags.B5_PVEnable_flag);
    printf("Bit6 : Generator_state          = %d\n", m_hb2_rt_m_flags.B6_Generator_state);
    printf("################################\n");
}

void CG320::ParserHybrid2RTFunctionFlags(int flags)
{
    int tmp = flags;

    m_hb2_rt_f_flags.B0_ADC = tmp & 0x01;
    tmp>>=1;
    m_hb2_rt_f_flags.B1_Buzz_flag = tmp & 0x01;
    tmp>>=1;
    m_hb2_rt_f_flags.B2_Bat_Protect = tmp & 0x01;
    tmp>>=1;
    m_hb2_rt_f_flags.B3_Bat_reserve_flag = tmp & 0x01;
    tmp>>=1;
    m_hb2_rt_f_flags.B4_Froce_Charge_flag = tmp & 0x01;
    tmp>>=1;
    m_hb2_rt_f_flags.B5_Grid_Char_En = tmp & 0x0001;
    tmp>>=1;
    m_hb2_rt_f_flags.B6_Grid_flag = tmp & 0x0001;
    tmp>>=1;
    m_hb2_rt_f_flags.B7_DD_en_Protect = tmp & 0x0001;
    tmp>>=1;
    m_hb2_rt_f_flags.B8_buck_Dis = tmp & 0x0001;
    tmp>>=1;
    m_hb2_rt_f_flags.B9_boots_dis = tmp & 0x0001;
    tmp>>=1;
    m_hb2_rt_f_flags.B10_Info_ready = tmp & 0x0001;
    tmp>>=1;
    m_hb2_rt_f_flags.B11_Inverter_Delay = tmp & 0x0001;

    printf("#### Parser Hybrid2 RT Function Flags ####\n");
    printf("Bit0 : ADC                 = %d\n", m_hb2_rt_f_flags.B0_ADC);
    printf("Bit1 : Buzz_flag           = %d\n", m_hb2_rt_f_flags.B1_Buzz_flag);
    printf("Bit2 : Bat_Protect         = %d\n", m_hb2_rt_f_flags.B2_Bat_Protect);
    printf("Bit3 : Bat_reserve_flag    = %d\n", m_hb2_rt_f_flags.B3_Bat_reserve_flag);
    printf("Bit4 : Froce_Charge_flag   = %d\n", m_hb2_rt_f_flags.B4_Froce_Charge_flag);
    printf("Bit5 : Grid_Char_En        = %d\n", m_hb2_rt_f_flags.B5_Grid_Char_En);
    printf("Bit6 : Grid_flag           = %d\n", m_hb2_rt_f_flags.B6_Grid_flag);
    printf("Bit7 : DD_en_Protect       = %d\n", m_hb2_rt_f_flags.B7_DD_en_Protect);
    printf("Bit8 : buck_Dis            = %d\n", m_hb2_rt_f_flags.B8_buck_Dis);
    printf("Bit9 : boots_dis           = %d\n", m_hb2_rt_f_flags.B9_boots_dis);
    printf("Bit10: Info_ready          = %d\n", m_hb2_rt_f_flags.B10_Info_ready);
    printf("Bit11: Inverter_Delay      = %d\n", m_hb2_rt_f_flags.B11_Inverter_Delay);
    printf("################################\n");
}

void CG320::ParserHybrid2PVInvErrCOD1(int COD1)
{
    int tmp = COD1;
    m_hb2_pvinv_err_cod1.B0_Fac_HL = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B1_CanBus_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B2_Islanding = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B3_Vac_H = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B4_Vac_L = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B5_Fac_H = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B6_Fac_L = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B7_Fac_LL = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B8_Vac_OCP = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B9_Vac_HL = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B10_Vac_LL = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B11_OPP = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B12_Iac_H = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B13_Ipv_H = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B14_ADCINT_OVF = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod1.B15_Vbus_H = tmp & 0x0001;

    printf("#### Parser Hybrid2 PV Inverter Error Code 1 ####\n");
    printf("Bit0  : Fac_HL = %d\n", m_hb2_pvinv_err_cod1.B0_Fac_HL);
    printf("Bit1  : CanBus_Fault = %d\n", m_hb2_pvinv_err_cod1.B1_CanBus_Fault);
    printf("Bit2  : Islanding = %d\n", m_hb2_pvinv_err_cod1.B2_Islanding);
    printf("Bit3  : Vac_H = %d\n", m_hb2_pvinv_err_cod1.B3_Vac_H);
    printf("Bit4  : Vac_L = %d\n", m_hb2_pvinv_err_cod1.B4_Vac_L);
    printf("Bit5  : Fac_H = %d\n", m_hb2_pvinv_err_cod1.B5_Fac_H);
    printf("Bit6  : Fac_L = %d\n", m_hb2_pvinv_err_cod1.B6_Fac_L);
    printf("Bit7  : Fac_LL = %d\n", m_hb2_pvinv_err_cod1.B7_Fac_LL);
    printf("Bit8  : Vac_OCP = %d\n", m_hb2_pvinv_err_cod1.B8_Vac_OCP);
    printf("Bit9  : Vac_HL = %d\n", m_hb2_pvinv_err_cod1.B9_Vac_HL);
    printf("Bit10 : Vac_LL = %d\n", m_hb2_pvinv_err_cod1.B10_Vac_LL);
    printf("Bit11 : OPP = %d\n", m_hb2_pvinv_err_cod1.B11_OPP);
    printf("Bit12 : Iac_H = %d\n", m_hb2_pvinv_err_cod1.B12_Iac_H);
    printf("Bit13 : Ipv_H = %d\n", m_hb2_pvinv_err_cod1.B13_Ipv_H);
    printf("Bit14 : ADCINT_OVF = %d\n", m_hb2_pvinv_err_cod1.B14_ADCINT_OVF);
    printf("Bit15 : Vbus_H = %d\n", m_hb2_pvinv_err_cod1.B15_Vbus_H);
    printf("################################################\n");
}

void CG320::ParserHybrid2PVInvErrCOD2(int COD2)
{
    int tmp = COD2;
    m_hb2_pvinv_err_cod2.B0_Arc = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B1_Para_Check = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B2_Ipv1_Short = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B3_Ipv2_Short = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B4_Vac_Short = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B5_CT_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B6_PV_Over_Power = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B7_NO_GRID = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B8_PV_Input_High = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B9_INV_Overload = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B10_RCMU_30 = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B11_RCMU_60 = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B12_RCMU_150 = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B13_RCMU_300 = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B14_RCMU_Test_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod2.B15_Vac_LM = tmp & 0x0001;

    printf("#### Parser Hybrid2 PV Inverter Error Code 2 ####\n");
    printf("Bit0  : Arc = %d\n", m_hb2_pvinv_err_cod2.B0_Arc);
    printf("Bit1  : Para_Check = %d\n", m_hb2_pvinv_err_cod2.B1_Para_Check);
    printf("Bit2  : Ipv1_Short = %d\n", m_hb2_pvinv_err_cod2.B2_Ipv1_Short);
    printf("Bit3  : Ipv2_Short = %d\n", m_hb2_pvinv_err_cod2.B3_Ipv2_Short);
    printf("Bit4  : Vac_Short = %d\n", m_hb2_pvinv_err_cod2.B4_Vac_Short);
    printf("Bit5  : CT_Fault = %d\n", m_hb2_pvinv_err_cod2.B5_CT_Fault);
    printf("Bit6  : PV_Over_Power = %d\n", m_hb2_pvinv_err_cod2.B6_PV_Over_Power);
    printf("Bit7  : NO_GRID = %d\n", m_hb2_pvinv_err_cod2.B7_NO_GRID);
    printf("Bit8  : PV_Input_High = %d\n", m_hb2_pvinv_err_cod2.B8_PV_Input_High);
    printf("Bit9  : INV_Overload = %d\n", m_hb2_pvinv_err_cod2.B9_INV_Overload);
    printf("Bit10 : RCMU_30 = %d\n", m_hb2_pvinv_err_cod2.B10_RCMU_30);
    printf("Bit11 : RCMU_60 = %d\n", m_hb2_pvinv_err_cod2.B11_RCMU_60);
    printf("Bit12 : RCMU_150 = %d\n", m_hb2_pvinv_err_cod2.B12_RCMU_150);
    printf("Bit13 : RCMU_300 = %d\n", m_hb2_pvinv_err_cod2.B13_RCMU_300);
    printf("Bit14 : RCMU_Test_Fault = %d\n", m_hb2_pvinv_err_cod2.B14_RCMU_Test_Fault);
    printf("Bit15 : Vac_LM = %d\n", m_hb2_pvinv_err_cod2.B15_Vac_LM);
    printf("################################################\n");
}

void CG320::ParserHybrid2PVInvErrCOD3(int COD3)
{
    int tmp = COD3;
    m_hb2_pvinv_err_cod3.B0_External_PV_OPP = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod3.B1_Model123_Reconnected_Delay = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod3.B2_Peak_Shaving_Over_Power = tmp & 0x0001;
    tmp>>=1;
    m_hb2_pvinv_err_cod3.B3_CLA_Execute_Time_Over = tmp & 0x0001;

    printf("#### Parser Hybrid2 PV Inverter Error Code 3 ####\n");
    printf("Bit0  : External_PV_OPP = %d\n", m_hb2_pvinv_err_cod3.B0_External_PV_OPP);
    printf("Bit1  : Model123_Reconnected_Delay = %d\n", m_hb2_pvinv_err_cod3.B1_Model123_Reconnected_Delay);
    printf("Bit2  : Peak_Shaving_Over_Power = %d\n", m_hb2_pvinv_err_cod3.B2_Peak_Shaving_Over_Power);
    printf("Bit3  : CLA_Execute_Time_Over = %d\n", m_hb2_pvinv_err_cod3.B3_CLA_Execute_Time_Over);
    printf("################################################\n");
}

void CG320::ParserHybrid2DDErrCOD1(int COD1)
{
    int tmp = COD1;
    m_hb2_dd_err_cod1.B0_Vbat_H = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod1.B1_Vbat_L = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod1.B2_Vbus_H = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod1.B3_Vbus_L = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod1.B4_Ibus_H = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod1.B5_Ibat_H = tmp & 0x0001;
    tmp>>=2;
    //m_hb2_dd_err_cod1.B6_Charger_T = tmp & 0x0001;
    //tmp>>=1;
    m_hb2_dd_err_cod1.B7_Code = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod1.B8_Vbat_Drop = tmp & 0x0001;
    tmp>>=3;
    //m_hb2_dd_err_cod1.B9_INV_Fault = tmp & 0x0001;
    //tmp>>=1;
    //m_hb2_dd_err_cod1.B10_GND_Fault = tmp & 0x0001;
    //tmp>>=1;
    m_hb2_dd_err_cod1.B11_No_bat = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod1.B12_BMS_Comute_Fault = tmp & 0x0001;
    tmp>>=2;
    //m_hb2_dd_err_cod1.B13_BMS_Over_Current = tmp & 0x0001;
    //tmp>>=1;
    m_hb2_dd_err_cod1.B14_Vbus_High_VBat = tmp & 0x0001;
    //tmp>>=1;
    //m_hb2_dd_err_cod1.B15_Bat_Setting_Fault = tmp & 0x0001;

    printf("#### Parser Hybrid2 DD Error Code ####\n");
    printf("Bit0  : Vbat_H = %d\n", m_hb2_dd_err_cod1.B0_Vbat_H);
    printf("Bit1  : Vbat_L = %d\n", m_hb2_dd_err_cod1.B1_Vbat_L);
    printf("Bit2  : Vbus_H = %d\n", m_hb2_dd_err_cod1.B2_Vbus_H);
    printf("Bit3  : Vbus_L = %d\n", m_hb2_dd_err_cod1.B3_Vbus_L);
    printf("Bit4  : Ibus_H = %d\n", m_hb2_dd_err_cod1.B4_Ibus_H);
    printf("Bit5  : Ibat_H = %d\n", m_hb2_dd_err_cod1.B5_Ibat_H);
    //printf("Bit6  : Charger_T = %d\n", m_hb2_dd_err_cod1.B6_Charger_T);
    printf("Bit7  : Code = %d\n", m_hb2_dd_err_cod1.B7_Code);
    printf("Bit8  : Vbat_Drop = %d\n", m_hb2_dd_err_cod1.B8_Vbat_Drop);
    //printf("Bit9  : INV_Fault = %d\n", m_hb2_dd_err_cod1.B9_INV_Fault);
    //printf("Bit10 : GND_Fault = %d\n", m_hb2_dd_err_cod1.B10_GND_Fault);
    printf("Bit11 : No_bat = %d\n", m_hb2_dd_err_cod1.B11_No_bat);
    printf("Bit12 : BMS_Comute_Fault = %d\n", m_hb2_dd_err_cod1.B12_BMS_Comute_Fault);
    //printf("Bit13 : BMS_Over_Current = %d\n", m_hb2_dd_err_cod1.B13_BMS_Over_Current);
    printf("Bit14 : Vbus_High_VBat = %d\n", m_hb2_dd_err_cod1.B14_Vbus_High_VBat);
    //printf("Bit15 : Bat_Setting_Fault = %d\n", m_hb2_dd_err_cod1.B15_Bat_Setting_Fault);
    printf("#####################################\n");
}

void CG320::ParserHybrid2DDErrCOD2(int COD2)
{
    int tmp = COD2;
    m_hb2_dd_err_cod2.B0_EEProm_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B1_Communi_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B2_OT_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B3_Fan_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B4_Low_Battery = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B5_PV3_S = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B6_PV4_S = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B7_PV_Over_Power = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B8_PV_Input_High = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B9_Restart = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B10_GND_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B11_OT_Alarm = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B12_Bat_Wake_Up_Fault = tmp & 0x0001;
    tmp>>=1;
    m_hb2_dd_err_cod2.B13_Vbat_Inconsistent = tmp & 0x0001;

    printf("#### Parser Hybrid2 DD Error Code 2 ####\n");
    printf("Bit0  : EEProm_Fault = %d\n", m_hb2_dd_err_cod2.B0_EEProm_Fault);
    printf("Bit1  : Communi_Fault = %d\n", m_hb2_dd_err_cod2.B1_Communi_Fault);
    printf("Bit2  : OT_Fault = %d\n", m_hb2_dd_err_cod2.B2_OT_Fault);
    printf("Bit3  : Fan_Fault = %d\n", m_hb2_dd_err_cod2.B3_Fan_Fault);
    printf("Bit4  : Low_Battery = %d\n", m_hb2_dd_err_cod2.B4_Low_Battery);
    printf("Bit5  : PV3_S = %d\n", m_hb2_dd_err_cod2.B5_PV3_S);
    printf("Bit6  : PV4_S = %d\n", m_hb2_dd_err_cod2.B6_PV4_S);
    printf("Bit7  : PV_Over_Power = %d\n", m_hb2_dd_err_cod2.B7_PV_Over_Power);
    printf("Bit8  : PV_Input_High = %d\n", m_hb2_dd_err_cod2.B8_PV_Input_High);
    printf("Bit9  : Restart = %d\n", m_hb2_dd_err_cod2.B9_Restart);
    printf("Bit10 : GND_Fault = %d\n", m_hb2_dd_err_cod2.B10_GND_Fault);
    printf("Bit10 : OT_Alarm = %d\n", m_hb2_dd_err_cod2.B11_OT_Alarm);
    printf("Bit10 : Bat_Wake_Up_Fault = %d\n", m_hb2_dd_err_cod2.B12_Bat_Wake_Up_Fault);
    printf("Bit10 : Vbat_Inconsistent = %d\n", m_hb2_dd_err_cod2.B13_Vbat_Inconsistent);
    printf("#####################################\n");
}

void CG320::DumpHybrid2CEValue(unsigned char *buf)
{
// 0x0160
    m_hb2_ce_value.Total_Life_TimeH = (*(buf) << 8) + *(buf+1);
    m_hb2_ce_value.Total_Life_TimeL = (*(buf+2) << 8) + *(buf+3);
    m_hb2_ce_value.PV_Total_EnergyH = (*(buf+4) << 8) + *(buf+5);
    m_hb2_ce_value.PV_Total_EnergyL = (*(buf+6) << 8) + *(buf+7);
    m_hb2_ce_value.Bat_Charge_Total_EnergyH = (*(buf+8) << 8) + *(buf+9);
    m_hb2_ce_value.Bat_Charge_Total_EnergyL = (*(buf+10) << 8) + *(buf+11);
    m_hb2_ce_value.Bat_Discharge_Total_EnergyH = (*(buf+12) << 8) + *(buf+13);
    m_hb2_ce_value.Bat_Discharge_Total_EnergyL = (*(buf+14) << 8) + *(buf+15);
    m_hb2_ce_value.Load_Total_EnergyH = (*(buf+16) << 8) + *(buf+17);
    m_hb2_ce_value.Load_Total_EnergyL = (*(buf+18) << 8) + *(buf+19);
    m_hb2_ce_value.Negative_Load_Total_EnergyH = (*(buf+20) << 8) + *(buf+21);
    m_hb2_ce_value.Negative_Load_Total_EnergyL = (*(buf+22) << 8) + *(buf+23);
    m_hb2_ce_value.GridFeed_TotalH = (*(buf+24) << 8) + *(buf+25);
    m_hb2_ce_value.GridFeed_TotalL = (*(buf+26) << 8) + *(buf+27);
    m_hb2_ce_value.GridCharge_TotalH = (*(buf+28) << 8) + *(buf+29);
    m_hb2_ce_value.GridCharge_TotalL = (*(buf+30) << 8) + *(buf+31);
// 0x0170
    m_hb2_ce_value.PV_Today_EnergyH = (*(buf+32) << 8) + *(buf+33);
    m_hb2_ce_value.PV_Today_EnergyL = (*(buf+34) << 8) + *(buf+35);
    m_hb2_ce_value.Bat_Charge_Today_EnergyH = (*(buf+36) << 8) + *(buf+37);
    m_hb2_ce_value.Bat_Charge_Today_EnergyL = (*(buf+38) << 8) + *(buf+39);
    m_hb2_ce_value.Bat_Discharge_Today_EnergyH = (*(buf+40) << 8) + *(buf+41);
    m_hb2_ce_value.Bat_Discharge_Today_EnergyL = (*(buf+42) << 8) + *(buf+43);
    m_hb2_ce_value.Load_Today_EnergyH = (*(buf+44) << 8) + *(buf+45);
    m_hb2_ce_value.Load_Today_EnergyL = (*(buf+46) << 8) + *(buf+47);
    m_hb2_ce_value.Negative_Load_Today_EnergyH = (*(buf+48) << 8) + *(buf+49);
    m_hb2_ce_value.Negative_Load_Today_EnergyL = (*(buf+50) << 8) + *(buf+51);
    m_hb2_ce_value.GridFeed_TodayH = (*(buf+52) << 8) + *(buf+53);
    m_hb2_ce_value.GridFeed_TodayL = (*(buf+54) << 8) + *(buf+55);
    m_hb2_ce_value.GridCharge_TodayH = (*(buf+56) << 8) + *(buf+57);
    m_hb2_ce_value.GridCharge_TodayL = (*(buf+58) << 8) + *(buf+59);
    m_hb2_ce_value.PV_Month_EnergyH = (*(buf+60) << 8) + *(buf+61);
    m_hb2_ce_value.PV_Month_EnergyL = (*(buf+62) << 8) + *(buf+63);
// 0x0180
    m_hb2_ce_value.Bat_Charge_Month_EnergyH = (*(buf+64) << 8) + *(buf+65);
    m_hb2_ce_value.Bat_Charge_Month_EnergyL = (*(buf+66) << 8) + *(buf+67);
    m_hb2_ce_value.Bat_Discharge_Month_EnergyH = (*(buf+68) << 8) + *(buf+69);
    m_hb2_ce_value.Bat_Discharge_Month_EnergyL = (*(buf+70) << 8) + *(buf+71);
    m_hb2_ce_value.Load_Month_EnergyH = (*(buf+72) << 8) + *(buf+73);
    m_hb2_ce_value.Load_Month_EnergyL = (*(buf+74) << 8) + *(buf+75);
    m_hb2_ce_value.Negative_Load_Month_EnergyH = (*(buf+76) << 8) + *(buf+77);
    m_hb2_ce_value.Negative_Load_Month_EnergyL = (*(buf+78) << 8) + *(buf+79);
    m_hb2_ce_value.GridFeed_MonthH = (*(buf+80) << 8) + *(buf+81);
    m_hb2_ce_value.GridFeed_MonthL = (*(buf+82) << 8) + *(buf+83);
    m_hb2_ce_value.GridCharge_MonthH = (*(buf+84) << 8) + *(buf+85);
    m_hb2_ce_value.GridCharge_MonthL = (*(buf+86) << 8) + *(buf+87);
    m_hb2_ce_value.CT_Total_Feedin_EnergyH = (*(buf+88) << 8) + *(buf+89);
    m_hb2_ce_value.CT_Total_Feedin_EnergyL = (*(buf+90) << 8) + *(buf+91);
    m_hb2_ce_value.CT_Today_Feedin_EnergyH = (*(buf+92) << 8) + *(buf+93);
    m_hb2_ce_value.CT_Today_Feedin_EnergyL = (*(buf+94) << 8) + *(buf+95);
// 0x0190
    m_hb2_ce_value.CT_Total_Charge_EnergyH = (*(buf+96) << 8) + *(buf+97);
    m_hb2_ce_value.CT_Total_Charge_EnergyL = (*(buf+98) << 8) + *(buf+99);
    m_hb2_ce_value.CT_Today_Charge_EnergyH = (*(buf+100) << 8) + *(buf+101);
    m_hb2_ce_value.CT_Today_Charge_EnergyL = (*(buf+102) << 8) + *(buf+103);

    printf("#### Dump Hybrid2 CE Value ####\n");
// 0x0160
    printf("Total_Life_TimeH = %d *100KWH\n", m_hb2_ce_value.Total_Life_TimeH);
    printf("Total_Life_TimeL = %d *0.01KWH\n", m_hb2_ce_value.Total_Life_TimeL);
    printf("PV_Total_EnergyH = %d *100KWH\n", m_hb2_ce_value.PV_Total_EnergyH);
    printf("PV_Total_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.PV_Total_EnergyL);
    printf("Bat_Charge_Total_EnergyH = %d *100KWH\n", m_hb2_ce_value.Bat_Charge_Total_EnergyH);
    printf("Bat_Charge_Total_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.Bat_Charge_Total_EnergyL);
    printf("Bat_Discharge_Total_EnergyH = %d *100KWH\n", m_hb2_ce_value.Bat_Discharge_Total_EnergyH);
    printf("Bat_Discharge_Total_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.Bat_Discharge_Total_EnergyL);
    printf("Load_Total_EnergyH = %d *100KWH\n", m_hb2_ce_value.Load_Total_EnergyH);
    printf("Load_Total_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.Load_Total_EnergyL);
    printf("Negative_Load_Total_EnergyH = %d *100KWH\n", m_hb2_ce_value.Negative_Load_Total_EnergyH);
    printf("Negative_Load_Total_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.Negative_Load_Total_EnergyL);
    printf("GridFeed_TotalH = %d *100KWH\n", m_hb2_ce_value.GridFeed_TotalH);
    printf("GridFeed_TotalL = %d *0.01KWH\n", m_hb2_ce_value.GridFeed_TotalL);
    printf("GridCharge_TotalH = %d *100KWH\n", m_hb2_ce_value.GridCharge_TotalH);
    printf("GridCharge_TotalL = %d *0.01KWH\n", m_hb2_ce_value.GridCharge_TotalL);
// 0x0170
    printf("PV_Today_EnergyH = %d *100KWH\n", m_hb2_ce_value.PV_Today_EnergyH);
    printf("PV_Today_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.PV_Today_EnergyL);
    printf("Bat_Charge_Today_EnergyH = %d *100KWH\n", m_hb2_ce_value.Bat_Charge_Today_EnergyH);
    printf("Bat_Charge_Today_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.Bat_Charge_Today_EnergyL);
    printf("Bat_Discharge_Today_EnergyH = %d *100KWH\n", m_hb2_ce_value.Bat_Discharge_Today_EnergyH);
    printf("Bat_Discharge_Today_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.Bat_Discharge_Today_EnergyL);
    printf("Load_Today_EnergyH = %d *100KWH\n", m_hb2_ce_value.Load_Today_EnergyH);
    printf("Load_Today_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.Load_Today_EnergyL);
    printf("Negative_Load_Today_EnergyH = %d *100KWH\n", m_hb2_ce_value.Negative_Load_Today_EnergyH);
    printf("Negative_Load_Today_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.Negative_Load_Today_EnergyL);
    printf("GridFeed_TodayH = %d *100KWH\n", m_hb2_ce_value.GridFeed_TodayH);
    printf("GridFeed_TodayL = %d *0.01KWH\n", m_hb2_ce_value.GridFeed_TodayL);
    printf("GridCharge_TodayH = %d *100KWH\n", m_hb2_ce_value.GridCharge_TodayH);
    printf("GridCharge_TodayL = %d *0.01KWH\n", m_hb2_ce_value.GridCharge_TodayL);
    printf("PV_Month_EnergyH = %d *100KWH\n", m_hb2_ce_value.PV_Month_EnergyH);
    printf("PV_Month_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.PV_Month_EnergyL);
// 0x0180
    printf("Bat_Charge_Month_EnergyH = %d *100KWH\n", m_hb2_ce_value.Bat_Charge_Month_EnergyH);
    printf("Bat_Charge_Month_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.Bat_Charge_Month_EnergyL);
    printf("Bat_Discharge_Month_EnergyH = %d *100KWH\n", m_hb2_ce_value.Bat_Discharge_Month_EnergyH);
    printf("Bat_Discharge_Month_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.Bat_Discharge_Month_EnergyL);
    printf("Load_Month_EnergyH = %d *100KWH\n", m_hb2_ce_value.Load_Month_EnergyH);
    printf("Load_Month_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.Load_Month_EnergyL);
    printf("Negative_Load_Month_EnergyH = %d *100KWH\n", m_hb2_ce_value.Negative_Load_Month_EnergyH);
    printf("Negative_Load_Month_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.Negative_Load_Month_EnergyL);
    printf("GridFeed_MonthH = %d *100KWH\n", m_hb2_ce_value.GridFeed_MonthH);
    printf("GridFeed_MonthL = %d *0.01KWH\n", m_hb2_ce_value.GridFeed_MonthL);
    printf("GridCharge_MonthH = %d *100KWH\n", m_hb2_ce_value.GridCharge_MonthH);
    printf("GridCharge_MonthL = %d *0.01KWH\n", m_hb2_ce_value.GridCharge_MonthL);
    printf("CT_Total_Feedin_EnergyH = %d *100KWH\n", m_hb2_ce_value.CT_Total_Feedin_EnergyH);
    printf("CT_Total_Feedin_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.CT_Total_Feedin_EnergyL);
    printf("CT_Today_Feedin_EnergyH = %d *100KWH\n", m_hb2_ce_value.CT_Today_Feedin_EnergyH);
    printf("CT_Today_Feedin_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.CT_Today_Feedin_EnergyL);
// 0x0190
    printf("CT_Total_Charge_EnergyH = %d *100KWH\n", m_hb2_ce_value.CT_Total_Charge_EnergyH);
    printf("CT_Total_Charge_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.CT_Total_Charge_EnergyL);
    printf("CT_Today_Charge_EnergyH = %d *100KWH\n", m_hb2_ce_value.CT_Today_Charge_EnergyH);
    printf("CT_Today_Charge_EnergyL = %d *0.01KWH\n", m_hb2_ce_value.CT_Today_Charge_EnergyL);
    printf("#############################\n");
}

void CG320::DumpHybrid2DPInfo(unsigned char *buf)
{
// 0x0160
    m_hb2_dp_info.Display_Working_State = (*(buf) << 8) + *(buf+1);
    m_hb2_dp_info.Output_Power_Restraint_Reeson = (*(buf+2) << 8) + *(buf+3);
    m_hb2_dp_info.Battery_To_Load_Consumption_Time = (*(buf+4) << 8) + *(buf+5);
    m_hb2_dp_info.OnGrid_CountDown = (*(buf+6) << 8) + *(buf+7);
    m_hb2_dp_info.Hybrid_IconL = (*(buf+8) << 8) + *(buf+9);
    m_hb2_dp_info.Hybrid_IconH = (*(buf+10) << 8) + *(buf+11);

    printf("#### Dump Hybrid2 DP Info ####\n");
    printf("Display_Working_State            = %d\n", m_hb2_dp_info.Display_Working_State);
    printf("Output_Power_Restraint_Reeson    = %d\n", m_hb2_dp_info.Output_Power_Restraint_Reeson);
    printf("Battery_To_Load_Consumption_Time = %d\n", m_hb2_dp_info.Battery_To_Load_Consumption_Time);
    printf("OnGrid_CountDown                 = %d\n", m_hb2_dp_info.OnGrid_CountDown);
    printf("Hybrid_IconL                     = 0x%04X\n", m_hb2_dp_info.Hybrid_IconL);
    printf("Hybrid_IconH                     = 0x%04X\n", m_hb2_dp_info.Hybrid_IconH);
    printf("#############################\n");
}

void CG320::ParserHybrid2IconInfo(int Icon_L, int Icon_H)
{
    int tmp = Icon_L;
    m_hb2_icon_info.B0_1_PV = tmp & 0x0003;
    tmp>>=2;
    m_hb2_icon_info.B2_3_Battery = tmp & 0x0003;
    tmp>>=2;
    m_hb2_icon_info.B4_5_Grid = tmp & 0x0003;
    tmp>>=2;
    m_hb2_icon_info.B6_7_Load = tmp & 0x0003;
    tmp>>=2;
    m_hb2_icon_info.B8_1O_Inverter_System = tmp & 0x0007;
    tmp>>=3;
    m_hb2_icon_info.B11_Generator = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B12_DL_Comm = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B13_Cloud_Comm = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B14_Bat_Calibration = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B15_Derate = tmp & 0x0001;

    tmp = Icon_H;
    m_hb2_icon_info.B16_PV_To_Battery = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B17_PV_To_Grid = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B18_PV_To_Load = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B19_Battery_To_Grid = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B20_Battery_To_Load = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B21_Grid_To_Battery = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B22_Grid_To_Load = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B23_Load_To_Battery = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B24_Load_To_Grid = tmp & 0x0001;
    tmp>>=1;
    m_hb2_icon_info.B25_26_Mode = tmp & 0x0003;
    tmp>>=2;
    m_hb2_icon_info.B27_29_System_Status = tmp & 0x0007;

    printf("#### Parser Hybrid2 Icon ####\n");
    printf("Bit0-1     : PV = %d\n", m_hb2_icon_info.B0_1_PV);
    printf("Bit2-3     : Battery = %d\n", m_hb2_icon_info.B2_3_Battery);
    printf("Bit4-5     : Grid = %d\n", m_hb2_icon_info.B4_5_Grid);
    printf("Bit6-7     : Load = %d\n", m_hb2_icon_info.B6_7_Load);
    printf("Bit8-1O    : Inverter_System = %d\n", m_hb2_icon_info.B8_1O_Inverter_System);
    printf("Bit11      : Generator = %d\n", m_hb2_icon_info.B11_Generator);
    printf("Bit12      : DL_Comm = %d\n", m_hb2_icon_info.B12_DL_Comm);
    printf("Bit13      : Cloud_Comm = %d\n", m_hb2_icon_info.B13_Cloud_Comm);
    printf("Bit14      : Bat_Calibration = %d\n", m_hb2_icon_info.B14_Bat_Calibration);
    printf("Bit15      : Derate = %d\n", m_hb2_icon_info.B15_Derate);
    printf("Bit16      : PV_To_Battery = %d\n", m_hb2_icon_info.B16_PV_To_Battery);
    printf("Bit17      : PV_To_Grid = %d\n", m_hb2_icon_info.B17_PV_To_Grid);
    printf("Bit18      : PV_To_Load = %d\n", m_hb2_icon_info.B18_PV_To_Load);
    printf("Bit19      : Battery_To_Grid = %d\n", m_hb2_icon_info.B19_Battery_To_Grid);
    printf("Bit20      : Battery_To_Load = %d\n", m_hb2_icon_info.B20_Battery_To_Load);
    printf("Bit21      : Grid_To_Battery = %d\n", m_hb2_icon_info.B21_Grid_To_Battery);
    printf("Bit22      : Grid_To_Load = %d\n", m_hb2_icon_info.B22_Grid_To_Load);
    printf("Bit23      : Load_To_Battery = %d\n", m_hb2_icon_info.B23_Load_To_Battery);
    printf("Bit24      : Load_To_Grid = %d\n", m_hb2_icon_info.B24_Load_To_Grid);
    printf("Bit25-26   : Mode = %d\n", m_hb2_icon_info.B25_26_Mode);
    printf("Bit27-29   : System_Status= %d\n", m_hb2_icon_info.B27_29_System_Status);
    printf("############################\n");
}

void CG320::DumpHybrid2RTCData(unsigned char *buf)
{
// 0x01D0
    m_hb2_rtc_data.Second = (*(buf) << 8) + *(buf+1);
    m_hb2_rtc_data.Minute = (*(buf+2) << 8) + *(buf+3);
    m_hb2_rtc_data.Hour = (*(buf+4) << 8) + *(buf+5);
    m_hb2_rtc_data.Date = (*(buf+6) << 8) + *(buf+7);
    m_hb2_rtc_data.Month = (*(buf+8) << 8) + *(buf+9);
    m_hb2_rtc_data.Year = (*(buf+10) << 8) + *(buf+11);

    printf("#### Dump Hybrid2 RTC Data ####\n");
    printf("Second = %d\n", m_hb2_rtc_data.Second);
    printf("Minute = %d\n", m_hb2_rtc_data.Minute);
    printf("Hour   = %d\n", m_hb2_rtc_data.Hour);
    printf("Date   = %d\n", m_hb2_rtc_data.Date); //
    printf("Month  = %d\n", m_hb2_rtc_data.Month);
    printf("Year   = %d\n", m_hb2_rtc_data.Year);
    printf("##############################\n");
    printf("rtc time : %4d/%02d/%02d ", m_hb2_rtc_data.Year, m_hb2_rtc_data.Month, m_hb2_rtc_data.Date);
    printf("%02d:%02d:%02d\n", m_hb2_rtc_data.Hour, m_hb2_rtc_data.Minute, m_hb2_rtc_data.Second);
    printf("##############################\n");
}

bool CG320::SetHybrid2RTCData(int index)
{
    time_t  current_time;
    struct tm   *st_time = NULL;

    printf("#### SetHybrid2RTCData Start ####\n");
    current_time = time(NULL);
    st_time = localtime(&current_time);

    // check ok time
    if ( current_time - arySNobj[index].m_ok_time >= OFFLINE_SECOND_HB ) {
        printf("Last m_ok_time more then 1200 sec.\n");
        ReRegister(index);
    }

    m_hb2_rtc_data.Second = st_time->tm_sec;
    m_hb2_rtc_data.Minute = st_time->tm_min;
    m_hb2_rtc_data.Hour = st_time->tm_hour;
    m_hb2_rtc_data.Date = st_time->tm_mday;
    m_hb2_rtc_data.Month = 1 + st_time->tm_mon; // ptm->tm_mon 0~11, m_hb2_rtc_data.Month 1~12
    m_hb2_rtc_data.Year = 1900 + st_time->tm_year;
    //printf("RTC timebuf : %4d/%02d/%02d ", m_hb2_rtc_data.Year, m_hb2_rtc_data.Month, m_hb2_rtc_data.Date);
    //printf("%02d:%02d:%02d\n", m_hb2_rtc_data.Hour, m_hb2_rtc_data.Minute, m_hb2_rtc_data.Second);
    //printf("#######################################\n");

    int err = 0;
    unsigned short crc;
    byte *lpdata = NULL;

    unsigned char szRTCData[41]={0};

    while ( err < 3 ) {
        // set buf
        szRTCData[0] = arySNobj[index].m_Addr;
        szRTCData[1] = 0x10; // function code
        szRTCData[2] = 0x01;
        szRTCData[3] = 0xD0; // star address
        szRTCData[4] = 0x00;
        szRTCData[5] = 0x08; // number of data
        szRTCData[6] = 0x10; // bytes
        // data 0x40 ~ 0x45
        szRTCData[7] = 0x00;
        szRTCData[8] = (unsigned char)m_hb2_rtc_data.Second;
        szRTCData[9] = 0x00;
        szRTCData[10] = (unsigned char)m_hb2_rtc_data.Minute;
        szRTCData[11] = 0x00;
        szRTCData[12] = (unsigned char)m_hb2_rtc_data.Hour;
        szRTCData[13] = 0x00;
        szRTCData[14] = (unsigned char)m_hb2_rtc_data.Date;
        szRTCData[15] = 0x00;
        szRTCData[16] = (unsigned char)m_hb2_rtc_data.Month;
        szRTCData[17] = (unsigned char)((m_hb2_rtc_data.Year >> 8) & 0xFF);
        szRTCData[18] = (unsigned char)(m_hb2_rtc_data.Year & 0xFF);
        // zero 0x46 ~ 0x4E
        szRTCData[19] = 0x00;
        szRTCData[20] = 0x00;
        //szRTCData[21] = 0x00;
        //szRTCData[22] = 0x00;
        //szRTCData[23] = 0x00;
        //szRTCData[24] = 0x00;
        //szRTCData[25] = 0x00;
        //szRTCData[26] = 0x00;
        //szRTCData[27] = 0x00;
        //szRTCData[28] = 0x00;
        //szRTCData[29] = 0x00;
        //szRTCData[30] = 0x00;
        //szRTCData[31] = 0x00;
        //szRTCData[32] = 0x00;
        //szRTCData[33] = 0x00;
        //szRTCData[34] = 0x00;
        //szRTCData[35] = 0x00;
        //szRTCData[36] = 0x00;
        // data crc 0x4F
        //crc = CalculateCRC(szRTCData+7, 30);
        crc = CalculateCRC(szRTCData+7, 14);
        //szRTCData[37] = (unsigned char) (crc >> 8); // data crc hi
        szRTCData[21] = (unsigned char) (crc >> 8); // data crc hi
        //szRTCData[38] = (unsigned char) (crc & 0xFF); // data crc lo
        szRTCData[22] = (unsigned char) (crc & 0xFF); // data crc lo
        //szRTCData[39] = 0x00; // cmd crc hi
        szRTCData[23] = 0x00; // cmd crc hi
        //szRTCData[40] = 0x00; // cmd crc lo
        szRTCData[24] = 0x00; // cmd crc lo
        //MakeReadDataCRC(szRTCData,41);
        MakeReadDataCRC(szRTCData,25);
        MClearRX();
        //txsize=41;
        txsize=25;
        waitAddr = arySNobj[index].m_Addr;
        waitFCode = 0x10;
        //memcpy(txbuffer, szRTCData, 41);
        memcpy(txbuffer, szRTCData, 25);
        MStartTX(m_busfd);
        //usleep(m_dl_config.m_delay_time_2);

        lpdata = GetRespond(m_busfd, 8, m_dl_config.m_delay_time_2);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 8) ) {
                printf("#### SetHybrid2RTCData OK ####\n");
                SaveLog((char *)"DataLogger SetHybrid2RTCData() : OK", st_time);
                arySNobj[index].m_ok_time = time(NULL);
                //free(lpdata);
                return true;
            } else {
                printf("#### SetHybrid2RTCData CRC Error ####\n");
                SaveLog((char *)"DataLogger SetHybrid2RTCData() : CRC Error", st_time);
                err++;
            }
            //free(lpdata);
        } else {
            printf("#### SetHybrid2RTCData No Response ####\n");
            SaveLog((char *)"DataLogger SetHybrid2RTCData() : No Response", st_time);
            //SaveLog((char *)"DataLogger SetHybrid2RTCData() : run reregister()", st_time);
            //ReRegister(index);
            err++;
        }

        usleep(1000000);
    }

    return false;
}

void CG320::DumpHybrid2BMSInfo(unsigned char *buf)
{
// 0x0200
    m_hb2_bms_info.Charge_Voltage_Requirement = (*(buf) << 8) + *(buf+1);
    m_hb2_bms_info.Charge_Current_Requirement = (*(buf+2) << 8) + *(buf+3);
    m_hb2_bms_info.Number_Of_Module_Warning = (*(buf+4) << 8) + *(buf+5);
    m_hb2_bms_info.Master_Total_Voltage = (*(buf+6) << 8) + *(buf+7);
    m_hb2_bms_info.Discharging_Charging_Current = (*(buf+8) << 8) + *(buf+9);
    m_hb2_bms_info.SOC = (*(buf+10) << 8) + *(buf+11);
    m_hb2_bms_info.FCC = (*(buf+12) << 8) + *(buf+13);
    m_hb2_bms_info.RC = (*(buf+14) << 8) + *(buf+15);
    m_hb2_bms_info.Status_Flag_Register = (*(buf+16) << 8) + *(buf+17);
    m_hb2_bms_info.IO_Flag_Register = (*(buf+18) << 8) + *(buf+19);
    m_hb2_bms_info.Warning_Flag_Register = (*(buf+20) << 8) + *(buf+21);
    m_hb2_bms_info.Alarm_Flag_Register = (*(buf+22) << 8) + *(buf+23);
    m_hb2_bms_info.Max_Cell_Voltage = (*(buf+24) << 8) + *(buf+25);
    m_hb2_bms_info.Min_Cell_Voltage = (*(buf+26) << 8) + *(buf+27);
    m_hb2_bms_info.Max_Cell_Temperature = (*(buf+28) << 8) + *(buf+29);
    m_hb2_bms_info.Min_Cell_Temperature = (*(buf+30) << 8) + *(buf+31);
// 0x0210
    m_hb2_bms_info.Master_Average_Cycle_Count = (*(buf+32) << 8) + *(buf+33);
    m_hb2_bms_info.Master_Average_SOH = (*(buf+34) << 8) + *(buf+35);
    m_hb2_bms_info.Battery_Cumulative_Input_Capacity = (*(buf+36) << 8) + *(buf+37);
    m_hb2_bms_info.Battery_Cumulative_Output_Capacity = (*(buf+38) << 8) + *(buf+39);
    m_hb2_bms_info.Master_FW_Version = (*(buf+40) << 8) + *(buf+41);
    m_hb2_bms_info.Master_Manufacture_Data = (*(buf+42) << 8) + *(buf+43);
    m_hb2_bms_info.Master_Serial_Number = (*(buf+44) << 8) + *(buf+45);
    m_hb2_bms_info.Number_Of_Whole_Cells = (*(buf+46) << 8) + *(buf+47);
    m_hb2_bms_info.Number_Of_Module = (*(buf+48) << 8) + *(buf+49);

// 0x0200
    printf("#### Dump Hybrid2 BMS Info ####\n");
    printf("Charge_Voltage_Requirement         = %d *10mV\n", m_hb2_bms_info.Charge_Voltage_Requirement);
    printf("Charge_Current_Requirement         = %d *10mA\n", m_hb2_bms_info.Charge_Current_Requirement);
    printf("Number_Of_Module_Warning           = %d\n", m_hb2_bms_info.Number_Of_Module_Warning);
    printf("Master_Total_Voltage               = %d *10mV\n", m_hb2_bms_info.Master_Total_Voltage);
    printf("Discharging_Charging_Current       = %d *10mA\n", m_hb2_bms_info.Discharging_Charging_Current);
    printf("SOC                                = %d %% *0.1\n", m_hb2_bms_info.SOC);
    printf("FCC                                = %d *10mAh\n", m_hb2_bms_info.FCC);
    printf("RC                                 = %d *10mAh\n", m_hb2_bms_info.RC);
    printf("Status_Flag_Register               = 0x%04X\n", m_hb2_bms_info.Status_Flag_Register);
    printf("IO_Flag_Register                   = 0x%04X\n", m_hb2_bms_info.IO_Flag_Register);
    printf("Warning_Flag_Register              = 0x%04X\n", m_hb2_bms_info.Warning_Flag_Register);
    printf("Alarm_Flag_Register                = 0x%04X\n", m_hb2_bms_info.Alarm_Flag_Register);
    printf("Max_Cell_Voltage                   = %d mV\n", m_hb2_bms_info.Max_Cell_Voltage);
    printf("Min_Cell_Voltage                   = %d mV\n", m_hb2_bms_info.Min_Cell_Voltage);
    printf("Max_Cell_Temperature               = %d *0.1'C\n", m_hb2_bms_info.Max_Cell_Temperature);
    printf("Min_Cell_Temperature               = %d *0.1'C\n", m_hb2_bms_info.Min_Cell_Temperature);
// 0x0210
    printf("Master_Average_Cycle_Count         = %d\n", m_hb2_bms_info.Master_Average_Cycle_Count);
    printf("Master_Average_SOH                 = %d %% *0.1\n", m_hb2_bms_info.Master_Average_SOH);
    printf("Battery_Cumulative_Input_Capacity  = %d KWh\n", m_hb2_bms_info.Battery_Cumulative_Input_Capacity);
    printf("Battery_Cumulative_Output_Capacity = %d KWh\n", m_hb2_bms_info.Battery_Cumulative_Output_Capacity);
    printf("Master_FW_Version                  = 0x%04X\n", m_hb2_bms_info.Master_FW_Version);
    printf("Master_Manufacture_Data            = %d\n", m_hb2_bms_info.Master_Manufacture_Data);
    printf("Master_Serial_Number               = %d\n", m_hb2_bms_info.Master_Serial_Number);
    printf("Number_Of_Whole_Cells              = %d\n", m_hb2_bms_info.Number_Of_Whole_Cells);
    printf("Number_Of_Module                   = %d\n", m_hb2_bms_info.Number_Of_Module);
    printf("##############################\n");
}

bool CG320::GetTimezone()
{
    char buf[1024] = {0};
    char tmp[128]= {0};
    char timezone[128] = {0};
    char zonename[128] = {0};
    char time_offset[128] = {0};
    char *index = NULL;
    FILE *pFile = NULL;
    int i = 0, j = 0;

    time_t current_time;
    struct tm *log_time;
    current_time = time(NULL);
    log_time = localtime(&current_time);
    SaveLog((char *)"DataLogger GetTimezone() run.", log_time);

    m_do_get_TZ = true;
    printf("\n########### Get Timezone ###########\n");
    // get timezone from ip
    sprintf(buf, "curl -k %s --max-time 30 > /tmp/timezone", TIMEZONE_URL);
    //printf("cmd = %s\n", buf);
    system(buf);
    pFile = fopen("/tmp/timezone", "rb");
    if ( pFile == NULL ) {
        printf("Open /tmp/timezone fail!\n");
        return false;
    }
    fread(buf, 1024, 1, pFile);
    printf("Debug : /tmp/timezone = %s\n", buf);
    fclose(pFile);

    // find timezone
    index = strstr(buf, "timezone"); // find "timezone":"ZZZ/YYY" ex: Asia/Taipei
    if ( index == NULL ) {
        printf("timezone not found!\n");
        return false;
    }
    strncpy(tmp, index+11, 127); // copy start at Z, example "timezone":"ZZZ/YYY", get ZZZ/YYY, end  of "
    for (i = 0; i < 127; i++) {
        if ( tmp[i] == '"' ) { // stop at "
            timezone[j] = 0;
            break;
        } else if ( (('A' <= tmp[i]) && (tmp[i] <= 'Z')) || (('a' <= tmp[i]) && (tmp[i] <= 'z')) ) { // A ~ Z or a ~ z, copy
            timezone[j] = tmp[i];
            j++;
        } else if (('0' <= tmp[i]) && (tmp[i] <= '9')) { // 0 ~ 9, copy
            timezone[j] = tmp[i];
            j++;
        } else if ( tmp[i] == '/' ) { // /, copy
            timezone[j] = tmp[i];
            j++;
        } else if ( (tmp[i] == '+') || (tmp[i] == '-') ) { // + or - , copy
            timezone[j] = tmp[i];
            j++;
            /*if ( ('0' <= tmp[i+1]) && (tmp[i+1] <= '9') ) { // next byte is number 0 ~ 9, copy
                timezone[j] = tmp[i];
                j++;
            } else { // next byte not number, set .
                timezone[j] = '.';
                j++;
            }*/
        } else if ( tmp[i] == '_' ) { // _ , set to space
            timezone[j] = ' ';
            j++;
        } else { // other, do nothing
            //timezone[j] = '.';
            //j++;
        }
    }
    printf("Debug : timezone[] = %s\n", timezone);
    sprintf(tmp, "DataLogger GetTimezone() : timezone = [%s]", timezone);
    SaveLog(tmp, log_time);

    if ( strlen(timezone) == 0 )
        return false;

    // get time offset from openwrt if match timezone
    sprintf(buf, "curl -k %s --max-time 30 | grep -i \"%s\" > /tmp/time_offset", TIME_OFFSET_URL, timezone);
    //printf("cmd = %s\n", buf);
    system(buf);
    pFile = fopen("/tmp/time_offset", "rb");
    if ( pFile == NULL ) {
        printf("Open /tmp/time_offset fail!\n");
        return false;
    }
    memset(buf, 0x00, 1024);
    fread(buf, 1024, 1, pFile);
    printf("Debug : /tmp/time_offset = %s\n", buf);
    fclose(pFile);
    if ( strlen(buf) == 0 )
        return false;

    // parser timezone & offset from /tmp/time_offset
    index = NULL;
    index = strchr(buf, '\''); // find first "'"
    if ( index == NULL ) {
        printf("start index ' not found!\n");
        return false;
    }
    index++; // set to next char
    // set zonename
    i = 0;
    while ( (*index != '\'') && (*index != NULL) && (i < 128) ) {
        zonename[i] = *index;
        index++;
        i++;
    }
    printf("Debug : zonename[] = %s\n", zonename);
    index++; // set to next char
    index = strchr(index, '\''); // find third "'"
    index++; // set to next char
    // set time_offset
    i = 0;
    while ( (*index != '\'') && (*index != NULL) && (i < 128) ) {
        time_offset[i] = *index;
        index++;
        i++;
    }
    printf("Debug : time_offset[] = %s\n", time_offset);
    sprintf(tmp, "DataLogger GetTimezone() : time_offset = [%s]", time_offset);
    SaveLog(tmp, log_time);

    SetTimezone(zonename, time_offset);
    m_do_get_TZ = false;

    usleep(1000000);
    GetNTPTime();

    printf("####################################\n");

    return true;
}

// zonename : ex Asia/Taipei, timazone : ex CST-8
void CG320::SetTimezone(char *zonename, char *timazone)
{
    char buf[128] = {0};

    sprintf(buf, "uci set system.@system[0].zonename='%s'", zonename);
    system(buf);
    sprintf(buf, "uci set system.@system[0].timezone='%s'", timazone);
    system(buf);
    system("uci commit system");
    system("/etc/init.d/system restart");

    return;
}

void CG320::GetLocalTime()
{
    printf("########### Get Local Time ###########\n");
    time_t timep;
    time(&timep);
    //m_st_time = gmtime(&timep); // get UTC time
    //printf("gmtime : %4d/%02d/%02d ", 1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday);
    //printf("day[%d] %02d:%02d:%02d\n", m_st_time->tm_wday, m_st_time->tm_hour, m_st_time->tm_min, m_st_time->tm_sec);
    m_st_time = localtime(&timep); // get local time
    printf("localtime : %4d/%02d/%02d ", 1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday);
    printf("day[%d] %02d:%02d:%02d\n", m_st_time->tm_wday, m_st_time->tm_hour, m_st_time->tm_min, m_st_time->tm_sec);
    printf("######################################\n");

    return;
}

void CG320::GetNTPTime()
{
    char buf[1024] = {0};
    char NTP_SERVER[4][256] = {0};
    int PID = 0, i = 0;
    FILE *fd = NULL;

    printf("############ Get NTP Time ############\n");

    fd = popen("uci get system.ntp.server", "r");
    if ( fd == NULL ) {
        printf("GetNTPTime server fail!\n");
        return;
    }
    fread(buf, 1, 1024, fd);
    pclose(fd);

    if ( strlen(buf) == 0 ) {
        printf("GetNTPTime server empty!\n");
        return;
    }

    sscanf(buf, "%s %s %s %s", NTP_SERVER[0], NTP_SERVER[1], NTP_SERVER[2], NTP_SERVER[3]);
    printf("NTP_SERVER[0] = %s\n", NTP_SERVER[0]);
    printf("NTP_SERVER[1] = %s\n", NTP_SERVER[1]);
    printf("NTP_SERVER[2] = %s\n", NTP_SERVER[2]);
    printf("NTP_SERVER[3] = %s\n", NTP_SERVER[3]);

    for ( i = 0; i < 4; i++) {
        sprintf(buf, "ntpd -n -d -q -p %s &", NTP_SERVER[i]);
        system(buf);
        printf("wait 10s for ntpd end\n");
        usleep(10000000);

        // check before ntpd process, if exist, kill it!
        sprintf(buf, "ps | grep \"ntpd -n -d -q -p %s\" | grep -v grep | awk '{print $1}'", NTP_SERVER[i]);
        fd = popen(buf, "r");
        if ( fd == NULL ) {
            printf("GetNTPTime check ntpd fail!\n");
            break;
        }
        memset(buf, 0, 1024);
        fread(buf, 1, 1024, fd);
        pclose(fd);

        if ( strlen(buf) == 0 ) {
            printf("GetNTPTime check ntpd empty!\n");
            m_do_set_RTC = 1;
            break;
        }

        sscanf(buf, "%d", &PID);
        sprintf(buf, "kill %d", PID);
        system(buf);
    }

    printf("############ Get NTP END #############\n");

    return;
}

void CG320::SetLogXML()
{
    sprintf(m_log_filename, "%s/%02d%02d%02d", m_dl_path.m_log_path, m_data_st_time.tm_hour, m_data_st_time.tm_min, m_data_st_time.tm_sec);
    //printf("log path = %s\n", m_log_filename);
    return;
}

bool CG320::WriteLogXML(int index)
{
    char buf[256] = {0};
    char idtmp[13] = {0};
    unsigned long long int dev_id = 0;
    int error_tmp = 0;
    int model = 0;

    //time_t current_time;
    //struct tm *log_time;
    //current_time = time(NULL);
    //log_time = localtime(&current_time);

    //SaveLog((char *)"DataLogger WriteLogXML() : run", log_time);
    printf("==================== Set Log XML start ====================\n");
    //if ( first && (strlen(m_log_buf) == 0) ) // empty, new file, add header <records>
    //    strcpy(m_log_buf, "<records>");

    sscanf(arySNobj[index].m_Sn+4, "%04X", &model); // get 5-8 digit from SN
    //printf("Index %d model = %X\n", index, model);

    if ( arySNobj[index].m_Device < 0x0A ) {
        // MI part
        if ( model < 3 ) {
            // G240/300, G320, G321 single channel
            sscanf(arySNobj[index].m_Sn+4, "%012llX", &dev_id); // get last 12 digit
            sprintf(buf, "<record dev_id=\"%lld\" date=\"%04d-%02d-%02d %02d:%02d:%02d\" sn=\"%s\">", dev_id,
                    1900+m_data_st_time.tm_year, 1+m_data_st_time.tm_mon, m_data_st_time.tm_mday,
                    m_data_st_time.tm_hour, m_data_st_time.tm_min, m_data_st_time.tm_sec, arySNobj[index].m_Sn);
            strcat(m_log_buf, buf);

            //if ( m_loopflag == 0 ) {
                sprintf(buf, "<Inv_Temp>%03.1f</Inv_Temp>", ((float)m_mi_power_info.Temperature)/10);
                strcat(m_log_buf, buf);

                // set total KWH, in range 0 ~ 999999999999999.999 KWH, max 0xFFFFFFFF => ‭‭6553599.99?, anyway must in range
                sprintf(buf, "<total_KWH>%05.3f</total_KWH>", m_mi_power_info.Ch2_EacH*100 + ((float)m_mi_power_info.Ch2_EacL)*0.01);
                strcat(m_log_buf, buf);

                // set ac power, in range 0 ~ 999999.999 KW, max 0xFFFF = 65535 => 6.5535 KW, must in range
                sprintf(buf, "<ac_power_A>%05.3f</ac_power_A>", ((float)m_mi_power_info.Total_Pac)/10000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_power>%05.3f</ac_power>", ((float)m_mi_power_info.Total_Pac)/10000);
                strcat(m_log_buf, buf);

                // set dc voltage, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 6553.5 V, must in range
                sprintf(buf, "<dcv_1>%03.1f</dcv_1>", ((float)m_mi_power_info.Ch1_Vpv)*0.1);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_voltage>%03.1f</dc_voltage>", ((float)m_mi_power_info.Ch1_Vpv)*0.1);
                strcat(m_log_buf, buf);

                // set dc current, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 655.35 A, must in range
                sprintf(buf, "<dci_1>%04.2f</dci_1>", ((float)m_mi_power_info.Ch1_Ipv)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_current>%04.2f</dc_current>", ((float)m_mi_power_info.Ch1_Ipv)/100);
                strcat(m_log_buf, buf);

                // set dc power, in range 0 ~ 999999.999 KW, max 0xFFFF = 65535 => 6.5535 KW, must in range
                sprintf(buf, "<dc_power>%05.3f</dc_power>", ((float)m_mi_power_info.Ch1_Ppv)/10000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_power_1>%05.3f</dc_power_1>", ((float)m_mi_power_info.Ch1_Ppv)/10000);
                strcat(m_log_buf, buf);

                // set ac voltage, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 6553.5 V, must in range
                sprintf(buf, "<acv_AN>%03.1f</acv_AN>", ((float)m_mi_power_info.Vac)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_voltage>%03.1f</ac_voltage>", ((float)m_mi_power_info.Vac)/10);
                strcat(m_log_buf, buf);

                // set ac current, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 65.535 A, must in range
                sprintf(buf, "<aci_A>%05.3f</aci_A>", ((float)m_mi_power_info.Total_Iac)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_current>%05.3f</ac_current>", ((float)m_mi_power_info.Total_Iac)/1000);
                strcat(m_log_buf, buf);

                // set frequency, in range 0 ~ 999.99 Hz, max 0xFFFF = 65535 => 655.35 Hz, must in range
                sprintf(buf, "<frequency>%04.2f</frequency>", ((float)m_mi_power_info.Fac)/100);
                strcat(m_log_buf, buf);

                // FW ver
                sprintf(buf, "<Ver_MI>%d</Ver_MI>", arySNobj[index].m_FWver);
                strcat(m_log_buf, buf);

                // set error code
                if ( m_mi_power_info.Error_Code1 )
                    error_tmp = m_mi_power_info.Error_Code1;
                else if ( m_mi_power_info.Error_Code2 )
                    error_tmp = m_mi_power_info.Error_Code2;
                else
                    error_tmp = 0;
                sprintf(buf, "<Error_Code>%d</Error_Code>", error_tmp);
                strcat(m_log_buf, buf);
            //}

            // set status
            if ( error_tmp ) {
                strcat(m_log_buf, "<Status>2</Status>");
            } else {
                if ( m_loopflag == 1 ) {
                    strcat(m_log_buf, "<Status>1</Status>");
                    m_inverter_state = 1;
                    m_sys_error |= SYS_0010_Off_Line;
                } else {
                    strcat(m_log_buf, "<Status>0</Status>");
                    m_inverter_state = 0;
                    m_sys_error &= ~SYS_0010_Off_Line;
                }
            }

        } else {
            // G640, G642, G640R dual channel
            // set ch1
            idtmp[0] = 'A';
            strcpy(idtmp+1, arySNobj[index].m_Sn+5);
            sscanf(idtmp, "%012llX", &dev_id); // get last 12 digit
            sprintf(buf, "<record dev_id=\"%lld\" date=\"%04d-%02d-%02d %02d:%02d:%02d\" sn=\"A%s\">", dev_id,
                    1900+m_data_st_time.tm_year, 1+m_data_st_time.tm_mon, m_data_st_time.tm_mday,
                    m_data_st_time.tm_hour, m_data_st_time.tm_min, m_data_st_time.tm_sec, arySNobj[index].m_Sn+1);
            strcat(m_log_buf, buf);

            //if ( m_loopflag == 0 ) {
                sprintf(buf, "<Inv_Temp>%03.1f</Inv_Temp>", ((float)m_mi_power_info.Temperature)/10);
                strcat(m_log_buf, buf);

                // set total KWH, in range 0 ~ 999999999999999.999 KWH, max 0xFFFFFFFF => ‭‭6553599.99?, anyway must in range
                sprintf(buf, "<total_KWH>%05.3f</total_KWH>", m_mi_power_info.Ch1_EacH*100 + ((float)m_mi_power_info.Ch1_EacL)*0.01);
                strcat(m_log_buf, buf);

                // set ac power, in range 0 ~ 999999.999 KW, max 0xFFFF = 65535 => 6.5535 KW, must in range
                sprintf(buf, "<ac_power_A>%05.3f</ac_power_A>", ((float)m_mi_power_info.Ch1_Pac)/10000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_power>%05.3f</ac_power>", ((float)m_mi_power_info.Ch1_Pac)/10000);
                strcat(m_log_buf, buf);

                // set dc voltage, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 6553.5 V, must in range
                sprintf(buf, "<dcv_1>%03.1f</dcv_1>", ((float)m_mi_power_info.Ch1_Vpv)*0.1);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_voltage>%03.1f</dc_voltage>", ((float)m_mi_power_info.Ch1_Vpv)*0.1);
                strcat(m_log_buf, buf);

                // set dc current, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 655.35 A, must in range
                sprintf(buf, "<dci_1>%04.2f</dci_1>", ((float)m_mi_power_info.Ch1_Ipv)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_current>%04.2f</dc_current>", ((float)m_mi_power_info.Ch1_Ipv)/100);
                strcat(m_log_buf, buf);

                // set dc power, in range 0 ~ 999999.999 KW, max 0xFFFF = 65535 => 6.5535 KW, must in range
                sprintf(buf, "<dc_power>%05.3f</dc_power>", ((float)m_mi_power_info.Ch1_Ppv)/10000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_power_1>%05.3f</dc_power_1>", ((float)m_mi_power_info.Ch1_Ppv)/10000);
                strcat(m_log_buf, buf);

                // set ac voltage, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 6553.5 V, must in range
                sprintf(buf, "<acv_AN>%03.1f</acv_AN>", ((float)m_mi_power_info.Vac)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_voltage>%03.1f</ac_voltage>", ((float)m_mi_power_info.Vac)/10);
                strcat(m_log_buf, buf);

                //sprintf(buf, "<aci_A>%05.3f</aci_A>", ((float)m_mi_power_info.Total_Iac)/1000);
                //strcat(m_log_buf, buf);
                //sprintf(buf, "<ac_current>%05.3f</ac_current>", ((float)m_mi_power_info.Total_Iac)/1000);
                //strcat(m_log_buf, buf);
                if ( m_mi_power_info.Vac == 0 ) {
                    sprintf(buf, "<aci_A>0</aci_A>");
                    strcat(m_log_buf, buf);
                    sprintf(buf, "<ac_current>0</ac_current>");
                    strcat(m_log_buf, buf);
                } else {
                    // set ac current, in range 0 ~ 9999.99 V
                    if ( (((float)m_mi_power_info.Ch1_Pac)/((float)m_mi_power_info.Vac)) < 10000 ) {
                        sprintf(buf, "<aci_A>%05.3f</aci_A>", ((float)m_mi_power_info.Ch1_Pac)/((float)m_mi_power_info.Vac));
                        strcat(m_log_buf, buf);
                        sprintf(buf, "<ac_current>%05.3f</ac_current>", ((float)m_mi_power_info.Ch1_Pac)/((float)m_mi_power_info.Vac));
                        strcat(m_log_buf, buf);
                    }
                }

                // set frequency, in range 0 ~ 999.99 Hz, max 0xFFFF = 65535 => 655.35 Hz, must in range
                sprintf(buf, "<frequency>%04.2f</frequency>", ((float)m_mi_power_info.Fac)/100);
                strcat(m_log_buf, buf);

		// FW ver
                sprintf(buf, "<Ver_MI>%d</Ver_MI>", arySNobj[index].m_FWver);
                strcat(m_log_buf, buf);

                // set error code
                if ( m_mi_power_info.Error_Code1 )
                    error_tmp = m_mi_power_info.Error_Code1;
                else if ( m_mi_power_info.Error_Code2 )
                    error_tmp = m_mi_power_info.Error_Code2;
                else
                    error_tmp = 0;
                sprintf(buf, "<Error_Code>%d</Error_Code>", error_tmp);
                strcat(m_log_buf, buf);
            //}

            // set status
            if ( error_tmp ) {
                strcat(m_log_buf, "<Status>2</Status>");
            } else {
                if ( m_loopflag == 1 ) {
                    strcat(m_log_buf, "<Status>1</Status>");
                    m_inverter_state = 1;
                    m_sys_error |= SYS_0010_Off_Line;
                } else {
                    strcat(m_log_buf, "<Status>0</Status>");
                    m_inverter_state = 0;
                    m_sys_error &= ~SYS_0010_Off_Line;
                }
            }

            strcat(m_log_buf, "</record>");

            // set ch2
            idtmp[0] = 'B';
            //strcpy(idtmp+1, arySNobj[index].m_Sn+5);
            sscanf(idtmp, "%012llX", &dev_id); // get last 12 digit
            sprintf(buf, "<record dev_id=\"%lld\" date=\"%04d-%02d-%02d %02d:%02d:%02d\" sn=\"B%s\">", dev_id,
                    1900+m_data_st_time.tm_year, 1+m_data_st_time.tm_mon, m_data_st_time.tm_mday,
                    m_data_st_time.tm_hour, m_data_st_time.tm_min, m_data_st_time.tm_sec, arySNobj[index].m_Sn+1);
            strcat(m_log_buf, buf);

            //if ( m_loopflag == 0 ) {
                sprintf(buf, "<Inv_Temp>%03.1f</Inv_Temp>", ((float)m_mi_power_info.Temperature)/10);
                strcat(m_log_buf, buf);

                // set total KWH, in range 0 ~ 999999999999999.999 KWH, max 0xFFFFFFFF => ‭‭6553599.99?, anyway must in range
                sprintf(buf, "<total_KWH>%05.3f</total_KWH>", m_mi_power_info.Ch2_EacH*100 + ((float)m_mi_power_info.Ch2_EacL)*0.01);
                strcat(m_log_buf, buf);

                // set ac power, in range 0 ~ 999999.999 KW, max 0xFFFF = 65535 => 6.5535 KW, must in range
                sprintf(buf, "<ac_power_A>%05.3f</ac_power_A>", ((float)m_mi_power_info.Ch2_Pac)/10000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_power>%05.3f</ac_power>", ((float)m_mi_power_info.Ch2_Pac)/10000);
                strcat(m_log_buf, buf);

                // set dc voltage, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 6553.5 V, must in range
                sprintf(buf, "<dcv_1>%03.1f</dcv_1>", ((float)m_mi_power_info.Ch2_Vpv)*0.1);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_voltage>%03.1f</dc_voltage>", ((float)m_mi_power_info.Ch2_Vpv)*0.1);
                strcat(m_log_buf, buf);

                // set dc current, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 655.35 A, must in range
                sprintf(buf, "<dci_1>%04.2f</dci_1>", ((float)m_mi_power_info.Ch2_Ipv)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_current>%04.2f</dc_current>", ((float)m_mi_power_info.Ch2_Ipv)/100);
                strcat(m_log_buf, buf);

                // set dc power, in range 0 ~ 999999.999 KW, max 0xFFFF = 65535 => 6.5535 KW, must in range
                sprintf(buf, "<dc_power>%05.3f</dc_power>", ((float)m_mi_power_info.Ch2_Ppv)/10000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_power_1>%05.3f</dc_power_1>", ((float)m_mi_power_info.Ch2_Ppv)/10000);
                strcat(m_log_buf, buf);

                // set ac voltage, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 6553.5 V, must in range
                sprintf(buf, "<acv_AN>%03.1f</acv_AN>", ((float)m_mi_power_info.Vac)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_voltage>%03.1f</ac_voltage>", ((float)m_mi_power_info.Vac)/10);
                strcat(m_log_buf, buf);

                //sprintf(buf, "<aci_A>%05.3f</aci_A>", ((float)m_mi_power_info.Total_Iac)/1000);
                //strcat(m_log_buf, buf);
                //sprintf(buf, "<ac_current>%05.3f</ac_current>", ((float)m_mi_power_info.Total_Iac)/1000);
                //strcat(m_log_buf, buf);
                if ( m_mi_power_info.Vac == 0 ) {
                    sprintf(buf, "<aci_A>0</aci_A>");
                    strcat(m_log_buf, buf);
                    sprintf(buf, "<ac_current>0</ac_current>");
                    strcat(m_log_buf, buf);
                } else {
                    // set ac current, in range 0 ~ 9999.99 V
                    if ( (((float)m_mi_power_info.Ch2_Pac)/((float)m_mi_power_info.Vac)) < 10000 ) {
                        sprintf(buf, "<aci_A>%05.3f</aci_A>", ((float)m_mi_power_info.Ch2_Pac)/((float)m_mi_power_info.Vac));
                        strcat(m_log_buf, buf);
                        sprintf(buf, "<ac_current>%05.3f</ac_current>", ((float)m_mi_power_info.Ch2_Pac)/((float)m_mi_power_info.Vac));
                        strcat(m_log_buf, buf);
                    }
                }

                // set frequency, in range 0 ~ 999.99 Hz, max 0xFFFF = 65535 => 655.35 Hz, must in range
                sprintf(buf, "<frequency>%04.2f</frequency>", ((float)m_mi_power_info.Fac)/100);
                strcat(m_log_buf, buf);

		// FW ver
                sprintf(buf, "<Ver_MI>%d</Ver_MI>", arySNobj[index].m_FWver);
                strcat(m_log_buf, buf);

                // set error code
                if ( m_mi_power_info.Error_Code1 )
                    error_tmp = m_mi_power_info.Error_Code1;
                else if ( m_mi_power_info.Error_Code2 )
                    error_tmp = m_mi_power_info.Error_Code2;
                else
                    error_tmp = 0;
                sprintf(buf, "<Error_Code>%d</Error_Code>", error_tmp);
                strcat(m_log_buf, buf);
            //}

            // set status
            if ( error_tmp ) {
                strcat(m_log_buf, "<Status>2</Status>");
            } else {
                if ( m_loopflag == 1 ) {
                    strcat(m_log_buf, "<Status>1</Status>");
                    m_inverter_state = 1;
                    m_sys_error |= SYS_0010_Off_Line;
                } else {
                    strcat(m_log_buf, "<Status>0</Status>");
                    m_inverter_state = 0;
                    m_sys_error &= ~SYS_0010_Off_Line;
                }
            }
        }
    } else {
        // Hybrid part
        // set slave ID, date time, SN
        sscanf(arySNobj[index].m_Sn+4, "%012llX", &dev_id); // get last 12 digit
        sprintf(buf, "<record dev_id=\"%lld\" date=\"%04d-%02d-%02d %02d:%02d:%02d\" sn=\"%s\">", dev_id,
                1900+m_data_st_time.tm_year, 1+m_data_st_time.tm_mon, m_data_st_time.tm_mday,
                m_data_st_time.tm_hour, m_data_st_time.tm_min, m_data_st_time.tm_sec, arySNobj[index].m_Sn);
        strcat(m_log_buf, buf);

        //if ( m_loopflag == 0 ) {
            // set real time part///////////////////////////////////////////////////////////////////////////////////////////
            // set DC power (KW)
            // set dc power, in range 0 ~ 999999.999 KW, max 0xFFFF = 65535 => 65.535 KW, must in range
            if ( m_save_hb_rt_info ) {
                sprintf(buf, "<dc_power>%05.3f</dc_power>", ((float)m_hb_rt_info.PV_Total_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_power_1>%05.3f</dc_power_1>", ((float)m_hb_rt_info.PV1_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_power_2>%05.3f</dc_power_2>", ((float)m_hb_rt_info.PV2_Power)/1000);
                strcat(m_log_buf, buf);
                // set DC voltage (V)
                // set dc voltage, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 65535 V
                if ( m_hb_rt_info.PV1_Voltage < 10000 ) {
                    sprintf(buf, "<dcv_1>%d</dcv_1>", m_hb_rt_info.PV1_Voltage);
                    strcat(m_log_buf, buf);
                }
                if ( m_hb_rt_info.PV2_Voltage < 10000 ) {
                    sprintf(buf, "<dcv_2>%d</dcv_2>", m_hb_rt_info.PV2_Voltage);
                    strcat(m_log_buf, buf);
                }
                if ( ((m_hb_rt_info.PV1_Voltage + m_hb_rt_info.PV2_Voltage)/2) < 10000 ) {
                    sprintf(buf, "<dc_voltage>%03.1f</dc_voltage>", ((float)(m_hb_rt_info.PV1_Voltage + m_hb_rt_info.PV2_Voltage))/2);
                    strcat(m_log_buf, buf);
                }
                // set DC current (A)
                // set dc current, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 655.35 A, must in range
                sprintf(buf, "<dci_1>%04.2f</dci_1>", ((float)m_hb_rt_info.PV1_Current)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dci_2>%04.2f</dci_2>", ((float)m_hb_rt_info.PV2_Current)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_current>%d</dc_current>", m_hb_rt_info.PV1_Current + m_hb_rt_info.PV2_Current);
                strcat(m_log_buf, buf);

                // set AC power (KW)
                // set ac power, in range 0 ~ 999999.999 KW, max 0xFFFF = 65535 => 65.535 KW, must in range
                sprintf(buf, "<ac_power_A>%05.3f</ac_power_A>", ((float)m_hb_rt_info.Load_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_power>%05.3f</ac_power>", ((float)m_hb_rt_info.Load_Power)/1000);
                strcat(m_log_buf, buf);
                // set AC voltage (V)
                // set ac voltage, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 65535 V
                if ( m_hb_rt_info.Load_Voltage < 10000 ) {
                    sprintf(buf, "<acv_AN>%d</acv_AN>", m_hb_rt_info.Load_Voltage);
                    strcat(m_log_buf, buf);
                    sprintf(buf, "<ac_voltage>%d</ac_voltage>", m_hb_rt_info.Load_Voltage);
                    strcat(m_log_buf, buf);
                }
                // set AC current (A)
                // set ac current, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 655.35 A, must in range
                sprintf(buf, "<aci_A>%04.2f</aci_A>", ((float)m_hb_rt_info.Load_Current)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_current>%04.2f</ac_current>", ((float)m_hb_rt_info.Load_Current)/100);
                strcat(m_log_buf, buf);

                // set total power
                // set total KWH, in range 0 ~ 999999999999999.999 KWH, max 0xFFFFFFFF => ‭‭6553599.99?, anyway must in range
                sprintf(buf, "<total_KWH>%04.2f</total_KWH>", m_hb_rt_info.PV_Total_EnergyH*100 + ((float)m_hb_rt_info.PV_Total_EnergyL)*0.01);
                strcat(m_log_buf, buf);

                // set battery SOC
                // set soc, in range 0 ~ 9999.99 %, max 0xFFFF = > 65535
                if ( m_hb_rt_info.Battery_SOC < 10000 ) {
                    sprintf(buf, "<soc>%d</soc>", m_hb_rt_info.Battery_SOC);
                    strcat(m_log_buf, buf);
                }

                // set temperature
                sprintf(buf, "<Inv_Temp>%03.1f</Inv_Temp>", ((float)m_hb_rt_info.Inv_Temp)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PV1_Temp>%03.1f</PV1_Temp>", ((float)m_hb_rt_info.PV1_Temp)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PV2_Temp>%03.1f</PV2_Temp>", ((float)m_hb_rt_info.PV2_Temp)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<DD_Temp>%03.1f</DD_Temp>", ((float)m_hb_rt_info.DD_Temp)/10);
                strcat(m_log_buf, buf);

                // set Grid
                sprintf(buf, "<VGrid_A>%d</VGrid_A>", m_hb_rt_info.Grid_Voltage);
                strcat(m_log_buf, buf);
                sprintf(buf, "<IGrid_A>%04.2f</IGrid_A>", ((float)m_hb_rt_info.Grid_Current)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PGrid_A>%05.3f</PGrid_A>", ((float)m_hb_rt_info.Grid_Power)/1000);
                strcat(m_log_buf, buf);

                // set battery
                sprintf(buf, "<VBattery>%03.1f</VBattery>", ((float)m_hb_rt_info.Battery_Voltage)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<IBattery>%03.1f</IBattery>", ((float)m_hb_rt_info.Battery_Current)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PBattery>%05.3f</PBattery>", ((float)m_hb_rt_info.PBat)/1000);
                strcat(m_log_buf, buf);


                // set bus
                sprintf(buf, "<Vbus>%03.1f</Vbus>", ((float)m_hb_rt_info.Bus_Voltage)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Ibus>%03.1f</Ibus>", ((float)m_hb_rt_info.Bus_Current)/10);
                strcat(m_log_buf, buf);

                // set battery power
                sprintf(buf, "<Pbat_Total>%04.2f</Pbat_Total>", m_hb_rt_info.Bat_Total_EnergyH*100 + ((float)m_hb_rt_info.Bat_Total_EnergyL)*0.01);
                strcat(m_log_buf, buf);

                // set load power
                sprintf(buf, "<Pload_Total>%04.2f</Pload_Total>", m_hb_rt_info.Load_Total_EnergyH*100 + ((float)m_hb_rt_info.Load_Total_EnergyL)*0.01);
                strcat(m_log_buf, buf);

                // set grid feed power
                sprintf(buf, "<GridFeed_Total>%04.2f</GridFeed_Total>", m_hb_rt_info.GridFeed_TotalH*100 + ((float)m_hb_rt_info.GridFeed_TotalL)*0.01);
                strcat(m_log_buf, buf);

                // set grid charge power
                sprintf(buf, "<GridCharge_Total>%04.2f</GridCharge_Total>", m_hb_rt_info.GridCharge_TotalH*100 + ((float)m_hb_rt_info.GridCharge_TotalL)*0.01);
                strcat(m_log_buf, buf);

                // set external power
                sprintf(buf, "<ExtPower>%05.3f</ExtPower>", ((float)m_hb_rt_info.External_Power)/10);
                strcat(m_log_buf, buf);

                // set system state
                sprintf(buf, "<Sys_State>%d</Sys_State>", m_hb_rt_info.Sys_State);
                strcat(m_log_buf, buf);

                // set Icon
                sprintf(buf, "<Hybrid_Icon>%d</Hybrid_Icon>", (m_hb_rt_info.Hybrid_IconH << 16) + m_hb_rt_info.Hybrid_IconL);
                strcat(m_log_buf, buf);

                // set error code
                sprintf(buf, "<Error_Code>%d</Error_Code>", m_hb_rt_info.Error_Code);
                strcat(m_log_buf, buf);

                // set frequency, in range 0 ~ 999.9 Hz, max 0xFFFF = 65535 => 6553.5 Hz
                if ( m_hb_rt_info.Invert_Frequency < 10000 ) {
                    sprintf(buf, "<Inverterfrequency>%03.1f</Inverterfrequency>", ((float)m_hb_rt_info.Invert_Frequency)/10);
                    strcat(m_log_buf, buf);
                }
                // set frequency, in range 0 ~ 999.99 Hz, max 0xFFFF = 65535 => 6553.5 Hz
                if ( m_hb_rt_info.Grid_Frequency < 10000 ) {
                    sprintf(buf, "<frequency>%03.1f</frequency>", ((float)m_hb_rt_info.Grid_Frequency)/10);
                    strcat(m_log_buf, buf);
                }
            }
        //}

        // set status
        if ( arySNobj[index].m_Model < 16 ) { // H5000/5001
            if ( m_hb_rt_info.Error_Code || m_hb_rt_info.PV_Inv_Error_COD1_Record || m_hb_rt_info.PV_Inv_Error_COD2_Record || m_hb_rt_info.PV_Inv_Error_COD3_Record ||
                m_hb_rt_info.DD_Error_COD_Record || m_hb_rt_info.DD_Error_COD2_Record ) {
                strcat(m_log_buf, "<Status>2</Status>");
            } else {
                if ( m_loopflag == 6 ) {
                    strcat(m_log_buf, "<Status>1</Status>");
                    m_inverter_state = 1;
                    m_sys_error |= SYS_0010_Off_Line;
                } else {
                    strcat(m_log_buf, "<Status>0</Status>");
                    m_inverter_state = 0;
                    m_sys_error &= ~SYS_0010_Off_Line;
                }
            }
        } else { // H5500/9600
            if ( m_hb2_rt_info.Error_Code || m_hb2_rt_info.PV_Inv_Error_COD1_Record || m_hb2_rt_info.PV_Inv_Error_COD2_Record || m_hb2_rt_info.PV_Inv_Error_COD3_Record ||
                m_hb2_rt_info.DD_Error_COD1_Record || m_hb2_rt_info.DD_Error_COD2_Record ) {
                strcat(m_log_buf, "<Status>2</Status>");
            } else {
                if ( m_loopflag == 7 ) {
                    strcat(m_log_buf, "<Status>1</Status>");
                    m_inverter_state = 1;
                    m_sys_error |= SYS_0010_Off_Line;
                } else {
                    strcat(m_log_buf, "<Status>0</Status>");
                    m_inverter_state = 0;
                    m_sys_error &= ~SYS_0010_Off_Line;
                }
            }
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //if ( m_loopflag == 0 ) {
            // set BMS module0
            if ( m_save_hb_bms_info ) {
                sprintf(buf, "<BMS_Voltage>%04.2f</BMS_Voltage>", ((float)m_hb_bms_info.Voltage/100));
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_Current>%04.2f</BMS_Current>", ((float)m_hb_bms_info.Current/100));
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_SOC>%d</BMS_SOC>", m_hb_bms_info.SOC);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_Max_Temp>%d</BMS_Max_Temp>", m_hb_bms_info.MaxTemperature);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_CycleCount>%d</BMS_CycleCount>", m_hb_bms_info.CycleCount);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_Status>%d</BMS_Status>", m_hb_bms_info.Status);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_Error>%d</BMS_Error>", m_hb_bms_info.Error);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_ModuleNo>%d</BMS_ModuleNo>", m_hb_bms_info.Number);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_Info>%d</BMS_Info>", m_hb_bms_info.BMS_Info);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_MaxCell>%05.3f</BMS_MaxCell>", ((float)m_hb_bms_info.BMS_Max_Cell/1000));
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_MinCell>%05.3f</BMS_MinCell>", ((float)m_hb_bms_info.BMS_Min_Cell/1000));
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_BaudRate>%d</BMS_BaudRate>", m_hb_bms_info.BMS_BaudRate);
                strcat(m_log_buf, buf);
            }

            // set BMS ver
            if ( m_hb_bms_ver ) {
                sprintf(buf, "<BMS_Ver>%d</BMS_Ver>", m_hb_bms_ver);
                strcat(m_log_buf, buf);
            }

            // set remote setting
            if ( m_save_hb_rs_info ) {
                sprintf(buf, "<InverterMode>%d</InverterMode>", m_hb_rs_info.Mode);
                strcat(m_log_buf, buf);
                sprintf(buf, "<StarHour>%d</StarHour>", m_hb_rs_info.StarHour);
                strcat(m_log_buf, buf);
                sprintf(buf, "<StarMin>%d</StarMin>", m_hb_rs_info.StarMin);
                strcat(m_log_buf, buf);
                sprintf(buf, "<EndHour>%d</EndHour>", m_hb_rs_info.EndHour);
                strcat(m_log_buf, buf);
                sprintf(buf, "<EndMin>%d</EndMin>", m_hb_rs_info.EndMin);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Multi_Module>%d</Multi_Module>", m_hb_rs_info.MultiModuleSetting);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Battery_Type>%d</Battery_Type>", m_hb_rs_info.BatteryType);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ChargeCurrent>%d</ChargeCurrent>", m_hb_rs_info.BatteryCurrent);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BatShutdownVolt>%03.1f</BatShutdownVolt>", ((float)m_hb_rs_info.BatteryShutdownVoltage)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BatFloatingVolt>%03.1f</BatFloatingVolt>", ((float)m_hb_rs_info.BatteryFloatingVoltage)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BatReservePercent>%d</BatReservePercent>", m_hb_rs_info.BatteryReservePercentage);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PeakShavingPower>%05.3f</PeakShavingPower>", ((float)m_hb_rs_info.PeakShavingPower)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<StartFrequency>%03.1f</StartFrequency>", ((float)m_hb_rs_info.StartFrequency)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<EndFrequency>%03.1f</EndFrequency>", ((float)m_hb_rs_info.EndFrequency)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<FeedInPower>%05.3f</FeedInPower>", ((float)m_hb_rs_info.FeedinPower)/10);
                strcat(m_log_buf, buf);
            }

            // set ID part
            // set grid voltage
            if ( m_save_hb_id_data ) {
                sprintf(buf, "<Grid_Voltage>%d</Grid_Voltage>", m_hb_id_data.Grid_Voltage);
                strcat(m_log_buf, buf);
                // set model
                sprintf(buf, "<Model>%d</Model>", m_hb_id_data.Model);
                strcat(m_log_buf, buf);
                // set hw ver
                sprintf(buf, "<HWver>%d</HWver>", m_hb_id_data.HW_Ver);
                strcat(m_log_buf, buf);
                // set date
                sprintf(buf, "<Product_Y>%04d</Product_Y>", m_hb_id_data.Year);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Product_M>%02d</Product_M>", m_hb_id_data.Month);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Product_D>%02d</Product_D>", m_hb_id_data.Date);
                strcat(m_log_buf, buf);
                // set version
                sprintf(buf, "<Ver_HW>%d</Ver_HW>", m_hb_id_data.Inverter_Ver);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Ver_FW>%d</Ver_FW>", m_hb_id_data.DD_Ver);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Ver_EE>%d</Ver_EE>", m_hb_id_data.EEPROM_Ver);
                strcat(m_log_buf, buf);
                sprintf(buf, "<DisplayVer>%d</DisplayVer>", m_hb_id_data.Display_Ver);
                strcat(m_log_buf, buf);
                // set flags1
                sprintf(buf, "<SystemFlag>%d</SystemFlag>", m_hb_id_data.Flags1);
                strcat(m_log_buf, buf);
                // set flags2
                sprintf(buf, "<Rule_Flag>%d</Rule_Flag>", m_hb_id_data.Flags2);
                strcat(m_log_buf, buf);
            }

            // set RTC part
            if ( m_save_hb_rtc_data ) {
                sprintf(buf, "<Second>%d</Second>", m_hb_rtc_data.Second);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Minute>%d</Minute>", m_hb_rtc_data.Minute);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Hour>%d</Hour>", m_hb_rtc_data.Hour);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Day>%d</Day>", m_hb_rtc_data.Date);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Month>%d</Month>", m_hb_rtc_data.Month);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Year>%d</Year>", m_hb_rtc_data.Year);
                strcat(m_log_buf, buf);
            }

            // set remote real time setting
            if ( m_save_hb_rrs_info ) {
                sprintf(buf, "<ChargeSetting>%d</ChargeSetting>", m_hb_rrs_info.ChargeSetting);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ChargePower>%05.3f</ChargePower>", ((float)m_hb_rrs_info.ChargePower)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<DischargePower>%05.3f</DischargePower>", ((float)m_hb_rrs_info.DischargePower)/10);
                strcat(m_log_buf, buf);
                // charge discharge power undefine in web server
                sprintf(buf, "<RampRate>%d</RampRate>", m_hb_rrs_info.RampRatePercentage);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Degree>%d</Degree>", m_hb_rrs_info.DegreeLeadLag);
                strcat(m_log_buf, buf);
                // AC_Coupling_Power
                sprintf(buf, "<AC_Coupling_Power>%d</AC_Coupling_Power>", m_hb_rrs_info.AC_Coupling_Power);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Q_Value>%d</Q_Value>", m_hb_rrs_info.Volt_VAr);
                strcat(m_log_buf, buf);
                // SunSpec_Write_All
                sprintf(buf, "<SunSpec_Write_All>%d</SunSpec_Write_All>", m_hb_rrs_info.SunSpec_Write_All);
                strcat(m_log_buf, buf);
                // Remote_Control
                sprintf(buf, "<Remote_Control>%d</Remote_Control>", m_hb_rrs_info.Remote_Control);
                strcat(m_log_buf, buf);
            }
        //}

            // set id data
            if ( m_save_hb2_id_data ) {
            // 0x0001
                sprintf(buf, "<Grid_Voltage>%d</Grid_Voltage>", m_hb2_id_data.Grid_Voltage);
                strcat(m_log_buf, buf);
                // set model
                sprintf(buf, "<Model>%d</Model>", m_hb2_id_data.Model);
                strcat(m_log_buf, buf);
                // set date
                sprintf(buf, "<Product_Y>%04d</Product_Y>", m_hb2_id_data.Year);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Product_M>%02d</Product_M>", m_hb2_id_data.Month);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Product_D>%02d</Product_D>", m_hb2_id_data.Date);
                strcat(m_log_buf, buf);
                // set version
                sprintf(buf, "<Ver_HW>%d</Ver_HW>", m_hb2_id_data.Inverter_Ver);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Ver_FW>%d</Ver_FW>", m_hb2_id_data.DD_Ver);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Ver_EE>%d</Ver_EE>", m_hb2_id_data.Parameter_Ver);
                strcat(m_log_buf, buf);
                sprintf(buf, "<DisplayVer>%d</DisplayVer>", m_hb2_id_data.Display_Ver);
                strcat(m_log_buf, buf);
                // set hw ver
                sprintf(buf, "<HWver>%d</HWver>", m_hb2_id_data.HW_Ver);
                strcat(m_log_buf, buf);
                // set flags1
                sprintf(buf, "<Safety_Control>%d</Safety_Control>", m_hb2_id_data.Safety_Control);
                strcat(m_log_buf, buf);
            }

            // set remote setting
            if ( m_save_hb2_rs_info ) {
            // 0x00B0
                // remove ori 0x091 ~ 0x94, set 0
                sprintf(buf, "<StarHour>0</StarHour>");
                strcat(m_log_buf, buf);
                sprintf(buf, "<StarMin>0</StarMin>");
                strcat(m_log_buf, buf);
                sprintf(buf, "<EndHour>0</EndHour>");
                strcat(m_log_buf, buf);
                sprintf(buf, "<EndMin>0</EndMin>");
                strcat(m_log_buf, buf);
                //
                sprintf(buf, "<InverterMode>%d</InverterMode>", m_hb2_rs_info.Mode);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ChargeSetting>%d</ChargeSetting>", m_hb2_rs_info.BatteryPolic);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Multi_Module>%d</Multi_Module>", m_hb2_rs_info.MultiModuleSetting);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Battery_Type>%d</Battery_Type>", m_hb2_rs_info.BatteryType);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ChargePower>%05.3f</ChargePower>", ((float)m_hb2_rs_info.ChargePower)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<DischargePower>%05.3f</DischargePower>", ((float)m_hb2_rs_info.DischargePower)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<FeedInPower>%05.3f</FeedInPower>", ((float)m_hb2_rs_info.FeedinPower)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<AC_Coupling_Power>%d</AC_Coupling_Power>", m_hb2_rs_info.BatteryPowerRating);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ChargeCurrent>%03.1f</ChargeCurrent>", ((float)m_hb2_rs_info.MaxBatteryChargingCurrent)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BatShutdownVolt>%03.1f</BatShutdownVolt>", ((float)m_hb2_rs_info.BatteryShutdownVoltage)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BatFloatingVolt>%03.1f</BatFloatingVolt>", ((float)m_hb2_rs_info.BatteryAbsorptionChargingVoltage)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BatReservePercent>%d</BatReservePercent>", m_hb2_rs_info.BatteryReservePercentage);
                strcat(m_log_buf, buf);
                sprintf(buf, "<GridTiedMaxSOC>%d</GridTiedMaxSOC>", m_hb2_rs_info.GridTiedMaxSOC);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PeakShavingPower>%05.3f</PeakShavingPower>", ((float)m_hb2_rs_info.PeakShavingPower)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<StartFrequency>%03.1f</StartFrequency>", ((float)m_hb2_rs_info.StartFrequency)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<EndFrequency>%03.1f</EndFrequency>", ((float)m_hb2_rs_info.EndFrequency)/10);
                strcat(m_log_buf, buf);
            // 0x00C0
                sprintf(buf, "<RampRate>%d</RampRate>", m_hb2_rs_info.RampRatePercentage);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Degree>%d</Degree>", m_hb2_rs_info.DegreeLeadLag);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Q_Value>%d</Q_Value>", m_hb2_rs_info.PowerFactorControl);
                strcat(m_log_buf, buf);
                sprintf(buf, "<TMP_C3>%d</TMP_C3>", m_hb2_rs_info.TMP_C3);
                strcat(m_log_buf, buf);
                sprintf(buf, "<SpecifiedModbusSlaveID>%d</SpecifiedModbusSlaveID>", m_hb2_rs_info.SpecifiedModbusSlaveID);
                strcat(m_log_buf, buf);
                sprintf(buf, "<FunctionControl1>%d</FunctionControl1>", m_hb2_rs_info.FunctionControl1);
                strcat(m_log_buf, buf);
                sprintf(buf, "<FunctionControl2>%d</FunctionControl2>", m_hb2_rs_info.FunctionControl2);
                strcat(m_log_buf, buf);
                sprintf(buf, "<SunSpec_Write_All>%d</SunSpec_Write_All>", m_hb2_rs_info.SunSpecWritable);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Remote_Control>%d</Remote_Control>", m_hb2_rs_info.SoftwareControl);
                strcat(m_log_buf, buf);
            // 0x00D0
                sprintf(buf, "<TOU_Season1_Operation_Mode>%d</TOU_Season1_Operation_Mode>", m_hb2_rs_info.TOU_Season1_Operation_Mode);
                strcat(m_log_buf, buf);
                sprintf(buf, "<TOU_OtherSeason_Operation_Mode>%d</TOU_OtherSeason_Operation_Mode>", m_hb2_rs_info.TOU_OtherSeason_Operation_Mode);
                strcat(m_log_buf, buf);
                sprintf(buf, "<TOU_Season1_Start_Month>%d</TOU_Season1_Start_Month>", m_hb2_rs_info.TOU_Season1_Start_Month);
                strcat(m_log_buf, buf);
                sprintf(buf, "<TOU_Season1_Start_Day>%d</TOU_Season1_Start_Day>", m_hb2_rs_info.TOU_Season1_Start_Day);
                strcat(m_log_buf, buf);
                sprintf(buf, "<TOU_Season1_End_Month>%d</TOU_Season1_End_Month>", m_hb2_rs_info.TOU_Season1_End_Month);
                strcat(m_log_buf, buf);
                sprintf(buf, "<TOU_Season1_End_Day>%d</TOU_Season1_End_Day>", m_hb2_rs_info.TOU_Season1_End_Day);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_Start_Hour1_of_Season1>%d</Peak_Period_Start_Hour1_of_Season1>", m_hb2_rs_info.Peak_Period_Start_Hour1_of_Season1);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_Start_Minute1_of_Season1>%d</Peak_Period_Start_Minute1_of_Season1>", m_hb2_rs_info.Peak_Period_Start_Minute1_of_Season1);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_End_Hour1_of_Season1>%d</Peak_Period_End_Hour1_of_Season1>", m_hb2_rs_info.Peak_Period_End_Hour1_of_Season1);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_End_Minute1_of_Season1>%d</Peak_Period_End_Minute1_of_Season1>", m_hb2_rs_info.Peak_Period_End_Minute1_of_Season1);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_Start_Hour2_of_Season1>%d</Peak_Period_Start_Hour2_of_Season1>", m_hb2_rs_info.Peak_Period_Start_Hour2_of_Season1);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_Start_Minute2_of_Season1>%d</Peak_Period_Start_Minute2_of_Season1>", m_hb2_rs_info.Peak_Period_Start_Minute2_of_Season1);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_End_Hour2_of_Season1>%d</Peak_Period_End_Hour2_of_Season1>", m_hb2_rs_info.Peak_Period_End_Hour2_of_Season1);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_End_Minute2_of_Season1>%d</Peak_Period_End_Minute2_of_Season1>", m_hb2_rs_info.Peak_Period_End_Minute2_of_Season1);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_Start_Hour1_of_OtherSeason>%d</Peak_Period_Start_Hour1_of_OtherSeason>", m_hb2_rs_info.Peak_Period_Start_Hour1_of_OtherSeason);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_Start_Minute1_of_OtherSeason>%d</Peak_Period_Start_Minute1_of_OtherSeason>", m_hb2_rs_info.Peak_Period_Start_Minute1_of_OtherSeason);
                strcat(m_log_buf, buf);
            // 0x00E0
                sprintf(buf, "<Peak_Period_End_Hour1_of_OtherSeason>%d</Peak_Period_End_Hour1_of_OtherSeason>", m_hb2_rs_info.Peak_Period_End_Hour1_of_OtherSeason);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_End_Minute1_of_OtherSeason>%d</Peak_Period_End_Minute1_of_OtherSeason>", m_hb2_rs_info.Peak_Period_End_Minute1_of_OtherSeason);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_Start_Hour2_of_OtherSeason>%d</Peak_Period_Start_Hour2_of_OtherSeason>", m_hb2_rs_info.Peak_Period_Start_Hour2_of_OtherSeason);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_Start_Minute2_of_OtherSeason>%d</Peak_Period_Start_Minute2_of_OtherSeason>", m_hb2_rs_info.Peak_Period_Start_Minute2_of_OtherSeason);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_End_Hour2_of_OtherSeason>%d</Peak_Period_End_Hour2_of_OtherSeason>", m_hb2_rs_info.Peak_Period_End_Hour2_of_OtherSeason);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Peak_Period_End_Minute2_of_OtherSeason>%d</Peak_Period_End_Minute2_of_OtherSeason>", m_hb2_rs_info.Peak_Period_End_Minute2_of_OtherSeason);
                strcat(m_log_buf, buf);
            }

            // set real time info
            if ( m_save_hb2_rt_info ) {
            // 0x0100
                sprintf(buf, "<Inv_Temp>%03.1f</Inv_Temp>", ((float)m_hb2_rt_info.Inv_Temp)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PV1_Temp>%03.1f</PV1_Temp>", ((float)m_hb2_rt_info.Charger_Temp)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PV2_Temp>%03.1f</PV2_Temp>", ((float)m_hb2_rt_info.Environment_Temp)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<DD_Temp>%03.1f</DD_Temp>", ((float)m_hb2_rt_info.DD_Temp)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dcv_1>%d</dcv_1>", m_hb2_rt_info.PV1_Voltage);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dci_1>%04.2f</dci_1>", ((float)m_hb2_rt_info.PV1_Current)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_power_1>%05.3f</dc_power_1>", ((float)m_hb2_rt_info.PV1_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dcv_2>%d</dcv_2>", m_hb2_rt_info.PV2_Voltage);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dci_2>%04.2f</dci_2>", ((float)m_hb2_rt_info.PV2_Current)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_power_2>%05.3f</dc_power_2>", ((float)m_hb2_rt_info.PV2_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<acv_AN>%d</acv_AN>", m_hb2_rt_info.Load_Voltage);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_voltage>%d</ac_voltage>", m_hb2_rt_info.Load_Voltage);
                strcat(m_log_buf, buf);
                sprintf(buf, "<aci_A>%04.2f</aci_A>", ((float)m_hb2_rt_info.Load_Current)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_current>%04.2f</ac_current>", ((float)m_hb2_rt_info.Load_Current)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_power_A>%05.3f</ac_power_A>", ((float)m_hb2_rt_info.Load_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ac_power>%05.3f</ac_power>", ((float)m_hb2_rt_info.Load_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<VGrid_A>%d</VGrid_A>", m_hb2_rt_info.Grid_Voltage);
                strcat(m_log_buf, buf);
                sprintf(buf, "<IGrid_A>%04.2f</IGrid_A>", ((float)m_hb2_rt_info.Grid_Current)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PGrid_A>%05.3f</PGrid_A>", ((float)m_hb2_rt_info.Grid_Power)/1000);
                strcat(m_log_buf, buf);
            // 0x0110
                sprintf(buf, "<VBattery>%03.1f</VBattery>", ((float)m_hb2_rt_info.Battery_Voltage)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<IBattery>%03.1f</IBattery>", ((float)m_hb2_rt_info.Battery_Current)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PBattery>%05.3f</PBattery>", ((float)m_hb2_rt_info.Battery_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Vbus>%03.1f</Vbus>", ((float)m_hb2_rt_info.Bus_Voltage)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Ibus>%03.1f</Ibus>", ((float)m_hb2_rt_info.Bus_Current)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dcv_3>%d</dcv_3>", m_hb2_rt_info.PV3_Voltage);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dci_3>%04.2f</dci_3>", ((float)m_hb2_rt_info.PV3_Current)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_power_3>%05.3f</dc_power_3>", ((float)m_hb2_rt_info.PV3_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dcv_4>%d</dcv_4>", m_hb2_rt_info.PV4_Voltage);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dci_4>%04.2f</dci_4>", ((float)m_hb2_rt_info.PV4_Current)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_power_4>%05.3f</dc_power_4>", ((float)m_hb2_rt_info.PV4_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_power>%05.3f</dc_power>", ((float)m_hb2_rt_info.PV_Total_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<soc>%d</soc>", m_hb2_rt_info.Battery_SOC);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Sys_State>%d</Sys_State>", m_hb2_rt_info.Sys_State);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Module_Status>%d</Module_Status>", m_hb2_rt_info.Module_Status);
                strcat(m_log_buf, buf);
            // 0x0120
                sprintf(buf, "<Function_Status>%d</Function_Status>", m_hb2_rt_info.Function_Status);
                strcat(m_log_buf, buf);
                // 0x0121 ~ 0x0125 error code for debug
                sprintf(buf, "<PV_Inv_Error_COD1_Record>0x%04X</PV_Inv_Error_COD1_Record>", m_hb2_rt_info.PV_Inv_Error_COD1_Record);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PV_Inv_Error_COD2_Record>0x%04X</PV_Inv_Error_COD2_Record>", m_hb2_rt_info.PV_Inv_Error_COD2_Record);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PV_Inv_Error_COD3_Record>0x%04X</PV_Inv_Error_COD3_Record>", m_hb2_rt_info.PV_Inv_Error_COD3_Record);
                strcat(m_log_buf, buf);
                sprintf(buf, "<DD_Error_COD1_Record>0x%04X</DD_Error_COD1_Record>", m_hb2_rt_info.DD_Error_COD1_Record);
                strcat(m_log_buf, buf);
                sprintf(buf, "<DD_Error_COD2_Record>0x%04X</DD_Error_COD2_Record>", m_hb2_rt_info.DD_Error_COD2_Record);
                strcat(m_log_buf, buf);
                //
                sprintf(buf, "<PV_Inv_Error_COD2_ATE>%d</PV_Inv_Error_COD2_ATE>", m_hb2_rt_info.PV_Inv_Error_COD2);
                strcat(m_log_buf, buf);
                sprintf(buf, "<DD_Error_COD1_ATE>%d</DD_Error_COD1_ATE>", m_hb2_rt_info.DD_Error_COD);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Error_Code>%d</Error_Code>", m_hb2_rt_info.Error_Code);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Inverterfrequency>%03.1f</Inverterfrequency>", ((float)m_hb2_rt_info.Invert_Frequency)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<frequency>%03.1f</frequency>", ((float)m_hb2_rt_info.Grid_Frequency)/10);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Total_Load_Power>%05.3f</Total_Load_Power>", ((float)m_hb2_rt_info.Total_Load_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Total_Load_Current>%04.2f</Total_Load_Current>", ((float)m_hb2_rt_info.Total_Load_Current)/100);
                strcat(m_log_buf, buf);
                sprintf(buf, "<ExtPower>%05.3f</ExtPower>", ((float)m_hb2_rt_info.CT_Power)/1000);
                strcat(m_log_buf, buf);
                sprintf(buf, "<CT_Current>%04.2f</CT_Current>", ((float)m_hb2_rt_info.CT_Current)/100);
                strcat(m_log_buf, buf);
            // other
                sprintf(buf, "<dc_voltage>%03.1f</dc_voltage>", ((float)(m_hb2_rt_info.PV1_Voltage + m_hb2_rt_info.PV2_Voltage + m_hb2_rt_info.PV3_Voltage + m_hb2_rt_info.PV4_Voltage))/4);
                strcat(m_log_buf, buf);
                sprintf(buf, "<dc_current>%d</dc_current>", m_hb2_rt_info.PV1_Current + m_hb2_rt_info.PV2_Current + m_hb2_rt_info.PV3_Current + m_hb2_rt_info.PV4_Current);
                strcat(m_log_buf, buf);
            }

            // set cumulative energy value
            if ( m_save_hb2_ce_value ) {
            // 0x0160
                sprintf(buf, "<Total_Life_Time>%04.2f</Total_Life_Time>", m_hb2_ce_value.Total_Life_TimeH*100 + ((float)m_hb2_ce_value.Total_Life_TimeL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<total_KWH>%04.2f</total_KWH>", m_hb2_ce_value.PV_Total_EnergyH*100 + ((float)m_hb2_ce_value.PV_Total_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Pbat_Total>%04.2f</Pbat_Total>", m_hb2_ce_value.Bat_Charge_Total_EnergyH*100 + ((float)m_hb2_ce_value.Bat_Charge_Total_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Bat_Discharge_Total_Energy>%04.2f</Bat_Discharge_Total_Energy>", m_hb2_ce_value.Bat_Discharge_Total_EnergyH*100 + ((float)m_hb2_ce_value.Bat_Discharge_Total_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Pload_Total>%04.2f</Pload_Total>", m_hb2_ce_value.Load_Total_EnergyH*100 + ((float)m_hb2_ce_value.Load_Total_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Negative_Load_Total_Energy>%04.2f</Negative_Load_Total_Energy>", m_hb2_ce_value.Negative_Load_Total_EnergyH*100 + ((float)m_hb2_ce_value.Negative_Load_Total_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<GridFeed_Total>%04.2f</GridFeed_Total>", m_hb2_ce_value.GridFeed_TotalH*100 + ((float)m_hb2_ce_value.GridFeed_TotalL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<GridCharge_Total>%04.2f</GridCharge_Total>", m_hb2_ce_value.GridCharge_TotalH*100 + ((float)m_hb2_ce_value.GridCharge_TotalL)*0.01);
                strcat(m_log_buf, buf);
            // 0x0170
                sprintf(buf, "<PV_Today_Energy>%04.2f</PV_Today_Energy>", m_hb2_ce_value.PV_Today_EnergyH*100 + ((float)m_hb2_ce_value.PV_Today_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Bat_Charge_Today_Energy>%04.2f</Bat_Charge_Today_Energy>", m_hb2_ce_value.Bat_Charge_Today_EnergyH*100 + ((float)m_hb2_ce_value.Bat_Charge_Today_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Bat_Discharge_Today_Energy>%04.2f</Bat_Discharge_Today_Energy>", m_hb2_ce_value.Bat_Discharge_Today_EnergyH*100 + ((float)m_hb2_ce_value.Bat_Discharge_Today_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Load_Today_Energy>%04.2f</Load_Today_Energy>", m_hb2_ce_value.Load_Today_EnergyH*100 + ((float)m_hb2_ce_value.Load_Today_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Negative_Load_Today_Energy>%04.2f</Negative_Load_Today_Energy>", m_hb2_ce_value.Negative_Load_Today_EnergyH*100 + ((float)m_hb2_ce_value.Negative_Load_Today_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<GridFeed_Today_Energy>%04.2f</GridFeed_Today_Energy>", m_hb2_ce_value.GridFeed_TodayH*100 + ((float)m_hb2_ce_value.GridFeed_TodayL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<GridCharge_Today_Energy>%04.2f</GridCharge_Today_Energy>", m_hb2_ce_value.GridCharge_TodayH*100 + ((float)m_hb2_ce_value.GridCharge_TodayL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<PV_Month_Energy>%04.2f</PV_Month_Energy>", m_hb2_ce_value.PV_Month_EnergyH*100 + ((float)m_hb2_ce_value.PV_Month_EnergyL)*0.01);
                strcat(m_log_buf, buf);
            // 0x0180
                sprintf(buf, "<Bat_Charge_Month_Energy>%04.2f</Bat_Charge_Month_Energy>", m_hb2_ce_value.Bat_Charge_Month_EnergyH*100 + ((float)m_hb2_ce_value.Bat_Charge_Month_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Bat_Discharge_Month_Energy>%04.2f</Bat_Discharge_Month_Energy>", m_hb2_ce_value.Bat_Discharge_Month_EnergyH*100 + ((float)m_hb2_ce_value.Bat_Discharge_Month_EnergyH)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Load_Month_Energy>%04.2f</Load_Month_Energy>", m_hb2_ce_value.Load_Month_EnergyH*100 + ((float)m_hb2_ce_value.Load_Month_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Negative_Load_Month_Energy>%04.2f</Negative_Load_Month_Energy>", m_hb2_ce_value.Negative_Load_Month_EnergyH*100 + ((float)m_hb2_ce_value.Negative_Load_Month_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<GridFeed_Month_Energy>%04.2f</GridFeed_Month_Energy>", m_hb2_ce_value.GridFeed_MonthH*100 + ((float)m_hb2_ce_value.GridFeed_MonthL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<GridCharge_Month_Energy>%04.2f</GridCharge_Month_Energy>", m_hb2_ce_value.GridCharge_MonthH*100 + ((float)m_hb2_ce_value.GridCharge_MonthL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<CT_Total_Feedin_Energy>%04.2f</CT_Total_Feedin_Energy>", m_hb2_ce_value.CT_Total_Feedin_EnergyH*100 + ((float)m_hb2_ce_value.CT_Total_Feedin_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<CT_Today_Feedin_Energy>%04.2f</CT_Today_Feedin_Energy>", m_hb2_ce_value.CT_Today_Feedin_EnergyH*100 + ((float)m_hb2_ce_value.CT_Today_Feedin_EnergyL)*0.01);
                strcat(m_log_buf, buf);
            // 0x0190
                sprintf(buf, "<CT_Total_Charge_Energy>%04.2f</CT_Total_Charge_Energy>", m_hb2_ce_value.CT_Total_Charge_EnergyH*100 + ((float)m_hb2_ce_value.CT_Total_Charge_EnergyL)*0.01);
                strcat(m_log_buf, buf);
                sprintf(buf, "<CT_Today_Charge_Energy>%04.2f</CT_Today_Charge_Energy>", m_hb2_ce_value.CT_Today_Charge_EnergyH*100 + ((float)m_hb2_ce_value.CT_Today_Charge_EnergyL)*0.01);
                strcat(m_log_buf, buf);
            }

            // set display info
            if ( m_save_hb2_dp_info ) {
            // 0x01A0
                sprintf(buf, "<Display_Working_State>%d</Display_Working_State>", m_hb2_dp_info.Display_Working_State);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Output_Power_Restraint_Reeson>%d</Output_Power_Restraint_Reeson>", m_hb2_dp_info.Output_Power_Restraint_Reeson);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Battery_To_Load_Consumption_Time>%d</Battery_To_Load_Consumption_Time>", m_hb2_dp_info.Battery_To_Load_Consumption_Time);
                strcat(m_log_buf, buf);
                sprintf(buf, "<OnGrid_CountDown>%d</OnGrid_CountDown>", m_hb2_dp_info.OnGrid_CountDown);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Hybrid_Icon>%d</Hybrid_Icon>", (m_hb2_dp_info.Hybrid_IconH << 16) + m_hb2_dp_info.Hybrid_IconL);
                strcat(m_log_buf, buf);
            }

            // set RTC part
            if ( m_save_hb2_rtc_info ) {
                sprintf(buf, "<Second>%d</Second>", m_hb2_rtc_data.Second);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Minute>%d</Minute>", m_hb2_rtc_data.Minute);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Hour>%d</Hour>", m_hb2_rtc_data.Hour);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Day>%d</Day>", m_hb2_rtc_data.Date);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Month>%d</Month>", m_hb2_rtc_data.Month);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Year>%d</Year>", m_hb2_rtc_data.Year);
                strcat(m_log_buf, buf);
            }

            // set bms info
            if ( m_save_hb2_bms_info ) {
            // 0x0200
                sprintf(buf, "<BMS_Voltage>%04.2f</BMS_Voltage>", ((float)m_hb2_bms_info.Charge_Voltage_Requirement/100));
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_Current>%04.2f</BMS_Current>", ((float)m_hb2_bms_info.Charge_Current_Requirement/100));
                strcat(m_log_buf, buf);
                sprintf(buf, "<Number_Of_Module_Warning>%d</Number_Of_Module_Warning>", m_hb2_bms_info.Number_Of_Module_Warning);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Master_Total_Voltage>%04.2f</Master_Total_Voltage>", ((float)m_hb2_bms_info.Master_Total_Voltage/100));
                strcat(m_log_buf, buf);
                sprintf(buf, "<Discharging_Charging_Current>%04.2f</Discharging_Charging_Current>", ((float)m_hb2_bms_info.Discharging_Charging_Current/100));
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_SOC>%03.1f</BMS_SOC>", ((float)m_hb2_bms_info.SOC/10));
                strcat(m_log_buf, buf);
                sprintf(buf, "<FCC>%04.2f</FCC>", ((float)m_hb2_bms_info.FCC/100));
                strcat(m_log_buf, buf);
                sprintf(buf, "<RC>%04.2f</RC>", ((float)m_hb2_bms_info.RC/100));
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_Status>%d</BMS_Status>", m_hb2_bms_info.Status_Flag_Register);
                strcat(m_log_buf, buf);
                sprintf(buf, "<IO_Flag_Register>%d</IO_Flag_Register>", m_hb2_bms_info.IO_Flag_Register);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Warning_Flag_Register>%d</Warning_Flag_Register>", m_hb2_bms_info.Warning_Flag_Register);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Alarm_Flag_Register>%d</Alarm_Flag_Register>", m_hb2_bms_info.Alarm_Flag_Register);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_MaxCell>%05.3f</BMS_MaxCell>", ((float)m_hb2_bms_info.Max_Cell_Voltage/1000));
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_MinCell>%05.3f</BMS_MinCell>", ((float)m_hb2_bms_info.Min_Cell_Voltage/1000));
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_Max_Temp>%d</BMS_Max_Temp>", m_hb2_bms_info.Max_Cell_Temperature);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_Min_Temp>%d</BMS_Min_Temp>", m_hb2_bms_info.Min_Cell_Temperature);
                strcat(m_log_buf, buf);
            // 0x0210
                sprintf(buf, "<BMS_CycleCount>%d</BMS_CycleCount>", m_hb2_bms_info.Master_Average_Cycle_Count);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Master_Average_SOH>%03.1f</Master_Average_SOH>", ((float)m_hb2_bms_info.Master_Average_SOH/10));
                strcat(m_log_buf, buf);
                sprintf(buf, "<Battery_Cumulative_Input_Capacity>%d</Battery_Cumulative_Input_Capacity>", m_hb2_bms_info.Battery_Cumulative_Input_Capacity);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Battery_Cumulative_Output_Capacity>%d</Battery_Cumulative_Output_Capacity>", m_hb2_bms_info.Battery_Cumulative_Output_Capacity);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Master_FW_Version>%d</Master_FW_Version>", m_hb2_bms_info.Master_FW_Version);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Master_Manufacture_Data>%d</Master_Manufacture_Data>", m_hb2_bms_info.Master_Manufacture_Data);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Master_Serial_Number>%d</Master_Serial_Number>", m_hb2_bms_info.Master_Serial_Number);
                strcat(m_log_buf, buf);
                sprintf(buf, "<Number_Of_Whole_Cells>%d</Number_Of_Whole_Cells>", m_hb2_bms_info.Number_Of_Whole_Cells);
                strcat(m_log_buf, buf);
                sprintf(buf, "<BMS_ModuleNo>%d</BMS_ModuleNo>", m_hb2_bms_info.Number_Of_Module);
                strcat(m_log_buf, buf);

                //sprintf(buf, "<BMS_Error>0</BMS_Error>");
                //strcat(m_log_buf, buf);
                //sprintf(buf, "<BMS_Info>0</BMS_Info>");
                //strcat(m_log_buf, buf);
                //sprintf(buf, "<BMS_BaudRate>0</BMS_BaudRate>");
                //strcat(m_log_buf, buf);
            }
    }

    strcat(m_log_buf, "</record>");
    printf("===================== Set Log XML end =====================\n");
    return true;
}

bool CG320::SaveLogXML(bool first, bool last)
{
    FILE *fd = NULL;
    struct stat filest;
    char buf[256] = {0};
    char tmp[256] = {0};
    int filesize = 0, offset = 0, ret = 0;

    if ( first ) {
        fd = fopen("/tmp/tmplog", "wb");
        if ( fd != NULL ) {
            fwrite("<records>", 1, 9, fd);
            filesize = 9;
        }
    } else {
        stat("/tmp/tmplog", &filest);
        filesize = filest.st_size;
        fd = fopen("/tmp/tmplog", "ab");
    }

    if ( last ) {
        strcat(m_log_buf, "</records>");
        while ( (strlen(m_log_buf) + filesize) % 3 != 0 ) {
            m_log_buf[strlen(m_log_buf)] = 0x20; // add space to end
        }
    }

    if ( fd != NULL ) {
        fwrite(m_log_buf, 1, strlen(m_log_buf), fd);
        filesize += strlen(m_log_buf);
        //printf("Log filesize = %d\n", filesize);
        fclose(fd);
    } else {
        SaveLog((char *)"DataLogger SaveLogXML() : open /tmp/tmplog Fail", m_st_time);
        printf("open /tmp/tmplog Fail!\n");
        return false;
    }

    if ( !last )
        return true;
    else {
        if ( filesize < 42 ) {
            printf("==== SaveLogXML file size = %d, too small, don't copy file to storage ====\n", filesize);
            return true;
        }
    }

    // copy file to target
    sprintf(buf, "cp /tmp/tmplog %s", m_log_filename);
    ret = system(buf);
    if ( ret == 0 ) {
        sprintf(buf, "DataLogger SaveLogXML() : write %s OK", m_log_filename);
        SaveLog(buf, m_st_time);
        if ( strstr(m_log_filename, USB_PATH) )
            m_sys_error  &= ~SYS_0002_Save_USB_Fail;
        return true;
    } else {
        sprintf(buf, "DataLogger SaveLogXML() : write %s Fail", m_log_filename);
        SaveLog(buf, m_st_time);
        if ( strstr(m_log_filename, USB_PATH) )
            m_sys_error |= SYS_0002_Save_USB_Fail;
    }

    // if copy file to storage fail, then copy to tmp
    if ( strstr(m_log_filename, DEF_PATH) == NULL ) {
        strcpy(tmp, DEF_PATH);
        if ( strstr(m_log_filename, USB_PATH) != NULL )
            offset = strlen(USB_PATH);
        // save to tmp
        strcat(tmp, m_log_filename+offset);
        sprintf(buf, "cp /tmp/tmplog %s", tmp);
        ret = system(buf);
        if ( ret == 0 ) {
            SaveLog((char *)"DataLogger SaveLogXML() : write to tmp OK", m_st_time);
            return true;
        } else {
            SaveLog((char *)"DataLogger SaveLogXML() : write to tmp Fail", m_st_time);
            return false;
        }
    }

    return false;
}

void CG320::SetErrorLogXML()
{
    sprintf(m_errlog_filename, "%s/%02d%02d%02d", m_dl_path.m_errlog_path, m_data_st_time.tm_hour, m_data_st_time.tm_min, m_data_st_time.tm_sec);
    //printf("errlog path = %s\n", m_errlog_filename);
    return;
}

bool CG320::WriteErrorLogXML(int index)
{
    char buf[256] = {0};
    char idtmp[13] = {0};
    unsigned long long int dev_id = 0;
    int model = 0;

    //time_t current_time;
    //struct tm *log_time;
    //current_time = time(NULL);
    //log_time = localtime(&current_time);

    //SaveLog((char *)"DataLogger WriteErrorLogXML() : run", log_time);
    printf("==================== Set Error Log XML start ====================\n");
    //if ( strlen(m_errlog_buf) == 0 ) // empty, new file, add header <records>
    //    strcpy(m_errlog_buf, "<records>");

    sscanf(arySNobj[index].m_Sn+4, "%04X", &model); // get 5-8 digit from SN
    //printf("Index %d model = %X\n", index, model);

    if ( arySNobj[index].m_Device < 0x0A ) {
        // MI part
        if ( model < 3 ) {
            // G240/300, G320, G321 single channel
            sscanf(arySNobj[index].m_Sn+4, "%012llX", &dev_id); // get last 12 digit
            sprintf(buf, "<record dev_id=\"%lld\" date=\"%04d-%02d-%02d %02d:%02d:%02d\" sn=\"%s\">", dev_id,
                1900+m_data_st_time.tm_year, 1+m_data_st_time.tm_mon, m_data_st_time.tm_mday,
                m_data_st_time.tm_hour, m_data_st_time.tm_min, m_data_st_time.tm_sec, arySNobj[index].m_Sn);
            strcat(m_errlog_buf, buf);

            // Error_Code1
            if ( m_mi_power_info.Error_Code1 & 0x0001 )
                strcat(m_errlog_buf, "<code>COD1_0001</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0002 )
                strcat(m_errlog_buf, "<code>COD1_0002</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0004 )
                strcat(m_errlog_buf, "<code>COD1_0004</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0008 )
                strcat(m_errlog_buf, "<code>COD1_0008</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0010 )
                strcat(m_errlog_buf, "<code>COD1_0010</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0020 )
                strcat(m_errlog_buf, "<code>COD1_0020</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0040 )
                strcat(m_errlog_buf, "<code>COD1_0040</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0080 )
                strcat(m_errlog_buf, "<code>COD1_0080</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0100 )
                strcat(m_errlog_buf, "<code>COD1_0100</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0200 )
                strcat(m_errlog_buf, "<code>COD1_0200</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0400 )
                strcat(m_errlog_buf, "<code>COD1_0400</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0800 )
                strcat(m_errlog_buf, "<code>COD1_0800</code>");
            if ( m_mi_power_info.Error_Code1 & 0x1000 )
                strcat(m_errlog_buf, "<code>COD1_1000</code>");
            if ( m_mi_power_info.Error_Code1 & 0x2000 )
                strcat(m_errlog_buf, "<code>COD1_2000</code>");
            if ( m_mi_power_info.Error_Code1 & 0x4000 )
                strcat(m_errlog_buf, "<code>COD1_4000</code>");
            if ( m_mi_power_info.Error_Code1 & 0x8000 )
                strcat(m_errlog_buf, "<code>COD1_8000</code>");
            // Error_Code2
            if ( m_mi_power_info.Error_Code2 & 0x0001 )
                strcat(m_errlog_buf, "<code>COD2_0001</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0002 )
                strcat(m_errlog_buf, "<code>COD2_0002</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0004 )
                strcat(m_errlog_buf, "<code>COD2_0004</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0008 )
                strcat(m_errlog_buf, "<code>COD2_0001</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0010 )
                strcat(m_errlog_buf, "<code>COD2_0010</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0020 )
                strcat(m_errlog_buf, "<code>COD2_0020</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0040 )
                strcat(m_errlog_buf, "<code>COD2_0040</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0080 )
                strcat(m_errlog_buf, "<code>COD2_0080</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0100 )
                strcat(m_errlog_buf, "<code>COD2_0100</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0200 )
                strcat(m_errlog_buf, "<code>COD2_0200</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0400 )
                strcat(m_errlog_buf, "<code>COD2_0400</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0800 )
                strcat(m_errlog_buf, "<code>COD2_0800</code>");
            if ( m_mi_power_info.Error_Code2 & 0x1000 )
                strcat(m_errlog_buf, "<code>COD2_1000</code>");
            if ( m_mi_power_info.Error_Code2 & 0x2000 )
                strcat(m_errlog_buf, "<code>COD2_2000</code>");
            if ( m_mi_power_info.Error_Code2 & 0x4000 )
                strcat(m_errlog_buf, "<code>COD2_4000</code>");
            if ( m_mi_power_info.Error_Code2 & 0x8000 )
                strcat(m_errlog_buf, "<code>COD2_8000</code>");
        } else {
            // G640, G642, G640R dual channel
            // set ch1
            idtmp[0] = 'A';
            strcpy(idtmp+1, arySNobj[index].m_Sn+5);
            sscanf(idtmp, "%012llX", &dev_id); // get last 12 digit
            sprintf(buf, "<record dev_id=\"%lld\" date=\"%04d-%02d-%02d %02d:%02d:%02d\" sn=\"%s\">", dev_id,
                    1900+m_data_st_time.tm_year, 1+m_data_st_time.tm_mon, m_data_st_time.tm_mday,
                    m_data_st_time.tm_hour, m_data_st_time.tm_min, m_data_st_time.tm_sec, arySNobj[index].m_Sn);
            strcat(m_errlog_buf, buf);

            // Error_Code1
            if ( m_mi_power_info.Error_Code1 & 0x0001 )
                strcat(m_errlog_buf, "<code>COD1_0001</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0002 )
                strcat(m_errlog_buf, "<code>COD1_0002</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0004 )
                strcat(m_errlog_buf, "<code>COD1_0004</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0008 )
                strcat(m_errlog_buf, "<code>COD1_0008</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0010 )
                strcat(m_errlog_buf, "<code>COD1_0010</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0020 )
                strcat(m_errlog_buf, "<code>COD1_0020</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0040 )
                strcat(m_errlog_buf, "<code>COD1_0040</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0080 )
                strcat(m_errlog_buf, "<code>COD1_0080</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0100 )
                strcat(m_errlog_buf, "<code>COD1_0100</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0200 )
                strcat(m_errlog_buf, "<code>COD1_0200</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0400 )
                strcat(m_errlog_buf, "<code>COD1_0400</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0800 )
                strcat(m_errlog_buf, "<code>COD1_0800</code>");
            if ( m_mi_power_info.Error_Code1 & 0x1000 )
                strcat(m_errlog_buf, "<code>COD1_1000</code>");
            //if ( m_mi_power_info.Error_Code1 & 0x2000 ) // ch2
            //    strcat(m_errlog_buf, "<code>COD1_2000</code>");
            //if ( m_mi_power_info.Error_Code1 & 0x4000 ) // ch2
            //    strcat(m_errlog_buf, "<code>COD1_4000</code>");
            if ( m_mi_power_info.Error_Code1 & 0x8000 )
                strcat(m_errlog_buf, "<code>COD1_8000</code>");
            // Error_Code2
            if ( m_mi_power_info.Error_Code2 & 0x0001 )
                strcat(m_errlog_buf, "<code>COD2_0001</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0002 )
                strcat(m_errlog_buf, "<code>COD2_0002</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0004 )
                strcat(m_errlog_buf, "<code>COD2_0004</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0008 )
                strcat(m_errlog_buf, "<code>COD2_0001</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0010 )
                strcat(m_errlog_buf, "<code>COD2_0010</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0020 )
                strcat(m_errlog_buf, "<code>COD2_0020</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0040 )
                strcat(m_errlog_buf, "<code>COD2_0040</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0080 )
                strcat(m_errlog_buf, "<code>COD2_0080</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0100 )
                strcat(m_errlog_buf, "<code>COD2_0100</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0200 )
                strcat(m_errlog_buf, "<code>COD2_0200</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0400 )
                strcat(m_errlog_buf, "<code>COD2_0400</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0800 )
                strcat(m_errlog_buf, "<code>COD2_0800</code>");
            if ( m_mi_power_info.Error_Code2 & 0x1000 )
                strcat(m_errlog_buf, "<code>COD2_1000</code>");
            if ( m_mi_power_info.Error_Code2 & 0x2000 )
                strcat(m_errlog_buf, "<code>COD2_2000</code>");
            if ( m_mi_power_info.Error_Code2 & 0x4000 )
                strcat(m_errlog_buf, "<code>COD2_4000</code>");
            //if ( m_mi_power_info.Error_Code2 & 0x8000 ) // ch2
            //    strcat(m_errlog_buf, "<code>COD2_8000</code>");

            // set system error log
            if ( m_sys_error && (m_data_st_time.tm_hour%2 == 0) && (m_data_st_time.tm_min == 0) ) {
                if ( m_sys_error & SYS_0001_No_USB )
                    strcat(m_errlog_buf, "<code>SYS_0001_No_USB</code>");
                if ( m_sys_error & SYS_0002_Save_USB_Fail )
                    strcat(m_errlog_buf, "<code>SYS_0002_Save_USB_Fail</code>");
                if ( m_sys_error & SYS_0004_No_SD )
                    strcat(m_errlog_buf, "<code>SYS_0004_No_SD</code>");
                if ( m_sys_error & SYS_0008_Save_SD_Fail )
                    strcat(m_errlog_buf, "<code>SYS_0008_Save_SD_Fail</code>");
            }

            strcat(m_errlog_buf, "</record>");

            // set ch2
            idtmp[0] = 'B';
            //strcpy(idtmp+1, arySNobj[index].m_Sn+5);
            sscanf(idtmp, "%012llX", &dev_id); // get last 12 digit
            sprintf(buf, "<record dev_id=\"%lld\" date=\"%04d-%02d-%02d %02d:%02d:%02d\" sn=\"%s\">", dev_id,
                    1900+m_data_st_time.tm_year, 1+m_data_st_time.tm_mon, m_data_st_time.tm_mday,
                    m_data_st_time.tm_hour, m_data_st_time.tm_min, m_data_st_time.tm_sec, arySNobj[index].m_Sn);
            strcat(m_errlog_buf, buf);

            // Error_Code1
            if ( m_mi_power_info.Error_Code1 & 0x0001 )
                strcat(m_errlog_buf, "<code>COD1_0001</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0002 )
                strcat(m_errlog_buf, "<code>COD1_0002</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0004 )
                strcat(m_errlog_buf, "<code>COD1_0004</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0008 )
                strcat(m_errlog_buf, "<code>COD1_0008</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0010 )
                strcat(m_errlog_buf, "<code>COD1_0010</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0020 )
                strcat(m_errlog_buf, "<code>COD1_0020</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0040 )
                strcat(m_errlog_buf, "<code>COD1_0040</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0080 )
                strcat(m_errlog_buf, "<code>COD1_0080</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0100 )
                strcat(m_errlog_buf, "<code>COD1_0100</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0200 )
                strcat(m_errlog_buf, "<code>COD1_0200</code>");
            if ( m_mi_power_info.Error_Code1 & 0x0400 )
                strcat(m_errlog_buf, "<code>COD1_0400</code>");
            //if ( m_mi_power_info.Error_Code1 & 0x0800 ) // ch1
            //    strcat(m_errlog_buf, "<code>COD1_0800</code>");
            //if ( m_mi_power_info.Error_Code1 & 0x1000 ) // ch1
            //    strcat(m_errlog_buf, "<code>COD1_1000</code>");
            if ( m_mi_power_info.Error_Code1 & 0x2000 )
                strcat(m_errlog_buf, "<code>COD1_2000</code>");
            if ( m_mi_power_info.Error_Code1 & 0x4000 )
                strcat(m_errlog_buf, "<code>COD1_4000</code>");
            if ( m_mi_power_info.Error_Code1 & 0x8000 )
                strcat(m_errlog_buf, "<code>COD1_8000</code>");
            // Error_Code2
            if ( m_mi_power_info.Error_Code2 & 0x0001 )
                strcat(m_errlog_buf, "<code>COD2_0001</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0002 )
                strcat(m_errlog_buf, "<code>COD2_0002</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0004 )
                strcat(m_errlog_buf, "<code>COD2_0004</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0008 )
                strcat(m_errlog_buf, "<code>COD2_0001</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0010 )
                strcat(m_errlog_buf, "<code>COD2_0010</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0020 )
                strcat(m_errlog_buf, "<code>COD2_0020</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0040 )
                strcat(m_errlog_buf, "<code>COD2_0040</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0080 )
                strcat(m_errlog_buf, "<code>COD2_0080</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0100 )
                strcat(m_errlog_buf, "<code>COD2_0100</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0200 )
                strcat(m_errlog_buf, "<code>COD2_0200</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0400 )
                strcat(m_errlog_buf, "<code>COD2_0400</code>");
            if ( m_mi_power_info.Error_Code2 & 0x0800 )
                strcat(m_errlog_buf, "<code>COD2_0800</code>");
            if ( m_mi_power_info.Error_Code2 & 0x1000 )
                strcat(m_errlog_buf, "<code>COD2_1000</code>");
            if ( m_mi_power_info.Error_Code2 & 0x2000 )
                strcat(m_errlog_buf, "<code>COD2_2000</code>");
            //if ( m_mi_power_info.Error_Code2 & 0x4000 ) // ch1
            //    strcat(m_errlog_buf, "<code>COD2_4000</code>");
            if ( m_mi_power_info.Error_Code2 & 0x8000 )
                strcat(m_errlog_buf, "<code>COD2_8000</code>");
        }
    } else {
        // Hybrid part
        sscanf(arySNobj[index].m_Sn+4, "%012llX", &dev_id); // get last 12 digit
        sprintf(buf, "<record dev_id=\"%lld\" date=\"%04d-%02d-%02d %02d:%02d:%02d\" sn=\"%s\">", dev_id,
            1900+m_data_st_time.tm_year, 1+m_data_st_time.tm_mon, m_data_st_time.tm_mday,
            m_data_st_time.tm_hour, m_data_st_time.tm_min, m_data_st_time.tm_sec, arySNobj[index].m_Sn);
        strcat(m_errlog_buf, buf);

        // H5000/5001
        // 0xDB : error code
        if ( m_hb_rt_info.Error_Code ) {
            sprintf(buf, "<code>%d</code>", m_hb_rt_info.Error_Code);
            strcat(m_errlog_buf, buf);
        }
        // PV_Inv_Error_COD1_Record 0xD3
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x0001 )
            strcat(m_errlog_buf, "<code>COD1_0001_Fac_HL</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x0002 )
            strcat(m_errlog_buf, "<code>COD1_0002_CanBus_Fault</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x0004 )
            strcat(m_errlog_buf, "<code>COD1_0004_Islanding</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x0008 )
            strcat(m_errlog_buf, "<code>COD1_0008_Vac_H</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x0010 )
            strcat(m_errlog_buf, "<code>COD1_0010_Vac_L</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x0020 )
            strcat(m_errlog_buf, "<code>COD1_0020_Fac_H</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x0040 )
            strcat(m_errlog_buf, "<code>COD1_0040_Fac_L</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x0080 )
            strcat(m_errlog_buf, "<code>COD1_0080_Fac_LL</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x0100 )
            strcat(m_errlog_buf, "<code>COD1_0100_Vac_OCP</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x0200 )
            strcat(m_errlog_buf, "<code>COD1_0200_Vac_HL</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x0400 )
            strcat(m_errlog_buf, "<code>COD1_0400_Vac_LL</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x0800 )
            strcat(m_errlog_buf, "<code>COD1_0800_OPP</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x1000 )
            strcat(m_errlog_buf, "<code>COD1_1000_Iac_H</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x2000 )
            strcat(m_errlog_buf, "<code>COD1_2000_Ipv_H</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x4000 )
            strcat(m_errlog_buf, "<code>COD1_4000_ADCINT_OVF</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD1_Record & 0x8000 )
            strcat(m_errlog_buf, "<code>COD1_8000_Vbus_H</code>");
        // PV_Inv_Error_COD2_Record 0xD4
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x0001 )
            strcat(m_errlog_buf, "<code>COD2_0001_Arc</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x0002 )
            strcat(m_errlog_buf, "<code>COD2_0002_Vac_Relay_Fault</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x0004 )
            strcat(m_errlog_buf, "<code>COD2_0004_Ipv1_short</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x0008 )
            strcat(m_errlog_buf, "<code>COD2_0008_Ipv2_short</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x0010 )
            strcat(m_errlog_buf, "<code>COD2_0010_Vac_Short</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x0020 )
            strcat(m_errlog_buf, "<code>COD2_0020_CT_fault</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x0040 )
            strcat(m_errlog_buf, "<code>COD2_0040_PVOverPower</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x0080 )
            strcat(m_errlog_buf, "<code>COD2_0080_NO_GRID</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x0100 )
            strcat(m_errlog_buf, "<code>COD2_0100_PV_Input_High</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x0200 )
            strcat(m_errlog_buf, "<code>COD2_0200_INV_Overload</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x0400 )
            strcat(m_errlog_buf, "<code>COD2_0400_RCMU_30</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x0800 )
            strcat(m_errlog_buf, "<code>COD2_0800_RCMU_60</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x1000 )
            strcat(m_errlog_buf, "<code>COD2_1000_RCMU_150</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x2000 )
            strcat(m_errlog_buf, "<code>COD2_2000_RCMU_300</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x4000 )
            strcat(m_errlog_buf, "<code>COD2_4000_RCMUtest_Fault</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD2_Record & 0x8000 )
            strcat(m_errlog_buf, "<code>COD2_8000_Vac_LM</code>");
        // DD_Error_COD_Record 0xD5
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x0001 )
            strcat(m_errlog_buf, "<code>COD3_0001_Vbat_H</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x0002 )
            strcat(m_errlog_buf, "<code>COD3_0002_Vbat_L_fault</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x0004 )
            strcat(m_errlog_buf, "<code>COD3_0004_Vbus_H</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x0008 )
            strcat(m_errlog_buf, "<code>COD3_0008_Vbus_L</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x0010 )
            strcat(m_errlog_buf, "<code>COD3_0010_Ibus_H</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x0020 )
            strcat(m_errlog_buf, "<code>COD3_0020_Ibat_H</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x0040 )
            strcat(m_errlog_buf, "<code>COD3_0040_Charger_T</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x0080 )
            strcat(m_errlog_buf, "<code>COD3_0080_Code</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x0100 )
            strcat(m_errlog_buf, "<code>COD3_0100_Vbat_Drop</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x0200 )
            strcat(m_errlog_buf, "<code>COD3_0200_INV_fault</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x0400 )
            strcat(m_errlog_buf, "<code>COD3_0400_GND_Fault</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x0800 )
            strcat(m_errlog_buf, "<code>COD3_0800_No_Bat</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x1000 )
            strcat(m_errlog_buf, "<code>COD3_1000_BMS_Comute_fault</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x2000 )
            strcat(m_errlog_buf, "<code>COD3_2000_BMS_Over_Current</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x4000 )
            strcat(m_errlog_buf, "<code>COD3_4000_Restart</code>");
        if ( m_hb_rt_info.DD_Error_COD_Record & 0x8000 )
            strcat(m_errlog_buf, "<code>COD3_8000_Bat_Setting_fault</code>");
        // PV_Inv_Error_COD3_Record 0xF0
        if ( m_hb_rt_info.PV_Inv_Error_COD3_Record & 0x0001 )
            strcat(m_errlog_buf, "<code>COD4_0001_External_PV_OPP</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD3_Record & 0x0002 )
            strcat(m_errlog_buf, "<code>COD4_0002_Model123_Reconnected_Delay</code>");
        if ( m_hb_rt_info.PV_Inv_Error_COD3_Record & 0x0004 )
            strcat(m_errlog_buf, "<code>COD4_0004_Peak_Shaving_Over_Power</code>");
        // DD_Error_COD2_Record 0xF1
        if ( m_hb_rt_info.DD_Error_COD2_Record & 0x0001 )
            strcat(m_errlog_buf, "<code>COD5_0001_EEProm_Fault</code>");
        if ( m_hb_rt_info.DD_Error_COD2_Record & 0x0002 )
            strcat(m_errlog_buf, "<code>COD5_0002_Communi_Fault</code>");
        if ( m_hb_rt_info.DD_Error_COD2_Record & 0x0004 )
            strcat(m_errlog_buf, "<code>COD5_0004_OT_Fault</code>");
        if ( m_hb_rt_info.DD_Error_COD2_Record & 0x0008 )
            strcat(m_errlog_buf, "<code>COD5_0008_Fan_Fault</code>");
        if ( m_hb_rt_info.DD_Error_COD2_Record & 0x0010 )
            strcat(m_errlog_buf, "<code>COD5_0010_Low_Battery</code>");
        if ( m_hb_rt_info.DD_Error_COD2_Record & 0x0020 )
            strcat(m_errlog_buf, "<code>COD5_0020_Relay_state</code>");
        if ( m_hb_rt_info.DD_Error_COD2_Record & 0x0040 )
            strcat(m_errlog_buf, "<code>COD5_0040_Off_Grid_Operation</code>");
        if ( m_hb_rt_info.DD_Error_COD2_Record & 0x0080 )
            strcat(m_errlog_buf, "<code>COD5_0080_InvEnable_flag</code>");
        if ( m_hb_rt_info.DD_Error_COD2_Record & 0x0100 )
            strcat(m_errlog_buf, "<code>COD5_0100_Bypass_flag</code>");
        if ( m_hb_rt_info.DD_Error_COD2_Record & 0x0200 )
            strcat(m_errlog_buf, "<code>COD5_0200_DD_en</code>");
        if ( m_hb_rt_info.DD_Error_COD2_Record & 0x0400 )
            strcat(m_errlog_buf, "<code>COD5_0400_PVEnable_flag</code>");


        // H5500/9600
        // 0x0128 : error code
        if ( m_hb2_rt_info.Error_Code ) {
            sprintf(buf, "<code>%d</code>", m_hb2_rt_info.Error_Code);
            strcat(m_errlog_buf, buf);
        }
        // PV_Inv_Error_COD1_Record 0x0121
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x0001 )
            strcat(m_errlog_buf, "<code>COD1_0001_Fac_HL</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x0002 )
            strcat(m_errlog_buf, "<code>COD1_0002_CanBus_Fault</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x0004 )
            strcat(m_errlog_buf, "<code>COD1_0004_Islanding</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x0008 )
            strcat(m_errlog_buf, "<code>COD1_0008_Vac_H</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x0010 )
            strcat(m_errlog_buf, "<code>COD1_0010_Vac_L</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x0020 )
            strcat(m_errlog_buf, "<code>COD1_0020_Fac_H</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x0040 )
            strcat(m_errlog_buf, "<code>COD1_0040_Fac_L</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x0080 )
            strcat(m_errlog_buf, "<code>COD1_0080_Fac_LL</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x0100 )
            strcat(m_errlog_buf, "<code>COD1_0100_Vac_OCP</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x0200 )
            strcat(m_errlog_buf, "<code>COD1_0200_Vac_HL</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x0400 )
            strcat(m_errlog_buf, "<code>COD1_0400_Vac_LL</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x0800 )
            strcat(m_errlog_buf, "<code>COD1_0800_OPP</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x1000 )
            strcat(m_errlog_buf, "<code>COD1_1000_Iac_H</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x2000 )
            strcat(m_errlog_buf, "<code>COD1_2000_Ipv_H</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x4000 )
            strcat(m_errlog_buf, "<code>COD1_4000_ADCINT_OVF</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD1_Record & 0x8000 )
            strcat(m_errlog_buf, "<code>COD1_8000_Vbus_H</code>");
        // PV_Inv_Error_COD2_Record 0x0122
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x0001 )
            strcat(m_errlog_buf, "<code>COD2_0001_Arc</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x0002 )
            strcat(m_errlog_buf, "<code>COD2_0002_Para_Check</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x0004 )
            strcat(m_errlog_buf, "<code>COD2_0004_Ipv1_short</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x0008 )
            strcat(m_errlog_buf, "<code>COD2_0008_Ipv2_short</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x0010 )
            strcat(m_errlog_buf, "<code>COD2_0010_Vac_Short</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x0020 )
            strcat(m_errlog_buf, "<code>COD2_0020_CT_fault</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x0040 )
            strcat(m_errlog_buf, "<code>COD2_0040_PVOverPower</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x0080 )
            strcat(m_errlog_buf, "<code>COD2_0080_NO_GRID</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x0100 )
            strcat(m_errlog_buf, "<code>COD2_0100_PV_Input_High</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x0200 )
            strcat(m_errlog_buf, "<code>COD2_0200_INV_Overload</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x0400 )
            strcat(m_errlog_buf, "<code>COD2_0400_RCMU_30</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x0800 )
            strcat(m_errlog_buf, "<code>COD2_0800_RCMU_60</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x1000 )
            strcat(m_errlog_buf, "<code>COD2_1000_RCMU_150</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x2000 )
            strcat(m_errlog_buf, "<code>COD2_2000_RCMU_300</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x4000 )
            strcat(m_errlog_buf, "<code>COD2_4000_RCMUtest_Fault</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD2_Record & 0x8000 )
            strcat(m_errlog_buf, "<code>COD2_8000_Vac_LM</code>");
        // PV_Inv_Error_COD3_Record 0x0123
        if ( m_hb2_rt_info.PV_Inv_Error_COD3_Record & 0x0001 )
            strcat(m_errlog_buf, "<code>COD4_0001_External_PV_OPP</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD3_Record & 0x0002 )
            strcat(m_errlog_buf, "<code>COD4_0002_Model123_Reconnected_Delay</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD3_Record & 0x0004 )
            strcat(m_errlog_buf, "<code>COD4_0004_Peak_Shaving_Over_Power</code>");
        if ( m_hb2_rt_info.PV_Inv_Error_COD3_Record & 0x0008 )
            strcat(m_errlog_buf, "<code>COD4_0008_CLA_Execute_Time_Over</code>");
        // DD_Error_COD1_Record 0x0124
        if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x0001 )
            strcat(m_errlog_buf, "<code>COD3_0001_Vbat_H</code>");
        if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x0002 )
            strcat(m_errlog_buf, "<code>COD3_0002_Vbat_L_fault</code>");
        if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x0004 )
            strcat(m_errlog_buf, "<code>COD3_0004_Vbus_H</code>");
        if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x0008 )
            strcat(m_errlog_buf, "<code>COD3_0008_Vbus_L</code>");
        if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x0010 )
            strcat(m_errlog_buf, "<code>COD3_0010_Ibus_H</code>");
        if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x0020 )
            strcat(m_errlog_buf, "<code>COD3_0020_Ibat_H</code>");
        //if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x0040 )
        //    strcat(m_errlog_buf, "<code>COD3_0040_Charger_T</code>");
        if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x0080 )
            strcat(m_errlog_buf, "<code>COD3_0080_Code</code>");
        if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x0100 )
            strcat(m_errlog_buf, "<code>COD3_0100_Vbat_Drop</code>");
        //if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x0200 )
        //    strcat(m_errlog_buf, "<code>COD3_0200_INV_fault</code>");
        //if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x0400 )
        //    strcat(m_errlog_buf, "<code>COD3_0400_GND_Fault</code>");
        if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x0800 )
            strcat(m_errlog_buf, "<code>COD3_0800_No_Bat</code>");
        if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x1000 )
            strcat(m_errlog_buf, "<code>COD3_1000_BMS_Comute_fault</code>");
        //if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x2000 )
        //    strcat(m_errlog_buf, "<code>COD3_2000_BMS_Over_Current</code>");
        if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x4000 )
            strcat(m_errlog_buf, "<code>COD3_4000_Vbus_High_Vbat</code>");
        //if ( m_hb2_rt_info.DD_Error_COD1_Record & 0x8000 )
        //    strcat(m_errlog_buf, "<code>COD3_8000_Bat_Setting_fault</code>");
        // DD_Error_COD2_Record 0x0125
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x0001 )
            strcat(m_errlog_buf, "<code>COD5_0001_EEProm_Fault</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x0002 )
            strcat(m_errlog_buf, "<code>COD5_0002_Communi_Fault</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x0004 )
            strcat(m_errlog_buf, "<code>COD5_0004_OT_Fault</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x0008 )
            strcat(m_errlog_buf, "<code>COD5_0008_Fan_Fault</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x0010 )
            strcat(m_errlog_buf, "<code>COD5_0010_Low_Battery</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x0020 )
            strcat(m_errlog_buf, "<code>COD5_0020_PV3_S</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x0040 )
            strcat(m_errlog_buf, "<code>COD5_0040_PV4_S</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x0080 )
            strcat(m_errlog_buf, "<code>COD5_0080_PV_Over_Power</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x0100 )
            strcat(m_errlog_buf, "<code>COD5_0100_PV_Input_High</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x0200 )
            strcat(m_errlog_buf, "<code>COD5_0200_Restart</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x0400 )
            strcat(m_errlog_buf, "<code>COD5_0400_GND_Fault</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x0800 )
            strcat(m_errlog_buf, "<code>COD5_0800_OT_Alarm</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x1000 )
            strcat(m_errlog_buf, "<code>COD5_1000_Bat_Wake_Up_Fault</code>");
        if ( m_hb2_rt_info.DD_Error_COD2_Record & 0x2000 )
            strcat(m_errlog_buf, "<code>COD5_2000_Vbat_Inconsistent</code>");
    }

    // set system error log
    if ( m_sys_error && (m_data_st_time.tm_hour%2 == 0) && (m_data_st_time.tm_min == 0) ) {
        if ( m_sys_error & SYS_0001_No_USB )
            strcat(m_errlog_buf, "<code>SYS_0001_No_USB</code>");
        if ( m_sys_error & SYS_0002_Save_USB_Fail )
            strcat(m_errlog_buf, "<code>SYS_0002_Save_USB_Fail</code>");
        if ( m_sys_error & SYS_0004_No_SD )
            strcat(m_errlog_buf, "<code>SYS_0004_No_SD</code>");
        if ( m_sys_error & SYS_0008_Save_SD_Fail )
            strcat(m_errlog_buf, "<code>SYS_0008_Save_SD_Fail</code>");
    }

    // set inverter off line log
    if ( m_inverter_state ) {
        strcat(m_errlog_buf, "<code>SYS_0010_Off_Line</code>");
    }

    strcat(m_errlog_buf, "</record>");

    printf("===================== Set Error Log XML end =====================\n");

    return true;
}

bool CG320::SaveErrorLogXML(bool first, bool last)
{
    FILE *fd = NULL;
    struct stat filest;
    char buf[256] = {0};
    char tmp[256] = {0};
    int filesize = 0, offset = 0, ret = 0;

    if ( first ) {
        fd = fopen("/tmp/tmperrlog", "wb");
        if ( fd != NULL ) {
            fwrite("<records>", 1, 9, fd);
            filesize = 9;
        }
    } else {
        stat("/tmp/tmperrlog", &filest);
        filesize = filest.st_size;
        fd = fopen("/tmp/tmperrlog", "ab");
    }

    if ( last ) {
        strcat(m_errlog_buf, "</records>");
        while ( (strlen(m_errlog_buf) + filesize) % 3 != 0 ) {
            m_errlog_buf[strlen(m_errlog_buf)] = 0x20; // add space to end
        }
    }

    if ( fd != NULL ) {
        fwrite(m_errlog_buf, 1, strlen(m_errlog_buf), fd);
        filesize += strlen(m_errlog_buf);
        //printf("Errlog filesize = %d\n", filesize);
        fclose(fd);
    } else {
        SaveLog((char *)"DataLogger SaveErrorLogXML() : open /tmp/tmperrlog Fail", m_st_time);
        printf("open /tmp/tmperrlog Fail!\n");
        return false;
    }

    if ( !last )
        return true;
    else {
        if ( filesize < 42 ) {
            printf("==== SaveErrorLogXML file size = %d, too small, don't copy file to storage ====\n", filesize);
            return true;
        }
    }

    // copy file to target
    sprintf(buf, "cp /tmp/tmperrlog %s", m_errlog_filename);
    ret = system(buf);
    if ( ret == 0 ) {
        sprintf(buf, "DataLogger SaveErrorLogXML() : write %s OK", m_errlog_filename);
        SaveLog(buf, m_st_time);
        if ( strstr(m_errlog_filename, USB_PATH) )
            m_sys_error  &= ~SYS_0002_Save_USB_Fail;
        return true;
    } else {
        sprintf(buf, "DataLogger SaveErrorLogXML() : write %s Fail", m_errlog_filename);
        SaveLog(buf, m_st_time);
        if ( strstr(m_errlog_filename, USB_PATH) )
            m_sys_error |= SYS_0002_Save_USB_Fail;
    }

    // if copy file to storage fail, then copy to tmp
    if ( strstr(m_errlog_filename, DEF_PATH) == NULL ) {
        strcpy(tmp, DEF_PATH);
        if ( strstr(m_errlog_filename, USB_PATH) != NULL )
            offset = strlen(USB_PATH);
        // save to tmp
        strcat(tmp, m_errlog_filename+offset);
        sprintf(buf, "cp /tmp/tmperrlog %s", tmp);
        ret = system(buf);
        if ( ret == 0 ) {
            SaveLog((char *)"DataLogger SaveErrorLogXML() : write to tmp OK", m_st_time);
            return true;
        } else {
            SaveLog((char *)"DataLogger SaveErrorLogXML() : write to tmp Fail", m_st_time);
            return false;
        }
    }

    return false;
}

void CG320::SetEnvXML()
{
    sprintf(m_env_filename, "%s/%02d%02d", m_dl_path.m_env_path, m_data_st_time.tm_hour, m_data_st_time.tm_min);
    //printf("env path = %s\n", m_env_filename);
    return;
}

bool CG320::SaveEnvXML(bool first, bool last)
{
    FILE *fd = NULL;
    struct stat filest;
    char buf[256] = {0};
    char tmp[256] = {0};
    int filesize = 0, offset = 0, ret = 0;

    if ( first ) {
        fd = fopen("/tmp/tmpenv", "wb");
        if ( fd != NULL ) {
            fwrite("<records>", 1, 9, fd);
            filesize = 9;
        }
    } else {
        stat("/tmp/tmpenv", &filest);
        filesize = filest.st_size;
        fd = fopen("/tmp/tmpenv", "ab");
    }

    if ( last ) {
        strcat(buf, "</records>");
        while ( (strlen(buf) + filesize) % 3 != 0 ) {
            buf[strlen(buf)] = 0x20; // add space to end
        }
    }

    if ( fd != NULL ) {
        fwrite(buf, 1, strlen(buf), fd);
        filesize += strlen(buf);
        //printf("Log filesize = %d\n", filesize);
        fclose(fd);
    } else {
        SaveLog((char *)"CG320 SaveEnvXML() : open /tmp/tmpenv Fail", m_st_time);
        printf("open /tmp/tmpenv Fail!\n");
        return false;
    }

    if ( !last )
        return true;
    else {
        if ( filesize < 42 ) {
            printf("==== SaveEnvXML file size = %d, too small, don't copy file to storage ====\n", filesize);
            return true;
        }
    }

    // copy file to target
    sprintf(buf, "cp /tmp/tmpenv %s", m_env_filename);
    ret = system(buf);
    if ( ret == 0 ) {
        sprintf(buf, "CG320 SaveEnvXML() : write %s OK", m_env_filename);
        SaveLog(buf, m_st_time);
        //if ( strstr(m_env_filename, USB_PATH) )
        //    m_sys_error  &= ~SYS_0002_Save_USB_Fail;
        return true;
    } else {
        sprintf(buf, "CG320 SaveEnvXML() : write %s Fail", m_env_filename);
        SaveLog(buf, m_st_time);
        //if ( strstr(m_env_filename, USB_PATH) )
        //    m_sys_error |= SYS_0002_Save_USB_Fail;
    }

    // if copy file to storage fail, then copy to tmp
    if ( strstr(m_env_filename, DEF_PATH) == NULL ) {
        strcpy(tmp, DEF_PATH);
        if ( strstr(m_env_filename, USB_PATH) != NULL )
            offset = strlen(USB_PATH);
        // save to tmp
        strcat(tmp, m_env_filename+offset);
        sprintf(buf, "cp /tmp/tmpenv %s", tmp);
        ret = system(buf);
        if ( ret == 0 ) {
            SaveLog((char *)"CG320 SaveEnvXML() : write to tmp OK", m_st_time);
            return true;
        } else {
            SaveLog((char *)"CG320 SaveEnvXML() : write to tmp Fail", m_st_time);
            return false;
        }
    }

    return false;
}

void CG320::SetBMSPath(int index)
{
    sprintf(m_bms_filename, "%s/%s_%02d", m_dl_path.m_bms_path, arySNobj[index].m_Sn, m_data_st_time.tm_hour);
    //printf("bms path = %s\n", m_bms_filename);
    return;
}

bool CG320::SaveBMS()
{
    char buf[256] = {0};
    FILE *pFile = NULL;
    int offset = 0;
    struct stat st;
    bool set_header = false;

    if ( stat(m_bms_filename, &st) == -1 )
        set_header = true;
    else
        set_header = false;

    // save to storage
    pFile = fopen(m_bms_filename, "ab");
    if ( pFile != NULL ) {
        if ( set_header )
            fwrite(m_bms_header, 1, strlen(m_bms_header), pFile);
        if ( fwrite(m_bms_mainbuf, 1, strlen(m_bms_mainbuf), pFile) ) {
            sprintf(buf, "DataLogger SaveBMS() : write %s OK", m_bms_filename);
            SaveLog(buf, m_st_time);
            if ( strstr(m_log_filename, USB_PATH) )
                m_sys_error  &= ~SYS_0002_Save_USB_Fail;
            fclose(pFile);
            return true;
        } else {
            sprintf(buf, "DataLogger SaveBMS() : write %s Fail", m_bms_filename);
            SaveLog(buf, m_st_time);
            if ( strstr(m_log_filename, USB_PATH) )
                m_sys_error  |= SYS_0002_Save_USB_Fail;
            fclose(pFile);
        }
    } else {
        sprintf(buf, "DataLogger SaveBMS() : open %s Fail", m_bms_filename);
        SaveLog(buf, m_st_time);
        if ( strstr(m_log_filename, USB_PATH) )
            m_sys_error  |= SYS_0002_Save_USB_Fail;
    }

    if ( strstr(m_bms_filename, DEF_PATH) == NULL ) {
        strcpy(buf, DEF_PATH);
        if ( strstr(m_bms_filename, USB_PATH) != NULL )
            offset = strlen(USB_PATH);
        // save to tmp
        strcat(buf, m_bms_filename+offset);

        if ( stat(buf, &st) == -1 )
            set_header = true;
        else
            set_header = false;

        pFile = fopen(buf, "ab");
        if ( pFile != NULL ) {
            if ( set_header )
                fwrite(m_bms_header, 1, strlen(m_bms_header), pFile);
            if ( fwrite(m_bms_mainbuf, 1, strlen(m_bms_mainbuf), pFile) ) {
                SaveLog((char *)"DataLogger SaveBMS() : write to tmp OK", m_st_time);
                fclose(pFile);
                return true;
            } else {
                SaveLog((char *)"DataLogger SaveBMS() : write to tmp Fail", m_st_time);
                fclose(pFile);
                return false;
            }
        } else {
            SaveLog((char *)"DataLogger SaveBMS() : open tmp Fail", m_st_time);
            return false;
        }
    }

    return false;
}

bool CG320::WriteMIListXML()
{
    char buf[256] = {0};
    char tmp[256] = {0};
    char idtmp[13] = {0};
    int i = 0, model = 0, listsize = 0;
    unsigned long long int dev_id = 0;
    struct stat filest;
    FILE *pFile = NULL;

    //GetLocalTime(); // cancel, GetData set time already

    sprintf(buf, "/tmp/tmpMIList");

    if ( m_first )
        pFile = fopen(buf, "wb");
    else
        pFile = fopen(buf, "ab");
    if ( pFile == NULL ) {
        printf("open %s fail\n", buf);
        sprintf(tmp, "DataLogger WriteMIListXML() : fopen %s fail", buf);
        SaveLog(tmp, m_st_time);
        return false;
    }

    printf("==================== Set MIList XML start ====================\n");
    if ( m_first )
        fputs("<records>\n", pFile);

    for ( i = 0; i < m_snCount; i++) {
        if ( strlen(arySNobj[i].m_Sn) == 0 )
            continue;
        if ( arySNobj[i].m_Device > -1 && arySNobj[i].m_Device < 0x0A ) { // MI device
            model = 0;
            sscanf(arySNobj[i].m_Sn+4, "%04d", &model); // get 5-8 digit
            printf("Get index %d model = %d\n", i, model);
            if ( model > 2 ) {
                // A part
                fputs("\t<record>\n", pFile);

                sprintf(buf, "\t\t<sn>A%s</sn>\n", arySNobj[i].m_Sn+1);
                //printf("%s", buf);
                fputs(buf, pFile);

                sprintf(buf, "\t\t<OriSn>%s</OriSn>\n", arySNobj[i].m_Sn);
                //printf("%s", buf);
                fputs(buf, pFile);

                sprintf(buf, "\t\t<port>COM%d</port>\n", m_dl_config.m_inverter_port);
                //printf("%s", buf);
                fputs(buf, pFile);

                sprintf(buf, "\t\t<slaveId>%d</slaveId>\n", arySNobj[i].m_Addr);
                //printf("%s", buf);
                fputs(buf, pFile);

                fputs("\t\t<Manufacturer>DARFON</Manufacturer>\n", pFile);

                sprintf(buf, "\t\t<Model>%d</Model>\n", model);
                //printf("%s", buf);
                fputs(buf, pFile);

                fputs("\t\t<OtherType>0</OtherType>\n", pFile);

                idtmp[0] = 'A';
                strcpy(idtmp+1, arySNobj[i].m_Sn+5);
                sscanf(idtmp, "%012llX", &dev_id); // get last 12 digit
                sprintf(buf, "\t\t<dev_id>%lld</dev_id>\n", dev_id);
                //printf("%s", buf);
                fputs(buf, pFile);

                fputs("\t</record>\n", pFile);

                // B part
                fputs("\t<record>\n", pFile);

                sprintf(buf, "\t\t<sn>B%s</sn>\n", arySNobj[i].m_Sn+1);
                //printf("%s", buf);
                fputs(buf, pFile);

                sprintf(buf, "\t\t<OriSn>%s</OriSn>\n", arySNobj[i].m_Sn);
                //printf("%s", buf);
                fputs(buf, pFile);

                sprintf(buf, "\t\t<port>COM%d</port>\n", m_dl_config.m_inverter_port);
                //printf("%s", buf);
                fputs(buf, pFile);

                sprintf(buf, "\t\t<slaveId>%d</slaveId>\n", arySNobj[i].m_Addr);
                //printf("%s", buf);
                fputs(buf, pFile);

                fputs("\t\t<Manufacturer>DARFON</Manufacturer>\n", pFile);

                sprintf(buf, "\t\t<Model>%d</Model>\n", model);
                //printf("%s", buf);
                fputs(buf, pFile);

                fputs("\t\t<OtherType>0</OtherType>\n", pFile);

                idtmp[0] = 'B';
                //strcpy(idtmp+1, arySNobj[i].m_Sn+5);
                sscanf(idtmp, "%012llX", &dev_id); // get last 12 digit
                sprintf(buf, "\t\t<dev_id>%lld</dev_id>\n", dev_id);
                //printf("%s", buf);
                fputs(buf, pFile);

                fputs("\t</record>\n", pFile);
            } else {
                fputs("\t<record>\n", pFile);

                sprintf(buf, "\t\t<sn>%s</sn>\n", arySNobj[i].m_Sn);
                //printf("%s", buf);
                fputs(buf, pFile);

                sprintf(buf, "\t\t<OriSn>%s</OriSn>\n", arySNobj[i].m_Sn);
                //printf("%s", buf);
                fputs(buf, pFile);

                sprintf(buf, "\t\t<port>COM%d</port>\n", m_dl_config.m_inverter_port);
                //printf("%s", buf);
                fputs(buf, pFile);

                sprintf(buf, "\t\t<slaveId>%d</slaveId>\n", arySNobj[i].m_Addr);
                //printf("%s", buf);
                fputs(buf, pFile);

                fputs("\t\t<Manufacturer>DARFON</Manufacturer>\n", pFile);

                sprintf(buf, "\t\t<Model>%d</Model>\n", model);
                //printf("%s", buf);
                fputs(buf, pFile);

                fputs("\t\t<OtherType>0</OtherType>\n", pFile);

                strcpy(idtmp, arySNobj[i].m_Sn+4);
                sscanf(idtmp, "%012llX", &dev_id); // get last 12 digit
                sprintf(buf, "\t\t<dev_id>%lld</dev_id>\n", dev_id);
                //printf("%s", buf);
                fputs(buf, pFile);

                fputs("\t</record>\n", pFile);
            }
        } else if ( arySNobj[i].m_Device > 0x09 ) { // Hybrid device
            fputs("\t<record>\n", pFile);

            sprintf(buf, "\t\t<sn>%s</sn>\n", arySNobj[i].m_Sn);
            //printf("%s", buf);
            fputs(buf, pFile);

            sprintf(buf, "\t\t<OriSn>%s</OriSn>\n", arySNobj[i].m_Sn);
            //printf("%s", buf);
            fputs(buf, pFile);

            sprintf(buf, "\t\t<port>COM%d</port>\n", m_dl_config.m_inverter_port);
            //printf("%s", buf);
            fputs(buf, pFile);

            sprintf(buf, "\t\t<slaveId>%d</slaveId>\n", arySNobj[i].m_Addr);
            //printf("%s", buf);
            fputs(buf, pFile);

            fputs("\t\t<Manufacturer>DARFON</Manufacturer>\n", pFile);

            model = 0;
            sscanf(arySNobj[i].m_Sn+6, "%02x", &model); // get 7-8 digit
            sprintf(buf, "\t\t<Model>%d</Model>\n", model);
            //printf("%s", buf);
            fputs(buf, pFile);

            fputs("\t\t<OtherType>2</OtherType>\n", pFile);

            sscanf(arySNobj[i].m_Sn+4, "%012llX", &dev_id); // get last 12 digit
            sprintf(buf, "\t\t<dev_id>%lld</dev_id>\n", dev_id);
            //printf("%s", buf);
            fputs(buf, pFile);

            fputs("\t</record>\n", pFile);
        } else { // unknown device
            fputs("\t<record>\n", pFile);

            sprintf(buf, "\t\t<sn>%s</sn>\n", arySNobj[i].m_Sn);
            //printf("%s", buf);
            fputs(buf, pFile);

            sprintf(buf, "\t\t<OriSn>%s</OriSn>\n", arySNobj[i].m_Sn);
            //printf("%s", buf);
            fputs(buf, pFile);

            sprintf(buf, "\t\t<port>COM%d</port>\n", m_dl_config.m_inverter_port);
            //printf("%s", buf);
            fputs(buf, pFile);

            sprintf(buf, "\t\t<slaveId>%d</slaveId>\n", arySNobj[i].m_Addr);
            //printf("%s", buf);
            fputs(buf, pFile);

            fputs("\t\t<Manufacturer>DARFON</Manufacturer>\n", pFile);

            model = 0;
            sscanf(arySNobj[i].m_Sn+4, "%04d", &model); // get 5-8 digit
            sprintf(buf, "\t\t<Model>%d</Model>\n", model);
            //printf("%s", buf);
            fputs(buf, pFile);

            //fputs("\t\t<OtherType>2</OtherType>\n", pFile);

            sscanf(arySNobj[i].m_Sn+4, "%012llX", &dev_id); // get last 12 digit
            sprintf(buf, "\t\t<dev_id>%lld</dev_id>\n", dev_id);
            //printf("%s", buf);
            fputs(buf, pFile);

            fputs("\t</record>\n", pFile);
        }
    }

    if ( m_last )
        fputs("</records>", pFile);
    printf("===================== Set MIList XML end =====================\n");

    fclose(pFile);

    if ( m_last ) {
        //system("sync");
        stat("/tmp/tmpMIList", &filest);
        listsize = filest.st_size;
        //printf("listsize = %d\n", listsize);
        if ( m_milist_size != listsize ) {
            // clean old MIList
            printf("clean old MIList\n");
            system("rm /tmp/MIList_*");
            sprintf(buf, "cp /tmp/tmpMIList /tmp/MIList_%4d%02d%02d_%02d%02d00",
                    1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday,
                    m_st_time->tm_hour, m_st_time->tm_min);
            printf("run command : \n%s\n", buf);
            system(buf);
            m_milist_size = listsize;
        }
    }

    //SaveLog((char *)"DataLogger WriteMIListXML() : OK", m_st_time);

    return true;
}

