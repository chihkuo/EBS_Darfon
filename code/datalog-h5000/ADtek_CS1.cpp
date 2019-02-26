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
    extern void     MStartTX(int fd);
    extern unsigned char*   GetADtekRespond(int fd, int iSize, int delay);

    extern unsigned int     txsize;
    extern unsigned char    waitAddr, waitFCode;
    extern bool             have_respond;

    extern unsigned char    txbuffer[1544];//MODBUS_TX_BUFFER_SIZE
}

ADtek_CS1::ADtek_CS1()
{
    m_addr = 0;
    m_devid = 0;
    m_busfd = 0;
    m_env_temp = 0;
    m_env_point = 0;
    m_milist_size = 0;
    m_loopflag = 0;
    m_sys_error = 0;
    m_do_get_TZ = false;
    m_st_time = NULL;
    m_current_time = 0;

    m_dl_config = {0};
    m_dl_path = {0};

    memset(m_env_filename, 0x00, 128);
    memset(m_log_filename, 0x00, 128);
    memset(m_errlog_filename, 0x00, 128);
}

ADtek_CS1::~ADtek_CS1()
{
    //dtor
}

int ADtek_CS1::Init(int com, bool open_com, bool first, int busfd)
{
    char *port = NULL;
    char szbuf[32] = {0};
    char inverter_parity = 0;
    int ret = 0;

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

    printf("\n##### ADtek CS1 Init End #####\n");

    return ret;
}

void ADtek_CS1::GetEnv(int addr, int devid, time_t data_time, bool first, bool last)
{
    struct  stat st;

    //m_current_time = time(NULL);
    m_current_time = data_time;
    m_st_time = localtime(&m_current_time);
    // set path
    SetPath();
    SetEnvXML();
    SetLogXML();
    SetErrorLogXML();
    CheckConfig();
    CleanParameter();

    m_addr = addr;
    m_devid = devid;
    m_loopflag = 0;

    if ( stat(m_env_filename, &st) == 0 ) {
        printf("======== %s exist! ========\n", m_env_filename);
        return;
    }

    OpenLog(m_dl_path.m_syslog_path, m_st_time);

    // if need, add here
    //RunTODOList();

    if ( GetTemp() )
        ;
    else
        m_loopflag++;

    if ( GetPoint() )
        ;
    else
        m_loopflag++;

    WriteEnvXML();
    SaveEnvXML(first, last);

    SaveLogXML(first, last);
    SaveErrorLogXML(first, last);

    WriteMIListXML(first, last);

    SaveDeviceList(first, last);

    //CloseLog(); // cancel, move to main loop execute at loop end
    system("sync");

    // get timezone if before not succss
    if ( m_do_get_TZ )
        GetTimezone();

    return;
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

    printf("m_xml_path    = %s\n", m_dl_path.m_xml_path);
    printf("m_log_path    = %s\n", m_dl_path.m_log_path);
    printf("m_errlog_path = %s\n", m_dl_path.m_errlog_path);
    printf("m_env_path    = %s\n", m_dl_path.m_env_path);
    printf("m_syslog_path = %s\n", m_dl_path.m_syslog_path);
    printf("m_bms_path    = %s\n", m_dl_path.m_bms_path);

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

        // create /tmp ENV dir
        sprintf(tmpbuf, "%s/XML/ENV", DEF_PATH);
        if ( stat(tmpbuf, &st) == -1 ) {
            printf("%s not exist, run mkdir!\n", tmpbuf);
            if ( mkdir(tmpbuf, 0755) == -1 )
                printf("mkdir %s fail!\n", tmpbuf);
            else
                printf("mkdir %s OK\n", tmpbuf);
        }
        // create /tmp ENV date dir
        sprintf(tmpbuf, "%s/XML/ENV/%4d%02d%02d", DEF_PATH, 1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday);
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

void ADtek_CS1::CleanParameter()
{
    memset(m_env_buf, 0x00, AD_ENV_BUF_SIZE);
    m_addr = 0;
    m_devid = 0;
    m_env_temp = 0;
    m_env_point = 0;

    return;
}

bool ADtek_CS1::CheckConfig()
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

bool ADtek_CS1::GetTemp()
{
    printf("#### GetTemp start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    unsigned char cmd_buf[]={0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
    cmd_buf[0] = m_devid;
    MakeReadDataCRC(cmd_buf,8);

    MClearRX();
    txsize = 8;
    waitAddr = m_devid;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd_buf, 8);
        MStartTX(m_busfd);
        usleep(m_dl_config.m_delay_time_1);

        lpdata = GetADtekRespond(m_busfd, 7, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### GetTemp OK ####\n");
            SaveLog((char *)"ADtek GetTemp() : OK", m_st_time);
            DumpTemp(lpdata+3);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetTemp CRC Error ####\n");
                SaveLog((char *)"ADtek GetTemp() : CRC Error", m_st_time);
            }
            else {
                printf("#### GetTemp No Response ####\n");
                SaveLog((char *)"ADtek GetTemp() : No Response", m_st_time);
            }
            err++;
        }
    }

    return false;
}

