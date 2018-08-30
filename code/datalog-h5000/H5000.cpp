#include "datalog.h"
#include "H5000.h"
#include <unistd.h>
#include <time.h>

extern "C" {
   extern int    MyModbusDrvInit(char *szport);
   extern void    RemoveAllRegister(int times);
   extern int    MyStartRegisterProcess(byte *psn);
   extern int     ModbusDrvDeinit();
   extern void    RemoveRegisterQuery(byte byAddr);
   extern int    MySyncOffLineQuery(byte addr, byte MOD, byte buf[]);
   extern int  MyOffLineQuery(byte addr, byte  buf[]);
   extern int   MyAssignAddress(unsigned char *ID, unsigned char Addr);
}

CH5000::CH5000()
{
    m_snCount= 0;
    m_hb_iddata = {0};
    m_hb_idflags = {0};
    m_hb_rtcdata = {0};
    m_hb_rsinfo = {0};
}

CH5000::~CH5000()
{

}

void CH5000::Init()
{

}

void CH5000::Start()
{
    char *port;

    port = szPort[g_dlData.g_port];

    char szbuf[32];
    sprintf(szbuf,"port = %s \n",port);
    printf(szbuf);

    MyModbusDrvInit(port);

    char   szmsg[200];
    int    idc, i;
    printf("\n================================\n");
    printf("StartRegisterProcess() Start!\n");
    printf("================================\n");
    if (idc = StartRegisterProcess()) {
        sprintf(szmsg, "StartRegisterProcess success find %d invert\n", idc);
        printf(szmsg);
    } else {
        printf("StartRegisterProcess fail\n");
        return;
    }
    printf("================================\n");
    printf("StartRegisterProcess() End!\n");
    printf("================================\n");

    while (1) {

        for (i=0; i<idc; i++) {
            //MyWriteAllMIDataToRAM(arySNobj[i].m_Addr);
            if ( GetIDData(i) )
                ; // do something
            //else
            //    if ( ReRegiser(arySNobj[i].m_Addr) ) // do something
            //        GetIDData(arySNobj[i].m_Addr);
            //    else
            //        printf("ReRegiser(%d) Fail\n", i);

            /*if ( SetRTCData(i) )
                ; // do something
            else
                if ( ReRegiser(i) ) // do something
                    SetRTCData(i);
                else
                    printf("ReRegiser(%d) Fail\n", i);*/

            if ( GetRTCData(i) )
                ; // do something
            //else
            //    if ( ReRegiser(arySNobj[i].m_Addr) ) // do something
            //        GetRTCData(arySNobj[i].m_Addr);
            //    else
            //        printf("ReRegiser(%d) Fail\n", arySNobj[i].m_Addr);


            if ( GetRemoteSettingInfo(i) )
                ; // do something
            //else
            //    if ( ReRegiser(arySNobj[i].m_Addr) ) // do something
            //        GetRemoteSettingInfo(arySNobj[i].m_Addr);
            //    else
            //        printf("ReRegiser(%d) Fail\n", arySNobj[i].m_Addr);

            if ( GetRemoteRealtimeSettingInfo(i) )
                ; // do something
            //else
            //    if ( ReRegiser(arySNobj[i].m_Addr) ) // do something
            //        GetRemoteRealtimeSettingInfo(arySNobj[i].m_Addr);
            //    else
            //        printf("ReRegiser(%d) Fail\n", arySNobj[i].m_Addr);

                    GetRealTimeInfo(i);

                    GetBMSInfo(i);
        }
        usleep(1000000);

        system("date");
        printf("press any key to next loop~\n");
        getchar();
    }

	ModbusDrvDeinit();
}

void CH5000::Pause()
{

}
void CH5000::Play()
{

}
void CH5000::Stop()
{

}

void CH5000::WriteAllMIDataToRAM()
{
	int i,i2;
	byte *lpdata;

	//char tempData[100],MIROMData[1000],LastError[50],CommunicationStatus[50];
    //char csmanvalue[20];

    unsigned char  szMIinfo[]={0x03, 0x03, 0x00, 0x01, 0x00, 0x07, 0x00, 0x00};
    MakeReadDataCRC(szMIinfo,8);

    unsigned char  szMiData[]={0x03, 0x03, 0x02, 0x00, 0x00, 0x21, 0xc5, 0x98};

    MakeReadDataCRC(szMiData,8);
    txsize = 8;


    while (1) {
      memcpy(txbuffer, szMIinfo, 8);
	  MStartTX();
	  usleep(1000000);
	  lpdata = GetRespond(19, 200);
	  if (lpdata) {
	  //  DumpMiInfo(lpdata, &g_miInfo);
      //  free(lpdata);
	  } else {
	    break; // error
	  }
      memcpy(txbuffer, szMiData, 8);
	  MStartTX();
	  usleep(1000000);

	  lpdata = GetRespond(71, 200);

     // usleep(g_global.g_fetchtime*1000000);
    }
}

