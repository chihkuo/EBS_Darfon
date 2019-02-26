#include "datalog.h"
#include "CyberPower.h"
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
#define USB_PATH        "/tmp/run/mountd/sda1"
#define SDCARD_PATH     "/tmp/sdcard"

//#define WHITE_LIST_PATH "/usr/home/White-List.txt"
#define TODOLIST_PATH   "/tmp/TODOList"
//#define WL_CHANGED_PATH "/tmp/WL_Changed"

#define TIMEZONE_URL    "http://ip-api.com/json"
#define TIME_OFFSET_URL "http://svn.fonosfera.org/fon-ng/trunk/luci/modules/admin-fon/root/etc/timezones.db"
//#define KEY             "O10936IZHJTQ"
#define TIME_SERVER_URL "https://www.worldtimeserver.com/handlers/GetData.ashx?action=GCTData"

extern "C"
{
    #include "../common/SaveLog.h"
}

extern "C" {
    extern int      MyModbusDrvInit(char *port, int baud, int data_bits, char parity, int stop_bits);
    extern void     MakeReadDataCRC(unsigned char *,int );
    extern void     MClearRX();
    extern void     MStartTX(int fd);
    extern unsigned char*   GetCyberPowerRespond(int fd, int iSize, int delay);

    extern unsigned int     txsize;
    extern unsigned char    waitAddr, waitFCode;
    extern bool             have_respond;

    extern unsigned char    txbuffer[1544];//MODBUS_TX_BUFFER_SIZE
}

CyberPower::CyberPower()
{
    m_addr = 0;
    m_devid = 0;
    m_busfd = 0;
    m_milist_size = 0;
    m_cp_sn = {0};
    m_cp_1ppi = {0};
    m_cp_ec = {0};
    m_cp_ec_e = {0};
    m_cp_ec_w = {0};
    m_cp_ec_f = {0};
    m_dl_config = {0};
    m_dl_path = {0};
    m_loopflag = 0;
    m_get_error = 0;
    m_sys_error = 0;
    m_do_get_TZ = false;
    m_st_time = NULL;
    m_current_time = 0;

    memset(m_log_buf, 0x00, CP_LOG_BUF_SIZE);
    memset(m_log_filename, 0x00, 128);
    memset(m_errlog_buf, 0x00, CP_LOG_BUF_SIZE);
    memset(m_errlog_filename, 0x00, 128);
    memset(m_env_filename, 0x00, 128);
}

CyberPower::~CyberPower()
{
    //dtor
}

int CyberPower::Init(int com, bool open_com, bool first, int busfd)
{
    char *port = NULL;
    char szbuf[32] = {0};
    char inverter_parity = 0;
    int ret = 0;

    printf("#### CyberPower Init Start ####\n");

    // set com port
    m_dl_config.m_inverter_port = com;

    GetMAC();
    GetDLConfig();

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
        if ( m_busfd < 0 )
            ret = -1;
        else
            ret = m_busfd;
    } else {
        m_busfd = busfd;
    }

    // get time zone
    if ( first ) {
        GetTimezone();
        usleep(1000000);
    }

    // set save file path
    GetLocalTime();
    SetPath();

    printf("\n##### CyberPower Init End #####\n");

    return ret;
}

void CyberPower::Get1PData(int addr, int devid, time_t data_time, bool first, bool last)
{
    struct  stat st;

    //m_current_time = time(NULL);
    m_current_time = data_time;
    m_st_time = localtime(&m_current_time);
    // set path
    SetPath();
    SetLogXML();
    SetErrorLogXML();
    SetEnvXML();
    CheckConfig();
    CleanParameter();

    m_addr = addr;
    m_devid = devid;
    m_loopflag = 0;

    if ( stat(m_log_filename, &st) == 0 ) {
        printf("======== %s exist! ========\n", m_log_filename);
        return;
    }

    OpenLog(m_dl_path.m_syslog_path, m_st_time);

    // if need, add here
    //RunTODOList();

    if ( GetSN() )
        ;
    else
        m_loopflag++;

    if ( Get1PPowerInfo() )
        ;
    else
        m_loopflag++;

    if ( GetErrorCode() )
        ;
    else
        m_loopflag++;

    if ( m_loopflag == 3 ) {
        m_get_error++;
        if ( m_get_error > 3 )
            m_get_error = 3;
    } else
        m_get_error = 0;

    WriteLogXML();
    SaveLogXML(first, last);
    if ( m_cp_ec.E1_E00_15 || m_cp_ec.E2_E16_31 || m_cp_ec.E3_W00_15 || m_cp_ec.E4_F00_15 ||
            m_cp_ec.E5_F16_31 || m_cp_ec.E6_F32_47 || m_cp_ec.E7_F48_63 || m_cp_ec.E8_F64_79 ||
            (m_sys_error && (m_st_time->tm_hour%2 == 0) && (m_st_time->tm_min == 0)))
        WriteErrorLogXML();
    SaveErrorLogXML(first, last);

    SaveEnvXML(first, last);

    WriteMIListXML(first, last, 1);

    SaveDeviceList(first, last, 1);

    //CloseLog(); // cancel, move to main loop execute at loop end
    system("sync");

    // get timezone if before not succss
    if ( m_do_get_TZ )
        GetTimezone();

    return;
}

void CyberPower::GetMAC()
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

bool CyberPower::GetDLConfig()
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
    pFile = popen("uci get dlsetting.@sms[0].sample_time", "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &m_dl_config.m_sample_time);
    printf("Sample time (Min.) = %d\n", m_dl_config.m_sample_time);
    // get delay_time
    pFile = popen("uci get dlsetting.@sms[0].delay_time_1", "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &m_dl_config.m_delay_time_1);
    printf("Delay time (us.) = %d\n", m_dl_config.m_delay_time_1);

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

bool CyberPower::SetPath()
{
    //printf("#### SetPath Start ####\n");

    char buf[256] = {0};
    char tmpbuf[256] = {0};
    struct stat st;
    bool mk_tmp_dir = false;

    // set root path (XML & BMS & SYSLOG in the same dir.)
    if ( stat(USB_PATH, &st) == 0 ) { /*linux usb storage detect*/
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
    memset(buf, 0, 256);
    sprintf(buf, "%s/BMS/%4d%02d%02d", m_dl_path.m_root_path, 1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday);
    strcpy(m_dl_path.m_bms_path, buf);
    if ( stat(m_dl_path.m_bms_path, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", m_dl_path.m_bms_path);
        if ( mkdir(m_dl_path.m_bms_path, 0755) == -1 )
            printf("mkdir %s fail!\n", m_dl_path.m_bms_path);
        else
            printf("mkdir %s OK\n", m_dl_path.m_bms_path);
    }

    // set SYSLOG path
    sprintf(m_dl_path.m_syslog_path, "%s/SYSLOG", m_dl_path.m_root_path);
    if ( stat(m_dl_path.m_syslog_path, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", m_dl_path.m_syslog_path);
        if ( mkdir(m_dl_path.m_syslog_path, 0755) == -1 )
            printf("mkdir %s fail!\n", m_dl_path.m_syslog_path);
        else
            printf("mkdir %s OK\n", m_dl_path.m_syslog_path);
    }

    printf("m_xml_path = %s\n", m_dl_path.m_xml_path);
    printf("m_log_path = %s\n", m_dl_path.m_log_path);
    printf("m_errlog_path = %s\n", m_dl_path.m_errlog_path);
    printf("m_env_path    = %s\n", m_dl_path.m_env_path);
    printf("m_bms_path = %s\n", m_dl_path.m_bms_path);
    printf("m_syslog_path = %s\n", m_dl_path.m_syslog_path);

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
        sprintf(tmpbuf, "%s/BMS/%4d%02d%02d", DEF_PATH, 1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday);
        if ( stat(tmpbuf, &st) == -1 ) {
            printf("%s not exist, run mkdir!\n", tmpbuf);
            if ( mkdir(tmpbuf, 0755) == -1 )
                printf("mkdir %s fail!\n", tmpbuf);
            else
                printf("mkdir %s OK\n", tmpbuf);
        }

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

void CyberPower::CleanParameter()
{
    memset(m_log_buf, 0x00, CP_LOG_BUF_SIZE);
    memset(m_errlog_buf, 0x00, CP_LOG_BUF_SIZE);
    m_addr = 0;
    m_devid = 0;
    m_cp_sn = {0};
    m_cp_1ppi = {0};
    m_cp_ec = {0};
    m_cp_ec_e = {0};
    m_cp_ec_w = {0};
    m_cp_ec_f = {0};

    return;
}

bool CyberPower::CheckConfig()
{
    printf("#### CheckConfig start ####\n");

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
        printf("tmp sample_time = %d\n", tmp);

        if ( m_dl_config.m_sample_time == tmp )
            ;//printf("same sample_time\n");
        else
            m_dl_config.m_sample_time = tmp;
    }

    fd = popen("uci get dlsetting.@sms[0].delay_time", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
    } else {
        fgets(buf, 32, fd);
        pclose(fd);

        sscanf(buf, "%d", &tmp);
        printf("tmp delay_time = %d\n", tmp);

        if ( m_dl_config.m_delay_time_1 == tmp )
            ;//printf("same delay\n");
        else
            m_dl_config.m_delay_time_1 = tmp;
    }

    printf("#### CheckConfig end ####\n");

    return true;
}

bool CyberPower::GetTimezone()
{
    char buf[1024] = {0};
    char tmp[64]= {0};
    char timezone[64] = {0};
    char time_offset[64] = {0};
    char *index = NULL;
    FILE *pFile = NULL;
    int i = 0, j = 0;

    m_do_get_TZ = true;
    printf("\n########### Get Timezone ###########\n");
    // get timezone from ip
    sprintf(buf, "curl %s --max-time 30 > /tmp/timezone", TIMEZONE_URL);
    //printf("cmd = %s\n", buf);
    system(buf);
    pFile = fopen("/tmp/timezone", "rb");
    if ( pFile == NULL ) {
        printf("Open /tmp/timezone fail!\n");
        return false;
    }
    fread(buf, 1024, 1, pFile);
    //printf("Debug : buf[] = %s\n", buf);
    fclose(pFile);

    // find timezone
    index = strstr(buf, "timezone"); // find "timezone":"ZZZ/YYY" ex: Asia/Taipei
    if ( index == NULL ) {
        printf("timezone not found!\n");
        return false;
    }
    strncpy(tmp, index+11, 63); // copy start at Z, example "timezone":"ZZZ/YYY", get ZZZ/YYY, end  of "
    for (i = 0; i < 63; i++) {
        if ( tmp[i] == '"' ) {
            timezone[j] = 0; // stop at "
            break;
        }
        timezone[j] = tmp[i];
        j++;
    }
    printf("Debug : timezone[] = %s\n", timezone);
    if ( strlen(timezone) == 0 )
        return false;

    // get time offset
    sprintf(buf, "curl %s --max-time 30 | grep -i %s | awk '{print $2}' > /tmp/time_offset", TIME_OFFSET_URL, timezone);
    //printf("cmd = %s\n", buf);
    system(buf);
    pFile = fopen("/tmp/time_offset", "rb");
    if ( pFile == NULL ) {
        printf("Open /tmp/time_offset fail!\n");
        return false;
    }
    fgets(time_offset, 64, pFile);
    time_offset[strlen(time_offset)-1] = 0; // remove \n
    printf("Debug : time_offset[] = %s\n", time_offset);
    fclose(pFile);
    if ( strlen(time_offset) == 0 )
        return false;

    SetTimezone(timezone, time_offset);
    m_do_get_TZ = false;

    usleep(1000000);
    GetNTPTime();

    printf("####################################\n");

    return true;
}

// zonename : ex Asia/Taipei, timazone : ex CST-8
void CyberPower::SetTimezone(char *zonename, char *timazone)
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

void CyberPower::GetNTPTime()
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
            break;
        }

        sscanf(buf, "%d", &PID);
        sprintf(buf, "kill %d", PID);
        system(buf);
    }

    printf("############ Get NTP END #############\n");

    return;
}

void CyberPower::GetLocalTime()
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

bool CyberPower::GetSN()
{
    printf("#### GetSN start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    unsigned char cmd_buf[]={0x00, 0x04, 0x10, 0x08, 0x00, 0x08, 0x00, 0x00};
    cmd_buf[0] = m_devid;
    MakeReadDataCRC(cmd_buf,8);

    MClearRX();
    txsize = 8;
    waitAddr = m_devid;
    waitFCode = 0x04;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd_buf, 8);
        MStartTX(m_busfd);
        usleep(m_dl_config.m_delay_time_1);

        lpdata = GetCyberPowerRespond(m_busfd, 21, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### GetSN OK ####\n");
            SaveLog((char *)"CyberPower GetSN() : OK", m_st_time);
            DumpSN(lpdata+3);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetSN CRC Error ####\n");
                SaveLog((char *)"CyberPower GetSN() : CRC Error", m_st_time);
            }
            else {
                printf("#### GetSN No Response ####\n");
                SaveLog((char *)"CyberPower GetSN() : No Response", m_st_time);
            }
            err++;
        }
    }

    return false;
}

