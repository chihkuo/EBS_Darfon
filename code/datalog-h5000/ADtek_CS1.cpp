#include "datalog.h"
#include "ADtek_CS1.h"
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
    extern void     MStartTX();
    //extern unsigned char*   GetCyberPowerRespond(int iSize, int delay);

    extern unsigned int     txsize;
    extern unsigned char    waitAddr, waitFCode;
    extern bool             have_respond;

    extern unsigned char    txbuffer[1544];//MODBUS_TX_BUFFER_SIZE
}

ADtek_CS1::ADtek_CS1()
{
    m_sys_error = 0;
    m_do_get_TZ = false;
    m_st_time = NULL;
    m_current_time = 0;

    m_dl_config = {0};
    m_dl_path = {0};
}

ADtek_CS1::~ADtek_CS1()
{
    //dtor
}

bool ADtek_CS1::Init(int com, bool open_com, bool first)
{
    char *port = NULL;
    char szbuf[32] = {0};
    char inverter_parity = 0;
    bool ret = true;

    printf("#### ADtek CS1 Init Start ####\n");

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
        if ( MyModbusDrvInit(port, m_dl_config.m_inverter_baud, m_dl_config.m_inverter_data_bits, inverter_parity, m_dl_config.m_inverter_stop_bits) != 0 )
            ret = false;
    }

    // get time zone
    if ( first ) {
        GetTimezone();
        usleep(1000000);
    }

    // set save file path
    GetLocalTime();
    SetPath();

    printf("\n##### ADtek CS1 Init End #####\n");

    return ret;
}

void ADtek_CS1::GetMAC()
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

bool ADtek_CS1::GetDLConfig()
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
    pFile = popen("uci get dlsetting.@sms[0].delay_time", "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &m_dl_config.m_delay_time);
    printf("Delay time (us.) = %d\n", m_dl_config.m_delay_time);

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

bool ADtek_CS1::SetPath()
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

bool ADtek_CS1::GetTimezone()
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
void ADtek_CS1::SetTimezone(char *zonename, char *timazone)
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

void ADtek_CS1::GetNTPTime()
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

void ADtek_CS1::GetLocalTime()
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