void CH5000::SendAllocatedAddress(char *uniID, unsigned int AllocatedAddress)
{
    byte  buf[1024];
    byte  messageZigbee[19+16];
    MClearRX();
    txsize=19;
    waitAddr=0x00;
    waitFCode = 0xff;
    unsigned short crc;
    txbuffer[0] = 0x00;
    txbuffer[1] = 0xff;
    txbuffer[2]= 0x00;
    txbuffer[3]= 0x01;
    txbuffer[4]= 0x00;
    txbuffer[5]= 0x05;
    txbuffer[6]= 0x0a;
    int i;
    for (i = 0; i < 8; i++)
    {
        txbuffer[i + 7] = *(uniID+i);
    }
    txbuffer[txsize - 4]= (unsigned char) (AllocatedAddress);
    txbuffer[txsize - 3]= (unsigned char) (AllocatedAddress);
    crc=CalculateCRC(&(txbuffer[0]), txsize-2);
    txbuffer[txsize - 2]= (unsigned char) (crc >> 8);
    txbuffer[txsize - 1]= (unsigned char) (crc&0x00ff);

    printf("SendAllocatedAddress\n");

    //printf(txbuffer);
    MStartTX();
}


int  CH5000::AssignAddress(char* ID)
{
    byte  *pdata;
    int errorcount=0;
    while(errorcount<3)
    {
        MClearRX();

     //   DEBUG2(printf("%s\n",ID));

        m_SlaveID=3;
        SendAllocatedAddress(ID, m_SlaveID);
       // usleep(1000000);
        pdata = GetRespond(8, 200);
        if (!pdata) {
            //free(pdata);
            errorcount++;
            return 1;
        }
    }
    return 0;
}