void CyberPower::DumpSN(unsigned char *buf)
{
    int i = 0;
    int len = strlen((const char *)buf);

    // debug
    //printf("DumpSN strlen = %d\n", len);

    for ( i = 0; i < len; i++)
        m_cp_sn.SN[i] = buf[i];
    m_cp_sn.SN[len] = 0;

    printf("#### Dump CyberPower SN ####\n");
    printf("SN = %s\n", m_cp_sn.SN);
    printf("############################\n");

    return;
}

bool CyberPower::Get1PPowerInfo()
{
    printf("#### Get1PPowerInfo start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    unsigned char cmd_buf[]={0x00, 0x04, 0x10, 0x38, 0x00, 0x34, 0x00, 0x00};
    cmd_buf[0] = m_devid;
    MakeReadDataCRC(cmd_buf,8);

    MClearRX();
    txsize = 8;
    waitAddr = m_devid;
    waitFCode = 0x04;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd_buf, 8);
        MStartTX(m_busfd);
        usleep(m_dl_config.m_delay_time_1);

        lpdata = GetCyberPowerRespond(m_busfd, 109, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### Get1PPowerInfo OK ####\n");
            SaveLog((char *)"CyberPower Get1PPowerInfo() : OK", m_st_time);
            Dump1PPowerInfo(lpdata+3);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### Get1PPowerInfo CRC Error ####\n");
                SaveLog((char *)"CyberPower Get1PPowerInfo() : CRC Error", m_st_time);
            }
            else {
                printf("#### Get1PPowerInfo No Response ####\n");
                SaveLog((char *)"CyberPower Get1PPowerInfo() : No Response", m_st_time);
            }
            err++;
        }
    }

    return false;
}

