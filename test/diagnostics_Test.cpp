#include "joedb/journal/diagnostics.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"

#include "gtest/gtest.h"

#include <sstream>

/////////////////////////////////////////////////////////////////////////////
TEST(diagnostics, about)
/////////////////////////////////////////////////////////////////////////////
{
 std::stringstream out;
 joedb::about_joedb(out);
}

/////////////////////////////////////////////////////////////////////////////
TEST(diagnostics, dump_header)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 {
  joedb::Writable_Journal journal(file);
 }
 std::stringstream out;
 joedb::dump_header(out, file);
}