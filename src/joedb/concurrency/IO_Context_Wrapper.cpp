#include "joedb/concurrency/IO_Context_Wrapper.h"

namespace joedb::concurrency
{
 void IO_Context_Wrapper::run() {io_context.run();}
 IO_Context_Wrapper::IO_Context_Wrapper() = default;
 IO_Context_Wrapper::~IO_Context_Wrapper() = default;
}