void CyberPower::Dump1PPowerInfo(unsigned char *buf)
{
    int tmp_lo = 0, tmp_hi = 0;
    //int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11, tmp12, tmp13, tmp14, tmp15, tmp16;
    //int tmp17, tmp18, tmp19, tmp20, tmp21, tmp22, tmp23, tmp24, tmp25, tmp26, tmp27, tmp28, tmp29, tmp30;

    // Today kWh
    tmp_lo = (*(buf) << 8) + *(buf+1);
    tmp_hi = (*(buf+2) << 8) + *(buf+3);
    m_cp_1ppi.Today_KWH = (tmp_hi << 16) + tmp_lo;
    // Today Runtime
    //tmp_lo = (*(buf+4) << 8) + *(buf+5);
    //tmp_hi = (*(buf+6) << 8) + *(buf+7);
    //tmp1 = (tmp_hi << 16) + tmp_lo;
    // System Life kWh
    tmp_lo = (*(buf+8) << 8) + *(buf+9);
    tmp_hi = (*(buf+10) << 8) + *(buf+11);
    m_cp_1ppi.System_Life_KWH = (tmp_hi << 16) + tmp_lo;
    /*// System Life Runtime
    tmp_lo = (*(buf+12) << 8) + *(buf+13);
    tmp_hi = (*(buf+14) << 8) + *(buf+15);
    tmp2 = (tmp_hi << 16) + tmp_lo;
    // Input1 Today kWh
    tmp_lo = (*(buf+16) << 8) + *(buf+17);
    tmp_hi = (*(buf+18) << 8) + *(buf+19);
    tmp3 = (tmp_hi << 16) + tmp_lo;
    // Input1 Today Runtime
    tmp_lo = (*(buf+20) << 8) + *(buf+21);
    tmp_hi = (*(buf+22) << 8) + *(buf+23);
    tmp4 = (tmp_hi << 16) + tmp_lo;
    // Input1 Life kWh
    tmp_lo = (*(buf+24) << 8) + *(buf+25);
    tmp_hi = (*(buf+26) << 8) + *(buf+27);
    tmp5 = (tmp_hi << 16) + tmp_lo;
    // Input1 Life Runtime
    tmp_lo = (*(buf+28) << 8) + *(buf+29);
    tmp_hi = (*(buf+30) << 8) + *(buf+31);
    tmp6 = (tmp_hi << 16) + tmp_lo;
    // Input2 Today kWh
    tmp_lo = (*(buf+32) << 8) + *(buf+33);
    tmp_hi = (*(buf+34) << 8) + *(buf+35);
    tmp7 = (tmp_hi << 16) + tmp_lo;
    // Input2 Today Runtime
    tmp_lo = (*(buf+36) << 8) + *(buf+37);
    tmp_hi = (*(buf+38) << 8) + *(buf+39);
    tmp8 = (tmp_hi << 16) + tmp_lo;
    // Input2 Life kWh
    tmp_lo = (*(buf+40) << 8) + *(buf+41);
    tmp_hi = (*(buf+42) << 8) + *(buf+43);
    tmp9 = (tmp_hi << 16) + tmp_lo;
    // Input2 Life Runtime
    tmp_lo = (*(buf+44) << 8) + *(buf+45);
    tmp_hi = (*(buf+46) << 8) + *(buf+47);
    tmp10 = (tmp_hi << 16) + tmp_lo;
    // Input1 Volt (average)
    tmp11 = (*(buf+48) << 8) + *(buf+49);
    // Input1 Curr (average)
    tmp12 = (*(buf+50) << 8) + *(buf+51);
    // Input1 Watt (average)
    tmp13 = (*(buf+52) << 8) + *(buf+53);
    // Input2 Volt (average)
    tmp14 = (*(buf+54) << 8) + *(buf+55);
    // Input2 Curr (average)
    tmp15 = (*(buf+56) << 8) + *(buf+57);
    // Input2 Watt (average)
    tmp16 = (*(buf+58) << 8) + *(buf+59);
    // Bus Volt (Pos), (average)
    tmp17 = (*(buf+60) << 8) + *(buf+61);
    // Bus Volt (Neg), (average)
    tmp18 = (*(buf+62) << 8) + *(buf+63);
    // AC Volt (average)
    tmp19 = (*(buf+64) << 8) + *(buf+65);
    // AC Curr (average)
    tmp20 = (*(buf+66) << 8) + *(buf+67);
    // AC Watt (average)
    tmp21 = (*(buf+68) << 8) + *(buf+69);
    // AC Freq (average)
    tmp22 = (*(buf+70) << 8) + *(buf+71);
    // AC Volt (redundant) (average)
    tmp23 = (*(buf+72) << 8) + *(buf+73);
    // AC Freq (redundant) (average)
    tmp24 = (*(buf+74) << 8) + *(buf+75);
    // Temperature (Ambient)
    tmp25 = (*(buf+76) << 8) + *(buf+77);
    // Temperature (Heatsink)
    tmp26 = (*(buf+78) << 8) + *(buf+79);*/
    // Input1 Volt
    m_cp_1ppi.Input1_Volt = (*(buf+80) << 8) + *(buf+81);
    // Input1 Curr
    m_cp_1ppi.Input1_Curr = (*(buf+82) << 8) + *(buf+83);
    // Input1 Watt
    m_cp_1ppi.Input1_Watt = (*(buf+84) << 8) + *(buf+85);
    // Input2 Volt
    m_cp_1ppi.Input2_Volt = (*(buf+86) << 8) + *(buf+87);
    // Input2 Curr
    m_cp_1ppi.Input2_Curr = (*(buf+88) << 8) + *(buf+89);
    // Input2 Watt
    m_cp_1ppi.Input2_Watt = (*(buf+90) << 8) + *(buf+91);
    // Bus Volt (Pos)
    //tmp27 = (*(buf+92) << 8) + *(buf+93);
    // Bus Volt (Neg)
    //tmp28 = (*(buf+94) << 8) + *(buf+95);
    // AC Volt
    m_cp_1ppi.AC_Volt = (*(buf+96) << 8) + *(buf+97);
    // AC Curr
    m_cp_1ppi.AC_Curr = (*(buf+98) << 8) + *(buf+99);
    // AC Watt
    m_cp_1ppi.AC_Watt = (*(buf+100) << 8) + *(buf+101);
    // AC Freq
    m_cp_1ppi.AC_Freq = (*(buf+102) << 8) + *(buf+103);
    // AC Volt (redundant)
    //tmp29 = (*(buf+104) << 8) + *(buf+105);
    // AC Freq (redundant)
    //tmp30 = (*(buf+106) << 8) + *(buf+107);

    printf("#### Dump CyberPower 1P PowerInfo ####\n");
    printf("Today kWh       = %05.3f KWH\n", (float)m_cp_1ppi.Today_KWH/1000);
    //printf("Today Runtime         = %d Sec\n", tmp1);
    printf("System Life kWh = %05.3f KWH\n", (float)m_cp_1ppi.System_Life_KWH/1000);
    /*printf("System Life Runtime   = %d Sec\n", tmp2);
    printf("Input1 Today kWh      = %05.3f KWH\n", (float)tmp3/1000);
    printf("Input1 Today Runtime  = %d Sec\n", tmp4);
    printf("Input1 Life kWh       = %05.3f KWH\n", (float)tmp5/1000);
    printf("Input1 Life Runtime   = %d Sec\n", tmp6);
    printf("Input2 Today kWh      = %05.3f KWH\n", (float)tmp7/1000);
    printf("Input2 Today Runtime  = %d Sec\n", tmp8);
    printf("Input2 Life kWh       = %05.3f KWH\n", (float)tmp9/1000);
    printf("Input2 Life Runtime   = %d Sec\n", tmp10);
    printf("Input1 Volt (average) = %03.1f V\n", (float)tmp11/10);
    printf("Input1 Curr (average) = %04.2f A\n", (float)tmp12/100);
    printf("Input1 Watt (average) = %d W\n", tmp13);
    printf("Input2 Volt (average) = %03.1f V\n", (float)tmp14/10);
    printf("Input2 Curr (average) = %04.2f A\n", (float)tmp15/100);
    printf("Input2 Watt (average) = %d W\n", tmp16);
    printf("Bus Volt (Pos), (average) = %03.1f V\n", (float)tmp17/10);
    printf("Bus Volt (Neg), (average) = %03.1f V\n", (float)tmp18/10);
    printf("AC Volt (average) = %03.1f V\n", (float)tmp19/10);
    printf("AC Curr (average) = %04.2f A\n", (float)tmp20/100);
    printf("AC Watt (average) = %d W\n", tmp21);
    printf("AC Freq (average) = %04.2f Hz\n", (float)tmp22/100);
    printf("AC Volt (redundant) (average) = %03.1f V\n", (float)tmp23/10);
    printf("AC Freq (redundant) (average) = %04.2f Hz\n", (float)tmp24/100);
    printf("Temperature (Ambient)  = %d K\n", tmp25);
    printf("Temperature (Heatsink) = %d K\n", tmp26);*/
    printf("Input1 Volt     = %03.1f V\n", (float)m_cp_1ppi.Input1_Volt/10);
    printf("Input1 Curr     = %04.2f A\n", (float)m_cp_1ppi.Input1_Curr/100);
    printf("Input1 Watt     = %d W\n", m_cp_1ppi.Input1_Watt);
    printf("Input2 Volt     = %03.1f V\n", (float)m_cp_1ppi.Input2_Volt/10);
    printf("Input2 Curr     = %04.2f A\n", (float)m_cp_1ppi.Input2_Curr/100);
    printf("Input2 Watt     = %d W\n", m_cp_1ppi.Input2_Watt);
    //printf("Bus Volt (Pos) = %03.1f V\n", (float)tmp27/10);
    //printf("Bus Volt (Neg) = %03.1f V\n", (float)tmp28/10);
    printf("AC Volt         = %03.1f V\n", (float)m_cp_1ppi.AC_Volt/10);
    printf("AC Curr         = %04.2f A\n", (float)m_cp_1ppi.AC_Curr/100);
    printf("AC Watt         = %d W\n", m_cp_1ppi.AC_Watt);
    printf("AC Freq         = %04.2f Hz\n", (float)m_cp_1ppi.AC_Freq/100);
    //printf("AC Volt (redundant) = %03.1f V\n", (float)tmp29/10);
    //printf("AC Freq (redundant) = %04.2f Hz\n", (float)tmp30/100);
    printf("######################################\n");

    return;
}

bool CyberPower::GetErrorCode()
{
    printf("#### GetErrorCode start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    unsigned char cmd_buf[]={0x00, 0x04, 0x10, 0xC6, 0x00, 0x08, 0x00, 0x00};
    cmd_buf[0] = m_devid;
    MakeReadDataCRC(cmd_buf,8);

    MClearRX();
    txsize = 8;
    waitAddr = m_devid;
    waitFCode = 0x04;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd_buf, 8);
        MStartTX(m_busfd);
        usleep(m_dl_config.m_delay_time_1);

        lpdata = GetCyberPowerRespond(m_busfd, 21, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### GetErrorCode OK ####\n");
            SaveLog((char *)"CyberPower GetErrorCode() : OK", m_st_time);
            DumpErrorCode(lpdata+3);
            ParserErrorCode();
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetErrorCode CRC Error ####\n");
                SaveLog((char *)"CyberPower GetErrorCode() : CRC Error", m_st_time);
            }
            else {
                printf("#### GetErrorCode No Response ####\n");
                SaveLog((char *)"CyberPower GetErrorCode() : No Response", m_st_time);
            }
            err++;
        }
    }

    return false;
}

void CyberPower::DumpErrorCode(unsigned char *buf)
{
    // I_Event_1_Vendor (E00 ~ E15)
    m_cp_ec.E1_E00_15 = (*(buf) << 8) + *(buf+1);
    // I_Event_2_Vendor (E16 ~ E31)
    m_cp_ec.E2_E16_31 = (*(buf+2) << 8) + *(buf+3);
    // I_Event_3_Vendor (W00 ~ W15)
    m_cp_ec.E3_W00_15 = (*(buf+4) << 8) + *(buf+5);
    // I_Event_4_Vendor (F00 ~ F15)
    m_cp_ec.E4_F00_15 = (*(buf+6) << 8) + *(buf+7);
    // I_Event_5_Vendor (F16 ~ F31)
    m_cp_ec.E5_F16_31 = (*(buf+8) << 8) + *(buf+9);
    // I_Event_6_Vendor (F32 ~ F47)
    m_cp_ec.E6_F32_47 = (*(buf+10) << 8) + *(buf+11);
    // I_Event_7_Vendor (F48 ~ F63)
    m_cp_ec.E7_F48_63 = (*(buf+12) << 8) + *(buf+13);
    // I_Event_8_Vendor (F64 ~ F79)
    m_cp_ec.E8_F64_79 = (*(buf+14) << 8) + *(buf+15);

    printf("##### Dump CyberPower Error Code #####\n");
    printf("I_Event_1_Vendor = %04X\n", m_cp_ec.E1_E00_15);
    printf("I_Event_2_Vendor = %04X\n", m_cp_ec.E2_E16_31);
    printf("I_Event_3_Vendor = %04X\n", m_cp_ec.E3_W00_15);
    printf("I_Event_4_Vendor = %04X\n", m_cp_ec.E4_F00_15);
    printf("I_Event_5_Vendor = %04X\n", m_cp_ec.E5_F16_31);
    printf("I_Event_6_Vendor = %04X\n", m_cp_ec.E6_F32_47);
    printf("I_Event_7_Vendor = %04X\n", m_cp_ec.E7_F48_63);
    printf("I_Event_8_Vendor = %04X\n", m_cp_ec.E8_F64_79);
    printf("######################################\n");

    return;
}

