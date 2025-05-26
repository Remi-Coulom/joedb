# joedbi README

#### [Repository](https://github.com/Remi-Coulom/joedb)&nbsp;&nbsp;|&nbsp;&nbsp;[Documentation](https://www.joedb.org/intro.html)&nbsp;&nbsp;

## Features

This extension provides syntax highlighting for joedb files (.joedbi & .joedbc)

Joedb stands for the “Journal-Only Embedded Database”. It is a light-weight C++ database that keeps tabular data in memory, and writes a journal to a file. The whole data history is stored, so it is possible to re-create any past state of the database. Joedb has a network protocol, and can operate in a distributed fashion, a bit like git for data. It provides ACID transactions for local and remote concurrent access to a file.

Joedb comes with a compiler that takes a database schema as input, and produces C++ code. The generated C++ data-manipulation code is convenient to use, efficient, and type-safe.

## Requirements

You can follow this steps to intall joedb
[Getting Started](https://www.joedb.org/getting_started.html#getting-started)

## Release Notes

First release
