#include "Sample.h"

Sample::Sample(const std::string &file_prefix):
 file_prefix(file_prefix),
 ofsLog((file_prefix + "Sample.log").c_str())
{
}