void CyberPower::ParserErrorCode()
{
    int tmp;
    // parser I_Event_1_Vendor (E00 ~ E15)
    tmp = m_cp_ec.E1_E00_15;
    m_cp_ec_e.E00_No_Grid = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_e.E01_AC_Over_Frequency_A = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_e.E02_AC_Under_Frequency_A = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_e.E03_AC_Over_Volt_A = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_e.E04_AC_Under_Volt_A = tmp & 0x0001;
    tmp>>=5;
    m_cp_ec_e.E09_Grid_Quality = tmp & 0x0001;
    tmp>>=2;
    m_cp_ec_e.E11_AC_Over_Frequency_B = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_e.E12_AC_Under_Frequency_B = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_e.E13_AC_Over_Volt_B = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_e.E14_AC_Under_Volt_B = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_e.E15_AC_Over_Frequency_C = tmp & 0x0001;

    // parser I_Event_2_Vendor (E16 ~ E31)
    tmp = m_cp_ec.E2_E16_31;
    m_cp_ec_e.E16_AC_Under_Frequency_C = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_e.E17_AC_Over_Volt_C = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_e.E18_AC_Under_Volt_C = tmp & 0x0001;

    // parser I_Event_3_Vendor (W00 ~ W15)
    tmp = m_cp_ec.E3_W00_15;
    m_cp_ec_w.W00_Input1_Under_Volt = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_w.W01_Input2_Under_Volt = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_w.W02_Input3_Under_Volt = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_w.W03_Input4_Under_Volt = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_w.W04_Input1_Over_Volt = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_w.W05_Input2_Over_Volt = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_w.W06_Input3_Over_Volt = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_w.W07_Input4_Over_Volt = tmp & 0x0001;
    tmp>>=3;
    m_cp_ec_w.W10_HW_Fan = tmp & 0x0001;

    // parser I_Event_4_Vendor (F00 ~ F15)
    tmp = m_cp_ec.E4_F00_15;
    tmp>>=1;
    m_cp_ec_f.F01_Component_above_limit_A = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F02_Component_above_limit_B = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F03_Component_above_limit_C = tmp & 0x0001;
    tmp>>=2;
    m_cp_ec_f.F05_Under_Temp = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F06_NTC1_Over_Temp = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F07_HW_NTC1_Failure = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F08_NTC2_Over_Temp = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F09_HW_NTC2_Failure = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F10_NTC3_Over_Temp = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F11_HW_NTC3_Failure = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F12_NTC4_Over_Temp = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F13_HW_NTC4_Failure = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F14_Firmware_are_not_compatible = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F15_HW_DSP_ADC1 = tmp & 0x0001;

    // parser I_Event_5_Vendor (F16 ~ F31)
    tmp = m_cp_ec.E5_F16_31;
    m_cp_ec_f.F16_HW_DSP_ADC2 = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F17_HW_DSP_ADC3 = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F18_HW_DSP_ADC4 = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F19_HW_Red_ADC1 = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F20_HW_Efficiency = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F21_HW_FAN = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F22_HW_COMM1 = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F23_HW_COMM2 = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F24_Ground_Current = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F25_RCMU_Fail = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F26_Insulation = tmp & 0x0001;
    tmp>>=2;
    m_cp_ec_f.F28_HW_Relay_Short = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F29_HW_Relay_Open = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F30_Bus_Unbalance = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F31_HW_Bus1_OVR = tmp & 0x0001;

    // parser I_Event_6_Vendor (F32 ~ F47)
    tmp = m_cp_ec.E6_F32_47;
    m_cp_ec_f.F32_HW_Bus1_UVR = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F33_HW_Bus2_OVR = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F34_HW_Bus2_UVR = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F35_HW_Bus_OVR = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F36_AC_Over_Current_A_transient = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F37_AC_Over_Current_A = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F38_AC_Over_Current_B_transient = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F39_AC_Over_Current_B = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F40_AC_Over_Current_C_transient = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F41_AC_Over_Current_C = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F42_HW_CT_Fail_A = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F43_HW_CT_Fail_B = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F44_HW_CT_Fail_C = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F45_HW_AC_OCR = tmp & 0x0001;

    // parser I_Event_7_Vendor (F48 ~ F63)
    tmp = m_cp_ec.E7_F48_63;
    tmp>>=12;
    m_cp_ec_f.F60_Input1_Over_Current = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F61_Input2_Over_Current = tmp & 0x0001;

    // parser I_Event_8_Vendor (F64 ~ F79)
    tmp = m_cp_ec.E8_F64_79;
    tmp>>=6;
    m_cp_ec_f.F70_Input1_Over_Current_transient = tmp & 0x0001;
    tmp>>=1;
    m_cp_ec_f.F71_Input2_Over_Current_transient = tmp & 0x0001;

    printf("#### Parser CyberPower Error Code ####\n");
    // printf E code
    if ( m_cp_ec_e.E00_No_Grid )
        printf("E00_No_Grid = %d\n", m_cp_ec_e.E00_No_Grid);
    if ( m_cp_ec_e.E01_AC_Over_Frequency_A )
        printf("E01_AC_Over_Frequency_A = %d\n", m_cp_ec_e.E01_AC_Over_Frequency_A);
    if ( m_cp_ec_e.E02_AC_Under_Frequency_A )
        printf("E02_AC_Under_Frequency_A = %d\n", m_cp_ec_e.E02_AC_Under_Frequency_A);
    if ( m_cp_ec_e.E03_AC_Over_Volt_A )
        printf("E03_AC_Over_Volt_A = %d\n", m_cp_ec_e.E03_AC_Over_Volt_A);
    if ( m_cp_ec_e.E04_AC_Under_Volt_A )
        printf("E04_AC_Under_Volt_A = %d\n", m_cp_ec_e.E04_AC_Under_Volt_A);
    if ( m_cp_ec_e.E09_Grid_Quality )
        printf("E09_Grid_Quality = %d\n", m_cp_ec_e.E09_Grid_Quality);
    if ( m_cp_ec_e.E11_AC_Over_Frequency_B )
        printf("E11_AC_Over_Frequency_B = %d\n", m_cp_ec_e.E11_AC_Over_Frequency_B);
    if ( m_cp_ec_e.E12_AC_Under_Frequency_B )
        printf("E12_AC_Under_Frequency_B = %d\n", m_cp_ec_e.E12_AC_Under_Frequency_B);
    if ( m_cp_ec_e.E13_AC_Over_Volt_B )
        printf("E13_AC_Over_Volt_B = %d\n", m_cp_ec_e.E13_AC_Over_Volt_B);
    if ( m_cp_ec_e.E14_AC_Under_Volt_B )
        printf("E14_AC_Under_Volt_B = %d\n", m_cp_ec_e.E14_AC_Under_Volt_B);
    if ( m_cp_ec_e.E15_AC_Over_Frequency_C )
        printf("E15_AC_Over_Frequency_C = %d\n", m_cp_ec_e.E15_AC_Over_Frequency_C);
    if ( m_cp_ec_e.E16_AC_Under_Frequency_C )
        printf("E16_AC_Under_Frequency_C = %d\n", m_cp_ec_e.E16_AC_Under_Frequency_C);
    if ( m_cp_ec_e.E17_AC_Over_Volt_C )
        printf("E17_AC_Over_Volt_C = %d\n", m_cp_ec_e.E17_AC_Over_Volt_C);
    if ( m_cp_ec_e.E18_AC_Under_Volt_C )
        printf("E18_AC_Under_Volt_C = %d\n", m_cp_ec_e.E18_AC_Under_Volt_C);
    // printf W code
    if ( m_cp_ec_w.W00_Input1_Under_Volt )
        printf("W00_Input1_Under_Volt = %d\n", m_cp_ec_w.W00_Input1_Under_Volt);
    if ( m_cp_ec_w.W01_Input2_Under_Volt )
        printf("W01_Input2_Under_Volt = %d\n", m_cp_ec_w.W01_Input2_Under_Volt);
    if ( m_cp_ec_w.W02_Input3_Under_Volt )
        printf("W02_Input3_Under_Volt = %d\n", m_cp_ec_w.W02_Input3_Under_Volt);
    if ( m_cp_ec_w.W03_Input4_Under_Volt )
        printf("W03_Input4_Under_Volt = %d\n", m_cp_ec_w.W03_Input4_Under_Volt);
    if ( m_cp_ec_w.W04_Input1_Over_Volt )
        printf("W04_Input1_Over_Volt = %d\n", m_cp_ec_w.W04_Input1_Over_Volt);
    if ( m_cp_ec_w.W05_Input2_Over_Volt )
        printf("W05_Input2_Over_Volt = %d\n", m_cp_ec_w.W05_Input2_Over_Volt);
    if ( m_cp_ec_w.W06_Input3_Over_Volt )
        printf("W06_Input3_Over_Volt = %d\n", m_cp_ec_w.W06_Input3_Over_Volt);
    if ( m_cp_ec_w.W07_Input4_Over_Volt )
        printf("W07_Input4_Over_Volt = %d\n", m_cp_ec_w.W07_Input4_Over_Volt);
    if ( m_cp_ec_w.W10_HW_Fan )
        printf("W10_HW_Fan = %d\n", m_cp_ec_w.W10_HW_Fan);
    // printf F code
    if ( m_cp_ec_f.F01_Component_above_limit_A )
        printf("F01_Component_above_limit_A = %d\n", m_cp_ec_f.F01_Component_above_limit_A);
    if ( m_cp_ec_f.F02_Component_above_limit_B )
        printf("F02_Component_above_limit_B = %d\n", m_cp_ec_f.F02_Component_above_limit_B);
    if ( m_cp_ec_f.F03_Component_above_limit_C )
        printf("F03_Component_above_limit_C = %d\n", m_cp_ec_f.F03_Component_above_limit_C);
    if ( m_cp_ec_f.F05_Under_Temp )
        printf("F05_Under_Temp = %d\n", m_cp_ec_f.F05_Under_Temp);
    if ( m_cp_ec_f.F06_NTC1_Over_Temp )
        printf("F06_NTC1_Over_Temp = %d\n", m_cp_ec_f.F06_NTC1_Over_Temp);
    if ( m_cp_ec_f.F07_HW_NTC1_Failure )
        printf("F07_HW_NTC1_Failure = %d\n", m_cp_ec_f.F07_HW_NTC1_Failure);
    if ( m_cp_ec_f.F08_NTC2_Over_Temp )
        printf("F08_NTC2_Over_Temp = %d\n", m_cp_ec_f.F08_NTC2_Over_Temp);
    if ( m_cp_ec_f.F09_HW_NTC2_Failure )
        printf("F09_HW_NTC2_Failure = %d\n", m_cp_ec_f.F09_HW_NTC2_Failure);
    if ( m_cp_ec_f.F10_NTC3_Over_Temp )
        printf("F10_NTC3_Over_Temp = %d\n", m_cp_ec_f.F10_NTC3_Over_Temp);
    if ( m_cp_ec_f.F11_HW_NTC3_Failure )
        printf("F11_HW_NTC3_Failure = %d\n", m_cp_ec_f.F11_HW_NTC3_Failure);
    if ( m_cp_ec_f.F12_NTC4_Over_Temp )
        printf("F12_NTC4_Over_Temp = %d\n", m_cp_ec_f.F12_NTC4_Over_Temp);
    if ( m_cp_ec_f.F13_HW_NTC4_Failure )
        printf("F13_HW_NTC4_Failure = %d\n", m_cp_ec_f.F13_HW_NTC4_Failure);
    if ( m_cp_ec_f.F14_Firmware_are_not_compatible )
        printf("F14_Firmware_are_not_compatible = %d\n", m_cp_ec_f.F14_Firmware_are_not_compatible);
    if ( m_cp_ec_f.F15_HW_DSP_ADC1 )
        printf("F15_HW_DSP_ADC1 = %d\n", m_cp_ec_f.F15_HW_DSP_ADC1);
    if ( m_cp_ec_f.F16_HW_DSP_ADC2 )
        printf("F16_HW_DSP_ADC2 = %d\n", m_cp_ec_f.F16_HW_DSP_ADC2);
    if ( m_cp_ec_f.F17_HW_DSP_ADC3 )
        printf("F17_HW_DSP_ADC3 = %d\n", m_cp_ec_f.F17_HW_DSP_ADC3);
    if ( m_cp_ec_f.F18_HW_DSP_ADC4 )
        printf("F18_HW_DSP_ADC4 = %d\n", m_cp_ec_f.F18_HW_DSP_ADC4);
    if ( m_cp_ec_f.F19_HW_Red_ADC1 )
        printf("F19_HW_Red_ADC1 = %d\n", m_cp_ec_f.F19_HW_Red_ADC1);
    if ( m_cp_ec_f.F20_HW_Efficiency )
        printf("F20_HW_Efficiency = %d\n", m_cp_ec_f.F20_HW_Efficiency);
    if ( m_cp_ec_f.F21_HW_FAN )
        printf("F21_HW_FAN = %d\n", m_cp_ec_f.F21_HW_FAN);
    if ( m_cp_ec_f.F22_HW_COMM1 )
        printf("F22_HW_COMM1 = %d\n", m_cp_ec_f.F22_HW_COMM1);
    if ( m_cp_ec_f.F23_HW_COMM2 )
        printf("F23_HW_COMM2 = %d\n", m_cp_ec_f.F23_HW_COMM2);
    if ( m_cp_ec_f.F24_Ground_Current )
        printf("F24_Ground_Current = %d\n", m_cp_ec_f.F24_Ground_Current);
    if ( m_cp_ec_f.F25_RCMU_Fail )
        printf("F25_RCMU_Fail = %d\n", m_cp_ec_f.F25_RCMU_Fail);
    if ( m_cp_ec_f.F26_Insulation )
        printf("F26_Insulation = %d\n", m_cp_ec_f.F26_Insulation);
    if ( m_cp_ec_f.F28_HW_Relay_Short )
        printf("F28_HW_Relay_Short = %d\n", m_cp_ec_f.F28_HW_Relay_Short);
    if ( m_cp_ec_f.F29_HW_Relay_Open )
        printf("F29_HW_Relay_Open = %d\n", m_cp_ec_f.F29_HW_Relay_Open);
    if ( m_cp_ec_f.F30_Bus_Unbalance )
        printf("F30_Bus_Unbalance = %d\n", m_cp_ec_f.F30_Bus_Unbalance);
    if ( m_cp_ec_f.F31_HW_Bus1_OVR )
        printf("F31_HW_Bus1_OVR = %d\n", m_cp_ec_f.F31_HW_Bus1_OVR);
    if ( m_cp_ec_f.F32_HW_Bus1_UVR )
        printf("F32_HW_Bus1_UVR = %d\n", m_cp_ec_f.F32_HW_Bus1_UVR);
    if ( m_cp_ec_f.F33_HW_Bus2_OVR )
        printf("F33_HW_Bus2_OVR = %d\n", m_cp_ec_f.F33_HW_Bus2_OVR);
    if ( m_cp_ec_f.F34_HW_Bus2_UVR )
        printf("F34_HW_Bus2_UVR = %d\n", m_cp_ec_f.F34_HW_Bus2_UVR);
    if ( m_cp_ec_f.F35_HW_Bus_OVR )
        printf("F35_HW_Bus_OVR = %d\n", m_cp_ec_f.F35_HW_Bus_OVR);
    if ( m_cp_ec_f.F36_AC_Over_Current_A_transient )
        printf("F36_AC_Over_Current_A_transient = %d\n", m_cp_ec_f.F36_AC_Over_Current_A_transient);
    if ( m_cp_ec_f.F37_AC_Over_Current_A )
        printf("F37_AC_Over_Current_A = %d\n", m_cp_ec_f.F37_AC_Over_Current_A);
    if ( m_cp_ec_f.F38_AC_Over_Current_B_transient )
        printf("F38_AC_Over_Current_B_transient = %d\n", m_cp_ec_f.F38_AC_Over_Current_B_transient);
    if ( m_cp_ec_f.F39_AC_Over_Current_B )
        printf("F39_AC_Over_Current_B = %d\n", m_cp_ec_f.F39_AC_Over_Current_B);
    if ( m_cp_ec_f.F40_AC_Over_Current_C_transient )
        printf("F40_AC_Over_Current_C_transient = %d\n", m_cp_ec_f.F40_AC_Over_Current_C_transient);
    if ( m_cp_ec_f.F41_AC_Over_Current_C )
        printf("F41_AC_Over_Current_C = %d\n", m_cp_ec_f.F41_AC_Over_Current_C);
    if ( m_cp_ec_f.F42_HW_CT_Fail_A )
        printf("F42_HW_CT_Fail_A = %d\n", m_cp_ec_f.F42_HW_CT_Fail_A);
    if ( m_cp_ec_f.F43_HW_CT_Fail_B )
        printf("F43_HW_CT_Fail_B = %d\n", m_cp_ec_f.F43_HW_CT_Fail_B);
    if ( m_cp_ec_f.F44_HW_CT_Fail_C )
        printf("F44_HW_CT_Fail_C = %d\n", m_cp_ec_f.F44_HW_CT_Fail_C);
    if ( m_cp_ec_f.F45_HW_AC_OCR )
        printf("F45_HW_AC_OCR = %d\n", m_cp_ec_f.F45_HW_AC_OCR);
    if ( m_cp_ec_f.F60_Input1_Over_Current )
        printf("F60_Input1_Over_Current = %d\n", m_cp_ec_f.F60_Input1_Over_Current);
    if ( m_cp_ec_f.F61_Input2_Over_Current )
        printf("F61_Input2_Over_Current = %d\n", m_cp_ec_f.F61_Input2_Over_Current);
    if ( m_cp_ec_f.F70_Input1_Over_Current_transient )
        printf("F70_Input1_Over_Current_transient = %d\n", m_cp_ec_f.F70_Input1_Over_Current_transient);
    if ( m_cp_ec_f.F71_Input2_Over_Current_transient )
        printf("F71_Input2_Over_Current_transient = %d\n", m_cp_ec_f.F71_Input2_Over_Current_transient);
    printf("######################################\n");

    return;
}

