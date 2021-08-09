#include "datalog.h"
#include "G320.h"
#include "CyberPower.h"
#include "ADtek_CS1.h"
#include "inverter.h"

#include <unistd.h>
#include <time.h>

#define VERSION         "2.7.4"
#define MODEL_LIST_PATH "/usr/home/ModelList"
#define MODEL_NUM       1020 //255*4

CG320 *pg320 = NULL;
CyberPower *pcyberpower = NULL;
ADtek_CS1 *adtekcs1 = NULL;

bool GetConfig(); // 20181003 : now only sample time
bool GetModelList();
void Init();
int  ReRegister(time_t time);
int  AllRegister(time_t time);
void SaveList();
int GetAllData(time_t data_time);
void Show_State();
void Show_Time(struct tm *st_time);
//void Set_Sampletime(int num);

typedef struct system_config {
    int sample_time;
    //int upload_time;
} SYS_CONFIG;
SYS_CONFIG SConfig = {0};

typedef struct model_list {
    int addr;
    int devid;
    int port;
    char model[INV_SIZE];
    int model_index;
    bool init;
    bool first;
    bool last;
} MODEL_LIST;
MODEL_LIST MList[MODEL_NUM] = {0};

bool COM_OPENED[4] = {0};

int BUS_FD[4] = {0};

using namespace std;

extern "C" {
    #include "../common/SaveLog.h"
   //extern void    initdata();
   extern void    initenv(char *init_name);
   //extern  void MyStart();
}

int main(int argc, char* argv[])
{
    int previous_min = 60;
    int previous_hour = 24;
    int state = 0, ret = 0;
    time_t sys_current_time = 0, get_data_time = 0;
    struct tm *sys_st_time = NULL;

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

    sys_current_time = time(NULL);
    sys_st_time = localtime(&sys_current_time);
    //previous_hour = sys_st_time->tm_hour;
    //previous_min = sys_st_time->tm_min;

    memset(&SConfig, 0, sizeof(SConfig));
    GetConfig();

    memset(MList, 0, sizeof(MList));

    while (1) {
        // get system time
        sys_current_time = time(NULL);
        sys_st_time = localtime(&sys_current_time);

        // check time to run
        //if ( ( (previous_min != sys_st_time->tm_min) || ((previous_min == sys_st_time->tm_min) && (previous_hour != sys_st_time->tm_hour)) )
        //    && (sys_st_time->tm_min % SConfig.sample_time == 0) ) {
        if ( ((SConfig.sample_time <= 60) && (sys_st_time->tm_sec % SConfig.sample_time == 0)) ||
            ((SConfig.sample_time > 60) && (sys_st_time->tm_min % (SConfig.sample_time/60) == 0) && (previous_min != sys_st_time->tm_min) && (sys_st_time->tm_sec == 0)) ||
            ((SConfig.sample_time == 3600) && (sys_st_time->tm_min == 0) && (previous_hour != sys_st_time->tm_hour) && (sys_st_time->tm_sec == 0)) ) {

            printf("==== Run main loop start ====\n");
            previous_min = sys_st_time->tm_min;
            //previous_hour = sys_st_time->tm_hour;
            get_data_time = sys_current_time;

            // loop part
            GetConfig();
            GetModelList();
            Init();
            //Show_State();
            ret = GetAllData(get_data_time);
            if ( ret == -1 ) {
                printf("Continue main while loop.\n");
                continue;
            }
            // reregister
            if ( state != 0 ) {
                // one hour
                if ( previous_hour != sys_st_time->tm_hour ) {
                    previous_hour = sys_st_time->tm_hour;
                    ret = ReRegister(sys_current_time);
                    if ( ret == -1 ) {
                        printf("Continue main while loop.\n");
                        continue;
                    }
                }

                //every loop
                ret = AllRegister(sys_current_time);
                if ( ret == -1 ) {
                    printf("Continue main while loop.\n");
                    continue;
                }

                SaveList();
                system("sync");
            }

            ////////////
            if ( state == 0 ) {
                state = 1;
                previous_hour = sys_st_time->tm_hour;
            } else if ( state == 1 ) {
                // cancel auto setting sample time, but state change
                //Set_Sampletime((int)span_time/60+1);
                state = 2;
            } else if ( state == 2 )
                ;

            printf("======= main loop end =======\n");
        }

        usleep(100000);
        //printf("press any key to continue!!\n");
        //getchar();
    }
    return 0;
}

