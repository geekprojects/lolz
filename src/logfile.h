#ifndef __LOLZ_LOG_FILE_H_
#define __LOLZ_LOG_FILE_H_

#include <string>
#include <vector>

#include <geek/core-logger.h>

class LogDirectory;

class LogFile : Geek::Logger
{
 private:
    LogDirectory* m_logDir;
    std::string m_path;

    int m_fd;
    uint64_t m_position;

 public:
    LogFile(LogDirectory* dir, std::string path);
    virtual ~LogFile();

    void load();
};

#endif
