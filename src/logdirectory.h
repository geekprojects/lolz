#ifndef __LOLZ_LOG_DIRECTORY_H_
#define __LOLZ_LOG_DIRECTORY_H_

#include <string>
#include <vector>
#include <map>

#include <geek/core-logger.h>

class LogDirectory : Geek::Logger
{
 private:
    std::string m_path;
    std::map<std::string, LogFile*> m_logFiles;

    void scan(std::string dir);
    LogFile* addFile(std::string path);
    LogFile* findFile(std::string path);

 public:
    LogDirectory(std::string path);
    virtual ~LogDirectory();

    void watch();

    void fileUpdated(std::string path);
};

#endif
