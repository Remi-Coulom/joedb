#ifndef joedb_dump_declared
#define joedb_dump_declared

namespace joedb
{
 class Readable;
 class Writable;
 class Readonly_Journal;

 /// \ingroup ui
 void dump(const Readable &db, Writable &writable, bool schema_only = false);
 /// \ingroup ui
 void dump_data(const Readable &db, Writable &writable);
 /// \ingroup ui
 void pack(Readonly_Journal &input_journal, Writable &writable);
}

#endif
