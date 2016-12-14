#ifndef joedb_dump_declared
#define joedb_dump_declared

namespace joedb
{
 class Database;
 class Writeable;
 class Journal_File;

 void dump(const Database &db, Writeable &writeable);
 void dump_data(const Database &db, Writeable &writeable);
 void pack(Journal_File &input_journal, Writeable &writeable);
}

#endif
