#include "joedb/asio/io_context.h"

namespace joedb::asio
{
 void io_context::run() {context.run();}
 io_context::io_context() = default;
 io_context::io_context(int concurrency_hint): context(concurrency_hint) {}
 io_context::~io_context() = default;
}
