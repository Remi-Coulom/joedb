#include "joedb/journal/SHA_256.h"

namespace joedb
{
 constexpr std::array<uint32_t, 8> SHA_256::hash_init;
 constexpr std::array<uint32_t, 64> SHA_256::k;
}
