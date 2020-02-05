
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <geek/core-data.h>

#include "logfile.h"
#include "logdirectory.h"

using namespace std;
using namespace Geek;

LogFile::LogFile(LogDirectory* dir, std::string path) : Logger("LogFile[" + path + "]")
{
    m_logDir = dir;
    m_path = path;

    m_fd = -1;
    m_position = 0;

    m_queueMutex = Thread::createMutex();
}

LogFile::~LogFile()
{
}

void LogFile::init()
{
    m_fd = open(m_path.c_str(), O_RDONLY);
    log(INFO, "load: I'm in ur %s, findin ur bugz", m_path.c_str());
    if (m_position > 0)
    {
        log(INFO, "load: I is skippin to %lld", m_position);
        lseek(m_fd, m_position, SEEK_SET);
    }
}

void LogFile::load()
{
    Data* data = new Data();
    uint8_t buf[4096];
    uint64_t count = 0;
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
        count += res;
        data->append(buf, res);
    }

    if (count > 0)
    {
        m_dirty = true;

        m_queueMutex->lock();
        m_position += count;
        log(INFO, "load: I haz %lld moar bytes!", count);
        m_queue.push_back(data);
        m_queueMutex->unlock();

        m_logDir->signal();
    }
    else
    {
        delete data;
    }
}

