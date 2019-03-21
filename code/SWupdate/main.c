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

//#define USB_PATH    "/tmp/usb"
//#define USB_PATH    "/tmp/run/mountd/sda1"
#define USB_PATH    "/mnt"
#define USB_DEV     "/dev/sda1"
#define SDCARD_PATH "/tmp/sdcard"

#define VERSION             "2.2.2"
#define DLMODEL             "SBC700"
#define TIMEOUT             "30"
#define CURL_FILE           "/tmp/SWupdate"
#define CURL_CMD            "curl -H 'Content-Type: text/xml;charset=UTF-8;SOAPAction:\"\"' http://60.251.36.232:80/SmsWebService1.asmx?WSDL -d @"CURL_FILE" --max-time "TIMEOUT
#define UPDATE_FILE         "/tmp/update.tar"
#define UPDATE_DIR          "/tmp/update"
#define SYSLOG_PATH         "/tmp/test/SYSLOG"

char SOAP_HEAD[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\n\
\t<soap:Body>\n";

char SOAP_TAIL[] = "\t</soap:Body>\n</soap:Envelope>";

char MAC[18] = {0};
char SMS_SERVER[128] = {0};
int sms_port = 0;
char UPDATE_SERVER[128] = {0};
int update_port = 0;
int update_SW_time = 0;
int reboot_time = 0;
char g_CURL_CMD[256] = {0};
char g_SYSLOG_PATH[64] = {0};
char g_UPDATE_PATH[64] = {0};

// update parameter
typedef struct swupdate {
    int major;
    int minor;
    int patch;
    char DLModel[16];
    int hour;
    int minutes;
    char SWURL[128];
}SWUPDATE;
SWUPDATE myupdate = {0};


void getMAC(char *MAC)
{
    /*struct ifaddrs *ifaddr = NULL;
    struct ifaddrs *ifa = NULL;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
    }
    else
    {
        for ( ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if ( (ifa->ifa_addr) && (ifa->ifa_addr->sa_family == AF_PACKET) )
            {
                struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
                if (s->sll_addr[0]|s->sll_addr[1]|s->sll_addr[2]|s->sll_addr[3]|s->sll_addr[4]|s->sll_addr[5]) {
                    sprintf(MAC, "%02X:%02X:%02X:%02X:%02X:%02X",
                    s->sll_addr[0],s->sll_addr[1],s->sll_addr[2],s->sll_addr[3],s->sll_addr[4],s->sll_addr[5]);
                    MAC[17] = 0;
                    break;
                }
            }
        }
        freeifaddrs(ifaddr);
    }
    printf("getMAC = %s\n", MAC);*/

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

    // get sms server
/*    fd = popen("uci get dlsetting.@sms[0].sms_server", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(SMS_SERVER, 128, fd);
    pclose(fd);
    if ( strlen(SMS_SERVER) )
        SMS_SERVER[strlen(SMS_SERVER)-1] = 0; // clean \n
    printf("Sms Server = %s\n", SMS_SERVER);
*/
    // get sms server port
/*    fd = popen("uci get dlsetting.@sms[0].sms_port", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 32, fd);
    pclose(fd);
    sscanf(buf, "%d", &sms_port);
    printf("Sms Port = %d\n", sms_port);
*/
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

    // get reboot time
    fd = popen("uci get dlsetting.@sms[0].reboot_time", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 32, fd);
    pclose(fd);
    sscanf(buf, "%d", &reboot_time);
    printf("Reboot time = %d\n", reboot_time);

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

    if ( stat(USB_DEV, &st) == 0 ) { //linux storage detect
        strcpy(g_SYSLOG_PATH, USB_PATH);
        strcat(g_SYSLOG_PATH, "/SYSLOG");
        strcpy(g_UPDATE_PATH, USB_PATH);
        strcat(g_UPDATE_PATH, "/update.tar");
    }
    else if ( stat(SDCARD_PATH, &st) == 0 ) {
        strcpy(g_SYSLOG_PATH, SDCARD_PATH);
        strcat(g_SYSLOG_PATH, "/SYSLOG");
        strcpy(g_UPDATE_PATH, SDCARD_PATH);
        strcat(g_UPDATE_PATH, "/update.tar");
    }
    else {
        strcpy(g_SYSLOG_PATH, SYSLOG_PATH);
        strcpy(g_UPDATE_PATH, UPDATE_FILE);
    }

    return;
}

void init()
{
    getMAC(MAC);
    return;
}

int QryDLSWUpdate()
{
    char buf[512] = {0};
    char vertmp[32] = {0};
    FILE *fd = NULL;
    int size = 0, inlen = 0, outlen = 0;
    char *data = NULL, *start_index = NULL, *end_index = NULL, *start_time = NULL, *end_time = NULL;
    unsigned char *decode_data = NULL;
    time_t current_time;
    struct tm *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    printf("======================== QryDLSWUpdate start ========================\n");
    // set QryDLSWUpdate xml file
    fd = fopen(CURL_FILE, "wb");
    if ( fd == NULL ) {
        printf("#### QryDLSWUpdate() open %s Fail ####\n", CURL_FILE);
        return 1;
    }
    fputs(SOAP_HEAD, fd);
    sprintf(buf, "\t\t<QryDLSWUpdate xmlns=\"http://tempuri.org/\">\n");
    fputs(buf, fd);
    sprintf(buf, "\t\t\t<macaddress>%s</macaddress>\n", MAC);
    fputs(buf, fd);
    sprintf(buf, "\t\t</QryDLSWUpdate>\n");
    fputs(buf, fd);
    fputs(SOAP_TAIL, fd);
    fclose(fd);

    // run curl soap command, save result to /tmp/QryDLSWUpdate
    sprintf(buf, "%s > /tmp/QryDLSWUpdate", g_CURL_CMD);
    system(buf);

    // check size
    fd = fopen("/tmp/QryDLSWUpdate", "rb");
    if ( fd == NULL ) {
        printf("#### QryDLSWUpdate() open /tmp/QryDLSWUpdate Fail ####\n");
        SaveLog("SWupdate QryDLSWUpdate() : open /tmp/QryDLSWUpdate Fail", st_time);
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
    //printf("data : \n%s\n", data);

    // find start address & length
    start_index = strstr(data, "<QryDLSWUpdateResult>");
    if ( start_index == NULL ) {
        printf("#### QryDLSWUpdate() <QryDLSWUpdateResult> not found ####\n");
        SaveLog("SWupdate QryDLSWUpdate() : <QryDLSWUpdateResult> not found", st_time);
        if ( data )
            free(data);
        return 3;
    }
    start_index += 21; // <QryDLSWUpdateResult> length
    end_index = strstr(data, "</QryDLSWUpdateResult>");
    if ( end_index == NULL ) {
        printf("#### QryDLSWUpdate() </QryDLSWUpdateResult> not found ####\n");
        SaveLog("SWupdate QryDLSWUpdate() : </QryDLSWUpdateResult> not found", st_time);
        if ( data )
            free(data);
        return 4;
    }
    inlen = end_index - start_index;
    printf("inlen = %d\n", inlen);

    decode_data = base64_decode(start_index, inlen, &outlen);
    if ( decode_data == NULL ) {
        printf("#### QryDLSWUpdate() decode_data = NULL ####\n");
        SaveLog("SWupdate QryDLSWUpdate() : decode_data = NULL", st_time);
        if ( data )
            free(data);
        return 5;
    }
    //printf("decode_data len = %d, : \n%s\n", outlen, decode_data);
    if ( data )
        free(data);

    // parser update parameter, find last item
    start_index = (char*)decode_data;
    end_index = (char*)decode_data;
    while ( start_index != NULL && end_index != NULL ) {
        // set Ver
        start_index = strstr(start_index, "<Ver>");
        end_index = strstr(end_index, "</Ver>");
        if ( start_index == NULL || end_index == NULL )
            break;
        // clean parameter
        myupdate.major = -1;
        myupdate.minor = -1;
        myupdate.patch = -1;
        memset(myupdate.DLModel, 0, sizeof(myupdate.DLModel));
        myupdate.hour = -1;
        myupdate.minutes = -1;
        memset(myupdate.SWURL, 0, sizeof(myupdate.SWURL));

        strncpy(vertmp, start_index+5, end_index-(start_index+5));
        sscanf(vertmp, "%d.%d.%d", &myupdate.major, &myupdate.minor, &myupdate.patch);

        // set DLModel
        start_index = strstr(start_index, "<DLModel>");
        end_index = strstr(end_index, "</DLModel>");
        if ( start_index == NULL || end_index == NULL )
            break;
        strncpy(myupdate.DLModel, start_index+9, end_index-(start_index+9));

        // set time
        start_time = strstr(start_index, "<PlanTime>");
        end_time = strstr(end_index, "</PlanTime>");
        if ( start_time+10 < end_time ) // if equal, empty parameter
            sscanf(start_time+10, "%02d:%02d", &myupdate.hour, &myupdate.minutes);

        // set SWURL
        start_index = strstr(start_index, "<SWURL>");
        end_index = strstr(end_index, "</SWURL>");
        if ( start_index == NULL || end_index == NULL )
            break;
        strncpy(myupdate.SWURL, start_index+7, end_index-(start_index+7));
    }
    if ( decode_data )
        free(decode_data);

    printf("Get Ver = %d.%d.%d, DLModelp = %s, PlanTime = %02d:%02d, SWURL = %s\n",
           myupdate.major, myupdate.minor, myupdate.patch, myupdate.DLModel, myupdate.hour, myupdate.minutes, myupdate.SWURL);

    // check dl model, if not match then exit, no download
    if ( strcmp(myupdate.DLModel, DLMODEL) ) {
        sprintf(buf, "SWupdate QryDLSWUpdate() : DLModel %s not match this board %s", myupdate.DLModel, DLMODEL);
        printf("%s\n", buf);
        SaveLog(buf, st_time);
        return 6;
    }

    if ( strlen(myupdate.SWURL) ) {
        sprintf(buf, "curl -o %s %s", g_UPDATE_PATH, myupdate.SWURL);
        if ( !system(buf) ) {
            printf("Download OK\n");
            sprintf(buf, "SWupdate QryDLSWUpdate() : Download %s to %s OK", myupdate.SWURL, g_UPDATE_PATH);
            SaveLog(buf, st_time);
            printf("======================= QryDLSWUpdate end =======================\n");
            return 0;
        }
        else {
            printf("Download Fail\n");
            sprintf(buf, "SWupdate QryDLSWUpdate() : Download %s to %s Fail", myupdate.SWURL, g_UPDATE_PATH);
            SaveLog(buf, st_time);
            if ( strstr(g_UPDATE_PATH, USB_PATH) ) {
                sprintf(buf, "curl -o %s %s", UPDATE_DIR, myupdate.SWURL);
                if ( !system(buf) ) {
                    printf("Download OK\n");
                    sprintf(buf, "SWupdate QryDLSWUpdate() : Download %s to %s OK", myupdate.SWURL, UPDATE_DIR);
                    SaveLog(buf, st_time);
                    printf("======================= QryDLSWUpdate end =======================\n");
                    return 0;
                }
                else {
                    printf("Download Fail\n");
                    sprintf(buf, "SWupdate QryDLSWUpdate() : Download %s to %s Fail", myupdate.SWURL, UPDATE_DIR);
                    SaveLog(buf, st_time);
                }
            }
            printf("======================= QryDLSWUpdate end =======================\n");
            return 7;
        }
    }

    printf("SWURL empty!\n");
    SaveLog("SWupdate QryDLSWUpdate() : SWURL empty", st_time);
    printf("======================= QryDLSWUpdate end =======================\n");
    return 0;
}

int CheckTime(struct tm *st_time)
{
    printf("run CheckTime\n");

    if ( (myupdate.hour == -1) && (myupdate.minutes == -1) ) {
        printf("PlanTime empty, do update immediately!\n");
        return 3;
    }

    if ( st_time->tm_hour > myupdate.hour ) {
        printf("Bigger then PlanTime, do update!\n");
        return 2;
    } else {
        if ( (st_time->tm_hour == myupdate.hour) && (st_time->tm_min >= myupdate.minutes) ) {
            printf("PlanTime match, do update!\n");
            return 1;
        }
    }

    return 0;
}

int DoUpdate(char *file_path)
{
    char buf[256] = {0};
    int ret = 0;
    int major = 0, minor = 0, patch = 0;
    int dl_ori_ver = 0, dp_ori_ver = 0, dl_upd_ver = 0, dp_upd_ver = 0, upd_ver = 0;
    FILE *fd = NULL;
    struct stat st;
    time_t current_time;
    struct tm *st_time = NULL;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    printf("run DoUpdate\n");

    // unzip
    sprintf(buf, "tar -xf %s -C /tmp", file_path);
    ret = system(buf);
    printf("ret = %d\n", ret);
    system("sync");
    // clean update file
    printf("remove %s\n", file_path);
    sprintf(buf, "rm %s; sync;", file_path);
    system(buf);
    // check result, success = 0, fail > 0
    if ( ret ) {
        printf("command tar fail!\n");
        SaveLog("SWupdate DoUpdate() : tar fail", st_time);
        return 1;
    }

    // paser SW & HW VER if file from usb
    if ( !strlen(myupdate.DLModel) ) { // file not from download, no parameter
        if ( stat(USB_PATH, &st) == 0 ) { // file in usb
            sprintf(buf, "%s/update.txt", USB_PATH);
            fd = fopen(buf, "r");
            if ( fd == NULL ) {
                printf("fopen fail!\n");
                return 2;
            }
            memset(buf, 0, 256);
            fgets(buf, 256, fd);
            sscanf(buf, "%d.%d.%d", &myupdate.major, &myupdate.minor, &myupdate.patch);
            fgets(myupdate.DLModel, 16, fd);
            pclose(fd);
            myupdate.DLModel[strlen(myupdate.DLModel)-1] = 0; // remove \n
            printf("myupdate.DLModel = [%s] \n", myupdate.DLModel);
        }
    }

    // check dl model, if not match then exit
    if ( strcmp(myupdate.DLModel, DLMODEL) ) {
        sprintf(buf, "SWupdate QryDLSWUpdate() : DLModel %s not match this board %s", myupdate.DLModel, DLMODEL);
        printf("%s\n", buf);
        SaveLog(buf, st_time);
        return 3;
    }

    // check unzip file
    // get update dlg320.exe version
    sprintf(buf, "%s/dlg320.exe -v", UPDATE_DIR);
    fd = popen(buf, "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return 4;
    }
    memset(buf, 0, 256);
    fgets(buf, 256, fd);
    pclose(fd);
    sscanf(buf, "%d.%d.%d", &major, &minor, &patch);
    dl_upd_ver = major*10000 + minor*100 + patch;
    printf("dl_upd_ver = %d\n", dl_upd_ver);

    // get update DataProgram.exe version
    sprintf(buf, "%s/DataProgram.exe -v", UPDATE_DIR);
    fd = popen(buf, "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return 5;
    }
    memset(buf, 0, 256);
    fgets(buf, 256, fd);
    pclose(fd);
    sscanf(buf, "%d.%d.%d", &major, &minor, &patch);
    dp_upd_ver = major*10000 + minor*100 + patch;
    printf("dp_upd_ver = %d\n", dp_upd_ver);

    // check version & file match
    upd_ver = myupdate.major*10000 + myupdate.minor*100 + myupdate.patch;
    printf("upd_ver = %d\n", upd_ver);
    if ( (upd_ver != dl_upd_ver) || (upd_ver != dp_upd_ver) ) {
        printf("update file version not match!\n");
        sprintf(buf, "SWupdate DoUpdate() : update file version not match %d.%d.%d", myupdate.major, myupdate.minor, myupdate.patch);
        SaveLog(buf, st_time);
        return 6;
    }

    // get original dlg320.exe version
    fd = popen("/usr/home/dlg320.exe -v", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return 7;
    }
    memset(buf, 0, 256);
    fgets(buf, 256, fd);
    pclose(fd);
    sscanf(buf, "%d.%d.%d", &major, &minor, &patch);
    dl_ori_ver = major*10000 + minor*100 + patch;
    printf("dl_ori_ver = %d\n", dl_ori_ver);

    // get original DataProgram.exe version
    fd = popen("/usr/home/DataProgram.exe -v", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return 8;
    }
    memset(buf, 0, 256);
    fgets(buf, 256, fd);
    pclose(fd);
    sscanf(buf, "%d.%d.%d", &major, &minor, &patch);
    dp_ori_ver = major*10000 + minor*100 + patch;
    printf("dp_ori_ver = %d\n", dp_ori_ver);

    // compare original file & update file version
    if ( (dl_upd_ver == dl_ori_ver) && (dp_upd_ver == dp_ori_ver) ) {
        printf("update file version = original file version\n");
        SaveLog("SWupdate DoUpdate() : update file version = original file version", st_time);
        return 9;
    }

    // kill process
    system("killall -9 dlg320.exe");
    system("killall -9 DataProgram.exe");
    system("sync");
    // copy update file
    sprintf(buf, "cp %s/dlg320.exe /usr/home", UPDATE_DIR);
    ret = system(buf);
    if ( ret ) {
        printf("copy dlg320.exe fail!\n");
        SaveLog("SWupdate DoUpdate() : copy dlg320.exe fail", st_time);
        sprintf(buf, "rm -rf %s; sync", UPDATE_DIR);
        system(buf);
        return 10;
    }
    sprintf(buf, "cp %s/DataProgram.exe /usr/home", UPDATE_DIR);
    ret = system(buf);
    if ( ret ) {
        printf("copy DataProgram.exe fail!\n");
        SaveLog("SWupdate DoUpdate() : copy DataProgram.exe fail", st_time);
        sprintf(buf, "rm -rf %s; sync", UPDATE_DIR);
        system(buf);
        return 11;
    }
    // run update.sh if exist in usb
    if ( 0/*strstr(g_UPDATE_PATH, USB_PATH)*/ ) {
        sprintf(buf, "%s/update/update.sh", USB_PATH);
        if ( stat(buf, &st) == 0 ) {
            sprintf(buf, "chmod 755 %s/update/update.sh", USB_PATH);
            system(buf);
            sprintf(buf, "%s/update/update.sh", USB_PATH);
            system(buf);
            printf("run %s/update/update.sh\n", USB_PATH);
            sprintf(buf, "SWupdate DoUpdate() : run %s/update/update.sh", USB_PATH);
            SaveLog(buf, st_time);
        }
    } else { // in tmp
        sprintf(buf, "%s/update.sh", UPDATE_DIR);
        printf("cmd = %s\n", buf);
        if ( stat(buf, &st) == 0 ) {
            sprintf(buf, "chmod 755 %s/update.sh", UPDATE_DIR);
            system(buf);
            sprintf(buf, "%s/update.sh", UPDATE_DIR);
            system(buf);
            printf("run %s/update.sh\n", UPDATE_DIR);
            sprintf(buf, "SWupdate DoUpdate() : run %s/update.sh", UPDATE_DIR);
            SaveLog(buf, st_time);
        }
    }
    // clean file
    sprintf(buf, "rm -rf %s; sync", UPDATE_DIR);
    system(buf);

    // run new file
    system("chmod 755 /usr/home/dlg320.exe");
    system("chmod 755 /usr/home/DataProgram.exe");
    system("/usr/home/dlg320.exe &");
    system("/usr/home/DataProgram.exe &");
    system("sync");
    sprintf(buf, "SWupdate DoUpdate() : Update V%d.%d.%d OK!", myupdate.major, myupdate.minor, myupdate.patch);
    SaveLog(buf, st_time);

    return 0;
}

int UpdDLSWStatus()
{
    char buf[512] = {0};
    char *index = NULL;
    FILE *fd = NULL;
    time_t current_time = 0;
    struct tm *st_time = NULL;
    struct stat myst;
    int ret = 0;

    // set time
    current_time = time(NULL);
    st_time = localtime(&current_time);

    printf("======================== UpdDLSWStatus start ========================\n");
    // set UpdDLSWStatus xml file
    fd = fopen(CURL_FILE, "wb");
    if ( fd == NULL ) {
        printf("#### UpdDLSWStatus() open %s Fail ####\n", CURL_FILE);
        SaveLog("SWupdate UpdDLSWStatus() : open Fail", st_time);
        return 1;
    }
    fputs(SOAP_HEAD, fd);
    sprintf(buf, "\t\t<UpdDLSWStatus xmlns=\"http://tempuri.org/\">\n");
    fputs(buf, fd);
    sprintf(buf, "\t\t\t<macaddress>%s</macaddress>\n", MAC);
    fputs(buf, fd);
    sprintf(buf, "\t\t\t<ver>%d.%d.%d</ver>\n", myupdate.major, myupdate.minor, myupdate.patch);
    fputs(buf, fd);
    sprintf(buf, "\t\t\t<DLModel>%s</DLModel>\n", myupdate.DLModel);
    fputs(buf, fd);
    sprintf(buf, "\t\t\t<UpdatedTime>%04d-%02d-%02d %02d:%02d:%02d</UpdatedTime>\n", 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday,
                 st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
    fputs(buf, fd);
    sprintf(buf, "\t\t</UpdDLSWStatus>\n");
    fputs(buf, fd);
    fputs(SOAP_TAIL, fd);
    fclose(fd);

    while (1) {
        // run curl soap command, save result to /tmp/UpdDLSWStatus
        sprintf(buf, "%s > /tmp/UpdDLSWStatus", g_CURL_CMD);
        system(buf);

        // read result
        fd = fopen("/tmp/UpdDLSWStatus", "rb");
        if ( fd == NULL ) {
            printf("#### UpdDLSWStatus() open /tmp/UpdDLSWStatus Fail ####\n");
            SaveLog("SWupdate UpdDLSWStatus() : open /tmp/UpdDLSWStatus Fail", st_time);
            usleep(30000000); // sleep 30s
            return 2;
        }
        memset(buf, 0, 512);
        fread(buf, 1, 512, fd);
        fclose(fd);
        //printf("/tmp/UpdDLSWStatus : \n%s\n", buf);

        // check result
        index = strstr(buf, "<UpdDLSWStatusResult>");
        if ( index == NULL ) {
            printf("#### UpdDLSWStatus() <UpdDLSWStatusResult> not found ####\n");
            SaveLog("SWupdate UpdDLSWStatus() : <UpdDLSWStatusResult> not found", st_time);
            index = strstr(buf, "<UpdDLSWStatusResult />");
            if ( index == NULL )
                printf("#### UpdDLSWStatus() <UpdDLSWStatusResult /> not found ####\n");
            else
                printf("<UpdDLSWStatusResult /> find, no result data!\n");
            usleep(30000000); // sleep 30s
            continue;
        }
        sscanf(index, "<UpdDLSWStatusResult>%02d</UpdDLSWStatusResult>", &ret);
        printf("ret = %02d\n", ret);
        if ( ret == 0 ) {
            printf("UpdDLSWStatus() OK\n");
            SaveLog("SWupdate UpdDLSWStatus() : OK", st_time);
            printf("======================= UpdDLSWStatus end =======================\n");
            // newSWupdate.sh if exist
            if ( stat("/tmp/newSWupdate.sh", &myst) == 0 ) {
                system("/tmp/newSWupdate.sh &");
                printf("Wait update~\n");
                usleep(10000000); // sleep 10s
                printf("update end\n");
            }
            return 0;
        } else {
            printf("UpdDLSWStatus() get result Fail\n");
            SaveLog("SWupdate UpdDLSWStatus() : get result Fail", st_time);
            printf("======================= UpdDLSWStatus end =======================\n");
            return 3;
        }
    }
}

void CheckProcess()
{
    char buf[128] = {0};
    char result[128] = {0};
    time_t current_time = 0;
    struct tm *st_time = NULL;
    FILE *fd = NULL;

    // set time
    current_time = time(NULL);
    st_time = localtime(&current_time);

    printf("======================== CheckProcess start ========================\n");
    // check dlg320.exe
    sprintf(buf, "ps | grep dlg320.exe | grep -v grep");
    fd = popen(buf, "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(result, 128, fd);
    pclose(fd);
    if ( strlen(result) < 10 ) {
        printf("run dlg320.exe\n");
        SaveLog("SWupdate CheckProcess() : Run dlg320.exe", st_time);
        system("/usr/home/dlg320.exe &");
        usleep(1000000);
    }
    else
        printf("dlg320.exe alive\n");

    // check DataProgram.exe
    sprintf(buf, "ps | grep DataProgram.exe | grep -v grep");
    fd = popen(buf, "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    memset(result, 0, 128);
    fgets(result, 128, fd);
    pclose(fd);
    if ( strlen(result) < 15 ) {
        printf("run DataProgram.exe\n");
        SaveLog("SWupdate CheckProcess() : Run DataProgram.exe", st_time);
        system("/usr/home/DataProgram.exe &");
        usleep(1000000);
    }
    else
        printf("DataProgram.exe alive\n");

    SaveLog("SWupdate CheckProcess() : OK", st_time);
    printf("========================= CheckProcess end =========================\n");

    return;
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
    int counter, run_processs_min, syslog_count;
    struct stat st;
    int doUpdDLSWStatus = 0;
    int reboot_min = 0;

    current_time = time(NULL);
    st_time = localtime(&current_time);

    // when boot to run once first
    init();
    getConfig();
    setCMD();
    setPath();

    printf("SW update start~\n");
    OpenLog(g_SYSLOG_PATH, st_time);
    SaveLog("SWupdate main() : start", st_time);

    QryDLSWUpdate();

    // for test
    /*strcpy(myupdate.DLModel, DLMODEL);
    myupdate.major = 1;
    myupdate.minor = 0;
    myupdate.patch = 1;
    DoUpdate();*/

    counter = 0;
    run_processs_min = -1;
    syslog_count = 0;
    while (1) {
        // get local time
        current_time = time(NULL);
        st_time = localtime(&current_time);
        // check process (1/min)
        if ( run_processs_min != st_time->tm_min ) {
            run_processs_min = st_time->tm_min;
            CheckProcess();

            // save sys log (10 min)
            syslog_count++;
            if ( syslog_count == 10 ) {
                syslog_count = 0;
                CloseLog();
                system("sync");
                OpenLog(g_SYSLOG_PATH, st_time);
            }

            // check reboot time
            reboot_min++;
            printf("reboot_min = %d\n", reboot_min);
            if ( reboot_time > 0 ) {
                if ( reboot_min > (reboot_time * 24 * 60) ) {
                    SaveLog("SWupdate main() : Reboot time's up!", st_time);
                    CloseLog();
                    printf("SWupdate main() : Reboot now!\n");
                    system("sync; sync; sync;");
                    system("reboot");
                    usleep(2000000);
                }
            }
        }

        // get config & set parameter
        getConfig();
        setCMD();
        setPath();
        // do QryDLSWUpdate
        if ( st_time->tm_min % update_SW_time == 0 ) {
            // if update file not exist
            if ( stat(g_UPDATE_PATH, &st) ) { // not in storage
                if ( stat(UPDATE_FILE, &st) ) { // not in /tmp
                    previous_time = current_time;
                    printf("localtime : %4d/%02d/%02d %02d:%02d:%02d", 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
                    //printf("#### Debug : QryDLSWUpdate start time : %ld ####\n", previous_time);

                    // get update info
                    if ( strlen(myupdate.SWURL) == 0 )
                        QryDLSWUpdate();

                    current_time = time(NULL);
                    counter = current_time - previous_time;
                    //printf("#### Debug : QryDLSWUpdate end time : %ld ####\n", current_time);
                    printf("#### Debug : QryDLSWUpdate span time : %d ####\n", counter);
                }
                else
                    printf("%s exist!\n", UPDATE_FILE);
            }
            else
                printf("%s exist!\n", g_UPDATE_PATH);
        }
        // sleep
        //printf("usleep() 60s\n");
        usleep(60000000);

        printf("######### check time #########\n");
        current_time = time(NULL);
        st_time = localtime(&current_time);
        printf("localtime : %4d/%02d/%02d %02d:%02d:%02d\n", 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
        printf("##############################\n");

        // check storage
        if ( stat(g_UPDATE_PATH, &st) == 0 )
            if ( CheckTime(st_time) )
                if ( !DoUpdate(g_UPDATE_PATH) )
                    doUpdDLSWStatus = 1;
        // check /tmp
        if ( stat(UPDATE_FILE, &st) == 0 )
            if ( CheckTime(st_time) )
                if ( !DoUpdate(UPDATE_FILE) )
                    doUpdDLSWStatus = 1;
        // update report
        if ( doUpdDLSWStatus )
            if ( !UpdDLSWStatus() ) {
                doUpdDLSWStatus = 0;
                memset(myupdate.SWURL, 0, 128);
            }
    }

    return 0;
}
