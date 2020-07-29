#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#include "dlsocket.h"

#define MY_PORT 8700

// config
int delay_time_1 = 0;
int delay_time_2 = 0;
int com_port = 0;
int baud_rate = 0;
int data_bits = 0;
char parity[8] = {0};
int stop_bits = 0;
int com_fd = 0;
int update_interval = 0;
int inverter_state = 0;
int no_usb = 0;
time_t save_time = 0;

// for register
unsigned char query_buf[QUERY_SIZE];
int dev_count = 0;
DEV_LIST dev_list[253] = {0};
// for get data
int loop_err_cnt = 0;
HB_ID_DATA id_data = {0};
HB_ID_FLAGS1 id_flags1 = {0};
HB_ID_FLAGS2 id_flags2 = {0};
HB_RTC_DATA rtc_data = {0};
HB_RS_INFO rs_info = {0};
HB_RRS_INFO rrs_info = {0};
HB_RT_INFO rt_info = {0};
HB_PVINV_ERR_COD1 pvinv_err_cod1 = {0};
HB_PVINV_ERR_COD2 pvinv_err_cod2 = {0};
HB_PVINV_ERR_COD3 pvinv_err_cod3 = {0};
HB_DD_ERR_COD dd_err_cod = {0};
HB_DD_ERR_COD2 dd_err_cod2 = {0};
HB_ICON_INFO icon_info = {0};
HB_BMS_INFO bms_info = {0};

void stop_process()
{
    int pid = 0;
    FILE *pfile = NULL;
    char buf[128] = {0};

    // get pid of run_DLSW.sh if exit
    pfile = popen(" ps | grep run_DLSW.sh | grep -v grep", "r");
    if ( pfile != NULL ) {
        fgets(buf, 128, pfile);
        sscanf(buf, " %d", &pid);
        pclose(pfile);
    } else {
        printf("run_DLSW.sh not found.\n");
    }

    if (pid > 0) {
        sprintf(buf, "kill %d", pid);
        system(buf);
        system("sync");
    }

    //system("/etc/init.d/run_DL.sh stop");
    system("killall -9 SWupdate.exe FWupdate.exe dlg320.exe");

    return;
}

void start_process()
{
    int pid = 0;
    FILE *pfile = NULL;
    char buf[128] = {0};

    // get pid of run_DLSW.sh if exit
    pfile = popen(" ps | grep run_DLSW.sh | grep -v grep", "r");
    if ( pfile != NULL ) {
        fgets(buf, 128, pfile);
        sscanf(buf, " %d", &pid);
        pclose(pfile);
    } else {
        printf("run_DLSW.sh not found.\n");
    }

    if ( pid == 0 ) {
        printf("call run_DLSW.sh in background\n");
        system("/usr/home/run_DLSW.sh &");
    }

    return;
}

