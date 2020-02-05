
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <geek/core-data.h>

#include "logfile.h"

using namespace std;
using namespace Geek;

LogFile::LogFile(LogDirectory* dir, std::string path) : Logger("LogFile[" + path + "]")
{
    m_logDir = dir;
    m_path = path;

    m_fd = -1;
    m_position = 0;
}

LogFile::~LogFile()
{
}

void LogFile::load()
{
    if (m_fd == -1)
    {
        m_fd = open(m_path.c_str(), O_RDONLY);
        log(INFO, "load: I'm in ur %s, findin ur bugz", m_path.c_str());
    }

    Data* data = new Data();
    uint8_t buf[4096];
    while (true)
    {
        int res;
        res = read(m_fd, buf, 4096);
        if (res == -1)
        {
            error("load: Failed to read!");
            return;
        }
        else if (res == 0)
        {
            break;
        }
        log(INFO, "load: Read %d bytes", res);
        m_position += res;
        data->append(buf, res);
    }
    log(INFO, "load: Done, position=%lld", m_position);

    while (!data->eof())
    {
        string line = data->readLine();
        log(INFO, "load: LINE: %s", line.c_str());
    }

    delete data;
}

