#ifndef __LOLZ_LOLZ_H_
#define __LOLZ_LOLZ_H_

#include "logdirectory.h"

#include <geek/core-database.h>
#include <geek/core-data.h>
#include <geek/core-logger.h>
#include <geek/core-thread.h>

#include <yaml-cpp/yaml.h>

#include <map>
#include <string>

class Lolz : Geek::Logger
{
 private:
    std::map<std::string, LogDirectory*> m_dirs;
    Geek::Core::Database* m_db;

    YAML::Node m_config;

    Geek::CondVar* m_runningVar;

 public:
    Lolz();
    ~Lolz();

    bool init(std::string configPath);
    bool run();
    bool stop();

    void addDirectory(std::string path, YAML::Node config);
    void addLogFile(LogFile* logFile);
    void updateLogFile(LogFile* logFile, uint64_t position);
    void logEvents(LogFile* logFile, time_t timestamp, Geek::Data* data);

    static uint64_t getTimestamp();
};

#endif
