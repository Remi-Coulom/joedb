#ifndef joedb_dump_declared
#define joedb_dump_declared

namespace joedb
{
 class Readable;
 class Writeable;
 class Readonly_Journal;

 void dump(const Readable &db, Writeable &writeable);
 void dump_data(const Readable &db, Writeable &writeable);
 void pack(Readonly_Journal &input_journal, Writeable &writeable);
}

#endif