int CH5000::StartRegisterProcess()
{

	int RegisterLoopCount = 0;
    int DefaultMODValue = 20;
    int i;
	bool Conflict = false;
    char sz[32];

	if (m_snCount==0) {
        for (i=0; i<255; i++) {
            arySNobj[i].m_Addr=i+4;
            memset(arySNobj[i].m_Sn, 0x00, 17);
            arySNobj[i].m_Flag=0;
            //printf("i=%u, addr=%d\n",i,arySNobj[i].m_Addr );
        }
        //m_snCount=255;
	}
    //getchar();

	char byMOD = 3;//DefaultMODValue;
	unsigned char buffer[256];
	//char  a1[800];
	m_snCount = 0;

    RemoveRegisterQuery(0);//arySNobj[m_snCount].m_Addr);
    usleep(1000000);
    RemoveRegisterQuery(0);//arySNobj[m_snCount].m_Addr);
    usleep(1000000);
    RemoveRegisterQuery(0);//arySNobj[m_snCount].m_Addr);
    usleep(1000000);

    while ( 1 ) {
        if (MySyncOffLineQuery(0x00 /*arySNobj[m_snCount].m_Addr*/, byMOD, buffer)) {
            goto Allocate_address;
        } else {
            while ( m_snCount<255 && byMOD>=0 ) {
                if (MyOffLineQuery(0x00, buffer)) {
                    if ( !CheckCRC(buffer, 13) ) { // set Conflict condition
                        printf("#### Conflict! ####\n");
                        Conflict = true;
                        continue;
                    } else {
Allocate_address:
                        printf("#### Send allocate address parameter ####\n");
                        printf("m_snCount = %d\n", m_snCount);
                        printf("m_Addr = %d\n", arySNobj[m_snCount].m_Addr);
                        sprintf(arySNobj[m_snCount].m_Sn, "%02X%02X%02X%02X%02X%02X%02X%02X", buffer[3], buffer[4],
                                buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10]);
                        printf("m_Sn = %s\n", arySNobj[m_snCount].m_Sn);
                        printf("#########################################\n");
                        if ( MyAssignAddress(&buffer[3],  arySNobj[m_snCount].m_Addr) )
                        {
                            printf("=================================\n");
                            printf("#### MyAssignAddress(%d) OK! ####\n", arySNobj[m_snCount].m_Addr);
                            printf("=================================\n");
                            m_snCount++;
                        }
                        else
                            printf("#### MyAssignAddress(%d) fail! ####\n", arySNobj[m_snCount].m_Addr);
                    }
                    usleep(1000000);
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

    return m_snCount;
}

bool CH5000::ReRegiser(int index)
{
    // call MySyncOffLineQuery & MyAssignAddress OK?
    unsigned char buffer[256];
    int MOD = 20, i;
    bool Conflict = false;

    while ( 1 ) {
        if (MySyncOffLineQuery(index, MOD, buffer)) {
            goto Allocate_address;
        } else {
            while ( MOD>=0 ) {
                if (MyOffLineQuery(index, buffer)) {
                    if ( !CheckCRC(buffer, 13) ) { // set Conflict condition
                        printf("#### Conflict! ####\n");
                        Conflict = true;
                        continue;
                    } else {
Allocate_address:
                        printf("#### Reallocate address parameter ####\n");
                        printf("index = %d\n", index);
                        printf("m_Addr = %d\n", arySNobj[index].m_Addr);
                        sprintf(arySNobj[m_snCount].m_Sn, "%02X%02X%02X%02X%02X%02X%02X%02X", buffer[3], buffer[4],
                                buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10]);
                        printf("m_Sn = %s\n", arySNobj[m_snCount].m_Sn);
                        printf("#########################################\n");
                        if ( MyAssignAddress(&buffer[3],  arySNobj[index].m_Addr) )
                        {
                            printf("=================================\n");
                            printf("#### ReRegiser(%d) OK! ####\n", arySNobj[index].m_Addr);
                            printf("=================================\n");
                            return true;
                        }
                        else
                            printf("#### ReRegiser(%d) fail! ####\n", arySNobj[index].m_Addr);
                    }
                    usleep(1000000);
                }
                else
                    printf("#### ReRegiser(%d) No response! ####\n", arySNobj[index].m_Addr);

                MOD--;
                printf("================ MOD=%d ================\n",MOD);
            }
        }
        /*if (Conflict) {
                Conflict = false;
                if (DefaultMODValue == 20)
                    DefaultMODValue = 40;
                else if (DefaultMODValue == 40)
                    DefaultMODValue = 80;
                else if (DefaultMODValue == 80)
                    break;
                byMOD = DefaultMODValue;
        }
        else*/
            break;
    }

    return false;
}

bool CH5000::GetIDData(int index)
{
    printf("#### GetIDData Start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    MClearRX();

    unsigned char szIDData[]={0x00, 0x03, 0x00, 0x01, 0x00, 0x0E, 0x00, 0x00};
    szIDData[0]=arySNobj[index].m_Addr;
    MakeReadDataCRC(szIDData,8);
    txsize = 8;

    while ( err < 3 ) {
        memcpy(txbuffer, szIDData, 8);
        MStartTX();
        usleep(1000000);

        lpdata = GetRespond(33, 200);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 33) ) {
                printf("#### GetIDData OK ####\n");
                DumpIDData(lpdata+3);
                free(lpdata);
                return true;
            } else {
                printf("#### GetIDData CRC Error ####\n");
                err++;
            }
            free(lpdata);
        } else {
            printf("#### GetIDData No Response ####\n");
            err++;
        }

        usleep(1000000);
    }

    return false;
}

