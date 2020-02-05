
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lolz.h"

using namespace std;

int main(int argc, char** argv)
{
    Lolz* lolz = new Lolz();

    lolz->init();
    lolz->run();

    delete lolz;

    return 0;
}

