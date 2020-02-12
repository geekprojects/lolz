
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <getopt.h>

#include "lolz.h"

using namespace std;

static const struct option g_options[] =
{
    { "config",  required_argument, NULL, 'c' },
    { NULL,      0,                 NULL, 0 }
};

static Lolz* g_lolz = NULL;

void signalHandler(int sig)
{
    if (g_lolz != NULL)
    {
        g_lolz->stop();
    }
}

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

    struct sigaction act;
    memset (&act, 0, sizeof(act));
    act.sa_handler = signalHandler;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);

    g_lolz = new Lolz();

    bool res;
    res = g_lolz->init(configPath);
    if (res)
    {
        g_lolz->run();
    }

    delete g_lolz;

    return 0;
}

