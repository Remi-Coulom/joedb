#ifndef TYPE_MACRO
#define TYPE_MACRO(type, return_type, type_id, read_method, write_method)
#endif

#ifndef TYPE_MACRO_NO_STRING
TYPE_MACRO(std::string, const std::string &, string, read_string, write_string)
#endif
TYPE_MACRO(int32_t, int32_t, int32, read<int32_t>, write<int32_t>)
TYPE_MACRO(int64_t, int64_t, int64, read<int64_t>, write<int64_t>)
TYPE_MACRO(record_id_t, record_id_t, reference, compact_read<record_id_t>, compact_write<record_id_t>)
TYPE_MACRO(char, bool, boolean, read<char>, write<char>)
TYPE_MACRO(float, float, float32, read<float>, write<float>)
TYPE_MACRO(double, double, float64, read<double>, write<double>)
TYPE_MACRO(int8_t, int8_t, int8, read<int8_t>, write<int8_t>)

//
// Adding a type:
//  - Add one line above (very important: at the end, to keep compatibility!)
//  - Add a constructor in Type.h
//  - Add a case in dump.cpp (joedb::write_type)
//  - Add a case in joedbc.cpp (write_type)
//  - Add I/O in type_io.h (and maybe cpp)
//  - Add in Interpreter.cpp (Interpreter::parse_type)
//
