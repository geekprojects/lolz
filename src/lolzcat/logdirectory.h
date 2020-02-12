#ifndef __LOLZ_LOG_DIRECTORY_H_
#define __LOLZ_LOG_DIRECTORY_H_

#include <string>
#include <vector>
#include <map>

#include <geek/core-logger.h>
#include <geek/core-thread.h>

#include <libfswatch/c++/monitor.hpp>
#include <yaml-cpp/yaml.h>

class Lolz;
class LogFile;
class MonitorThread;

class LogDirectory : Geek::Logger, public Geek::Thread
{
 private:
    Lolz* m_lolz;
    uint64_t m_id;
    std::string m_path;
    YAML::Node m_config;

    std::map<std::string, LogFile*> m_logFiles;

    fsw::monitor* m_monitor;
    MonitorThread* m_monitorThread;

    bool m_running;
    Geek::CondVar* m_signal;

    void scan(std::string dir);
    LogFile* addFile(std::string path);
    LogFile* findFile(std::string path);

    void checkLogQueues();

 public:
    LogDirectory(Lolz* lolz, uint64_t id, std::string path, YAML::Node config);
    virtual ~LogDirectory();

    uint64_t getId() { return m_id; }
    std::string getPath() { return m_path; }

    fsw::monitor* getMonitor() { return m_monitor; }
    void watch();

    virtual bool main();
    void stop();

    void fileUpdated(std::string path);
    void fileDeleted(std::string path);

    void signal();
};

class MonitorThread : public Geek::Thread, Geek::Logger
{
 private:
    LogDirectory* m_logDir;

 public:
    MonitorThread(LogDirectory* logDir);
    ~MonitorThread();

    virtual bool main();
};


#endif