void CyberPower::SetLogXML()
{
    sprintf(m_log_filename, "%s/%02d%02d", m_dl_path.m_log_path, m_st_time->tm_hour, m_st_time->tm_min);
    printf("log path = %s\n", m_log_filename);
    return;
}

bool CyberPower::WriteLogXML()
{
    char buf[256] = {0};
    int error_tmp = 0;

    SaveLog((char *)"CyberPower WriteLogXML() : run", m_st_time);
    printf("==================== Set Log XML start ====================\n");
    //if ( first && (strlen(m_log_buf) == 0) ) // empty, new file, add header <records>
    //    strcpy(m_log_buf, "<records>");

    sprintf(buf, "<record dev_id=\"%d\" date=\"%04d-%02d-%02d %02d:%02d:00\" sn=\"%s\">", m_devid,
            1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday,
            m_st_time->tm_hour, m_st_time->tm_min, m_cp_sn.SN);
    strcat(m_log_buf, buf);

    if ( m_loopflag == 0 ) {
        sprintf(buf, "<daily_KWH>%05.3f</daily_KWH>", ((float)m_cp_1ppi.Today_KWH)/1000);
        strcat(m_log_buf, buf);

        sprintf(buf, "<total_KWH>%05.3f</total_KWH>", ((float)m_cp_1ppi.System_Life_KWH)/1000);
        strcat(m_log_buf, buf);

        sprintf(buf, "<dcv_1>%03.1f</dcv_1>", ((float)m_cp_1ppi.Input1_Volt)/10);
        strcat(m_log_buf, buf);
        sprintf(buf, "<dc_voltage>%03.1f</dc_voltage>", ((float)m_cp_1ppi.Input1_Volt)/10);
        strcat(m_log_buf, buf);

        sprintf(buf, "<dci_1>%04.2f</dci_1>", ((float)m_cp_1ppi.Input1_Curr)/100);
        strcat(m_log_buf, buf);
        sprintf(buf, "<dc_current>%04.2f</dc_current>", ((float)m_cp_1ppi.Input1_Curr)/100);
        strcat(m_log_buf, buf);

        sprintf(buf, "<dc_power_1>%05.3f</dc_power_1>", ((float)m_cp_1ppi.Input1_Watt)/1000);
        strcat(m_log_buf, buf);
        sprintf(buf, "<dc_power>%05.3f</dc_power>", ((float)m_cp_1ppi.Input1_Watt)/1000);
        strcat(m_log_buf, buf);

        sprintf(buf, "<acv_AN>%03.1f</acv_AN>", ((float)m_cp_1ppi.AC_Volt)/10);
        strcat(m_log_buf, buf);
        sprintf(buf, "<ac_voltage>%03.1f</ac_voltage>", ((float)m_cp_1ppi.AC_Volt)/10);
        strcat(m_log_buf, buf);

        sprintf(buf, "<aci_A>%04.2f</aci_A>", ((float)m_cp_1ppi.AC_Curr)/100);
        strcat(m_log_buf, buf);
        sprintf(buf, "<ac_current>%04.2f</ac_current>", ((float)m_cp_1ppi.AC_Curr)/100);
        strcat(m_log_buf, buf);

        sprintf(buf, "<ac_power_A>%05.3f</ac_power_A>", ((float)m_cp_1ppi.AC_Watt)/1000);
        strcat(m_log_buf, buf);
        sprintf(buf, "<ac_power>%05.3f</ac_power>", ((float)m_cp_1ppi.AC_Watt)/1000);
        strcat(m_log_buf, buf);

        sprintf(buf, "<frequency>%04.2f</frequency>", ((float)m_cp_1ppi.AC_Freq)/100);
        strcat(m_log_buf, buf);

        // set error code
        if ( m_cp_ec.E1_E00_15 )
            error_tmp = m_cp_ec.E1_E00_15;
        else if ( m_cp_ec.E2_E16_31 )
            error_tmp = m_cp_ec.E2_E16_31;
        else if ( m_cp_ec.E3_W00_15 )
            error_tmp = m_cp_ec.E3_W00_15;
        else if ( m_cp_ec.E4_F00_15 )
            error_tmp = m_cp_ec.E4_F00_15;
        else if ( m_cp_ec.E5_F16_31 )
            error_tmp = m_cp_ec.E5_F16_31;
        else if ( m_cp_ec.E6_F32_47 )
            error_tmp = m_cp_ec.E6_F32_47;
        else if ( m_cp_ec.E7_F48_63 )
            error_tmp = m_cp_ec.E7_F48_63;
        else if ( m_cp_ec.E8_F64_79 )
            error_tmp = m_cp_ec.E8_F64_79;
        else
            error_tmp = 0;

        sprintf(buf, "<Error_Code>%d</Error_Code>", error_tmp);
        strcat(m_log_buf, buf);
    }

    // set status
    if ( error_tmp ) {
        strcat(m_log_buf, "<Status>2</Status>");
    } else {
        if ( m_loopflag == 3 ) {
            strcat(m_log_buf, "<Status>1</Status>");
        } else {
            strcat(m_log_buf, "<Status>0</Status>");
        }
    }

    strcat(m_log_buf, "</record>");
    //printf("m_log_buf = \n%s\n", m_log_buf);
    printf("===================== Set Log XML end =====================\n");

    return true;
}

