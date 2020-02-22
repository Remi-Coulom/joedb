#ifndef joedb_dump_declared
#define joedb_dump_declared

namespace joedb
{
 class Readable;
 class Writable;
 class Readonly_Journal;

 void dump(const Readable &db, Writable &writable, bool schema_only = false);
 void dump_data(const Readable &db, Writable &writable);

 void pack(Readonly_Journal &input_journal, Writable &writable);
}

#endif
