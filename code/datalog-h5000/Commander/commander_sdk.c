//This file is for RELEASE IN SDK COPY
#ifdef SDK_RELEASE
#include "commander.h"


/**********************************************************************************************/
// cmd_jobs_sdk can be added some member datas to increase extra applications.
// The first parameter is the application's name.
// The second parameter is relating to handle_helloworld.c for initialization.
// The rest is no need to set.
//
/**********************************************************************************************/
CMD_JOBS_T cmd_jobs_sdk[]=
{
//	{"helloworld", init_helloworld, NULL, 0, 0},
        {"darfonlogger", init_darfonlogger, NULL, 0, 0},//miketest
    {0,0,0,0,0}
};

#endif
