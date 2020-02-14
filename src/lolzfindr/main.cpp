
#include <geek/core-database.h>

#include <yaml-cpp/yaml.h>

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>
#include <time.h>

using namespace std;
using namespace Geek;
using namespace Geek::Core;

static const struct option g_options[] =
{
    { "config",  required_argument, NULL, 'c' },
    { "exec",    required_argument, NULL, 'e' },
    { "tail",    no_argument,       NULL, 't' },
    { NULL,      0,                 NULL, 0 }
};

volatile bool g_running = false;

void signalHandler(int sig)
{
    g_running = false;
}

string substitute(string str, const string find, const string replace)
{
    int findLen = find.length();
    while (true)
    {
        string::size_type pos = str.find(find);
        if (pos == string::npos)
        {
            break;
        }
        str.replace(pos, findLen, replace);
    }
    return str;
}

void execute(string execStr, string line, string file)
{
    execStr = substitute(execStr, "{line}", line);
    execStr = substitute(execStr, "{file}", file);
    printf("execute: %s\n", execStr.c_str());

    int res = fork();
    if (res == 0)
    {
        system(execStr.c_str());
        exit(0);
    }
}

int main(int argc, char** argv)
{
    string configPath;
    configPath = string(getenv("HOME")) + "/.lolz.yml";

    string execStr = "";
    bool tail = false;

    while (true)
    {
        int c = getopt_long(
            argc,
            argv,
            "c:e:t",
            g_options,
            NULL);
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
            case 'c':
                configPath = string(optarg);
                break;
            case 'e':
                execStr = string(optarg);
                break;
            case 't':
                tail = true;
                break;
        }
    }

    int remaining_argc = argc - optind;
    if (remaining_argc <= 0)
    {
        printf("%s <search terms>\n", argv[0]);
        exit(1);
    }

    YAML::Node m_config = YAML::LoadFile(configPath);

    string dbpath = m_config["database"].as<std::string>();

    struct sigaction act;
    memset (&act, 0, sizeof(act));
    act.sa_handler = signalHandler;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);

    Database* db = new Database(dbpath, true);

    bool res;
    res = db->open();
    if (!res)
    {
        printf("Oh noes! Where iz the database?\n");
        return 1;
    }

    string keyword = argv[optind];

    uint64_t maxId = 0;

    if (tail)
    {
        string maxIdQuery = "SELECT MAX(id) FROM event";
        PreparedStatement* maxIdPs = db->prepareStatement(maxIdQuery);
        maxIdPs->executeQuery();
        if (maxIdPs->step())
        {
            maxId = maxIdPs->getInt64(0);
        }
        delete maxIdPs;
    }

    string query =
        "SELECT"
        "    e.id, e.line, f.path"
        "  FROM event_fts"
        "  JOIN event e ON e.id = event_fts.rowid"
        "  JOIN logfile f ON f.id = e.logfile_id"
        "  WHERE"
        "    event_fts MATCH (?) AND"
        "    e.id > ?"
        "  ORDER BY e.id";

    g_running = true;

    uint64_t lastId = maxId;
    while (g_running)
    {
        PreparedStatement* ps = db->prepareStatement(query);
        ps->bindString(1, keyword);
        ps->bindInt64(2, lastId);
        res = ps->executeQuery();
        if (res)
        {
            while (ps->step())
            {
                lastId = ps->getInt64(0);
                string line = ps->getString(1);
                string file = ps->getString(2);
                printf("%" PRIu64 ": %s: %s\n", lastId, file.c_str(), line.c_str());
                if (execStr.length() > 0)
                {
                    execute(execStr, line, file);
                }
            }
        }
        delete ps;

        if (!tail)
        {
            break;
        }

        struct timespec tv;
        tv.tv_sec = 0;
        tv.tv_nsec = 100000000;
        nanosleep(&tv, NULL);
    }

    if (tail)
    {
        printf("kthxbye!\n");
    }

    delete db;

    return 0;
}

