#ifndef TYPE_MACRO
#define TYPE_MACRO(type, return_type, type_id, read_method, write_method)
#endif

#ifndef TYPE_MACRO_NO_STRING
TYPE_MACRO(std::string, const std::string &, string, safe_read_string, write_string)
#endif
#ifndef TYPE_MACRO_NO_INT
TYPE_MACRO(int32_t, int32_t, int32, file.read<int32_t>, write<int32_t>)
TYPE_MACRO(int64_t, int64_t, int64, file.read<int64_t>, write<int64_t>)
#endif
#ifndef TYPE_MACRO_NO_REFERENCE
TYPE_MACRO(Record_Id, Record_Id, reference, file.compact_read<Record_Id>, compact_write<Record_Id>)
#endif
#ifndef TYPE_MACRO_NO_INT
TYPE_MACRO(char, bool, boolean, file.read<char>, write<char>)
#endif
#ifndef TYPE_MACRO_NO_FLOAT
TYPE_MACRO(float, float, float32, file.read<float>, write<float>)
TYPE_MACRO(double, double, float64, file.read<double>, write<double>)
#endif
#ifndef TYPE_MACRO_NO_INT
TYPE_MACRO(int8_t, int8_t, int8, file.read<int8_t>, write<int8_t>)
TYPE_MACRO(int16_t, int16_t, int16, file.read<int16_t>, write<int16_t>)
#endif

// Warning: only add at the bottom to keep compatibility with previous versions
// Don't forget to add case in SQL_Dump_Writable
