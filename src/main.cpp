
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "logfile.h"
#include "logdirectory.h"

using namespace std;
using namespace Geek;

int main(int argc, char** argv)
{
    LogDirectory* dir = new LogDirectory("/Users/ian/projects/lolz/logs");
    //LogDirectory* dir = new LogDirectory("/var/log");
    dir->watch();

    while (true)
    {
        sleep(1);
    }

    return 0;
}

