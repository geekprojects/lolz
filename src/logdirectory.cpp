
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#include <string>
#include <vector>

#include <geek/core-logger.h>
#include <geek/core-data.h>

#include <libfswatch/c++/event.hpp>
#include <libfswatch/c++/monitor.hpp>
#include <libfswatch/c++/monitor_factory.hpp>

#include "logfile.h"
#include "logdirectory.h"

using namespace std;
using namespace Geek;

LogDirectory::LogDirectory(std::string path) : Logger("LogDirectory[" + path + "]")
{
    m_path = path;
}

LogDirectory::~LogDirectory()
{
}

void process_events(const vector<fsw::event>& events, void* context)
{
    LogDirectory* dir = (LogDirectory*)context;
    for (const fsw::event& evt : events)
    {
        string eventnames = "";
        bool isFile = false;
        bool isUpdated = false;
        for (fsw_event_flag flag : evt.get_flags())
        {
            string eventname = fsw::event::get_event_flag_name(flag);
            eventnames += eventname + ", ";
            if (flag == IsFile)
            {
                isFile = true;
            }
            else if (flag == Created || flag == Updated)
            {
                isUpdated = true;
            }
        }

        printf("process_events: %s: %s\n", evt.get_path().c_str(), eventnames.c_str());
        if (isFile)
        {
            if (isUpdated)
            {
                dir->fileUpdated(evt.get_path());
            }
        }
    }
}

void LogDirectory::watch()
{
    scan(m_path);

    vector<string> paths;
    paths.push_back(m_path);
    fsw::monitor* monitor = fsw::monitor_factory::create_monitor(
        fsw_monitor_type::system_default_monitor_type,
        paths,
        process_events);

    monitor->set_context(this);
    monitor->set_recursive(true);
    monitor->start();
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
            log(INFO, "scan: I haz log!: %s", d->d_name);
            LogFile* logFile = addFile(entrypath);
            if (logFile != NULL)
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
    LogFile* logFile = findFile(path);
    if (logFile == NULL)
    {
        logFile = addFile(path);
    }
    if (logFile == NULL)
    {
        return;
    }
    logFile->load();
}

LogFile* LogDirectory::addFile(std::string path)
{
    string ext = "";
    int pos = path.rfind('.');
    if (pos != string::npos)
    {
        ext = path.substr(pos + 1);
    }
    log(INFO, "addFile: ext=%s", ext.c_str());

    if (ext == "gz")
    {
        return NULL;
    }

    LogFile* file = new LogFile(this, path);
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

