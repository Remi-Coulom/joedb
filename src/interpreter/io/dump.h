#ifndef joedb_dump_declared
#define joedb_dump_declared

namespace joedb
{
 class Database;
 class Listener;
 class Journal_File;

 void dump(const Database &db, Listener &listener);
 void dump_data(const Database &db, Listener &listener);
 void pack(Journal_File &input_journal, Listener &listener);
}

#endif
