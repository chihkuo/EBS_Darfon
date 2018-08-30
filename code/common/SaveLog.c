#include "SaveLog.h"

FILE    *syslog_fd = NULL;

int OpenLog(char *path, struct tm *time)
{
    struct stat st;
    char buf[256] = {0};
    char tmp[256] = {0};

    if ( syslog_fd != NULL )
        return 1;

    sprintf(buf, "%s/%4d%02d%02d", path, 1900+time->tm_year, 1+time->tm_mon, time->tm_mday);
    if ( stat(buf, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", buf);
        if ( mkdir(buf, 0755) == -1 )
            printf("mkdir %s fail!\n", buf);
        else
            printf("mkdir %s OK\n", buf);
    }
    sprintf(tmp, "%s/%02d", buf, time->tm_hour);
    syslog_fd = fopen(tmp, "ab");
    if ( syslog_fd == NULL ) {
        printf("SaveLog() open %s fail!\n", tmp);
        return 2;
    }

    return 0;
}

int SaveLog(char *msg, struct tm *time)
{
    char buf[256] = {0};

    memset(buf, 0x00, 256);
    sprintf(buf, "%02d:%02d:%02d, %s\n", time->tm_hour, time->tm_min, time->tm_sec, msg);
    fputs(buf, syslog_fd);

    return 0;
}

void CloseLog()
{
    if ( syslog_fd != NULL )
        fclose(syslog_fd);
    syslog_fd = NULL;

    return;
}
