#ifndef ADTEK_CS1_H
#define ADTEK_CS1_H

#include "string.h"
#include "gdefine.h"

#include <time.h>

#define AD_ENV_BUF_SIZE    1024

class ADtek_CS1
{
    public:
        ADtek_CS1();
        virtual ~ADtek_CS1();

        int     Init(int com, bool open_com, bool first, int busfd);

        void    GetEnv(int addr, int devid, time_t data_time, bool first, bool last);

    protected:
        void        GetMAC();
        bool        GetDLConfig();
        bool        SetPath();

        bool        CheckConfig();
        void        CleanParameter();

        bool        GetTimezone();
        void        SetTimezone(char *zonename, char *timazone);
        void        GetNTPTime();
        void        GetLocalTime();

        bool        GetTemp();
        void        DumpTemp(unsigned char *buf);
        bool        GetPoint();
        void        DumpPoint(unsigned char *buf);

        void        SetEnvXML();
        bool        WriteEnvXML();
        bool        SaveEnvXML(bool first, bool last);
        void        SetLogXML();
        bool        SaveLogXML(bool first, bool last);
        void        SetErrorLogXML();
        bool        SaveErrorLogXML(bool first, bool last);

        bool        WriteMIListXML(bool first, bool last);
        bool        SaveDeviceList(bool first, bool last);

        int         m_addr;
        int         m_devid;
        int         m_busfd;
        int         m_env_temp;
        int         m_env_point;
        int         m_milist_size;
        int         m_loopflag;
        int         m_sys_error;
        bool        m_do_get_TZ;
        struct tm   *m_st_time;
        time_t      m_current_time;

        DL_CONFIG   m_dl_config;
        DL_PATH     m_dl_path;

        char        m_env_buf[AD_ENV_BUF_SIZE];
        char        m_env_filename[128];
        char        m_log_filename[128];
        char        m_errlog_filename[128];

    private:
};

#endif // ADTEK_CS1_H
