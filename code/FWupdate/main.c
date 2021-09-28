#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>

#include "../common/base64.h"
#include "../common/SaveLog.h"

//#define USB_PATH    "/tmp/run/mountd/sda1"
#define USB_PATH    "/mnt"
#define USB_DEV     "/dev/sda1"
#define SDCARD_PATH "/tmp/sdcard"

#define VERSION             "1.2.7"
#define TIMEOUT             "30"
#define CURL_FILE           "/tmp/FWupdate"
#define CURL_CMD            "curl -H 'Content-Type: text/xml;charset=UTF-8;SOAPAction:\"\"' -k https://52.9.235.220:8443/SmsWebService1.asmx?WSDL -d @"CURL_FILE" --max-time "TIMEOUT

#define MIFW_LIST               "mifwlist"
#define MIFW_BLIST              "mifwbroadcastlist"
#define HBFW_LIST               "hybridfwlist"
#define PLC_LIST                "plclist"
#define PLCM_LIST               "plcmlist"
#define BATFW_LIST              "batteryfwlist"

#define TMP_MIFW_LIST           "/tmp/mifwlist"
#define TMP_MIFW_BLIST          "/tmp/mifwbroadcastlist"
#define TMP_HBFW_LIST           "/tmp/hybridfwlist"
#define TMP_PLC_LIST            "/tmp/plclist"
#define TMP_PLCM_LIST           "/tmp/plcmlist"
#define TMP_BATFW_LIST          "/tmp/batteryfwlist"

#define USB_MIFW_LIST           "/mnt/mifwlist"
#define USB_MIFW_BLIST          "/mnt/mifwbroadcastlist"
#define USB_HBFW_LIST           "/mnt/hybridfwlist"
#define USB_PLC_LIST            "/mnt/plclist"
#define USB_PLCM_LIST           "/mnt/plcmlist"
#define USB_BATFW_LIST          "/mnt/batteryfwlist"

#define SYSLOG_PATH         "/tmp/test/SYSLOG"
#define MAX_DATA_SIZE       144
#define MAX_HYBRID_SIZE     100
#define MAX_BATTERY_SIZE    100

#define	HYBRIDFW_FILE       "/mnt/hybridfw.tar.gz"
//#define HYBRIDFW_LIST       "/mnt/hybridfwlist"

// extern part
extern int  MyModbusDrvInit(char *port, int baud, int data_bits, char parity, int stop_bits);
extern int  ModbusDrvDeinit(int fd);
extern void MakeReadDataCRC(unsigned char *,int );
extern void MClearRX();
extern void MStartTX(int fd);
extern unsigned char *GetRespond(int fd, int iSize, int delay);
extern void RemoveRegisterQuery(int fd, unsigned int byAddr);
extern void CleanRespond(int fd);
extern void initenv(char *init_name);
extern unsigned short CalculateCRC(unsigned char *, unsigned int );

extern unsigned int     txsize;
extern unsigned char    waitAddr, waitFCode;
#define bool int
extern bool have_respond;
extern bool check_respond;
extern unsigned char    txbuffer[1544];//MODBUS_TX_BUFFER_SIZE
extern unsigned char respond_buff[4096];

void getMAC(char *MAC);
void getConfig();
void setCMD();
void setPath();
int QryDeviceFWUpdate();
int GetComPortSetting(int port);
int OpenComPort(int comport);
int CheckVer();
int WriteVerV2(int slaveid, unsigned char *fwver);
int WriteVerV3(char *sn, unsigned char *fwver);
int RunRegister(char *sn);
int RunEnableP3(int slaveid);
int RunShutdown(int slaveid);
int RunReboot(int slaveid);
int RunRebootSpecify(char *sn);
int LBDReregister(char *sn);
int WriteDataV2(int slaveid, unsigned char *fwdata, int datasize);
int WriteHBData(int slaveid, unsigned char *fwdata, int datasize);
int WriteDataV3(char *sn, unsigned char *fwdata, int datasize);
int ReadV3Ver(char *sn, unsigned char *fwver);
int GetFWData(char *list_path);
int GetHbFWData(char *list_path);
int RunBatFWUpdate(char *list_path);
int stopProcess();
int runProcess();
int GetPort(char *file_path);
int GetMIList(char *file_path);
int GetSNList(char *list_path);
int CheckType(int index, int *ret);
int DoUpdate(char *list_path);
int UpdDLFWStatus();
int CLEANSN(char *sn, char *file_path, char *list_path);
int Updheartbeattime(time_t time);

// add battery fw update
int RunStopBat(int loop, int slaveid);
int RunStartBat(int loop, int slaveid);
int GetBatInfo(int loop, int slaveid, int retry);
int GetBatVer(int loop, int slaveid);
int GetSlaveInfo(int loop, int slaveid, int number);
int SetControl(int loop, int slaveid, unsigned char value, int retry);
int SetHeader(int loop, int slaveid, unsigned char *header);
int StopBatFWUpdate(int loop, int slaveid);
int WriteBatData(int loop, int slaveid, unsigned short section, unsigned short section_size, FILE* filefd);
int CheckResult(int loop, int slaveid);
int updBATFWstatus();

char SOAP_HEAD[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\n\
\t<soap:Body>\n";

char SOAP_TAIL[] = "\t</soap:Body>\n</soap:Envelope>";

char MAC[18] = {0};
char UPDATE_SERVER[128] = {0};
int update_port = 0;
int update_SW_time = 0;
int delay_time_1 = 0;
int delay_time_2 = 0;
int update_FW_start = 0;
int update_FW_stop = 0;
char g_CURL_CMD[256] = {0};
char g_SYSLOG_PATH[64] = {0};
char g_UPDATE_PATH[64] = {0};

int gisusb = 0;
int gbaud = 0;
int gdatabits = 0;
char gparity[8] = {0};
int gstopbits = 0;
int gcomportfd = 0;
int gprotocolver = 0;
int gV2id = -1;
int bat_status_4 = 0;
int bat_status_5 = 0;

int gmicount = 0;
typedef struct mi_list {
    char SN[17];
    int slave_id;
    unsigned char sn_bin[8];
    int OtherType;
} MI_LIST;
MI_LIST milist[255] = {0};

int gsncount = 0;
typedef struct sn_list {
    char SN[17];
    char FILE[128];
    int status;
    unsigned short ver;
} SN_LIST;
SN_LIST snlist[255] = {0};

// update parameter
typedef struct fwupdate {
    char UpdateTarget[16];
    char UpdateType[16];
    char FWURL[128];
    char SN[17];
    char FILE[128];
}FWUPDATE;
FWUPDATE myupdate = {0};

typedef struct batinfo {
    unsigned short boot_ver;
    unsigned short app_ver;
    unsigned short backup_ver;
    unsigned short update_ver;
    unsigned short status;
    unsigned short burn_status;
    unsigned short section;
    unsigned short BMS_number;
}BATINFO;
BATINFO mybatinfo = {0};

void getMAC(char *MAC)
{
    FILE *fd = NULL;

    // get MAC address
    fd = popen("uci get network.lan_dev.macaddr", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(MAC, 18, fd);
    pclose(fd);

    printf("MAC = %s\n", MAC);

    return;
}

void getConfig()
{
    char buf[32] = {0};
    FILE *fd = NULL;

    // get update server
    fd = popen("uci get dlsetting.@sms[0].update_server", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(UPDATE_SERVER, 128, fd);
    pclose(fd);
    if ( strlen(UPDATE_SERVER) )
        UPDATE_SERVER[strlen(UPDATE_SERVER)-1] = 0; // clean \n
    printf("Update Server = %s\n", UPDATE_SERVER);

    // get update port
    fd = popen("uci get dlsetting.@sms[0].update_port", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 32, fd);
    pclose(fd);
    sscanf(buf, "%d", &update_port);
    printf("Update Port = %d\n", update_port);

    // get update SW time
    fd = popen("uci get dlsetting.@sms[0].update_SW_time", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 32, fd);
    pclose(fd);
    sscanf(buf, "%d", &update_SW_time);
    printf("Update SW time = %d\n", update_SW_time);

    // get update FW start
    fd = popen("uci get dlsetting.@sms[0].update_FW_start", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 32, fd);
    pclose(fd);
    sscanf(buf, "%d", &update_FW_start);
    printf("Update FW start = %d\n", update_FW_start);

    // get update FW stop
    fd = popen("uci get dlsetting.@sms[0].update_FW_stop", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 32, fd);
    pclose(fd);
    sscanf(buf, "%d", &update_FW_stop);
    printf("Update FW stop = %d\n", update_FW_stop);

    // get delay_time_1
    fd = popen("uci get dlsetting.@sms[0].delay_time_1", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 32, fd);
    pclose(fd);
    sscanf(buf, "%d", &delay_time_1);
    printf("Delay time 1 (us.) = %d\n", delay_time_1);

    // get delay_time_2
    fd = popen("uci get dlsetting.@sms[0].delay_time_2", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 32, fd);
    pclose(fd);
    sscanf(buf, "%d", &delay_time_2);
    printf("Delay time 2 (us.) = %d\n", delay_time_2);

    return;
}

void setCMD()
{
    if ( strlen(UPDATE_SERVER) )
        sprintf(g_CURL_CMD, "curl -H 'Content-Type: text/xml;charset=UTF-8;SOAPAction:\"\"' -k %s:%d/SmsWebService1.asmx?WSDL -d @%s --max-time %s", UPDATE_SERVER, update_port, CURL_FILE, TIMEOUT);
    else
        sprintf(g_CURL_CMD, "curl -H 'Content-Type: text/xml;charset=UTF-8;SOAPAction:\"\"' http://60.248.27.82:8080/SmsWebService1.asmx?WSDL -d @%s --max-time %s", CURL_FILE, TIMEOUT);

    return;
}

void setPath()
{
    struct stat st;

    if ( stat(USB_DEV, &st) == 0 ) { //linux storage detect
        strcpy(g_SYSLOG_PATH, USB_PATH);
        strcat(g_SYSLOG_PATH, "/SYSLOG");
        //strcpy(g_UPDATE_PATH, USB_PATH);
        //strcat(g_UPDATE_PATH, "/test.hex");

        gisusb = 1;
    }
    else if ( stat(SDCARD_PATH, &st) == 0 ) {
        strcpy(g_SYSLOG_PATH, SDCARD_PATH);
        strcat(g_SYSLOG_PATH, "/SYSLOG");
        //strcpy(g_UPDATE_PATH, SDCARD_PATH);
        //strcat(g_UPDATE_PATH, "/test.hex");

        gisusb = 0;
    }
    else {
        strcpy(g_SYSLOG_PATH, SYSLOG_PATH);
        //strcpy(g_UPDATE_PATH, UPDATE_FILE);

        gisusb = 0;
    }

    //strcpy(g_UPDATE_PATH, UPDATE_FILE);

    printf("g_SYSLOG_PATH = %s\n", g_SYSLOG_PATH);
    //printf("g_UPDATE_PATH = %s\n", g_UPDATE_PATH);

    return;
}

int QryDeviceFWUpdate()
{
    char buf[1024] = {0};
    FILE *fd = NULL, *miufd = NULL, *mibfd = NULL, *hbufd = NULL, *plcfd = NULL, *plcmfd = NULL, *batfd = NULL;
    int size = 0, inlen = 0, outlen = 0;
    char *data = NULL, *start_index = NULL, *end_index = NULL, *search_file = NULL, *save_file = NULL;
    unsigned char *decode_data = NULL;
    struct stat mystat;
    time_t current_time;
    struct tm *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    printf("======================== QryDeviceFWUpdate start ========================\n");
    // set QryDeviceFWUpdate xml file
    fd = fopen(CURL_FILE, "wb");
    if ( fd == NULL ) {
        printf("#### QryDeviceFWUpdate() open %s Fail ####\n", CURL_FILE);
        return 1;
    }
    fputs(SOAP_HEAD, fd);
    sprintf(buf, "\t\t<QryDeviceFWUpdate xmlns=\"http://tempuri.org/\">\n");
    fputs(buf, fd);
    sprintf(buf, "\t\t\t<macaddress>%s</macaddress>\n", MAC);
    fputs(buf, fd);
    sprintf(buf, "\t\t</QryDeviceFWUpdate>\n");
    fputs(buf, fd);
    fputs(SOAP_TAIL, fd);
    fclose(fd);

    // run curl soap command, save result to /tmp/QryDeviceFWUpdate
    sprintf(buf, "%s > /tmp/QryDeviceFWUpdate", g_CURL_CMD);
    system(buf);

    // check size
    // for debug
    fd = fopen("/tmp/QryDeviceFWUpdate", "rb");
    if ( fd == NULL ) {
        printf("#### QryDeviceFWUpdate() open /tmp/QryDeviceFWUpdate Fail ####\n");
        SaveLog("FWupdate QryDeviceFWUpdate() : open /tmp/QryDeviceFWUpdate Fail", st_time);
        return 2;
    }
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    printf("size = %d\n", size);
    // read result
    data = (char*)malloc(size+1);
    memset(data, 0x00, size+1);
    fread(data, 1, size, fd);
    fclose(fd);
    printf("data : \n%s\n", data);

    // find start address & length
    start_index = strstr(data, "<QryDeviceFWUpdateResult>");
    if ( start_index == NULL ) {
        printf("#### QryDeviceFWUpdate() <QryDeviceFWUpdateResult> not found ####\n");
        SaveLog("FWupdate QryDeviceFWUpdate() : <QryDeviceFWUpdateResult> not found", st_time);
        if ( data )
            free(data);
        return 3;
    }
    start_index += 25; // <QryDeviceFWUpdateResult> length
    end_index = strstr(data, "</QryDeviceFWUpdateResult>");
    if ( end_index == NULL ) {
        printf("#### QryDeviceFWUpdate() </QryDeviceFWUpdateResult> not found ####\n");
        SaveLog("FWupdate QryDeviceFWUpdate() : </QryDeviceFWUpdateResult> not found", st_time);
        if ( data )
            free(data);
        return 4;
    }
    inlen = end_index - start_index;
    printf("inlen = %d\n", inlen);

    decode_data = base64_decode(start_index, inlen, &outlen);
    if ( decode_data == NULL ) {
        printf("#### QryDLSWUpdate() decode_data = NULL ####\n");
        SaveLog("FWupdate QryDLSWUpdate() : decode_data = NULL", st_time);
        if ( data )
            free(data);
        return 5;
    }
    printf("decode_data len = %d, : \n%s\n", outlen, decode_data);
    if ( data )
        free(data);

    // parser update parameter
    start_index = (char*)decode_data;
    end_index = (char*)decode_data;
    while ( start_index != NULL && end_index != NULL ) {
        // set UpdateTarget
        start_index = strstr(start_index, "<UpdateTarget>");
        end_index = strstr(end_index, "</UpdateTarget>");
        if ( start_index == NULL || end_index == NULL )
            break;
        memset(myupdate.UpdateTarget, 0, 16);
        strncpy(myupdate.UpdateTarget, start_index+14, end_index-(start_index+14));

        // set UpdateType
        start_index = strstr(start_index, "<UpdateType>");
        end_index = strstr(end_index, "</UpdateType>");
        if ( start_index == NULL || end_index == NULL )
            break;
        memset(myupdate.UpdateType, 0, 16);
        strncpy(myupdate.UpdateType, start_index+12, end_index-(start_index+12));

        // set FWURL
        start_index = strstr(start_index, "<FWURL>");
        end_index = strstr(end_index, "</FWURL>");
        if ( start_index == NULL || end_index == NULL )
            break;
        memset(myupdate.FWURL, 0, 128);
        strncpy(myupdate.FWURL, start_index+7, end_index-(start_index+7));

        // set FILE from FWURL
        search_file = myupdate.FWURL;
        save_file = NULL;
        while ( (search_file = strchr(search_file, '/')) ) {
            save_file = search_file;
            search_file++;
        }
        if ( save_file ) {
            //save_file++;
            memset(myupdate.FILE, 0, 128);
            if ( gisusb )
                strcpy(myupdate.FILE, USB_PATH);
            else
                strcpy(myupdate.FILE, "/tmp");
            strncat(myupdate.FILE, save_file, strlen(save_file));
        }

        // set SN
        start_index = strstr(start_index, "<sn>");
        end_index = strstr(end_index, "</sn>");
        if ( start_index == NULL || end_index == NULL )
            break;
        memset(myupdate.SN, 0, 17);
        strncpy(myupdate.SN, start_index+4, end_index-(start_index+4));

        // debug print
        printf("===============================================================================\n");
        printf("UpdateTarget = %s\n", myupdate.UpdateTarget);
        printf("UpdateType = %s\n", myupdate.UpdateType);
        printf("FWURL = %s\n", myupdate.FWURL);
        printf("FILE = %s\n", myupdate.FILE);
        printf("sn = %s\n", myupdate.SN);

        // write list & file name
        if ( strcmp(myupdate.UpdateTarget, "MI") == 0 ) {
            if ( strcmp(myupdate.UpdateType, "Unicast") == 0 ) {
                // open list file
                if ( miufd == NULL ) {
                    if ( gisusb ) {
                        // usb
                        if ( stat(USB_MIFW_LIST, &mystat) == 0 )
                            miufd = fopen(USB_MIFW_LIST, "ab");
                        else
                            miufd = fopen(USB_MIFW_LIST, "wb");
                    } else {
                        // tmp
                        if ( stat(TMP_MIFW_LIST, &mystat) == 0 )
                            miufd = fopen(TMP_MIFW_LIST, "ab");
                        else
                            miufd = fopen(TMP_MIFW_LIST, "wb");
                    }
                }
                // write sn & file path in to list
                sprintf(buf, "%s %s\n", myupdate.SN, myupdate.FILE);
                fputs(buf, miufd);

            } else if ( strcmp(myupdate.UpdateType, "Broadcast") == 0 ) {
                // open list file
                if ( mibfd == NULL ) {
                    if ( gisusb ) {
                        // usb
                        mibfd = fopen(USB_MIFW_BLIST, "wb");
                    } else {
                        // tmp
                        mibfd = fopen(TMP_MIFW_BLIST, "wb");
                    }
                }
                // write sn & file path in to list
                sprintf(buf, "MIALL %s\n", myupdate.FILE);
                fputs(buf, mibfd);

            } else {
                printf("UpdateType error!\n");
            }

        } else if ( strcmp(myupdate.UpdateTarget, "Hybrid") == 0 ) {
            if ( strcmp(myupdate.UpdateType, "Unicast") == 0 ) {
                // open list file
                if ( hbufd == NULL ) {
                    if ( gisusb ) {
                        // usb
                        if ( stat(USB_HBFW_LIST, &mystat) == 0 )
                            hbufd = fopen(USB_HBFW_LIST, "ab");
                        else
                            hbufd = fopen(USB_HBFW_LIST, "wb");
                    } else {
                        // tmp
                        if ( stat(TMP_HBFW_LIST, &mystat) == 0 )
                            hbufd = fopen(TMP_HBFW_LIST, "ab");
                        else
                            hbufd = fopen(TMP_HBFW_LIST, "wb");
                    }
                }
                // write sn & file path in to list
                sprintf(buf, "%s %s\n", myupdate.SN, myupdate.FILE);
                fputs(buf, hbufd);
            } else if ( strcmp(myupdate.UpdateType, "Battery") == 0 ) {
                // open list file
                if ( batfd == NULL ) {
                    if ( gisusb ) {
                        // usb
                        if ( stat(USB_BATFW_LIST, &mystat) == 0 )
                            batfd = fopen(USB_BATFW_LIST, "ab");
                        else
                            batfd = fopen(USB_BATFW_LIST, "wb");
                    } else {
                        // tmp
                        if ( stat(TMP_BATFW_LIST, &mystat) == 0 )
                            batfd = fopen(TMP_BATFW_LIST, "ab");
                        else
                            batfd = fopen(TMP_BATFW_LIST, "wb");
                    }
                }
                // write sn & file path in to list
                sprintf(buf, "%s %s\n", myupdate.SN, myupdate.FILE);
                fputs(buf, batfd);
            } else {
                printf("UpdateType error!\n");
            }

        } else if ( strcmp(myupdate.UpdateTarget, "PLC") == 0 ) {
            if ( strcmp(myupdate.UpdateType, "Unicast") == 0 ) {
                // open list file
                if ( plcfd == NULL ) {
                    if ( gisusb ) {
                        // usb
                        plcfd = fopen(USB_PLC_LIST, "wb");
                    } else {
                        // tmp
                        plcfd = fopen(TMP_PLC_LIST, "wb");
                    }
                }
                // write sn & file path in to file
                sprintf(buf, "PLC %s\n", myupdate.FILE);
                fputs(buf, plcfd);

            } else {
                printf("UpdateType error!\n");
            }

        } else if ( strcmp(myupdate.UpdateTarget, "PLC Module") == 0 ) {
            if ( strcmp(myupdate.UpdateType, "Broadcast") == 0 ) {
                // open list file
                if ( plcmfd == NULL ) {
                    if ( gisusb ) {
                        // usb
                        plcmfd = fopen(USB_PLCM_LIST, "wb");
                    } else {
                        // tmp
                        plcmfd = fopen(TMP_PLCM_LIST, "wb");
                    }
                }
                // write sn & file path in to file
                sprintf(buf, "PLCMALL %s\n", myupdate.FILE);
                fputs(buf, plcmfd);

            } else {
                printf("UpdateType error!\n");
            }

        } else {
            printf("UpdateTarget error!\n");
        }

        // download file
        if ( strlen(myupdate.FWURL) ) {
            // check file exist?
            if ( stat(myupdate.FILE, &mystat) == 0 )
                printf("%s already exist!\n", myupdate.FILE);
            else {
                // download
                current_time = time(NULL);
                st_time = localtime(&current_time);
                sprintf(buf, "curl -o %s %s", myupdate.FILE, myupdate.FWURL);
                if ( !system(buf) ) {
                    printf("Download %s to %s OK\n", myupdate.FWURL, myupdate.FILE);
                    sprintf(buf, "FWupdate QryDeviceFWUpdate() : Download %s to %s OK", myupdate.FWURL, myupdate.FILE);
                    SaveLog(buf, st_time);
                } else {
                    printf("Download %s to %s FAIL\n", myupdate.FWURL, myupdate.FILE);
                    sprintf(buf, "FWupdate QryDeviceFWUpdate() : Download %s to %s FAIL", myupdate.FWURL, myupdate.FILE);
                    SaveLog(buf, st_time);
                }
            }
        }

        printf("===============================================================================\n");
    }
    if ( miufd )
        fclose(miufd);
    if ( mibfd )
        fclose(mibfd);
    if ( hbufd )
        fclose(hbufd);
    if ( batfd )
        fclose(batfd);
    if ( plcfd )
        fclose(plcfd);
    if ( plcmfd )
        fclose(plcmfd);
    if ( decode_data )
        free(decode_data);

    printf("======================= QryDeviceFWUpdate end =======================\n");

    return 0;
}

int GetComPortSetting(int port)
{
    char buf[32] = {0};
    char cmd[128] = {0};
    FILE *pFile = NULL;

    // get baud
    sprintf(cmd, "uci get dlsetting.@comport[0].com%d_baud", port);
    pFile = popen(cmd, "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return 1;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &gbaud);
    printf("Baud rate = %d\n", gbaud);
    // get data bits
    sprintf(cmd, "uci get dlsetting.@comport[0].com%d_data_bits", port);
    pFile = popen(cmd, "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return 2;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &gdatabits);
    printf("Data bits = %d\n", gdatabits);
    // get parity
    sprintf(cmd, "uci get dlsetting.@comport[0].com%d_parity", port);
    pFile = popen(cmd, "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return 3;
    }
    fgets(gparity, 8, pFile);
    pclose(pFile);
    gparity[strlen(gparity)-1] = 0; // clean \n
    printf("Parity = %s\n", gparity);
    // get stop bits
    sprintf(cmd, "uci get dlsetting.@comport[0].com%d_stop_bits", port);
    pFile = popen(cmd, "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return 4;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &gstopbits);
    printf("Stop bits = %d\n", gstopbits);

    return 0;
}

int OpenComPort(int comport)
{
    char *strPort[]={"/dev/ttyUSB0","/dev/ttyUSB1","/dev/ttyUSB2","/dev/ttyUSB3"};
    char inverter_parity = 0;

    // set parity
    if ( strstr(gparity, "Odd") )
        inverter_parity = 'O';
    else if ( strstr(gparity, "Even") )
        inverter_parity = 'E';
    else
        inverter_parity = 'N';

    printf("device node = %s\n", strPort[comport-1]);
    gcomportfd = MyModbusDrvInit(strPort[comport-1], gbaud, gdatabits, inverter_parity, gstopbits);
    printf("\ngcomportfd = %d\n", gcomportfd);

    return gcomportfd;
}

int CheckVer()
{
    printf("\n#### CheckVer start ####\n");

    int err = 0, ret = 0;
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x01, 0x30, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00};
    MakeReadDataCRC(cmd,14);

    MClearRX();
    txsize=14;
    waitAddr = 0x01;
    waitFCode = 0x30;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 14);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        lpdata = GetRespond(gcomportfd, 15, delay_time_1); // from uci config
        if ( lpdata ) {
            printf("#### CheckVer OK ####\n");
            SaveLog((char *)"FWupdate 0x30 CheckVer() : OK", st_time);
            gprotocolver = 3;
            SaveLog((char *)"FWupdate 0x30 CheckVer() : Set V3.0", st_time);
            printf("Set protocol V3.0\n");
            return 0;
        } else {
            if ( have_respond ) {
                printf("#### CheckVer CRC Error ####\n");
                SaveLog((char *)"FWupdate 0x30 CheckVer() : CRC Error", st_time);
                ret = 1;
            }
            else {
                printf("#### CheckVer No Response ####\n");
                SaveLog((char *)"FWupdate 0x30 CheckVer() : No Response", st_time);
                ret = -1;
            }
            err++;
            if ( err == 3 ) {
                gprotocolver = 2;
                SaveLog((char *)"FWupdate 0x30 CheckVer() : Set V2.0", st_time);
                printf("Set protocol V2.0\n");
            }
        }
    }

    return ret;
}

int WriteVerV2(int slaveid, unsigned char *fwver)
{
    printf("\n#### WriteVerV2 start ####\n");

    int err = 0, ret = 0;
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x00, 0x10, 0xFF, 0xFF, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // set slave id
    cmd[0] = (unsigned char)slaveid;
    // set fw ver
    cmd[9]  = fwver[2];
    cmd[10] = fwver[3];
    MakeReadDataCRC(cmd,13);

    MClearRX();
    txsize=13;
    waitAddr = cmd[0];
    waitFCode = 0x10;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 13);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        lpdata = GetRespond(gcomportfd, 8, delay_time_2); // from uci config
        if ( lpdata ) {
            printf("#### WriteVerV2 OK ####\n");
            SaveLog((char *)"FWupdate WriteVerV2() : OK", st_time);
            return 0;
        } else {
            if ( have_respond ) {
                printf("#### WriteVerV2 CRC Error ####\n");
                SaveLog((char *)"FWupdate WriteVerV2() : CRC Error", st_time);
                ret = 1;
            }
            else {
                printf("#### WriteVerV2 No Response ####\n");
                SaveLog((char *)"FWupdate WriteVerV2() : No Response", st_time);
                ret = -1;
            }
            err++;
        }
    }

    return ret;
}

int WriteVerV3(char *sn, unsigned char *fwver)
{
    printf("\n#### WriteVerV3 start ####\n");

    int err = 0, ret = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0, i = 0;
    char buf[256] = {0}, log[1024] = {0};
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x01, 0x48, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    sscanf(sn, "%06s%02X%02X%02X%02X%02X", buf, &tmp1, &tmp2, &tmp3, &tmp4, &tmp5);
    cmd[3] = (unsigned char)tmp1;
    cmd[4] = (unsigned char)tmp2;
    cmd[5] = (unsigned char)tmp3;
    cmd[6] = (unsigned char)tmp4;
    cmd[7] = (unsigned char)tmp5;
    cmd[13] = fwver[0];
    cmd[14] = fwver[1];
    cmd[15] = fwver[2];
    cmd[16] = fwver[3];
    MakeReadDataCRC(cmd,19);

    MClearRX();
    txsize=19;
    waitAddr = 0x01;
    waitFCode = cmd[1];

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 19);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate WriteVerV3() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 14, delay_time_1); // from uci config
        if ( lpdata ) {
            printf("#### WriteVerV3 OK ####\n");
            SaveLog((char *)"FWupdate WriteVerV3() : OK", st_time);
            return 0;
        } else {
            if ( have_respond ) {
                printf("#### WriteVerV3 CRC Error ####\n");
                SaveLog((char *)"FWupdate WriteVerV3() : CRC Error", st_time);
                ret = 1;
            }
            else {
                printf("#### WriteVerV3 No Response ####\n");
                SaveLog((char *)"FWupdate WriteVerV3() : No Response", st_time);
                ret = -1;
            }
            err++;
        }
    }

    return ret;
}

