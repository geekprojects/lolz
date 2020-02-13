
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>

#include <string>
#include <vector>

#include <geek/core-logger.h>
#include <geek/core-data.h>

#include <libfswatch/c++/event.hpp>
#include <libfswatch/c++/monitor.hpp>

#ifdef HAS_FSW_MONITOR_FACTORY
#include <libfswatch/c++/monitor_factory.hpp>
#endif

#include "logfile.h"
#include "logdirectory.h"
#include "lolz.h"

using namespace std;
using namespace Geek;

LogDirectory::LogDirectory(Lolz* lolz, uint64_t id, std::string path, YAML::Node config)
    : Logger("LogDirectory[" + path + "]")
{
    m_lolz = lolz;
    m_id = id;
    m_path = path;
    m_config = config;

    m_monitor = NULL;
    m_monitorThread = NULL;

    m_running = false;
    m_signal = Thread::createCondVar();
}

LogDirectory::~LogDirectory()
{
    if (m_monitor != NULL)
    {
        delete m_monitor;
    }

    for (auto it : m_logFiles)
    {
        delete it.second;
    }

    delete m_monitorThread;
    delete m_signal;
}

void process_events(const vector<fsw::event>& events, void* context)
{
    LogDirectory* dir = (LogDirectory*)context;
    for (const fsw::event& evt : events)
    {
        string eventnames = "";
        bool isFile = true;
        bool isUpdated = false;
        bool isDeleted = false;
        bool isPlatformSpecific = false;
        for (fsw_event_flag flag : evt.get_flags())
        {
            string eventname = fsw::event::get_event_flag_name(flag);
            eventnames += eventname + ", ";
            switch (flag)
            {
                case IsDir:
                case IsSymLink:
                    // The inotify watcher doesn't seem to tell us if it *is* a file,
                    // just when it isn't :-(
                    isFile = false;
                    break;

                case Created:
                case Updated:
                case MovedTo:
                case Renamed:
                    isUpdated = true;
                    break;

                case Removed:
                case MovedFrom:
                    isDeleted = true;
                    break;

                case PlatformSpecific:
                    isPlatformSpecific = true;
                    break;

                default:
                    // Ignore
                    break;
            }
        }

        if (isPlatformSpecific)
        {
            continue;
        }

        if (isFile)
        {
            if (isUpdated)
            {
                dir->fileUpdated(evt.get_path());
            }
            else if (isDeleted)
            {
                dir->fileDeleted(evt.get_path());
            }
            else
            {
                printf("process_events: Unhandled event: %s: %s\n", evt.get_path().c_str(), eventnames.c_str());
            }
        }
    }
}

void LogDirectory::watch()
{
    scan(m_path);

    vector<string> paths;
    paths.push_back(m_path);
    m_monitor = fsw::monitor_factory::create_monitor(
        fsw_monitor_type::system_default_monitor_type,
        paths,
        process_events);

    m_monitor->set_context(this);
    m_monitor->set_recursive(true);

    m_monitorThread = new MonitorThread(this);
    m_monitorThread->start();
}

bool LogDirectory::main()
{
    m_running = true;

    checkLogQueues();
    while (m_running)
    {
        // Wait for more data
        m_signal->wait();

        checkLogQueues();
    }

    log(INFO, "main: Ok, stoppin");
    return true;
}

void LogDirectory::checkLogQueues()
{
    for (auto it : m_logFiles)
    {
        LogFile* logFile = it.second;
        logFile->getQueueMutex()->lock();

        uint64_t position = logFile->getPosition();

        vector<LogEvent> queue = logFile->getQueue();
        logFile->clearQueue();

        logFile->getQueueMutex()->unlock();

        if (!queue.empty())
        {
            for (LogEvent event : queue)
            {
                m_lolz->logEvents(logFile, event.timestamp, event.data);
                delete event.data;
            }
        }
        m_lolz->updateLogFile(logFile, position);
    }
}

void LogDirectory::stop()
{
    if (m_monitor != NULL)
    {
        m_monitor->stop();
    }

    m_running = false;
    m_signal->signal();
}

void LogDirectory::scan(std::string dir)
{
    log(INFO, "scan: scanning: %s", dir.c_str());
    DIR* fd = opendir(dir.c_str());
    if (fd == NULL)
    {
        error("Failed to scan directory: {}", dir.c_str());
        return;
    }

    while (true)
    {
        dirent* d = readdir(fd);
        if (d == NULL)
        {
            break;
        }
        if (d->d_name[0] == '.')
        {
            continue;
        }

        string entrypath = dir + string("/") + d->d_name;
        if (d->d_type == DT_REG)
        {
            LogFile* logFile = addFile(entrypath);
            if (logFile != NULL && !logFile->isIgnore())
            {
                logFile->load();
            }
        }
        else if (d->d_type == DT_DIR)
        {
            scan(entrypath);
        }
    }

    closedir(fd);
}

void LogDirectory::fileUpdated(std::string path)
{
    LogFile* logFile = addFile(path);
    if (logFile == NULL || logFile->isIgnore())
    {
        return;
    }
    logFile->load();
}

void LogDirectory::fileDeleted(std::string path)
{
    auto it = m_logFiles.find(path);
    if (it == m_logFiles.end())
    {
        // I no care
        return;
    }

    LogFile* logFile = it->second;
    log(INFO, "fileDeleted: KTHXBYE %s", path.c_str());
    m_lolz->updateLogFile(logFile, 0);
    m_logFiles.erase(it);

    delete logFile;
}

LogFile* LogDirectory::addFile(std::string path)
{
    LogFile* file = findFile(path);
    if (file != NULL)
    {
        return file;
    }

    string ext = "";
    string::size_type pos = path.rfind('.');
    if (pos != string::npos)
    {
        ext = path.substr(pos + 1);
    }

    string relativePath = path.substr(m_path.length() + 1);
    bool acceptFile = false;
    if (m_config["include"])
    {
        for (YAML::Node node : m_config["include"])
        {
            string include = node.as<std::string>();
            //log(DEBUG, "addFile: include: %s", include.c_str());
            int matches = fnmatch(include.c_str(), relativePath.c_str(), 0);//, FNM_PATHNAME);
            //log(DEBUG, "addFile: include: matches=%d\n", matches);
            if (matches != FNM_NOMATCH)
            {
                acceptFile = true;
                break;
            }
        }
    }
    else
    {
        acceptFile = true;
    }

    if (ext == "gz")
    {
        acceptFile = false;
    }

    file = new LogFile(this, path);

    if (acceptFile)
    {
        file->setIgnore(false);
        m_lolz->addLogFile(file);
        bool res;
        res = file->init();
        if (!res)
        {
            delete file;
            return NULL;
        }
    }
    else
    {
        file->setIgnore(true);
    }

    m_logFiles.insert(make_pair(path, file));

    return file;
}

LogFile* LogDirectory::findFile(std::string path)
{
    auto it = m_logFiles.find(path);
    if (it != m_logFiles.end())
    {
        return it->second;
    }
    return NULL;
}

void LogDirectory::signal()
{
    m_signal->signal();
}

MonitorThread::MonitorThread(LogDirectory* logDir) : Logger("MonitorThread[" + logDir->getPath() + "]")
{
    m_logDir = logDir;
}

MonitorThread::~MonitorThread()
{
}

bool MonitorThread::main()
{
    m_logDir->getMonitor()->start();
    log(INFO, "main: I not looking!!");
    return true;
}