void CH5000::DumpIDData(byte *buf)
{
    m_hb_iddata.Grid_Voltage = (*(buf) << 8) + *(buf+1);
    m_hb_iddata.Model = (*(buf+2) << 8) + *(buf+3);
    m_hb_iddata.SN_Hi = (*(buf+4) << 8) + *(buf+5);
    m_hb_iddata.SN_Lo = (*(buf+6) << 8) + *(buf+7);
    m_hb_iddata.Year = (*(buf+8) << 8) + *(buf+9);
    m_hb_iddata.Month = (*(buf+10) << 8) + *(buf+11);
    m_hb_iddata.Date = (*(buf+12) << 8) + *(buf+13);
    m_hb_iddata.Inverter_Ver = (*(buf+14) << 8) + *(buf+15);
    m_hb_iddata.DD_Ver = (*(buf+16) << 8) + *(buf+17);
    m_hb_iddata.EEPROM_Ver = (*(buf+18) << 8) + *(buf+19);
    m_hb_iddata.Flags = (*(buf+26) << 8) + *(buf+27);
    unsigned char tmp = m_hb_iddata.Flags;
    m_hb_idflags.Rule21 = tmp%2;
    tmp/=2;
    m_hb_idflags.PVParallel = tmp%2;
    tmp/=2;
    m_hb_idflags.PVOffGrid = tmp%2;
    tmp/=2;
    m_hb_idflags.Heco1 = tmp%2;
    tmp/=2;
    m_hb_idflags.Heco2 = tmp%2;

    printf("#### Dump ID Data ####\n");
    printf("Grid Voltage = %d ==> ", m_hb_iddata.Grid_Voltage);
    switch (m_hb_iddata.Grid_Voltage)
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
    printf("Model = %d\n", m_hb_iddata.Model);
    printf("SN Hi = %d\n", m_hb_iddata.SN_Hi);
    printf("SN Lo = %d\n", m_hb_iddata.SN_Lo);
    printf("Years = %d\n", m_hb_iddata.Year);
    printf("Month = %d\n", m_hb_iddata.Month);
    printf("Data = %d\n", m_hb_iddata.Date);
    printf("Inverter Ver = %d\n", m_hb_iddata.Inverter_Ver);
    printf("DD Ver = %d\n", m_hb_iddata.DD_Ver);
    printf("EEPROM Ver = %d\n", m_hb_iddata.EEPROM_Ver);
    printf("Flags = %d\n", m_hb_iddata.Flags);
    printf("Rule21 = %d\n", m_hb_idflags.Rule21);
    printf("PV Parallel = %d\n", m_hb_idflags.PVParallel);
    printf("PV Off Grid = %d\n", m_hb_idflags.PVOffGrid);
    printf("Heco1 = %d\n", m_hb_idflags.Heco1);
    printf("Heco2 = %d\n", m_hb_idflags.Heco2);
    printf("#######################\n");
}

bool CH5000::GetRTCData(int index)
{
    printf("#### GetRTCData Start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    MClearRX();

    unsigned char szRTCData[]={0x00, 0x03, 0x00, 0x40, 0x00, 0x07, 0x00, 0x00};
    szRTCData[0]=arySNobj[index].m_Addr;
    MakeReadDataCRC(szRTCData,8);
    txsize = 8;

    while ( err < 3 ) {
        memcpy(txbuffer, szRTCData, 8);
        MStartTX();
        usleep(1000000);

        lpdata = GetRespond(19, 200);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 19) ) {
                printf("#### GetRTCData OK ####\n");
                DumpRTCData(lpdata+3);
                free(lpdata);
                return true;
            } else {
                printf("#### GetRTCData CRC Error ####\n");
                err++;
            }
            free(lpdata);
        } else {
            printf("#### GetRTCData No Response ####\n");
            err++;
        }

        usleep(1000000);
    }

    return false;
}

