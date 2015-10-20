#ifndef TYPE_MACRO
#define TYPE_MACRO(type, return_type, type_id, read_method, write_method)
#endif

TYPE_MACRO(std::string, const std::string &, string, read_string, write_string)
TYPE_MACRO(int32_t, int32_t, int32, read<int32_t>, write<int32_t>)
TYPE_MACRO(int64_t, int64_t, int64, read<int64_t>, write<int64_t>)
TYPE_MACRO(record_id_t, record_id_t, reference, compact_read<record_id_t>, compact_write<record_id_t>)
TYPE_MACRO(bool, bool, boolean, read<bool>, write<bool>)
