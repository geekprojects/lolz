
#include <geek/core-database.h>

#include <yaml-cpp/yaml.h>

#include <stdio.h>
#include <getopt.h>
#include <inttypes.h>

using namespace std;
using namespace Geek;
using namespace Geek::Core;

static const struct option g_options[] =
{
    { "config",  required_argument, NULL, 'c' },
    { NULL,      0,                 NULL, 0 }
};

int main(int argc, char** argv)
{
    string configPath;
    configPath = string(getenv("HOME")) + "/.lolz.yml";

    while (true)
    {
        int c = getopt_long(
            argc,
            argv,
            "c:",
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

    Database* db = new Database(dbpath);

    bool res;
    res = db->open();
    if (!res)
    {
        printf("Oh noes! Where iz the database?\n");
        return 1;
    }

    string keyword = argv[1];

    string query = "SELECT rowid, highlight(event_fts, 0, '<b>', '</b>') FROM event_fts WHERE event_fts MATCH (?)";

    PreparedStatement* ps = db->prepareStatement(query);
    ps->bindString(1, keyword);
    ps->executeQuery();
    while (ps->step())
    {
        uint64_t id = ps->getInt64(0);
        string line = ps->getString(1);
        printf("%" PRIu64 ": %s\n", id, line.c_str());
    }
    delete ps;

    delete db;

    return 0;
}