void ADtek_CS1::DumpTemp(unsigned char *buf)
{
    m_env_temp = (*(buf) << 8) + *(buf+1);

    printf("##### Dump Temp #####\n");
    printf("m_env_temp = %04X\n", m_env_temp);
    printf("#####################\n");

    return;
}

bool ADtek_CS1::GetPoint()
{
    printf("#### GetPoint start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    unsigned char cmd_buf[]={0x00, 0x03, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00};
    cmd_buf[0] = m_devid;
    MakeReadDataCRC(cmd_buf,8);

    MClearRX();
    txsize = 8;
    waitAddr = m_devid;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd_buf, 8);
        MStartTX(m_busfd);
        usleep(m_dl_config.m_delay_time_1);

        lpdata = GetADtekRespond(m_busfd, 7, m_dl_config.m_delay_time_1);
        if ( lpdata ) {
            printf("#### GetPoint OK ####\n");
            SaveLog((char *)"ADtek GetPoint() : OK", m_st_time);
            DumpPoint(lpdata+3);
            return true;
        } else {
            if ( have_respond == true ) {
                printf("#### GetPoint CRC Error ####\n");
                SaveLog((char *)"ADtek GetPoint() : CRC Error", m_st_time);
            }
            else {
                printf("#### GetPoint No Response ####\n");
                SaveLog((char *)"ADtek GetPoint() : No Response", m_st_time);
            }
            err++;
        }
    }

    return false;
}

void ADtek_CS1::DumpPoint(unsigned char *buf)
{
    m_env_point = (*(buf) << 8) + *(buf+1);

    printf("##### Dump Point #####\n");
    printf("m_env_point = %04X\n", m_env_point);
    printf("#####################\n");

    return;
}

void ADtek_CS1::SetEnvXML()
{
    sprintf(m_env_filename, "%s/%02d%02d", m_dl_path.m_env_path, m_st_time->tm_hour, m_st_time->tm_min);
    printf("env path = %s\n", m_env_filename);
    return;
}

void ADtek_CS1::SetLogXML()
{
    sprintf(m_log_filename, "%s/%02d%02d", m_dl_path.m_log_path, m_st_time->tm_hour, m_st_time->tm_min);
    printf("log path = %s\n", m_log_filename);
    return;
}

bool ADtek_CS1::SaveLogXML(bool first, bool last)
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
        fd = fopen("/tmp/tmplog", "ab");
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
        SaveLog((char *)"ADtek_CS1 SaveLogXML() : open /tmp/tmplog Fail", m_st_time);
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
        sprintf(buf, "ADtek_CS1 SaveLogXML() : write %s OK", m_log_filename);
        SaveLog(buf, m_st_time);
        if ( strstr(m_log_filename, USB_PATH) )
            m_sys_error  &= ~SYS_0002_Save_USB_Fail;
        return true;
    } else {
        sprintf(buf, "ADtek_CS1 SaveLogXML() : write %s Fail", m_log_filename);
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
            SaveLog((char *)"ADtek_CS1 SaveLogXML() : write to tmp OK", m_st_time);
            return true;
        } else {
            SaveLog((char *)"ADtek_CS1 SaveLogXML() : write to tmp Fail", m_st_time);
            return false;
        }
    }

    return false;
}

