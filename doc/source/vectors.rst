Vectors
=======

Joedb stores each column of a table in a ``std::vector`` container. This allows
very efficient data manipulation: a large column of a primitive type (float or
int) can be read from or written to a joedb file with a single system call.

Joedb offers functions to allocate a set of consecutive rows in a table, and
manipulate them like a vector. For instance, when using a database defined by
this schema:

.. code-block:: joedbi

    create_table float
    add_field float value float32

It is possible to allocate and manipulate a vector of floats like this:

.. code-block:: c++

    const size_t vector_size = 5;
    auto vector = db.new_vector_of_float(vector_size);
    for (size_t i = 0; i < vector_size; i++)
     db.set_value(vector[i], 0.1f * float(i));

``vector`` is a usual reference, and can be stored in a field of type
``references float``.

The loop above will write vector values one by one. In order to efficiently
write the whole vector in a single operation, it should be written this way
instead:

.. code-block:: c++

    const size_t vector_size = 5;
    auto vector = db.new_vector_of_float(vector_size);
    {
     auto value = db.update_vector_of_value(vector, vector_size);
     for (size_t i = 0; i < vector_size; i++)
      value[i] = 0.1f * float(i);
    }

The destructor of the ``value`` object will perform a single large write of the
data to the joedb file, which is more compact and efficient than writing each
record one by one.

If a joedb file was created by a succession of different insertions and
updates, then the storage of a column will not be contiguous in the joedb file,
and loading performance will not be optimal. Optimal loading performance can be
obtained by packing the file with :ref:`joedb_pack`.
