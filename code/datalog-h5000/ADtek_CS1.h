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

        bool    Init(int com, bool open_com, bool first);

    protected:
        void        GetMAC();
        bool        GetDLConfig();
        bool        SetPath();

        bool        GetTimezone();
        void        SetTimezone(char *zonename, char *timazone);
        void        GetNTPTime();
        void        GetLocalTime();

        int         m_sys_error;
        bool        m_do_get_TZ;
        struct tm   *m_st_time;
        time_t      m_current_time;

        DL_CONFIG   m_dl_config;
        DL_PATH     m_dl_path;

    private:
};

#endif // ADTEK_CS1_H