void ADtek_CS1::SetErrorLogXML()
{
    sprintf(m_errlog_filename, "%s/%02d%02d", m_dl_path.m_errlog_path, m_st_time->tm_hour, m_st_time->tm_min);
    printf("errlog path = %s\n", m_errlog_filename);
    return;
}

bool ADtek_CS1::SaveErrorLogXML(bool first, bool last)
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
        strcat(buf, "</records>");
        while ( (strlen(buf) + filesize) % 3 != 0 ) {
            buf[strlen(buf)] = 0x20; // add space to end
        }
    }

    if ( fd != NULL ) {
        fwrite(buf, 1, strlen(buf), fd);
        filesize += strlen(buf);
        //printf("Errlog filesize = %d\n", filesize);
        fclose(fd);
    } else {
        SaveLog((char *)"ADtek_CS1 SaveErrorLogXML() : open /tmp/tmperrlog Fail", m_st_time);
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
        sprintf(buf, "ADtek_CS1 SaveErrorLogXML() : write %s OK", m_errlog_filename);
        SaveLog(buf, m_st_time);
        if ( strstr(m_errlog_filename, USB_PATH) )
            m_sys_error  &= ~SYS_0002_Save_USB_Fail;
        return true;
    } else {
        sprintf(buf, "ADtek_CS1 SaveErrorLogXML() : write %s Fail", m_errlog_filename);
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
            SaveLog((char *)"ADtek_CS1 SaveErrorLogXML() : write to tmp OK", m_st_time);
            return true;
        } else {
            SaveLog((char *)"ADtek_CS1 SaveErrorLogXML() : write to tmp Fail", m_st_time);
            return false;
        }
    }

    return false;
}

bool ADtek_CS1::WriteEnvXML()
{
    char buf[256] = {0};
    float value = 0;

    SaveLog((char *)"ADtek_CS1 WriteEnvXML() : run", m_st_time);

    printf("==================== Set Env XML start ====================\n");
    sprintf(buf, "<record dev_id=\"%d\" date=\"%04d-%02d-%02d %02d:%02d:00\" sn=\"\">", m_devid,
            1900+m_st_time->tm_year, 1+m_st_time->tm_mon, m_st_time->tm_mday,
            m_st_time->tm_hour, m_st_time->tm_min);
    strcat(m_env_buf, buf);

    if ( m_loopflag == 0 ) {
        switch (m_env_point)
        {
            case 0:
                sprintf(buf, "<Temperature>%d</Temperature>", m_env_temp);
                break;
            case 1:
                value = (float)m_env_temp*0.1;
                sprintf(buf, "<Temperature>%.1f</Temperature>", value);
                break;
            case 2:
                value = (float)m_env_temp*0.01;
                sprintf(buf, "<Temperature>%.2f</Temperature>", value);
                break;
            case 3:
                value = (float)m_env_temp*0.001;
                sprintf(buf, "<Temperature>%.3f</Temperature>", value);
                break;
            case 4:
                value = (float)m_env_temp*0.0001;
                sprintf(buf, "<Temperature>%.4f</Temperature>", value);
                break;
            default:
                printf("Out of range!\n");
        }
        strcat(m_env_buf, buf);
    }

    strcat(m_env_buf, "</record>");
    printf("m_env_buf = \n%s\n", m_env_buf);
    printf("===================== Set Env XML end =====================\n");

    return true;
}