int RunRegister(char *sn)
{
    printf("######### run RunRegister() #########\n");

    int i = 0, index = -1, err = 0, ret = 0;
    char buf[256] = {0};
    unsigned char *lpdata = NULL;

    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x00, 0xFF, 0x00, 0x01, 0x00, 0x05, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // find match device
    for (i = 0; i < gmicount; i++) {
        if ( strstr(sn, milist[i].SN) != NULL ) {
            index = i;
            break;
        }
    }
    if ( index == -1 ) {
        printf("sn = %s not found in milist\n", sn);
        sprintf(buf, "FWupdate RunRegister() : sn = %s not found in milist", sn);
        SaveLog(buf, st_time);
        printf("######### RunRegister() end #########\n");
        return -1;
    }

    // set sn & crc
    cmd[7]  = milist[index].sn_bin[0];
    cmd[8]  = milist[index].sn_bin[1];
    cmd[9]  = milist[index].sn_bin[2];
    cmd[10] = milist[index].sn_bin[3];
    cmd[11] = milist[index].sn_bin[4];
    cmd[12] = milist[index].sn_bin[5];
    cmd[13] = milist[index].sn_bin[6];
    cmd[14] = milist[index].sn_bin[7];
    cmd[15] = (unsigned char)milist[index].slave_id;
    cmd[16] = (unsigned char)milist[index].slave_id;
    MakeReadDataCRC(cmd,19);

    MClearRX();
    txsize=19;
    waitAddr = 0x00;
    waitFCode = 0xFF;

    // register
    printf("index = %d\n", index);
    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 19);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        lpdata = GetRespond(gcomportfd, 8, delay_time_2); // from uci config
        if ( lpdata ) {
            printf("#### RunRegister OK ####\n");
            gV2id = milist[index].slave_id;
            printf("gV2id = %d\n", gV2id);
            SaveLog((char *)"FWupdate RunRegister() : OK", st_time);
            return 0;
        } else {
            if ( have_respond ) {
                printf("#### RunRegister CRC Error ####\n");
                SaveLog((char *)"FWupdate RunRegister() : CRC Error", st_time);
                ret = 1;
                gV2id = -1;
            }
            else {
                printf("#### RunRegister No Response ####\n");
                SaveLog((char *)"FWupdate RunRegister() : No Response", st_time);
                ret = -2;
                gV2id = -1;
            }
            err++;
        }
    }

    printf("######### RunRegister() end #########\n");

    return ret;
}

int RunStopBat(int loop, int slaveid)
{
    printf("\n#### RunStopBat start ####\n");

    int err = 0, ret = 0, i = 0;
    char buf[256] = {0}, log[1024] = {0};
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x00, 0x34, 0xFF, 0xFF, 0x00, 0x02, 0x04, 0x42, 0x4D, 0x53, 0x54, 0x00, 0x00};

    // set slave id
    cmd[0] = (unsigned char)slaveid;
    MakeReadDataCRC(cmd,13);

    MClearRX();
    txsize=13;
    waitAddr = cmd[0];
    waitFCode = 0x34;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 13);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate RunStopBat() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 8, delay_time_2); // from uci config
        // save debug log
        current_time = time(NULL);
        st_time = localtime(&current_time);
        if ( have_respond && (lpdata != NULL) ) {
            sprintf(log, "FWupdate RunStopBat() get :");
            for (i = 0; i < 8; i++) {
                sprintf(buf, " %02X", lpdata[i]);
                strcat(log, buf);
            }
            SaveLog(log, st_time);
        }
        if ( lpdata ) {
            // check result
            if ( (lpdata[4] == 0) && (lpdata[5] == 0) ) {
                // no battery
                printf("#### RunStopBat No Battery ####\n");
                SaveLog((char *)"FWupdate RunStopBat() : No Battery", st_time);
                ret = 1;
                snlist[loop].status = 11;
            } else if ( (lpdata[4] == 0xFF) && (lpdata[5] == 0xFF) ) {
                // manufacturer not match
                printf("#### RunStopBat manufacturer not match ####\n");
                SaveLog((char *)"FWupdate RunStopBat() : manufacturer not match", st_time);
                ret = 2;
                snlist[loop].status = 12;
            } else if ( (lpdata[4] == 0) && (lpdata[5] == 2) ){
                printf("#### RunStopBat OK ####\n");
                SaveLog((char *)"FWupdate RunStopBat() : OK", st_time);
                ret = 0;
            } else {
                printf("#### RunStopBat unknow status ####\n");
                SaveLog((char *)"FWupdate RunStopBat() : unknow status", st_time);
                ret = 3;
                snlist[loop].status = 13;
            }
            break;
        } else {
            if ( have_respond ) {
                if ( check_respond ) {
                    printf("#### RunStopBat No Response From Battery ####\n");
                    SaveLog((char *)"FWupdate RunStopBat() : No Response From Battery", st_time);
                    ret = -1;
                    snlist[loop].status = 14;
                } else {
                    printf("#### RunStopBat data Error ####\n");
                    SaveLog((char *)"FWupdate RunStopBat() : data Error", st_time);
                    ret = -2;
                    snlist[loop].status = 15;
                }
            }
            else {
                printf("#### RunStopBat No Response From Inverter ####\n");
                SaveLog((char *)"FWupdate RunStopBat() : No Response From Inverter", st_time);
                ret = -3;
                snlist[loop].status = 16;
            }
            err++;
        }
    }

    return ret;
}

int RunStartBat(int loop, int slaveid)
{
    printf("\n#### RunStartBat start ####\n");

    int err = 0, ret = 0, i = 0;
    char buf[256] = {0}, log[1024] = {0};
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x00, 0x34, 0xFF, 0xFE, 0x00, 0x02, 0x04, 0x42, 0x4D, 0x45, 0x44, 0x00, 0x00};

    // set slave id
    cmd[0] = (unsigned char)slaveid;
    MakeReadDataCRC(cmd,13);

    MClearRX();
    txsize=13;
    waitAddr = cmd[0];
    waitFCode = 0x34;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 13);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate RunStartBat() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 8, delay_time_2); // from uci config
        // save debug log
        current_time = time(NULL);
        st_time = localtime(&current_time);
        if ( have_respond && (lpdata != NULL) ) {
            sprintf(log, "FWupdate RunStartBat() get :");
            for (i = 0; i < 8; i++) {
                sprintf(buf, " %02X", lpdata[i]);
                strcat(log, buf);
            }
            SaveLog(log, st_time);
        }
        if ( lpdata ) {
            // check result
            if ( (lpdata[4] == 0) && (lpdata[5] == 0) ) {
                // no battery
                printf("#### RunStartBat No Battery ####\n");
                SaveLog((char *)"FWupdate RunStartBat() : No Battery", st_time);
                ret = 1;
                snlist[loop].status = 17;
            } else if ( (lpdata[4] == 0xFF) && (lpdata[5] == 0xFF) ) {
                // manufacturer not match
                printf("#### RunStartBat manufacturer not match ####\n");
                SaveLog((char *)"FWupdate RunStartBat() : manufacturer not match", st_time);
                ret = 2;
                snlist[loop].status = 18;
            } else if ( (lpdata[4] == 0) && (lpdata[5] == 2) ){
                printf("#### RunStartBat OK ####\n");
                SaveLog((char *)"FWupdate RunStartBat() : OK", st_time);
                ret = 0;
            } else {
                printf("#### RunStartBat unknow status ####\n");
                SaveLog((char *)"FWupdate RunStartBat() : unknow status", st_time);
                ret = 3;
                snlist[loop].status = 19;
            }
            break;
        } else {
            if ( have_respond ) {
                if ( check_respond ) {
                    printf("#### RunStartBat No Response From Battery ####\n");
                    SaveLog((char *)"FWupdate RunStartBat() : No Response From Battery", st_time);
                    ret = -1;
                    snlist[loop].status = 20;
                } else {
                    printf("#### RunStartBat data Error ####\n");
                    SaveLog((char *)"FWupdate RunStartBat() : data Error", st_time);
                    ret = -2;
                    snlist[loop].status = 21;
                }
            }
            else {
                printf("#### RunStartBat No Response From Inverter ####\n");
                SaveLog((char *)"FWupdate RunStartBat() : No Response From Inverter", st_time);
                ret = -3;
                snlist[loop].status = 22;
            }
            err++;
        }
    }

    return ret;
}

int GetBatInfo(int loop, int slaveid, int retry)
{
    printf("\n#### GetBatInfo start ####\n");

    int err = 0, ret = 0, i = 0;
    char buf[256] = {0}, log[1024] = {0};
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x00, 0x33, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00};

    // set slave id
    cmd[0] = (unsigned char)slaveid;
    MakeReadDataCRC(cmd,8);

    MClearRX();
    txsize=8;
    waitAddr = cmd[0];
    waitFCode = 0x33;

    while ( err < retry ) {
        memcpy(txbuffer, cmd, 8);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate GetBatInfo() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 21, delay_time_2); // from uci config
        // save debug log
        current_time = time(NULL);
        st_time = localtime(&current_time);
        if ( have_respond && (lpdata != NULL) ) {
            if ( lpdata ) {
                sprintf(log, "FWupdate GetBatInfo() get :");
                for (i = 0; i < 21; i++) {
                    sprintf(buf, " %02X", lpdata[i]);
                    strcat(log, buf);
                }
                SaveLog(log, st_time);
            } else {
                sprintf(log, "FWupdate GetBatInfo() get :");
                for (i = 0; i < 21; i++) {
                    sprintf(buf, " %02X", respond_buff[i]);
                    strcat(log, buf);
                }
                SaveLog(log, st_time);
            }
        }
        if ( lpdata ) {
            // check result
            if ( (lpdata[2] == 0) && (lpdata[3] == 0) && (lpdata[4] == 0) && (lpdata[5] == 0) ) {
                // no response from bms
                printf("#### GetBatInfo No Response From BMS ####\n");
                SaveLog((char *)"FWupdate GetBatInfo() : No Response From BMS", st_time);
                ret = 1;
                if ( (snlist[loop].status == 0) || ((snlist[loop].status > 22) && (snlist[loop].status < 28)) )
                    snlist[loop].status = 23;
                err++;
            } else if ( lpdata[1] == 0x3B ) {
                // get error code
                sprintf(buf, "FWupdate GetBatInfo() : Get Error Code %d", lpdata[1]);
                printf(buf);
                SaveLog(buf, st_time);
                ret = 2;
                if ( (snlist[loop].status == 0) || ((snlist[loop].status > 22) && (snlist[loop].status < 28)) )
                    snlist[loop].status = 24;
                err++;
            } else {
                printf("#### GetBatInfo OK ####\n");
                SaveLog((char *)"FWupdate GetBatInfo() : OK", st_time);
                ret = 0;
                if ( (snlist[loop].status == 0) || ((snlist[loop].status > 22) && (snlist[loop].status < 28)) )
                    snlist[loop].status = 0;
                mybatinfo.boot_ver = (lpdata[3] << 8) + lpdata[4];
                mybatinfo.app_ver = (lpdata[5] << 8) + lpdata[6];
                mybatinfo.backup_ver = (lpdata[7] << 8) + lpdata[8];
                mybatinfo.update_ver = (lpdata[9] << 8) + lpdata[10];
                mybatinfo.status = (lpdata[11] << 8) + lpdata[12];
                mybatinfo.burn_status = (lpdata[13] << 8) + lpdata[14];
                mybatinfo.section = (lpdata[15] << 8) + lpdata[16];
                mybatinfo.BMS_number = (lpdata[17] << 8) + lpdata[18];
                break;
            }
        } else {
            if ( have_respond ) {
                if ( check_respond ) {
                    printf("#### GetBatInfo No Response From Battery ####\n");
                    SaveLog((char *)"FWupdate GetBatInfo() : No Response From Battery", st_time);
                    ret = -1;
                    if ( (snlist[loop].status == 0) || ((snlist[loop].status > 22) && (snlist[loop].status < 28)) )
                        snlist[loop].status = 25;
                } else {
                    printf("#### GetBatInfo data Error ####\n");
                    SaveLog((char *)"FWupdate GetBatInfo() : data Error", st_time);
                    ret = -2;
                    if ( (snlist[loop].status == 0) || ((snlist[loop].status > 22) && (snlist[loop].status < 28)) )
                        snlist[loop].status = 26;
                }
            }
            else {
                printf("#### GetBatInfo No Response From Inverter####\n");
                SaveLog((char *)"FWupdate GetBatInfo() : No Response From Inverter", st_time);
                ret = -3;
                if ( (snlist[loop].status == 0) || ((snlist[loop].status > 22) && (snlist[loop].status < 28)) )
                    snlist[loop].status = 27;
            }
            err++;
        }
    }

    return ret;
}

int GetBatVer(int loop, int slaveid)
{
    printf("\n#### GetBatVer start ####\n");

    int err = 0, ret = 0, i = 0;
    char buf[256] = {0}, log[1024] = {0};
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x00, 0x03, 0x02, 0x71, 0x00, 0x01, 0x00, 0x00};

    // set slave id
    cmd[0] = (unsigned char)slaveid;
    MakeReadDataCRC(cmd,8);

    MClearRX();
    txsize=8;
    waitAddr = cmd[0];
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 8);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate GetBatVer() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 7, delay_time_2); // from uci config
        // save debug log
        current_time = time(NULL);
        st_time = localtime(&current_time);
        if ( have_respond && (lpdata != NULL) ) {
            if ( lpdata ) {
                sprintf(log, "FWupdate GetBatVer() get :");
                for (i = 0; i < 7; i++) {
                    sprintf(buf, " %02X", lpdata[i]);
                    strcat(log, buf);
                }
                SaveLog(log, st_time);
            } else {
                sprintf(log, "FWupdate GetBatVer() get :");
                for (i = 0; i < 7; i++) {
                    sprintf(buf, " %02X", respond_buff[i]);
                    strcat(log, buf);
                }
                SaveLog(log, st_time);
            }
        }
        if ( lpdata ) {
            // check result
            printf("#### GetBatVer OK ####\n");
            printf("BatVer = 0x%04X\n", (lpdata[3] << 8) + lpdata[4]);
            SaveLog((char *)"FWupdate GetBatVer() : OK", st_time);
            ret = 0;
            break;
        } else {
            if ( have_respond ) {
                if ( check_respond ) {
                    printf("#### GetBatVer No Response From Battery ####\n");
                    SaveLog((char *)"FWupdate GetBatVer() : No Response From Battery", st_time);
                    ret = -1;
                } else {
                    printf("#### GetBatVer data Error ####\n");
                    SaveLog((char *)"FWupdate GetBatVer() : data Error", st_time);
                    ret = -2;
                }
            }
            else {
                printf("#### GetBatVer No Response From Inverter####\n");
                SaveLog((char *)"FWupdate GetBatVer() : No Response From Inverter", st_time);
                ret = -3;
            }
            err++;
        }
    }

    return ret;
}

int GetSlaveInfo(int loop, int slaveid, int number)
{
    printf("\n#### GetSlaveInfo start ####\n");

    int err = 0, ret = 0, i = 0, flag = 0;
    char buf[256] = {0}, log[1024] = {0};
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x00, 0x33, 0x0B, 0xB8, 0x00, 0x00, 0x00, 0x00};

    // set slave id
    cmd[0] = (unsigned char)slaveid;
    cmd[5] = 6*number;
    MakeReadDataCRC(cmd,8);

    MClearRX();
    txsize=8;
    waitAddr = cmd[0];
    waitFCode = 0x33;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 8);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate GetSlaveInfo() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 12*number+5, delay_time_2); // from uci config
        // save debug log
        current_time = time(NULL);
        st_time = localtime(&current_time);
        if ( have_respond && (lpdata != NULL) ) {
            if ( lpdata ) {
                sprintf(log, "FWupdate GetSlaveInfo() get :");
                for (i = 0; i < 12*number+5; i++) {
                    sprintf(buf, " %02X", lpdata[i]);
                    strcat(log, buf);
                }
                SaveLog(log, st_time);
            } else {
                sprintf(log, "FWupdate GetSlaveInfo() get :");
                for (i = 0; i < 12*number+5; i++) {
                    sprintf(buf, " %02X", respond_buff[i]);
                    strcat(log, buf);
                }
                SaveLog(log, st_time);
            }
        }
        if ( lpdata ) {
            // check result
            if ( (lpdata[2] == 0) && (lpdata[3] == 0) && (lpdata[4] == 0) && (lpdata[5] == 0) ) {
                // no response from bms
                printf("#### GetSlaveInfo No Response From BMS ####\n");
                SaveLog((char *)"FWupdate GetSlaveInfo() : No Response From BMS", st_time);
                ret = 1;
                err++;
            } else if ( lpdata[1] == 0x3B ) {
                // get error code
                sprintf(buf, "FWupdate GetSlaveInfo() : Get Error Code %d", lpdata[1]);
                printf(buf);
                SaveLog(buf, st_time);
                ret = 2;
                err++;
            } else {
                printf("#### GetSlaveInfo OK ####\n");
                printf("#### Check slave ver ####\n");
                for (i = 0; i < number; i++) {
                    // check app ver
                    if ( snlist[loop].ver != ((lpdata[i*12+5]<<8) + lpdata[i*12+6]) ) {
                        sprintf(buf, "FWupdate GetSlaveInfo() : %d app ver 0x%02X%02X not match", i, lpdata[i*12+5], lpdata[i*12+6]);
                        printf(buf);
                        printf("\n");
                        SaveLog(buf, st_time);
                        flag++;
                    }

                    // check backup ver
                    if ( snlist[loop].ver != ((lpdata[i*12+7]<<8) + lpdata[i*12+8]) ) {
                        sprintf(buf, "FWupdate GetSlaveInfo() : %d backup ver 0x%02X%02X not match", i, lpdata[i*12+7], lpdata[i*12+8]);
                        printf(buf);
                        printf("\n");
                        SaveLog(buf, st_time);
                        flag++;
                    }
                }
                // check result
                if ( flag == 0 ) {
                    printf("#### GetSlaveInfo Check slave ver OK ####\n");
                    SaveLog((char *)"FWupdate GetSlaveInfo() : Check slave ver OK", st_time);
                    ret = 0;
                } else {
                    printf("#### GetSlaveInfo Check slave ver Fail ####\n");
                    SaveLog((char *)"FWupdate GetSlaveInfo() : Check slave ver Fail", st_time);
                    ret = 3;
                }
                //SaveLog((char *)"FWupdate GetSlaveInfo() : OK", st_time);
                break;
            }
        } else {
            if ( have_respond ) {
                if ( check_respond ) {
                    printf("#### GetSlaveInfo No Response From Battery ####\n");
                    SaveLog((char *)"FWupdate GetSlaveInfo() : No Response From Battery", st_time);
                    ret = -1;
                } else {
                    printf("#### GetSlaveInfo data Error ####\n");
                    SaveLog((char *)"FWupdate GetSlaveInfo() : data Error", st_time);
                    ret = -2;
                }
            }
            else {
                printf("#### GetSlaveInfo No Response From Inverter####\n");
                SaveLog((char *)"FWupdate GetSlaveInfo() : No Response From Inverter", st_time);
                ret = -3;
            }
            err++;
        }
    }

    return ret;
}

