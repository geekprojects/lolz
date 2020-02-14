
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <inttypes.h>

#include <geek/core-data.h>

#include "logfile.h"
#include "logdirectory.h"

using namespace std;
using namespace Geek;

LogFile::LogFile(LogDirectory* dir, const string path) : Logger("LogFile[" + path + "]")
{
    m_id = 0;
    m_logDir = dir;
    m_path = path;
    m_ignore = false;

    m_fd = -1;
    m_position = 0;
    m_timestamp = 0;

    m_queueMutex = Thread::createMutex();
}

LogFile::~LogFile()
{
    delete m_queueMutex;
}

bool LogFile::init()
{
    m_fd = open(m_path.c_str(), O_RDONLY);
    if (m_fd == -1)
    {
        // Oh noes, it no work
        return false;
    }

    int res;
    struct stat s;
    res = fstat(m_fd, &s);
    if (res == -1)
    {
        close(m_fd);
        return false;
    }

#ifdef _DARWIN_FEATURE_64_BIT_INODE
    uint64_t birth = s.st_birthtimespec.tv_sec;
    if (birth > 0 && m_position > 0 && (m_timestamp / 1000) < birth)
    {
        log(WARN, "load: NO WAI! FILE REPLACED!");
        m_position = 0;
    }
#endif

    if ((uint64_t)s.st_size < m_position)
    {
        log(WARN, "load: NO WAI! FILE SHORTR THAN ME THINK!");
        m_position = 0;
    }

    log(INFO, "load: I'm in ur log, findin ur bugz");
    if (m_position > 0)
    {
        log(INFO, "load: I no this file! I is skippin to %" PRIu64, m_position);
        lseek(m_fd, m_position, SEEK_SET);
    }

    return true;
}

void LogFile::load()
{
    time_t timestamp = time(NULL);

    Data* data = new Data();
    uint64_t count = 0;
    while (true)
    {
        int res;
        uint8_t buf[4096];
        res = read(m_fd, buf, 4096);
        if (res == -1)
        {
            error("load: Oh noes!");
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
        LogEvent event;
        event.data = data;
        event.timestamp = timestamp;

        m_queueMutex->lock();
        m_position += count;
        log(INFO, "load: I haz %" PRIu64 " moar bytes n stuff", count);
        m_queue.push_back(event);
        m_queueMutex->unlock();

        m_logDir->signal();
    }
    else
    {
        delete data;
    }
}

