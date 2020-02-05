
#include "lolz.h"
#include "logfile.h"
#include "logdirectory.h"

#include <unistd.h>

using namespace std;
using namespace Geek;
using namespace Geek::Core;

Lolz::Lolz() : Logger("Lolz")
{
}

Lolz::~Lolz()
{
}

bool Lolz::init()
{
    m_db = new Database("lolz.db");

    bool res;
    res = m_db->open();
    if (!res)
    {
        error("init: Oh noes!");
        return false;
    }

    vector<Table> tables;

    Table logdir;
    logdir.name = "logdirectory";
    logdir.columns.insert(Column("id", true, true));
    logdir.columns.insert(Column("path"));
    tables.push_back(logdir);

    Table logfile;
    logfile.name = "logfile";
    logfile.columns.insert(Column("id", true, true));
    logfile.columns.insert(Column("logdir_id"));
    logfile.columns.insert(Column("path"));
    logfile.columns.insert(Column("position"));
    logfile.columns.insert(Column("timestamp"));
    tables.push_back(logfile);

    Table event;
    event.name = "event";
    event.columns.insert(Column("id", true, true));
    event.columns.insert(Column("logfile_id"));
    event.columns.insert(Column("timestamp"));
    event.columns.insert(Column("line"));
    tables.push_back(event);

    m_db->checkSchema(tables);

    m_db->execute("CREATE VIRTUAL TABLE IF NOT EXISTS event_fts USING fts5(line, id UNINDEXED)");

    addDirectory("/Users/ian/projects/lolz/logs");

    return true;
}

bool Lolz::run()
{
    for (auto it : m_dirs)
    {
        it.second->watch();
        it.second->start();
    }

    log(INFO, "run: Sleeping");
    while (true)
    {
        sleep(1);
    }

    return true;
}

void Lolz::addDirectory(string path)
{
    string querySql = "SELECT id FROM logdirectory WHERE path=?";
    PreparedStatement* ps = m_db->prepareStatement(querySql);
    ps->bindString(1, path);

    uint64_t id = 0;
    ps->executeQuery();
    if (ps->step())
    {
        id = ps->getInt64(0);
        log(INFO, "addDirectory: %s already exists with id: %lld", path.c_str(), id);
    }
    else
    {
        string insertSql = "INSERT INTO logdirectory (id, path) VALUES (null, ?)";
        m_db->startTransaction();
        PreparedStatement* insertPs = m_db->prepareStatement(insertSql);
        insertPs->bindString(1, path);
        insertPs->execute();
        id = m_db->getLastInsertId();
        log(INFO, "addDirectory: %s is new, id=%lld", path.c_str(), id);
        delete insertPs;
        m_db->endTransaction();
    }
    delete ps;

    LogDirectory* dir = new LogDirectory(this, id, path);

    m_dirs.insert(make_pair(path, dir));
}

void Lolz::addLogFile(LogFile* logFile)
{
/*
    logfile.columns.insert(Column("id", true, true));
    logfile.columns.insert(Column("logdir_id"));
    logfile.columns.insert(Column("path"));
    logfile.columns.insert(Column("position"));
    logfile.columns.insert(Column("timestamp"));
*/

    string querySql = "SELECT id, position, timestamp FROM logfile WHERE logdir_id=? AND path=?";
    PreparedStatement* ps = m_db->prepareStatement(querySql);
    ps->bindInt64(1, logFile->getDirectory()->getId());
    ps->bindString(2, logFile->getPath());
    ps->executeQuery();
    if (ps->step())
    {
        // Entry exists. But the file may have changed since we last saw it!

        uint64_t id = ps->getInt64(0);
        uint64_t position = ps->getInt64(1);
        uint64_t timestamp = ps->getInt64(2);

        logFile->setId(id);
        logFile->setPosition(position);

        delete ps;
        return;

    }
    delete ps;

    string insertSql = "INSERT INTO logfile (id, logdir_id, path, position, timestamp) VALUES (null, ?, ?, 0, 0)";
    m_db->startTransaction();
    PreparedStatement* insertPs = m_db->prepareStatement(insertSql);
    insertPs->bindInt64(1, logFile->getDirectory()->getId());
    insertPs->bindString(2, logFile->getPath());

    insertPs->execute();
    uint64_t id = m_db->getLastInsertId();
    logFile->setId(id);
    delete insertPs;
    m_db->endTransaction();
}

void Lolz::updateLogFile(LogFile* logFile, uint64_t position)
{
    string updateSql = "UPDATE logfile SET position=? WHERE id = ?";

    m_db->startTransaction();
    PreparedStatement* updatePs = m_db->prepareStatement(updateSql);
    updatePs->bindInt64(1, position);
    updatePs->bindInt64(2, logFile->getId());
    updatePs->execute();
    delete updatePs;
    m_db->endTransaction();
}

void Lolz::logEvents(LogFile* logFile, Data* data)
{
    string insertSql = "INSERT INTO event (id, logfile_id, timestamp, line) VALUES (null, ?, ?, ?)";
    PreparedStatement* insertPs = m_db->prepareStatement(insertSql);

    string insertFtsSql = "INSERT INTO event_fts (line, id) VALUES (?, ?)";
    PreparedStatement* insertFtsPs = m_db->prepareStatement(insertFtsSql);

    data->reset();

    m_db->startTransaction();
    while (!data->eof())
    {
        string line = data->readLine();
        insertPs->bindInt64(1, logFile->getId());
        insertPs->bindInt64(2, 0);
        insertPs->bindString(3, line);
        insertPs->execute();

        uint64_t id = m_db->getLastInsertId();

        insertFtsPs->bindString(1, line);
        insertFtsPs->bindInt64(2, id);
        insertFtsPs->execute();
    }
    m_db->endTransaction();

    delete insertPs;
    delete insertFtsPs;
}

