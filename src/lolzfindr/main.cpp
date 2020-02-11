
#include <geek/core-database.h>

#include <stdio.h>

using namespace std;
using namespace Geek;
using namespace Geek::Core;

int main(int argc, char** argv)
{
    Database* db = new Database("/Users/ian/projects/lolz/lolz.db");

    bool res;
    res = db->open();

string keyword = argv[1];

string query = "SELECT highlight(event_fts, 0, '<b>', '</b>') FROM event_fts WHERE event_fts MATCH (?)";

PreparedStatement* ps = db->prepareStatement(query);
ps->bindString(1, keyword);
ps->executeQuery();
while (ps->step())
{
string result = ps->getString(0);
printf("%s\n", result.c_str());
}
delete ps;

    delete db;
}