bool GetConfig()
{
    char buf[32] = {0};
    FILE *pFile = NULL;

    // get sample_time
    pFile = popen("uci get dlsetting.@sms[0].sample_time", "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &SConfig.sample_time);
    printf("Sample time (Sec.) = %d\n", SConfig.sample_time);

    // get upload_time
    /*pFile = popen("uci get dlsetting.@sms[0].upload_time", "r");
    if ( pFile == NULL ) {
        printf("popen fail!\n");
        return false;
    }
    fgets(buf, 32, pFile);
    pclose(pFile);
    sscanf(buf, "%d", &SConfig.upload_time);
    printf("Upload time (Min.) = %d\n", SConfig.upload_time);*/

    return true;
}

bool GetModelList()
{
    char buf[128] = {0}, tmpmodel[64] = {0};
    int i = 0, num = 0, tmpaddr = 0, tmpid = 0, tmpport = 0;
    FILE *pfile = NULL;

    // get model list
    pfile = fopen(MODEL_LIST_PATH, "r");
    if ( pfile == NULL ) {
        printf("fopen %s fail!\n", MODEL_LIST_PATH);
        return false;
    }
    // clean addr, reload model list addr again
    for (i = 0; i < MODEL_NUM; i++) {
        MList[i].addr = 0;
        MList[i].first = false;
        MList[i].last = false;
    }
    while ( fgets(buf, 128, pfile) != NULL ) {
        if ( strlen(buf) == 0 )
            break;

        sscanf(buf, "%04d Addr:%03d DEVID:%d Port:COM%d Model:%63s", &num, &tmpaddr, &tmpid, &tmpport, tmpmodel);
        //i = (tmpport-1)*255 + tmpaddr;
        MList[num].addr = tmpaddr;
        MList[num].devid = tmpid;
        MList[num].port = tmpport;
        strcpy(MList[num].model, tmpmodel);
        printf("Get [%03d] Addr = %03d, DEVID = %d, port = %d, model = %s\n", num, MList[num].addr, MList[num].devid, MList[num].port, MList[num].model);
    }
    fclose(pfile);

    // if addr = 0 , it's meean model list delete the model from luci page if it exist before, so clean other data
    for (i = 0; i < MODEL_NUM; i++) {
        if ( MList[i].addr == 0 ) {
            memset(&MList[i], 0, sizeof(MList[i]));
        }
    }

    return true;
}

void Init()
{
    int i = 0, j = 0;

    // for test ////////////
    char buf[256] = {0};
    ////////////////////////

    // find first index
    for (i = 0; i < MODEL_NUM; i++) {
        if ( MList[i].addr > 0 ) {
            MList[i].first = true;
            break;
        }
    }

    // fnd last index
    for (i = MODEL_NUM-1; i >= 0; i--) {
        if ( MList[i].addr > 0 ) {
            MList[i].last = true;
            break;
        }
    }

    printf("==== Run main Init start ====\n");

    for (i = 0; i < MODEL_NUM; i++) {
        if ( (MList[i].addr > 0) && (MList[i].init == false) ) {
            printf("find %d\n", MList[i].addr);
            for (j = 1; j < INV_COUNT; j++) {
                //printf("check %s\n", INVERTER[j]);
                if ( !strcmp(MList[i].model, INVERTER[j]) ) {
                    printf("match %s\n", INVERTER[j]);
                    MList[i].model_index = j;

                    switch (MList[i].model_index)
                    {
                        case ID_Unknown:
                            printf("%d Unknown model!\n", MList[i].addr);
                            printf("Nothing to do~\n");
                            break;
                        case ID_Darfon:
                            printf("%d Darfon init start~\n", MList[i].addr);
                            if ( COM_OPENED[MList[i].port-1] == false ) {
                                printf("Do open com port %d init\n", MList[i].port);
                                initenv((char *)"/usr/home/G320.ini");
                                if ( pg320 == NULL )
                                    pg320 = new CG320;
                                BUS_FD[MList[i].port-1] = pg320->Init(MList[i].devid, MList[i].port, true, MList[i].first, 0);
                                if ( BUS_FD[MList[i].port-1] > 0 ) {
                                    COM_OPENED[MList[i].port-1] = true;
                                    MList[i].init = 1;
                                }
                            } else {
                                if ( MList[i].init == 0 ) {
                                    printf("Do init\n");
                                    initenv((char *)"/usr/home/G320.ini");
                                    if ( pg320 == NULL )
                                        pg320 = new CG320;
                                    if ( pg320->Init(MList[i].devid, MList[i].port, false, false, BUS_FD[MList[i].port-1]) == 0 ) {
                                        MList[i].init = 1;
                                    }
                                }
                            }
                            printf("Darfon init end.\n");
                            break;
                        case ID_CyberPower1P:
                        case ID_CyberPower3P:
                            printf("%d CyberPower init start~\n", MList[i].addr);
                            if ( COM_OPENED[MList[i].port-1] == false ) {
                                printf("Do open com port %d init\n", MList[i].port);
                                initenv((char *)"/usr/home/G320.ini");
                                if ( pcyberpower == NULL )
                                    pcyberpower = new CyberPower;
                                BUS_FD[MList[i].port-1] = pcyberpower->Init(MList[i].port, true, MList[i].first, 0);
                                if ( BUS_FD[MList[i].port-1] > 0 ) {
                                    COM_OPENED[MList[i].port-1] = true;
                                    MList[i].init = 1;
                                }
                            } else {
                                if ( MList[i].init == 0 ) {
                                    printf("Do init\n");
                                    initenv((char *)"/usr/home/G320.ini");
                                    if ( pcyberpower == NULL )
                                        pcyberpower = new CyberPower;
                                    if ( pcyberpower->Init(MList[i].port, false, false, BUS_FD[MList[i].port-1]) == 0 ) {
                                        MList[i].init = 1;
                                    }
                                }
                            }
                            printf("CyberPower init end.\n");
                            break;
                        case ID_ADtekCS1T:
                            printf("%d ADtekCS1 init start~\n", MList[i].addr);
                            if ( COM_OPENED[MList[i].port-1] == false ) {
                                printf("Do open com port %d init\n", MList[i].port);
                                initenv((char *)"/usr/home/G320.ini");
                                if ( adtekcs1 == NULL )
                                    adtekcs1 = new ADtek_CS1;
                                BUS_FD[MList[i].port-1] = adtekcs1->Init(MList[i].port, true, MList[i].first, 0);
                                if ( BUS_FD[MList[i].port-1] > 0 ) {
                                    COM_OPENED[MList[i].port-1] = true;
                                    MList[i].init = 1;
                                }
                            } else {
                                if ( MList[i].init == 0 ) {
                                    printf("Do init\n");
                                    initenv((char *)"/usr/home/G320.ini");
                                    if ( adtekcs1 == NULL )
                                        adtekcs1 = new ADtek_CS1;
                                    if ( adtekcs1->Init(MList[i].port, false, false, BUS_FD[MList[i].port-1]) == 0 ) {
                                        MList[i].init = 1;
                                    }
                                }
                            }
                            printf("ADtekCS1 init end.\n");
                            break;
                        case ID_Test:
                            // for test
                            printf("%d Test init start~\n", MList[i].addr);
                            if ( COM_OPENED[MList[i].port-1] == false ) {
                                printf("Do open com port %d init\n", MList[i].port);
                                BUS_FD[MList[i].port-1] = 99;
                                COM_OPENED[MList[i].port-1] = true;
                                MList[i].init = 1;
                            } else {
                                if ( MList[i].init == 0 ) {
                                    printf("Do init\n");
                                    MList[i].init = 1;
                                    if ( MList[i].last ) {
                                        sprintf(buf, "cp -f /tmp/tmpDeviceList /tmp/DeviceList");
                                        system(buf);
                                    }
                                }
                            }
                            printf("Test init end.\n");
                            break;
                        default:
                            printf("%d Other init\n", MList[i].addr);
                    }
                    break;
                }
            }
        }
    }

    printf("======= main Init end =======\n");

    return;
}

int ReRegister(time_t time)
{
    int i = 0, ret = 0, cnt = 0;

    printf("==== Run main ReRegister start ====\n");

    for (i = 0; i < MODEL_NUM; i++) {
        if ( (MList[i].addr > 0) && (MList[i].init == true) ) {
            switch (MList[i].model_index)
            {
                case ID_Unknown:
                    printf("%d Unknown model!\n", MList[i].addr);
                    printf("Nothing to do~\n");
                    break;
                case ID_Darfon:
                    printf("%d Darfon DoReRegister start~\n", MList[i].addr);
                    ret = pg320->DoReRegister(time);
                    if ( ret == -1 ) {
                        printf("Time's up, Quit ReRegister()\n");
                        return -1;
                    }
                    if ( ret ) {
                        printf("DoReRegister %d device\n", ret);
                        cnt += ret;
                    }
                    printf("Darfon DoReRegister end.\n");
                    break;
                case ID_CyberPower1P:
                case ID_CyberPower3P:
                    printf("%d CyberPower reregister start~\n", MList[i].addr);
                    printf("Nothing to do~\n");
                    break;
                case ID_Test:
                    printf("%d Test reregister start~\n", MList[i].addr);
                    printf("Test reregister end.\n");
                    break;
                default:
                    printf("%d Other reregister\n", MList[i].addr);
            }
        }
    }

    printf("======= main ReRegister end =======\n");

    return cnt;
}

int AllRegister(time_t time)
{
    int i = 0, ret = 0, cnt = 0;

    printf("==== Run main AllRegister start ====\n");

    for (i = 0; i < MODEL_NUM; i++) {
        if ( (MList[i].addr > 0) && (MList[i].init == true) ) {
            switch (MList[i].model_index)
            {
                case ID_Unknown:
                    printf("%d Unknown model!\n", MList[i].addr);
                    printf("Nothing to do~\n");
                    break;
                case ID_Darfon:
                    printf("%d Darfon DoAllRegister start~\n", MList[i].addr);
                    ret = pg320->DoAllRegister(time);
                    if ( ret == -1 ) {
                        printf("Time's up, Quit AllRegister()\n");
                        return -1;
                    }
                    if ( ret ) {
                        printf("DoAllRegister %d device\n", ret);
                        cnt += ret;
                    }
                    printf("Darfon DoAllRegister end.\n");
                    break;
                case ID_CyberPower1P:
                case ID_CyberPower3P:
                    printf("%d CyberPower allregister start~\n", MList[i].addr);
                    printf("Nothing to do~\n");
                    break;
                case ID_Test:
                    printf("%d Test allregister start~\n", MList[i].addr);
                    printf("Test allregister end.\n");
                    break;
                default:
                    printf("%d Other allregister\n", MList[i].addr);
            }
        }
    }

    printf("======= main AllRegister end =======\n");

    return cnt;
}

void SaveList()
{
    int i = 0, ret = 0;

    printf("==== Run main SaveList start ====\n");

    for (i = 0; i < MODEL_NUM; i++) {
        if ( (MList[i].addr > 0) && (MList[i].init == true) ) {
            switch (MList[i].model_index)
            {
                case ID_Unknown:
                    printf("%d Unknown model!\n", MList[i].addr);
                    printf("Nothing to do~\n");
                    break;
                case ID_Darfon:
                    printf("%d Darfon SaveList start~\n", MList[i].addr);
                    ret = pg320->SaveDeviceList(MList[i].first, MList[i].last);
                    if ( ret )
                        printf("SaveDeviceList %d device\n", ret);
                    printf("Darfon SaveDeviceList end.\n");
                    break;
                case ID_CyberPower1P:
                case ID_CyberPower3P:
                    printf("%d CyberPower SaveList start~\n", MList[i].addr);
                    printf("Nothing to do~\n");
                    break;
                case ID_Test:
                    printf("%d Test SaveList start~\n", MList[i].addr);
                    printf("Test SaveList end.\n");
                    break;
                default:
                    printf("%d Other SaveList\n", MList[i].addr);
            }
        }
    }

    printf("======= main SaveList end =======\n");

    return;
}

int GetAllData(time_t data_time)
{
    int i = 0;

    // for test ////////////
    FILE *pFile = NULL;
    struct stat filest;
    int filesize = 0, ret = 0;
    static int tmpsize = 0;
    char buf[256] = {0};
    struct tm *st_time = NULL;
    ////////////////////////

    printf("==== Run main GetAllData start ====\n");

    for (i = 0; i < MODEL_NUM; i++) {
        if ( (MList[i].addr > 0) && (MList[i].init == true) ) {
            // sleep
            //usleep(20000);
            switch (MList[i].model_index)
            {
                case ID_Unknown:
                    printf("%d Unknown model!\n", MList[i].addr);
                    printf("Nothing to do~\n");
                    break;
                case ID_Darfon:
                    printf("%d Darfon GetData start~\n", MList[i].addr);
                    ret = pg320->GetData(data_time, MList[i].first, MList[i].last);
                    printf("Darfon GetData end.\n");
                    if ( ret == -1 ) {
                        printf("Time's up, Quit GetAllData()\n");
                        return -1;
                    }
                    break;
                case ID_CyberPower1P:
                    printf("%d CyberPower Get1PData %d start~\n", MList[i].addr, MList[i].devid);
                    pcyberpower->Get1PData(MList[i].addr, MList[i].devid, data_time, MList[i].first, MList[i].last);
                    printf("CyberPower Get1PData end.\n");
                    break;
                case ID_CyberPower3P:
                    printf("%d CyberPower Get3PData %d start~\n", MList[i].addr, MList[i].devid);
                    //pcyberpower->Get3PData(MList[i].addr, MList[i].devid, data_time, MList[i].first, MList[i].last);
                    printf("CyberPower Get3PData end.\n");
                    break;
                case ID_ADtekCS1T:
                    printf("%d ID_ADtekCS1T GetEnv %d start~\n", MList[i].addr, MList[i].devid);
                    adtekcs1->GetEnv(MList[i].addr, MList[i].devid, data_time, MList[i].first, MList[i].last);
                    printf("ADtekCS1T GetEnv end.\n");
                    break;
                case ID_Test:
                    // for test
                    printf("%d Test getdata start~\n", MList[i].addr);
                    if ( MList[i].first ) {
                        pFile = fopen("/tmp/tmpDeviceList", "w");
                        if ( pFile != NULL )
                            fclose(pFile);

                        pFile = fopen("/tmp/tmplog", "wb");
                        if ( pFile != NULL ) {
                            fwrite("<records>", 1, 9, pFile);
                            sprintf(buf, "<test log id = %d>", MList[i].addr);
                            fwrite(buf, 1, strlen(buf), pFile);
                            fclose(pFile);
                        }

                        pFile = fopen("/tmp/tmperrlog", "wb");
                        if ( pFile != NULL ) {
                            fwrite("<records>", 1, 9, pFile);
                            sprintf(buf, "<test errlog id = %d>", MList[i].addr);
                            fwrite(buf, 1, strlen(buf), pFile);
                            fclose(pFile);
                        }

                        pFile = fopen("/tmp/tmpenv", "wb");
                        if ( pFile != NULL ) {
                            fwrite("<records>", 1, 9, pFile);
                            sprintf(buf, "<test env id = %d>", MList[i].addr);
                            fwrite(buf, 1, strlen(buf), pFile);
                            fclose(pFile);
                        }

                        pFile = fopen("/tmp/tmpMIList", "wb");
                        if ( pFile != NULL ) {
                            fwrite("<records>\n", 1, 10, pFile);
                            sprintf(buf, "<test MIList id = %d>\n", MList[i].addr);
                            fwrite(buf, 1, strlen(buf), pFile);
                            fclose(pFile);
                        }
                    } else {
                        pFile = fopen("/tmp/tmplog", "ab");
                        if ( pFile != NULL ) {
                            sprintf(buf, "<test log id = %d>", MList[i].addr);
                            fwrite(buf, 1, strlen(buf), pFile);
                            fclose(pFile);
                        }

                        pFile = fopen("/tmp/tmperrlog", "ab");
                        if ( pFile != NULL ) {
                            sprintf(buf, "<test errlog id = %d>", MList[i].addr);
                            fwrite(buf, 1, strlen(buf), pFile);
                            fclose(pFile);
                        }

                        pFile = fopen("/tmp/tmpenv", "ab");
                        if ( pFile != NULL ) {
                            sprintf(buf, "<test env id = %d>", MList[i].addr);
                            fwrite(buf, 1, strlen(buf), pFile);
                            fclose(pFile);
                        }

                        pFile = fopen("/tmp/tmpMIList", "ab");
                        if ( pFile != NULL ) {
                            sprintf(buf, "<test MIList id = %d>\n", MList[i].addr);
                            fwrite(buf, 1, strlen(buf), pFile);
                            fclose(pFile);
                        }
                    }

                    if ( MList[i].last ) {
                        sprintf(buf, "cp -f /tmp/tmpDeviceList /tmp/DeviceList");
                        system(buf);

                        st_time = localtime(&data_time);
                        if ( stat("/tmp/tmplog", &filest) == 0 )
                            filesize = filest.st_size;
                        else
                            filesize = 0;
                        pFile = fopen("/tmp/tmplog", "ab");
                        if ( pFile != NULL ) {
                            fwrite("</records>", 1, 10, pFile);
                            filesize += 10;
                            while ( filesize%3 != 0 ) {
                                fwrite(" ", 1, 1, pFile);
                                filesize++;
                            }
                            fclose(pFile);
                        }
                        if ( filesize > 100 ) {
                            sprintf(buf, "cp /tmp/tmplog /tmp/test/XML/LOG/%04d%02d%02d/%02d%02d",
                                        1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday, st_time->tm_hour, st_time->tm_min);
                            system(buf);
                        }

                        if ( stat("/tmp/tmperrlog", &filest) == 0 )
                            filesize = filest.st_size;
                        else
                            filesize = 0;
                        pFile = fopen("/tmp/tmperrlog", "ab");
                        if ( pFile != NULL ) {
                            fwrite("</records>", 1, 10, pFile);
                            filesize += 10;
                            while ( filesize%3 != 0 ) {
                                fwrite(" ", 1, 1, pFile);
                                filesize++;
                            }
                            fclose(pFile);
                        }
                        if ( filesize > 100 ) {
                            sprintf(buf, "cp /tmp/tmperrlog /tmp/test/XML/ERRLOG/%04d%02d%02d/%02d%02d",
                                        1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday, st_time->tm_hour, st_time->tm_min);
                            system(buf);
                        }

                        if ( stat("/tmp/tmpenv", &filest) == 0 )
                            filesize = filest.st_size;
                        else
                            filesize = 0;
                        pFile = fopen("/tmp/tmpenv", "ab");
                        if ( pFile != NULL ) {
                            fwrite("</records>", 1, 10, pFile);
                            filesize += 10;
                            while ( filesize%3 != 0 ) {
                                fwrite(" ", 1, 1, pFile);
                                filesize++;
                            }
                            fclose(pFile);
                        }
                        if ( filesize > 50 ) {
                            sprintf(buf, "cp /tmp/tmpenv /tmp/test/XML/ENV/%04d%02d%02d/%02d%02d",
                                        1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday, st_time->tm_hour, st_time->tm_min);
                            system(buf);
                        }

                        if ( stat("/tmp/tmpMIList", &filest) == 0 )
                            filesize = filest.st_size;
                        else
                            filesize = 0;
                        pFile = fopen("/tmp/tmpMIList", "ab");
                        if ( pFile != NULL ) {
                            fwrite("</records>", 1, 10, pFile);
                            filesize += 10;
                            while ( filesize%3 != 0 ) {
                                fwrite(" ", 1, 1, pFile);
                                filesize++;
                            }
                            fclose(pFile);
                        }
                        if ( filesize > 100 ) {
                            printf("tmpsize = %d, filesize = %d\n", tmpsize, filesize);
                            if ( tmpsize != filesize ) {
                                sprintf(buf, "cp /tmp/tmpMIList /tmp/MIList_%4d%02d%02d_%02d%02d00",
                                            1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday, st_time->tm_hour, st_time->tm_min);
                                system(buf);
                                tmpsize = filesize;
                            }
                        }

                        //system("sync");
                    }
                    printf("Test getdata end.\n");
                    break;
                default:
                    printf("%d Other getdata\n", MList[i].addr);
            }
        }
    }
    CloseLog();

    printf("======= main GetAllData end =======\n");

    return 0;
}

void Show_State()
{
    int i = 0;

    printf("================================================================================================================\n");
    for (i = 0; i < MODEL_NUM; i++) {
        if ( MList[i].addr > 0 )
            printf("[%03d] Addr = %03d, devid = %d, port = %d, model_index = %d, init = %d, first = %d, lase = %d, model = %s\n",
                i, MList[i].addr, MList[i].devid, MList[i].port, MList[i].model_index, MList[i].init, MList[i].first, MList[i].last, MList[i].model);
    }
    for (i = 0; i < 4; i++) {
        printf("Port %d fd = %d\n", i, BUS_FD[i]);
    }
    printf("================================================================================================================\n");

    return;
}

void Show_Time(struct tm *st_time)
{
    printf("############## Show Time ##############\n");
    printf("localtime : %4d/%02d/%02d ", 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday);
    printf("day[%d] %02d:%02d:%02d\n", st_time->tm_wday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
    printf("#######################################\n");

    return;
}
/*
void Set_Sampletime(int num)
{
    char buf[256] = {0};

    int sampletime[12] = {1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60};
    int i = 0;

    if ( num <= 0 )
        return;
    if ( num > 20 )
        num = 20;

    printf("############## Set Sample Time ##############\n");
    printf("Input num = %d\n", num);
    if ( SConfig.sample_time < num ) {
        // set sample time
        for (i = 0; i < 10; i++) {
            if ( sampletime[i] >= num )
                break;
        }
        SConfig.sample_time = sampletime[i];
        sprintf(buf, "uci set dlsetting.@sms[0].sample_time='%d'", SConfig.sample_time);
        system(buf);
        system("uci commit dlsetting");
        printf("Set Sample Time = %d\n", SConfig.sample_time);
        // set upload time
        if ( i < 4 )
            i = 4; // set sampletime[4] = 5 min
        //else if ( i < 11 )
        //    i++; // set next bigger time
        SConfig.upload_time = sampletime[i];
        sprintf(buf, "uci set dlsetting.@sms[0].upload_time='%d'", SConfig.upload_time);
        system(buf);
        system("uci commit dlsetting");
        printf("Set Upload Time = %d\n", SConfig.upload_time);
    }
    printf("#############################################\n");

    return;
}
*/