bool CyberPower::SaveLogXML(bool first, bool last)
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
        SaveLog((char *)"CyberPower SaveLogXML() : open /tmp/tmplog Fail", m_st_time);
        printf("open /tmp/tmplog Fail!\n");
        return false;
    }

    if ( !last )
        return true;
    else {
        if ( filesize < 42 ) {
            printf("==== file size = %d, too small, don't copy file to storage ====\n", filesize);
            return true;
        }
    }

    // copy file to target
    sprintf(buf, "cp /tmp/tmplog %s", m_log_filename);
    ret = system(buf);
    if ( ret == 0 ) {
        sprintf(buf, "CyberPower SaveLogXML() : write %s OK", m_log_filename);
        SaveLog(buf, m_st_time);
        if ( strstr(m_log_filename, USB_PATH) )
            m_sys_error  &= ~SYS_0002_Save_USB_Fail;
        return true;
    } else {
        sprintf(buf, "CyberPower SaveLogXML() : write %s Fail", m_log_filename);
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
            SaveLog((char *)"CyberPower SaveLogXML() : write to tmp OK", m_st_time);
            return true;
        } else {
            SaveLog((char *)"CyberPower SaveLogXML() : write to tmp Fail", m_st_time);
            return false;
        }
    }

    return false;
}

void CyberPower::SetErrorLogXML()
{
    sprintf(m_errlog_filename, "%s/%02d%02d", m_dl_path.m_errlog_path, m_st_time->tm_hour, m_st_time->tm_min);
    printf("errlog path = %s\n", m_errlog_filename);
    return;
}