bool CH5000::SetRTCData(int index)
{
    printf("#### SetRTCData Start ####\n");
    printf("#### Get logger time ####\n");
    time_t timep;
    struct tm *ptm;
    time(&timep);
    /*ptm = gmtime(&timep); // get UTC time
    printf("gmtime : %4d/%02d/%02d ", (1900+ptm->tm_year),( 1+ptm-> tm_mon), ptm->tm_mday);
    printf("%s %02d:%02d:%02d\n", wday[ptm->tm_wday],ptm->tm_hour, ptm->tm_min, ptm->tm_sec);*/
    ptm = localtime(&timep); // get local time
    m_hb_rtcdata.Seconds = ptm->tm_sec;
    m_hb_rtcdata.Minutes = ptm->tm_min;
    m_hb_rtcdata.Hours = ptm->tm_hour;
    m_hb_rtcdata.Date = ptm->tm_mday;
    m_hb_rtcdata.Month = 1 + ptm->tm_mon; // ptm->tm_mon 0~11, m_hb_rtcdata.Month 1~12
    m_hb_rtcdata.Year = 1900 + ptm->tm_year;
    m_hb_rtcdata.Day = ptm->tm_wday;
    printf("localtime : %4d/%02d/%02d ", m_hb_rtcdata.Year, m_hb_rtcdata.Month, m_hb_rtcdata.Date);
    printf("%s %02d:%02d:%02d\n", wday[m_hb_rtcdata.Day], m_hb_rtcdata.Hours, m_hb_rtcdata.Minutes, m_hb_rtcdata.Seconds);

    MClearRX();

    int err = 0;
    unsigned short crc;
    byte *lpdata = NULL;

    unsigned char szRTCData[41]={0};
    szRTCData[0] = arySNobj[index].m_Addr;
    szRTCData[1] = 0x10; // function code
    szRTCData[2] = 0x00;
    szRTCData[3] = 0x40; // star address
    szRTCData[4] = 0x00;
    szRTCData[5] = 0x10; // number of data
    szRTCData[6] = 0x20; // bytes
    // data 0x40 ~ 0x46
    szRTCData[7] = 0x00;
    szRTCData[8] = m_hb_rtcdata.Seconds;
    szRTCData[9] = 0x00;
    szRTCData[10] = m_hb_rtcdata.Minutes;
    szRTCData[11] = 0x00;
    szRTCData[12] = m_hb_rtcdata.Hours;
    szRTCData[13] = 0x00;
    szRTCData[14] = m_hb_rtcdata.Date;
    szRTCData[15] = 0x00;
    szRTCData[16] = m_hb_rtcdata.Month;
    szRTCData[17] = m_hb_rtcdata.Year >> 8;
    szRTCData[18] = m_hb_rtcdata.Year & 0xff;
    szRTCData[19] = 0x00;
    szRTCData[20] = m_hb_rtcdata.Day;
    // zero 0x47 ~ 0x4E
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
    crc = CalculateCRC(szRTCData+7, 14);
    szRTCData[37] = (unsigned char) (crc >> 8); // data crc hi
    szRTCData[38] = (unsigned char) (crc & 0xff); // data crc lo
    szRTCData[39] = 0x00; // cmd crc hi
    szRTCData[40] = 0x00; // cmd crc lo
    MakeReadDataCRC(szRTCData,41);
    txsize = 41;

    while ( err < 3 ) {
        memcpy(txbuffer, szRTCData, 41);
        MStartTX();
        usleep(1000000);

        lpdata = GetRespond(8, 2000000);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 8) ) {
                printf("#### SetRTCData OK ####\n");
                free(lpdata);
                return true;
            } else {
                printf("#### SetRTCData CRC Error ####\n");
                err++;
            }
            free(lpdata);
        } else {
            printf("#### SetRTCData No Response ####\n");
            err++;
        }

        usleep(1000000);
    }

    return false;
}

void CH5000::DumpRTCData(byte *buf)
{
    m_hb_rtcdata.Seconds = (*(buf) << 8) + *(buf+1);
    m_hb_rtcdata.Minutes = (*(buf+2) << 8) + *(buf+3);
    m_hb_rtcdata.Hours = (*(buf+4) << 8) + *(buf+5);
    m_hb_rtcdata.Date = (*(buf+6) << 8) + *(buf+7);
    m_hb_rtcdata.Month = (*(buf+8) << 8) + *(buf+9);
    m_hb_rtcdata.Year = (*(buf+10) << 8) + *(buf+11);
    m_hb_rtcdata.Day = (*(buf+12) << 8) + *(buf+13);

    printf("#### Dump RTC Data ####\n");
    printf("Seconds = %d\n", m_hb_rtcdata.Seconds);
    printf("Minutes = %d\n", m_hb_rtcdata.Minutes);
    printf("Hours = %d\n", m_hb_rtcdata.Hours);
    printf("Date = %d\n", m_hb_rtcdata.Date);
    printf("Month = %d\n", m_hb_rtcdata.Month);
    printf("Year = %d\n", m_hb_rtcdata.Year);
    printf("Day = %d\n", m_hb_rtcdata.Day);
    printf("###############################\n");
    printf("rtc time : %4d/%02d/%02d ", m_hb_rtcdata.Year, m_hb_rtcdata.Month, m_hb_rtcdata.Date);
    printf("%02d:%02d:%02d\n", m_hb_rtcdata.Hours, m_hb_rtcdata.Minutes, m_hb_rtcdata.Seconds);
    printf("###############################\n");
}

