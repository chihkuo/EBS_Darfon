#include "datalog.h"
#include "G320.h"
#include "CyberPower.h"
#include "inverter.h"

#include <unistd.h>
#include <time.h>

#define VERSION         "2.0.1"
#define MODEL_LIST_PATH "/usr/home/ModelList"
#define MODEL_NUM       1020 //255*4

CG320 *pg320 = NULL;
CyberPower *pcyberpower = NULL;

bool GetConfig(); // 20181003 : now only sample time
bool GetModelList();
void Init();
int  ReRegister(time_t time);
int  AllRegister(time_t time);
void GetAllData(time_t data_time);
void Show_State();
void Show_Time(struct tm *st_time);

typedef struct system_config {
    int sample_time;
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
    int reregister_hour = 24;
    int allregister_day = 0;
    time_t sys_current_time = 0;
    time_t get_data_time = 0;
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
    reregister_hour = sys_st_time->tm_hour;
    allregister_day = sys_st_time->tm_mday;

    memset(&SConfig, 0, sizeof(SConfig));
    GetConfig();

    memset(MList, 0, sizeof(MList));

    while (1) {
        // get system time
        sys_current_time = time(NULL);
        sys_st_time = localtime(&sys_current_time);
        if ( sys_st_time->tm_sec % 10 == 0 )
            Show_Time(sys_st_time);

        // check time to run
        if ( ( (previous_min != sys_st_time->tm_min) || ((previous_min == sys_st_time->tm_min) && (previous_hour != sys_st_time->tm_hour)) )
            && (sys_st_time->tm_min % SConfig.sample_time == 0) ) {
        //if (1) {
            printf("==== Run main loop start ====\n");
            previous_min = sys_st_time->tm_min;
            previous_hour = sys_st_time->tm_hour;
            get_data_time = sys_current_time;

            // loop part
            GetConfig();
            GetModelList();
            Init();
            Show_State();
            GetAllData(get_data_time);
            ////////////

            printf("======= main loop end =======\n");
        }

        if ( reregister_hour != sys_st_time->tm_hour ) {
            reregister_hour = sys_st_time->tm_hour;
            ReRegister(sys_current_time);
        }

        if ( allregister_day != sys_st_time->tm_mday ) {
            allregister_day = sys_st_time->tm_mday;
            AllRegister(sys_current_time);
        }

        usleep(1000000);
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
    printf("Sample time (Min.) = %d\n", SConfig.sample_time);

    return true;
}

bool GetModelList()
{
    char buf[128] = {0}, tmpmodel[64] = {0};
    int i = 0, tmpaddr = 0, tmpid = 0, tmpport = 0;
    FILE *pfile = NULL;

    // get model list
    pfile = fopen(MODEL_LIST_PATH, "r");
    if ( pfile == NULL ) {
        printf("fopen %s fail!\n", MODEL_LIST_PATH);
        return false;
    }
    // clean addr, reload model list addr again
    for (i = 0; i < MODEL_NUM; i++)
        MList[i].addr = 0;
    while ( fgets(buf, 128, pfile) != NULL ) {
        if ( strlen(buf) == 0 )
            break;

        sscanf(buf, "Addr:%03d DEVID:%d Port:COM%d Model:%63s", &tmpaddr, &tmpid, &tmpport, tmpmodel);
        //sscanf(buf, "Addr:%03d DEVID:%d Port:COM%d Model:%63s", &MList[tmp-1].addr, &MList[tmp-1].devid, &MList[tmp-1].port, MList[tmp-1].model);
        i = (tmpport-1)*255 + tmpaddr;
        MList[i-1].addr = tmpaddr;
        MList[i-1].devid = tmpid;
        MList[i-1].port = tmpport;
        strcpy(MList[i-1].model, tmpmodel);
        printf("Get [%03d] Addr = %03d, DEVID = %d, port = %d, model = %s\n", i-1, MList[i-1].addr, MList[i-1].devid, MList[i-1].port, MList[i-1].model);
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
                                if ( pg320->Init(MList[i].devid, MList[i].port, true, MList[i].first) ) {
                                    COM_OPENED[MList[i].port-1] = true;
                                    MList[i].init = 1;
                                }
                            } else {
                                if ( MList[i].init == 0 ) {
                                    printf("Do init\n");
                                    initenv((char *)"/usr/home/G320.ini");
                                    if ( pg320 == NULL )
                                        pg320 = new CG320;
                                    if ( pg320->Init(MList[i].devid, MList[i].port, false, false) ) {
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
                                if ( pcyberpower->Init(MList[i].port, true, MList[i].first) ) {
                                    COM_OPENED[MList[i].port-1] = true;
                                    MList[i].init = 1;
                                }
                            } else {
                                if ( MList[i].init == 0 ) {
                                    printf("Do init\n");
                                    initenv((char *)"/usr/home/G320.ini");
                                    if ( pcyberpower == NULL )
                                        pcyberpower = new CyberPower;
                                    if ( pcyberpower->Init(MList[i].port, false, false) ) {
                                        MList[i].init = 1;
                                    }
                                }
                            }
                            printf("CyberPower init end.\n");
                            break;
                        case ID_Test:
                            // for test
                            printf("%d Test init start~\n", MList[i].addr);
                            if ( COM_OPENED[MList[i].port-1] == false ) {
                                printf("Do open com port %d init\n", MList[i].port);
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

void GetAllData(time_t data_time)
{
    int i = 0;

    // for test ////////////
    FILE *pFile = NULL;
    struct stat filest;
    int filesize = 0;
    static int tmpsize = 0;
    char buf[256] = {0};
    struct tm *st_time = NULL;
    ////////////////////////

    printf("==== Run main GetAllData start ====\n");

    for (i = 0; i < MODEL_NUM; i++) {
        if ( (MList[i].addr > 0) && (MList[i].init == true) ) {
            // sleep
            usleep(20000);
            switch (MList[i].model_index)
            {
                case ID_Unknown:
                    printf("%d Unknown model!\n", MList[i].addr);
                    printf("Nothing to do~\n");
                    break;
                case ID_Darfon:
                    printf("%d Darfon GetData start~\n", MList[i].addr);
                    pg320->GetData(data_time, MList[i].first, MList[i].last);
                    printf("Darfon GetData end.\n");
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

                        system("sync");
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

    return;
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
