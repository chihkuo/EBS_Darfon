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

#define USB_PATH    "/tmp/run/mountd/sda1"
#define SDCARD_PATH "/tmp/sdcard"

#define VERSION             "1.0.0"
#define TIMEOUT             "30"
#define CURL_FILE           "/tmp/FWupdate"
#define CURL_CMD            "curl -H 'Content-Type: text/xml;charset=UTF-8;SOAPAction:\"\"' http://60.251.36.232:80/SmsWebService1.asmx?WSDL -d @"CURL_FILE" --max-time "TIMEOUT
#define UPDATE_FILE         "/tmp/test.file"
#define UPDATE_LIST         "/tmp/fwulist"
#define SYSLOG_PATH         "/tmp/test/SYSLOG"
#define MAX_DATA_SIZE       144
#define MAX_HYBRID_SIZE     100

// extern part
extern int  MyModbusDrvInit(char *port, int baud, int data_bits, char parity, int stop_bits);
extern int  ModbusDrvDeinit(int fd);
extern void MakeReadDataCRC(unsigned char *,int );
extern void MClearRX();
extern void MStartTX(int fd);
extern unsigned char *GetRespond(int fd, int iSize, int delay);
extern void RemoveRegisterQuery(int fd, unsigned int byAddr);
extern void CleanRespond();
extern void initenv(char *init_name);

extern unsigned int     txsize;
extern unsigned char    waitAddr, waitFCode;
#define bool int
extern bool have_respond;
extern unsigned char    txbuffer[1544];//MODBUS_TX_BUFFER_SIZE

void getMAC(char *MAC);
void getConfig();
void setCMD();
void setPath();
int QryDLFWUpdate();
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
int GetFWData();
int GetHbFWData();
int stopProcess();
int runProcess();
int GetPort(char *file_path);
int GetMIList(char *file_path);
int GetSNList();
int CheckType(int index, int *ret);
int DoUpdate(char *file_path);
int UpdDLFWStatus();

char SOAP_HEAD[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\n\
\t<soap:Body>\n";

char SOAP_TAIL[] = "\t</soap:Body>\n</soap:Envelope>";

char MAC[18] = {0};
char UPDATE_SERVER[128] = {0};
int update_port = 0;
int update_SW_time = 0;
int delay_time = 0;
char g_CURL_CMD[256] = {0};
char g_SYSLOG_PATH[64] = {0};
char g_UPDATE_PATH[64] = {0};

int gbaud = 0;
int gdatabits = 0;
char gparity[8] = {0};
int gstopbits = 0;
int gcomportfd = 0;
int gprotocolver = 0;
int gV2id = -1;

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
} SN_LIST;
SN_LIST snlist[255] = {0};

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

    // get delay_time
    fd = popen("uci get dlsetting.@sms[0].delay_time", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 32, fd);
    pclose(fd);
    sscanf(buf, "%d", &delay_time);
    printf("Delay time (us.) = %d\n", delay_time);

    return;
}

void setCMD()
{
    if ( strlen(UPDATE_SERVER) )
        sprintf(g_CURL_CMD, "curl -H 'Content-Type: text/xml;charset=UTF-8;SOAPAction:\"\"' http://%s:%d/SmsWebService1.asmx?WSDL -d @%s --max-time %s", UPDATE_SERVER, update_port, CURL_FILE, TIMEOUT);
    else
        sprintf(g_CURL_CMD, "curl -H 'Content-Type: text/xml;charset=UTF-8;SOAPAction:\"\"' http://60.248.27.82:8080/SmsWebService1.asmx?WSDL -d @%s --max-time %s", CURL_FILE, TIMEOUT);

    return;
}