bool CyberPower::WriteErrorLogXML()
{
    char buf[256] = {0};

    SaveLog((char *)"CyberPower WriteErrorLogXML() : run", m_st_time);
    printf("==================== Set Error Log XML start ====================\n");
    //if ( first && (strlen(m_errlog_buf) == 0) ) // empty, new file, add header <records>
    //    strcpy(m_errlog_buf, "<records>");

    sprintf(buf, "<record dev_id=\"%d\" date=\"%04d-%02d-%02d %02d:%02d:00\" sn=\"%s\">", m_devid,
            1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday,
            m_st_time->tm_hour, m_st_time->tm_min, m_cp_sn.SN);
    strcat(m_errlog_buf, buf);

    if ( m_cp_ec.E1_E00_15 ) {
        // E code E00 ~ E15
        if ( m_cp_ec_e.E00_No_Grid )
            strcat(m_errlog_buf, "<code>CP_E00</code>");
        if ( m_cp_ec_e.E01_AC_Over_Frequency_A )
            strcat(m_errlog_buf, "<code>CP_E01</code>");
        if ( m_cp_ec_e.E02_AC_Under_Frequency_A )
            strcat(m_errlog_buf, "<code>CP_E02</code>");
        if ( m_cp_ec_e.E03_AC_Over_Volt_A )
            strcat(m_errlog_buf, "<code>CP_E03</code>");
        if ( m_cp_ec_e.E04_AC_Under_Volt_A )
            strcat(m_errlog_buf, "<code>CP_E04</code>");
        if ( m_cp_ec_e.E09_Grid_Quality )
            strcat(m_errlog_buf, "<code>CP_E09</code>");
        if ( m_cp_ec_e.E11_AC_Over_Frequency_B )
            strcat(m_errlog_buf, "<code>CP_E11</code>");
        if ( m_cp_ec_e.E12_AC_Under_Frequency_B )
            strcat(m_errlog_buf, "<code>CP_E12</code>");
        if ( m_cp_ec_e.E13_AC_Over_Volt_B )
            strcat(m_errlog_buf, "<code>CP_E13</code>");
        if ( m_cp_ec_e.E14_AC_Under_Volt_B )
            strcat(m_errlog_buf, "<code>CP_E14</code>");
        if ( m_cp_ec_e.E15_AC_Over_Frequency_C )
            strcat(m_errlog_buf, "<code>CP_E15</code>");
    }

    if ( m_cp_ec.E2_E16_31 ) {
        // E code E16 ~ E31
        if ( m_cp_ec_e.E16_AC_Under_Frequency_C )
            strcat(m_errlog_buf, "<code>CP_E16</code>");
        if ( m_cp_ec_e.E17_AC_Over_Volt_C )
            strcat(m_errlog_buf, "<code>CP_E17</code>");
        if ( m_cp_ec_e.E18_AC_Under_Volt_C )
            strcat(m_errlog_buf, "<code>CP_E18</code>");
    }

    if ( m_cp_ec.E3_W00_15 ) {
        // W code W00 ~ W15
        if ( m_cp_ec_w.W00_Input1_Under_Volt )
            strcat(m_errlog_buf, "<code>CP_W00</code>");
        if ( m_cp_ec_w.W01_Input2_Under_Volt )
            strcat(m_errlog_buf, "<code>CP_W01</code>");
        if ( m_cp_ec_w.W02_Input3_Under_Volt )
            strcat(m_errlog_buf, "<code>CP_W02</code>");
        if ( m_cp_ec_w.W03_Input4_Under_Volt )
            strcat(m_errlog_buf, "<code>CP_W03</code>");
        if ( m_cp_ec_w.W04_Input1_Over_Volt )
            strcat(m_errlog_buf, "<code>CP_W04</code>");
        if ( m_cp_ec_w.W05_Input2_Over_Volt )
            strcat(m_errlog_buf, "<code>CP_W05</code>");
        if ( m_cp_ec_w.W06_Input3_Over_Volt )
            strcat(m_errlog_buf, "<code>CP_W06</code>");
        if ( m_cp_ec_w.W07_Input4_Over_Volt )
            strcat(m_errlog_buf, "<code>CP_W07</code>");
        if ( m_cp_ec_w.W10_HW_Fan )
            strcat(m_errlog_buf, "<code>CP_W10</code>");
    }

    if ( m_cp_ec.E4_F00_15 ) {
        // F code F00 ~ F15
        if ( m_cp_ec_f.F01_Component_above_limit_A )
            strcat(m_errlog_buf, "<code>CP_F01</code>");
        if ( m_cp_ec_f.F02_Component_above_limit_B )
            strcat(m_errlog_buf, "<code>CP_F02</code>");
        if ( m_cp_ec_f.F03_Component_above_limit_C )
            strcat(m_errlog_buf, "<code>CP_F03</code>");
        if ( m_cp_ec_f.F05_Under_Temp )
            strcat(m_errlog_buf, "<code>CP_F05</code>");
        if ( m_cp_ec_f.F06_NTC1_Over_Temp )
            strcat(m_errlog_buf, "<code>CP_F06</code>");
        if ( m_cp_ec_f.F07_HW_NTC1_Failure )
            strcat(m_errlog_buf, "<code>CP_F07</code>");
        if ( m_cp_ec_f.F08_NTC2_Over_Temp )
            strcat(m_errlog_buf, "<code>CP_F08</code>");
        if ( m_cp_ec_f.F09_HW_NTC2_Failure )
            strcat(m_errlog_buf, "<code>CP_F09</code>");
        if ( m_cp_ec_f.F10_NTC3_Over_Temp )
            strcat(m_errlog_buf, "<code>CP_F10</code>");
        if ( m_cp_ec_f.F11_HW_NTC3_Failure )
            strcat(m_errlog_buf, "<code>CP_F11</code>");
        if ( m_cp_ec_f.F12_NTC4_Over_Temp )
            strcat(m_errlog_buf, "<code>CP_F12</code>");
        if ( m_cp_ec_f.F13_HW_NTC4_Failure )
            strcat(m_errlog_buf, "<code>CP_F13</code>");
        if ( m_cp_ec_f.F14_Firmware_are_not_compatible )
            strcat(m_errlog_buf, "<code>CP_F14</code>");
        if ( m_cp_ec_f.F15_HW_DSP_ADC1 )
            strcat(m_errlog_buf, "<code>CP_F15</code>");
    }

    if ( m_cp_ec.E5_F16_31 ) {
        // F code F16 ~ F31
        if ( m_cp_ec_f.F16_HW_DSP_ADC2 )
            strcat(m_errlog_buf, "<code>CP_F16</code>");
        if ( m_cp_ec_f.F17_HW_DSP_ADC3 )
            strcat(m_errlog_buf, "<code>CP_F17</code>");
        if ( m_cp_ec_f.F18_HW_DSP_ADC4 )
            strcat(m_errlog_buf, "<code>CP_F18</code>");
        if ( m_cp_ec_f.F19_HW_Red_ADC1 )
            strcat(m_errlog_buf, "<code>CP_F19</code>");
        if ( m_cp_ec_f.F20_HW_Efficiency )
            strcat(m_errlog_buf, "<code>CP_F20</code>");
        if ( m_cp_ec_f.F21_HW_FAN )
            strcat(m_errlog_buf, "<code>CP_F21</code>");
        if ( m_cp_ec_f.F22_HW_COMM1 )
            strcat(m_errlog_buf, "<code>CP_F22</code>");
        if ( m_cp_ec_f.F23_HW_COMM2 )
            strcat(m_errlog_buf, "<code>CP_F23</code>");
        if ( m_cp_ec_f.F24_Ground_Current )
            strcat(m_errlog_buf, "<code>CP_F24</code>");
        if ( m_cp_ec_f.F25_RCMU_Fail )
            strcat(m_errlog_buf, "<code>CP_F25</code>");
        if ( m_cp_ec_f.F26_Insulation )
            strcat(m_errlog_buf, "<code>CP_F26</code>");
        if ( m_cp_ec_f.F28_HW_Relay_Short )
            strcat(m_errlog_buf, "<code>CP_F28</code>");
        if ( m_cp_ec_f.F29_HW_Relay_Open )
            strcat(m_errlog_buf, "<code>CP_F29</code>");
        if ( m_cp_ec_f.F30_Bus_Unbalance )
            strcat(m_errlog_buf, "<code>CP_F30</code>");
        if ( m_cp_ec_f.F31_HW_Bus1_OVR )
            strcat(m_errlog_buf, "<code>CP_F31</code>");
    }

    if ( m_cp_ec.E6_F32_47 ) {
        // F code F32 ~ F47
        if ( m_cp_ec_f.F32_HW_Bus1_UVR )
            strcat(m_errlog_buf, "<code>CP_F32</code>");
        if ( m_cp_ec_f.F33_HW_Bus2_OVR )
            strcat(m_errlog_buf, "<code>CP_F33</code>");
        if ( m_cp_ec_f.F34_HW_Bus2_UVR )
            strcat(m_errlog_buf, "<code>CP_F34</code>");
        if ( m_cp_ec_f.F35_HW_Bus_OVR )
            strcat(m_errlog_buf, "<code>CP_F35</code>");
        if ( m_cp_ec_f.F36_AC_Over_Current_A_transient )
            strcat(m_errlog_buf, "<code>CP_F36</code>");
        if ( m_cp_ec_f.F37_AC_Over_Current_A )
            strcat(m_errlog_buf, "<code>CP_F37</code>");
        if ( m_cp_ec_f.F38_AC_Over_Current_B_transient )
            strcat(m_errlog_buf, "<code>CP_F38</code>");
        if ( m_cp_ec_f.F39_AC_Over_Current_B )
            strcat(m_errlog_buf, "<code>CP_F39</code>");
        if ( m_cp_ec_f.F40_AC_Over_Current_C_transient )
            strcat(m_errlog_buf, "<code>CP_F40</code>");
        if ( m_cp_ec_f.F41_AC_Over_Current_C )
            strcat(m_errlog_buf, "<code>CP_F41</code>");
        if ( m_cp_ec_f.F42_HW_CT_Fail_A )
            strcat(m_errlog_buf, "<code>CP_F42</code>");
        if ( m_cp_ec_f.F43_HW_CT_Fail_B )
            strcat(m_errlog_buf, "<code>CP_F43</code>");
        if ( m_cp_ec_f.F44_HW_CT_Fail_C )
            strcat(m_errlog_buf, "<code>CP_F44</code>");
        if ( m_cp_ec_f.F45_HW_AC_OCR )
            strcat(m_errlog_buf, "<code>CP_F45</code>");
    }

    if ( m_cp_ec.E7_F48_63 ) {
        // F code F48 ~ F63
        if ( m_cp_ec_f.F60_Input1_Over_Current )
            strcat(m_errlog_buf, "<code>CP_F60</code>");
        if ( m_cp_ec_f.F61_Input2_Over_Current )
            strcat(m_errlog_buf, "<code>CP_F61</code>");
    }

    if ( m_cp_ec.E8_F64_79 ) {
        // F code F64 ~ F79
        if ( m_cp_ec_f.F70_Input1_Over_Current_transient )
            strcat(m_errlog_buf, "<code>CP_F70</code>");
        if ( m_cp_ec_f.F71_Input2_Over_Current_transient )
            strcat(m_errlog_buf, "<code>CP_F71</code>");
    }

    // set system error log
    if ( m_sys_error && (m_st_time->tm_hour%2 == 0) && (m_st_time->tm_min == 0) ) {
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
    //printf("m_errlog_buf = \n%s\n", m_errlog_buf);
    printf("===================== Set Error Log XML end =====================\n");

    return true;
}

bool CyberPower::SaveErrorLogXML(bool first, bool last)
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
        SaveLog((char *)"CyberPower SaveErrorLogXML() : open /tmp/tmperrlog Fail", m_st_time);
        printf("open /tmp/tmperrlog Fail!\n");
        return false;
    }

    if ( !last )
        return true;
    else {
        if ( filesize < 42 ) {
            printf("==== file size = %d, too small, don't copy file to storage ====\n", filesize);
            return true;
        }
    }

    // copy file to target
    sprintf(buf, "cp /tmp/tmperrlog %s", m_errlog_filename);
    ret = system(buf);
    if ( ret == 0 ) {
        sprintf(buf, "CyberPower SaveErrorLogXML() : write %s OK", m_errlog_filename);
        SaveLog(buf, m_st_time);
        if ( strstr(m_errlog_filename, USB_PATH) )
            m_sys_error  &= ~SYS_0002_Save_USB_Fail;
        return true;
    } else {
        sprintf(buf, "CyberPower SaveErrorLogXML() : write %s Fail", m_errlog_filename);
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
            SaveLog((char *)"CyberPower SaveErrorLogXML() : write to tmp OK", m_st_time);
            return true;
        } else {
            SaveLog((char *)"CyberPower SaveErrorLogXML() : write to tmp Fail", m_st_time);
            return false;
        }
    }

    return false;
}

