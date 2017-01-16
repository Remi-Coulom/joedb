Schema Upgrade
==============

An important practical problem when storing structured data to files is schema upgrades. A new version of an application may change the data structure, typically by adding tables or fields. In order to allow the new version to use data produced by previous versions, it is necessary to upgrade the old data files. Joedb has convenient features that lets database be easily upgraded in a way that is automatic, transparent, and safe.

Code compiled by joedbc will usually refuse to open joedb databases when the schema of the file does not match the schema used for compiling. But if the schema of the opened file matches the beginning of the current schema, then it will open the file successfully, and upgrade it to the new schema.

In addition to modifying tables and fields, it is also possible to write custom code to adjust table content during an upgrade.

Here is an example that shows how it works. Let's suppose we start from a simple schema with persons and their names:

``schema_v1.joedbi``:

.. code-block:: none

  create_table person
  add_field person name string

Then, in the next version, we wish to add a language table, and indicate a preferred language for each person. When upgrading an old database, we wish to set the default language of existing persons to English.

``schema_v2.joedbi``:

.. code-block:: none

  create_table person
  add_field person name string
  create_table language
  add_field language name string
  add_field language id string
  add_field person preferred_language references language
  custom set_default_preferred_language_to_english

Note that the first two lines are identical to ``schema_v1.joedbi``. For the automatic upgrade process to work, it is necessary to keep the first lines of the new schema identical to the old schema. For instance, if you wish to rename a field, you should not edit the field name directly, but append a ``rename_field`` operation instead.

The ``custom`` command defines the name of a custom function that the programmer has to implement.

.. code-block:: c++

  void schema_v2::File_Database::set_default_preferred_language_to_english
  ( 
   Database &db
  )
  {
   auto english = db.new_language("English", "en");
   for (auto person: db.get_person_table())
    db.set_preferred_language(person, english);
  } 

This way, when a joedb file with the old schema is opened, the new tables and fields will be created, and data will be initialized correctly thanks to the custom function.

Creating a new file works like upgrading from an empty schema, and will also invoke the custom functions.