void setPath()
{
    struct stat st;

    if ( stat(USB_PATH, &st) == 0 ) { //linux storage detect
        strcpy(g_SYSLOG_PATH, USB_PATH);
        strcat(g_SYSLOG_PATH, "/SYSLOG");
        strcpy(g_UPDATE_PATH, USB_PATH);
        strcat(g_UPDATE_PATH, "/test.hex");
    }
    else if ( stat(SDCARD_PATH, &st) == 0 ) {
        strcpy(g_SYSLOG_PATH, SDCARD_PATH);
        strcat(g_SYSLOG_PATH, "/SYSLOG");
        strcpy(g_UPDATE_PATH, SDCARD_PATH);
        strcat(g_UPDATE_PATH, "/test.hex");
    }
    else {
        strcpy(g_SYSLOG_PATH, SYSLOG_PATH);
        strcpy(g_UPDATE_PATH, UPDATE_FILE);
    }

    printf("g_SYSLOG_PATH = %s\n", g_SYSLOG_PATH);
    printf("g_UPDATE_PATH = %s\n", g_UPDATE_PATH);

    return;
}

int QryDLFWUpdate()
{
    //char buf[128] = {0};

    printf("run QryDLFWUpdate()\n");

    // save result in /tmp/QryDLFWUpdate
    // read /tmp/QryDLFWUpdate
    // check result
    // download update file, set /tmp/fwupdate.xml, for debug use name /tmp/test.xml
    // set update list, set name /tmp/fwulist, save SN pre line

    //sprintf(buf, "touch %s", UPDATE_FILE);
    //system(buf);

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

        if ( err == 0 )
            lpdata = GetRespond(gcomportfd, 15, 100000); // 0.1s
        else if ( err == 1 )
            lpdata = GetRespond(gcomportfd, 15, 500000); // 0.5s
        else
            lpdata = GetRespond(gcomportfd, 15, 1000000); // 1s
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

        if ( err == 0 )
            lpdata = GetRespond(gcomportfd, 8, 100000); // 0.1s
        else if ( err == 1 )
            lpdata = GetRespond(gcomportfd, 8, 500000); // 0.5s
        else
            lpdata = GetRespond(gcomportfd, 8, 1000000); // 1s
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

    int err = 0, ret = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0;
    char buf[256] = {0};
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

        if ( err == 0 )
            lpdata = GetRespond(gcomportfd, 14, 100000); // 0.1s
        else if ( err == 1 )
            lpdata = GetRespond(gcomportfd, 14, 500000); // 0.5s
        else
            lpdata = GetRespond(gcomportfd, 14, 1000000); // 1s
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

        if ( err == 0 )
            lpdata = GetRespond(gcomportfd, 8, 100000); // 0.1s
        else if ( err == 1 )
            lpdata = GetRespond(gcomportfd, 8, 500000); // 0.5s
        else
            lpdata = GetRespond(gcomportfd, 8, 1000000); // 1s
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

        if ( err == 0 )
            lpdata = GetRespond(gcomportfd, 8, 100000); // 0.1s
        else if ( err == 1 )
            lpdata = GetRespond(gcomportfd, 8, 500000); // 0.5s
        else
            lpdata = GetRespond(gcomportfd, 8, 1000000); // 1s
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

        if ( err == 0 )
            lpdata = GetRespond(gcomportfd, 8, 100000); // 0.1s
        else if ( err == 1 )
            lpdata = GetRespond(gcomportfd, 8, 500000); // 0.5s
        else
            lpdata = GetRespond(gcomportfd, 8, 1000000); // 1s
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
    usleep(100000); // 0.1s

    SaveLog((char *)"FWupdate RunReboot() : Send", st_time);

    printf("\n#### RunReboot end ####\n");

    return 0;
}