void get_config()
{
    char buf[128] = {0};
    char cmd[128] = {0};
    char *cptr = NULL;
    FILE *fd = NULL;
    int tmp_interval = 0;
    struct stat st;

    initenv((char *)"/usr/home/G320.ini");

    // get delay_time_1
    fd = popen("uci get dlsetting.@sms[0].delay_time_1", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 128, fd);
    pclose(fd);
    sscanf(buf, "%d", &delay_time_1);
    printf("Delay time 1 (us.) = %d\n", delay_time_1);

    // get delay_time_2
    fd = popen("uci get dlsetting.@sms[0].delay_time_2", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 128, fd);
    pclose(fd);
    sscanf(buf, "%d", &delay_time_2);
    printf("Delay time 2 (us.) = %d\n", delay_time_2);

    // get com port
    fd = fopen(MODEL_PATH, "rb");
    if ( fd == NULL ) {
        printf("fopen fail!\n");
        return;
    }
    while ( fgets(buf, 128, fd) != NULL ) {
        if ( strstr(buf, "Darfon") != NULL ) {
            cptr = strstr(buf, "Port:");
            sscanf(cptr, "Port:COM%d", &com_port);
            printf("get comport = %d\n", com_port);
            break;
        }
    }
    fclose(fd);

    if ( com_port == 0 )
        return;

    // get com port setting
    // get baud
    sprintf(cmd, "uci get dlsetting.@comport[0].com%d_baud", com_port);
    fd = popen(cmd, "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 128, fd);
    pclose(fd);
    sscanf(buf, "%d", &baud_rate);
    printf("Baud rate = %d\n", baud_rate);

    // get data bits
    sprintf(cmd, "uci get dlsetting.@comport[0].com%d_data_bits", com_port);
    fd = popen(cmd, "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 128, fd);
    pclose(fd);
    sscanf(buf, "%d", &data_bits);
    printf("Data bits = %d\n", data_bits);

    // get parity
    sprintf(cmd, "uci get dlsetting.@comport[0].com%d_parity", com_port);
    fd = popen(cmd, "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(parity, 8, fd);
    pclose(fd);
    parity[strlen(parity)-1] = 0; // clean \n
    printf("Parity = %s\n", parity);

    // get stop bits
    sprintf(cmd, "uci get dlsetting.@comport[0].com%d_stop_bits", com_port);
    fd = popen(cmd, "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 128, fd);
    pclose(fd);
    sscanf(buf, "%d", &stop_bits);
    printf("Stop bits = %d\n", stop_bits);

    // get upload interval
    fd = popen("uci get dlsetting.@sms[0].upload_time", "r");
    if ( fd == NULL ) {
        printf("popen fail!\n");
        return;
    }
    fgets(buf, 128, fd);
    pclose(fd);
    sscanf(buf, "%d", &tmp_interval);
    if ( update_interval == tmp_interval ) {
        printf("same update interval %d\n", update_interval);
    } else {
        update_interval = tmp_interval;
        printf("set update interval %d\n", update_interval);
    }

    // check usb device exist
    if ( stat(USB_DEV, &st) == 0 ) {
        // exist
        no_usb = 0;
    } else {
        // none
        no_usb = 1;
    }

    return;
}

int open_com_port()
{
    char dev_name[13] = "/dev/ttyUSB0";
    char par_tmp = 0;

    if ( com_fd > 0 ) {
        ModbusDrvDeinit(com_fd);
        usleep(1000000);
    }

    //ModbusDrvDeinit(1);
    //ModbusDrvDeinit(2);
    //ModbusDrvDeinit(3);
    //ModbusDrvDeinit(4);
    //ModbusDrvDeinit(5);
    //ModbusDrvDeinit(6);

    switch (com_port) {
        case 1:
            break;
        case 2:
            dev_name[11] = '1';
            break;
        case 3:
            dev_name[11] = '2';
            break;
        case 4:
            dev_name[11] = '3';
            break;
        default:
            printf("com port = %d\n", com_port);
    }

    if ( strstr(parity, "Odd") )
        par_tmp = 'O';
    else if ( strstr(parity, "Even") )
        par_tmp = 'E';
    else
        par_tmp = 'N';

    com_fd = MyModbusDrvInit(dev_name, baud_rate, data_bits, par_tmp, stop_bits);
    if ( com_fd < 0 ) {
        printf("run MyModbusDrvInit() fail\n");
        return -1;
    } else {
        printf("\ncom_fd = %d\n", com_fd);
        return com_fd;
    }
}

int AllocateProcess(unsigned char *query, int len)
{
    int i = 0, j = 0, index = 0, cnt = 0;
    char sn_tmp[17] = {0};

    //time_t current_time;

    //SaveLog((char *)"DataLogger AllocateProcess() : run", m_st_time);
    for (i = 0; i < len-12; i++) {
        if ( query[i] == 0x00 && query[i+1] == 0x00 && query[i+2] == 0x08 ) {
            if ( CheckCRC(&query[i], 13) ) {
                //printf("#### Send allocate address parameter ####\n");
                // get SN
                sprintf(sn_tmp, "%02X%02X%02X%02X%02X%02X%02X%02X", query[i+3], query[i+4],
                        query[i+5], query[i+6], query[i+7], query[i+8], query[i+9], query[i+10]);
                // check SN
                for (j = 0; j < dev_count; j++) {
                    if ( !strncmp(dev_list[j].m_Sn, sn_tmp, 16) ) {
                        // already have device
                        index = j;
                        break;
                    } else
                        index++;
                }
                // if new device
                if ( index == dev_count )
                    memcpy(dev_list[index].m_Sn, sn_tmp, 17);

                printf("dev_count = %d\n", dev_count);
                printf("index = %d\n", index);
                printf("m_Addr = %d\n", dev_list[index].m_Addr);
                printf("m_Sn = %s\n", dev_list[index].m_Sn);
                printf("#########################################\n");

                // get time
                //current_time = time(NULL);
                //m_st_time = localtime(&current_time);
                // get data time not 0 min. current time is 0 min.
                //if ( m_data_st_time.tm_min != 0 )
                //    if ( m_st_time->tm_min == 0 )
                //        return -1;

                if ( MyAssignAddress(com_fd, &query[i+3],  dev_list[index].m_Addr) )
                {
                    printf("=================================\n");
                    printf("#### MyAssignAddress(%d) OK! ####\n", dev_list[index].m_Addr);
                    printf("=================================\n");
                    dev_list[index].m_Device = -1;
                    dev_list[index].m_Err = 0;
                    dev_list[index].m_state = 1;
                    //dev_list[index].m_FWver = 0;
                    dev_list[index].m_ok_time = time(NULL);
                    if ( index == dev_count ) {
                        dev_count++;
                    }
                    cnt++;
                    i+=12;
                }
                else
                    printf("#### MyAssignAddress(%d) fail! ####\n", dev_list[dev_count].m_Addr);
            }
        }
    }

    return cnt;
}

int run_register()
{
    int DefaultMODValue = 3;
    int i, ret = 0, cnt = 0;
    dev_count = 0;
	bool Conflict = false;

	//time_t current_time;

    // initial parameter
    for (i=0; i<253; i++) {
        dev_list[i].m_Addr=i+1; // address range 1 ~ 253
        memset(dev_list[i].m_Sn, 0x00, 17);
        dev_list[i].m_Device = -2;
        dev_list[i].m_ok_time = 0;
        dev_list[i].m_Err = 0;
        dev_list[i].m_state = 0; // default offline
        //printf("i=%u, addr=%d\n",i,dev_list[i].m_Addr );
    }

	char byMOD = DefaultMODValue;
	//m_snCount = 0;

	//if (m_snCount==0) {
        RemoveRegisterQuery(com_fd, 0);
        CleanRespond();
        usleep(500000);
        RemoveRegisterQuery(com_fd, 0);
        CleanRespond();
        usleep(500000);
        RemoveRegisterQuery(com_fd, 0);
        CleanRespond();
        usleep(500000);
	//}

    //SaveLog((char *)"DataLogger StartRegisterProcess() : run", m_st_time);
    while ( 1 ) {
        ret = MySyncOffLineQuery(com_fd, 0x00, (byte)byMOD, query_buf, QUERY_SIZE);
        if ( ret > 0) {
            printf("#### MySyncOffLineQuery return %d ####\n", ret);
            printf("========================================= Debug date value =========================================");
            for ( i = 0; i < ret; i++ ) {
                if ( i%16 == 0 )
                    printf("\n 0x%04X ~ 0x%04X : ", i, i+15);
                printf("0x%02X ", query_buf[i]);
            }
            printf("\n====================================================================================================\n");
            goto Allocate_address;
        } else {
            //while ( m_snCount<253 && byMOD>=0 ) {
            while ( byMOD>=0 ) {
                ret = MyOffLineQuery(com_fd, 0x00, query_buf, QUERY_SIZE);

                // get time
                //current_time = time(NULL);
                //m_st_time = localtime(&current_time);
                // get data time not 0 min. current time is 0 min.
                //if ( m_data_st_time.tm_min != 0 )
                //    if ( m_st_time->tm_min == 0 )
                //        return -1;

                if ( ret != -1 ) {
                    printf("#### MyOffLineQuery return %d ####\n", ret);
                    printf("========================================= Debug date value =========================================");
                    for ( i = 0; i < ret; i++ ) {
                        if ( i%16 == 0 )
                            printf("\n 0x%04X ~ 0x%04X : ", i, i+15);
                        printf("0x%02X ", query_buf[i]);
                    }
                    printf("\n====================================================================================================\n");
                    if ( !CheckCRC(query_buf, 13) ) {
                        printf("#### Conflict! ####\n");
                        Conflict = true;
                        continue;
                    } else {
Allocate_address:
                        cnt = AllocateProcess(query_buf, ret);
                        if ( cnt == -1 )
                            return -1;
                        printf("#### AllocateProcess success = %d ####\n", cnt);
                        //dev_count += cnt;

                    }
                    usleep(1000000); // 1s
                }
                else
                    printf("#### MyOffLineQuery(%d) No response! ####\n", dev_list[dev_count].m_Addr);

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

    return dev_count;
    //return 0;
}

int get_device(int index)
{
    printf("#### GetDevice start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    //struct tm *log_time;
    // check ok time
    //time_t current_time = 0;
    //current_time = time(NULL);
    //if ( current_time - dev_list[index].m_ok_time >= OFFLINE_SECOND_MI ) {
    //    printf("Last m_ok_time more then 1200 sec.\n");
    //    if ( !ReRegister(index) )
    //        return false;
    //}

    //char buf[256] = {0};
    unsigned char szDevice[] = {0x00, 0x03, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00};
    szDevice[0] = dev_list[index].m_Addr;
    MakeReadDataCRC(szDevice,8);

    MClearRX();
    txsize=8;
    waitAddr = dev_list[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szDevice, 8);
        MStartTX(com_fd);
        //usleep(m_dl_config.m_delay_time_1);

        //current_time = time(NULL);
        //log_time = localtime(&current_time);

        lpdata = GetRespond(com_fd, 7, delay_time_1);
        if ( lpdata ) {
            printf("#### GetDevice OK ####\n");
            dev_list[index].m_Device = (*(lpdata+3) << 8) + *(lpdata+4);
            dev_list[index].m_ok_time = time(NULL);
            if ( dev_list[index].m_Device < 0x0A ) {
                printf("#### Address %d, Device 0x%04X ==> MI ####\n", dev_list[index].m_Addr, dev_list[index].m_Device);
                //sprintf(buf, "DataLogger GetDevice() : Address %d, Device 0x%04X ==> MI", dev_list[index].m_Addr, dev_list[index].m_Device);
                //SaveLog(buf, log_time);
            }
            else {
                printf("#### Address %d, Device 0x%04X ==> Hybrid ####\n", dev_list[index].m_Addr, dev_list[index].m_Device);
                //sprintf(buf, "DataLogger GetDevice() : Address %d, Device 0x%04X ==> Hybrid", dev_list[index].m_Addr, dev_list[index].m_Device);
                //SaveLog(buf, log_time);
            }
            return 0;
        } else {
            if ( have_respond == true ) {
                printf("#### GetDevice CRC Error ####\n");
                //SaveLog((char *)"DataLogger GetDevice() : CRC Error", log_time);
            }
            else {
                printf("#### GetDevice No Response ####\n");
                //SaveLog((char *)"DataLogger GetDevice() : No Response", log_time);
            }
            err++;
        }
    }

    return -1;
}

int write_Hybrid_list()
{
    int i = 0, flag = 0, model = 0;
    unsigned long long int dev_id = 0;
    char buf[256] = {0};
    FILE *pFile = NULL;

    sprintf(buf, "rm %s; sync", HYBRID_LIST_PATH);
    system(buf);

    for ( i = 0; i < dev_count; i++) {
        if ( dev_list[i].m_Device > 0x09 ) { // find Hybrid device
            // find first device
            if ( flag == 0 ) {
                pFile = fopen(HYBRID_LIST_PATH, "wb");
                if ( pFile == NULL ) {
                    printf("open %s fail\n", HYBRID_LIST_PATH);
                    return -1;
                }
                printf("==================== Set Hybrid List start ====================\n");
                fputs("<records>\n", pFile);
                flag = 1;
            }

            // set info for every device
            fputs("\t<record>\n", pFile);

            sprintf(buf, "\t\t<sn>%s</sn>\n", dev_list[i].m_Sn);
            printf("%s", buf);
            fputs(buf, pFile);

            sprintf(buf, "\t\t<OriSn>%s</OriSn>\n", dev_list[i].m_Sn);
            printf("%s", buf);
            fputs(buf, pFile);

            sprintf(buf, "\t\t<port>COM%d</port>\n", com_port);
            printf("%s", buf);
            fputs(buf, pFile);

            sprintf(buf, "\t\t<slaveId>%d</slaveId>\n", dev_list[i].m_Addr);
            printf("%s", buf);
            fputs(buf, pFile);

            fputs("\t\t<Manufacturer>DARFON</Manufacturer>\n", pFile);

            model = 0;
            sscanf(dev_list[i].m_Sn+6, "%02x", &model); // get 7-8 digit
            sprintf(buf, "\t\t<Model>%d</Model>\n", model);
            printf("%s", buf);
            fputs(buf, pFile);

            fputs("\t\t<OtherType>2</OtherType>\n", pFile);

            sscanf(dev_list[i].m_Sn+4, "%012llX", &dev_id); // get last 12 digit
            sprintf(buf, "\t\t<dev_id>%lld</dev_id>\n", dev_id);
            printf("%s", buf);
            fputs(buf, pFile);

            fputs("\t</record>\n", pFile);
        }
    }
    if ( flag ) {
        fputs("</records>", pFile);
        printf("===================== Set Hybrid List end =====================\n");
        fclose(pFile);
    }

    return 0;
}

int get_current_data()
{
    int i = 0, flag = 0, flag_err = 0, ret = 0, miss = 0;
    unsigned long long int dev_id = 0;
    char buf[256] = {0};
    FILE *pFile = NULL, *pFile_err = NULL;
    struct stat st;

    time_t current_time;
    struct tm *st_time;

    // get local time
    current_time = time(NULL);
    st_time = localtime(&current_time);
    //st_time.tm_year
	//st_time.tm_mon
	//st_time.tm_mday
	//st_time.tm_hour
	//st_time.tm_min
	//st_time.tm_sec

	sprintf(buf, "rm %s; sync", HYBRID_TMP_DATA_PATH);
    system(buf);
    sprintf(buf, "rm %s; sync", HYBRID_TMP_ERROR_PATH);
    system(buf);

    loop_err_cnt = 0;
    for ( i = 0; i < dev_count; i++) {
        if ( dev_list[i].m_Device > 0x09 ) { // find Hybrid device

            if ( dev_list[i].m_Err < 3 ) {
                // get data part
                // get id
                ret = get_id(i);
                if ( ret == 0 ) {
                    dev_list[i].m_Err = 0;
                } else {
                    if ( loop_err_cnt == 0 )
                        dev_list[i].m_Err++;
                    loop_err_cnt++;
                }
                // get rtc
                //ret = get_RTC(i);
                //if ( ret == 0 ) {
                //    dev_list[i].m_Err = 0;
                //} else {
                //    if ( loop_err_cnt == 0 )
                //        dev_list[i].m_Err++;
                //    loop_err_cnt++;
                //}
                // get remote setting
                ret = get_RS(i);
                if ( ret == 0 ) {
                    dev_list[i].m_Err = 0;
                } else {
                    if ( loop_err_cnt == 0 )
                        dev_list[i].m_Err++;
                    loop_err_cnt++;
                }
                // get remote real-time setting
                ret = get_RRS(i);
                if ( ret == 0 ) {
                    dev_list[i].m_Err = 0;
                } else {
                    if ( loop_err_cnt == 0 )
                        dev_list[i].m_Err++;
                    loop_err_cnt++;
                }
                // get real time info
                ret = get_RT(i);
                if ( ret == 0 ) {
                    dev_list[i].m_Err = 0;
                } else {
                    if ( loop_err_cnt == 0 )
                        dev_list[i].m_Err++;
                    loop_err_cnt++;
                }
                // get real time info
                ret = get_BMS(i);
                if ( ret == 0 ) {
                    dev_list[i].m_Err = 0;
                } else {
                    if ( loop_err_cnt == 0 )
                        dev_list[i].m_Err++;
                    loop_err_cnt++;
                }

                // set xml part
                // find first device, set start xml item
                if ( flag == 0 ) {
                    pFile = fopen(HYBRID_TMP_DATA_PATH, "wb");
                    if ( pFile == NULL ) {
                        printf("open %s fail\n", HYBRID_TMP_DATA_PATH);
                        return -1;
                    }
                    printf("==================== Set Hybrid Tmp Data start ====================\n");
                    fputs("<records>", pFile);
                    flag = 1;
                }
                // set header
                sscanf(dev_list[i].m_Sn+4, "%012llX", &dev_id); // get last 12 digit
                sprintf(buf, "<record dev_id=\"%lld\" date=\"%04d-%02d-%02d %02d:%02d:%02d\" sn=\"%s\">", dev_id,
                        1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday,
                        st_time->tm_hour, st_time->tm_min, st_time->tm_sec, dev_list[i].m_Sn);
                fputs(buf, pFile);

                // check all data ok, send(&save?) data
                if ( loop_err_cnt == 0 ) {
                // set ID part
                    // set grid voltage
                    sprintf(buf, "<Grid_Voltage>%d</Grid_Voltage>", id_data.Grid_Voltage);
                    fputs(buf, pFile);
                    // set model
                    sprintf(buf, "<Model>%d</Model>", id_data.Model);
                    fputs(buf, pFile);
                    // set hw ver
                    sprintf(buf, "<HWver>%d</HWver>", id_data.HW_Ver);
                    fputs(buf, pFile);
                    // set date
                    sprintf(buf, "<Product_Y>%04d</Product_Y>", id_data.Year);
                    fputs(buf, pFile);
                    sprintf(buf, "<Product_M>%02d</Product_M>", id_data.Month);
                    fputs(buf, pFile);
                    sprintf(buf, "<Product_D>%02d</Product_D>", id_data.Date);
                    fputs(buf, pFile);
                    // set version
                    sprintf(buf, "<Ver_HW>%d</Ver_HW>", id_data.Inverter_Ver);
                    fputs(buf, pFile);
                    sprintf(buf, "<Ver_FW>%d</Ver_FW>", id_data.DD_Ver);
                    fputs(buf, pFile);
                    sprintf(buf, "<Ver_EE>%d</Ver_EE>", id_data.EEPROM_Ver);
                    fputs(buf, pFile);
                    sprintf(buf, "<DisplayVer>%d</DisplayVer>", id_data.Display_Ver);
                    fputs(buf, pFile);
                    // set flags1
                    sprintf(buf, "<SystemFlag>%d</SystemFlag>", id_data.Flags1);
                    fputs(buf, pFile);
                    // set flags2
                    sprintf(buf, "<Rule_Flag>%d</Rule_Flag>", id_data.Flags2);
                    fputs(buf, pFile);

                // set remote setting
                    sprintf(buf, "<InverterMode>%d</InverterMode>", rs_info.Mode);
                    fputs(buf, pFile);
                    sprintf(buf, "<StarHour>%d</StarHour>", rs_info.StarHour);
                    fputs(buf, pFile);
                    sprintf(buf, "<StarMin>%d</StarMin>", rs_info.StarMin);
                    fputs(buf, pFile);
                    sprintf(buf, "<EndHour>%d</EndHour>", rs_info.EndHour);
                    fputs(buf, pFile);
                    sprintf(buf, "<EndMin>%d</EndMin>", rs_info.EndMin);
                    fputs(buf, pFile);
                    sprintf(buf, "<Multi_Module>%d</Multi_Module>", rs_info.MultiModuleSetting);
                    fputs(buf, pFile);
                    sprintf(buf, "<Battery_Type>%d</Battery_Type>", rs_info.BatteryType);
                    fputs(buf, pFile);
                    sprintf(buf, "<ChargeCurrent>%d</ChargeCurrent>", rs_info.BatteryCurrent);
                    fputs(buf, pFile);
                    sprintf(buf, "<BatShutdownVolt>%03.1f</BatShutdownVolt>", ((float)rs_info.BatteryShutdownVoltage)/10);
                    fputs(buf, pFile);
                    sprintf(buf, "<BatFloatingVolt>%03.1f</BatFloatingVolt>", ((float)rs_info.BatteryFloatingVoltage)/10);
                    fputs(buf, pFile);
                    sprintf(buf, "<BatReservePercent>%d</BatReservePercent>", rs_info.BatteryReservePercentage);
                    fputs(buf, pFile);
                    sprintf(buf, "<PeakShavingPower>%05.3f</PeakShavingPower>", ((float)rs_info.PeakShavingPower)/10);
                    fputs(buf, pFile);
                    sprintf(buf, "<StartFrequency>%03.1f</StartFrequency>", ((float)rs_info.StartFrequency)/10);
                    fputs(buf, pFile);
                    sprintf(buf, "<EndFrequency>%03.1f</EndFrequency>", ((float)rs_info.EndFrequency)/10);
                    fputs(buf, pFile);
                    sprintf(buf, "<FeedInPower>%05.3f</FeedInPower>", ((float)rs_info.FeedinPower)/10);
                    fputs(buf, pFile);

                // set remote real time setting
                    sprintf(buf, "<ChargeSetting>%d</ChargeSetting>", rrs_info.ChargeSetting);
                    fputs(buf, pFile);
                    sprintf(buf, "<ChargePower>%05.3f</ChargePower>", ((float)rrs_info.ChargePower)/10);
                    fputs(buf, pFile);
                    sprintf(buf, "<DischargePower>%05.3f</DischargePower>", ((float)rrs_info.DischargePower)/10);
                    fputs(buf, pFile);
                    // charge discharge power undefine in web server
                    sprintf(buf, "<RampRate>%d</RampRate>", rrs_info.RampRatePercentage);
                    fputs(buf, pFile);
                    sprintf(buf, "<Degree>%d</Degree>", rrs_info.DegreeLeadLag);
                    fputs(buf, pFile);
                    sprintf(buf, "<Q_Value>%d</Q_Value>", rrs_info.Volt_VAr);
                    fputs(buf, pFile);
                    // AC_Coupling_Power
                    sprintf(buf, "<AC_Coupling_Power>%d</AC_Coupling_Power>", rrs_info.AC_Coupling_Power);
                    fputs(buf, pFile);

                // set real time info
                    // set DC power (KW)
                    // set dc power, in range 0 ~ 999999.999 KW, max 0xFFFF = 65535 => 65.535 KW, must in range
                    sprintf(buf, "<dc_power>%05.3f</dc_power>", ((float)rt_info.PV_Total_Power)/1000);
                    fputs(buf, pFile);
                    sprintf(buf, "<dc_power_1>%05.3f</dc_power_1>", ((float)rt_info.PV1_Power)/1000);
                    fputs(buf, pFile);
                    sprintf(buf, "<dc_power_2>%05.3f</dc_power_2>", ((float)rt_info.PV2_Power)/1000);
                    fputs(buf, pFile);
                    // set DC voltage (V)
                    // set dc voltage, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 65535 V
                    if ( rt_info.PV1_Voltage < 10000 ) {
                        sprintf(buf, "<dcv_1>%d</dcv_1>", rt_info.PV1_Voltage);
                        fputs(buf, pFile);
                    }
                    if ( rt_info.PV2_Voltage < 10000 ) {
                        sprintf(buf, "<dcv_2>%d</dcv_2>", rt_info.PV2_Voltage);
                        fputs(buf, pFile);
                    }
                    if ( ((rt_info.PV1_Voltage + rt_info.PV2_Voltage)/2) < 10000 ) {
                        sprintf(buf, "<dc_voltage>%03.1f</dc_voltage>", ((float)(rt_info.PV1_Voltage + rt_info.PV2_Voltage))/2);
                        fputs(buf, pFile);
                    }
                    // set DC current (A)
                    // set dc current, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 655.35 A, must in range
                    sprintf(buf, "<dci_1>%04.2f</dci_1>", ((float)rt_info.PV1_Current)/100);
                    fputs(buf, pFile);
                    sprintf(buf, "<dci_2>%04.2f</dci_2>", ((float)rt_info.PV2_Current)/100);
                    fputs(buf, pFile);
                    sprintf(buf, "<dc_current>%d</dc_current>", rt_info.PV1_Current + rt_info.PV2_Current);
                    fputs(buf, pFile);

                    // set AC power (KW)
                    // set ac power, in range 0 ~ 999999.999 KW, max 0xFFFF = 65535 => 65.535 KW, must in range
                    sprintf(buf, "<ac_power_A>%05.3f</ac_power_A>", ((float)rt_info.Load_Power)/1000);
                    fputs(buf, pFile);
                    sprintf(buf, "<ac_power>%05.3f</ac_power>", ((float)rt_info.Load_Power)/1000);
                    fputs(buf, pFile);
                    // set AC voltage (V)
                    // set ac voltage, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 65535 V
                    if ( rt_info.Load_Voltage < 10000 ) {
                        sprintf(buf, "<acv_AN>%d</acv_AN>", rt_info.Load_Voltage);
                        fputs(buf, pFile);
                        sprintf(buf, "<ac_voltage>%d</ac_voltage>", rt_info.Load_Voltage);
                        fputs(buf, pFile);
                    }
                    // set AC current (A)
                    // set ac current, in range 0 ~ 9999.99 V, max 0xFFFF = 65535 => 655.35 A, must in range
                    sprintf(buf, "<aci_A>%04.2f</aci_A>", ((float)rt_info.Load_Current)/100);
                    fputs(buf, pFile);
                    sprintf(buf, "<ac_current>%04.2f</ac_current>", ((float)rt_info.Load_Current)/100);
                    fputs(buf, pFile);

                    // set total power
                    // set total KWH, in range 0 ~ 999999999999999.999 KWH, max 0xFFFFFFFF => ‭‭6553599.99?, anyway must in range
                    sprintf(buf, "<total_KWH>%04.2f</total_KWH>", rt_info.PV_Total_EnergyH*100 + ((float)rt_info.PV_Total_EnergyL)*0.01);
                    fputs(buf, pFile);

                    // set battery SOC
                    // set soc, in range 0 ~ 9999.99 %, max 0xFFFF = > 65535
                    if ( rt_info.Battery_SOC < 10000 ) {
                        sprintf(buf, "<soc>%d</soc>", rt_info.Battery_SOC);
                        fputs(buf, pFile);
                    }

                    // set temperature
                    sprintf(buf, "<Inv_Temp>%03.1f</Inv_Temp>", ((float)rt_info.Inv_Temp)/10);
                    fputs(buf, pFile);
                    sprintf(buf, "<PV1_Temp>%03.1f</PV1_Temp>", ((float)rt_info.PV1_Temp)/10);
                    fputs(buf, pFile);
                    sprintf(buf, "<PV2_Temp>%03.1f</PV2_Temp>", ((float)rt_info.PV2_Temp)/10);
                    fputs(buf, pFile);
                    sprintf(buf, "<DD_Temp>%03.1f</DD_Temp>", ((float)rt_info.DD_Temp)/10);
                    fputs(buf, pFile);

                    // set Grid
                    sprintf(buf, "<VGrid_A>%d</VGrid_A>", rt_info.Grid_Voltage);
                    fputs(buf, pFile);
                    sprintf(buf, "<IGrid_A>%04.2f</IGrid_A>", ((float)rt_info.Grid_Current)/100);
                    fputs(buf, pFile);
                    sprintf(buf, "<PGrid_A>%05.3f</PGrid_A>", ((float)rt_info.Grid_Power)/1000);
                    fputs(buf, pFile);

                    // set battery
                    sprintf(buf, "<VBattery>%03.1f</VBattery>", ((float)rt_info.Battery_Voltage)/10);
                    fputs(buf, pFile);
                    sprintf(buf, "<IBattery>%03.1f</IBattery>", ((float)rt_info.Battery_Current)/10);
                    fputs(buf, pFile);
                    sprintf(buf, "<PBattery>%05.3f</PBattery>", ((float)rt_info.PBat)/1000);
                    fputs(buf, pFile);


                    // set bus
                    sprintf(buf, "<Vbus>%03.1f</Vbus>", ((float)rt_info.Bus_Voltage)/10);
                    fputs(buf, pFile);
                    sprintf(buf, "<Ibus>%03.1f</Ibus>", ((float)rt_info.Bus_Current)/10);
                    fputs(buf, pFile);

                    // set battery power
                    sprintf(buf, "<Pbat_Total>%04.2f</Pbat_Total>", rt_info.Bat_Total_EnergyH*100 + ((float)rt_info.Bat_Total_EnergyL)*0.01);
                    fputs(buf, pFile);

                    // set load power
                    sprintf(buf, "<Pload_Total>%04.2f</Pload_Total>", rt_info.Load_Total_EnergyH*100 + ((float)rt_info.Load_Total_EnergyL)*0.01);
                    fputs(buf, pFile);

                    // set grid feed power
                    sprintf(buf, "<GridFeed_Total>%04.2f</GridFeed_Total>", rt_info.GridFeed_TotalH*100 + ((float)rt_info.GridFeed_TotalL)*0.01);
                    fputs(buf, pFile);

                    // set grid charge power
                    sprintf(buf, "<GridCharge_Total>%04.2f</GridCharge_Total>", rt_info.GridCharge_TotalH*100 + ((float)rt_info.GridCharge_TotalL)*0.01);
                    fputs(buf, pFile);

                    // set external power
                    sprintf(buf, "<ExtPower>%05.3f</ExtPower>", ((float)rt_info.External_Power)/10);
                    fputs(buf, pFile);

                    // set system state
                    sprintf(buf, "<Sys_State>%d</Sys_State>", rt_info.Sys_State);
                    fputs(buf, pFile);

                    // set Icon
                    sprintf(buf, "<Hybrid_Icon>%d</Hybrid_Icon>", (rt_info.Hybrid_IconH << 16) + rt_info.Hybrid_IconL);
                    fputs(buf, pFile);

                    // set error code
                    sprintf(buf, "<Error_Code>%d</Error_Code>", rt_info.Error_Code);
                    fputs(buf, pFile);

                    // set frequency, in range 0 ~ 999.9 Hz, max 0xFFFF = 65535 => 6553.5 Hz
                    if ( rt_info.Invert_Frequency < 10000 ) {
                        sprintf(buf, "<Inverterfrequency>%03.1f</Inverterfrequency>", ((float)rt_info.Invert_Frequency)/10);
                        fputs(buf, pFile);
                    }
                    // set frequency, in range 0 ~ 999.99 Hz, max 0xFFFF = 65535 => 6553.5 Hz
                    if ( rt_info.Grid_Frequency < 10000 ) {
                        sprintf(buf, "<frequency>%03.1f</frequency>", ((float)rt_info.Grid_Frequency)/10);
                        fputs(buf, pFile);
                    }

                // set BMS info
                    sprintf(buf, "<BMS_Voltage>%04.2f</BMS_Voltage>", ((float)bms_info.Voltage/100));
                    fputs(buf, pFile);
                    sprintf(buf, "<BMS_Current>%04.2f</BMS_Current>", ((float)bms_info.Current/100));
                    fputs(buf, pFile);
                    sprintf(buf, "<BMS_SOC>%d</BMS_SOC>", bms_info.SOC);
                    fputs(buf, pFile);
                    sprintf(buf, "<BMS_Max_Temp>%d</BMS_Max_Temp>", bms_info.MaxTemperature);
                    fputs(buf, pFile);
                    sprintf(buf, "<BMS_CycleCount>%d</BMS_CycleCount>", bms_info.CycleCount);
                    fputs(buf, pFile);
                    sprintf(buf, "<BMS_Status>%d</BMS_Status>", bms_info.Status);
                    fputs(buf, pFile);
                    sprintf(buf, "<BMS_Error>%d</BMS_Error>", bms_info.Error);
                    fputs(buf, pFile);
                    sprintf(buf, "<BMS_ModuleNo>%d</BMS_ModuleNo>", bms_info.Number);
                    fputs(buf, pFile);
                    sprintf(buf, "<BMS_Info>%d</BMS_Info>", bms_info.BMS_Info);
                    fputs(buf, pFile);
                    sprintf(buf, "<BMS_MaxCell>%05.3f</BMS_MaxCell>", ((float)bms_info.BMS_Max_Cell/1000));
                    fputs(buf, pFile);
                    sprintf(buf, "<BMS_MinCell>%05.3f</BMS_MinCell>", ((float)bms_info.BMS_Min_Cell/1000));
                    fputs(buf, pFile);
                    sprintf(buf, "<BMS_BaudRate>%d</BMS_BaudRate>", bms_info.BMS_BaudRate);
                    fputs(buf, pFile);

                } else {  // miss data, do not save
                    printf("Addr %d SN %s missing part of data\n", dev_list[i].m_Addr, dev_list[i].m_Sn);
                    miss++;
                }

                // set status
                if ( rt_info.Error_Code || rt_info.PV_Inv_Error_COD1_Record || rt_info.PV_Inv_Error_COD2_Record || rt_info.PV_Inv_Error_COD3_Record ||
                    rt_info.DD_Error_COD_Record || rt_info.DD_Error_COD2_Record ) {
                    fputs("<Status>2</Status>", pFile);
                } else {
                    if ( loop_err_cnt == 5 ) {
                        fputs("<Status>1</Status>", pFile);
                        inverter_state = 1;
                        //m_sys_error |= SYS_0010_Off_Line;
                    } else {
                        fputs("<Status>0</Status>", pFile);
                        inverter_state = 0;
                        //m_sys_error &= ~SYS_0010_Off_Line;
                    }
                }

                fputs("</record>", pFile);


                if ( rt_info.Error_Code || rt_info.PV_Inv_Error_COD1_Record || rt_info.PV_Inv_Error_COD2_Record || rt_info.PV_Inv_Error_COD3_Record ||
                         rt_info.DD_Error_COD_Record || rt_info.DD_Error_COD2_Record || inverter_state || no_usb ) {
                    // set err xml part
                    // find first device, set start xml item
                    if ( flag_err == 0 ) {
                        pFile_err = fopen(HYBRID_TMP_ERROR_PATH, "wb");
                        if ( pFile_err == NULL ) {
                            printf("open %s fail\n", HYBRID_TMP_ERROR_PATH);
                            return -1;
                        }
                        printf("==================== Set Hybrid Tmp Error start ====================\n");
                        fputs("<records>", pFile_err);
                        flag_err = 1;
                    }
                    // set header
                    sscanf(dev_list[i].m_Sn+4, "%012llX", &dev_id); // get last 12 digit
                    sprintf(buf, "<record dev_id=\"%lld\" date=\"%04d-%02d-%02d %02d:%02d:%02d\" sn=\"%s\">", dev_id,
                            1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday,
                            st_time->tm_hour, st_time->tm_min, st_time->tm_sec, dev_list[i].m_Sn);
                    fputs(buf, pFile_err);

                    // 0xDB : error code
                    if ( rt_info.Error_Code ) {
                        sprintf(buf, "<code>%d</code>", rt_info.Error_Code);
                        fputs(buf, pFile_err);
                    }
                    // PV_Inv_Error_COD1_Record 0xD3
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x0001 )
                        fputs("<code>COD1_0001_Fac_HL</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x0002 )
                        fputs("<code>COD1_0002_CanBus_Fault</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x0004 )
                        fputs("<code>COD1_0004_Islanding</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x0008 )
                        fputs("<code>COD1_0008_Vac_H</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x0010 )
                        fputs("<code>COD1_0010_Vac_L</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x0020 )
                        fputs("<code>COD1_0020_Fac_H</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x0040 )
                        fputs("<code>COD1_0040_Fac_L</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x0080 )
                        fputs("<code>COD1_0080_Fac_LL</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x0100 )
                        fputs("<code>COD1_0100_Vac_OCP</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x0200 )
                        fputs("<code>COD1_0200_Vac_HL</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x0400 )
                        fputs("<code>COD1_0400_Vac_LL</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x0800 )
                        fputs("<code>COD1_0800_OPP</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x1000 )
                        fputs("<code>COD1_1000_Iac_H</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x2000 )
                        fputs("<code>COD1_2000_Ipv_H</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x4000 )
                        fputs("<code>COD1_4000_ADCINT_OVF</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD1_Record & 0x8000 )
                        fputs("<code>COD1_8000_Vbus_H</code>", pFile_err);
                    // PV_Inv_Error_COD2_Record 0xD4
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x0001 )
                        fputs("<code>COD2_0001_Arc</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x0002 )
                        fputs("<code>COD2_0002_Vac_Relay_fault</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x0004 )
                        fputs("<code>COD2_0004_Ipv1_short</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x0008 )
                        fputs("<code>COD2_0008_Ipv2_short</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x0010 )
                        fputs("<code>COD2_0010_Vac_Short</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x0020 )
                        fputs("<code>COD2_0020_CT_fault</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x0040 )
                        fputs("<code>COD2_0040_PVOverPower</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x0080 )
                        fputs("<code>COD2_0080_NO_GRID</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x0100 )
                        fputs("<code>COD2_0100_PV_Input_High</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x0200 )
                        fputs("<code>COD2_0200_INV_Overload</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x0400 )
                        fputs("<code>COD2_0400_RCMU_30</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x0800 )
                        fputs("<code>COD2_0800_RCMU_60</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x1000 )
                        fputs("<code>COD2_1000_RCMU_150</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x2000 )
                        fputs("<code>COD2_2000_RCMU_300</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x4000 )
                        fputs("<code>COD2_4000_RCMUtest_Fault</code>", pFile_err);
                    if ( rt_info.PV_Inv_Error_COD2_Record & 0x8000 )
                        fputs("<code>COD2_8000_Vac_LM</code>", pFile_err);
                    // DD_Error_COD_Record 0xD5
                    if ( rt_info.DD_Error_COD_Record & 0x0001 )
                        fputs("<code>COD3_0001_Vbat_H</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x0002 )
                        fputs("<code>COD3_0002_Vbat_L_fault</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x0004 )
                        fputs("<code>COD3_0004_Vbus_H</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x0008 )
                        fputs("<code>COD3_0008_Vbus_L</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x0010 )
                        fputs("<code>COD3_0010_Ibus_H</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x0020 )
                        fputs("<code>COD3_0020_Ibat_H</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x0040 )
                        fputs("<code>COD3_0040_Charger_T</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x0080 )
                        fputs("<code>COD3_0080_Code</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x0100 )
                        fputs("<code>COD3_0100_Vbat_Drop</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x0200 )
                        fputs("<code>COD3_0200_INV_fault</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x0400 )
                        fputs("<code>COD3_0400_GND_Fault</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x0800 )
                        fputs("<code>COD3_0800_No_Bat</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x1000 )
                        fputs("<code>COD3_1000_BMS_Comute_fault</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x2000 )
                        fputs("<code>COD3_2000_BMS_Over_Current</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x4000 )
                        fputs("<code>COD3_4000_Restart</code>", pFile_err);
                    if ( rt_info.DD_Error_COD_Record & 0x8000 )
                        fputs("<code>COD3_8000_Bat_Setting_fault</code>", pFile_err);
                    // PV_Inv_Error_COD3_Record 0xF0
                    if ( rt_info.PV_Inv_Error_COD3_Record & 0x0001 )
                        fputs("<code>COD4_0001_External_PV_OPP</code>", pFile_err);
                    // DD_Error_COD2_Record 0xF1
                    if ( rt_info.DD_Error_COD2_Record & 0x0001 )
                        fputs("<code>COD5_0001_EEProm_Fault</code>", pFile_err);
                    if ( rt_info.DD_Error_COD2_Record & 0x0002 )
                        fputs("<code>COD5_0002_Communi_Fault</code>", pFile_err);
                    if ( rt_info.DD_Error_COD2_Record & 0x0004 )
                        fputs("<code>COD5_0004_OT_Fault</code>", pFile_err);
                    if ( rt_info.DD_Error_COD2_Record & 0x0008 )
                        fputs("<code>COD5_0008_Fan_Fault</code>", pFile_err);
                    if ( rt_info.DD_Error_COD2_Record & 0x0010 )
                        fputs("<code>COD5_0010_Low_Battery</code>", pFile_err);
                    // set inverter off line log
                    if ( inverter_state )
                        fputs("<code>SYS_0010_Off_Line</code>", pFile_err);
                    // set no usb
                    if ( no_usb )
                        fputs("<code>SYS_0001_No_USB</code>", pFile_err);

                    fputs("</record>", pFile_err);
                }

            } else {
                printf("Addr %d SN %s Error 3 times, call re_register()\n", dev_list[i].m_Addr, dev_list[i].m_Sn);
                // set state to offline
                dev_list[i].m_state = 0; // offline
                re_register(i);
            }
        }
    }
    if ( flag ) {
        fputs("</records>", pFile);
        printf("===================== Set Hybrid Tmp Data end =====================\n");
        fclose(pFile);
    }
    if ( flag_err ) {
        fputs("</records>", pFile_err);
        printf("===================== Set Hybrid Tmp Error end =====================\n");
        fclose(pFile_err);
    }

    if ( miss )
        return miss;
    else {
        // file exist
        if ( stat(HYBRID_TMP_DATA_PATH, &st) == 0 ) {
            // save file if time is up
            if ( current_time - save_time >= update_interval ) {
                save_time = current_time;
                // copy file
                if ( no_usb ) {
                    // check dir exist, or make dir
                    sprintf(buf, "%s/%04d%02d%02d", DEF_LOG_PATH, 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday);
                    if ( stat(buf, &st) != 0 ) {
                        sprintf(buf, "mkdir -p %s/%04d%02d%02d", DEF_LOG_PATH, 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday);
                        system(buf);
                    }
                    sprintf(buf, "cp %s %s/%04d%02d%02d/%02d%02d%02d", HYBRID_TMP_DATA_PATH, DEF_LOG_PATH, 1900+st_time->tm_year, 1+st_time->tm_mon,
                        st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
                    system(buf);

                    // copy error code if exist
                    if ( stat(HYBRID_TMP_ERROR_PATH, &st) == 0 ) {
                        // check dir exist, or make dir
                        sprintf(buf, "%s/%04d%02d%02d", DEF_ERRLOG_PATH, 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday);
                        if ( stat(buf, &st) != 0 ) {
                            sprintf(buf, "mkdir -p %s/%04d%02d%02d", DEF_ERRLOG_PATH, 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday);
                            system(buf);
                        }
                        sprintf(buf, "cp %s %s/%04d%02d%02d/%02d%02d%02d", HYBRID_TMP_ERROR_PATH, DEF_ERRLOG_PATH, 1900+st_time->tm_year, 1+st_time->tm_mon,
                            st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
                        system(buf);
                    }
                } else {
                    // check dir exist, or make dir
                    sprintf(buf, "%s/%04d%02d%02d", LOG_PATH, 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday);
                    if ( stat(buf, &st) != 0 ) {
                        sprintf(buf, "mkdir -p %s/%04d%02d%02d", LOG_PATH, 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday);
                        system(buf);
                    }
                    sprintf(buf, "cp %s %s/%04d%02d%02d/%02d%02d%02d", HYBRID_TMP_DATA_PATH, LOG_PATH, 1900+st_time->tm_year, 1+st_time->tm_mon,
                        st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
                    system(buf);

                    // copy error code if exist
                    if ( stat(HYBRID_TMP_ERROR_PATH, &st) == 0 ) {
                        // check dir exist, or make dir
                        sprintf(buf, "%s/%04d%02d%02d", ERRLOG_PATH, 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday);
                        if ( stat(buf, &st) != 0 ) {
                            sprintf(buf, "mkdir -p %s/%04d%02d%02d", ERRLOG_PATH, 1900+st_time->tm_year, 1+st_time->tm_mon, st_time->tm_mday);
                            system(buf);
                        }
                        sprintf(buf, "cp %s %s/%04d%02d%02d/%02d%02d%02d", HYBRID_TMP_ERROR_PATH, ERRLOG_PATH, 1900+st_time->tm_year, 1+st_time->tm_mon,
                            st_time->tm_mday, st_time->tm_hour, st_time->tm_min, st_time->tm_sec);
                        system(buf);
                    }
                }
            }
        }
        return 0;
    }
}

int re_register(int index)
{
    // send remove the slave address, send MySyncOffLineQuery, send MyAssignAddress
    unsigned int tmp[8] = {0};
    unsigned char buffer[9] = {0};
    //int MOD = 20, ret = 0, i;
    int i = 0;
    //char buf[256] = {0};

    //sprintf(buf, "DataLogger re_register() : addr %d run", dev_list[index].m_Addr);
    //SaveLog(buf, m_st_time);
    printf("#### re_register start ####\n");
    printf("#### Remove %d Query ####\n", dev_list[index].m_Addr);
    RemoveRegisterQuery(com_fd, dev_list[index].m_Addr);
    usleep(500000);

    printf("re_register SN = %s\n", dev_list[index].m_Sn);
    for (i = 0; i < 8; i++) {
        sscanf(dev_list[index].m_Sn+2*i, "%02X", &tmp[i]);
        buffer[i] = (unsigned char)tmp[i];
        //printf("buffer[%d] = %02X\n", i, buffer[i]);
    }

    if ( MyAssignAddress(com_fd, buffer, dev_list[index].m_Addr) )
    {
        //sprintf(buf, "DataLogger re_register() : addr %d OK", dev_list[index].m_Addr);
        //SaveLog(buf, m_st_time);
        printf("=================================\n");
        printf("#### re_register(%d) OK! ####\n", dev_list[index].m_Addr);
        printf("=================================\n");
        //dev_list[index].m_Device = -1; // device not change
        dev_list[index].m_Err = 0;
        dev_list[index].m_state = 1; // set online
        dev_list[index].m_ok_time = time(NULL);
        return 0;
    }
    else {
        printf("#### re_register(%d) fail! ####\n", dev_list[index].m_Addr);
        //sprintf(buf, "DataLogger re_register() : addr %d fail", dev_list[index].m_Addr);
        //SaveLog(buf, m_st_time);
    }

    return -1;
}

int get_id(int index)
{
    printf("#### get_id start ####\n");

    memset(&id_data, 0x00, sizeof(id_data));
    memset(&id_flags1, 0x00, sizeof(id_flags1));
    memset(&id_flags2, 0x00, sizeof(id_flags2));

    int err = 0;
    byte *lpdata = NULL;

    //struct tm *log_time;
    // check ok time
    time_t current_time = 0;
    current_time = time(NULL);
    if ( current_time - dev_list[index].m_ok_time >= OFFLINE_SECOND_HB ) {
        printf("Last m_ok_time more then 180 sec.\n");
        if ( re_register(index) )
            return -1;
    }

    unsigned char szHBIDdata[]={0x00, 0x03, 0x00, 0x01, 0x00, 0x0E, 0x00, 0x00};

    // set buf
    // slave id
    szHBIDdata[0]=dev_list[index].m_Addr;
    // function code
    //szHBIDdata[1]=0x03;
    // start address 2 byte
    //szHBIDdata[2]=0x00;
    //szHBIDdata[3]=0x01;
    // no, of data 2 byte
    //szHBIDdata[4]=0x00;
    //szHBIDdata[5]=0x0E;
    // crc
    MakeReadDataCRC(szHBIDdata,8);

    MClearRX();
    txsize=8;
    waitAddr = dev_list[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBIDdata, 8);
        MStartTX(com_fd);
        //usleep(delay_time_2);

        //current_time = time(NULL);
		//log_time = localtime(&current_time);

        lpdata = GetRespond(com_fd, 33, delay_time_2);
        if ( lpdata ) {
            printf("#### get_id OK ####\n");
            //SaveLog((char *)"DataLogger get_id() : OK", log_time);
            dump_id(lpdata+3);
            dev_list[index].m_ok_time = time(NULL);
            parser_id_flags1(id_data.Flags1);
            parser_id_flags2(id_data.Flags2);
            return 0;
        } else {
            if ( have_respond == true ) {
                printf("#### get_id CRC Error ####\n");
                //SaveLog((char *)"DataLogger get_id() : CRC Error", log_time);
            }
            else {
                printf("#### get_id No Response ####\n");
                //SaveLog((char *)"DataLogger get_id() : No Response", log_time);
                //SaveLog((char *)"DataLogger get_id() : run reregister()", log_time);
                re_register(index);
            }
            err++;
        }
    }

    return -2;
}

void dump_id(unsigned char *buf)
{
    id_data.Grid_Voltage = (*(buf) << 8) + *(buf+1);
    id_data.HW_Ver = *(buf+2);
    id_data.Model = *(buf+3);
    //id_data.Model = (*(buf+2) << 8) + *(buf+3);
    id_data.SN_Hi = (*(buf+4) << 8) + *(buf+5);
    id_data.SN_Lo = (*(buf+6) << 8) + *(buf+7);
    id_data.Year = (*(buf+8) << 8) + *(buf+9);
    id_data.Month = (*(buf+10) << 8) + *(buf+11);
    id_data.Date = (*(buf+12) << 8) + *(buf+13);
    id_data.Inverter_Ver = (*(buf+14) << 8) + *(buf+15);
    id_data.DD_Ver = (*(buf+16) << 8) + *(buf+17);
    id_data.EEPROM_Ver = (*(buf+18) << 8) + *(buf+19);
    id_data.Display_Ver = (*(buf+20) << 8) + *(buf+21);
    id_data.Flags1 = (*(buf+24) << 8) + *(buf+25);
    id_data.Flags2 = (*(buf+26) << 8) + *(buf+27);

    printf("#### Dump ID Data ####\n");
    printf("Grid_Voltage = %d ==> ", id_data.Grid_Voltage);
    switch (id_data.Grid_Voltage)
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
            printf("full range(208~240V)\n");
            break;
    }
    printf("Model        = %d ==> ", id_data.Model);
    switch (id_data.Model)
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
    printf("HW_Ver       = 0x%02X\n", id_data.HW_Ver);
    printf("SN_Hi        = 0x%04X\n", id_data.SN_Hi);
    printf("SN_Lo        = 0x%04X\n", id_data.SN_Lo);
    printf("Year         = %d\n", id_data.Year);
    printf("Month        = %02d\n", id_data.Month);
    printf("Date         = %02d\n", id_data.Date);
    printf("Inverter_Ver = 0x%04X ==> ", id_data.Inverter_Ver);
    if ( id_data.Inverter_Ver < 0x0A )
        printf("MI\n");
    else
        printf("Hybrid\n");
    printf("DD_Ver       = %d\n", id_data.DD_Ver);
    printf("EEPROM_Ver   = %d\n", id_data.EEPROM_Ver);
    printf("Display_Ver  = %d\n", id_data.Display_Ver);
    printf("Flags1        = 0x%02X ==> \n", id_data.Flags1);
    printf("Flags2        = 0x%02X ==> \n", id_data.Flags2);
    printf("#############################\n");
}

//int set_id(int index);

void parser_id_flags1(int flags)
{
    int tmp = flags;

    id_flags1.B0B1_External_Sensor = tmp & 0x03;
    //tmp>>=2;

    printf("#### Parser ID Flags1 ####\n");
    printf("Bit0Bit1 : External Sensor = %d\n", id_flags1.B0B1_External_Sensor);
    printf("################################\n");
}

void parser_id_flags2(int flags)
{
    int tmp = flags;

    id_flags2.B0_Rule21 = tmp & 0x01;
    tmp>>=1;
    id_flags2.B1_PVParallel = tmp & 0x01;
    tmp>>=1;
    id_flags2.B2_PVOffGrid = tmp & 0x01;
    tmp>>=1;
    id_flags2.B3_Heco1 = tmp & 0x01;
    tmp>>=1;
    id_flags2.B4_Heco2 = tmp & 0x01;
    tmp>>=1;
    id_flags2.B5_ACCoupling = tmp & 0x01;
    tmp>>=1;
    id_flags2.B6_FreControl = tmp & 0x01;
    tmp>>=1;
    id_flags2.B7_ArcDetection = tmp & 0x01;
    tmp>>=1;
    id_flags2.B8_PREPA = tmp & 0x01;
    tmp>>=1;
    id_flags2.B9_Self_Supply = tmp & 0x01;
    tmp>>=1;
    id_flags2.B10_Charge_only_from_PV = tmp & 0x01;
    tmp>>=1;
    id_flags2.B11_Dominion = tmp & 0x01;

    printf("#### Parser ID Flags2 ####\n");
    printf("Bit0 : Rule21              = %d\n", id_flags2.B0_Rule21);
    printf("Bit1 : PV Parallel         = %d\n", id_flags2.B1_PVParallel);
    printf("Bit2 : PV Off Grid         = %d\n", id_flags2.B2_PVOffGrid);
    printf("Bit3 : Heco1               = %d\n", id_flags2.B3_Heco1);
    printf("Bit4 : Heco2               = %d\n", id_flags2.B4_Heco2);
    printf("Bit5 : AC Coupling         = %d\n", id_flags2.B5_ACCoupling);
    printf("Bit6 : Fre Control         = %d\n", id_flags2.B6_FreControl);
    printf("Bit7 : Arc Detection       = %d\n", id_flags2.B7_ArcDetection);
    printf("Bit8 : PREPA               = %d\n", id_flags2.B8_PREPA);
    printf("Bit9 : Self Supply         = %d\n", id_flags2.B9_Self_Supply);
    printf("Bit10: Charge only from PV = %d\n", id_flags2.B10_Charge_only_from_PV);
    printf("Bit11: Dominion            = %d\n", id_flags2.B11_Dominion);
    printf("################################\n");
}

int get_RTC(int index)
{
    printf("#### get_RTC start ####\n");

    memset(&rtc_data, 0x00, sizeof(rtc_data));

    int err = 0;
    byte *lpdata = NULL;
    //time_t current_time;
    //struct tm *log_time;

    unsigned char szHBRTCdata[]={0x00, 0x03, 0x00, 0x40, 0x00, 0x06, 0x00, 0x00};
    szHBRTCdata[0]=dev_list[index].m_Addr;
    MakeReadDataCRC(szHBRTCdata,8);

    MClearRX();
    txsize=8;
    waitAddr = dev_list[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBRTCdata, 8);
        MStartTX(com_fd);
        //usleep(delay_time_2);

        //current_time = time(NULL);
		//log_time = localtime(&current_time);

        lpdata = GetRespond(com_fd, 17, delay_time_2);
        if ( lpdata ) {
            printf("#### get_RTC OK ####\n");
            //SaveLog((char *)"DataLogger get_RTC() : OK", log_time);
            dev_list[index].m_ok_time = time(NULL);
            dump_RTC(lpdata+3);
            return 0;
        } else {
            if ( have_respond == true ) {
                printf("#### get_RTC CRC Error ####\n");
                //SaveLog((char *)"DataLogger get_RTC() : CRC Error", log_time);
            }
            else {
                printf("#### get_RTC No Response ####\n");
                //SaveLog((char *)"DataLogger get_RTC() : No Response", log_time);
            }
            err++;
        }
    }

    return -1;
}

void dump_RTC(unsigned char *buf)
{
    rtc_data.Second = (*(buf) << 8) + *(buf+1);
    rtc_data.Minute = (*(buf+2) << 8) + *(buf+3);
    rtc_data.Hour = (*(buf+4) << 8) + *(buf+5);
    rtc_data.Date = (*(buf+6) << 8) + *(buf+7);
    rtc_data.Month = (*(buf+8) << 8) + *(buf+9);
    rtc_data.Year = (*(buf+10) << 8) + *(buf+11);

    printf("#### Dump Hybrid RTC Data ####\n");
    printf("Second = %d\n", rtc_data.Second);
    printf("Minute = %d\n", rtc_data.Minute);
    printf("Hour   = %d\n", rtc_data.Hour);
    printf("Date   = %d\n", rtc_data.Date);
    printf("Month  = %d\n", rtc_data.Month);
    printf("Year   = %d\n", rtc_data.Year);
    printf("##############################\n");
    printf("rtc time : %4d/%02d/%02d ", rtc_data.Year, rtc_data.Month, rtc_data.Date);
    printf("%02d:%02d:%02d\n", rtc_data.Hour, rtc_data.Minute, rtc_data.Second);
    printf("##############################\n");
}

//int set_RTC(int index);

int get_RS(int index)
{
    printf("#### get_RS start ####\n");

    memset(&rs_info, 0x00, sizeof(rs_info));

    int err = 0;
    byte *lpdata = NULL;
    //time_t current_time;
	//struct tm *log_time;

    unsigned char szHBRSinfo[]={0x00, 0x03, 0x00, 0x90, 0x00, 0x0F, 0x00, 0x00};
    szHBRSinfo[0]=dev_list[index].m_Addr;
    MakeReadDataCRC(szHBRSinfo,8);

    MClearRX();
    txsize=8;
    waitAddr = dev_list[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBRSinfo, 8);
        MStartTX(com_fd);
        //usleep(delay_time_2);

        //current_time = time(NULL);
		//log_time = localtime(&current_time);

        lpdata = GetRespond(com_fd, 35, delay_time_2);
        if ( lpdata ) {
            printf("#### get_RS OK ####\n");
            //SaveLog((char *)"DataLogger get_RS() : OK", log_time);
            dev_list[index].m_ok_time = time(NULL);
            dump_RS(lpdata+3);
            return 0;
        } else {
            if ( have_respond == true ) {
                printf("#### get_RS CRC Error ####\n");
                //SaveLog((char *)"DataLogger get_RS() : CRC Error", log_time);
            }
            else {
                printf("#### get_RS No Response ####\n");
                //SaveLog((char *)"DataLogger get_RS() : No Response", log_time);
            }
            err++;
        }
    }

    return -1;
}

void dump_RS(unsigned char *buf)
{
    rs_info.Mode = (*(buf) << 8) + *(buf+1);
    rs_info.StarHour = (*(buf+2) << 8) + *(buf+3);
    rs_info.StarMin = (*(buf+4) << 8) + *(buf+5);
    rs_info.EndHour = (*(buf+6) << 8) + *(buf+7);
    rs_info.EndMin = (*(buf+8) << 8) + *(buf+9);
    rs_info.MultiModuleSetting = (*(buf+10) << 8) + *(buf+11);
    rs_info.BatteryType = (*(buf+12) << 8) + *(buf+13);
    rs_info.BatteryCurrent = (*(buf+14) << 8) + *(buf+15);
    rs_info.BatteryShutdownVoltage = (*(buf+16) << 8) + *(buf+17);
    rs_info.BatteryFloatingVoltage = (*(buf+18) << 8) + *(buf+19);
    rs_info.BatteryReservePercentage = (*(buf+20) << 8) + *(buf+21);
    rs_info.PeakShavingPower = (*(buf+22) << 8) + *(buf+23);
    rs_info.StartFrequency = (*(buf+24) << 8) + *(buf+25);
    rs_info.EndFrequency = (*(buf+26) << 8) + *(buf+27);
    rs_info.FeedinPower = (*(buf+28) << 8) + *(buf+29);

    printf("#### Dump Hybrid RS Info ####\n");
    printf("Mode = %d ==> ", rs_info.Mode);
    switch (rs_info.Mode)
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
    printf("StarHour = %d\n", rs_info.StarHour);
    printf("StarMin = %d\n", rs_info.StarMin);
    printf("EndHour = %d\n", rs_info.EndHour);
    printf("EndMin = %d\n", rs_info.EndMin);
    printf("Multi Module Setting = %d ==> ", rs_info.MultiModuleSetting);
    switch (rs_info.MultiModuleSetting)
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
    printf("Battery Type = %d ==> ", rs_info.BatteryType);
    switch (rs_info.BatteryType)
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
            printf("Darfon LNMC\n");
            break;
        case 4:
            printf("Panasonic\n");
            break;
        case 5:
            printf("Darfon LFP\n");
            break;
        default:
            printf("other\n");
    }
    printf("Battery Current = %d A\n", rs_info.BatteryCurrent);
    printf("Battery Shutdown Voltage = %03.1f V\n", ((float)rs_info.BatteryShutdownVoltage)/10);
    printf("Battery Floating Voltage = %03.1f V\n", ((float)rs_info.BatteryFloatingVoltage)/10);
    printf("Battery Reserve Percentage = %d%%\n", rs_info.BatteryReservePercentage);
    printf("Peak Shaving Power = %d W\n", rs_info.PeakShavingPower*100);
    printf("Start Frequency = %03.1f Hz\n", (float)rs_info.StartFrequency);
    printf("End Frequency = %03.1f Hz\n", (float)rs_info.EndFrequency);
    printf("Feed-in Power = %d W\n", rs_info.FeedinPower*100);
    printf("#############################\n");
}

//int set_RS(int index);

int get_RRS(int index)
{
    printf("#### get_RRS start ####\n");

    memset(&rrs_info, 0x00, sizeof(rrs_info));

    int err = 0;
    byte *lpdata = NULL;
    //time_t current_time;
	//struct tm *log_time;

    unsigned char szHBRRSinfo[]={0x00, 0x03, 0x00, 0xA0, 0x00, 0x07, 0x00, 0x00};
    szHBRRSinfo[0]=dev_list[index].m_Addr;
    MakeReadDataCRC(szHBRRSinfo,8);

    MClearRX();
    txsize=8;
    waitAddr = dev_list[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBRRSinfo, 8);
        MStartTX(com_fd);
        //usleep(delay_time_2);

        //current_time = time(NULL);
		//log_time = localtime(&current_time);

        lpdata = GetRespond(com_fd, 19, delay_time_2);
        if ( lpdata ) {
            printf("#### get_RRS OK ####\n");
            //SaveLog((char *)"DataLogger get_RRS() : OK", log_time);
            dev_list[index].m_ok_time = time(NULL);
            dump_RRS(lpdata+3);
            return 0;
        } else {
            if ( have_respond == true ) {
                printf("#### get_RRS CRC Error ####\n");
                //SaveLog((char *)"DataLogger get_RRS() : CRC Error", log_time);
            }
            else {
                printf("#### get_RRS No Response ####\n");
                //SaveLog((char *)"DataLogger get_RRS() : No Response", log_time);
            }
            err++;
        }
    }

    return -1;
}

void dump_RRS(unsigned char *buf)
{
    rrs_info.ChargeSetting = (*(buf) << 8) + *(buf+1);
    rrs_info.ChargePower = (*(buf+2) << 8) + *(buf+3);
    rrs_info.DischargePower = (*(buf+4) << 8) + *(buf+5);
    rrs_info.RampRatePercentage = (*(buf+6) << 8) + *(buf+7);
    rrs_info.DegreeLeadLag = (*(buf+8) << 8) + *(buf+9);
    rrs_info.Volt_VAr = (*(buf+10) << 8) + *(buf+11);
    rrs_info.AC_Coupling_Power = (*(buf+12) << 8) + *(buf+13);

    printf("#### Dump Hybrid RRS Info ####\n");
    printf("Charge = %d ==> ", rrs_info.ChargeSetting);
    switch (rrs_info.ChargeSetting)
    {
        case 0:
            printf("Charge\n");
            break;
        case 1:
            printf("Discharge\n");
            break;
    }
    printf("Charge Power = %d W\n", rrs_info.ChargePower*100);
    printf("Discharge Power = %d W\n", rrs_info.DischargePower*100);
    printf("Ramp Rate Percentage = %d %%\n", rrs_info.RampRatePercentage);
    printf("Degree Lead/Lag = %d\n ==> A = ", rrs_info.DegreeLeadLag);
    switch (rrs_info.DegreeLeadLag/100)
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
    printf("\n ==> B = %02d\n", rrs_info.DegreeLeadLag%100);
    printf("Volt/VAr Q(V) = %d ==>", rrs_info.Volt_VAr);
    switch (rrs_info.Volt_VAr)
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
    printf("AC Coupling Power = %d *100W\n", rrs_info.AC_Coupling_Power);
    printf("##############################\n");
}

//int set_RRS(int index);

int get_RT(int index)
{
    printf("#### get_RT start ####\n");

    memset(&rt_info, 0x00, sizeof(rt_info));
    memset(&pvinv_err_cod1, 0x00, sizeof(pvinv_err_cod1));
    memset(&pvinv_err_cod2, 0x00, sizeof(pvinv_err_cod2));
    memset(&pvinv_err_cod3, 0x00, sizeof(pvinv_err_cod3));
    memset(&dd_err_cod, 0x00, sizeof(dd_err_cod));
    memset(&dd_err_cod2, 0x00, sizeof(dd_err_cod2));
    memset(&icon_info, 0x00, sizeof(icon_info));

    int err = 0;
    int flag = 0;
    byte *lpdata = NULL;
    //time_t current_time;
	//struct tm *log_time;

    unsigned char szHBRTinfo[]={0x00, 0x03, 0x00, 0xB0, 0x00, 0x30, 0x00, 0x00};
    szHBRTinfo[0]=dev_list[index].m_Addr;
    MakeReadDataCRC(szHBRTinfo,8);

    MClearRX();
    txsize=8;
    waitAddr = dev_list[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBRTinfo, 8);
        MStartTX(com_fd);
        //usleep(m_dl_config.m_delay_time_2);

        //current_time = time(NULL);
		//log_time = localtime(&current_time);

        lpdata = GetRespond(com_fd, 101, delay_time_2);
        if ( lpdata ) {
            printf("#### get_RT OK ####\n");
            //SaveLog((char *)"DataLogger get_RT() : OK", log_time);
            dev_list[index].m_ok_time = time(NULL);
            dump_RT(lpdata+3);
            parser_PVInvErrCOD1(rt_info.PV_Inv_Error_COD1_Record);
            //parser_PVInvErrCOD1(rt_info.PV_Inv_Error_COD1);
            parser_PVInvErrCOD2(rt_info.PV_Inv_Error_COD2_Record);
            //parser_PVInvErrCOD2(rt_info.PV_Inv_Error_COD2);
            parser_DDErrCOD(rt_info.DD_Error_COD_Record);
            //parser_DDErrCOD(rt_info.DD_Error_COD);
            parser_IconInfo(rt_info.Hybrid_IconL, rt_info.Hybrid_IconH);
            flag = 1;
            break;
            //return true;
        } else {
            if ( have_respond == true ) {
                printf("#### get_RT CRC Error ####\n");
                //SaveLog((char *)"DataLogger get_RT() : CRC Error", log_time);
            }
            else {
                printf("#### get_RT No Response ####\n");
                //SaveLog((char *)"DataLogger get_RT() : No Response", log_time);
            }
            err++;
        }
    }

    if ( flag == 0 )
        return -1;

    //szHBRTinfo[]={0x00, 0x03, 0x00, 0xB0, 0x00, 0x30, 0x00, 0x00};
    szHBRTinfo[3] = 0xF0; // addr
    szHBRTinfo[5] = 0x02; // no. of data
    MakeReadDataCRC(szHBRTinfo,8);

    MClearRX();
    txsize=8;
    waitAddr = dev_list[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBRTinfo, 8);
        MStartTX(com_fd);
        //usleep(m_dl_config.m_delay_time_2);

        //current_time = time(NULL);
		//log_time = localtime(&current_time);

        lpdata = GetRespond(com_fd, 9, delay_time_2);
        if ( lpdata ) {
            printf("#### GetHybridRTInfo2 OK ####\n");
            //SaveLog((char *)"DataLogger GetHybridRTInfo2() : OK", log_time);
            dev_list[index].m_ok_time = time(NULL);
            dump_RT2(lpdata+3);
            parser_PVInvErrCOD3(rt_info.PV_Inv_Error_COD3_Record);
            parser_DDErrCOD2(rt_info.DD_Error_COD2_Record);
            return 0;
        } else {
            if ( have_respond == true ) {
                printf("#### GetHybridRTInfo2 CRC Error ####\n");
                //SaveLog((char *)"DataLogger GetHybridRTInfo2() : CRC Error", log_time);
            }
            else {
                printf("#### GetHybridRTInfo2 No Response ####\n");
                //SaveLog((char *)"DataLogger GetHybridRTInfo2() : No Response", log_time);
            }
            err++;
        }
    }

    return -2;
}

void dump_RT(unsigned char *buf)
{
    rt_info.Inv_Temp = (*(buf) << 8) + *(buf+1);
    rt_info.PV1_Temp = (*(buf+2) << 8) + *(buf+3);
    rt_info.PV2_Temp = (*(buf+4) << 8) + *(buf+5);
    rt_info.DD_Temp = (*(buf+6) << 8) + *(buf+7);
    rt_info.PV1_Voltage = (*(buf+8) << 8) + *(buf+9);
    rt_info.PV1_Current = (*(buf+10) << 8) + *(buf+11);
    rt_info.PV1_Power = (*(buf+12) << 8) + *(buf+13);
    rt_info.PV2_Voltage = (*(buf+14) << 8) + *(buf+15);
    rt_info.PV2_Current = (*(buf+16) << 8) + *(buf+17);
    rt_info.PV2_Power = (*(buf+18) << 8) + *(buf+19);
    rt_info.Load_Voltage = (*(buf+20) << 8) + *(buf+21);
    rt_info.Load_Current = (*(buf+22) << 8) + *(buf+23);
    rt_info.Load_Power = (*(buf+24) << 8) + *(buf+25);
    rt_info.Grid_Voltage = (*(buf+26) << 8) + *(buf+27);
    rt_info.Grid_Current = (*(buf+28) << 8) + *(buf+29);
    rt_info.Grid_Power = (*(buf+30) << 8) + *(buf+31);
    rt_info.Battery_Voltage = (*(buf+32) << 8) + *(buf+33);
    rt_info.Battery_Current = (*(buf+34) << 8) + *(buf+35);
    rt_info.Bus_Voltage = (*(buf+36) << 8) + *(buf+37);
    rt_info.Bus_Current = (*(buf+38) << 8) + *(buf+39);
    rt_info.PV_Total_Power = (*(buf+40) << 8) + *(buf+41);
    rt_info.PV_Today_EnergyH = (*(buf+42) << 8) + *(buf+43);
    rt_info.PV_Today_EnergyL = (*(buf+44) << 8) + *(buf+45);
    rt_info.PV_Total_EnergyH = (*(buf+46) << 8) + *(buf+47);
    rt_info.PV_Total_EnergyL = (*(buf+48) << 8) + *(buf+49);
    rt_info.Bat_Total_EnergyH = (*(buf+50) << 8) + *(buf+51);
    rt_info.Bat_Total_EnergyL = (*(buf+52) << 8) + *(buf+53);
    rt_info.Load_Total_EnergyH = (*(buf+54) << 8) + *(buf+55);
    rt_info.Load_Total_EnergyL = (*(buf+56) << 8) + *(buf+57);
    rt_info.GridFeed_TotalH = (*(buf+58) << 8) + *(buf+59);
    rt_info.GridFeed_TotalL = (*(buf+60) << 8) + *(buf+61);
    rt_info.GridCharge_TotalH = (*(buf+62) << 8) + *(buf+63);
    rt_info.GridCharge_TotalL = (*(buf+64) << 8) + *(buf+65);
    rt_info.External_Power = (*(buf+66) << 8) + *(buf+67);
    rt_info.Sys_State = (*(buf+68) << 8) + *(buf+69);
    rt_info.PV_Inv_Error_COD1_Record = (*(buf+70) << 8) + *(buf+71);
    rt_info.PV_Inv_Error_COD2_Record = (*(buf+72) << 8) + *(buf+73);
    rt_info.DD_Error_COD_Record = (*(buf+74) << 8) + *(buf+75);
    rt_info.PV_Inv_Error_COD1 = (*(buf+76) << 8) + *(buf+77);
    rt_info.PV_Inv_Error_COD2 = (*(buf+78) << 8) + *(buf+79);
    rt_info.DD_Error_COD = (*(buf+80) << 8) + *(buf+81);
    rt_info.Hybrid_IconL = (*(buf+82) << 8) + *(buf+83);
    rt_info.Hybrid_IconH = (*(buf+84) << 8) + *(buf+85);
    rt_info.Error_Code = (*(buf+86) << 8) + *(buf+87);
    rt_info.Battery_SOC = (*(buf+88) << 8) + *(buf+89);
    rt_info.Invert_Frequency = (*(buf+90) << 8) + *(buf+91);
    rt_info.Grid_Frequency = (*(buf+92) << 8) + *(buf+93);
    rt_info.PBat = (*(buf+94) << 8) + *(buf+95);

    printf("#### Dump Hybrid RT Info ####\n");
    printf("Inv_Temp = %03.1f C\n", ((float)rt_info.Inv_Temp)/10);
    printf("PV1_Temp = %03.1f C\n", ((float)rt_info.PV1_Temp)/10);
    printf("PV2_Temp = %03.1f C\n", ((float)rt_info.PV2_Temp)/10);
    printf("DD_Temp = %03.1f C\n", ((float)rt_info.DD_Temp)/10);
    printf("PV1_Voltage = %d V\n", rt_info.PV1_Voltage);
    printf("PV1_Current = %04.2f A\n", ((float)rt_info.PV1_Current)/100);
    printf("PV1_Power = %d W\n", rt_info.PV1_Power);
    printf("PV2_Voltage = %d V\n", rt_info.PV2_Voltage);
    printf("PV2_Current = %04.2f A\n", ((float)rt_info.PV2_Current)/100);
    printf("PV2_Power = %d W\n", rt_info.PV2_Power);
    printf("Load Voltage = %d V\n", rt_info.Load_Voltage);
    printf("Load Current = %04.2f A\n", ((float)rt_info.Load_Current)/100);
    printf("Load Power = %d W\n", rt_info.Load_Power);
    printf("Grid Voltage = %d V\n", rt_info.Grid_Voltage);
    printf("Grid Current = %04.2f A\n", ((float)rt_info.Grid_Current)/100);
    printf("Grid Power = %d W\n", rt_info.Grid_Power);
    printf("Battery Voltage = %03.1f V\n", ((float)rt_info.Battery_Voltage)/10);
    printf("Battery Current = %03.1f A\n", ((float)rt_info.Battery_Current)/10);
    printf("Bus Voltage = %03.1f V\n", ((float)rt_info.Bus_Voltage)/10);
    printf("Bus Current = %03.1f A\n", ((float)rt_info.Bus_Current)/10);
    printf("PV Total Power = %d W\n", rt_info.PV_Total_Power);
    printf("PV Today EnergyH = %d\n", rt_info.PV_Today_EnergyH);
    printf("PV Today EnergyL = %d\n", rt_info.PV_Today_EnergyL);
    printf("PV Today Energy = %04.2f kWHr\n", rt_info.PV_Today_EnergyH*100 + ((float)rt_info.PV_Today_EnergyL)*0.01);
    printf("PV Total EnergyH = %d\n", rt_info.PV_Total_EnergyH);
    printf("PV Total EnergyL = %d\n", rt_info.PV_Total_EnergyL);
    printf("PV Total Energy = %04.2f kWHr\n", rt_info.PV_Total_EnergyH*100 + ((float)rt_info.PV_Total_EnergyL)*0.01);
    printf("Bat Total EnergyH = %d\n", rt_info.Bat_Total_EnergyH);
    printf("Bat Total EnergyL = %d\n", rt_info.Bat_Total_EnergyL);
    printf("Bat Total Energy = %04.2f kWHr\n", rt_info.Bat_Total_EnergyH*100 + ((float)rt_info.Bat_Total_EnergyL)*0.01);
    printf("Load Total EnergyH = %d\n", rt_info.Load_Total_EnergyH);
    printf("Load Total EnergyL = %d\n", rt_info.Load_Total_EnergyL);
    printf("Load Total Energy = %04.2f kWHr\n", rt_info.Load_Total_EnergyH*100 + ((float)rt_info.Load_Total_EnergyL)*0.01);
    printf("GridFeed_TotalH = %d\n", rt_info.GridFeed_TotalH);
    printf("GridFeed_TotalL = %d\n", rt_info.GridFeed_TotalL);
    printf("GridFeed_Total = %04.2f kWHr\n", rt_info.GridFeed_TotalH*100 + ((float)rt_info.GridFeed_TotalL)*0.01);
    printf("GridCharge_TotalH = %d\n", rt_info.GridCharge_TotalH);
    printf("GridCharge_TotalL = %d\n", rt_info.GridCharge_TotalL);
    printf("GridCharge_Total = %04.2f kWHr\n", rt_info.GridCharge_TotalH*100 + ((float)rt_info.GridCharge_TotalL)*0.01);
    printf("External_Power = %d W\n", rt_info.External_Power*100);
    printf("Sys_State = %x\n", rt_info.Sys_State);
    printf("PV_Inv_Error_COD1_Record = 0x%04X\n", rt_info.PV_Inv_Error_COD1_Record);
    printf("PV_Inv_Error_COD2_Record = 0x%04X\n", rt_info.PV_Inv_Error_COD2_Record);
    printf("DD_Error_COD_Record = 0x%04X\n", rt_info.DD_Error_COD_Record);
    printf("PV_Inv_Error_COD1 = 0x%04X\n", rt_info.PV_Inv_Error_COD1);
    printf("PV_Inv_Error_COD2 = 0x%04X\n", rt_info.PV_Inv_Error_COD2);
    printf("DD_Error_COD = 0x%04X\n", rt_info.DD_Error_COD);
    printf("Hybrid_IconL = 0x%04X\n", rt_info.Hybrid_IconL);
    printf("Hybrid_IconH = 0x%04X\n", rt_info.Hybrid_IconH);
    printf("Error_Code = 0x%04X\n", rt_info.Error_Code);
    printf("Battery_SOC = %d %%\n", rt_info.Battery_SOC);
    printf("Invert Frequency = %03.1f Hz\n", ((float)rt_info.Invert_Frequency)/10);
    printf("Grid Frequency = %03.1f Hz\n", ((float)rt_info.Grid_Frequency)/10);
    printf("PBat = %d W\n", rt_info.PBat);
    printf("#############################\n");
}

void dump_RT2(unsigned char *buf)
{
    rt_info.PV_Inv_Error_COD3_Record = (*(buf) << 8) + *(buf+1);
    rt_info.DD_Error_COD2_Record = (*(buf+2) << 8) + *(buf+3);

    printf("#### Dump Hybrid RT Info ####\n");
    printf("PV_Inv_Error_COD3_Record = 0x%04X\n", rt_info.PV_Inv_Error_COD3_Record);
    printf("DD_Error_COD2_Record = 0x%04X\n", rt_info.DD_Error_COD2_Record);
    printf("#############################\n");
}

void parser_PVInvErrCOD1(int COD1)
{
    int tmp = COD1;
    pvinv_err_cod1.B0_Fac_HL = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B1_CanBus_Fault = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B2_Islanding = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B3_Vac_H = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B4_Vac_L = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B5_Fac_H = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B6_Fac_L = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B7_Fac_LL = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B8_Vac_OCP = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B9_Vac_HL = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B10_Vac_LL = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B11_OPP = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B12_Iac_H = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B13_Ipv_H = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B14_ADCINT_OVF = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod1.B15_Vbus_H = tmp & 0x0001;

    printf("#### Parser Hybrid PV Inverter Error Code 1 ####\n");
    printf("Bit0  : Fac_HL = %d\n", pvinv_err_cod1.B0_Fac_HL);
    printf("Bit1  : CanBus_Fault = %d\n", pvinv_err_cod1.B1_CanBus_Fault);
    printf("Bit2  : Islanding = %d\n", pvinv_err_cod1.B2_Islanding);
    printf("Bit3  : Vac_H = %d\n", pvinv_err_cod1.B3_Vac_H);
    printf("Bit4  : Vac_L = %d\n", pvinv_err_cod1.B4_Vac_L);
    printf("Bit5  : Fac_H = %d\n", pvinv_err_cod1.B5_Fac_H);
    printf("Bit6  : Fac_L = %d\n", pvinv_err_cod1.B6_Fac_L);
    printf("Bit7  : Fac_LL = %d\n", pvinv_err_cod1.B7_Fac_LL);
    printf("Bit8  : Vac_OCP = %d\n", pvinv_err_cod1.B8_Vac_OCP);
    printf("Bit9  : Vac_HL = %d\n", pvinv_err_cod1.B9_Vac_HL);
    printf("Bit10 : Vac_LL = %d\n", pvinv_err_cod1.B10_Vac_LL);
    printf("Bit11 : OPP = %d\n", pvinv_err_cod1.B11_OPP);
    printf("Bit12 : Iac_H = %d\n", pvinv_err_cod1.B12_Iac_H);
    printf("Bit13 : Ipv_H = %d\n", pvinv_err_cod1.B13_Ipv_H);
    printf("Bit14 : ADCINT_OVF = %d\n", pvinv_err_cod1.B14_ADCINT_OVF);
    printf("Bit15 : Vbus_H = %d\n", pvinv_err_cod1.B15_Vbus_H);
    printf("################################################\n");
}

void parser_PVInvErrCOD2(int COD2)
{
    int tmp = COD2;
    pvinv_err_cod2.B0_Arc = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B1_Vac_Relay_Fault = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B2_Ipv1_Short = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B3_Ipv2_Short = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B4_Vac_Short = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B5_CT_Fault = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B6_PV_Over_Power = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B7_NO_GRID = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B8_PV_Input_High = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B9_INV_Overload = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B10_RCMU_30 = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B11_RCMU_60 = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B12_RCMU_150 = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B13_RCMU_300 = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B14_RCMU_Test_Fault = tmp & 0x0001;
    tmp>>=1;
    pvinv_err_cod2.B15_Vac_LM = tmp & 0x0001;

    printf("#### Parser Hybrid PV Inverter Error Code 2 ####\n");
    printf("Bit0  : Arc = %d\n", pvinv_err_cod2.B0_Arc);
    printf("Bit1  : Vac_Relay_Fault = %d\n", pvinv_err_cod2.B1_Vac_Relay_Fault);
    printf("Bit2  : Ipv1_Short = %d\n", pvinv_err_cod2.B2_Ipv1_Short);
    printf("Bit3  : Ipv2_Short = %d\n", pvinv_err_cod2.B3_Ipv2_Short);
    printf("Bit4  : Vac_Short = %d\n", pvinv_err_cod2.B4_Vac_Short);
    printf("Bit5  : CT_Fault = %d\n", pvinv_err_cod2.B5_CT_Fault);
    printf("Bit6  : PV_Over_Power = %d\n", pvinv_err_cod2.B6_PV_Over_Power);
    printf("Bit7  : NO_GRID = %d\n", pvinv_err_cod2.B7_NO_GRID);
    printf("Bit8  : PV_Input_High = %d\n", pvinv_err_cod2.B8_PV_Input_High);
    printf("Bit9  : INV_Overload = %d\n", pvinv_err_cod2.B9_INV_Overload);
    printf("Bit10 : RCMU_30 = %d\n", pvinv_err_cod2.B10_RCMU_30);
    printf("Bit11 : RCMU_60 = %d\n", pvinv_err_cod2.B11_RCMU_60);
    printf("Bit12 : RCMU_150 = %d\n", pvinv_err_cod2.B12_RCMU_150);
    printf("Bit13 : RCMU_300 = %d\n", pvinv_err_cod2.B13_RCMU_300);
    printf("Bit14 : RCMU_Test_Fault = %d\n", pvinv_err_cod2.B14_RCMU_Test_Fault);
    printf("Bit15 : Vac_LM = %d\n", pvinv_err_cod2.B15_Vac_LM);
    printf("################################################\n");
}

void parser_PVInvErrCOD3(int COD3)
{
    int tmp = COD3;
    pvinv_err_cod3.B0_External_PV_OPP = tmp & 0x0001;

    printf("#### Parser Hybrid PV Inverter Error Code 3 ####\n");
    printf("Bit0  : External_PV_OPP = %d\n", pvinv_err_cod3.B0_External_PV_OPP);
    printf("################################################\n");
}

void parser_DDErrCOD(int COD)
{
    int tmp = COD;
    dd_err_cod.B0_Vbat_H = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B1_Vbat_L = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B2_Vbus_H = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B3_Vbus_L = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B4_Ibus_H = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B5_Ibat_H = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B6_Charger_T = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B7_Code = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B8_Vbat_Drop = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B9_INV_Fault = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B10_GND_Fault = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B11_No_bat = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B12_BMS_Comute_Fault = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B13_BMS_Over_Current = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B14_Restart = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod.B15_Bat_Setting_Fault = tmp & 0x0001;

    printf("#### Parser Hybrid DD Error Code ####\n");
    printf("Bit0  : Vbat_H = %d\n", dd_err_cod.B0_Vbat_H);
    printf("Bit1  : Vbat_L = %d\n", dd_err_cod.B1_Vbat_L);
    printf("Bit2  : Vbus_H = %d\n", dd_err_cod.B2_Vbus_H);
    printf("Bit3  : Vbus_L = %d\n", dd_err_cod.B3_Vbus_L);
    printf("Bit4  : Ibus_H = %d\n", dd_err_cod.B4_Ibus_H);
    printf("Bit5  : Ibat_H = %d\n", dd_err_cod.B5_Ibat_H);
    printf("Bit6  : Charger_T = %d\n", dd_err_cod.B6_Charger_T);
    printf("Bit7  : Code = %d\n", dd_err_cod.B7_Code);
    printf("Bit8  : Vbat_Drop = %d\n", dd_err_cod.B8_Vbat_Drop);
    printf("Bit9  : INV_Fault = %d\n", dd_err_cod.B9_INV_Fault);
    printf("Bit10 : GND_Fault = %d\n", dd_err_cod.B10_GND_Fault);
    printf("Bit11 : No_bat = %d\n", dd_err_cod.B11_No_bat);
    printf("Bit12 : BMS_Comute_Fault = %d\n", dd_err_cod.B12_BMS_Comute_Fault);
    printf("Bit13 : BMS_Over_Current = %d\n", dd_err_cod.B13_BMS_Over_Current);
    printf("Bit14 : Restart = %d\n", dd_err_cod.B14_Restart);
    printf("Bit15 : Bat_Setting_Fault = %d\n", dd_err_cod.B15_Bat_Setting_Fault);
    printf("#####################################\n");
}

void parser_DDErrCOD2(int COD2)
{
    int tmp = COD2;
    dd_err_cod2.B0_EEProm_Fault = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod2.B1_Communi_Fault = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod2.B2_OT_Fault = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod2.B3_Fan_Fault = tmp & 0x0001;
    tmp>>=1;
    dd_err_cod2.B4_Low_Battery = tmp & 0x0001;

    printf("#### Parser Hybrid DD Error Code 2 ####\n");
    printf("Bit0  : EEProm_Fault = %d\n", dd_err_cod2.B0_EEProm_Fault);
    printf("Bit1  : Communi_Fault = %d\n", dd_err_cod2.B1_Communi_Fault);
    printf("Bit2  : OT_Fault = %d\n", dd_err_cod2.B2_OT_Fault);
    printf("Bit3  : Fan_Fault = %d\n", dd_err_cod2.B3_Fan_Fault);
    printf("Bit4  : Low_Battery = %d\n", dd_err_cod2.B4_Low_Battery);
    printf("#####################################\n");
}

void parser_IconInfo(int Icon_L, int Icon_H)
{
    int tmp = Icon_L;
    icon_info.B0_PV = tmp & 0x0001;
    tmp>>=1;
    icon_info.B1_MPPT = tmp & 0x0001;
    tmp>>=1;
    icon_info.B2_Battery = tmp & 0x0001;
    tmp>>=1;
    icon_info.B3_Inverter = tmp & 0x0001;
    tmp>>=1;
    icon_info.B4_Grid = tmp & 0x0001;
    tmp>>=1;
    icon_info.B5_Load = tmp & 0x0001;
    tmp>>=1;
    icon_info.B6_OverLoad = tmp & 0x0001;
    tmp>>=1;
    icon_info.B7_Error = tmp & 0x0001;
    tmp>>=1;
    icon_info.B8_Warning = tmp & 0x0001;
    tmp>>=1;
    icon_info.B9_PC = tmp & 0x0001;
    tmp>>=1;
    icon_info.B10_BatCharge = tmp & 0x0001;
    tmp>>=1;
    icon_info.B11_BatDischarge = tmp & 0x0001;
    tmp>>=1;
    icon_info.B12_FeedingGrid = tmp & 0x0001;
    tmp>>=1;
    icon_info.B13_PFCMode = tmp & 0x0001;
    tmp>>=1;
    icon_info.B14_GridCharge = tmp & 0x0001;
    tmp>>=1;
    icon_info.B15_GridDischarge = tmp & 0x0001;

    tmp = Icon_H;
    icon_info.B16_18_INVFlag = tmp & 0x0007;
    tmp>>=3;
    icon_info.B19_GeneratorMode = tmp & 0x0001;
    tmp>>=1;
    icon_info.B20_Master_Slave = tmp & 0x0001;
    tmp>>=1;
    icon_info.B21_SettingOK = tmp & 0x0001;
    tmp>>=1;
    icon_info.B22_24_BatType = tmp & 0x0007;
    tmp>>=3;
    icon_info.B25_26_MultiINV = tmp & 0x0003;
    tmp>>=2;
    icon_info.B27_LoadCharge = tmp & 0x0001;
    tmp>>=1;
    icon_info.B28_LoadDischarge = tmp & 0x0001;
    tmp>>=1;
    icon_info.B29_30_LeadLag = tmp & 0x0003;
    tmp>>=2;

    printf("#### Parser Hybrid Icon ####\n");
    printf("Bit0     : PV = %d\n", icon_info.B0_PV);
    printf("Bit1     : MPPT = %d\n", icon_info.B1_MPPT);
    printf("Bit2     : Battery = %d\n", icon_info.B2_Battery);
    printf("Bit3     : Inverter = %d\n", icon_info.B3_Inverter);
    printf("Bit4     : Grid = %d\n", icon_info.B4_Grid);
    printf("Bit5     : Load = %d\n", icon_info.B5_Load);
    printf("Bit6     : OverLoad = %d\n", icon_info.B6_OverLoad);
    printf("Bit7     : Error = %d\n", icon_info.B7_Error);
    printf("Bit8     : Warning = %d\n", icon_info.B8_Warning);
    printf("Bit9     : PC = %d\n", icon_info.B9_PC);
    printf("Bit10    : Bat Charge = %d\n", icon_info.B10_BatCharge);
    printf("Bit11    : Bat Discharge = %d\n", icon_info.B11_BatDischarge);
    printf("Bit12    : Feeding Grid = %d\n", icon_info.B12_FeedingGrid);
    printf("Bit13    : PFC Mode = %d\n", icon_info.B13_PFCMode);
    printf("Bit14    : Grid Charge = %d\n", icon_info.B14_GridCharge);
    printf("Bit15    : Grid Discharge = %d\n", icon_info.B15_GridDischarge);
    printf("Bit16-18 : INV Flag = %d\n", icon_info.B16_18_INVFlag);
    printf("Bit19    : Generator Mode = %d\n", icon_info.B19_GeneratorMode);
    printf("Bit20    : Master Slave = %d\n", icon_info.B20_Master_Slave);
    printf("Bit21    : Setting OK = %d\n", icon_info.B21_SettingOK);
    printf("Bit22-24 : Bat Type = %d\n", icon_info.B22_24_BatType);
    printf("Bit25-26 : Multi-INV = %d\n", icon_info.B25_26_MultiINV);
    printf("Bit27    : Load Charge = %d\n", icon_info.B27_LoadCharge);
    printf("Bit28    : Load Discharge = %d\n", icon_info.B28_LoadDischarge);
    printf("Bit29-30 : Lead Lag = %d\n", icon_info.B29_30_LeadLag);
    printf("############################\n");
}

int get_BMS(int index)
{
    printf("#### get_BMS start ####\n");

    memset(&bms_info, 0x00, sizeof(bms_info));

    int err = 0;
    byte *lpdata = NULL;
    //time_t current_time;
	//struct tm *log_time;

    unsigned char szHBBMSinfo[]={0x00, 0x03, 0x02, 0x00, 0x00, 0x0C, 0x00, 0x00};
    szHBBMSinfo[0]=dev_list[index].m_Addr;
    MakeReadDataCRC(szHBBMSinfo,8);

    MClearRX();
    txsize=8;
    waitAddr = dev_list[index].m_Addr;
    waitFCode = 0x03;

    while ( err < 3 ) {
        memcpy(txbuffer, szHBBMSinfo, 8);
        MStartTX(com_fd);
        //usleep(m_dl_config.delay_time_2);

        //current_time = time(NULL);
		//log_time = localtime(&current_time);

        lpdata = GetRespond(com_fd, 29, delay_time_2);
        if ( lpdata ) {
            printf("#### get_BMS OK ####\n");
            //SaveLog((char *)"DataLogger get_BMS() : OK", log_time);
            dev_list[index].m_ok_time = time(NULL);
            dump_BMS(lpdata+3);
            return 0;
        } else {
            if ( have_respond == true ) {
                printf("#### get_BMS CRC Error ####\n");
                //SaveLog((char *)"DataLogger get_BMS() : CRC Error", log_time);
            }
            else {
                printf("#### get_BMS No Response ####\n");
                //SaveLog((char *)"DataLogger get_BMS() : No Response", log_time);
            }
            err++;
        }
    }

    return -1;
}

void dump_BMS(unsigned char *buf)
{
    bms_info.Voltage = (*(buf) << 8) + *(buf+1);
    bms_info.Current = (*(buf+2) << 8) + *(buf+3);
    bms_info.SOC = (*(buf+4) << 8) + *(buf+5);
    bms_info.MaxTemperature = (*(buf+6) << 8) + *(buf+7);
    bms_info.CycleCount = (*(buf+8) << 8) + *(buf+9);
    bms_info.Status = (*(buf+10) << 8) + *(buf+11);
    bms_info.Error = (*(buf+12) << 8) + *(buf+13);
    bms_info.Number = (*(buf+14) << 8) + *(buf+15);
    bms_info.BMS_Info = (*(buf+16) << 8) + *(buf+17);
    bms_info.BMS_Max_Cell = (*(buf+18) << 8) + *(buf+19);
    bms_info.BMS_Min_Cell = (*(buf+20) << 8) + *(buf+21);
    bms_info.BMS_BaudRate = (*(buf+22) << 8) + *(buf+23);

    printf("#### Dump Hybrid BMS Info ####\n");
    printf("Voltage           = %d mV\n", bms_info.Voltage*10);
    printf("Current           = %d mA\n", bms_info.Current*10);
    printf("SOC               = %d %%\n", bms_info.SOC);
    printf("MaxTemperature    = %d C\n", bms_info.MaxTemperature);
    printf("Cycle Count       = %d\n", bms_info.CycleCount);
    printf("Status            = %x\n", bms_info.Status);
    printf("Error             = 0x%04X\n", bms_info.Error);
    printf("Module Number     = %d\n", bms_info.Number);
    printf("BMS Info          = %d\n", bms_info.BMS_Info);
    printf("BMS Max Cell      = %d mV\n", bms_info.BMS_Max_Cell);
    printf("BMS Min Cell      = %d mV\n", bms_info.BMS_Min_Cell);
    printf("BMS BaudRate      = %d bps\n", bms_info.BMS_BaudRate);
    printf("##############################\n");
}

/*int find_list(char *FILENAME)
{
    char buf[64] = {0}; // PostMIList result date length about = 372
    FILE *pMIList_fd = NULL;

    // get MIlist list filename
    system("cd /tmp; ls MIList_* > /tmp/MIList");

    pMIList_fd = fopen("/tmp/MIList", "rb");
    if ( pMIList_fd == NULL ) {
        printf("#### Open /tmp/MIList Fail ####\n");
        return 1;
    }
    // get file name
    memset(buf, 0x00, 64);
    fgets(buf, 64, pMIList_fd);
    fclose(pMIList_fd);
    if ( strlen(buf) )
        buf[strlen(buf)-1] = 0; // set '\n' to 0
    else {
        printf("Empty file! Plese check MIList exist!\n");
        return 2;
    }

    sprintf(FILENAME, "/tmp/%s", buf);
    printf("FILENAME = %s\n", FILENAME);

    // set base64 file
    //pMIList_fd = fopen("/tmp/base64_MIList", "wb");
    //if ( pMIList_fd == NULL ) {
    //    printf("#### Open /tmp/base64_MIList Fail ####\n");
    //    return 2;
    //}
    //base64_encode(FILENAME, pMIList_fd);
    //memset(FILENAME, 0x00, 64);
    //strcpy(FILENAME, "/tmp/base64_MIList");
    //printf("base64 FILENAME = %s\n", FILENAME);

    return 0;
}*/

int check_cmd(unsigned char *cmd)
{
    int i, all_ok = 1;
    unsigned char checksum = 0;

    // start code
    if (cmd[0] != 0xFA) {
        all_ok = 0;
        printf("start code error\n");
    }
    // end code
    if (cmd[7] != 0xAF) {
        all_ok = 0;
        printf("end code error\n");
    }
    // checksum
    checksum = 0;
    for (i = 0; i < 6; i++)
        checksum += cmd[i];
    printf("checksum = %02X\n", checksum);
    if (cmd[6] != checksum) {
        all_ok = 0;
        printf("checksum error\n");
    }

    return all_ok;
}

int send_file(int sockfd, char *FILENAME, int filesize)
{
    FILE *pfile_fd = NULL;
    unsigned char buf[1024] = {0};
    int totalsize = filesize;
    int sendsize = 0;

    pfile_fd = fopen(FILENAME, "rb");
    if ( pfile_fd == NULL ) {
        printf("#### Open %s Fail ####\n", FILENAME);
        return 1;
    }

    while (totalsize) {
        memset(buf, 0x00, 1024);
        if (totalsize > 1024) {
            fread(buf, 1, 1024, pfile_fd);
            sendsize = send(sockfd, buf, 1024, 0);
            printf("sendsize = %d\n", sendsize);
            totalsize -= 1024;
        } else {
            fread(buf, 1, totalsize, pfile_fd);
            sendsize = send(sockfd, buf, totalsize, 0);
            printf("sendsize = %d\n", sendsize);
            totalsize = 0;
        }
    }
    fclose(pfile_fd);
    printf("send OK.\n");

    return 0;
}

int write_data(char *sn, int addr, int count, unsigned char *data)
{
    printf("#### write_data start ####\n");

    int i = 0, index = 0, err = 0;
    byte *lpdata = NULL;
    time_t current_time = 0;
    unsigned short crc;

    // find index
    for (i = 0; i < dev_count; i++) {
        if ( !strncmp(dev_list[i].m_Sn, sn, 16) ) {
            index = i;
            break;
        }
    }

    current_time = time(NULL);
    if ( current_time - dev_list[index].m_ok_time >= OFFLINE_SECOND_HB ) {
        printf("Last m_ok_time more then 180 sec.\n");
        if ( re_register(index) )
            return 3;
    }

    unsigned char write_buf[41];
    memset(write_buf, 0x00, 41);
    write_buf[0] = dev_list[index].m_Addr;
    write_buf[1] = 0x10; // function code
    write_buf[2] = (unsigned char)((addr>>8)&0x000000FF);
    write_buf[3] = (unsigned char)(addr&0x000000FF); //start addr
    write_buf[4] = 0x00;
    if ( addr == 1 ) {
        write_buf[5] = 0x0F; // number of data
        write_buf[6] = 0x1E; // bytes
    } else {
        write_buf[5] = 0x10; // number of data
        write_buf[6] = 0x20; // bytes
    }
    for (i = 0; i < 2*count; i++)
        write_buf[7+i] = data[i];
    crc = CalculateCRC(data, 2*count);
    if ( addr == 1 ) {
        write_buf[35] = (unsigned char) (crc >> 8); // data crc hi
        write_buf[36] = (unsigned char) (crc & 0xFF); // data crc lo
        MakeReadDataCRC(write_buf,39);
    } else {
        write_buf[37] = (unsigned char) (crc >> 8); // data crc hi
        write_buf[38] = (unsigned char) (crc & 0xFF); // data crc lo
        MakeReadDataCRC(write_buf,41);
    }

    MClearRX();
    if ( addr == 1 ) {
        txsize=39;
    } else {
        txsize=41;
    }
    waitAddr = dev_list[index].m_Addr;
    waitFCode = 0x10;

    while ( err < 3 ) {
        memcpy(txbuffer, write_buf, txsize);
        MStartTX(com_fd);
        //usleep(delay_time_2);

        //current_time = time(NULL);
		//log_time = localtime(&current_time);

        lpdata = GetRespond(com_fd, 8, delay_time_2);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 8) ) {
                printf("#### write_data OK ####\n");
                dev_list[index].m_ok_time = time(NULL);
                //free(lpdata);
                return 0;
            } else {
                printf("#### write_data CRC Error ####\n");
                err++;
            }
            //free(lpdata);
        } else {
            printf("#### write_data No Response ####\n");
            err++;
        }
    }

    return 4;
}

int main(int argc , char *argv[])
{
    char opt;
    while( (opt = getopt(argc, argv, "vVtT")) != -1 )
    {
        switch (opt)
        {
            case 'v':
            case 'V':
                printf("%s\n", VERSION);
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

    printf("Start DL socket.\n");

    char FILENAME[64] = {0};
    char buf[128] = {0};
    FILE *pdate_fd = NULL;
    FILE *ptime_fd = NULL;
    FILE *pset_fd = NULL;
    int logdate = 0, size = 0, cmddate = 0;
    int totalsize = 0, getsize = 0;
    struct stat st;

    // create socket
    unsigned char cmd_buf[8] = {0};
    unsigned char ret_buf[8] = {0};
    int sockfd = 0, forClientSockfd = 0;

    printf("run socket()\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1){
        printf("Fail to create a socket.\n");
    }

    // bind
    struct sockaddr_in serverInfo, clientInfo;
    int addrlen = sizeof(clientInfo);
    bzero(&serverInfo, sizeof(serverInfo));

    printf("run bind()\n");
    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(MY_PORT);
    bind(sockfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo));

    // set timeout
    //struct timeval tv;
    //printf("run setsockopt()\n");
    //tv.tv_sec = 10;
    //tv.tv_usec = 0;
    //setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    printf("run listen()\n");
    listen(sockfd, 2);

    int i = 0, ret = 0;
    while(1){
        printf("run get_config()\n");
        get_config();

        printf("run accept()\n");
        forClientSockfd = accept(sockfd, (struct sockaddr*) &clientInfo, (socklen_t*) &addrlen);

        printf("get coomand\n");
        memset(cmd_buf, 0x00, 8);
        recv(forClientSockfd, cmd_buf, sizeof(cmd_buf), 0);
        // debug print
        for (i = 0; i < 8; i++)
            printf("cmd_buf[%d] = %02X\n", i, cmd_buf[i]);
        // debug print end
        // parser command & do something
        ret = check_cmd(cmd_buf);
        if (ret) {
            // cmd OK, do action by function code
            switch (cmd_buf[1]) {
                case 0x01: //get list
                    printf("cmd 01 ok. get list.\n");

                    stop_process();
                    get_config();
                    open_com_port();
                    run_register();
                    for (i = 0; i < dev_count; i++)
                        get_device(i);
                    write_Hybrid_list();
                    ret = stat(HYBRID_LIST_PATH, &st);
                    if ( ret == 0 ) { // list exist
                        size = st.st_size;

                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x01;
                        ret_buf[2] = (unsigned char)((size >> 24) & 0x000000FF);
                        ret_buf[3] = (unsigned char)((size >> 16) & 0x000000FF);
                        ret_buf[4] = (unsigned char)((size >> 8) & 0x000000FF);
                        ret_buf[5] = (unsigned char)(size & 0x000000FF);
                        ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                        ret_buf[7] = 0xAF;
                        printf("send size num\n");
                        send(forClientSockfd, ret_buf, 8, 0);

                        // send list file
                        printf("send file %s\n", HYBRID_LIST_PATH);
                        send_file(forClientSockfd, HYBRID_LIST_PATH, size);

                    } else {
                        // send zero size
                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x01;
                        ret_buf[2] = 0x00;
                        ret_buf[3] = 0x00;
                        ret_buf[4] = 0x00;
                        ret_buf[5] = 0x00;
                        ret_buf[6] = 0xFB;
                        ret_buf[7] = 0xAF;
                        printf("send size 0\n");
                        send(forClientSockfd, ret_buf, 8, 0);
                    }
                    break;

                case 0x02: // get data
                    printf("cmd 02 ok. get data.\n");

                    cmddate = (cmd_buf[2] << 24) + (cmd_buf[3] << 16) + (cmd_buf[4] << 8) + cmd_buf[5];

                    // check usb file exist
                    if (stat(LOG_PATH, &st) != 0) {
                        // send zero size, finish
                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x02;
                        ret_buf[2] = 0x00;
                        ret_buf[3] = 0x00;
                        ret_buf[4] = 0x00;
                        ret_buf[5] = 0x00;
                        ret_buf[6] = 0xFC;
                        ret_buf[7] = 0xAF;
                        printf("send size 0, finish\n");
                        send(forClientSockfd, ret_buf, 8, 0);

                        break;
                    }
                    // get Log file date list
                    sprintf(buf, "cd %s; ls > /tmp/Socket_LogDate", LOG_PATH);
                    system(buf);

                    pdate_fd = fopen("/tmp/Socket_LogDate", "rb");
                    if ( pdate_fd == NULL ) {
                        printf("#### Open /tmp/Socket_LogDate Fail ####\n");

                        // send zero size, finish
                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x02;
                        ret_buf[2] = 0x00;
                        ret_buf[3] = 0x00;
                        ret_buf[4] = 0x00;
                        ret_buf[5] = 0x00;
                        ret_buf[6] = 0xFC;
                        ret_buf[7] = 0xAF;
                        printf("send size 0, finish\n");
                        send(forClientSockfd, ret_buf, 8, 0);

                        break;
                    }

                    // get Log file date
                    memset(buf, 0x00, 128);
                    while ( fgets(buf, 128, pdate_fd) != NULL ) {
                        if ( strlen(buf) == 0 )
                            break;
                        // set '\n' to 0
                        buf[strlen(buf)-1] = 0;
                        sscanf(buf, "%d", &logdate);
                        //printf("Get LogDate = %d\n", logdate);

                        if ( logdate < cmddate )
                            continue;

                        sprintf(buf, "cd %s/%d; ls > /tmp/Socket_LogTime", LOG_PATH, logdate);
                        //printf("buf cmd = %s\n", buf);
                        system(buf);

                        ptime_fd = fopen("/tmp/Socket_LogTime", "rb");
                        if ( ptime_fd == NULL ) {
                            printf("#### Open /tmp/Socket_LogTime Fail ####\n");
                            // some day error, continue;
                            continue;
                        }
                        // get log file time
                        memset(buf, 0x00, 128);
                        while ( fgets(buf, 128, ptime_fd) != NULL ) {
                            if ( strlen(buf) == 0 )
                                break;
                            // set '\n' to 0
                            buf[strlen(buf)-1] = 0;

                            memset(FILENAME, 0x00, 64);
                            sprintf(FILENAME, "%s/%d/%s", LOG_PATH, logdate, buf);
                            printf("FILENAME = %s\n", FILENAME);

                            // check size
                            stat(FILENAME, &st);
                            size = st.st_size;

                            if ( size == 0 )
                                continue;

                            ret_buf[0] = 0xFA;
                            ret_buf[1] = 0x02;
                            ret_buf[2] = (unsigned char)((size >> 24) & 0x000000FF);
                            ret_buf[3] = (unsigned char)((size >> 16) & 0x000000FF);
                            ret_buf[4] = (unsigned char)((size >> 8) & 0x000000FF);
                            ret_buf[5] = (unsigned char)(size & 0x000000FF);
                            ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                            ret_buf[7] = 0xAF;
                            printf("send size num\n");
                            send(forClientSockfd, ret_buf, 8, 0);

                            // send list file
                            printf("send file %s\n", FILENAME);
                            send_file(forClientSockfd, FILENAME, size);
                        }
                        fclose(ptime_fd);

                    }
                    fclose(pdate_fd);

                    // send zero size, finish
                    ret_buf[0] = 0xFA;
                    ret_buf[1] = 0x02;
                    ret_buf[2] = 0x00;
                    ret_buf[3] = 0x00;
                    ret_buf[4] = 0x00;
                    ret_buf[5] = 0x00;
                    ret_buf[6] = 0xFC;
                    ret_buf[7] = 0xAF;
                    printf("send size 0, finish\n");
                    send(forClientSockfd, ret_buf, 8, 0);
                    break;

                case 0x03:
                    printf("cmd 03 ok. get data.\n");

                    cmddate = (cmd_buf[2] << 24) + (cmd_buf[3] << 16) + (cmd_buf[4] << 8) + cmd_buf[5];

                    // check usb file exist
                    if (stat(ERRLOG_PATH, &st) != 0) {
                        // send zero size, finish
                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x03;
                        ret_buf[2] = 0x00;
                        ret_buf[3] = 0x00;
                        ret_buf[4] = 0x00;
                        ret_buf[5] = 0x00;
                        ret_buf[6] = 0xFD;
                        ret_buf[7] = 0xAF;
                        printf("send size 0, finish\n");
                        send(forClientSockfd, ret_buf, 8, 0);

                        break;
                    }

                    // get ErrLog file date list
                    sprintf(buf, "cd %s; ls > /tmp/Socket_ErrLogDate", ERRLOG_PATH);
                    system(buf);

                    pdate_fd = fopen("/tmp/Socket_ErrLogDate", "rb");
                    if ( pdate_fd == NULL ) {
                        printf("#### Open /tmp/Socket_ErrLogDate Fail ####\n");

                        // send zero size, finish
                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x03;
                        ret_buf[2] = 0x00;
                        ret_buf[3] = 0x00;
                        ret_buf[4] = 0x00;
                        ret_buf[5] = 0x00;
                        ret_buf[6] = 0xFD;
                        ret_buf[7] = 0xAF;
                        printf("send size 0, finish\n");
                        send(forClientSockfd, ret_buf, 8, 0);

                        break;
                    }

                    // get ErrLog file date
                    memset(buf, 0x00, 128);
                    while ( fgets(buf, 128, pdate_fd) != NULL ) {
                        if ( strlen(buf) == 0 )
                            break;
                        // set '\n' to 0
                        buf[strlen(buf)-1] = 0;
                        sscanf(buf, "%d", &logdate);
                        //printf("Get LogDate = %d\n", logdate);

                        if ( logdate < cmddate )
                            continue;

                        sprintf(buf, "cd %s/%d; ls > /tmp/Socket_ErrLogTime", ERRLOG_PATH, logdate);
                        //printf("buf cmd = %s\n", buf);
                        system(buf);

                        ptime_fd = fopen("/tmp/Socket_ErrLogTime", "rb");
                        if ( ptime_fd == NULL ) {
                            printf("#### Open /tmp/Socket_ErrLogTime Fail ####\n");
                            // some day error, continue;
                            continue;
                        }
                        // get log file time
                        memset(buf, 0x00, 128);
                        while ( fgets(buf, 128, ptime_fd) != NULL ) {
                            if ( strlen(buf) == 0 )
                                break;
                            // set '\n' to 0
                            buf[strlen(buf)-1] = 0;

                            memset(FILENAME, 0x00, 64);
                            sprintf(FILENAME, "%s/%d/%s", ERRLOG_PATH, logdate, buf);
                            printf("FILENAME = %s\n", FILENAME);

                            // check size
                            stat(FILENAME, &st);
                            size = st.st_size;

                            if ( size == 0 )
                                continue;

                            ret_buf[0] = 0xFA;
                            ret_buf[1] = 0x03;
                            ret_buf[2] = (unsigned char)((size >> 24) & 0x000000FF);
                            ret_buf[3] = (unsigned char)((size >> 16) & 0x000000FF);
                            ret_buf[4] = (unsigned char)((size >> 8) & 0x000000FF);
                            ret_buf[5] = (unsigned char)(size & 0x000000FF);
                            ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                            ret_buf[7] = 0xAF;
                            printf("send size num\n");
                            send(forClientSockfd, ret_buf, 8, 0);

                            // send list file
                            printf("send file %s\n", FILENAME);
                            send_file(forClientSockfd, FILENAME, size);
                        }
                        fclose(ptime_fd);

                    }
                    fclose(pdate_fd);

                    // send zero size, finish
                    ret_buf[0] = 0xFA;
                    ret_buf[1] = 0x03;
                    ret_buf[2] = 0x00;
                    ret_buf[3] = 0x00;
                    ret_buf[4] = 0x00;
                    ret_buf[5] = 0x00;
                    ret_buf[6] = 0xFD;
                    ret_buf[7] = 0xAF;
                    printf("send size 0, finish\n");
                    send(forClientSockfd, ret_buf, 8, 0);
                    break;

                case 0x04:
                    printf("cmd 04 ok. get data.\n");

                    cmddate = (cmd_buf[2] << 24) + (cmd_buf[3] << 16) + (cmd_buf[4] << 8) + cmd_buf[5];

                    // check usb file exist
                    if (stat(SYSLOG_PATH, &st) != 0) {
                        // send zero size, finish
                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x04;
                        ret_buf[2] = 0x00;
                        ret_buf[3] = 0x00;
                        ret_buf[4] = 0x00;
                        ret_buf[5] = 0x00;
                        ret_buf[6] = 0xFE;
                        ret_buf[7] = 0xAF;
                        printf("send size 0, finish\n");
                        send(forClientSockfd, ret_buf, 8, 0);

                        break;
                    }

                    // get SysLog file date list
                    sprintf(buf, "cd %s; ls > /tmp/Socket_SysLogDate", SYSLOG_PATH);
                    system(buf);

                    pdate_fd = fopen("/tmp/Socket_SysLogDate", "rb");
                    if ( pdate_fd == NULL ) {
                        printf("#### Open /tmp/Socket_SysLogDate Fail ####\n");

                        // send zero size, finish
                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x04;
                        ret_buf[2] = 0x00;
                        ret_buf[3] = 0x00;
                        ret_buf[4] = 0x00;
                        ret_buf[5] = 0x00;
                        ret_buf[6] = 0xFE;
                        ret_buf[7] = 0xAF;
                        printf("send size 0, finish\n");
                        send(forClientSockfd, ret_buf, 8, 0);

                        break;
                    }

                    // get SysLog file date
                    memset(buf, 0x00, 128);
                    while ( fgets(buf, 128, pdate_fd) != NULL ) {
                        if ( strlen(buf) == 0 )
                            break;
                        // set '\n' to 0
                        buf[strlen(buf)-1] = 0;
                        sscanf(buf, "%d", &logdate);
                        //printf("Get LogDate = %d\n", logdate);

                        if ( logdate < cmddate )
                            continue;

                        // get SysLog file time
                        sprintf(buf, "cd %s/%d; ls > /tmp/Socket_SysLogTime", SYSLOG_PATH, logdate);
                        //printf("buf cmd = %s\n", buf);
                        system(buf);

                        ptime_fd = fopen("/tmp/Socket_SysLogTime", "rb");
                        if ( ptime_fd == NULL ) {
                            printf("#### Open /tmp/Socket_SysLogTime Fail ####\n");
                            // some day error, continue;
                            continue;
                        }
                        // get log file time
                        memset(buf, 0x00, 128);
                        while ( fgets(buf, 128, ptime_fd) != NULL ) {
                            if ( strlen(buf) == 0 )
                                break;
                            // set '\n' to 0
                            buf[strlen(buf)-1] = 0;

                            memset(FILENAME, 0x00, 64);
                            sprintf(FILENAME, "%s/%d/%s", SYSLOG_PATH, logdate, buf);
                            printf("FILENAME = %s\n", FILENAME);

                            // check size
                            stat(FILENAME, &st);
                            size = st.st_size;

                            if ( size == 0 )
                                continue;

                            // send SysLog file date
                            ret_buf[0] = 0xFA;
                            ret_buf[1] = 0x04;
                            ret_buf[2] = (unsigned char)((logdate >> 24) & 0x000000FF);
                            ret_buf[3] = (unsigned char)((logdate >> 16) & 0x000000FF);
                            ret_buf[4] = (unsigned char)((logdate >> 8) & 0x000000FF);
                            ret_buf[5] = (unsigned char)(logdate & 0x000000FF);
                            ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                            ret_buf[7] = 0xAF;
                            printf("send date %d\n", logdate);
                            send(forClientSockfd, ret_buf, 8, 0);

                            ret_buf[0] = 0xFA;
                            ret_buf[1] = 0x04;
                            ret_buf[2] = (unsigned char)((size >> 24) & 0x000000FF);
                            ret_buf[3] = (unsigned char)((size >> 16) & 0x000000FF);
                            ret_buf[4] = (unsigned char)((size >> 8) & 0x000000FF);
                            ret_buf[5] = (unsigned char)(size & 0x000000FF);
                            ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                            ret_buf[7] = 0xAF;
                            printf("send size num %d\n", size);
                            send(forClientSockfd, ret_buf, 8, 0);

                            // send list file
                            printf("send file %s\n", FILENAME);
                            send_file(forClientSockfd, FILENAME, size);
                        }
                        fclose(ptime_fd);

                    }
                    fclose(pdate_fd);

                    // send zero size, finish
                    ret_buf[0] = 0xFA;
                    ret_buf[1] = 0x04;
                    ret_buf[2] = 0x00;
                    ret_buf[3] = 0x00;
                    ret_buf[4] = 0x00;
                    ret_buf[5] = 0x00;
                    ret_buf[6] = 0xFE;
                    ret_buf[7] = 0xAF;
                    printf("send size 0, finish\n");
                    send(forClientSockfd, ret_buf, 8, 0);
                    break;

                case 0x05:
                    printf("cmd 05 ok. get current data.\n");
                    get_config();
                    get_current_data();
                    ret = stat(HYBRID_TMP_DATA_PATH, &st);
                    if ( ret == 0 ) { // data exist
                        size = st.st_size;

                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x05;
                        ret_buf[2] = (unsigned char)((size >> 24) & 0x000000FF);
                        ret_buf[3] = (unsigned char)((size >> 16) & 0x000000FF);
                        ret_buf[4] = (unsigned char)((size >> 8) & 0x000000FF);
                        ret_buf[5] = (unsigned char)(size & 0x000000FF);
                        ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                        ret_buf[7] = 0xAF;
                        printf("send size num\n");
                        send(forClientSockfd, ret_buf, 8, 0);

                        // send list file
                        printf("send file %s\n", HYBRID_TMP_DATA_PATH);
                        send_file(forClientSockfd, HYBRID_TMP_DATA_PATH, size);

                    } else {
                        // send zero size
                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x05;
                        ret_buf[2] = 0x00;
                        ret_buf[3] = 0x00;
                        ret_buf[4] = 0x00;
                        ret_buf[5] = 0x00;
                        ret_buf[6] = 0xFF;
                        ret_buf[7] = 0xAF;
                        printf("send size 0\n");
                        send(forClientSockfd, ret_buf, 8, 0);
                        break;
                    }

                    ret = stat(HYBRID_TMP_ERROR_PATH, &st);
                    if ( ret == 0 ) { // data exist
                        size = st.st_size;

                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x05;
                        ret_buf[2] = (unsigned char)((size >> 24) & 0x000000FF);
                        ret_buf[3] = (unsigned char)((size >> 16) & 0x000000FF);
                        ret_buf[4] = (unsigned char)((size >> 8) & 0x000000FF);
                        ret_buf[5] = (unsigned char)(size & 0x000000FF);
                        ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                        ret_buf[7] = 0xAF;
                        printf("send size num\n");
                        send(forClientSockfd, ret_buf, 8, 0);

                        // send list file
                        printf("send file %s\n", HYBRID_TMP_ERROR_PATH);
                        send_file(forClientSockfd, HYBRID_TMP_ERROR_PATH, size);

                    } else {
                        // send zero size
                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x05;
                        ret_buf[2] = 0x00;
                        ret_buf[3] = 0x00;
                        ret_buf[4] = 0x00;
                        ret_buf[5] = 0x00;
                        ret_buf[6] = 0xFF;
                        ret_buf[7] = 0xAF;
                        printf("send size 0\n");
                        send(forClientSockfd, ret_buf, 8, 0);
                    }
                    break;

                case 0x06:
                    printf("cmd 06 ok. set data.\n");

                    unsigned char set_buf[1024] = {0};

                    totalsize = (cmd_buf[2] << 24) + (cmd_buf[3] << 16) + (cmd_buf[4] << 8) + cmd_buf[5];

                    // get data
                    pset_fd = fopen(HYBRID_TMP_SET_PATH, "wb");
                    if ( pset_fd == NULL ) {
                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x06;
                        ret_buf[2] = 0x00;
                        ret_buf[3] = 0x00;
                        ret_buf[4] = 0x00;
                        ret_buf[5] = 0x01;
                        ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                        ret_buf[7] = 0xAF;
                        printf("send result 1\n");
                        send(forClientSockfd, ret_buf, 8, 0);
                        break;
                    }

                    while (totalsize > 0) {
                        memset(set_buf, 0x00, 1024);
                        if (totalsize > 1024)
                            getsize = recv(forClientSockfd, set_buf, 1024, 0);
                        else
                            getsize = recv(forClientSockfd, set_buf, totalsize, 0);
                        printf("getsize = %d\n", getsize);
                        fwrite(set_buf, 1, getsize, pset_fd);
                        totalsize -= getsize;
                    }
                    fclose(pset_fd);
                    system("sync");

                    // paser parameter
                    char tmp_buf[1024] = {0};
                    char set_sn[17] = {0};
                    int start_addr = 0;
                    int data_count = 0;
                    int data_value = 0;
                    unsigned char set_data[32] = {0};
                    char *start_index = NULL;
                    int i = 0;

                    pset_fd = fopen(HYBRID_TMP_SET_PATH, "rb");
                    if ( pset_fd == NULL ) {
                        ret_buf[0] = 0xFA;
                        ret_buf[1] = 0x06;
                        ret_buf[2] = 0x00;
                        ret_buf[3] = 0x00;
                        ret_buf[4] = 0x00;
                        ret_buf[5] = 0x02;
                        ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                        ret_buf[7] = 0xAF;
                        printf("send result 2\n");
                        send(forClientSockfd, ret_buf, 8, 0);
                        break;
                    }
                    memset(tmp_buf, 0x00, 1024);
                    fgets(tmp_buf, 1024, pset_fd);
                    fclose(pset_fd);

                    start_index = tmp_buf;
                    // set sn
                    start_index = strstr(start_index, "<sn>");
                    strncpy(set_sn, start_index+4, 16);
                    printf("set_sn = %s\n", set_sn);
                    // set addr
                    start_index = strstr(start_index, "<StartingAddress>");
                    sscanf(start_index+17, "%d", &start_addr);
                    printf("start_addr = %d\n", start_addr);
                    // set count
                    start_index = strstr(start_index, "<DataCount>");
                    sscanf(start_index+11, "%d", &data_count);
                    printf("data_count = %d\n", data_count);
                    // set data
                    start_index = strstr(start_index, "<CommandValues>");
                    start_index += 15;
                    memset(set_data, 0x00, 32);
                    for (i = 0; i < data_count; i++) {
                        sscanf(start_index, "%02x", &data_value);
                        set_data[i*2] = (unsigned char)data_value;
                        start_index += 2;
                        sscanf(start_index, "%02x", &data_value);
                        set_data[i*2+1] = (unsigned char)data_value;
                        start_index += 2;
                    }
                    for (i = 0; i < 32; i++)
                        printf("set_data[%d] = %02X\n", i, set_data[i]);

                    // set data
                    ret = write_data(set_sn, start_addr, data_count, set_data);

                    // send result
                    ret_buf[0] = 0xFA;
                    ret_buf[1] = 0x06;
                    ret_buf[2] = 0x00;
                    ret_buf[3] = 0x00;
                    ret_buf[4] = 0x00;
                    ret_buf[5] = ret;
                    ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                    ret_buf[7] = 0xAF;
                    printf("send result %d\n", ret);
                    send(forClientSockfd, ret_buf, 8, 0);
                    break;

                case 0x99:
                    printf("cmd 99 ok. end.\n");

                    ret_buf[0] = 0xFA;
                    ret_buf[1] = 0x99;
                    ret_buf[2] = 0x00;
                    ret_buf[3] = 0x00;
                    ret_buf[4] = 0x00;
                    ret_buf[5] = 0x00;
                    ret_buf[6] = 0x93;
                    ret_buf[7] = 0xAF;
                    printf("send 0\n");
                    send(forClientSockfd, ret_buf, 8, 0);

                    if ( com_fd > 0 ) {
                        ModbusDrvDeinit(com_fd);
                        com_fd = 0;
                        // run other program
                        start_process();
                    }
                    break;

                default:
                    printf("Unknow function code: %02X\n", cmd_buf[1]);
                    break;
            }


        } else {
            // cmd error
            switch (cmd_buf[1]) {
                case 0x00:
                    printf("cmd 0, nothing to do.\n");
                    if ( com_fd > 0 ) {
                        ModbusDrvDeinit(com_fd);
                        com_fd = 0;
                        // run other program
                        start_process();
                    }
                    break;
                case 0x01:
                    printf("cmd 1 error\n");
                    ret_buf[0] = 0xFA;
                    ret_buf[1] = 0x01;
                    ret_buf[2] = 0xFF;
                    ret_buf[3] = 0xFF;
                    ret_buf[4] = 0xFF;
                    ret_buf[5] = 0xFF;
                    ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                    ret_buf[7] = 0xAF;
                    printf("send size 0xFFFFFFFF\n");
                    send(forClientSockfd, ret_buf, 8, 0);
                    break;
                case 0x02:
                    printf("cmd 2 error\n");
                    ret_buf[0] = 0xFA;
                    ret_buf[1] = 0x02;
                    ret_buf[2] = 0xFF;
                    ret_buf[3] = 0xFF;
                    ret_buf[4] = 0xFF;
                    ret_buf[5] = 0xFF;
                    ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                    ret_buf[7] = 0xAF;
                    printf("send size 0xFFFFFFFF\n");
                    send(forClientSockfd, ret_buf, 8, 0);
                    break;
                case 0x03:
                    printf("cmd 3 error\n");
                    ret_buf[0] = 0xFA;
                    ret_buf[1] = 0x03;
                    ret_buf[2] = 0xFF;
                    ret_buf[3] = 0xFF;
                    ret_buf[4] = 0xFF;
                    ret_buf[5] = 0xFF;
                    ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                    ret_buf[7] = 0xAF;
                    printf("send size 0xFFFFFFFF\n");
                    send(forClientSockfd, ret_buf, 8, 0);
                    break;
                case 0x04:
                    printf("cmd 4 error\n");
                    ret_buf[0] = 0xFA;
                    ret_buf[1] = 0x04;
                    ret_buf[2] = 0xFF;
                    ret_buf[3] = 0xFF;
                    ret_buf[4] = 0xFF;
                    ret_buf[5] = 0xFF;
                    ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                    ret_buf[7] = 0xAF;
                    printf("send size 0xFFFFFFFF\n");
                    send(forClientSockfd, ret_buf, 8, 0);
                    break;
                case 0x05:
                    printf("cmd 5 error\n");
                    ret_buf[0] = 0xFA;
                    ret_buf[1] = 0x05;
                    ret_buf[2] = 0xFF;
                    ret_buf[3] = 0xFF;
                    ret_buf[4] = 0xFF;
                    ret_buf[5] = 0xFF;
                    ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                    ret_buf[7] = 0xAF;
                    printf("send size 0xFFFFFFFF\n");
                    send(forClientSockfd, ret_buf, 8, 0);
                    break;
                case 0x06:
                    printf("cmd 6 error\n");
                    ret_buf[0] = 0xFA;
                    ret_buf[1] = 0x06;
                    ret_buf[2] = 0xFF;
                    ret_buf[3] = 0xFF;
                    ret_buf[4] = 0xFF;
                    ret_buf[5] = 0xFF;
                    ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                    ret_buf[7] = 0xAF;
                    printf("send size 0xFFFFFFFF\n");
                    send(forClientSockfd, ret_buf, 8, 0);
                    break;
                case 0x099:
                    printf("cmd 99 error\n");
                    ret_buf[0] = 0xFA;
                    ret_buf[1] = 0x99;
                    ret_buf[2] = 0xFF;
                    ret_buf[3] = 0xFF;
                    ret_buf[4] = 0xFF;
                    ret_buf[5] = 0xFF;
                    ret_buf[6] = ret_buf[0] + ret_buf[1] + ret_buf[2] + ret_buf[3] + ret_buf[4] + ret_buf[5];
                    ret_buf[7] = 0xAF;
                    printf("send size 0xFFFFFFFF\n");
                    send(forClientSockfd, ret_buf, 8, 0);
                    break;
                default:
                    printf("Unknow function code: %02X\n", cmd_buf[1]);
            }
        }

        close(forClientSockfd);
    }

    close(sockfd);

    return 0;
}