bool ADtek_CS1::SaveEnvXML(bool first, bool last)
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
        strcat(m_env_buf, "</records>");
        while ( (strlen(m_env_buf) + filesize) % 3 != 0 ) {
            m_env_buf[strlen(m_env_buf)] = 0x20; // add space to end
        }
    }

    if ( fd != NULL ) {
        fwrite(m_env_buf, 1, strlen(m_env_buf), fd);
        filesize += strlen(m_env_buf);
        //printf("Log filesize = %d\n", filesize);
        fclose(fd);
    } else {
        SaveLog((char *)"ADtek_CS1 SaveEnvXML() : open /tmp/tmpenv Fail", m_st_time);
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
        sprintf(buf, "ADtek_CS1 SaveEnvXML() : write %s OK", m_env_filename);
        SaveLog(buf, m_st_time);
        //if ( strstr(m_env_filename, USB_PATH) )
        //    m_sys_error  &= ~SYS_0002_Save_USB_Fail;
        return true;
    } else {
        sprintf(buf, "ADtek_CS1 SaveEnvXML() : write %s Fail", m_env_filename);
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
            SaveLog((char *)"ADtek_CS1 SaveEnvXML() : write to tmp OK", m_st_time);
            return true;
        } else {
            SaveLog((char *)"ADtek_CS1 SaveEnvXML() : write to tmp Fail", m_st_time);
            return false;
        }
    }

    return false;
}

bool ADtek_CS1::WriteMIListXML(bool first, bool last)
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
        sprintf(tmp, "ADtek_CS1 WriteMIListXML() : fopen %s fail", buf);
        SaveLog(tmp, m_st_time);
        return false;
    }

    printf("==================== ADtek_CS1 Set MIList XML start ====================\n");
    if ( first )
        fputs("<records>\n", pFile);

    fputs("\t<record>\n", pFile);

    sprintf(buf, "\t\t<port>COM%d</port>\n", m_dl_config.m_inverter_port);
    //printf("%s", buf);
    fputs(buf, pFile);

    sprintf(buf, "\t\t<slaveId>%d</slaveId>\n", m_addr);
    //printf("%s", buf);
    fputs(buf, pFile);

    sprintf(buf, "\t\t<Manufacturer>ADtek</Manufacturer>\n");
    //printf("%s", buf);
    fputs(buf, pFile);

    sprintf(buf, "\t\t<Model>ADtek-CS1-T</Model>\n");
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

bool ADtek_CS1::SaveDeviceList(bool first, bool last)
{
    FILE *pFile;
    char buf[256] = {0};
    int state = 0, offset = 0;
    char *index_tmp = NULL, *index_start = NULL, *index_end = NULL;

    printf("#### ADtek_CS1 SaveDeviceList Start ####\n");

    if ( first )
        pFile = fopen(DEVICELIST_TMP, "w");
    else
        pFile = fopen(DEVICELIST_TMP, "a");
    if ( pFile == NULL ) {
        printf("#### SaveDeviceList open file Fail ####\n");
        SaveLog((char *)"ADtek_CS1 SaveDeviceList() : fopen fail", m_st_time);
        return false;
    }

    if ( m_loopflag == 2 )
        state = 0; // off line
    else
        state = 1; // on line

    // addr fixed 3 digit, state fixed 1 digit, sn
    sprintf(buf, "%03d <SN>Temperature</SN> <STATE>%d</STATE> <DEVICE>ADtek-CS1-T</DEVICE> ", m_addr, state);

    if (state) {
        // set log data
        index_tmp = strstr(m_env_buf, "<record dev_id=");
        if ( index_tmp != NULL ) {
            if ( index_tmp - m_env_buf > 80)
                index_start = strstr(index_tmp-80, "<record dev_id=");
            else
                index_start = strstr(m_env_buf, "<record dev_id=");

            index_end = strstr(index_start, "</record>");
            offset = index_end - index_start + 9;
            strcat(buf, "<ENERGY>");
            strncat(buf, index_start, offset);
            strcat(buf, "</ENERGY>");

        } else {
            printf("index_tmp = NULL!\n");
            strcat(buf, "Empty");
        }

        strcat(buf, "Empty");
        // set error log data
        /*index_tmp = strstr(m_errlog_buf, m_cp_sn.SN);
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
        }*/
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
    SaveLog((char *)"ADtek_CS1 SaveDeviceList() : OK", m_st_time);

    return true;
}
