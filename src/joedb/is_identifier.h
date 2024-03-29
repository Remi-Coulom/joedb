/////////////////////////////////////////////////////////////////////////////
//
// is_identifier.h
//
// Rémi Coulom
//
// March, 2015
//
/////////////////////////////////////////////////////////////////////////////
#ifndef is_identifier_declared
#define is_identifier_declared

#include <string>

namespace joedb
{
 constexpr bool is_letter(char c);
 constexpr bool is_number(char c);
 bool is_identifier(const std::string &s);
}

#endif