int SetControl(int loop, int slaveid, unsigned char value, int retry)
{
    printf("\n#### SetControl start ####\n");

    int err = 0, ret = 0, i = 0;
    char buf[256] = {0}, log[1024] = {0};
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x00, 0x34, 0x02, 0x58, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00};

    // set slave id
    cmd[0] = (unsigned char)slaveid;
    cmd[8] = value;
    MakeReadDataCRC(cmd,11);

    MClearRX();
    txsize=11;
    waitAddr = cmd[0];
    waitFCode = 0x34;

    while ( err < retry ) {
        memcpy(txbuffer, cmd, 11);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate SetControl() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 8, delay_time_2); // from uci config
        // save debug log
        current_time = time(NULL);
        st_time = localtime(&current_time);
        if ( have_respond && (lpdata != NULL) ) {
            sprintf(log, "FWupdate SetControl() get :");
            for (i = 0; i < 8; i++) {
                sprintf(buf, " %02X", lpdata[i]);
                strcat(log, buf);
            }
            SaveLog(log, st_time);
        }
        if ( lpdata ) {
            // check result
            if ( (lpdata[2] == 0) && (lpdata[3] == 0) && (lpdata[4] == 0) && (lpdata[5] == 0) ) {
                // no response from bms
                printf("#### SetControl No Response From BMS ####\n");
                SaveLog((char *)"FWupdate SetControl() : No Response From BMS", st_time);
                ret = 1;
                snlist[loop].status = 28;
            } else if ( lpdata[1] == 0x3C ) {
                // get error code
                sprintf(buf, "FWupdate SetControl() : Get Error Code %d", lpdata[1]);
                printf(buf);
                SaveLog(buf, st_time);
                ret = 2;
                snlist[loop].status = 29;
            } else {
                printf("#### SetControl OK ####\n");
                SaveLog((char *)"FWupdate SetControl() : OK", st_time);
                ret = 0;
            }
            break;
        } else {
            if ( have_respond ) {
                if ( check_respond ) {
                    printf("#### SetControl No Response From Battery ####\n");
                    SaveLog((char *)"FWupdate SetControl() : No Response From Battery", st_time);
                    ret = -1;
                    snlist[loop].status = 30;
                } else {
                    printf("#### SetControl data Error ####\n");
                    SaveLog((char *)"FWupdate SetControl() : data Error", st_time);
                    ret = -2;
                    snlist[loop].status = 31;
                }
            }
            else {
                printf("#### SetControl No Response From Inverter####\n");
                SaveLog((char *)"FWupdate SetControl() : No Response From Inverter", st_time);
                ret = -3;
                snlist[loop].status = 32;
            }
            err++;
        }
    }

    return ret;
}

int SetHeader(int loop, int slaveid, unsigned char *header)
{
    printf("\n#### SetHeader start ####\n");

    int err = 0, ret = 0, i = 0;
    char buf[256] = {0}, log[1024] = {0};
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[31]={0};
    //{0x00, 0x34, 0x00, 0x0A, 0x00, 0x0B, 0x16, 0x00, 0x00, 0x00, 0x00};

    // set slave id
    cmd[0] = (unsigned char)slaveid;
    cmd[1] = 0x34;
    cmd[2] = 0x00;
    cmd[3] = 0x0A;
    cmd[4] = 0x00;
    cmd[5] = 0x0B;
    cmd[6] = 0x16;
    // set 2.customer info
    cmd[7] = header[2];
    cmd[8] = header[3];
    cmd[9] = header[4];
    cmd[10] = header[5];
    cmd[11] = header[6];
    cmd[12] = header[7];
    cmd[13] = header[8];
    cmd[14] = header[9];
    cmd[15] = header[10];
    cmd[16] = header[11];
    // set 3.model info
    cmd[17] = header[12];
    cmd[18] = header[13];
    cmd[19] = header[14];
    cmd[20] = header[15];
    cmd[21] = header[16];
    cmd[22] = header[17];
    // set 4.ver info
    cmd[23] = header[18];
    cmd[24] = header[19];
    // set 7.section number
    cmd[25] = header[28];
    cmd[26] = header[29];
    // set 9.crc
    cmd[27] = header[32];
    cmd[28] = header[33];
    MakeReadDataCRC(cmd,31);

    MClearRX();
    txsize=31;
    waitAddr = cmd[0];
    waitFCode = 0x34;

    while ( err < 60 ) {
        memcpy(txbuffer, cmd, 31);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate SetHeader() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 8, delay_time_2); // from uci config
        // save debug log
        current_time = time(NULL);
        st_time = localtime(&current_time);
        if ( have_respond && (lpdata != NULL) ) {
            sprintf(log, "FWupdate SetHeader() get :");
            for (i = 0; i < 8; i++) {
                sprintf(buf, " %02X", lpdata[i]);
                strcat(log, buf);
            }
            SaveLog(log, st_time);
        }
        if ( lpdata ) {
            // check result
            if ( (lpdata[2] == 0) && (lpdata[3] == 0) && (lpdata[4] == 0) && (lpdata[5] == 0) ) {
                // no response from bms
                printf("#### SetHeader No Response From BMS ####\n");
                SaveLog((char *)"FWupdate SetHeader() : No Response From BMS", st_time);
                ret = 1;
                snlist[loop].status = 33;
            } else if ( lpdata[1] == 0x3C ) {
                // get error code
                sprintf(buf, "FWupdate SetHeader() : Get Error Code %d", lpdata[1]);
                printf(buf);
                SaveLog(buf, st_time);
                ret = 2;
                snlist[loop].status = 34;
            } else {
                printf("#### SetHeader OK ####\n");
                SaveLog((char *)"FWupdate SetHeader() : OK", st_time);
                ret = 0;
            }
            break;
        } else {
            if ( have_respond ) {
                if ( check_respond ) {
                    printf("#### SetHeader No Response From Battery ####\n");
                    SaveLog((char *)"FWupdate SetHeader() : No Response From Battery", st_time);
                    ret = -1;
                    snlist[loop].status = 35;
                } else {
                    printf("#### SetHeader data Error ####\n");
                    SaveLog((char *)"FWupdate SetHeader() : data Error", st_time);
                    ret = -2;
                    snlist[loop].status = 36;
                }
            }
            else {
                printf("#### SetHeader No Response From Inverter####\n");
                SaveLog((char *)"FWupdate SetHeader() : No Response From Inverter", st_time);
                ret = -3;
                snlist[loop].status = 37;
            }
            err++;
        }
    }

    return ret;
}

int StopBatFWUpdate(int loop, int slaveid)
{
    printf("\n#### StopBatFWUpdate start ####\n");

    int ret = 0;

    time_t      start_time = 0, current_time = 0;

    start_time = time(NULL);
    printf("start_time = %ld\n", start_time);

    while (1) {
        memset(&mybatinfo, 0x00, sizeof(mybatinfo));
        ret = GetBatInfo(loop, slaveid, 1);
        //if (ret)
        //    break;

        if ( mybatinfo.status == 3 ) {
            printf("status = 3, end.\n");
            break;
        } else if ( mybatinfo.status == 0 ) {
            printf("status = %d, burn_status = %d, wait!\n", mybatinfo.status, mybatinfo.burn_status);
        } else if ( mybatinfo.status == 1 ) {
            printf("status = %d, send 0xE3\n", mybatinfo.status);
            ret = SetControl(loop, slaveid, 0xE3, 1);
        } else if ( mybatinfo.status == 15 ) {
            printf("status = %d, send 0xE2\n", mybatinfo.status);
            ret = SetControl(loop, slaveid, 0xE2, 1);
        } else if ( mybatinfo.status == 2 ) {
            if ( mybatinfo.burn_status == 1 || mybatinfo.burn_status == 5 || mybatinfo.burn_status == 7 || mybatinfo.burn_status == 10 || mybatinfo.burn_status == 13 ) {
                printf("status = %d, burn_status = %d, send 0xE2\n", mybatinfo.status, mybatinfo.burn_status);
                ret = SetControl(loop, slaveid, 0xE2, 1);
            } else if ( mybatinfo.burn_status == 8 || mybatinfo.burn_status == 9 || mybatinfo.burn_status == 11 || mybatinfo.burn_status == 12 ) {
                printf("status = %d, burn_status = %d, wait!\n", mybatinfo.status, mybatinfo.burn_status);
            }
        }

        current_time = time(NULL);
        printf("current_time = %ld\n", current_time);
        printf("%ld sec. passed\n", current_time - start_time);
        //if ( current_time - start_time >= (300*mybatinfo.BMS_number) ) {
        if ( current_time - start_time >= 900 ) {
            printf("Time out! end.\n");
            SetControl(loop, slaveid, 0xE2, 1);
            ret = 10;
            break;
        }

        usleep(3000000); // 1 sec.

    }

    return ret;
}

int CheckResult(int loop, int slaveid)
{
    printf("\n#### CheckResult start ####\n");

    int ret = 0;
    char buf[256] = {0};

    time_t      start_time = 0, current_time = 0;
    struct tm   *st_time = NULL;

    start_time = time(NULL);
    printf("start_time = %ld\n", start_time);

    while (1) {
        memset(&mybatinfo, 0x00, sizeof(mybatinfo));
        ret = GetBatInfo(loop, slaveid, 1);

        current_time = time(NULL);
        st_time = localtime(&current_time);

        if ( ret == 0 ) {
        //if ( mybatinfo.status == 3 ) {
            if ( snlist[loop].ver == mybatinfo.app_ver && snlist[loop].ver == mybatinfo.backup_ver ) {
                // check slave info
                if ( mybatinfo.BMS_number > 1 ) {
                    ret = GetSlaveInfo(loop, slaveid, mybatinfo.BMS_number-1);
                    if ( ret == 0 ) {
                        snlist[loop].status = 0;
                        sprintf(buf, "FWupdate CheckResult() : ver 0x%04X update ok", snlist[loop].ver);
                        printf(buf);
                        printf("\n");
                        SaveLog(buf, st_time);
                        return 0;
                    } else {
                        snlist[loop].status = 58;
                        printf("wait!\n");
                    }
                } else {
                    snlist[loop].status = 0;
                    sprintf(buf, "FWupdate CheckResult() : ver 0x%04X update ok", snlist[loop].ver);
                    printf(buf);
                    printf("\n");
                    SaveLog(buf, st_time);
                    return 0;
                }
            } else {
                snlist[loop].status = 53;
                sprintf(buf, "FWupdate CheckResult() : ver 0x%04X update fail, app 0x%04X, backup 0x%04X", snlist[loop].ver, mybatinfo.app_ver, mybatinfo.backup_ver);
                printf(buf);
                printf("\n");
                SaveLog(buf, st_time);
            }
        //} else if ( mybatinfo.status == 0 || mybatinfo.status == 1 ) {
        //    printf("status = %d, send 0xE3\n", mybatinfo.status);
        //    SetControl(loop, slaveid, 0xE3, 1);
        //} else if ( mybatinfo.status == 2 ) {
        //    printf("status = %d, send 0xE2\n", mybatinfo.status);
        //    SetControl(loop, slaveid, 0xE2, 1);
        //}
        }

        current_time = time(NULL);
        printf("current_time = %ld\n", current_time);
        printf("%ld sec. passed\n", current_time - start_time);
        if ( current_time - start_time >= 60 ) {
            printf("Time out! end.\n");
            sprintf(buf, "FWupdate CheckResult() : ver 0x%04X update fail", snlist[loop].ver);
            printf(buf);
            printf("\n");
            SaveLog(buf, st_time);
            return 1;
        }

        usleep(3000000); // 3 sec.
    }

    return 2;

}

int RunEnableP3(int slaveid)
{
    printf("\n#### RunEnableP3 start ####\n");

    int err = 0, ret = 0;
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x00, 0x05, 0x10, 0x00, 0xFF, 0x00, 0x00, 0x00};

    // set slave id
    cmd[0] = (unsigned char)slaveid;
    MakeReadDataCRC(cmd,8);

    MClearRX();
    txsize=8;
    waitAddr = cmd[0];
    waitFCode = 0x05;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 8);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        lpdata = GetRespond(gcomportfd, 8, delay_time_2); // from uci config
        if ( lpdata ) {
            printf("#### RunEnableP3 OK ####\n");
            SaveLog((char *)"FWupdate RunEnableP3() : OK", st_time);
            return 0;
        } else {
            if ( have_respond ) {
                printf("#### RunEnableP3 CRC Error ####\n");
                SaveLog((char *)"FWupdate RunEnableP3() : CRC Error", st_time);
                ret = 1;
            }
            else {
                printf("#### RunEnableP3 No Response ####\n");
                SaveLog((char *)"FWupdate RunEnableP3() : No Response", st_time);
                ret = -1;
            }
            err++;
        }
    }

    return ret;
}

int RunShutdown(int slaveid)
{
    printf("\n#### RunShutdown start ####\n");

    int err = 0, ret = 0;
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x00, 0x05, 0x10, 0x01, 0xFF, 0x00, 0x00, 0x00};

    // set slave id
    cmd[0] = (unsigned char)slaveid;
    MakeReadDataCRC(cmd,8);

    MClearRX();
    txsize=8;
    waitAddr = cmd[0];
    waitFCode = 0x05;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 8);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        lpdata = GetRespond(gcomportfd, 8, delay_time_2); // from uci config
        if ( lpdata ) {
            printf("#### RunShutdown OK ####\n");
            SaveLog((char *)"FWupdate RunShutdown() : OK", st_time);
            return 0;
        } else {
            if ( have_respond ) {
                printf("#### RunShutdown CRC Error ####\n");
                SaveLog((char *)"FWupdate RunShutdown() : CRC Error", st_time);
                ret = 1;
            }
            else {
                printf("#### RunShutdown No Response ####\n");
                SaveLog((char *)"FWupdate RunShutdown() : No Response", st_time);
                ret = -1;
            }
            err++;
        }
    }

    return ret;
}

int RunReboot(int slaveid)
{
    printf("\n#### RunReboot start ####\n");

    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x00, 0x05, 0x10, 0x10, 0xFF, 0x00, 0x00, 0x00};

    // set slave id
    cmd[0] = (unsigned char)slaveid;
    MakeReadDataCRC(cmd,8);

    MClearRX();
    txsize=8;
    waitAddr = cmd[0];
    waitFCode = 0x05;

    memcpy(txbuffer, cmd, 8);
    MStartTX(gcomportfd);
    usleep(1000000); // 1s

    SaveLog((char *)"FWupdate RunReboot() : Send", st_time);

    printf("\n#### RunReboot end ####\n");

    return 0;
}

int RunRebootSpecify(char *sn)
{
    printf("\n#### RunRebootSpecify start ####\n");

    int err = 0, ret = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0, i = 0;
    char buf[256] = {0}, log[1024] = {0};
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x01, 0x45, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00};

    sscanf(sn, "%06s%02X%02X%02X%02X%02X", buf, &tmp1, &tmp2, &tmp3, &tmp4, &tmp5);
    // set model & SN
    cmd[3] = (unsigned char)tmp1;
    cmd[4] = (unsigned char)tmp2;
    cmd[5] = (unsigned char)tmp3;
    cmd[6] = (unsigned char)tmp4;
    cmd[7] = (unsigned char)tmp5;
    // set crc
    MakeReadDataCRC(cmd,15);

    MClearRX();
    txsize=15;
    waitAddr = cmd[0];
    waitFCode = 0x45;

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 15);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate RunRebootSpecify() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 14, delay_time_1); // from uci config
        if ( lpdata ) {
            printf("#### RunRebootSpecify OK ####\n");
            SaveLog((char *)"FWupdate RunRebootSpecify() : OK", st_time);
            return 0;
        } else {
            if ( have_respond ) {
                printf("#### RunRebootSpecify CRC Error ####\n");
                SaveLog((char *)"FWupdate RunRebootSpecify() : CRC Error", st_time);
                ret = 1;
            }
            else {
                printf("#### RunRebootSpecify No Response ####\n");
                SaveLog((char *)"FWupdate RunRebootSpecify() : No Response", st_time);
                ret = -1;
            }
            err++;
        }
    }

    return ret;
}

int LBDReregister(char *sn)
{
    printf("\n#### LBDReregister start ####\n");

    int err = 0, ret = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0, i = 0;
    char buf[256] = {0}, log[1024] = {0};
    unsigned char *lpdata = NULL;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x01, 0x4B, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00};

    sscanf(sn, "%06s%02X%02X%02X%02X%02X", buf, &tmp1, &tmp2, &tmp3, &tmp4, &tmp5);
    // set model & SN
    cmd[3] = (unsigned char)tmp1;
    cmd[4] = (unsigned char)tmp2;
    cmd[5] = (unsigned char)tmp3;
    cmd[6] = (unsigned char)tmp4;
    cmd[7] = (unsigned char)tmp5;
    // set crc
    MakeReadDataCRC(cmd,15);

    MClearRX();
    txsize=15;
    waitAddr  = cmd[0];
    waitFCode = cmd[1];

    while ( err < 2 ) {
        memcpy(txbuffer, cmd, 15);
        MStartTX(gcomportfd);
        usleep(1000000); // 1s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate LBDReregister() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 14, delay_time_1); // from uci config
        if ( lpdata ) {
            printf("#### LBDReregister OK ####\n");
            SaveLog((char *)"FWupdate LBDReregister() : OK", st_time);
            return 0;
        } else {
            if ( have_respond ) {
                printf("#### LBDReregister CRC Error ####\n");
                SaveLog((char *)"FWupdate LBDReregister() : CRC Error", st_time);
                ret = 1;
            }
            else {
                printf("#### LBDReregister No Response ####\n");
                SaveLog((char *)"FWupdate LBDReregister() : No Response", st_time);
                ret = -1;
            }
            cmd[1] = 0x4D;
            MakeReadDataCRC(cmd,15);
            err++;
        }
    }

    return ret;
}

int WriteDataV2(int slaveid, unsigned char *fwdata, int datasize)
{
    printf("\n#### WriteDataV2 start ####\n");

    int i = 0, err = 0, index = 0, numofdata = MAX_DATA_SIZE/2, writesize = MAX_DATA_SIZE, address = 0, cnt = 0, end = 0;
    unsigned char addrh = 0, addrl = 0;
    unsigned char *lpdata = NULL;
    char buf[256] = {0};
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[MAX_DATA_SIZE+9]={0};
    // set slave id
    cmd[0] = (unsigned char)slaveid;
    // set function code
    cmd[1] = 0x10;

    // start write data loop
    while ( index < datasize ) {
        // check data size
        if ( (index + writesize) > datasize ) {
            writesize = datasize - index;
            numofdata = writesize/2;
        }

        // set addr
        addrh = (address>>8) & 0xFF;
        addrl = address & 0xFF;
        cmd[2] = addrh;
        cmd[3] = addrl;
        // set number of data, max 0x48 (dec.72), so hi always 0
        cmd[4] = 0;
        cmd[5] = (unsigned char)numofdata;
        // set byte count, max 0x90 (dec.144)
        cmd[6] = (unsigned char)writesize;

        // set data to buf
        for (i = 0; i < writesize; i++) {
            cmd[7+i] = fwdata[index+i];
        }

        // set crc
        MakeReadDataCRC(cmd, writesize+9);

        MClearRX();
        txsize = writesize+9;
        waitAddr = cmd[0];
        waitFCode = 0x10;

        while ( err < 3 ) {
            memcpy(txbuffer, cmd, txsize);
            MStartTX(gcomportfd);
            //usleep(10000); // 0.01s

            current_time = time(NULL);
            st_time = localtime(&current_time);

            lpdata = GetRespond(gcomportfd, 8, delay_time_2); // from uci config
            if ( lpdata ) {
                cnt++;
                printf("#### WriteDataV2 data count %d, index 0x%X, size %d OK ####\n", cnt, index, writesize);
                sprintf(buf, "FWupdate WriteDataV2() : write count %d, index 0x%X, size %d OK", cnt, index, writesize);
                SaveLog(buf, st_time);

                index+=writesize;
                address+=numofdata;

                break;
            } else {
                err++;
                printf("#### WriteDataV2 GetRespond Error %d ####\n", err);
                if ( err == 3 ) {
                    if ( have_respond ) {
                        printf("#### WriteDataV2 CRC Error ####\n");
                        SaveLog((char *)"FWupdate WriteDataV2() : CRC Error", st_time);
                    } else {
                        printf("#### WriteDataV2 No Response ####\n");
                        SaveLog((char *)"FWupdate WriteDataV2() : No Response", st_time);
                    }

                    // check data size
                    if ( numofdata > 0x08 ) {
                        numofdata-=0x08;
                        writesize = numofdata*2;
                        printf("set numofdata = 0x%X, writesize = %d\n", numofdata, writesize);
                        err = 0;
                        break;
                    } else {
                        printf("numofdata = 0x%X, too small so end this loop\n", numofdata);
                        end = 1;
                        break;
                    }
                }
            }
        }

        if ( end )
            break;
    }

    // send reboot cmd
    RunReboot(slaveid);

    printf("\n#### WriteDataV2 end ####\n");

    if ( index == datasize )
        return 0;
    else
        return 1;
}

