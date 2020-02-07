
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <getopt.h>

#include "lolz.h"

using namespace std;

static const struct option g_options[] =
{
    { "config",  required_argument, NULL, 'c' },
    { NULL,      0,                 NULL, 0 }
};

int main(int argc, char** argv)
{
    string configPath;
configPath = string(getenv("HOME")) + "/.lolz.yml";

    while (true)
    {
        int c = getopt_long(
            argc,
            argv,
            "c:",
            g_options,
            NULL);
if (c == -1)
{
break;
}
switch (c)
{
case 'c':
configPath = string(optarg);
break;

}
    }

printf("lolz: config=%s\n", configPath.c_str());

    Lolz* lolz = new Lolz();

    lolz->init(configPath);
    lolz->run();

    delete lolz;

    return 0;
}

