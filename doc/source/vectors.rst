Vectors
=======

Joedb stores each column of a table in a ``std::vector`` container. This allows
very efficient data manipulation. In particular, reading or writing a large
column of a primitive type (float or int) can be performed as a single call to
the operating system function that reads a file to a buffer or write a buffer
to a file.

For instance, when using a database defined by this schema:

.. code-block:: none

    create_table float
    add_field float value float32

It is possible to allocate and manipulate a vector of floats like this:

.. code-block:: c++

     const size_t n = 5;
     auto v = db.new_vector_of_float(n);
     for (size_t i = 0; i < n; i++)
      db.set_value(v[i], 0.1f * float(i));

``v`` is a usual reference, and can be stored in a field of type ``references float``.

The loop above will write vector values one by one. In order to efficiently
write the whole vector in a single operation, the for loop above can be
replaced by:

.. code-block:: c++

    {
     auto value = db.update_vector_of_value(v, n);
     for (size_t i = 0; i < n; i++)
      value[i] = 0.1f * float(i);
    }

The destructor of the ``value`` object will perform a single large write of the
data to the joedb file, which is more efficient than writing each record one by
one.

If a joedb file was created by a succession of different insertions and
updates, then its loading performance will not be optimal. Optimal loading performance can be obtained by packing the file with :ref:`joedb_pack`.