void CyberPower::SetEnvXML()
{
    sprintf(m_env_filename, "%s/%02d%02d", m_dl_path.m_env_path, m_st_time->tm_hour, m_st_time->tm_min);
    printf("env path = %s\n", m_env_filename);
    return;
}

bool CyberPower::SaveEnvXML(bool first, bool last)
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
        SaveLog((char *)"CyberPower SaveEnvXML() : open /tmp/tmpenv Fail", m_st_time);
        printf("open /tmp/tmpenv Fail!\n");
        return false;
    }

    if ( !last )
        return true;
    else {
        if ( filesize < 42 ) {
            printf("==== file size = %d, too small, don't copy file to storage ====\n", filesize);
            return true;
        }
    }

    // copy file to target
    sprintf(buf, "cp /tmp/tmpenv %s", m_env_filename);
    ret = system(buf);
    if ( ret == 0 ) {
        sprintf(buf, "CyberPower SaveEnvXML() : write %s OK", m_env_filename);
        SaveLog(buf, m_st_time);
        //if ( strstr(m_env_filename, USB_PATH) )
        //    m_sys_error  &= ~SYS_0002_Save_USB_Fail;
        return true;
    } else {
        sprintf(buf, "CyberPower SaveEnvXML() : write %s Fail", m_env_filename);
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
            SaveLog((char *)"CyberPower SaveEnvXML() : write to tmp OK", m_st_time);
            return true;
        } else {
            SaveLog((char *)"CyberPower SaveEnvXML() : write to tmp Fail", m_st_time);
            return false;
        }
    }

    return false;
}

bool CyberPower::SaveDeviceList(bool first, bool last, int device)
{
    FILE *pFile;
    char buf[4096] = {0}, devbuf[32] = {0};
    int state = 0, offset = 0;
    char *index_tmp = NULL, *index_start = NULL, *index_end = NULL;

    printf("#### CyberPower SaveDeviceList Start ####\n");

    if ( first )
        pFile = fopen(DEVICELIST_TMP, "w");
    else
        pFile = fopen(DEVICELIST_TMP, "a");
    if ( pFile == NULL ) {
        printf("#### SaveDeviceList open file Fail ####\n");
        SaveLog((char *)"CyberPower SaveDeviceList() : fopen fail", m_st_time);
        return false;
    }

    if ( m_loopflag == 3 )
        state = 0; // off line
    else
        state = 1; // on line

    if ( device == 1 )
        strcpy(devbuf, "CyberPower-1P");
    else if ( device == 3 )
        strcpy(devbuf, "CyberPower-3P");
    else
        strcpy(devbuf, "CyberPower-Unknown"); // unknown

    // addr fixed 3 digit, state fixed 1 digit, sn
    sprintf(buf, "%03d <SN>%s</SN> <STATE>%d</STATE> <DEVICE>%s</DEVICE> ", m_addr, m_cp_sn.SN, state, devbuf);

    if (state) {
        // set log data
        index_tmp = strstr(m_log_buf, m_cp_sn.SN);
        if ( index_tmp != NULL ) {
            if ( index_tmp - m_log_buf > 80)
                index_start = strstr(index_tmp-80, "<record dev_id=");
            else
                index_start = strstr(m_log_buf, "<record dev_id=");

            index_end = strstr(index_start, "</record>");
            offset = index_end - index_start + 9;
            strcat(buf, "<ENERGY>");
            strncat(buf, index_start, offset);
            strcat(buf, "</ENERGY>");

        } else {
            printf("index_tmp = NULL!\n");
            strcat(buf, "Empty");
        }

        // set error log data
        index_tmp = strstr(m_errlog_buf, m_cp_sn.SN);
        if ( index_tmp != NULL ) {
            if ( index_tmp - m_errlog_buf > 80)
                index_start = strstr(index_tmp-80, "<record dev_id=");
            else
                index_start = strstr(m_errlog_buf, "<record dev_id=");

            index_end = strstr(index_start, "</record>");
            offset = index_end - index_start + 9;
            strcat(buf, "<ERROR>");
            strncat(buf, index_start, offset);
            strcat(buf, "</ERROR>");
        } else {
            printf("index_tmp = NULL!\n");
            strcat(buf, "Empty");
        }
    } else
        strcat(buf, "Empty Empty");

    fputs(buf, pFile);
    fputc('\n', pFile);

    fclose(pFile);

    if ( last ) {
        sprintf(buf, "cp -f %s %s", DEVICELIST_TMP, DEVICELIST_PATH);
        system(buf);
        system("sync");
    }

    printf("#### SaveDeviceList OK ####\n");
    SaveLog((char *)"CyberPower SaveDeviceList() : OK", m_st_time);

    return true;
}

bool CyberPower::WriteMIListXML(bool first, bool last, int device)
{
    char buf[256] = {0};
    char tmp[256] = {0};
    int listsize = 0;
    struct stat filest;
    FILE *pFile = NULL;

    //GetLocalTime(); // cancel, GetData set time already

    sprintf(buf, "/tmp/tmpMIList");

    if ( first )
        pFile = fopen(buf, "wb");
    else
        pFile = fopen(buf, "ab");
    if ( pFile == NULL ) {
        printf("open %s fail\n", buf);
        sprintf(tmp, "CyberPower WriteMIListXML() : fopen %s fail", buf);
        SaveLog(tmp, m_st_time);
        return false;
    }

    printf("==================== CyberPower Set MIList XML start ====================\n");
    if ( first )
        fputs("<records>\n", pFile);

    fputs("\t<record>\n", pFile);

    sprintf(buf, "\t\t<sn>%s</sn>\n", m_cp_sn.SN);
    //printf("%s", buf);
    fputs(buf, pFile);

    sprintf(buf, "\t\t<OriSn>%s</OriSn>\n", m_cp_sn.SN);
    //printf("%s", buf);
    fputs(buf, pFile);

    sprintf(buf, "\t\t<port>COM%d</port>\n", m_dl_config.m_inverter_port);
    //printf("%s", buf);
    fputs(buf, pFile);

    sprintf(buf, "\t\t<slaveId>%d</slaveId>\n", m_addr);
    //printf("%s", buf);
    fputs(buf, pFile);

    sprintf(buf, "\t\t<Manufacturer>CyberPower</Manufacturer>\n");
    //printf("%s", buf);
    fputs(buf, pFile);

    sprintf(buf, "\t\t<Model>CyberPower-%dP</Model>\n", device);
    //printf("%s", buf);
    fputs(buf, pFile);

    fputs("\t\t<OtherType>0</OtherType>\n", pFile);

    sprintf(buf, "\t\t<dev_id>%d</dev_id>\n", m_devid);
    //printf("%s", buf);
    fputs(buf, pFile);

    fputs("\t</record>\n", pFile);

    if ( last )
        fputs("</records>", pFile);
    printf("===================== Set MIList XML end =====================\n");

    fclose(pFile);

    if ( last ) {
        system("sync");
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

    SaveLog((char *)"DataLogger WriteMIListXML() : OK", m_st_time);

    return true;
}