int WriteHBData(int slaveid, unsigned char *fwdata, int datasize)
{
    printf("\n#### WriteHBData start ####\n");

    //int i = 0, err = 0, index = 0, numofdata = MAX_HYBRID_SIZE/2, writesize = MAX_HYBRID_SIZE, address = 0, cnt = 0, end = 0;
    int i = 0, err = 0, index = 0, numofdata = 0, writesize = 0, address = 0, cnt = 0, end = 0, ret = 0, retry = 0;
    unsigned char addrh = 0, addrl = 0;
    unsigned char *lpdata = NULL;
    char buf[256] = {0}, log[1024] = {0};
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[MAX_HYBRID_SIZE+9]={0};
    // set first write command
    // set slave id
    cmd[0] = (unsigned char)slaveid;
    // set function code
    cmd[1] = 0x10;
    // set addr
    cmd[2] = 0xFF;
    cmd[3] = 0xFF;
    // set no. of data
    cmd[4] = 0x00;
    cmd[5] = 0x04;
    // set byte count
    cmd[6] = 0x08;
    // set data (file header 2 + check sum 2 + byte count 4)
    cmd[7] = fwdata[0];
    cmd[8] = fwdata[1];
    cmd[9] = fwdata[2];
    cmd[10] = fwdata[3];
    cmd[11] = fwdata[4];
    cmd[12] = fwdata[5];
    cmd[13] = fwdata[6];
    cmd[14] = fwdata[7];
    MakeReadDataCRC(cmd,17);

    MClearRX();
    txsize = 17;
    waitAddr = cmd[0];
    waitFCode = cmd[1];

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 17);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate WriteHBData() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 8, delay_time_2); // uci setting
        // save debug log
        if ( have_respond && (lpdata != NULL) ) {
            current_time = time(NULL);
            st_time = localtime(&current_time);

            sprintf(log, "FWupdate WriteHBData() get :");
            for (i = 0; i < 8; i++) {
                sprintf(buf, " %02X", lpdata[i]);
                strcat(log, buf);
            }
            SaveLog(log, st_time);
        }
        if ( lpdata ) {
            if ( (lpdata[4] == 0xFF) && (lpdata[5] == 0xF0) ) {
                // response the same FW data
                printf("the same FW data\n");
                SaveLog((char *)"FWupdate WriteHBData() : The same FW data", st_time);
                ret = 4;
                break;
            } else if ( (lpdata[4] == 0x00) && (lpdata[5] == 0x04) ) {
                printf("#### WriteHBData Header OK ####\n");
                SaveLog((char *)"FWupdate WriteHBData() : Send file header OK", st_time);
                ret = 0;
                break;
            } else {
                printf("#### Unknow No. of data 0x%02x%02x ####\n", lpdata[4], lpdata[5]);
                ret = 7;
                break;
            }
        } else {
            if ( have_respond ) {
                printf("#### WriteHBData Send file header CRC Error ####\n");
                SaveLog((char *)"FWupdate WriteHBData() : Send file header CRC Error", st_time);
                ret = 1;
            }
            else {
                printf("#### WriteHBData Send file header No Response ####\n");
                SaveLog((char *)"FWupdate WriteHBData() : Send file header No Response", st_time);
                ret = -1;
            }
            err++;
        }
    }
    if ( ret ) {
        return ret;
    }

    // set write data part
    index = 8;
    address = 0x4004;
    writesize = MAX_HYBRID_SIZE;
    numofdata = writesize/2;

    while ( index < datasize ) {
        // check data size
        if ( (index + writesize) > datasize ) {
            writesize = datasize - index;
            numofdata = writesize/2;
        }

        // set addr
        addrh = (unsigned char)((address>>8) & 0xFF);
        addrl = (unsigned char)(address & 0xFF);
        cmd[2] = addrh;
        cmd[3] = addrl;
        // set number of data, max 0x32 (dec.50), so hi always 0
        cmd[4] = 0;
        cmd[5] = (unsigned char)numofdata;
        // set byte count, max 0x64 (dec.100)
        cmd[6] = (unsigned char)writesize;

        // set data to buf
        for (i = 0; i < writesize; i++) {
            cmd[7+i] = fwdata[index+i];
        }

        // set crc
        MakeReadDataCRC(cmd, writesize+9);

        MClearRX();
        txsize = writesize+9;
        waitAddr = cmd[0];
        waitFCode = cmd[1];

        while ( err < 3 ) {
            memcpy(txbuffer, cmd, txsize);
            MStartTX(gcomportfd);
            //usleep(10000); // 0.01s

            current_time = time(NULL);
            st_time = localtime(&current_time);

            sprintf(log, "FWupdate WriteHBData() send :");
            for (i = 0; i < txsize; i++) {
                sprintf(buf, " %02X", cmd[i]);
                strcat(log, buf);
            }
            SaveLog(log, st_time);

            lpdata = GetRespond(gcomportfd, 8, delay_time_2); // uci setting
            current_time = time(NULL);
            st_time = localtime(&current_time);
            // save debug log
            if ( have_respond && (lpdata != NULL) ) {
                sprintf(log, "FWupdate WriteHBData() get :");
                for (i = 0; i < 8; i++) {
                    sprintf(buf, " %02X", lpdata[i]);
                    strcat(log, buf);
                }
                SaveLog(log, st_time);
            }
            if ( lpdata ) {
                //lpdata = cmd; // for test
                if ( (lpdata[2] == addrh) && (lpdata[3] == addrl) && (lpdata[4] == 00) && (lpdata[5] == numofdata) ) {
                    cnt++;
                    printf("#### WriteHBData data count %d, addr 0x%X, index 0x%X, size %d OK ####\n", cnt, address, index, writesize);
                    sprintf(buf, "FWupdate WriteHBData() : write count %d, addr 0x%X, index 0x%X, size %d OK", cnt, address, index, writesize);
                    SaveLog(buf, st_time);

                    index+=writesize;
                    address+=numofdata;
                    // check address overflow
                    if ( address >= 0x10000 ) {
                        // set function code
                        cmd[1] = 0x11;
                        // reset address at overflow point
                        address -= 0x10000;
                        printf("set function code 0x%X, address 0x%04X\n", cmd[1], address);
                    }

                    retry = 0;

                    break;
                } else if ( (lpdata[4] == 0xFF) && (lpdata[5] == 0xFE) ) {
                    // response byte count error
                    printf("response byte count error\n");
                    SaveLog((char *)"FWupdate WriteHBData() : Response Byte Count error", st_time);
                    return 2;
                } else if ( (lpdata[4] == 0xFF) && (lpdata[5] == 0xFF) ) {
                    // response check sum error
                    printf("response check sum error\n");
                    SaveLog((char *)"FWupdate WriteHBData() : Response Check Sum error", st_time);
                    return 3;
                } else if ( (lpdata[4] == 0xFF) && (lpdata[5] == 0xF0) ) {
                    // response the same FW data
                    printf("the same FW data\n");
                    SaveLog((char *)"FWupdate WriteHBData() : The same FW data", st_time);
                    return 4;
                } else {
                    printf("Response check error!\n");
                    SaveLog((char *)"FWupdate WriteHBData() : Response check error", st_time);
                    // save respond
                    sprintf(log, "FWupdate WriteHBData() get :");
                    for (i = 0; i < 8; i++) {
                        sprintf(buf, " %02X", lpdata[i]);
                        strcat(log, buf);
                    }
                    SaveLog(log, st_time);
                    // re-address
                    SaveLog((char *)"FWupdate WriteHBData() : Do Re-address", st_time);
                    numofdata = lpdata[5];
                    writesize = numofdata*2;
                    cnt++;
                    sprintf(buf, "FWupdate WriteHBData() : write count %d, addr 0x%X, index 0x%X, size %d already OK", cnt, address, index, writesize);
                    SaveLog(buf, st_time);
                    address = (lpdata[2]<<8) + lpdata[3] + numofdata;
                    index+=writesize;
                    printf("Re-address 0x%04X, numofdata 0x%02X, index %d\n", address, numofdata, index);
                    sprintf(buf, "FWupdate WriteHBData() : Re-address 0x%04X, numofdata 0x%02X, index 0x%X", address, numofdata, index);
                    SaveLog(buf, st_time);

                    if ( retry == 0 ) {
                        retry = 1;
                        break;
                    } else {
                        SaveLog((char *)"FWupdate WriteHBData() : Response check retry error, end", st_time);
                        return 5;
                    }
                }
            } else {
                err++;
                printf("#### WriteHBData GetRespond Error %d ####\n", err);
                if ( err == 3 ) {
                    if ( have_respond ) {
                        printf("#### WriteHBData CRC Error ####\n");
                        SaveLog((char *)"FWupdate WriteHBData() : CRC Error", st_time);
                    } else {
                        printf("#### WriteHBData No Response ####\n");
                        SaveLog((char *)"FWupdate WriteHBData() : No Response", st_time);
                    }

                    // check data size
                    if ( numofdata > 0x08 ) {
                        numofdata-=0x08;
                        writesize = numofdata*2;
                        if ( numofdata < 0x08 ) {
                            numofdata = 0x08;
                            writesize = numofdata*2;
                        }
                        printf("set numofdata = 0x%X, writesize = %d\n", numofdata, writesize);
                        sprintf(log, "FWupdate WriteHBData() set size %d", writesize);
                        SaveLog(log, st_time);
                        err = 0;
                        break;
                    } else {
                        printf("numofdata = 0x%X, too small so end this loop\n", numofdata);
                        SaveLog((char *)"FWupdate WriteHBData() : size too small, end", st_time);
                        end = 1;
                        break;
                    }
                }
            }
        }

        if ( end )
            break;
    }

    printf("\n#### WriteHBData end ####\n");

    if ( index == datasize )
        return 0;
    else
        return 6;
}

// 2.5.3.5
int WriteBatData(int loop, int slaveid, unsigned short section, unsigned short section_size, FILE* pfile_fd)
{
    printf("\n#### WriteBatData start ####\n");

    //int i = 0, err = 0, index = 0, numofdata = MAX_HYBRID_SIZE/2, writesize = MAX_HYBRID_SIZE, address = 0, cnt = 0, end = 0;
    int i = 0, err = 0, index = 0, numofdata = 0, writesize = 0, address = 0, cnt = 0, ret = 0, section_index = 0; //end = 0;
    unsigned char addrh = 0, addrl = 0;
    unsigned char *lpdata = NULL;
    unsigned char read_buf[1036] = {0};
    char buf[256] = {0}, log[1024] = {0};
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[MAX_BATTERY_SIZE+9]={0};

    for (section_index = 0; section_index < section-1; section_index++) {
        if ( section_index != 0 ) {
            memset(&mybatinfo, 0x00, sizeof(mybatinfo));
            ret = GetBatInfo(loop, slaveid, 40);
            if ( ret ) {
                printf("GetBatInfo fail timeout\n");
                snlist[loop].status = 59;
                break;
            }
            if (mybatinfo.status != 2) {
                printf("status = %d, stop\n", mybatinfo.status);
                ret = mybatinfo.status;
                snlist[loop].status = 56;
                break;
            } else if (mybatinfo.burn_status != 8) {
                printf("burn_status = %d, stop\n", mybatinfo.burn_status);
                ret = mybatinfo.burn_status;
                snlist[loop].status = 57;
                break;
            }
        }
        // read section data
        memset(read_buf, 0x00, 1036);
        fread(read_buf, 1, 1036, pfile_fd);

        // set first write command
        // set slave id
        cmd[0] = (unsigned char)slaveid;
        // set function code
        cmd[1] = 0x34;
        // set addr
        cmd[2] = 0x00;
        cmd[3] = 0x1E;
        // set no. of data
        cmd[4] = 0x00;
        cmd[5] = 0x03;
        // set byte count
        cmd[6] = 0x06;
        // set data
        cmd[7] = read_buf[0];
        cmd[8] = read_buf[1];
        cmd[9] = read_buf[2];
        cmd[10] = read_buf[3];
        cmd[11] = read_buf[4];
        cmd[12] = read_buf[5];
        MakeReadDataCRC(cmd,15);

        MClearRX();
        txsize = 15;
        waitAddr = cmd[0];
        waitFCode = cmd[1];

        err = 0;
        while ( err < 40 ) {
            memcpy(txbuffer, cmd, 15);
            MStartTX(gcomportfd);
            usleep(100000); // 0.1s

            current_time = time(NULL);
            st_time = localtime(&current_time);

            sprintf(log, "FWupdate WriteBatData() send :");
            for (i = 0; i < txsize; i++) {
                sprintf(buf, " %02X", cmd[i]);
                strcat(log, buf);
            }
            SaveLog(log, st_time);

            lpdata = GetRespond(gcomportfd, 8, delay_time_2); // uci setting
            // save debug log
            current_time = time(NULL);
            st_time = localtime(&current_time);
            if ( have_respond && (lpdata != NULL) ) {
                sprintf(log, "FWupdate WriteBatData() get :");
                for (i = 0; i < 8; i++) {
                    sprintf(buf, " %02X", lpdata[i]);
                    strcat(log, buf);
                }
                SaveLog(log, st_time);
            }
            if ( lpdata ) {
                // check result
                if ( (lpdata[1] == waitFCode) && (lpdata[2] == 0) && (lpdata[3] == 0) && (lpdata[4] == 0) && (lpdata[5] == 0) ) {
                    // no response from bms
                    printf("#### WriteBatData No Response From BMS ####\n");
                    SaveLog((char *)"FWupdate WriteBatData() : No Response From BMS", st_time);
                    ret = 1;
                    snlist[loop].status = 38;
                    err++;
                } else if ( lpdata[1] == waitFCode+0x08 ) {
                    // get error code
                    sprintf(buf, "FWupdate WriteBatData() : Get Error Code 0x%X", lpdata[1]);
                    printf(buf);
                    printf("\n");
                    SaveLog(buf, st_time);
                    ret = 2;
                    snlist[loop].status = 39;
                    err++;
                } else {
                    printf("#### WriteBatData OK ####\n");
                    SaveLog((char *)"FWupdate WriteBatData() : OK", st_time);
                    ret = 0;
                    snlist[loop].status = 0;
                    break;
                }
            } else {
                if ( have_respond ) {
                    if ( check_respond ) {
                        printf("#### WriteBatData No Response From Battery ####\n");
                        SaveLog((char *)"FWupdate WriteBatData() : No Response From Battery", st_time);
                        ret = -1;
                        snlist[loop].status = 40;
                    } else {
                        printf("#### WriteBatData data Error ####\n");
                        SaveLog((char *)"FWupdate WriteBatData() : data Error", st_time);
                        ret = -2;
                        snlist[loop].status = 41;
                    }
                }
                else {
                    printf("#### WriteBatData No Response From Inverter####\n");
                    SaveLog((char *)"FWupdate WriteBatData() : No Response From Inverter", st_time);
                    ret = -3;
                    snlist[loop].status = 42;
                }
                err++;
            }
        }
        if ( ret ) {
            return ret;
        }

        // set write data part
        index = 6;
        address = 0x0021;
        writesize = MAX_BATTERY_SIZE;
        numofdata = writesize/2;

        while ( index < (section_size-2) ) {
            // check data size
            if ( (index + writesize) > (section_size-2) ) {
                writesize = (section_size-2) - index;
                numofdata = writesize/2;
            }

            // set slave id
            cmd[0] = (unsigned char)slaveid;
            // set function code
            cmd[1] = 0x34;
            // set addr
            addrh = (unsigned char)((address>>8) & 0xFF);
            addrl = (unsigned char)(address & 0xFF);
            cmd[2] = addrh;
            cmd[3] = addrl;
            // set number of data, max 0x2E (dec.46), so hi always 0
            cmd[4] = 0;
            cmd[5] = (unsigned char)numofdata;
            // set byte count, max 0x5C (dec.92)
            cmd[6] = (unsigned char)writesize;

            // set data to buf
            for (i = 0; i < writesize; i++) {
                cmd[7+i] = read_buf[index+i];
            }

            // set crc
            MakeReadDataCRC(cmd, writesize+9);

            MClearRX();
            txsize = writesize+9;
            waitAddr = cmd[0];
            waitFCode = cmd[1];

            err = 0;
            while ( err < 40 ) {
                memcpy(txbuffer, cmd, txsize);
                MStartTX(gcomportfd);
                usleep(100000); // 0.1s

                current_time = time(NULL);
                st_time = localtime(&current_time);

                sprintf(log, "FWupdate WriteBatData() send :");
                for (i = 0; i < txsize; i++) {
                    sprintf(buf, " %02X", cmd[i]);
                    strcat(log, buf);
                }
                SaveLog(log, st_time);

                lpdata = GetRespond(gcomportfd, 8, delay_time_2); // uci setting
                current_time = time(NULL);
                st_time = localtime(&current_time);
                // save debug log
                if ( have_respond && (lpdata != NULL) ) {
                    sprintf(log, "FWupdate WriteBatData() get :");
                    for (i = 0; i < 8; i++) {
                        sprintf(buf, " %02X", lpdata[i]);
                        strcat(log, buf);
                    }
                    SaveLog(log, st_time);
                }
                if ( lpdata ) {
                    //lpdata = cmd; // for test
                    if ( (lpdata[1] == waitFCode) && (lpdata[2] == addrh) && (lpdata[3] == addrl) && (lpdata[4] == 00) && (lpdata[5] == numofdata) ) {
                        cnt++;
                        printf("#### WriteBatData data count %d, addr 0x%X, index 0x%X, size %d OK ####\n", cnt, address, index, writesize);
                        //sprintf(buf, "FWupdate WriteBatData() : write count %d, addr 0x%X, index 0x%X, size %d OK", cnt, address, index, writesize);
                        //SaveLog(buf, st_time);

                        index+=writesize;
                        address+=numofdata;

                        ret = 0;

                        snlist[loop].status = 0;
                        break;

                    } else if ( (lpdata[2] == 0) && (lpdata[3] == 0) && (lpdata[4] == 0) && (lpdata[5] == 0) ) {
                        // no response from bms
                        printf("#### WriteBatData No Response From BMS ####\n");
                        SaveLog((char *)"FWupdate WriteBatData() : No Response From BMS", st_time);
                        ret = 1;
                        snlist[loop].status = 43;
                        err++;
                    } else if ( lpdata[1] == waitFCode+0x08 ) {
                        // get error code
                        sprintf(buf, "FWupdate WriteBatData() : Get Error Code 0x%X", lpdata[1]);
                        printf(buf);
                        printf("\n");
                        SaveLog(buf, st_time);
                        ret = 2;
                        snlist[loop].status = 44;
                        err++;
                    }
                } else {
                    if ( have_respond ) {
                        if ( check_respond ) {
                            printf("#### WriteBatData No Response From Battery ####\n");
                            SaveLog((char *)"FWupdate WriteBatData() : No Response From Battery", st_time);
                            ret = -1;
                            snlist[loop].status = 45;
                        } else {
                            printf("#### WriteBatData data Error ####\n");
                            SaveLog((char *)"FWupdate WriteBatData() : data Error", st_time);
                            ret = -2;
                            snlist[loop].status = 46;
                        }
                    }
                    else {
                        printf("#### WriteBatData No Response From Inverter####\n");
                        SaveLog((char *)"FWupdate WriteBatData() : No Response From Inverter", st_time);
                        ret = -3;
                        snlist[loop].status = 47;
                    }

                    err++;
                    printf("#### WriteBatData GetRespond Error %d ####\n", err);
                    /*if ( err == 3 ) {
                        // check data size
                        if ( numofdata > 0x08 ) {
                            numofdata-=0x08;
                            writesize = numofdata*2;
                            if ( numofdata < 0x08 ) {
                                numofdata = 0x08;
                                writesize = numofdata*2;
                            }
                            printf("set numofdata = 0x%X, writesize = %d\n", numofdata, writesize);
                            sprintf(log, "FWupdate WriteBatData() set size %d", writesize);
                            SaveLog(log, st_time);
                            err = 0;
                            break;
                        } else {
                            printf("numofdata = 0x%X, too small so end this loop\n", numofdata);
                            SaveLog((char *)"FWupdate WriteBatData() : size too small, end", st_time);
                            end = 1;
                            break;
                        }
                    }*/
                }
            }
            if ( ret ) {
                return ret;
            }

            //if ( end )
            //    break;
        }
        if ( ret ) {
            return ret;
        }

        if ( section_index < section-2 ) {
            // send section flag
            // set slave id
            cmd[0] = (unsigned char)slaveid;
            // set function code
            cmd[1] = 0x34;
            // set addr
            cmd[2] = 0x02;
            cmd[3] = 0x23;
            // set no. of data
            cmd[4] = 0x00;
            cmd[5] = 0x01;
            // set byte count
            cmd[6] = 0x02;
            // set data
            cmd[7] = 0xAA;
            cmd[8] = 0x55;
            MakeReadDataCRC(cmd,11);

            MClearRX();
            txsize = 11;
            waitAddr = cmd[0];
            waitFCode = cmd[1];
        } else {
            // send section flag
            // set slave id
            cmd[0] = (unsigned char)slaveid;
            // set function code
            cmd[1] = 0x34;
            // set addr
            cmd[2] = 0x02;
            cmd[3] = 0x23;
            // set no. of data
            cmd[4] = 0x00;
            cmd[5] = 0x02;
            // set byte count
            cmd[6] = 0x04;
            // set data
            cmd[7] = 0xAA;
            cmd[8] = 0x55;
            cmd[9] = 0x55;
            cmd[10] = 0xAA;
            MakeReadDataCRC(cmd,13);

            MClearRX();
            txsize = 13;
            waitAddr = cmd[0];
            waitFCode = cmd[1];
        }

        err = 0;
        while ( err < 40 ) {
            if ( section_index < section-2 )
                memcpy(txbuffer, cmd, 11);
            else
                memcpy(txbuffer, cmd, 13);
            MStartTX(gcomportfd);
            usleep(100000); // 0.1s

            current_time = time(NULL);
            st_time = localtime(&current_time);

            sprintf(log, "FWupdate WriteBatData() send :");
            for (i = 0; i < txsize; i++) {
                sprintf(buf, " %02X", cmd[i]);
                strcat(log, buf);
            }
            SaveLog(log, st_time);

            //if ( section_index == section-2 )
            //    usleep(3000000); // 3s

            lpdata = GetRespond(gcomportfd, 8, delay_time_2); // uci setting
            // save debug log
            current_time = time(NULL);
            st_time = localtime(&current_time);
            if ( have_respond && (lpdata != NULL) ) {
                sprintf(log, "FWupdate WriteBatData() get :");
                for (i = 0; i < 8; i++) {
                    sprintf(buf, " %02X", lpdata[i]);
                    strcat(log, buf);
                }
                SaveLog(log, st_time);
            }
            if ( lpdata ) {
                // check result
                if ( (lpdata[1] == waitFCode) && (lpdata[2] == 0) && (lpdata[3] == 0) && (lpdata[4] == 0) && (lpdata[5] == 0) ) {
                    // no response from bms
                    printf("#### WriteBatData No Response From BMS ####\n");
                    SaveLog((char *)"FWupdate WriteBatData() : No Response From BMS", st_time);
                    ret = 1;
                    snlist[loop].status = 48;
                    err++;
                } else if ( lpdata[1] == waitFCode+0x08 ) {
                    // get error code
                    sprintf(buf, "FWupdate WriteBatData() : Get Error Code 0x%X", lpdata[1]);
                    printf(buf);
                    printf("\n");
                    SaveLog(buf, st_time);
                    ret = 2;
                    snlist[loop].status = 49;
                    err++;
                } else {
                    printf("#### WriteBatData OK ####\n");
                    SaveLog((char *)"FWupdate WriteBatData() : OK", st_time);
                    ret = 0;
                    snlist[loop].status = 0;
                    break;
                }
            } else {
                if ( have_respond ) {
                    if ( check_respond ) {
                        printf("#### WriteBatData No Response From Battery ####\n");
                        SaveLog((char *)"FWupdate WriteBatData() : No Response From Battery", st_time);
                        ret = -1;
                        snlist[loop].status = 50;
                    } else {
                        printf("#### WriteBatData data Error ####\n");
                        SaveLog((char *)"FWupdate WriteBatData() : data Error", st_time);
                        ret = -2;
                        snlist[loop].status = 51;
                    }
                }
                else {
                    printf("#### WriteBatData No Response From Inverter####\n");
                    SaveLog((char *)"FWupdate WriteBatData() : No Response From Inverter", st_time);
                    ret = -3;
                    snlist[loop].status = 52;
                }
                err++;
            }
        }
        if ( ret ) {
            return ret;
        }

        usleep(100000); // 0.1s

    }

/*    // send finish flag
    // set slave id
    cmd[0] = (unsigned char)slaveid;
    // set function code
    cmd[1] = 0x34;
    // set addr
    cmd[2] = 0x02;
    cmd[3] = 0x24;
    // set no. of data
    cmd[4] = 0x00;
    cmd[5] = 0x01;
    // set byte count
    cmd[6] = 0x02;
    // set data
    cmd[7] = 0x55;
    cmd[8] = 0xAA;
    MakeReadDataCRC(cmd,11);

    MClearRX();
    txsize = 11;
    waitAddr = cmd[0];
    waitFCode = cmd[1];

    err = 0;
    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 11);
        MStartTX(gcomportfd);
        usleep(100000); // 0.1s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate WriteBatData() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 8, delay_time_2); // uci setting
        // save debug log
        if ( have_respond ) {
            current_time = time(NULL);
            st_time = localtime(&current_time);

            sprintf(log, "FWupdate WriteBatData() get :");
            for (i = 0; i < 8; i++) {
                sprintf(buf, " %02X", respond_buff[i]);
                strcat(log, buf);
            }
            SaveLog(log, st_time);
        }
        if ( lpdata ) {
            // check result
            if ( (lpdata[2] == 0) && (lpdata[3] == 0) && (lpdata[4] == 0) && (lpdata[5] == 0) ) {
                // no response from bms
                printf("#### WriteBatData No Response From BMS ####\n");
                SaveLog((char *)"FWupdate WriteBatData() : No Response From BMS", st_time);
                ret = 1;
                snlist[loop].status = 53;
                err++;
            } else if ( lpdata[1] == 0x3C ) {
                // get error code
                sprintf(buf, "FWupdate WriteBatData() : Get Error Code %d", lpdata[1]);
                printf(buf);
                SaveLog(buf, st_time);
                ret = 2;
                snlist[loop].status = 54;
                err++;
            } else {
                printf("#### WriteBatData OK ####\n");
                SaveLog((char *)"FWupdate WriteBatData() : OK", st_time);
                ret = 0;
                snlist[loop].status = 0;
                break;
            }
        } else {
            if ( have_respond ) {
                if ( check_respond ) {
                    printf("#### WriteBatData No Response From Battery ####\n");
                    SaveLog((char *)"FWupdate WriteBatData() : No Response From Battery", st_time);
                    ret = -1;
                    snlist[loop].status = 55;
                } else {
                    printf("#### WriteBatData data Error ####\n");
                    SaveLog((char *)"FWupdate WriteBatData() : data Error", st_time);
                    ret = -2;
                    snlist[loop].status = 56;
                }
            }
            else {
                printf("#### WriteBatData No Response From Inverter####\n");
                SaveLog((char *)"FWupdate WriteBatData() : No Response From Inverter", st_time);
                ret = -3;
                snlist[loop].status = 57;
            }
            err++;
        }
    }
*/
    printf("\n#### WriteBatData end ####\n");

    return ret;
}

