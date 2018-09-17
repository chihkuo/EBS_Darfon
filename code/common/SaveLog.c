#include "SaveLog.h"

FILE    *syslog_fd = NULL;

int OpenLog(char *path, struct tm *time)
{
    struct stat st;
    char buf[256] = {0};
    char tmp[256] = {0};

    if ( syslog_fd != NULL )
        return 1;

    // check log dir
    if ( stat(path, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", path);
        if ( mkdir(path, 0755) == -1 ) {
            printf("mkdir %s fail!\n", path);
            return 2;
        }
        else
            printf("mkdir %s OK\n", path);
    }

    // create date dir
    sprintf(buf, "%s/%4d%02d%02d", path, 1900+time->tm_year, 1+time->tm_mon, time->tm_mday);
    if ( stat(buf, &st) == -1 ) {
        printf("%s not exist, run mkdir!\n", buf);
        if ( mkdir(buf, 0755) == -1 ) {
            printf("mkdir %s fail!\n", buf);
            return 3;
        }
        else
            printf("mkdir %s OK\n", buf);
    }
    sprintf(tmp, "%s/%02d", buf, time->tm_hour);
    syslog_fd = fopen(tmp, "ab");
    if ( syslog_fd == NULL ) {
        printf("SaveLog() open %s fail!\n", tmp);

        if ( stat("/tmp/test/SYSLOG", &st) == -1 ) {
            if ( mkdir("/tmp/test/SYSLOG", 0755) == -1 ) {
                printf("mkdir /tmp/test/SYSLOG fail!\n");
                return 4;
            }
            else
                printf("mkdir %s OK\n", buf);
        }

        sprintf(buf, "/tmp/test/SYSLOG/%4d%02d%02d", 1900+time->tm_year, 1+time->tm_mon, time->tm_mday);
        if ( stat(buf, &st) == -1 ) {
            printf("%s not exist, run mkdir!\n", buf);
            if ( mkdir(buf, 0755) == -1 ) {
                printf("mkdir %s fail!\n", buf);
                return 5;
            }
            else
                printf("mkdir %s OK\n", buf);
        }
        sprintf(tmp, "%s/%02d", buf, time->tm_hour);
        syslog_fd = fopen(tmp, "ab");

        return 6;
    }

    return 0;
}

int SaveLog(char *msg, struct tm *time)
{
    char buf[256] = {0};

    if ( syslog_fd != NULL ) {
        memset(buf, 0x00, 256);
        sprintf(buf, "%02d:%02d:%02d, %s\n", time->tm_hour, time->tm_min, time->tm_sec, msg);
        fputs(buf, syslog_fd);
        return 0;
    } else
        return 1;
}

void CloseLog()
{
    if ( syslog_fd != NULL )
        fclose(syslog_fd);
    syslog_fd = NULL;

    return;
}