bool CH5000::GetRemoteSettingInfo(int index)
{
    printf("#### GetRemoteSettingInfo Start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    MClearRX();

    unsigned char szRSInfo[]={0x00, 0x03, 0x00, 0x90, 0x00, 0x0E, 0x00, 0x00};
    szRSInfo[0]=arySNobj[index].m_Addr;
    MakeReadDataCRC(szRSInfo,8);
    txsize = 8;

    while ( err < 3 ) {
        memcpy(txbuffer, szRSInfo, 8);
        MStartTX();
        usleep(1000000);

        lpdata = GetRespond(33, 200);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 33) ) {
                printf("#### GetRemoteSettingInfo OK ####\n");
                DumpRemoteSettingInfo(lpdata+3);
                free(lpdata);
                return true;
            } else {
                printf("#### GetRemoteSettingInfo CRC Error ####\n");
                err++;
            }
            free(lpdata);
        } else {
            printf("#### GetRemoteSettingInfo No Response ####\n");
            err++;
        }

        usleep(1000000);
    }

    return false;
}

void CH5000::DumpRemoteSettingInfo(byte *buf)
{
    m_hb_rsinfo.Mode = (*(buf) << 8) + *(buf+1);
    m_hb_rsinfo.StarHour = (*(buf+2) << 8) + *(buf+3);
    m_hb_rsinfo.StarMin = (*(buf+4) << 8) + *(buf+5);
    m_hb_rsinfo.EndHour = (*(buf+6) << 8) + *(buf+7);
    m_hb_rsinfo.EndMin = (*(buf+8) << 8) + *(buf+9);
    m_hb_rsinfo.MultiModeSetting = (*(buf+10) << 8) + *(buf+11);
    m_hb_rsinfo.BatteryType = (*(buf+12) << 8) + *(buf+13);
    m_hb_rsinfo.BatteryCurrent = (*(buf+14) << 8) + *(buf+15);
    m_hb_rsinfo.BatteryShutdownVoltage = (float)((*(buf+16) << 8) + *(buf+17))/10;
    m_hb_rsinfo.BatteryFloatingVoltage = (float)((*(buf+18) << 8) + *(buf+19))/10;
    m_hb_rsinfo.BatteryReservePercentage = (*(buf+20) << 8) + *(buf+21);
    m_hb_rsinfo.RampRatePercentage = (*(buf+22) << 8) + *(buf+23);
    m_hb_rsinfo.VoltDividVAr = (*(buf+24) << 8) + *(buf+25);
    m_hb_rsinfo.DegreeLeadLag = (*(buf+26) << 8) + *(buf+27);
        // do parser parameter ABB, not sure format now

    printf("#### Dump Remote Setting Info ####\n");
    printf("Mode = %d ==> ", m_hb_rsinfo.Mode);
    switch (m_hb_rsinfo.Mode)
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
    }
    printf("StarHour = %d\n", m_hb_rsinfo.StarHour);
    printf("StarMin = %d\n", m_hb_rsinfo.StarMin);
    printf("EndHour = %d\n", m_hb_rsinfo.EndHour);
    printf("EndMin = %d\n", m_hb_rsinfo.EndMin);
    printf("Multi-Mode Setting = %d ==> ", m_hb_rsinfo.MultiModeSetting);
    switch (m_hb_rsinfo.MultiModeSetting)
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
    printf("Battery Type = %d ==> ", m_hb_rsinfo.BatteryType);
    switch (m_hb_rsinfo.BatteryType)
    {
        case 0:
            printf("No setting\n");
            break;
        case 1:
            printf("Lead-Acid\n");
            break;
        case 2:
            printf("Gloden Crown\n");
            break;
        case 3:
            printf("Darfon Growatt\n");
            break;
    }
    printf("Battery Current = %dA\n", m_hb_rsinfo.BatteryCurrent);
    printf("Battery Shutdown Voltage = %04.1fV\n", m_hb_rsinfo.BatteryShutdownVoltage);
    printf("Battery Floating Voltage = %04.1fV\n", m_hb_rsinfo.BatteryFloatingVoltage);
    printf("Battery Reserve Percentage = %d%%\n", m_hb_rsinfo.BatteryReservePercentage);
    printf("Ramp Rate Percentage = %d%%\n", m_hb_rsinfo.RampRatePercentage);
    printf("Volt/VAr Q(V) = %d\n", m_hb_rsinfo.VoltDividVAr);
    printf("Degree Lead/Lag = %d\n", m_hb_rsinfo.DegreeLeadLag);
    printf("A = %d, BB = %d\n", m_hb_rsinfo.DegreeLeadLag/100, m_hb_rsinfo.DegreeLeadLag%100);
    printf("##################################\n");
}