int WriteDataV3(char *sn, unsigned char *fwdata, int datasize)
{

    printf("\n#### WriteDataV3 start ####\n");

    int i = 0, err = 0, index = 0, numofdata = MAX_DATA_SIZE/2, writesize = MAX_DATA_SIZE, address = 0, cnt = 0, end = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0;
    unsigned char addrh = 0, addrl = 0;
    unsigned char *lpdata = NULL;
    char buf[256] = {0}, log[1024] = {0};
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[MAX_DATA_SIZE+15]={0};

    // set target
    cmd[0] = 0x01;
    // set function code
    cmd[1] = 0x49;

    sscanf(sn, "%06s%02X%02X%02X%02X%02X", buf, &tmp1, &tmp2, &tmp3, &tmp4, &tmp5);
    // set model & SN
    cmd[3] = (unsigned char)tmp1;
    cmd[4] = (unsigned char)tmp2;
    cmd[5] = (unsigned char)tmp3;
    cmd[6] = (unsigned char)tmp4;
    cmd[7] = (unsigned char)tmp5;

    // start write data loop
    while ( index < datasize ) {
        // check data size
        if ( (index + writesize) > datasize ) {
            writesize = datasize - index;
            numofdata = writesize/2;
        }

        // set addr
        addrh = (address>>8) & 0xFF;
        addrl = address & 0xFF;
        cmd[8] = addrh;
        cmd[9] = addrl;
        // set number of data, max 0x48 (dec.72), so hi always 0
        cmd[10] = 0;
        cmd[11] = (unsigned char)numofdata;
        // set byte count, max 0x90 (dec.144)
        cmd[12] = (unsigned char)writesize;
        // set package total byte
        cmd[2] = (unsigned char)(writesize+15);

        // set data to buf
        for (i = 0; i < writesize; i++) {
            cmd[13+i] = fwdata[index+i];
        }

        // set crc
        MakeReadDataCRC(cmd, writesize+15);

        MClearRX();
        txsize = writesize+15;
        waitAddr = cmd[0];
        waitFCode = cmd[1];

        while ( err < 3 ) {
            memcpy(txbuffer, cmd, txsize);
            MStartTX(gcomportfd);
            //usleep(10000); // 0.01s

            current_time = time(NULL);
            st_time = localtime(&current_time);

            sprintf(log, "FWupdate WriteDataV3() send :");
            for (i = 0; i < txsize; i++) {
                sprintf(buf, " %02X", cmd[i]);
                strcat(log, buf);
            }
            SaveLog(log, st_time);

            lpdata = GetRespond(gcomportfd, 8, delay_time_1); // from uci config
            current_time = time(NULL);
            st_time = localtime(&current_time);
            // save debug log
            if ( have_respond && (lpdata != NULL) ) {
                sprintf(log, "FWupdate WriteDataV3() get :");
                for (i = 0; i < 8; i++) {
                    sprintf(buf, " %02X", lpdata[i]);
                    strcat(log, buf);
                }
                SaveLog(log, st_time);
            }
            if ( lpdata ) {
                cnt++;
                printf("#### WriteDataV3 data count %d, index 0x%X, size %d OK ####\n", cnt, index, writesize);
                sprintf(buf, "FWupdate WriteDataV3() : write count %d, index 0x%X, size %d OK", cnt, index, writesize);
                SaveLog(buf, st_time);

                index+=writesize;
                //address+=numofdata;
                address+=writesize;

                break;
            } else {
                err++;
                printf("#### WriteDataV3 GetRespond Error %d ####\n", err);
                if ( err == 3 ) {
                    if ( have_respond ) {
                        printf("#### WriteDataV3 CRC Error ####\n");
                        SaveLog((char *)"FWupdate WriteDataV3() : CRC Error", st_time);
                    } else {
                        printf("#### WriteDataV3 No Response ####\n");
                        SaveLog((char *)"FWupdate WriteDataV3() : No Response", st_time);
                    }

                    // check data size
                    if ( numofdata > 0x08 ) {
                        numofdata-=0x08;
                        writesize = numofdata*2;
                        printf("set numofdata = 0x%X, writesize = %d\n", numofdata, writesize);
                        sprintf(log, "FWupdate WriteDataV3() set size %d", writesize);
                        SaveLog(log, st_time);
                        err = 0;
                        break;
                    } else {
                        printf("numofdata = 0x%X, too small so end this loop\n", numofdata);
                        SaveLog((char *)"FWupdate WriteDataV3() : size too small, end", st_time);
                        end = 1;
                        break;
                    }
                }
            }
        }

        if ( end )
            break;
    }

    // send reboot cmd
    RunRebootSpecify(sn);

    printf("\n#### WriteDataV3 end ####\n");

    if ( index == datasize )
        return 0;
    else
        return 1;
}

int ReadV3Ver(char *sn, unsigned char *fwver)
{
    printf("\n#### ReadV3Ver start ####\n");

    int err = 0, ret = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0, ver1 = 0, ver2 = 0, i = 0;
    char buf[256] = {0}, log[1024] = {0};
    unsigned char *lpdata = NULL, verh = 0, verl = 0;
    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    unsigned char cmd[]={0x01, 0x33, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    sscanf(sn, "%06s%02X%02X%02X%02X%02X", buf, &tmp1, &tmp2, &tmp3, &tmp4, &tmp5);
    // set model & SN
    cmd[3] = (unsigned char)tmp1;
    cmd[4] = (unsigned char)tmp2;
    cmd[5] = (unsigned char)tmp3;
    cmd[6] = (unsigned char)tmp4;
    cmd[7] = (unsigned char)tmp5;
    // set addr
    cmd[8] = 0x00;
    cmd[9] = 0x0A;
    // no. of data
    cmd[10] = 0x00;
    cmd[11] = 0x01;
    // set crc
    MakeReadDataCRC(cmd,14);

    MClearRX();
    txsize=14;
    waitAddr  = cmd[0];
    waitFCode = cmd[1];

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 14);
        MStartTX(gcomportfd);
        //usleep(10000); // 0.01s

        current_time = time(NULL);
        st_time = localtime(&current_time);

        sprintf(log, "FWupdate ReadV3Ver() send :");
        for (i = 0; i < txsize; i++) {
            sprintf(buf, " %02X", cmd[i]);
            strcat(log, buf);
        }
        SaveLog(log, st_time);

        lpdata = GetRespond(gcomportfd, 13, delay_time_1); // from uci config
        if ( lpdata ) {
            printf("#### ReadV3Ver OK ####\n");
            SaveLog((char *)"FWupdate ReadV3Ver() : OK", st_time);

            // check ver
            verh = lpdata[9];
            verl = lpdata[10];
            ver1 = (verh<<8) + verl;
            ver2 = fwver[0]*1000 + fwver[1]*100 + fwver[2]*10 + fwver[3];
            printf("ver1 = %d, ver2 = %d\n", ver1, ver2);
            sprintf(buf, "FWupdate ReadV3Ver() : Get Ver %d", ver1);
            SaveLog(buf, st_time);
            if ( ver1 == ver2 ) {
                printf("FW ver match\n");
                SaveLog((char *)"FWupdate ReadV3Ver() : fw ver match", st_time);
                return 0;
            } else {
                printf("FW ver not match\n");
                SaveLog((char *)"FWupdate ReadV3Ver() : fw ver not match", st_time);
                return 2;
            }
        } else {
            if ( have_respond ) {
                printf("#### ReadV3Ver CRC Error ####\n");
                SaveLog((char *)"FWupdate ReadV3Ver() : CRC Error", st_time);
                ret = 1;
            }
            else {
                printf("#### ReadV3Ver No Response ####\n");
                SaveLog((char *)"FWupdate ReadV3Ver() : No Response", st_time);
                ret = -1;
            }
            err++;
        }
    }

    return ret;
}

int GetFWData(char *list_path)
{
    unsigned char *ucbuffer = NULL;
    char read_buf[256] = {0}, strtmp[1024] = {0};
    char *cptr = NULL;
    FILE *pfile_fd = NULL;
    int major = 0, minor = 0, patchh = 0, patchl = 0, datasize = 0;
    unsigned char ucfwver[4] = {0};
    int index = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, start = 0, end = 0, i = 0, j = 0, count = 0, dowritedata = 0, retver = -1, retdata = -1, clearsn = 0, clearfile = 0;

    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    printf("######### run GetFWData() #########\n");

    printf("gsncount = %d\n", gsncount);

    for (count = 0; count < gsncount; count++) {
        printf("count = %d\n", count);
        datasize = 0;
        index = 0;
        start = 0;
        end = 0;
        cptr = NULL;
        current_time = time(NULL);
        st_time = localtime(&current_time);

        Updheartbeattime(current_time);
        CloseLog();
        system("sync");
        OpenLog(g_SYSLOG_PATH, st_time);

        pfile_fd = fopen(snlist[count].FILE, "r");
        if ( pfile_fd == NULL ) {
            printf("#### Open %s Fail ####\n", snlist[count].FILE);
            SaveLog((char *)"FWupdate GetFWData() : Open fw file Fail", st_time);
            // delete first sn & file
            CLEANSN(snlist[count].SN, snlist[count].FILE, list_path);
            memset(strtmp, 0x00, 1024);
            sprintf(strtmp, "rm %s; sync;", snlist[count].FILE);
            system(strtmp);
            continue;
        }

        // jump to beginning of file
        fseek(pfile_fd, 0, SEEK_SET);
        // get data size
        while ( fgets(read_buf, 256, pfile_fd) != NULL ) {
            printf("read_buf = %s\n", read_buf);
            // find start point
            cptr = strstr(read_buf, "<program_bank_1_bytes>");
            if ( cptr )
                start = 1;

            // find end point
            cptr = strstr(read_buf, "</program_bank_1_bytes>");
            if ( cptr )
                end = 1;

            // end this loop
            if ( end ) {
                start = 0;
                end = 0;
                break;
            }

            // count data size
            if ( start )
                datasize += 4;
        }
        printf("datasize = %d\n", datasize);
        // jump to beginning of file
        fseek(pfile_fd, 0, SEEK_SET);

        // create fw data buffer
        ucbuffer = calloc(datasize, sizeof(unsigned char));
        if ( ucbuffer == NULL ) {
            printf("#### calloc %d Fail ####\n", datasize);
            SaveLog((char *)"FWupdate GetFWData() : calloc Fail", st_time);
            fclose(pfile_fd);
            continue;
        }
        printf("calloc size %d OK\n", datasize);

        // get xml data
        while ( fgets(read_buf, 256, pfile_fd) != NULL ) {
            // debuf printf
            //printf("read_buf = %s", read_buf);

            // check <software_version>
            cptr = strstr(read_buf, "<software_version>");
            if ( cptr ) {
                sscanf(cptr, "<software_version>0x%02x%02x%02x%02x</software_version>", &major, &minor, &patchh, &patchl);
                //printf("version = V%d.%d.%d.%d\n", major, minor, patchh, patchl);
                ucfwver[0] = (unsigned char)major;
                ucfwver[1] = (unsigned char)minor;
                ucfwver[2] = (unsigned char)patchh;
                ucfwver[3] = (unsigned char)patchl;
                printf("ucfwver[0] = 0x%02X\n", ucfwver[0]);
                printf("ucfwver[1] = 0x%02X\n", ucfwver[1]);
                printf("ucfwver[2] = 0x%02X\n", ucfwver[2]);
                printf("ucfwver[3] = 0x%02X\n", ucfwver[3]);
            }

            // check <program_bank_1_bytes>
            cptr = strstr(read_buf, "<program_bank_1_bytes>");
            if ( cptr ) {
                sscanf(cptr, "<program_bank_1_bytes>%02X %02X %02X %02X", &tmp1, &tmp2, &tmp3, &tmp4);
                ucbuffer[index] = (unsigned char)tmp1;
                ucbuffer[index+1] = (unsigned char)tmp2;
                ucbuffer[index+2] = (unsigned char)tmp3;
                ucbuffer[index+3] = (unsigned char)tmp4;
                index += 4;

                // start get data loop
                while ( fgets(read_buf, 256, pfile_fd) != NULL ) {
                    // check end point
                    if ( strstr(read_buf, "</program_bank_1_bytes>") ) {
                        printf("end\n");
                        end = 1;
                        break;
                    }

                    sscanf(read_buf, "%02X %02X %02X %02X", &tmp1, &tmp2, &tmp3, &tmp4);
                    ucbuffer[index]   = (unsigned char)tmp1;
                    ucbuffer[index+1] = (unsigned char)tmp2;
                    ucbuffer[index+2] = (unsigned char)tmp3;
                    ucbuffer[index+3] = (unsigned char)tmp4;
                    index += 4;
                }
            }

            if ( end )
                break;
        }
        fclose(pfile_fd);
        system("sync; sync;");
        printf("index = %d\n", index);

        // save fw version in log
        memset(strtmp, 0x00, 1024);
        sprintf(strtmp, "FWupdate GetFWData() : Get version = 0x%02X%02X%02X%02X", ucfwver[0], ucfwver[1], ucfwver[2], ucfwver[3]);
        SaveLog(strtmp, st_time);

        // save fw data in log
        for (i = 0, j = 0; i < index; i++) {
            // part tital
            if ( i%MAX_DATA_SIZE == 0 ) {
                j++;
                sprintf(strtmp, "FWupdate GetFWData() : Data count %d", j);
                SaveLog(strtmp, st_time);
                sprintf(strtmp, "FWupdate GetFWData() : Data =");
            }
            // value
            sprintf(read_buf, " %02X", ucbuffer[i]);
            strcat(strtmp, read_buf);
            // line end
            if ( i%MAX_DATA_SIZE == MAX_DATA_SIZE-1) {
                SaveLog(strtmp, st_time);
            } else if ( i == index-1 ) {
                SaveLog(strtmp, st_time);
            }
        }

        // do write action
        dowritedata = 0;
        printf("==================== FW update start ====================\n");
        //printf("gsncount = %d\n", gsncount);
        // fw update loop
        //for (count = 0; count < gsncount; count++) {
            //printf("count = %d\n", count);
        // check time
        while (1) {
            current_time = time(NULL);
            st_time = localtime(&current_time);

            if ( update_FW_start == update_FW_stop )
                break;
            else if ( update_FW_start < update_FW_stop ) {
                if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                    break;
            } else {
                if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                    break;
            }

            Updheartbeattime(current_time);
            CloseLog();
            system("sync");
            OpenLog(g_SYSLOG_PATH, st_time);
            printf("sleep 60 sec.\n");
            usleep(60000000);
        }

        // protocol V2.0 part
        if ( gprotocolver == 2 ) {
            // register at first
            retver = RunRegister(snlist[count].SN);
            if ( !retver ) {
                // register OK, get slave id gV2id
                printf("RunRegister return OK, get gV2id = %d\n", gV2id);
                // enable priority 3
                retver = RunEnableP3(gV2id);
                if ( !retver ) {
                    printf("RunEnableP3 return OK\n");
                    // shutdown system
                    retver = RunShutdown(gV2id);
                    if ( !retver ) {
                        printf("RunShutdown return OK\n");
                        // write fw ver by protocol V2.0
                        retver = WriteVerV2(gV2id, ucfwver);
                    }
                }
            }
        } else if ( gprotocolver == 3 ) { // protocol V3.0 part
            // re register
            LBDReregister(snlist[count].SN);
            // check fw ver
            retver = ReadV3Ver(snlist[count].SN, ucfwver);
            if ( !retver ) {
                printf("The same fw version, skip index = %d update action\n", count);
                sprintf(strtmp, "FWupdate GetFWData() : The same fw version, skip index = %d, SN = %s update action", count, snlist[count].SN);
                SaveLog(strtmp, st_time);

                // delete first sn & file
                CLEANSN(snlist[count].SN, snlist[count].FILE, list_path);
                // search file in list, if exist, do not delete it
                for (i = count+1; i < gsncount; i++ ) {
                    if ( strcmp(snlist[count].FILE, snlist[i].FILE) == 0 ) {
                        clearfile = 0;
                        break;
                    } else
                        clearfile = 1;
                }
                if ( count == gsncount-1 )
                    clearfile = 1;
                if ( clearfile ) {
                    memset(strtmp, 0x00, 1024);
                    sprintf(strtmp, "rm %s; sync;", snlist[count].FILE);
                    system(strtmp);
                }

                free(ucbuffer);

                continue;
            }
            // write fw ver by protocol V3.0
            retver = WriteVerV3(snlist[count].SN, ucfwver);
        }

        // check write fw ver result
        if ( !retver ) {
            printf("sn[%d] WriteVer() OK\n", count);
            dowritedata = 1;
        } else {
            printf("sn[%d] WriteVer() Fail\n", count);
            dowritedata = 0;
        }

        // write fw data
        if ( dowritedata ) {
            if ( gprotocolver == 2 ) {
                printf("sn[%d] do write data V2.0 action\n", count);
                // write fw data by protocol V2.0
                retdata = WriteDataV2(gV2id, ucbuffer, datasize);
            } else if ( gprotocolver == 3 ) {
                printf("sn[%d] do write data V3.0 action\n", count);
                // write fw data by protocol V3.0
                retdata = WriteDataV3(snlist[count].SN, ucbuffer, datasize);
            }
            if ( !retdata ) {
                printf("sn[%d] = %s write fw data OK\n", count, snlist[count].SN);
                sprintf(strtmp, "FWupdate GetFWData() : SN %s write data OK", snlist[count].SN);
                SaveLog(strtmp, st_time);
                //if ( gprotocolver == 3 )
                //    ReadV3Ver(snlist[count].SN, ucfwver);
                // delete first sn & file
                CLEANSN(snlist[count].SN, snlist[count].FILE, list_path);
                // search file in list, if exist, do not delete it
                for (i = count+1; i < gsncount; i++ ) {
                    if ( strcmp(snlist[count].FILE, snlist[i].FILE) == 0 ) {
                        clearfile = 0;
                        break;
                    } else
                        clearfile = 1;
                }
                if ( count == gsncount-1 )
                    clearfile = 1;
                if ( clearfile ) {
                    memset(strtmp, 0x00, 1024);
                    sprintf(strtmp, "rm %s; sync;", snlist[count].FILE);
                    system(strtmp);
                }
            } else {
                printf("sn[%d] = %s write fw data Fail\n", count, snlist[count].SN);
                sprintf(strtmp, "FWupdate GetFWData() : SN %s write data FAIL", snlist[count].SN);
                SaveLog(strtmp, st_time);
                // delete the SN from list if stop time is not up
                current_time = time(NULL);
                st_time = localtime(&current_time);
                clearsn = 0;
                if ( update_FW_start == update_FW_stop )
                    clearsn = 1;
                else if ( update_FW_start < update_FW_stop ) {
                    if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                        clearsn = 1;
                } else {
                    if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                        clearsn = 1;
                }
                if ( clearsn ) {
                    // delete first sn & file
                    CLEANSN(snlist[count].SN, snlist[count].FILE, list_path);
                    // search file in list, if exist, do not delete it
                    for (i = count+1; i < gsncount; i++ ) {
                        if ( strcmp(snlist[count].FILE, snlist[i].FILE) == 0 ) {
                            clearfile = 0;
                            break;
                        } else
                            clearfile = 1;
                    }
                    if ( count == gsncount-1 )
                        clearfile = 1;
                    if ( clearfile ) {
                        memset(strtmp, 0x00, 1024);
                        sprintf(strtmp, "rm %s; sync;", snlist[count].FILE);
                        system(strtmp);
                    }
                }
            }
        } else {
            printf("WriteVer Fail, sn[%d] = %s don't write data\n", count, snlist[count].SN);
            sprintf(strtmp, "FWupdate GetFWData() : SN %s WriteVer() FAIL", snlist[count].SN);
            SaveLog(strtmp, st_time);
            // delete the SN from list if stop time is not up
            current_time = time(NULL);
            st_time = localtime(&current_time);
            clearsn = 0;
            if ( update_FW_start == update_FW_stop )
                clearsn = 1;
            else if ( update_FW_start < update_FW_stop ) {
                if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                    clearsn = 1;
            } else {
                if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                    clearsn = 1;
            }
            if ( clearsn ) {
                // delete first sn & file
                CLEANSN(snlist[count].SN, snlist[count].FILE, list_path);
                // search file in list, if exist, do not delete it
                for (i = count+1; i < gsncount; i++ ) {
                    if ( strcmp(snlist[count].FILE, snlist[i].FILE) == 0 ) {
                        clearfile = 0;
                        break;
                    } else
                        clearfile = 1;
                }
                if ( count == gsncount-1 )
                    clearfile = 1;
                if ( clearfile ) {
                    memset(strtmp, 0x00, 1024);
                    sprintf(strtmp, "rm %s; sync;", snlist[count].FILE);
                    system(strtmp);
                }
            }
        }

        // some info

        printf("===================== FW update end =====================\n");

        free(ucbuffer);

        // save log immediately
        CloseLog();
        system("sync");
        OpenLog(g_SYSLOG_PATH, st_time);
    }

    printf("######### GetFWData() end #########\n");

    return 0;
}

