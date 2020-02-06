#ifndef __LOLZ_LOG_FILE_H_
#define __LOLZ_LOG_FILE_H_

#include <string>
#include <vector>

#include <geek/core-logger.h>
#include <geek/core-thread.h>
#include <geek/core-data.h>

class LogDirectory;

class LogFile : Geek::Logger
{
 private:
    uint64_t m_id;
    LogDirectory* m_logDir;
    std::string m_path;

    Geek::Mutex* m_queueMutex;
    std::vector<Geek::Data*> m_queue;

    int m_fd;
    uint64_t m_position;
    uint64_t m_timestamp;

 public:
    LogFile(LogDirectory* dir, std::string path);
    virtual ~LogFile();

    void setId(uint64_t id) { m_id = id; }
    uint64_t getId() { return m_id; }
    LogDirectory* getDirectory() { return m_logDir; }
    std::string getPath() { return m_path; }

    void setPosition(uint64_t position) { m_position = position; }
    uint64_t getPosition() { return m_position; }
    void setTimestamp(uint64_t timestamp) { m_timestamp = timestamp; }
    uint64_t getTimestamp() { return m_timestamp; }

    Geek::Mutex* getQueueMutex() { return m_queueMutex; }
    std::vector<Geek::Data*>& getQueue() { return m_queue; }
    void clearQueue() { m_queue.clear(); }

    bool init();
    void load();
};

#endif
