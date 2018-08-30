#include "datalog.h"
#include "G320.h"

#include <unistd.h>

#define VERSION "1.0.0"

//#include "log4z.h"
//using namespace zsummer::log4z;


void Start();

//void MyLog(char *lplog, int type=0);

using namespace std;

extern "C" {
   extern void    initdata();
   extern void    initenv(char *init_name);
   //extern  void MyStart();
}

int main(int argc, char* argv[])
{
    // ILog4zManager::GetInstance()->Start();

    char opt;
    while( (opt = getopt(argc, argv, "vV")) != -1 )
    {
        switch (opt)
        {
            case 'v':
            case 'V':
                printf("%s\n", VERSION);
                //printf("TIMESTAMP=%s\n", __TIMESTAMP__);
                return 0;
            case '?':
                return 1;
        }
    }

    Start();
    printf("press any key to stop!!");
    getchar();
    return 0;
}

void Start()
{
	//initdata(); // get mac address
	initenv((char *)"/usr/home/G320.ini");

	CG320  *pg320;
	pg320 = new CG320;
	pg320->Init();
	pg320->Start();

}