bool CH5000::GetRemoteRealtimeSettingInfo(int index)
{
    printf("#### GetRemoteRealtimeSettingInfo Start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    MClearRX();

    unsigned char szRRSInfo[]={0x00, 0x03, 0x00, 0xA0, 0x00, 0x02, 0x00, 0x00};
    szRRSInfo[0]=arySNobj[index].m_Addr;
    MakeReadDataCRC(szRRSInfo,8);
    txsize = 8;

    while ( err < 3 ) {
        memcpy(txbuffer, szRRSInfo, 8);
        MStartTX();
        usleep(1000000);

        lpdata = GetRespond(9, 200);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 9) ) {
                printf("#### GetRemoteRealtimeSettingInfo OK ####\n");
                DumpRemoteRealtimeSettingInfo(lpdata+3);
                free(lpdata);
                return true;
            } else {
                printf("#### GetRemoteRealtimeSettingInfo CRC Error ####\n");
                err++;
            }
            free(lpdata);
        } else {
            printf("#### GetRemoteRealtimeSettingInfo No Response ####\n");
            err++;
        }

        usleep(1000000);
    }

    return false;
}

void CH5000::DumpRemoteRealtimeSettingInfo(byte *buf)
{
    m_hb_rrsinfo.Charge = (*(buf) << 8) + *(buf+1);
    m_hb_rrsinfo.Grid = (*(buf+2) << 8) + *(buf+3);

    printf("#### Dump Remote Real-time Setting Info ####\n");
    printf("Charge = %d ==> ", m_hb_rrsinfo.Charge);
    switch (m_hb_rrsinfo.Charge)
    {
        case 0:
            printf("Charge\n");
            break;
        case 1:
            printf("DisCharge\n");
            break;
    }
    printf("Grid = %d ==> ", m_hb_rrsinfo.Grid);
    switch (m_hb_rrsinfo.Grid)
    {
        case 0:
            printf("Default\n");
            break;
        case 1:
            printf("On Grid Mode\n");
            break;
        case 2:
            printf("Off Grid Mode\n");
            break;
    }
    printf("##################################\n");
}

bool CH5000::GetRealTimeInfo(int index)
{
    printf("#### GetRealTimeInfo Start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    MClearRX();

    unsigned char szRTInfo[]={0x00, 0x03, 0x00, 0xB0, 0x00, 0x2D, 0x00, 0x00};
    szRTInfo[0]=arySNobj[index].m_Addr;
    MakeReadDataCRC(szRTInfo,8);
    txsize = 8;

    while ( err < 3 ) {
        memcpy(txbuffer, szRTInfo, 8);
        MStartTX();
        usleep(1000000);

        lpdata = GetRespond(95, 200);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 95) ) {
                printf("#### GetRealTimeInfo OK ####\n");
                //DumpRealTimeInfo(lpdata+3);
                free(lpdata);
                return true;
            } else {
                printf("#### GetRealTimeInfo CRC Error ####\n");
                err++;
            }
            free(lpdata);
        } else {
            printf("#### GetRealTimeInfo No Response ####\n");
            err++;
        }

        usleep(1000000);
    }

    return false;
}

bool CH5000::GetBMSInfo(int index)
{
    printf("#### GetBMSInfo Start ####\n");

    int err = 0;
    byte *lpdata = NULL;

    MClearRX();

    unsigned char szBMSInfo[]={0x00, 0x03, 0x02, 0x00, 0x00, 0x7D, 0x00, 0x00};
    szBMSInfo[0]=arySNobj[index].m_Addr;
    MakeReadDataCRC(szBMSInfo,8);
    txsize = 8;

    while ( err < 3 ) {
        memcpy(txbuffer, szBMSInfo, 8);
        MStartTX();
        usleep(1000000);

        lpdata = GetRespond(255, 200);
        if ( lpdata ) {
            if ( CheckCRC(lpdata, 255) ) {
                printf("#### GetBMSInfo OK ####\n");
                //DumpBMSInfo(lpdata+3);
                free(lpdata);
                return true;
            } else {
                printf("#### GetBMSInfo CRC Error ####\n");
                err++;
            }
            free(lpdata);
        } else {
            printf("#### GetBMSInfo No Response ####\n");
            err++;
        }

        usleep(1000000);
    }

    return false;
}
