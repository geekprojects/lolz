
#include <geek/core-database.h>

#include <yaml-cpp/yaml.h>

#include <stdio.h>

using namespace std;
using namespace Geek;
using namespace Geek::Core;

int main(int argc, char** argv)
{
    string configPath;
    configPath = string(getenv("HOME")) + "/.lolz.yml";
    YAML::Node m_config = YAML::LoadFile(configPath);

    string dbpath = m_config["database"].as<std::string>();

    Database* db = new Database(dbpath);

    bool res;
    res = db->open();

    string keyword = argv[1];

    string query = "SELECT rowid, highlight(event_fts, 0, '<b>', '</b>') FROM event_fts WHERE event_fts MATCH (?)";

    PreparedStatement* ps = db->prepareStatement(query);
    ps->bindString(1, keyword);
    ps->executeQuery();
    while (ps->step())
    {
        uint64_t id = ps->getInt64(0);
        string line = ps->getString(1);
        printf("%lld: %s\n", id, line.c_str());
    }
    delete ps;

    delete db;
}

