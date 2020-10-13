#include "Multiplexer.h"

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::add_writable(Writable &writable)
/////////////////////////////////////////////////////////////////////////////
{
 writables.push_back(&writable);
}

#define MULTIPLEX(x) do {for (auto w: writables) w->x;} while(0)

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::create_table(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 MULTIPLEX(create_table(name));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::drop_table(Table_Id table_id)
/////////////////////////////////////////////////////////////////////////////
{
 MULTIPLEX(drop_table(table_id));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::rename_table
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 const std::string &name
)
{
 MULTIPLEX(rename_table(table_id, name));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::add_field
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 const std::string &name,
 Type type
)
{
 MULTIPLEX(add_field(table_id, name, type));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::drop_field
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Field_Id field_id
)
{
 MULTIPLEX(drop_field(table_id, field_id));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::rename_field
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Field_Id field_id,
 const std::string &name
)
{
 MULTIPLEX(rename_field(table_id, field_id, name));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::custom(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 MULTIPLEX(custom(name));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::comment(const std::string &comment)
/////////////////////////////////////////////////////////////////////////////
{
 MULTIPLEX(comment(comment));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::timestamp(int64_t timestamp)
/////////////////////////////////////////////////////////////////////////////
{
 MULTIPLEX(timestamp(timestamp));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::valid_data()
/////////////////////////////////////////////////////////////////////////////
{
 MULTIPLEX(valid_data());
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::insert_into
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Record_Id record_id
)
{
 MULTIPLEX(insert_into(table_id, record_id));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::insert_vector
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Record_Id record_id,
 Record_Id size
)
{
 MULTIPLEX(insert_vector(table_id, record_id, size));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Multiplexer::delete_from
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Record_Id record_id
)
{
 MULTIPLEX(delete_from(table_id, record_id));
}

#define TYPE_MACRO(type, return_type, type_id, R, W)\
void joedb::Multiplexer::update_##type_id\
(\
 Table_Id table_id,\
 Record_Id record_id,\
 Field_Id field_id,\
 return_type value\
)\
{\
 MULTIPLEX(update_##type_id(table_id, record_id, field_id, value));\
}\
\
void joedb::Multiplexer::update_vector_##type_id\
(\
 Table_Id table_id,\
 Record_Id record_id,\
 Field_Id field_id,\
 Record_Id size,\
 const type *value\
)\
{\
 MULTIPLEX\
 (\
  update_vector_##type_id(table_id, record_id, field_id, size, value)\
 );\
}
#include "TYPE_MACRO.h"
#undef TYPE_MACRO
#undef MULTIPLEX