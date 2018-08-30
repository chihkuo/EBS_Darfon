#ifndef SAVELOG_H_INCLUDED
#define SAVELOG_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

int OpenLog(char *path, struct tm *time);
int SaveLog(char *msg, struct tm *time);
void CloseLog();

#endif // SAVELOG_H_INCLUDED
