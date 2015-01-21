#include "JournalFile.h"

using namespace joedb;

const uint32_t JournalFile::version_number = 0x00000001;
const int64_t JournalFile::header_size = 41;

/////////////////////////////////////////////////////////////////////////////
JournalFile::JournalFile(File &file):
/////////////////////////////////////////////////////////////////////////////
 file(file),
 checkpoint_index(0),
 state(state_t::no_error)
{
 //
 // Create a new empty joedb file
 //
 if (file.get_mode() == File::mode_t::create_new)
 {
  file.write_uint8('j');
  file.write_uint8('o');
  file.write_uint8('e');
  file.write_uint8('d');
  file.write_uint8('b');
  file.write_uint32(version_number);
  file.write_uint64(0);
  file.write_uint64(0);
  file.write_uint64(0);
  file.write_uint64(0);
  checkpoint();
 }

 //
 // Check the format of an existing joedb file
 //
 else
 {
  //
  // First, check for initial "joedb"
  //
  if (file.read_uint8() != 'j' ||
      file.read_uint8() != 'o' ||
      file.read_uint8() != 'e' ||
      file.read_uint8() != 'd' ||
      file.read_uint8() != 'b')
  {
   state = state_t::bad_format;
  }
  else
  {
   //
   // Check version number
   //
   const uint32_t version = file.read_uint32();
   if (version > version_number)
    state = state_t::unsupported_version;

   //
   // Find the most recent checkpoint
   //
   int64_t pos[4];
   for (int i = 0; i < 4; i++)
    pos[i] = int64_t(file.read_uint64());

   if (pos[0] != pos[1] || pos[2] != pos[3])
    state = state_t::crash_check;

   int64_t position = 0;

   for (int i = 0; i < 2; i++)
    if (pos[2 * i] == pos[2 * i + 1] && int64_t(pos[2 * i]) > position)
    {
     position = int64_t(pos[2 * i]);
     checkpoint_index = i;
    }

   if (position < header_size)
    state = state_t::bad_format;
   else
    file.set_position(position);
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
void JournalFile::checkpoint()
/////////////////////////////////////////////////////////////////////////////
{
 checkpoint_index ^= 1;
 const int64_t position = file.get_position();
 file.set_position(9 + 16 * checkpoint_index);
 file.write_uint64(uint64_t(position));
 file.write_uint64(uint64_t(position));
 file.set_position(position);
 file.flush();
}

/////////////////////////////////////////////////////////////////////////////
JournalFile::~JournalFile()
/////////////////////////////////////////////////////////////////////////////
{
 checkpoint();
}
