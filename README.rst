Welcom to joedb!
================

Joedb stands for the Journal-Only Embedded Database. Its purpose is to allow
crash-safe manipulation of data stored in permanent storage in a way that is
convenient, efficient, and reliable. Data is manipulated directly in the target
programming language, without using SQL: a compiler produces native type-safe
data-manipulation code from the database schema. The data is stored in
permanent storage as a journal of all modifications. This way, the whole data
history is remembered, and it is possible to re-create any past state of the
database. It is also a way to make the system extremely simple, and fast.

For more information, please take at a look at the _`documentation https://www.remi-coulom.fr/joedb/intro.html`.