int GetHbFWData(char *list_path)
{
    unsigned char *ucbuffer = NULL;
    int i = 0, byte_count = 0, addr = 0, recoed_type = 0, tmp = 0, next_addr = 0, size = 0, index = 0, retval = -1, count = 0, index_tmp = 0, clearsn = 0, clearfile = 0, loop = 0;
    unsigned char hi_addr_hi = 0, hi_addr_lo = 0, lo_addr_hi = 0, lo_addr_lo = 0;
    unsigned short checksum = 0;
    char read_buf[256] = {0}, strtmp[1024] = {0};
    char *cptr = NULL;
    FILE *pfile_fd = NULL;

    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    printf("######### run GetHbFWData() #########\n");

    printf("gsncount = %d\n", gsncount);

    for (loop = 0; loop < gsncount; loop++) {
        printf("loop = %d\n", loop);
        size = 0;
        current_time = time(NULL);
        st_time = localtime(&current_time);

        Updheartbeattime(current_time);
        CloseLog();
        system("sync");
        OpenLog(g_SYSLOG_PATH, st_time);

        pfile_fd = fopen(snlist[loop].FILE, "r");
        if ( pfile_fd == NULL ) {
            printf("#### Open %s FAIL ####\n", snlist[loop].FILE);
            SaveLog((char *)"FWupdate GetHbFWData() : Open fw file Fail", st_time);
            // delete first sn & file
            CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
            memset(strtmp, 0x00, 1024);
            sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
            system(strtmp);
            continue;
        }

        // check file type
        memset(read_buf, 0x00, 256);
        fread(read_buf, 1, 1, pfile_fd);
        // standard format
        if ( read_buf[0] == ':' ) {
            // jump to beginning of file
            fseek(pfile_fd, 0, SEEK_SET);
            // count hex buffer size
            while ( fgets(read_buf, 256, pfile_fd) != NULL ) {
                // debug print
                //printf("read_buf = [%s]\n", read_buf);
                // check start byte ':'
                cptr = strchr(read_buf, ':');
                if ( cptr ) {
                    // get byte count, addr, type
                    sscanf(cptr, ":%02x%04x%02x", &byte_count, &addr, &recoed_type);
                    //printf("get byte_count = 0x%02X, addr = 0x%04X, recoed_type = 0x%02X\n", byte_count, addr, recoed_type);
                    // check record type
                    if ( recoed_type == 4 ) {
                        // new addr
                        if ( size == 0 )
                            size = 8; // File header 2 + File Checksum 2 + Data Byte Count 4

                        size += 10; // File Section Header 2 + Section Address 4 + Section Data Byte Count 4

                        next_addr = 0;
                    } else if ( recoed_type == 1 ) {
                        size += 2; // File End
                    } else if ( recoed_type == 0 ) {
                        if ( next_addr ) {
                            if ( next_addr == addr ) {
                                size += byte_count;
                            } else {
                                size += 10; // New Section Header 2 + Section Address 4 + Section Data Byte Count 4
                                size += byte_count;
                            }
                        } else
                            size += byte_count;

                        next_addr = byte_count/2 + addr;
                        //printf("next_addr = 0x%04X\n", next_addr);
                    }
                    // debug print
                    //printf("size = %d\n", size);
                } else {
                    printf("[:] not found, stop, please check your fw file\n");
                    fclose(pfile_fd);
                    // wrong file?
                    // delete first sn & file
                    CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                    memset(strtmp, 0x00, 1024);
                    sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                    system(strtmp);
                    continue;
                }
            }
            printf("size = %d\n", size);
            // jump to beginning of file
            fseek(pfile_fd, 0, SEEK_SET);

            if ( size == 0 ) {
                fclose(pfile_fd);
                printf("size = 0\n");
                SaveLog((char *)"FWupdate GetHbFWData() : size = 0", st_time);
                // wrong file?
                // delete first sn & file
                CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                memset(strtmp, 0x00, 1024);
                sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                system(strtmp);
                continue;
            }

            // create fw data buffer
            ucbuffer = calloc(size, sizeof(unsigned char));
            if ( ucbuffer == NULL ) {
                printf("#### calloc %d Fail ####\n", size);
                SaveLog((char *)"FWupdate GetHbFWData() : calloc Fail", st_time);
                fclose(pfile_fd);
                continue;
            }
            printf("calloc size %d OK\n", size);

            // set fw data
            index = 0;
            while ( fgets(read_buf, 256, pfile_fd) != NULL ) {
                // debug print
                //printf("read_buf = [%s]\n", read_buf);
                // check start byte ':'
                cptr = strchr(read_buf, ':');
                if ( cptr ) {
                    // get byte count, addr, type
                    sscanf(cptr, ":%02x%04x%02x", &byte_count, &addr, &recoed_type);
                    //printf("get byte_count = 0x%02X, addr = 0x%04X, recoed_type = 0x%02X\n", byte_count, addr, recoed_type);
                    lo_addr_hi = (unsigned char)(addr >> 8);
                    lo_addr_lo = (unsigned char)(addr & 0xFF);
                    // check record type
                    if ( recoed_type == 4 ) {
                        // save hi addr
                        cptr += 9;
                        sscanf(cptr, "%04x", &tmp);
                        hi_addr_hi = (unsigned char)(tmp >> 8);
                        hi_addr_lo = (unsigned char)(tmp & 0xFF);
                        //printf("hi_addr_hi = 0x%02X, hi_addr_lo = 0x%02X\n", hi_addr_hi, hi_addr_lo);
                        // get next line
                        fgets(read_buf, 256, pfile_fd);
                        // debug print
                        //printf("read_buf = [%s]\n", read_buf);
                        cptr = strchr(read_buf, ':');
                        if ( cptr ) {
                            // get byte count, addr, type
                            sscanf(cptr, ":%02x%04x%02x", &byte_count, &addr, &recoed_type);
                            //printf("get next byte_count = 0x%02X, addr = 0x%04X, recoed_type = 0x%02X\n", byte_count, addr, recoed_type);
                            lo_addr_hi = (unsigned char)(addr >> 8);
                            lo_addr_lo = (unsigned char)(addr & 0xFF);

                            // set data
                            // first section, set file header
                            if ( index == 0 ) {
                                // (0,1)File header + (2,3)File Checksum + (4,5,6,7)Byte Count + (8,9)First Section Header + (10,11,12,13)Section Address
                                if ( hi_addr_hi == 0x00 && hi_addr_lo == 0x3E && lo_addr_hi == 0x80 && lo_addr_lo == 0x00 ) {
                                    // set chip 1
                                    ucbuffer[0] = 0xDA;
                                    ucbuffer[1] = 0x01;
                                } else {
                                    // set chip 2
                                    ucbuffer[0] = 0xDA;
                                    ucbuffer[1] = 0x02;
                                }
                                // skip 2,3 check sum, final set it
                                tmp = size - 8;
                                ucbuffer[4] = (unsigned char)((tmp >> 24) & 0xFF);
                                ucbuffer[5] = (unsigned char)((tmp >> 16) & 0xFF);
                                ucbuffer[6] = (unsigned char)((tmp >> 8) & 0xFF);
                                ucbuffer[7] = (unsigned char)(tmp & 0xFF);
                                ucbuffer[8] = 0xDA;
                                ucbuffer[9] = 0xF6;
                                ucbuffer[10] = hi_addr_hi;
                                ucbuffer[11] = hi_addr_lo;
                                ucbuffer[12] = lo_addr_hi;
                                ucbuffer[13] = lo_addr_lo;
                                // skip 14,15,16,17 byte count, next section set it
                                index_tmp = 14;
                                index += 18;
                            } else {
                                // set previous section byte count
                                ucbuffer[index_tmp] = (unsigned char)((count >> 24) & 0xFF);
                                ucbuffer[index_tmp+1] = (unsigned char)((count >> 16) & 0xFF);
                                ucbuffer[index_tmp+2] = (unsigned char)((count >> 8) & 0xFF);
                                ucbuffer[index_tmp+3] = (unsigned char)(count & 0xFF);
                                // not first, only set (0,1)New Section Header + (2,3,4,5)Section Address
                                ucbuffer[index]   = 0xDA;
                                ucbuffer[index+1] = 0xF6;
                                ucbuffer[index+2] = hi_addr_hi;
                                ucbuffer[index+3] = hi_addr_lo;
                                ucbuffer[index+4] = lo_addr_hi;
                                ucbuffer[index+5] = lo_addr_lo;
                                // skip 6,7,8,9 byte count, next section set it
                                index_tmp = index+6;
                                index += 10;
                            }

                            // set fw data
                            cptr += 9;
                            for (i = 0; i < byte_count; i++) {
                                sscanf(cptr, "%02x", &tmp);
                                ucbuffer[index+i] = (unsigned char)tmp;
                                cptr += 2;
                            }
                            count = byte_count;
                            index += byte_count;
                            next_addr = byte_count/2 + addr;
                            //printf("next_addr = 0x%04X\n", next_addr);
                        }
                    } else if ( recoed_type == 1 ) {
                        // set previous section byte count
                        ucbuffer[index_tmp] = (unsigned char)((count >> 24) & 0xFF);
                        ucbuffer[index_tmp+1] = (unsigned char)((count >> 16) & 0xFF);
                        ucbuffer[index_tmp+2] = (unsigned char)((count >> 8) & 0xFF);
                        ucbuffer[index_tmp+3] = (unsigned char)(count & 0xFF);
                        // File End
                        ucbuffer[index]   = 0xDA;
                        ucbuffer[index+1] = 0xF7;
                        index += 2;
                        //break;
                    } else if ( recoed_type == 0 ) {
                        if ( next_addr ) {
                            if ( next_addr == addr ) {
                                // set fw data
                                cptr += 9;
                                for (i = 0; i < byte_count; i++) {
                                    sscanf(cptr, "%02x", &tmp);
                                    ucbuffer[index+i] = (unsigned char)tmp;
                                    cptr += 2;
                                }
                                count += byte_count;
                                index += byte_count;
                            } else {
                                // set previous section byte count
                                ucbuffer[index_tmp] = (unsigned char)((count >> 24) & 0xFF);
                                ucbuffer[index_tmp+1] = (unsigned char)((count >> 16) & 0xFF);
                                ucbuffer[index_tmp+2] = (unsigned char)((count >> 8) & 0xFF);
                                ucbuffer[index_tmp+3] = (unsigned char)(count & 0xFF);
                                // New Section Header + Section Address
                                ucbuffer[index] = 0xDA;
                                ucbuffer[index+1] = 0xF6;
                                ucbuffer[index+2] = hi_addr_hi;
                                ucbuffer[index+3] = hi_addr_lo;
                                ucbuffer[index+4] = lo_addr_hi;
                                ucbuffer[index+5] = lo_addr_lo;
                                // skip 6,7,8,9 byte count, next section set it
                                index_tmp = index+6;
                                index += 10;
                                count = byte_count;

                                // set fw data
                                cptr += 9;
                                for (i = 0; i < byte_count; i++) {
                                    sscanf(cptr, "%02x", &tmp);
                                    ucbuffer[index+i] = (unsigned char)tmp;
                                    cptr += 2;
                                }
                                index += byte_count;
                            }
                        } else
                            printf("next_addr = 0!\n");

                        next_addr = byte_count/2 + addr;
                        //printf("next_addr = 0x%04X\n", next_addr);
                    }
                }
            }
            printf("index = %d\n", index);
            fclose(pfile_fd);

            if ( index == 0 ) {
                free(ucbuffer);
                printf("index = 0\n");
                SaveLog((char *)"FWupdate GetHbFWData() : index = 0", st_time);
                // wrong file?
                // delete first sn & file
                CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                memset(strtmp, 0x00, 1024);
                sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                system(strtmp);
                continue;
            }

            // set check sum
            //printf("count check sum start:\n");
            checksum = 0;
            for (i = 8; i < index; i++) {
                checksum += ucbuffer[i];
            }
            //printf("\ncount check sum end\n");
            checksum = ~checksum;
            checksum += 1;
            //printf("checksum = 0x%04X\n", checksum);
            ucbuffer[2] = (unsigned char)((checksum>>8) & 0x00FF);
            ucbuffer[3] = (unsigned char)(checksum & 0x00FF);
            printf("checksum = 0x%02X 0x%02X\n", ucbuffer[2], ucbuffer[3]);
        // darfon format
        } else {
            fseek(pfile_fd, 0, SEEK_END);
            index = ftell(pfile_fd);
            printf("size = %d\n", index);
            fseek(pfile_fd, 0, SEEK_SET);
            ucbuffer = calloc(index, sizeof(unsigned char));
            fread(ucbuffer, 1, index, pfile_fd);
            fclose(pfile_fd);
        }

        // debug print fw data
        //printf("======================== fw data =========================\n");
        //for (i = 0; i < index; i++) {
        //    if ( (ucbuffer[i] == 0xDA) && ((ucbuffer[i+1] == 0xF6) || (ucbuffer[i+1] == 0xF7)) )
        //        printf("\n");
        //    printf("%02X ", ucbuffer[i]);
        //}
        //printf("\n==========================================================\n");

        // save fw data to log
        count = 0;
        for (i = 0; i < index; i++) {
            // part tital
            if ( i%MAX_DATA_SIZE == 0 ) {
                count++;
                sprintf(strtmp, "FWupdate GetHbFWData() : Data count %d", count);
                SaveLog(strtmp, st_time);
                sprintf(strtmp, "FWupdate GetHbFWData() : Data =");
            }
            // value
            sprintf(read_buf, " %02X", ucbuffer[i]);
            strcat(strtmp, read_buf);
            // line end
            if ( i%MAX_DATA_SIZE == MAX_DATA_SIZE-1) {
                SaveLog(strtmp, st_time);
            } else if ( i == index-1 ) {
                SaveLog(strtmp, st_time);
            }
        }
        // save log immediately
        CloseLog();
        system("sync");
        OpenLog(g_SYSLOG_PATH, st_time);


        printf("==================== FW update start ====================\n");
        printf("gsncount = %d\n", gsncount);
        // fw update loop
        //for (count = 0; count < gsncount; count++) {
            //printf("count = %d\n", count);
            // check time
        while (1) {
            current_time = time(NULL);
            st_time = localtime(&current_time);

            if ( update_FW_start == update_FW_stop )
                break;
            else if ( update_FW_start < update_FW_stop ) {
                if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                    break;
            } else {
                if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                    break;
            }

            Updheartbeattime(current_time);
            CloseLog();
            system("sync");
            OpenLog(g_SYSLOG_PATH, st_time);
            printf("sleep 60 sec.\n");
            usleep(60000000);
        }

        // register part
        // remove register
        if ( gcomportfd > 0 ) {
            RemoveRegisterQuery(gcomportfd, 0);
            CleanRespond(gcomportfd);
            usleep(500000);
            RemoveRegisterQuery(gcomportfd, 0);
            CleanRespond(gcomportfd);
            usleep(500000);
            RemoveRegisterQuery(gcomportfd, 0);
            CleanRespond(gcomportfd);
            usleep(500000);
        }
        // register
        retval = RunRegister(snlist[loop].SN);
        if ( retval ) {
            // fail
            printf("RunRegister fail retval = %d\n", retval);
            sprintf(strtmp, "FWupdate GetHbFWData() : SN %s RunRegister FAIL", snlist[loop].SN);
            SaveLog(strtmp, st_time);
            //free(ucbuffer);
            //return 6;
            // delete the SN from list if stop time is not up
            current_time = time(NULL);
            st_time = localtime(&current_time);
            clearsn = 0;
            if ( update_FW_start == update_FW_stop )
                clearsn = 1;
            else if ( update_FW_start < update_FW_stop ) {
                if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                    clearsn = 1;
            } else {
                if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                    clearsn = 1;
            }
            if ( clearsn ) {
                // delete first sn & file
                CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                // search file in list, if exist, do not delete it
                for (i = loop+1; i < gsncount; i++ ) {
                    if ( strcmp(snlist[loop].FILE, snlist[i].FILE) == 0 ) {
                        clearfile = 0;
                        break;
                    } else
                        clearfile = 1;
                }
                if ( loop == gsncount-1 )
                    clearfile = 1;
                if ( clearfile ) {
                    memset(strtmp, 0x00, 1024);
                    sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                    system(strtmp);
                }
            }

        } else {
            // register OK, get slave id gV2id
            printf("RunRegister return OK, get gV2id = %d\n", gV2id);

            // send file header & check sum & byte count
            retval = WriteHBData(gV2id, ucbuffer, index);
            if ( retval ) {
                // fail
                printf("sn[%d] = %s write fw data FAIL\n", loop, snlist[loop].SN);
                sprintf(strtmp, "FWupdate GetHbFWData() : SN %s write data FAIL", snlist[loop].SN);
                SaveLog(strtmp, st_time);
                // delete the SN from list if stop time is not up
                current_time = time(NULL);
                st_time = localtime(&current_time);
                clearsn = 0;
                if ( update_FW_start == update_FW_stop )
                    clearsn = 1;
                else if ( update_FW_start < update_FW_stop ) {
                    if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                        clearsn = 1;
                } else {
                    if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                        clearsn = 1;
                }
                if ( clearsn ) {
                    // delete first sn & file
                    CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                    // search file in list, if exist, do not delete it
                    for (i = loop+1; i < gsncount; i++ ) {
                        if ( strcmp(snlist[loop].FILE, snlist[i].FILE) == 0 ) {
                            clearfile = 0;
                            break;
                        } else
                            clearfile = 1;
                    }
                    if ( loop == gsncount-1 )
                        clearfile = 1;
                    if ( clearfile ) {
                        memset(strtmp, 0x00, 1024);
                        sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                        printf("strtmp = %s\n", strtmp);
                        system(strtmp);
                    }
                }
            } else {
                // ok
                printf("sn[%d] = %s write fw data OK\n", loop, snlist[loop].SN);
                sprintf(strtmp, "FWupdate GetHbFWData() : SN %s write data OK", snlist[loop].SN);
                SaveLog(strtmp, st_time);
                // delete first sn & file
                CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                // search file in list, if exist, do not delete it
                for (i = loop+1; i < gsncount; i++ ) {
                    if ( strcmp(snlist[loop].FILE, snlist[i].FILE) == 0 ) {
                        clearfile = 0;
                        break;
                    } else
                        clearfile = 1;
                }
                if ( loop == gsncount-1 )
                    clearfile = 1;
                if ( clearfile ) {
                    memset(strtmp, 0x00, 1024);
                    sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                    system(strtmp);
                }
            }
        }

        printf("===================== FW update end =====================\n");

        free(ucbuffer);

        // save log immediately
        CloseLog();
        system("sync");
        OpenLog(g_SYSLOG_PATH, st_time);

        // debug
        usleep(30000000);
    }

    printf("######### GetHbFWData() end #########\n");

    return 0;
}