int RunRebootSpecify(char *sn)
{
    printf("\n#### RunRebootSpecify start ####\n");

    int err = 0, ret = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0;
    char buf[256] = {0};
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

        if ( err == 0 )
            lpdata = GetRespond(gcomportfd, 14, 100000); // 0.1s
        else if ( err == 1 )
            lpdata = GetRespond(gcomportfd, 14, 500000); // 0.5s
        else
            lpdata = GetRespond(gcomportfd, 14, 1000000); // 1s
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

    int err = 0, ret = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0;
    char buf[256] = {0};
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

    while ( err < 3 ) {
        memcpy(txbuffer, cmd, 15);
        MStartTX(gcomportfd);
        usleep(1000000); // 1s

        if ( err == 0 )
            lpdata = GetRespond(gcomportfd, 14, 100000); // 0.1s
        else if ( err == 1 )
            lpdata = GetRespond(gcomportfd, 14, 500000); // 0.5s
        else
            lpdata = GetRespond(gcomportfd, 14, 1000000); // 1s
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

            if ( err == 0 )
                lpdata = GetRespond(gcomportfd, 8, 100000); // 0.1s
            else if ( err == 1 )
                lpdata = GetRespond(gcomportfd, 8, 500000); // 0.5s
            else
                lpdata = GetRespond(gcomportfd, 8, 1000000); // 1s

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
    unsigned char addrh = 0, addrl = 0, tmpstr[256] = {0}; // tmpstr for test, can delete
    unsigned char *lpdata = NULL;
    char buf[256] = {0};
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

        if ( err == 0 )
            lpdata = GetRespond(gcomportfd, 8, 100000); // 0.1s
        else if ( err == 1 )
            lpdata = GetRespond(gcomportfd, 8, 500000); // 0.5s
        else
            lpdata = GetRespond(gcomportfd, 8, 1000000); // 1s
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

            if ( err == 0 )
                lpdata = GetRespond(gcomportfd, 8, 100000); // 0.1s
            else if ( err == 1 )
                lpdata = GetRespond(gcomportfd, 8, 500000); // 0.5s
            else
                lpdata = GetRespond(gcomportfd, 8, 1000000); // 1s

            if ( lpdata ) {
                //lpdata = cmd; // for test
                if ( (lpdata[2] == addrh) && (lpdata[3] == addrl) && (lpdata[4] == 00) && (lpdata[5] == numofdata) ) {
                    cnt++;
                    printf("#### WriteHBData data count %d, index 0x%X, size %d OK ####\n", cnt, index, writesize);
                    sprintf(buf, "FWupdate WriteHBData() : write count %d, index 0x%X, size %d OK", cnt, index, writesize);
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
                    // re-address
                    address = (lpdata[2]<<8) + lpdata[3];
                    numofdata = lpdata[5];
                    writesize = numofdata*2;
                    index+=writesize;
                    printf("Re-address 0x%04X, numofdata 0x%02X, index %d\n", address, numofdata, index);

                    if ( retry == 0 ) {
                        retry = 1;
                        break;
                    } else {
                        SaveLog((char *)"FWupdate WriteHBData() : Response check retry error", st_time);
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

    printf("\n#### WriteHBData end ####\n");

    if ( index == datasize )
        return 0;
    else
        return 6;
}

int WriteDataV3(char *sn, unsigned char *fwdata, int datasize)
{

    printf("\n#### WriteDataV3 start ####\n");

    int i = 0, err = 0, index = 0, numofdata = MAX_DATA_SIZE/2, writesize = MAX_DATA_SIZE, address = 0, cnt = 0, end = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0;
    unsigned char addrh = 0, addrl = 0;
    unsigned char *lpdata = NULL;
    char buf[256] = {0};
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

            if ( err == 0 )
                lpdata = GetRespond(gcomportfd, 8, 100000); // 0.1s
            else if ( err == 1 )
                lpdata = GetRespond(gcomportfd, 8, 500000); // 0.5s
            else
                lpdata = GetRespond(gcomportfd, 8, 1000000); // 1s

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

    int err = 0, ret = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, tmp5 = 0, ver1 = 0, ver2 = 0;
    char buf[256] = {0};
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

        if ( err == 0 )
            lpdata = GetRespond(gcomportfd, 13, 100000); // 0.1s
        else if ( err == 1 )
            lpdata = GetRespond(gcomportfd, 13, 500000); // 0.5s
        else
            lpdata = GetRespond(gcomportfd, 13, 1000000); // 1s
        if ( lpdata ) {
            printf("#### ReadV3Ver OK ####\n");
            SaveLog((char *)"FWupdate ReadV3Ver() : OK", st_time);

            // check ver
            verh = lpdata[9];
            verl = lpdata[10];
            ver1 = (verh<<8) + verl;
            ver2 = fwver[0]*1000 + fwver[1]*100 + fwver[2]*10 + fwver[3];
            printf("ver1 = %d, ver2 = %d\n", ver1, ver2);
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

int GetFWData()
{
    unsigned char *ucbuffer = NULL;
    char read_buf[256] = {0}, strtmp[1024] = {0};
    char *cptr = NULL;
    FILE *pfile_fd = NULL;
    int major = 0, minor = 0, patchh = 0, patchl = 0, datasize = 0;
    unsigned char ucfwver[4] = {0};
    int index = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, start = 0, end = 0, i = 0, count = 0, dowritedata = 0, retver = -1, retdata = -1;

    time_t      current_time = 0;
    struct tm   *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    printf("######### run GetFWData() #########\n");

    pfile_fd = fopen("/tmp/test.file", "r");
    if ( pfile_fd == NULL ) {
        printf("#### Open /tmp/test.xml Fail ####\n");
        SaveLog((char *)"FWupdate GetFWData() : Open /tmp/test.xml Fail", st_time);
        return 1;
    }

    // get data size
    while ( fgets(read_buf, 256, pfile_fd) != NULL ) {
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
        return 2;
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
    printf("index = %d\n", index);

    // save fw version in log
    memset(strtmp, 0x00, 1024);
    sprintf(strtmp, "FWupdate GetFWData() : Get version = 0x%02X%02X%02X%02X", ucfwver[0], ucfwver[1], ucfwver[2], ucfwver[3]);
    SaveLog(strtmp, st_time);

    // save fw data in log
    for (i = 0; i < index; i++) {
        // part tital
        if ( i%MAX_DATA_SIZE == 0 ) {
            count++;
            sprintf(strtmp, "FWupdate GetFWData() : Data count %d", count);
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
    if ( gsncount ) {
        printf("==================== FW update start ====================\n");
        printf("gsncount = %d\n", gsncount);
        // fw update loop
        for (count = 0; count < gsncount; count++) {
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
                    printf("sn[%d] write fw data OK\n", count);
                    //if ( gprotocolver == 3 )
                    //    ReadV3Ver(snlist[count].SN, ucfwver);
                } else
                    printf("sn[%d] write fw data Fail\n", count);
            } else {
                printf("sn[%d] don't write data\n", count);
            }

            // some info
        }
        printf("===================== FW update end =====================\n");
    }
    free(ucbuffer);

    // save log immediately
    CloseLog();
    system("sync");
    OpenLog(g_SYSLOG_PATH, st_time);

    printf("######### GetFWData() end #########\n");

    return 0;
}

int GetHbFWData()
{
    unsigned char *ucbuffer = NULL;
    int i = 0, byte_count = 0, addr = 0, recoed_type = 0, tmp = 0, next_addr = 0, size = 0, index = 0, retval = -1, count = 0, index_tmp = 0;
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

    pfile_fd = fopen("/tmp/test.file", "r");
    if ( pfile_fd == NULL ) {
        printf("#### Open /tmp/test.hex Fail ####\n");
        SaveLog((char *)"FWupdate GetHbFWData() : Open /tmp/test.hex Fail", st_time);
        return 1;
    }

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
            printf("[:] not found\n");
            getchar();
        }
    }
    printf("size = %d\n", size);
    // jump to beginning of file
    fseek(pfile_fd, 0, SEEK_SET);

    if ( size == 0 ) {
        fclose(pfile_fd);
        printf("size = 0\n");
        SaveLog((char *)"FWupdate GetHbFWData() : size = 0", st_time);
        return 2;
    }

    // create fw data buffer
    ucbuffer = calloc(size, sizeof(unsigned char));
    if ( ucbuffer == NULL ) {
        printf("#### calloc %d Fail ####\n", size);
        SaveLog((char *)"FWupdate GetHbFWData() : calloc Fail", st_time);
        fclose(pfile_fd);
        return 3;
    }
    printf("calloc size %d OK\n", size);

    // set fw data
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
        return 4;
    }

    // set check sum
    //printf("count check sum start:\n");
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

    // register part
    // remove register
    if ( gcomportfd > 0 ) {
        RemoveRegisterQuery(gcomportfd, 0);
        CleanRespond();
        usleep(500000);
        RemoveRegisterQuery(gcomportfd, 0);
        CleanRespond();
        usleep(500000);
        RemoveRegisterQuery(gcomportfd, 0);
        CleanRespond();
        usleep(500000);
    }
    // register
    retval = RunRegister(snlist[0].SN);
    if ( retval ) {
        // fail
        printf("RunRegister fail retval = %d\n", retval);
        free(ucbuffer);
        return 5;
    }
    // register OK, get slave id gV2id
    printf("RunRegister return OK, get gV2id = %d\n", gV2id);

    // send file header & check sum & byte count
    retval = WriteHBData(gV2id, ucbuffer, index);

    free(ucbuffer);

    printf("######### GetHbFWData() end #########\n");

    if ( !retval ) // ok
        return 0;
    else    // fail
        return 6;
}

int stopProcess()
{
    printf("run stopProcess()\n");
    system("/etc/init.d/run_DL.sh stop");
    system("sync");

    return 0;
}

int runProcess()
{
    printf("run runProcess()\n");
    system("/etc/init.d/run_DL.sh start");
    system("sync");

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
    }
    printf("===========================================\n");

    sprintf(buf, "FWupdate GetMIList() : Get mi list count %d", gmicount);
    SaveLog(buf, st_time);

    printf("######### GetMIList() end #########\n");

    return 0;
}

int GetSNList()
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
    for (i = 0; i < 255; i++)
        memset(snlist[i].SN, 0x00, 17);

    printf("######### run GetSNList() #########\n");

    pfile_fd = fopen(UPDATE_LIST, "rb");
    if ( pfile_fd == NULL ) {
        printf("#### Open %s Fail ####\n", UPDATE_LIST);
        sprintf(buf, "FWupdate GetSNList() : Open %s Fail", UPDATE_LIST);
        SaveLog(buf, st_time);
        return -1;
    }

    // get list
    while ( fgets(buf, 512, pfile_fd) != NULL ) {
        //printf("buf = %s\n", buf);
        strncpy(snlist[gsncount].SN, buf, 16);
        gsncount++;
    }
    fclose(pfile_fd);

    // debug print
    printf("===========================================\n");
    printf("sn list count = %d\n", gsncount);
    for (i = 0; i < gsncount; i++)
        printf("set sn list %d : sn = %s\n", i, snlist[i].SN);
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

int DoUpdate(char *file_path)
{
    FILE *pfile_fd = NULL;
    char buf[512] = {0}, strtmp[512] = {0};
    char FILENAME[64] = {0};
    int comport = 0, ret = 0, index = -1;

    time_t      current_time;
    struct tm   *st_time = NULL;
    current_time = time(NULL);
    st_time = localtime(&current_time);
    SaveLog((char *)"FWupdate DoUpdate() : start", st_time);

    printf("######### run DoUpdate() #########\n");
    printf("file_path = %s\n", file_path);

    // stop process
    stopProcess();

    // get milist name
    printf("Get MIList\n");
    system("cd /tmp; ls MIList_* > /tmp/MIList");

    pfile_fd = fopen("/tmp/MIList", "rb");
    if ( pfile_fd == NULL ) {
        printf("#### Open /tmp/MIList Fail ####\n");
        SaveLog((char *)"FWupdate DoUpdate() : Open /tmp/MIList Fail", st_time);
        return 1;
    }
    // get file name
    memset(buf, 0, 512);
    fgets(buf, 64, pfile_fd);
    fclose(pfile_fd);
    if ( strlen(buf) )
        buf[strlen(buf)-1] = 0; // set '\n' to 0
    else {
        printf("Empty file! Plese check MIList exist!\n");
        SaveLog((char *)"FWupdate DoUpdate() : MIList not found", st_time);
        return 2;
    }
    sprintf(FILENAME, "/tmp/%s", buf);
    printf("FILENAME = %s\n", FILENAME);

    // get Darfon use com port
    comport = GetPort(FILENAME);
    // get mi list
    GetMIList(FILENAME);
    // get sn list
    GetSNList();

    // open com port
    if ( gcomportfd == 0 ) {
        // get com port setting
        GetComPortSetting(comport);
        // open com port, get gcomportfd
        OpenComPort(comport);
        sprintf(strtmp, "FWupdate DoUpdate() : Get comport fd %d", gcomportfd);
        SaveLog(strtmp, st_time);
    }

    // if only 1 device, check type (Hybrid?)
    if ( gsncount == 1 )
        ret = CheckType(0, &index);
    // check run Hybrid or MI
    if ( ret == 2 ) {
        // Hybrid
        printf("Check Hybrid, list index = %d\n", index);
        GetHbFWData();
    } else if ( ret == 0 ) {
        // MI
        // check version
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

        if ( gprotocolver )
            GetFWData();
    }

    printf("##################################\n");

    return 0;
}

int UpdDLFWStatus()
{
    printf("run UpdDLFWStatus()\n");

    char buf[128] = {0};
    sprintf(buf, "rm %s", UPDATE_FILE);
    system(buf);

    //runProcess();

    return 0;
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
    int counter, run_min, syslog_count;
    struct stat st;
    int doUpdDLFWStatus = 0;
    char FWURL[128] = {0};

    current_time = time(NULL);
    st_time = localtime(&current_time);

    ModbusDrvDeinit(3);
    ModbusDrvDeinit(4);
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

    QryDLFWUpdate();

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
            if ( syslog_count == 5 ) {
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
        // do QryDLFWUpdate
        if ( st_time->tm_min % update_SW_time == 0 ) {
            // if update file not exist
            if ( stat(g_UPDATE_PATH, &st) ) { // not in storage
                if ( stat(UPDATE_FILE, &st) ) { // not in /tmp
                    previous_time = current_time;
                    printf("localtime : %4d/%02d/%02d %02d:%02d:%02d\n", 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
                    //printf("#### Debug : QryDLSWUpdate start time : %ld ####\n", previous_time);

                    // get update info
                    if ( strlen(FWURL) == 0 )
                        QryDLFWUpdate();

                    current_time = time(NULL);
                    counter = current_time - previous_time;
                    //printf("#### Debug : QryDLSWUpdate end time : %ld ####\n", current_time);
                    printf("#### Debug : QryDLFWUpdate span time : %d ####\n", counter);
                }
                else
                    printf("%s exist!\n", UPDATE_FILE);
            }
            else
                printf("%s exist!\n", g_UPDATE_PATH);
        }
        // sleep
        printf("usleep() 60s\n");
        usleep(60000000);

        printf("######### check time #########\n");
        current_time = time(NULL);
        st_time = localtime(&current_time);
        printf("localtime : %4d/%02d/%02d %02d:%02d:%02d\n", 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
        printf("##############################\n");

        // check storage
        if ( stat(g_UPDATE_PATH, &st) == 0 )
            //if ( CheckTime(st_time) )
                if ( !DoUpdate(g_UPDATE_PATH) )
                    doUpdDLFWStatus = 1;
        // check /tmp
        if ( strstr(g_UPDATE_PATH, UPDATE_FILE) == NULL ) {
            if ( stat(UPDATE_FILE, &st) == 0 )
                //if ( CheckTime(st_time) )
                    if ( !DoUpdate(UPDATE_FILE) )
                        doUpdDLFWStatus = 1;
        }
        // update report
        if ( doUpdDLFWStatus )
            if ( !UpdDLFWStatus() ) {
                doUpdDLFWStatus = 0;
                memset(FWURL, 0, 128);
            }
    }
    return 0;
}
