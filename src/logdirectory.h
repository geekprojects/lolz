#ifndef __LOLZ_LOG_DIRECTORY_H_
#define __LOLZ_LOG_DIRECTORY_H_

#include <string>
#include <vector>
#include <map>

#include <geek/core-logger.h>
#include <geek/core-thread.h>

#include <libfswatch/c++/monitor.hpp>

class Lolz;
class LogFile;
class MonitorThread;

class LogDirectory : Geek::Logger, public Geek::Thread
{
 private:
    Lolz* m_lolz;
    uint64_t m_id;
    std::string m_path;
    std::map<std::string, LogFile*> m_logFiles;

    fsw::monitor* m_monitor;
    MonitorThread* m_monitorThread;

    Geek::CondVar* m_signal;

    void scan(std::string dir);
    LogFile* addFile(std::string path);
    LogFile* findFile(std::string path);

 public:
    LogDirectory(Lolz* lolz, uint64_t id, std::string path);
    virtual ~LogDirectory();

    uint64_t getId() { return m_id; }
    std::string getPath() { return m_path; }

    fsw::monitor* getMonitor() { return m_monitor; }
    void watch();

    virtual bool main();

    void fileUpdated(std::string path);

    void signal();
};

class MonitorThread : public Geek::Thread
{
 private:
    LogDirectory* m_logDir;

 public:
    MonitorThread(LogDirectory* logDir);
    ~MonitorThread();

    virtual bool main();
};


#endif