int RunBatFWUpdate(char *list_path)
{
    int i = 0, loop = 0, addr = 0, total_size = 0, retval = 0, clearsn = 0, clearfile = 0, retry = 0, fail_check = 0;
    unsigned short section = 0, section_size = 0, crc_tmp = 0;
    char strtmp[256] = {0}, valuetmp[16] = {0}; // for debug
    unsigned char read_buf[38] = {0}, crc_check[36] = {0};
    FILE *pfile_fd = NULL;

    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    printf("######### run RunBatFWUpdate() #########\n");

    printf("gsncount = %d\n", gsncount);

    for (loop = 0; loop < gsncount; loop++) {
        printf("loop = %d\n", loop);
        //size = 0;
        current_time = time(NULL);
        st_time = localtime(&current_time);

        Updheartbeattime(current_time);
        CloseLog();
        system("sync");
        OpenLog(g_SYSLOG_PATH, st_time);

        fail_check = 0;

        // open battery fw file
        pfile_fd = fopen(snlist[loop].FILE, "r");
        if ( pfile_fd == NULL ) {
            printf("#### Open %s FAIL ####\n", snlist[loop].FILE);
            memset(strtmp, 0x00, 256);
            sprintf(strtmp, "FWupdate RunBatFWUpdate() : Open %s fail:", snlist[loop].FILE);
            SaveLog(strtmp, st_time);
            // delete first sn & file
            CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
            memset(strtmp, 0x00, 256);
            sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
            system(strtmp);
            snlist[loop].status = 1;
            continue;
        }

        // read file header
        memset(read_buf, 0x00, 38);
        fread(read_buf, 1, 38, pfile_fd);
        // debug
        memset(strtmp, 0x00, 256);
        sprintf(strtmp, "FWupdate RunBatFWUpdate() : get header :");
        for ( i = 0; i < 38; i++) {
            sprintf(valuetmp, " %02X", (unsigned char)read_buf[i]);
            strcat(strtmp, valuetmp);
        }
        SaveLog(strtmp, st_time);
        strcat(strtmp, "\n");
        printf(strtmp);

        // check crc
        memset(crc_check, 0x00, 36);
        for ( i = 0; i < 36; i++)
            crc_check[i] = read_buf[i];
        //MakeReadDataCRC(crc_check,38);
        crc_tmp = CalculateCRC(crc_check, 36);
        printf("debug crc = 0x%04X, 0x%02X, 0x%02X\n", crc_tmp, crc_tmp >> 8, crc_tmp & 0x00FF);
        if ( (read_buf[37] != (crc_tmp >> 8)) || (read_buf[36] != (crc_tmp & 0x00FF)) ) {
            printf("#### %s header crc error ####\n", snlist[loop].FILE);
            memset(strtmp, 0x00, 256);
            sprintf(strtmp, "FWupdate RunBatFWUpdate() : %s header crc error:", snlist[loop].FILE);
            SaveLog(strtmp, st_time);
            // delete first sn & file
            CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
            memset(strtmp, 0x00, 256);
            sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
            system(strtmp);
            snlist[loop].status = 2;
            fclose(pfile_fd);
            continue;
        }

        // check file header
        if ( (read_buf[0] != 0x5A) || (read_buf[1] != 0xA5) || (read_buf[34] != 0xA5) || (read_buf[35] != 0x5A) ) {
            printf("#### %s header error ####\n", snlist[loop].FILE);
            memset(strtmp, 0x00, 256);
            sprintf(strtmp, "FWupdate RunBatFWUpdate() : %s header error:", snlist[loop].FILE);
            SaveLog(strtmp, st_time);
            // delete first sn & file
            CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
            memset(strtmp, 0x00, 256);
            sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
            system(strtmp);
            snlist[loop].status = 3;
            fclose(pfile_fd);
            continue;
        } else {
            snlist[loop].ver = (read_buf[18]<<8) + read_buf[19];
            printf("get ver = 0x%04X\n", snlist[loop].ver);
            addr = (read_buf[20]<<24) + (read_buf[21]<<16) + (read_buf[22]<<8) + read_buf[23];
            printf("get addr = 0x%08X\n", addr);
            total_size = (read_buf[24]<<24) + (read_buf[25]<<16) + (read_buf[26]<<8) + read_buf[27];
            printf("get total_size = 0x%08X\n", total_size);
            section = (read_buf[28]<<8) + read_buf[29];
            printf("get section = 0x%04X\n", section);
            section_size = (read_buf[30]<<8) + read_buf[31];
            printf("get section_size = 0x%04X\n", section_size);
        }

        // check time
        while (1) {
            current_time = time(NULL);
            st_time = localtime(&current_time);

            if ( update_FW_start == update_FW_stop )
                break;
            else if ( update_FW_start < update_FW_stop ) {
                if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                    break;
            } else {
                if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                    break;
            }

            Updheartbeattime(current_time);
            CloseLog();
            system("sync");
            OpenLog(g_SYSLOG_PATH, st_time);
            printf("sleep 60 sec.\n");
            usleep(60000000);
        }

        // register part
        // remove register
        if ( gcomportfd > 0 ) {
            RemoveRegisterQuery(gcomportfd, 0);
            CleanRespond(gcomportfd);
            usleep(500000);
            RemoveRegisterQuery(gcomportfd, 0);
            CleanRespond(gcomportfd);
            usleep(500000);
            RemoveRegisterQuery(gcomportfd, 0);
            CleanRespond(gcomportfd);
            usleep(500000);
        }

        // register
        retval = RunRegister(snlist[loop].SN);
        if ( retval ) {
            // fail
            printf("RunRegister fail retval = %d\n", retval);
            sprintf(strtmp, "FWupdate RunBatFWUpdate() : SN %s RunRegister FAIL", snlist[loop].SN);
            SaveLog(strtmp, st_time);
            // delete the SN from list if stop time is not up
            current_time = time(NULL);
            st_time = localtime(&current_time);
            clearsn = 0;
            if ( update_FW_start == update_FW_stop )
                clearsn = 1;
            else if ( update_FW_start < update_FW_stop ) {
                if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                    clearsn = 1;
            } else {
                if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                    clearsn = 1;
            }
            if ( clearsn ) {
                // delete first sn & file
                CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                // search file in list, if exist, do not delete it
                for (i = loop+1; i < gsncount; i++ ) {
                    if ( strcmp(snlist[loop].FILE, snlist[i].FILE) == 0 ) {
                        clearfile = 0;
                        break;
                    } else
                        clearfile = 1;
                }
                if ( loop == gsncount-1 )
                    clearfile = 1;
                if ( clearfile ) {
                    memset(strtmp, 0x00, 256);
                    sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                    system(strtmp);
                }
            }

            snlist[loop].status = 4;
            fclose(pfile_fd);
            continue;

        } else {
            // register OK, get slave id gV2id
            printf("RunRegister return OK, get gV2id = %d\n", gV2id);

            // fro test
            GetBatVer(loop, gV2id);
            usleep(200000);
            //

            // stop battery
            retval = RunStopBat(loop, gV2id);
            if ( retval != 0 ) {
                // delete the SN from list if stop time is not up
                current_time = time(NULL);
                st_time = localtime(&current_time);
                clearsn = 0;
                if ( update_FW_start == update_FW_stop )
                    clearsn = 1;
                else if ( update_FW_start < update_FW_stop ) {
                    if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                        clearsn = 1;
                } else {
                    if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                        clearsn = 1;
                }
                if ( clearsn ) {
                    // delete first sn & file
                    CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                    // search file in list, if exist, do not delete it
                    for (i = loop+1; i < gsncount; i++ ) {
                        if ( strcmp(snlist[loop].FILE, snlist[i].FILE) == 0 ) {
                            clearfile = 0;
                            break;
                        } else
                            clearfile = 1;
                    }
                    if ( loop == gsncount-1 )
                        clearfile = 1;
                    if ( clearfile ) {
                        memset(strtmp, 0x00, 256);
                        sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                        system(strtmp);
                    }
                }

                // stop battery fail
                printf("stop battery fail, retval = %d\n", retval);
                fclose(pfile_fd);
                continue;
            }
            // add sleep 0.5 s
            usleep(500000);

            // 2.5.3.2
            // send check bms info
            memset(&mybatinfo, 0x00, sizeof(mybatinfo));
            retval = GetBatInfo(loop, gV2id, 40);
            if ( retval != 0 ) {
                // delete the SN from list if stop time is not up
                current_time = time(NULL);
                st_time = localtime(&current_time);
                clearsn = 0;
                if ( update_FW_start == update_FW_stop )
                    clearsn = 1;
                else if ( update_FW_start < update_FW_stop ) {
                    if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                        clearsn = 1;
                } else {
                    if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                        clearsn = 1;
                }
                if ( clearsn ) {
                    // delete first sn & file
                    CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                    // search file in list, if exist, do not delete it
                    for (i = loop+1; i < gsncount; i++ ) {
                        if ( strcmp(snlist[loop].FILE, snlist[i].FILE) == 0 ) {
                            clearfile = 0;
                            break;
                        } else
                            clearfile = 1;
                    }
                    if ( loop == gsncount-1 )
                        clearfile = 1;
                    if ( clearfile ) {
                        memset(strtmp, 0x00, 256);
                        sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                        system(strtmp);
                    }
                }

                // get bat info fail
                printf("get bat info fail, retval = %d\n", retval);
                fclose(pfile_fd);
                RunStartBat(loop, gV2id);
                continue;
            }
            // send enter update cmd. if status 3. app mode
            //mybatinfo.status = 3; // for test
            if ( mybatinfo.status == 3 ) {
                retry = 20;
                while ( (mybatinfo.status == 3) && retry ) {
                    // send entry update cmd.
                    SetControl(loop, gV2id, 0xE0, 1);
                    usleep(1000000);
                    retval = GetBatInfo(loop, gV2id, 3);
                    retry--;
                    //mybatinfo.status = 0; // for test
                }
            } else {
                // delete the SN from list if stop time is not up
                /*current_time = time(NULL);
                st_time = localtime(&current_time);
                clearsn = 0;
                if ( update_FW_start == update_FW_stop )
                    clearsn = 1;
                else if ( update_FW_start < update_FW_stop ) {
                    if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                        clearsn = 1;
                } else {
                    if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                        clearsn = 1;
                }
                if ( clearsn ) {
                    // delete first sn & file
                    CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                    // search file in list, if exist, do not delete it
                    for (i = loop+1; i < gsncount; i++ ) {
                        if ( strcmp(snlist[loop].FILE, snlist[i].FILE) == 0 ) {
                            clearfile = 0;
                            break;
                        } else
                            clearfile = 1;
                    }
                    if ( loop == gsncount-1 )
                        clearfile = 1;
                    if ( clearfile ) {
                        memset(strtmp, 0x00, 256);
                        sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                        system(strtmp);
                    }
                }*/

                sprintf(strtmp, "FWupdate RunBatFWUpdate() : step 2.5.3.2 mybatinfo.status = %d, end", mybatinfo.status);
                SaveLog(strtmp, st_time);
                snlist[loop].status = 5;
                //fclose(pfile_fd);
                //RunStartBat(loop, gV2id);
                //continue;
            }
            if ( retval ) {
                fail_check = 1;
            }

            if ( fail_check == 0 ) {
                // 2.5.3.3
                //mybatinfo.status = 0; // for test
                if ( mybatinfo.status == 0 ) {
                    retry = 10;
                    while ( (mybatinfo.status != 1) && retry ) {
                        // send entry update cmd.
                        SetControl(loop, gV2id, 0xE0, 1);
                        usleep(3000000);
                        retval = GetBatInfo(loop, gV2id, 3);
                        retry--;
                        //mybatinfo.status = 1; // for test
                    }
                } else {
                    // delete the SN from list if stop time is not up
                    /*current_time = time(NULL);
                    st_time = localtime(&current_time);
                    clearsn = 0;
                    if ( update_FW_start == update_FW_stop )
                        clearsn = 1;
                    else if ( update_FW_start < update_FW_stop ) {
                        if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                            clearsn = 1;
                    } else {
                        if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                            clearsn = 1;
                    }
                    if ( clearsn ) {
                        // delete first sn & file
                        CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                        // search file in list, if exist, do not delete it
                        for (i = loop+1; i < gsncount; i++ ) {
                            if ( strcmp(snlist[loop].FILE, snlist[i].FILE) == 0 ) {
                                clearfile = 0;
                                break;
                            } else
                                clearfile = 1;
                        }
                        if ( loop == gsncount-1 )
                            clearfile = 1;
                        if ( clearfile ) {
                            memset(strtmp, 0x00, 256);
                            sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                            system(strtmp);
                        }
                    }*/

                    sprintf(strtmp, "FWupdate RunBatFWUpdate() : step 2.5.3.3 mybatinfo.status = %d, end", mybatinfo.status);
                    SaveLog(strtmp, st_time);
                    snlist[loop].status = 6;
                    //fclose(pfile_fd);
                    //RunStartBat(loop, gV2id);
                    //continue;
                }
                if ( retval ) {
                    fail_check = 2;
                }
            }

            if ( fail_check == 0 ) {
                // 2.5.3.4
                //mybatinfo.status = 1; // for test
                if ( mybatinfo.status == 1 ) {
                    retry = 20;
                    while ( !((mybatinfo.status == 2) && (mybatinfo.burn_status == 4)) && retry ) {
                        // send entry update cmd.
                        SetControl(loop, gV2id, 0xE1, 1);
                        usleep(1000000);
                        retval = GetBatInfo(loop, gV2id, 3);
                        retry--;
                        //mybatinfo.status = 2; // for test
                        //mybatinfo.burn_status = 2; // for test
                    }

                    if ( ((mybatinfo.status == 2) && (mybatinfo.burn_status == 4)) ) {
                        retry = 20;
                        retval = 0;
                        while ( retry ) {
                            // send file header
                            SetHeader(loop, gV2id, read_buf);
                            usleep(1000000);
                            GetBatInfo(loop, gV2id, 3);
                            if (mybatinfo.burn_status == 5 || mybatinfo.burn_status == 9 || mybatinfo.burn_status == 10 || mybatinfo.burn_status == 12 || mybatinfo.burn_status == 13) {
                                retval = 1;
                                break;
                            } else if (mybatinfo.burn_status == 4) {
                                retval = 2;
                                break;
                            } else if (mybatinfo.burn_status == 11) {
                                retval = 3;
                                break;
                            } else if ( ((mybatinfo.status == 1) && (mybatinfo.burn_status == 2)) ) {
                                retval = 4;
                                break;
                            } else {
                                retval = 5;
                                break;
                            }

                            retry--;
                        }

                        // check retval
                        switch (retval)
                        {
                            case 1:
                                printf("stop update\n");
                                StopBatFWUpdate(loop, gV2id);
                                usleep(1000000);
                                CheckResult(loop, gV2id);
                                RunStartBat(loop, gV2id);
                                snlist[loop].status = 7;
                                break;
                            case 2:
                                printf("start update\n");
                                WriteBatData(loop, gV2id, section, section_size, pfile_fd);
                                //usleep(1000000);
                                StopBatFWUpdate(loop, gV2id);
                                usleep(1000000);
                                CheckResult(loop, gV2id);
                                RunStartBat(loop, gV2id);
                                break;
                            case 3:
                                printf("wait slave update\n");
                                StopBatFWUpdate(loop, gV2id);
                                usleep(1000000);
                                CheckResult(loop, gV2id);
                                RunStartBat(loop, gV2id);
                                snlist[loop].status = 8;
                                break;
                            case 4:
                                printf("same version\n");
                                StopBatFWUpdate(loop, gV2id);
                                usleep(1000000);
                                CheckResult(loop, gV2id);
                                RunStartBat(loop, gV2id);
                                snlist[loop].status = 9;
                                break;
                            case 5:
                                printf("unknow status\n");
                                StopBatFWUpdate(loop, gV2id);
                                usleep(1000000);
                                CheckResult(loop, gV2id);
                                RunStartBat(loop, gV2id);
                                snlist[loop].status = 10;
                                break;
                        }
                        // delete the SN from list if stop time is not up
                        current_time = time(NULL);
                        st_time = localtime(&current_time);
                        clearsn = 0;
                        if ( update_FW_start == update_FW_stop )
                            clearsn = 1;
                        else if ( update_FW_start < update_FW_stop ) {
                            if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                                clearsn = 1;
                        } else {
                            if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                                clearsn = 1;
                        }
                        if ( clearsn ) {
                            // delete first sn & file
                            CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                            // search file in list, if exist, do not delete it
                            for (i = loop+1; i < gsncount; i++ ) {
                                if ( strcmp(snlist[loop].FILE, snlist[i].FILE) == 0 ) {
                                    clearfile = 0;
                                    break;
                                } else
                                    clearfile = 1;
                            }
                            if ( loop == gsncount-1 )
                                clearfile = 1;
                            if ( clearfile ) {
                                memset(strtmp, 0x00, 256);
                                sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                                system(strtmp);
                            }
                        }

                    } else {
                        // delete the SN from list if stop time is not up
                        current_time = time(NULL);
                        st_time = localtime(&current_time);
                        clearsn = 0;
                        if ( update_FW_start == update_FW_stop )
                            clearsn = 1;
                        else if ( update_FW_start < update_FW_stop ) {
                            if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                                clearsn = 1;
                        } else {
                            if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                                clearsn = 1;
                        }
                        if ( clearsn ) {
                            // delete first sn & file
                            CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                            // search file in list, if exist, do not delete it
                            for (i = loop+1; i < gsncount; i++ ) {
                                if ( strcmp(snlist[loop].FILE, snlist[i].FILE) == 0 ) {
                                    clearfile = 0;
                                    break;
                                } else
                                    clearfile = 1;
                            }
                            if ( loop == gsncount-1 )
                                clearfile = 1;
                            if ( clearfile ) {
                                memset(strtmp, 0x00, 256);
                                sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                                system(strtmp);
                            }
                        }

                        sprintf(strtmp, "FWupdate RunBatFWUpdate() : step 2.5.3.4 step2 mybatinfo.status = %d, mybatinfo.burn_status = %d, end", mybatinfo.status, mybatinfo.burn_status);
                        SaveLog(strtmp, st_time);
                        StopBatFWUpdate(loop, gV2id);
                        usleep(1000000);
                        CheckResult(loop, gV2id);
                        RunStartBat(loop, gV2id);
                        snlist[loop].status = 54;
                        fclose(pfile_fd);
                        continue;
                    }
                } else {
                    // delete the SN from list if stop time is not up
                    current_time = time(NULL);
                    st_time = localtime(&current_time);
                    clearsn = 0;
                    if ( update_FW_start == update_FW_stop )
                        clearsn = 1;
                    else if ( update_FW_start < update_FW_stop ) {
                        if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                            clearsn = 1;
                    } else {
                        if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                            clearsn = 1;
                    }
                    if ( clearsn ) {
                        // delete first sn & file
                        CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                        // search file in list, if exist, do not delete it
                        for (i = loop+1; i < gsncount; i++ ) {
                            if ( strcmp(snlist[loop].FILE, snlist[i].FILE) == 0 ) {
                                clearfile = 0;
                                break;
                            } else
                                clearfile = 1;
                        }
                        if ( loop == gsncount-1 )
                            clearfile = 1;
                        if ( clearfile ) {
                            memset(strtmp, 0x00, 256);
                            sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                            system(strtmp);
                        }
                    }

                    sprintf(strtmp, "FWupdate RunBatFWUpdate() : step 2.5.3.4 step1 mybatinfo.status = %d, end", mybatinfo.status);
                    SaveLog(strtmp, st_time);
                    StopBatFWUpdate(loop, gV2id);
                    usleep(1000000);
                    CheckResult(loop, gV2id);
                    RunStartBat(loop, gV2id);
                    snlist[loop].status = 55;
                    fclose(pfile_fd);
                    continue;
                }
            } else {
                // delete the SN from list if stop time is not up
                current_time = time(NULL);
                st_time = localtime(&current_time);
                clearsn = 0;
                if ( update_FW_start == update_FW_stop )
                    clearsn = 1;
                else if ( update_FW_start < update_FW_stop ) {
                    if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                        clearsn = 1;
                } else {
                    if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                        clearsn = 1;
                }
                if ( clearsn ) {
                    // delete first sn & file
                    CLEANSN(snlist[loop].SN, snlist[loop].FILE, list_path);
                    // search file in list, if exist, do not delete it
                    for (i = loop+1; i < gsncount; i++ ) {
                        if ( strcmp(snlist[loop].FILE, snlist[i].FILE) == 0 ) {
                            clearfile = 0;
                            break;
                        } else
                            clearfile = 1;
                    }
                    if ( loop == gsncount-1 )
                        clearfile = 1;
                    if ( clearfile ) {
                        memset(strtmp, 0x00, 256);
                        sprintf(strtmp, "rm %s; sync;", snlist[loop].FILE);
                        system(strtmp);
                    }
                }

                sprintf(strtmp, "FWupdate RunBatFWUpdate() : fail_check = %d, end", fail_check);
                SaveLog(strtmp, st_time);
                StopBatFWUpdate(loop, gV2id);
                usleep(1000000);
                CheckResult(loop, gV2id);
                RunStartBat(loop, gV2id);
                snlist[loop].status = 60;
                fclose(pfile_fd);
                continue;
            }

        }

        if (pfile_fd)
            fclose(pfile_fd);
    }

    printf("######### RunBatFWUpdate() end #########\n");

    return 0;
}

int stopProcess()
{
    printf("run stopProcess()\n");

    // kill process
    system("killall -9 SWupdate.exe");
    system("sleep 1; sync;");
    system("killall -9 dlg320.exe");
    system("sleep 1; sync;");
    system("killall -9 DataProgram.exe");
    system("sleep 1; sync;");
    system("killall -9 DLsocket.exe");
    system("sleep 1; sync;");
    printf("stopProcess() end\n");

    return 0;
}

int runProcess()
{
    //printf("run runProcess()\n");
    // call boot script
    //system("/usr/home/run_DLSW.sh");
    //system("sync");

    time_t      current_time;
    struct tm   *st_time = NULL;
    current_time = time(NULL);
    st_time = localtime(&current_time);

    printf("run reboot\n");
    SaveLog((char *)"FWupdate end, reboot", st_time);
    CloseLog();
    system("sync");
    system("reboot");

    return 0;
}

int GetPort(char *file_path)
{
    FILE *pfile_fd = NULL;
    char buf[512] = {0};
    char *cptr = NULL;
    int comport = 0;

    time_t      current_time;
    struct tm   *st_time = NULL;
    current_time = time(NULL);
    st_time = localtime(&current_time);
    SaveLog((char *)"FWupdate GetPort() : start", st_time);

    printf("######### run GetPort() #########\n");

    pfile_fd = fopen(file_path, "rb");
    if ( pfile_fd == NULL ) {
        printf("#### Open %s Fail ####\n", file_path);
        sprintf(buf, "FWupdate GetPort() : Open %s Fail", file_path);
        SaveLog(buf, st_time);
        return -1;
    }

    // get com port
    while ( fgets(buf, 512, pfile_fd) != NULL ) {
        cptr = strstr(buf, "<port>");
        if ( cptr ) {
            sscanf(cptr, "<port>COM%d</port>", &comport);
            printf("get comport = %d\n", comport);
        }

        cptr = strstr(buf, "DARFON");
        if ( cptr ) {
            printf("get model DARFON\n");
            break;
        }
    }
    fclose(pfile_fd);
    printf("final comport = %d\n", comport);
    sprintf(buf, "FWupdate GetPort() : Get comport %d", comport);
    SaveLog(buf, st_time);

    printf("######### GetPort() end #########\n");

    return comport;
}

int GetMIList(char *file_path)
{
    FILE *pfile_fd = NULL;
    char buf[512] = {0}, tmpsn[17] = {0};
    char *cptr = NULL;
    int tmpid = 0, tmptype = -1, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0, tmp6 = 0, tmp7 = 0, tmp8 = 0;

    time_t      current_time;
    struct tm   *st_time = NULL;
    current_time = time(NULL);
    st_time = localtime(&current_time);
    SaveLog((char *)"FWupdate GetMIList() : start", st_time);

    // initial
    gmicount = 0;
    for (tmpid = 0; tmpid < 255; tmpid++) {
        memset(milist[tmpid].SN, 0x00, 17);
        milist[tmpid].slave_id = 0;
        memset(milist[tmpid].sn_bin, 0x00, 8);
        milist[tmpid].OtherType = -1;
    }

    printf("######### run GetMIList() #########\n");

    pfile_fd = fopen(file_path, "rb");
    if ( pfile_fd == NULL ) {
        printf("#### Open %s Fail ####\n", file_path);
        sprintf(buf, "FWupdate GetMIList() : Open %s Fail", file_path);
        SaveLog(buf, st_time);
        return -1;
    }

    // get list
    while ( fgets(buf, 512, pfile_fd) != NULL ) {
        // get OriSn
        cptr = strstr(buf, "<OriSn>");
        if ( cptr ) {
            sscanf(cptr, "<OriSn>%16s</OriSn>", tmpsn);
            printf("tmpsn = %s\n", tmpsn);
        }

        // get slaveId
        cptr = strstr(buf, "<slaveId>");
        if ( cptr ) {
            sscanf(cptr, "<slaveId>%d</slaveId>", &tmpid);
            printf("tmpid = %d\n", tmpid);
        }

        // get Manufacturer
        cptr = strstr(buf, "DARFON");
        if ( cptr ) {
            printf("get model DARFON\n");
            // get OtherType
            fgets(buf, 512, pfile_fd);
            fgets(buf, 512, pfile_fd);
            cptr = strstr(buf, "<OtherType>");
            if ( cptr ) {
                sscanf(cptr, "<OtherType>%d</OtherType>", &tmptype);
                printf("tmptype = %d\n", tmptype);
            }

            // skip B part
            if ( gmicount > 0 ) {
                // check milist data (G640), skip the same sn & id (ex. G640 A & B part)
                if ( tmpid == milist[gmicount-1].slave_id ) {
                    printf("skip sn = %s, id = %d\n", tmpsn, tmpid);
                    continue;
                }
            }
            strcpy(milist[gmicount].SN, tmpsn);
            milist[gmicount].slave_id = tmpid;
            milist[gmicount].OtherType = tmptype;
            sscanf(milist[gmicount].SN, "%02X%02X%02X%02X%02X%02X%02X%02X",
                &tmp1, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6, &tmp7, &tmp8);
            milist[gmicount].sn_bin[0] = (unsigned char)tmp1;
            milist[gmicount].sn_bin[1] = (unsigned char)tmp2;
            milist[gmicount].sn_bin[2] = (unsigned char)tmp3;
            milist[gmicount].sn_bin[3] = (unsigned char)tmp4;
            milist[gmicount].sn_bin[4] = (unsigned char)tmp5;
            milist[gmicount].sn_bin[5] = (unsigned char)tmp6;
            milist[gmicount].sn_bin[6] = (unsigned char)tmp7;
            milist[gmicount].sn_bin[7] = (unsigned char)tmp8;
            printf("set mi list %d : sn = %s, id = %d, type = %d\n", gmicount, milist[gmicount].SN, milist[gmicount].slave_id, milist[gmicount].OtherType);
            printf("ucsn = 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
                milist[gmicount].sn_bin[0], milist[gmicount].sn_bin[1], milist[gmicount].sn_bin[2], milist[gmicount].sn_bin[3],
                milist[gmicount].sn_bin[4], milist[gmicount].sn_bin[5], milist[gmicount].sn_bin[6], milist[gmicount].sn_bin[7]);
            gmicount++;
        }
    }
    fclose(pfile_fd);

    // debug print
    printf("===========================================\n");
    printf("mi list count = %d\n", gmicount);
    for (tmpid = 0; tmpid < gmicount; tmpid++) {
        printf("List %03d : sn = %s, id = %d, type = %d\n", tmpid, milist[tmpid].SN, milist[tmpid].slave_id, milist[tmpid].OtherType);
        sprintf(buf, "FWupdate GetMIList() : List %03d : sn = %s, id = %d, type = %d", tmpid, milist[tmpid].SN, milist[tmpid].slave_id, milist[tmpid].OtherType);
        SaveLog(buf, st_time);
    }
    printf("===========================================\n");

    sprintf(buf, "FWupdate GetMIList() : Get mi list count %d", gmicount);
    SaveLog(buf, st_time);

    printf("######### GetMIList() end #########\n");

    return 0;
}

int GetSNList(char *list_path)
{
    FILE *pfile_fd = NULL;
    char buf[512] = {0};
    int i = 0;

    time_t      current_time;
    struct tm   *st_time = NULL;
    current_time = time(NULL);
    st_time = localtime(&current_time);
    SaveLog((char *)"FWupdate GetSNList() : start", st_time);

    // initial
    gsncount = 0;
    for (i = 0; i < 255; i++) {
        memset(snlist[i].SN, 0x00, 17);
        memset(snlist[i].FILE, 0x00, 128);
        snlist[i].status = 0;
        snlist[i].ver = 0;
    }

    printf("######### run GetSNList() #########\n");
    printf("list_path = %s\n", list_path);

    pfile_fd = fopen(list_path, "rb");
    if ( pfile_fd == NULL ) {
        printf("#### Open %s Fail ####\n", list_path);
        sprintf(buf, "FWupdate GetSNList() : Open %s Fail", list_path);
        SaveLog(buf, st_time);
        return -1;
    }

    // mi unicast
    if ( strstr(list_path, MIFW_LIST) != NULL ) {
        // get sn & file
        printf("set mi unicast list\n");
        while ( fgets(buf, 512, pfile_fd) != NULL ) {
            if ( strlen(buf) > 16 ) {
                sscanf(buf, "%16s %127s", snlist[gsncount].SN, snlist[gsncount].FILE);
                gsncount++;
                memset(buf, 0x00, 512);
            }
        }
    // mi broadcast
    } else if ( strstr(list_path, MIFW_BLIST) != NULL ) {
        // set file
        printf("set mi broadcast list\n");
        fgets(buf, 512, pfile_fd);
        sscanf(buf, "%16s %127s", snlist[0].SN, snlist[0].FILE);
        gsncount = 1;
    // hybrid unicast
    } else if ( strstr(list_path, HBFW_LIST) != NULL ) {
        // get sn & file
        printf("set hybrid unicast list\n");
        while ( fgets(buf, 512, pfile_fd) != NULL ) {
            if ( strlen(buf) > 16 ) {
                sscanf(buf, "%16s %127s", snlist[gsncount].SN, snlist[gsncount].FILE);
                gsncount++;
                memset(buf, 0x00, 512);
            }
        }
    // plc
    } else if ( strstr(list_path, PLC_LIST) != NULL ) {
        // set file
        printf("set plc list\n");
        fgets(buf, 512, pfile_fd);
        sscanf(buf, "%16s %127s", snlist[0].SN, snlist[0].FILE);
        gsncount = 1;
    // plc module
    } else if ( strstr(list_path, PLCM_LIST) != NULL ) {
        // set file
        printf("set plc module list\n");
        fgets(buf, 512, pfile_fd);
        sscanf(buf, "%16s %127s", snlist[0].SN, snlist[0].FILE);
        gsncount = 1;
    // battery fw
    } else if ( strstr(list_path, BATFW_LIST) != NULL ) {
        // get sn & file
        printf("set battery fw list\n");
        while ( fgets(buf, 512, pfile_fd) != NULL ) {
            if ( strlen(buf) > 16 ) {
                sscanf(buf, "%16s %127s", snlist[gsncount].SN, snlist[gsncount].FILE);
                gsncount++;
                memset(buf, 0x00, 512);
            }
        }
    } else {
        // list_path error
        printf("list_path error!\n");
    }
    fclose(pfile_fd);

    // debug print
    printf("===========================================\n");
    printf("sn list count = %d\n", gsncount);
    for (i = 0; i < gsncount; i++) {
        printf("set sn list %d : sn = %s, file = %s\n", i, snlist[i].SN, snlist[i].FILE);
        sprintf(buf, "FWupdate GetSNList() : set sn list %d : sn = %s, file = %s", i, snlist[i].SN, snlist[i].FILE);
        SaveLog(buf, st_time);
    }
    printf("===========================================\n");

    sprintf(buf, "FWupdate GetSNList() : Get sn list count %d", gsncount);
    SaveLog(buf, st_time);

    printf("######### GetSNList() end #########\n");

    return 0;
}

int CheckType(int index, int *ret)
{
    int i = 0;
    char buf[512] = {0};

    time_t      current_time;
    struct tm   *st_time = NULL;
    current_time = time(NULL);
    st_time = localtime(&current_time);
    SaveLog((char *)"FWupdate CheckType() : start", st_time);

    printf("######### run CheckType() #########\n");
    // check sn in milist
    for (i = 0; i < gmicount; i++) {
        // match sn
        if ( strncmp(snlist[index].SN, milist[i].SN, 16) == 0 ) {
            sprintf(buf, "FWupdate CheckType() : match index %d", i);
            SaveLog(buf, st_time);
            *ret = i;
            printf("match index %d\n", *ret);
            printf("######### CheckType() end #########\n");
            return milist[i].OtherType;
        }
    }

    printf("######### CheckType() end #########\n");
    *ret = -1;
    return -1;
}

