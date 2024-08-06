if exists("b:current_syntax")
 finish
endif

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
syn match joedbc_comment "#.*$"

syn keyword joedbc_keyword namespace nextgroup=joedbc_namespace skipwhite

syn match joedbc_namespace '[a-zA-Z_]\w*' contained nextgroup=joedbc_namespace_continuation
syn match joedbc_namespace_continuation '::' contained nextgroup=joedbc_namespace

syn keyword joedbc_keyword create_unique_index nextgroup=joedbc_index_table_fields skipwhite

syn keyword joedbc_keyword create_index nextgroup=joedbc_index_table_fields skipwhite

syn match joedbc_index_table_fields '[a-zA-Z_]\w*' contained nextgroup=joedbc_table_fields skipwhite
syn match joedbc_table_fields '[a-zA-Z_]\w*' contained nextgroup=joedbc_fields skipwhite
syn match joedbc_fields '[a-zA-Z_]\w*' contained nextgroup=joedbc_fields_continuation
syn match joedbc_fields_continuation ',' contained nextgroup=joedbc_fields

syn keyword joedbc_keyword set_table_null_initialization nextgroup=joedbc_table_bool skipwhite
syn keyword joedbc_keyword set_single_row nextgroup=joedbc_table_bool skipwhite
syn match joedbc_table_bool '[a-zA-Z_]\w*' contained nextgroup=joedbc_bool
syn keyword joedbc_bool true false

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
hi def link joedbc_comment            Comment
hi def link joedbc_keyword            Statement
hi def link joedbc_namespace          Type
hi def link joedbc_index_table_fields Type
hi def link joedbc_table_fields       Identifier
hi def link joedbc_table_bool         Identifier
hi def link joedbc_fields             Special
hi def link joedbc_bool               Constant

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
let b:current_syntax = "joedbc"
