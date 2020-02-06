#ifndef __LOLZ_LOLZ_H_
#define __LOLZ_LOLZ_H_

#include "logdirectory.h"

#include <geek/core-database.h>
#include <geek/core-data.h>
#include <geek/core-logger.h>

#include <map>
#include <string>

class Lolz : Geek::Logger
{
 private:
    std::map<std::string, LogDirectory*> m_dirs;
    Geek::Core::Database* m_db;

 public:
    Lolz();
    ~Lolz();

    bool init();
    bool run();

    void addDirectory(std::string path);
    void addLogFile(LogFile* logFile);
    void updateLogFile(LogFile* logFile, uint64_t position);
    void logEvents(LogFile* logFile, Geek::Data* data);

    static uint64_t getTimestamp();
};

#endif
