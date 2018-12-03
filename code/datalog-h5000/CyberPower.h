#ifndef CYBERPOWER_H
#define CYBERPOWER_H

#include "string.h"
#include "gdefine.h"

#include <time.h>

#define CP_LOG_BUF_SIZE    1024

typedef struct cyberpower_SN
{
    char SN[17];
}CP_SN;

typedef struct cyberpower_1PPowerInfo
{
    int Today_KWH;
    int System_Life_KWH;
    int Input1_Volt;
    int Input1_Curr;
    int Input1_Watt;
    int Input2_Volt;
    int Input2_Curr;
    int Input2_Watt;
    int AC_Volt;
    int AC_Curr;
    int AC_Watt;
    int AC_Freq;
}CP_1PPI;

typedef struct cyberpower_ErrorCode
{
    int E1_E00_15;
    int E2_E16_31;
    int E3_W00_15;
    int E4_F00_15;
    int E5_F16_31;
    int E6_F32_47;
    int E7_F48_63;
    int E8_F64_79;
}CP_EC;

typedef struct cyberpower_ErrorCode_E
{
    char E00_No_Grid;
    char E01_AC_Over_Frequency_A;
    char E02_AC_Under_Frequency_A;
    char E03_AC_Over_Volt_A;
    char E04_AC_Under_Volt_A;
    char E09_Grid_Quality;
    char E11_AC_Over_Frequency_B;
    char E12_AC_Under_Frequency_B;
    char E13_AC_Over_Volt_B;
    char E14_AC_Under_Volt_B;
    char E15_AC_Over_Frequency_C;
    char E16_AC_Under_Frequency_C;
    char E17_AC_Over_Volt_C;
    char E18_AC_Under_Volt_C;
}CP_EC_E;

typedef struct cyberpower_ErrorCode_W
{
    char W00_Input1_Under_Volt;
    char W01_Input2_Under_Volt;
    char W02_Input3_Under_Volt;
    char W03_Input4_Under_Volt;
    char W04_Input1_Over_Volt;
    char W05_Input2_Over_Volt;
    char W06_Input3_Over_Volt;
    char W07_Input4_Over_Volt;
    char W10_HW_Fan;
}CP_EC_W;

typedef struct cyberpower_ErrorCode_F
{
    char F01_Component_above_limit_A;
    char F02_Component_above_limit_B;
    char F03_Component_above_limit_C;
    char F05_Under_Temp;
    char F06_NTC1_Over_Temp;
    char F07_HW_NTC1_Failure;
    char F08_NTC2_Over_Temp;
    char F09_HW_NTC2_Failure;
    char F10_NTC3_Over_Temp;
    char F11_HW_NTC3_Failure;
    char F12_NTC4_Over_Temp;
    char F13_HW_NTC4_Failure;
    char F14_Firmware_are_not_compatible;
    char F15_HW_DSP_ADC1;
    char F16_HW_DSP_ADC2;
    char F17_HW_DSP_ADC3;
    char F18_HW_DSP_ADC4;
    char F19_HW_Red_ADC1;
    char F20_HW_Efficiency;
    char F21_HW_FAN;
    char F22_HW_COMM1;
    char F23_HW_COMM2;
    char F24_Ground_Current;
    char F25_RCMU_Fail;
    char F26_Insulation;
    char F28_HW_Relay_Short;
    char F29_HW_Relay_Open;
    char F30_Bus_Unbalance;
    char F31_HW_Bus1_OVR;
    char F32_HW_Bus1_UVR;
    char F33_HW_Bus2_OVR;
    char F34_HW_Bus2_UVR;
    char F35_HW_Bus_OVR;
    char F36_AC_Over_Current_A_transient;
    char F37_AC_Over_Current_A;
    char F38_AC_Over_Current_B_transient;
    char F39_AC_Over_Current_B;
    char F40_AC_Over_Current_C_transient;
    char F41_AC_Over_Current_C;
    char F42_HW_CT_Fail_A;
    char F43_HW_CT_Fail_B;
    char F44_HW_CT_Fail_C;
    char F45_HW_AC_OCR;
    char F60_Input1_Over_Current;
    char F61_Input2_Over_Current;
    char F70_Input1_Over_Current_transient;
    char F71_Input2_Over_Current_transient;
}CP_EC_F;

class CyberPower
{
    public:
        CyberPower();
        virtual ~CyberPower();

        int     Init(int com, bool open_com, bool first, int busfd);

        void    Get1PData(int addr, int devid, time_t data_time, bool first, bool last);
        //void    Get3PData(int addr, time_t data_time, bool first, bool last);

    protected:
        void        GetMAC();
        bool        GetDLConfig();
        bool        SetPath();

        void        CleanParameter();
        bool        CheckConfig();

        bool        GetTimezone();
        void        SetTimezone(char *zonename, char *timazone);
        void        GetNTPTime();
        void        GetLocalTime();

        bool        GetSN();
        void        DumpSN(unsigned char *buf);
        bool        Get1PPowerInfo();
        void        Dump1PPowerInfo(unsigned char *buf);
        bool        GetErrorCode();
        void        DumpErrorCode(unsigned char *buf);
        void        ParserErrorCode();

        void        SetLogXML();
        bool        WriteLogXML();
        bool        SaveLogXML(bool first, bool last);
        void        SetErrorLogXML();
        bool        WriteErrorLogXML();
        bool        SaveErrorLogXML(bool first, bool last);
        void        SetEnvXML();
        bool        SaveEnvXML(bool first, bool last);

        bool        SaveDeviceList(bool first, bool last, int device);
        bool        WriteMIListXML(bool first, bool last, int device);

        int         m_addr;
        int         m_devid;
        int         m_busfd;
        int         m_milist_size;
        int         m_loopflag;
        int         m_get_error;
        int         m_sys_error;
        bool        m_do_get_TZ;
        struct tm   *m_st_time;
        time_t      m_current_time;

        CP_SN       m_cp_sn;
        CP_1PPI     m_cp_1ppi;
        CP_EC       m_cp_ec;
        CP_EC_E     m_cp_ec_e;
        CP_EC_W     m_cp_ec_w;
        CP_EC_F     m_cp_ec_f;

        DL_CONFIG   m_dl_config;
        DL_PATH     m_dl_path;

        char        m_log_buf[CP_LOG_BUF_SIZE];
        char        m_log_filename[128];
        char        m_errlog_buf[CP_LOG_BUF_SIZE];
        char        m_errlog_filename[128];
        char        m_env_filename[128];

    private:
};

#endif // CYBERPOWER_H
