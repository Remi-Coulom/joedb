#include "Multiplexer.h"

/////////////////////////////////////////////////////////////////////////////
joedb::Internal_Listener::Internal_Listener
/////////////////////////////////////////////////////////////////////////////
(
 Multiplexer *multiplexer,
 size_t id
):
 multiplexer(multiplexer),
 id(id)
{
}

#define MULTIPLEX(x) do {\
 if (!multiplexer->multiplexing) {\
  multiplexer->multiplexing = true;\
  for (size_t i = 0; i < multiplexer->external_listeners.size(); i++) \
   if (i != id) \
    multiplexer->external_listeners[i]->x;\
  multiplexer->multiplexing = false;\
 }\
} while(0)

/////////////////////////////////////////////////////////////////////////////
void joedb::Internal_Listener::after_create_table(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 MULTIPLEX(after_create_table(name));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Internal_Listener::after_drop_table(table_id_t table_id)
/////////////////////////////////////////////////////////////////////////////
{
 MULTIPLEX(after_drop_table(table_id));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Internal_Listener::after_rename_table
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name
)
{
 MULTIPLEX(after_rename_table(table_id, name));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Internal_Listener::after_add_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name,
 Type type
)
{
 MULTIPLEX(after_add_field(table_id, name, type));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Internal_Listener::after_drop_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id
)
{
 MULTIPLEX(after_drop_field(table_id, field_id));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Internal_Listener::after_rename_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id,
 const std::string &name
)
{
 MULTIPLEX(after_rename_field(table_id, field_id, name));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Internal_Listener::after_custom(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 MULTIPLEX(after_custom(name));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Internal_Listener::after_comment(const std::string &comment)
/////////////////////////////////////////////////////////////////////////////
{
 MULTIPLEX(after_comment(comment));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Internal_Listener::after_time_stamp(int64_t time_stamp)
/////////////////////////////////////////////////////////////////////////////
{
 MULTIPLEX(after_time_stamp(time_stamp));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Internal_Listener::after_insert
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
)
{
 MULTIPLEX(after_insert(table_id, record_id));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Internal_Listener::after_insert_vector
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id,
 record_id_t size
)
{
 MULTIPLEX(after_insert_vector(table_id, record_id, size));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Internal_Listener::after_delete
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
)
{
 MULTIPLEX(after_delete(table_id, record_id));
}

#define TYPE_MACRO(type, return_type, type_id, R, W)\
void joedb::Internal_Listener::after_update_##type_id\
(\
 table_id_t table_id,\
 record_id_t record_id,\
 field_id_t field_id,\
 return_type value\
)\
{\
 MULTIPLEX(after_update_##type_id(table_id, record_id, field_id, value));\
}\
\
void joedb::Internal_Listener::after_update_vector_##type_id\
(\
 table_id_t table_id,\
 record_id_t record_id,\
 field_id_t field_id,\
 record_id_t size,\
 const type *value\
)\
{\
 MULTIPLEX\
 (\
  after_update_vector_##type_id(table_id, record_id, field_id, size, value)\
 );\
}
#include "TYPE_MACRO.h"
#undef TYPE_MACRO
#undef MULTIPLEX

/////////////////////////////////////////////////////////////////////////////
joedb::Listener &joedb::Multiplexer::add_listener(Listener &external_listener)
/////////////////////////////////////////////////////////////////////////////
{
 const size_t id = external_listeners.size();

 external_listeners.push_back(&external_listener);
 internal_listeners.push_back(
  std::unique_ptr<Internal_Listener>(new Internal_Listener(this, id)));
 // TODO later: C++14 has std::make_unique. Should be used here in the future.

 return *internal_listeners.back();
}