int DoUpdate(char *list_path)
{
    FILE *pfile_fd = NULL;
    char buf[512] = {0}, strtmp[512] = {0};
    char FILENAME[64] = {0};
    int comport = 0;//, ret = 0, index = -1;
    struct stat listst;

    time_t      current_time;
    struct tm   *st_time = NULL;
    current_time = time(NULL);
    st_time = localtime(&current_time);
    SaveLog((char *)"FWupdate DoUpdate() : start", st_time);

    printf("######### run DoUpdate() #########\n");

    while (1)
    {
        current_time = time(NULL);
        st_time = localtime(&current_time);
        CloseLog();
        system("sync");
        OpenLog(g_SYSLOG_PATH, st_time);

        // get milist name
        printf("Get MIList\n");
        system("cd /tmp; ls MIList_* > /tmp/MIList");

        pfile_fd = fopen("/tmp/MIList", "rb");
        if ( pfile_fd == NULL ) {
            printf("#### Open /tmp/MIList Fail ####\n");
            printf("wait 1 min.\n");
            SaveLog((char *)"FWupdate DoUpdate() : Open /tmp/MIList Fail, wait 1 min.", st_time);
            usleep(60000000); // 60s;
            continue;
        }
        // get file name
        memset(buf, 0, 512);
        fgets(buf, 64, pfile_fd);
        fclose(pfile_fd);

        if ( strlen(buf) ) {
            buf[strlen(buf)-1] = 0; // set '\n' to 0
            sprintf(FILENAME, "/tmp/%s", buf);
            printf("FILENAME = %s\n", FILENAME);
            break;
        } else {
            printf("Empty file! Plese wait 1 min. to create MIList!\n");
            SaveLog((char *)"FWupdate DoUpdate() : MIList not found, wait 1 min.", st_time);
            usleep(60000000); // 60s;
            continue;
        }
    }

    // stop process
    stopProcess();

    // get Darfon use com port
    comport = GetPort(FILENAME);
    // get mi list
    GetMIList(FILENAME);
    // get sn list
    GetSNList(list_path);

    current_time = time(NULL);
    st_time = localtime(&current_time);

    // open com port
    if ( gcomportfd == 0 ) {
        // get com port setting
        GetComPortSetting(comport);
        // open com port, get gcomportfd
        OpenComPort(comport);
        sprintf(strtmp, "FWupdate DoUpdate() : Get comport fd %d", gcomportfd);
        SaveLog(strtmp, st_time);
    }

    if ( gsncount == 0 ) {
        printf("gsncount = 0, exit DoUpdate.\n");
        printf("remove sn list.\n");
        sprintf(buf, "rm %s; sync; sync", list_path);
        system(buf);
        ModbusDrvDeinit(gcomportfd);
        return 0;
    }

    // plc
    if ( strstr(list_path, PLC_LIST) != NULL ) {
        printf("Do PLC update\n");
        // run plc update function here
        printf("update end, remove fw file & list.\n");
        sprintf(buf, "rm %s; sync; sync", snlist[0].FILE);
        system(buf);
        sprintf(buf, "rm %s; sync; sync", list_path);
        system(buf);
    // plc module
    } else if ( strstr(list_path, PLCM_LIST) != NULL ) {
        printf("Do PLC Module update\n");
        // run plc module update function here
        printf("update end, remove fw file & list.\n");
        sprintf(buf, "rm %s; sync; sync", snlist[0].FILE);
        system(buf);
        sprintf(buf, "rm %s; sync; sync", list_path);
        system(buf);
    // mi unicast
    } else if ( strstr(list_path, MIFW_LIST) != NULL ) {
        // check version 2.0 or 3.0
        if ( gcomportfd > 0 ) {
            if ( CheckVer() ) {
                // fail
                printf("Use V2.0 protocol\n");

                RemoveRegisterQuery(gcomportfd, 0);
                CleanRespond(gcomportfd);
                usleep(500000);
                RemoveRegisterQuery(gcomportfd, 0);
                CleanRespond(gcomportfd);
                usleep(500000);
                RemoveRegisterQuery(gcomportfd, 0);
                CleanRespond(gcomportfd);
                usleep(500000);
            } else {
                // success
                printf("Use V3.0 protocol\n");
            }
        }
        printf("gprotocolver = %d\n", gprotocolver);

        if ( gprotocolver ) {
            printf("run GetFWData()\n");
            GetFWData(list_path);
            // check list status & clean list
            if ( stat(list_path, &listst) == 0 ) {
                if ( listst.st_size < 16 ) {
                    printf("update end, remove fw list.\n");
                    sprintf(buf, "rm %s; sync; sync", list_path);
                    system(buf);
                }
            }
        }
    // mi broadcast
    } else if ( strstr(list_path, MIFW_BLIST) != NULL ) {
        printf("Do MI broadcast update\n");
        // run MI broadcast function here
        printf("update end, remove fw file & list.\n");
        sprintf(buf, "rm %s; sync; sync", snlist[0].FILE);
        system(buf);
        sprintf(buf, "rm %s; sync; sync", list_path);
        system(buf);
    // hybrid unicast
    } else if ( strstr(list_path, HBFW_LIST) != NULL ) {
        printf("run GetHbFWData()\n");
        GetHbFWData(list_path);
        // check list status & clean list
        if ( stat(list_path, &listst) == 0 ) {
            if ( listst.st_size < 16 ) {
                printf("update end, remove fw list.\n");
                sprintf(buf, "rm %s; sync; sync", list_path);
                system(buf);
            }
        }
    // battery fw
    } else if ( strstr(list_path, BATFW_LIST) != NULL ) {
        printf("run RunBatFWUpdate()\n");
        RunBatFWUpdate(list_path);
        CloseLog(); //for test
        // set update result, then upload
        updBATFWstatus();
        // check list status & clean list
        if ( stat(list_path, &listst) == 0 ) {
            if ( listst.st_size < 16 ) {
                printf("update end, remove fw list.\n");
                sprintf(buf, "rm %s; sync; sync", list_path);
                system(buf);
            }
        }
        system("sync; sync");
        printf("sleep 10 sec. for save log\n");
        usleep(10000000); //for test
    } else {
        printf("Do nothing, list_path = %s\n", list_path);
    }


/*    // mi unicast
    if ( (strstr(file_path, USB_MIFW_FILE) != NULL) || (strstr(file_path, TMP_MIFW_FILE) != NULL) ) {
        // check version 2.0 or 3.0
        if ( gcomportfd > 0 ) {
            if ( CheckVer() ) {
                // fail
                printf("Use V2.0 protocol\n");

                RemoveRegisterQuery(gcomportfd, 0);
                CleanRespond();
                usleep(500000);
                RemoveRegisterQuery(gcomportfd, 0);
                CleanRespond();
                usleep(500000);
                RemoveRegisterQuery(gcomportfd, 0);
                CleanRespond();
                usleep(500000);
            } else {
                // success
                printf("Use V3.0 protocol\n");
            }
        }
        printf("gprotocolver = %d\n", gprotocolver);

        if ( gprotocolver ) {
            printf("run GetFWData()\n");
            GetFWData(file_path, list_path);
            // check list status
            if ( stat(list_path, &listst) == 0 ) {
                if ( listst.st_size < 16 ) {
                    printf("update end, remove fw file.\n");
                    sprintf(buf, "rm %s; sync; sync", file_path);
                    system(buf);
                    printf("remove sn list.\n");
                    sprintf(buf, "rm %s; sync; sync", list_path);
                    system(buf);
                }
            }
        }
    // mi broadcast
    } else if ( (strstr(file_path, USB_MIFW_BFILE) != NULL) || (strstr(file_path, TMP_MIFW_BFILE) != NULL) ) {
        printf("run GetFWBCData()\n");
        // GetFWBCData(file_path, list_path); // 0x50 & 0x51
        printf("update end, remove fw file.\n");
        sprintf(buf, "rm %s; sync; sync", file_path);
        system(buf);
        printf("remove sn list.\n");
        sprintf(buf, "rm %s; sync; sync", list_path);
        system(buf);
        return 0;
    // hybrid
    } else if ( (strstr(file_path, USB_HBFW_FILE) != NULL) || (strstr(file_path, TMP_HBFW_FILE) != NULL) ) {
        ret = CheckType(0, &index);
        if ( ret == 2 ) {
            printf("Check Hybrid, list index = %d\n", index);
            GetHbFWData(file_path, list_path);
            // check list status
            if ( stat(list_path, &listst) == 0 ) {
                if ( listst.st_size < 16 ) {
                    printf("update end, remove fw file.\n");
                    sprintf(buf, "rm %s; sync; sync", file_path);
                    system(buf);
                    printf("remove sn list.\n");
                    sprintf(buf, "rm %s; sync; sync", list_path);
                    system(buf);
                }
            }
        }
    } else {
        printf("file_path = %s error.\n", file_path);
    }
*/
    // check sn list, empty then delete list
    // list not exist, delete fw file

    printf("##################################\n");

    ModbusDrvDeinit(gcomportfd);

    return 0;
}

int UpdDLFWStatus()
{
    printf("run UpdDLFWStatus()\n");

    //char buf[128] = {0};
    //sprintf(buf, "rm %s", UPDATE_FILE);
    //system(buf);

    //runProcess();

    return 0;
}
int updBATFWstatus()
{
    int loop = 0, ret = 0;
    char buf[512] = {0};
    char *index = NULL;
    FILE *file_fd = NULL;
    time_t current_time = 0;
    struct tm *st_time = NULL;

    // set time
    current_time = time(NULL);
    st_time = localtime(&current_time);

    OpenLog(g_SYSLOG_PATH, st_time);

    printf("run updBATFWstatus()\n");
    for (loop = 0; loop < gsncount; loop++) {
        sprintf(buf, "FWupdate updBATFWstatus() result : loop = %d, sn = %s, ver = 0x%04X, status = %d", loop, snlist[loop].SN, snlist[loop].ver, snlist[loop].status);
        printf(buf);
        printf("\n");
        SaveLog(buf, st_time);
    }

    printf("not ready, nothing to do!\n");

    // set updBATFWstatus xml file
    /*file_fd = fopen(CURL_FILE, "wb");
    if ( file_fd == NULL ) {
        printf("#### updBATFWstatus() open %s Fail ####\n", CURL_FILE);
        SaveLog("FWupdate updBATFWstatus() : open Fail", st_time);
        return 1;
    }
    memset(buf, 0, 512);
    fputs(SOAP_HEAD, file_fd);

    for (loop = 0; loop < gsncount; loop++) {
        //set sn
        //set ver
        //set status
    }

    fclose(file_fd);
*/
    CloseLog();

    return 0;
}

int CLEANSN(char *sn, char *file_path, char *list_path)
{
    char buf[1024] = {0};
    FILE *sn_fd = NULL;
    int snline = 0, ret = 0;

    sprintf(buf, "grep -n \"%s %s\" %s", sn, file_path, list_path);
    sn_fd = popen(buf, "r");
    fgets(buf, sizeof(buf), sn_fd);
    if ( strlen(buf) ) {
        printf("grep : %s\n", buf);
        sscanf(buf, "%d", &snline);
        sprintf(buf, "sed -i '%dd' %s", snline, list_path);
        printf("%s\n", buf);
        system(buf);
        ret = 0;
    } else
        ret = -1;

    if ( sn_fd )
        pclose(sn_fd);

    return ret;
}

int Updheartbeattime(time_t time)
{
    // Updheartbeattime result date length about = 396
    char buf[512] = {0};
    char timebuf[32] = {0}, swverbuf[32] = {0};
    char *index = NULL;
    FILE *fd = NULL, *swver_fd = NULL;
    struct tm *st_time;
    int ret = 0;
    struct stat st;

    printf("====================== Updheartbeattime start ======================\n");

    // get local time
    st_time = localtime(&time);
    sprintf(timebuf, "%4d-%02d-%02d %02d:%02d:%02d", 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
    printf("localtime : %s\n", timebuf);

    // get SW ver
    swver_fd = popen("/usr/home/dlg320.exe -v", "r");
    if ( swver_fd != NULL ) {
        fgets(swverbuf, 32, swver_fd);
        swverbuf[strlen(swverbuf)-1] = 0; // set \n to 0
        pclose(swver_fd);
    } else {
        strcpy(swverbuf, "Unknow");
    }

    // set Updheartbeattime xml file
    fd = fopen(CURL_FILE, "wb");
    if ( fd == NULL ) {
        printf("#### Updheartbeattime() open %s Fail ####\n", CURL_FILE);
        return 1;
    }
    memset(buf, 0, 512);
    fputs(SOAP_HEAD, fd);
    sprintf(buf, "\t\t<UpdheartbeattimeV2 xmlns=\"http://tempuri.org/\">\n");
    fputs(buf, fd);
    sprintf(buf, "\t\t\t<macaddress>%s</macaddress>\n", MAC);
    fputs(buf, fd);
    sprintf(buf, "\t\t\t<hearttime>%s</hearttime>\n", timebuf);
    fputs(buf, fd);
    sprintf(buf, "\t\t\t<Ver>%s</Ver>\n", swverbuf);
    fputs(buf, fd);
    sprintf(buf, "\t\t</UpdheartbeattimeV2>\n");
    fputs(buf, fd);
    fputs(SOAP_TAIL, fd);
    fclose(fd);

    // run curl soap command, save result to /tmp/Updheartbeattime
    sprintf(buf, "%s > /tmp/Updheartbeattime", g_CURL_CMD);
    system(buf);

    // check responds
    if ( stat("/tmp/Updheartbeattime", &st) == 0 )
        if ( st.st_size == 0 ) {
            printf("#### Updheartbeattime() /tmp/Updheartbeattime empty ####\n");
            SaveLog("FWUpdate Updheartbeattime() : /tmp/Updheartbeattime empty", st_time);
            return 2;
        }
    // read result
    fd = fopen("/tmp/Updheartbeattime", "rb");
    if ( fd == NULL ) {
        printf("#### Updheartbeattime() open /tmp/Updheartbeattime Fail ####\n");
        return 3;
    }
    memset(buf, 0, 512);
    fread(buf, 1, 512, fd);
    fclose(fd);
    //printf("/tmp/Updheartbeattime : \n%s\n", buf);

    // check result
    index = strstr(buf, "<UpdheartbeattimeV2Result>");
    if ( index == NULL ) {
        printf("#### Updheartbeattime() <UpdheartbeattimeV2Result> not found ####\n");
        SaveLog("FWUpdate Updheartbeattime() : <UpdheartbeattimeV2Result> not found", st_time);
        index = strstr(buf, "<UpdheartbeattimeV2Result />");
        if ( index == NULL )
            printf("#### Updheartbeattime() <UpdheartbeattimeV2Result /> not found ####\n");
        else {
            printf("<UpdheartbeattimeV2Result /> find, result data not exist!\n");
            SaveLog("FWUpdate Updheartbeattime() : result data not exist", st_time);
        }

        return 4;
    }
    sscanf(index, "<UpdheartbeattimeV2Result>%02d</UpdheartbeattimeV2Result>", &ret);
    printf("ret = %02d\n", ret);
    if ( ret == 0 ) {
        printf("Updheartbeattime update OK\n");
        SaveLog("FWUpdate Updheartbeattime() : update OK", st_time);
        printf("======================= Updheartbeattime end =======================\n");
        return 0;
    } else {
        printf("Updheartbeattime update Fail\n");
        SaveLog("FWUpdate Updheartbeattime() : update Fail", st_time);
        printf("======================= Updheartbeattime end =======================\n");
        return 5;
    }
}

int main(int argc, char* argv[])
{
    char opt;
    while( (opt = getopt(argc, argv, "vVtT")) != -1 )
    {
        switch (opt)
        {
            case 'v':
            case 'V':
                printf("%s\n", VERSION);
                //printf("TIMESTAMP=%s\n", __TIMESTAMP__);
                return 0;
            case 't':
            case 'T':
                printf("========Test mode start========\n");
                printf("=========Test mode end=========\n");
                return 0;
            case '?':
                return 1;
        }
    }

    time_t  previous_time;
    time_t  current_time;
    struct tm   *st_time = NULL;
    int counter, run_min, syslog_count, doflag = 0, restart = 0;
    struct stat st;
    //int doUpdDLFWStatus = 0;

    char strbuf[256] = {0};
    char milistbuf[256] = {0};
    char filebuf[256] = {0};
    char hybrid_sn[17] = {0};
    FILE *pfile = NULL;
    FILE *plist = NULL;
    char *pindex = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    ModbusDrvDeinit(3);
    ModbusDrvDeinit(4);
    ModbusDrvDeinit(5);
    ModbusDrvDeinit(6);
    printf("Do init\n");
    initenv((char *)"/usr/home/G320.ini");

    // when boot to run once first
    getMAC(MAC);
    getConfig();
    setCMD();
    setPath();

    printf("FW update start~\n");
    OpenLog(g_SYSLOG_PATH, st_time);
    SaveLog((char *)"FWupdate main() : start", st_time);

    QryDeviceFWUpdate();

    counter = 0;
    run_min = -1;
    syslog_count = 0;
    while (1) {
        // get local time
        current_time = time(NULL);
        st_time = localtime(&current_time);
        // check min (1/min)
        if ( run_min != st_time->tm_min ) {
            run_min = st_time->tm_min;

            // for debug
            SaveLog((char *)"FWupdate main() : alive", st_time);

            // save sys log (5 min)
            syslog_count++;
            if ( syslog_count == 10 ) {
                printf("savelog!\n");
                syslog_count = 0;
                CloseLog();
                system("sync");
                OpenLog(g_SYSLOG_PATH, st_time);
            }
        }

        // get config & set parameter
        getConfig();
        setCMD();
        setPath();
        // do QryDeviceFWUpdate
        if ( st_time->tm_min % update_SW_time == 0 ) {
            // if update file not exist
            //if ( stat(g_UPDATE_PATH, &st) ) { // not in /tmp
                //if ( stat(MIFW_FILE, &st) ) { // not in /tmp
                    previous_time = current_time;
                    printf("localtime : %4d/%02d/%02d %02d:%02d:%02d\n", 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
                    //printf("#### Debug : QryDeviceFWUpdate start time : %ld ####\n", previous_time);

                    // get update info
                    //if ( strlen(myupdate.FWURL) == 0 )
                    QryDeviceFWUpdate();

                    current_time = time(NULL);
                    counter = current_time - previous_time;
                    //printf("#### Debug : QryDeviceFWUpdate end time : %ld ####\n", current_time);
                    printf("#### Debug : QryDeviceFWUpdate span time : %d ####\n", counter);
                //}
                //else
                //    printf("%s exist!\n", MIFW_FILE);
            //}
            //else
                //printf("%s exist!\n", g_UPDATE_PATH);
        }
        // sleep
        printf("usleep() 30s\n");
        usleep(30000000);

        printf("######### check time #########\n");
        current_time = time(NULL);
        st_time = localtime(&current_time);
        printf("localtime : %4d/%02d/%02d %02d:%02d:%02d\n", 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
        printf("##############################\n");

        doflag = 0;
        // check fw update time
        if ( update_FW_start == update_FW_stop )
            doflag = 1;
        else if ( update_FW_start < update_FW_stop ) {
            if ( (update_FW_start <= st_time->tm_hour) && (st_time->tm_hour < update_FW_stop) )
                doflag = 1;
        } else { // update_FW_stop < update_FW_start, e.g. update_FW_start = 20, update_FW_stop = 8
            if ( (update_FW_start <= st_time->tm_hour) || (st_time->tm_hour < update_FW_stop) )
                doflag = 1;
        }

        if ( doflag ) {
            printf("doflag = 1, fw update check start\n");
            if ( gisusb ) {
                // check fw data from usb
                if ( stat(HYBRIDFW_FILE, &st) == 0 ) {
                    printf("detect fw data\n");
                    // unzip
                    sprintf(strbuf, "tar -zxvf %s -C /mnt/", HYBRIDFW_FILE);
                    system(strbuf);
                    usleep(1000000);
                    system("sync");
                    // remove fw data
                    if ( remove(HYBRIDFW_FILE) == 0 )
                        printf("Remove %s\n", HYBRIDFW_FILE);
                    else
                        perror("remove");
                    usleep(1000000);
                    system("sync");

                    // check milist
                    while (1) {
                        printf("check MIList\n");
                        pfile = popen("ls /tmp/MIList_*", "r");
                        if ( pfile != NULL ) {
                            memset(strbuf, 0x00, 256);
                            memset(milistbuf, 0x00, 256);
                            fgets(strbuf, 255, pfile);
                            if ( strlen(strbuf) > 0 ) {
                                strncpy(milistbuf, strbuf, strlen(strbuf)-1);
                                break;
                            } else {
                                printf("MIList not found\n");
                            }
                            pclose(pfile);
                        }
                        usleep(10000000);
                    }
                    printf("get milistbuf name [%s]\n", milistbuf);

                    // check fw file cpu1
                    printf("check cpu1\n");
                    pfile = popen("ls /mnt/cpu1_*", "r");
                    if ( pfile != NULL ) {
                        memset(strbuf, 0x00, 256);
                        memset(filebuf, 0x00, 256);
                        fgets(strbuf, 255, pfile);
                        if ( strlen(strbuf) > 0 ) {
                            strncpy(filebuf, strbuf, strlen(strbuf)-1);
                            printf("get filebuf name [%s]\n", filebuf);
                            pclose(pfile);

                            // set hybridfwlist
                            if ( stat(USB_HBFW_LIST, &st) != 0 )
                                plist = fopen(USB_HBFW_LIST, "w");
                            else
                                plist = fopen(USB_HBFW_LIST, "a");

                            // find sn in milist
                            pfile = fopen(milistbuf, "r");
                            if ( pfile != NULL ) {
                                while( fgets(strbuf, 255, pfile) != NULL ) {
                                    pindex = strstr(strbuf, "<sn>");
                                    if ( pindex ) {
                                        strncpy(hybrid_sn, pindex+4, 16);
                                        printf("find sn = [%s]\n", hybrid_sn);
                                        sprintf(strbuf, "%s %s\n", hybrid_sn, filebuf);
                                        fputs(strbuf, plist);
                                    }
                                }
                                fclose(pfile);
                            }
                            fclose(plist);

                        } else {
                            printf("cpu1 not found.\n");
                        }
                    }
                    // check fw file cpu2
                    printf("check cpu2\n");
                    pfile = popen("ls /mnt/cpu2_*", "r");
                    if ( pfile != NULL ) {
                        memset(strbuf, 0x00, 256);
                        memset(filebuf, 0x00, 256);
                        fgets(strbuf, 255, pfile);
                        if ( strlen(strbuf) > 0 ) {
                            strncpy(filebuf, strbuf, strlen(strbuf)-1);
                            printf("get filebuf name [%s]\n", filebuf);
                            pclose(pfile);

                            // set hybridfwlist
                            if ( stat(USB_HBFW_LIST, &st) != 0 )
                                plist = fopen(USB_HBFW_LIST, "w");
                            else
                                plist = fopen(USB_HBFW_LIST, "a");

                            // find sn in milist
                            pfile = fopen(milistbuf, "r");
                            if ( pfile != NULL ) {
                                while( fgets(strbuf, 255, pfile) != NULL ) {
                                    pindex = strstr(strbuf, "<sn>");
                                    if ( pindex ) {
                                        strncpy(hybrid_sn, pindex+4, 16);
                                        printf("find sn = [%s]\n", hybrid_sn);
                                        sprintf(strbuf, "%s %s\n", hybrid_sn, filebuf);
                                        fputs(strbuf, plist);
                                    }
                                }
                                fclose(pfile);
                            }
                            fclose(plist);

                        } else {
                            printf("cpu2 not found.\n");
                        }
                    }
                }

                // check mi unicast fw
                if ( stat(USB_MIFW_LIST, &st) == 0 ) {
                    DoUpdate(USB_MIFW_LIST);
                    restart = 1;
                }

                // check mi broadcast fw
                if ( stat(USB_MIFW_BLIST, &st) == 0 ) {
                    DoUpdate(USB_MIFW_BLIST);
                    restart = 1;
                }

                // check hybrid unicast fw
                if ( stat(USB_HBFW_LIST, &st) == 0 ) {
                    DoUpdate(USB_HBFW_LIST);
                    restart = 1;
                }

                // check plc unicast fw
                if ( stat(USB_PLC_LIST, &st) == 0 ) {
                    DoUpdate(USB_PLC_LIST);
                    restart = 1;
                }

                // check plc module broadcast fw
                if ( stat(USB_PLCM_LIST, &st) == 0 ) {
                    DoUpdate(USB_PLCM_LIST);
                    restart = 1;
                }

                // check battery fw
                if ( stat(USB_BATFW_LIST, &st) == 0 ) {
                    DoUpdate(USB_BATFW_LIST);
                    restart = 1;
                }
            } else {
                // check mi unicast fw
                if ( stat(TMP_MIFW_LIST, &st) == 0 ) {
                    DoUpdate(TMP_MIFW_LIST);
                    restart = 1;
                }

                // check mi broadcast fw
                if ( stat(TMP_MIFW_BLIST, &st) == 0 ) {
                    DoUpdate(TMP_MIFW_BLIST);
                    restart = 1;
                }

                // check hybrid unicast fw
                if ( stat(TMP_HBFW_LIST, &st) == 0 ) {
                    DoUpdate(TMP_HBFW_LIST);
                    restart = 1;
                }

                // check plc unicast fw
                if ( stat(TMP_PLC_LIST, &st) == 0 ) {
                    DoUpdate(TMP_PLC_LIST);
                    restart = 1;
                }

                // check plc module broadcast fw
                if ( stat(TMP_PLCM_LIST, &st) == 0 ) {
                    DoUpdate(TMP_PLCM_LIST);
                    restart = 1;
                }

                 // check battery fw
                if ( stat(TMP_BATFW_LIST, &st) == 0 ) {
                    DoUpdate(TMP_BATFW_LIST);
                    restart = 1;
                }
            }

            if ( restart == 1 ) {
                runProcess();
                restart = 0;
            }
        } else
            printf("doflag = 0, nothing to do\n");
    }
    return 0;
}
