#ifndef joedb_dump_declared
#define joedb_dump_declared

namespace joedb
{
 class Readable;
 class Writeable;
 class Journal_File;

 void dump(const Readable &db, Writeable &writeable);
 void dump_data(const Readable &db, Writeable &writeable);
 void pack(Journal_File &input_journal, Writeable &writeable);
}

#endif
