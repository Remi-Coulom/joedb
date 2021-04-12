if exists("b:current_syntax")
 finish
endif

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
syn keyword joedbc_keyword namespace nextgroup=joedbc_namespace skipwhite

syn match joedbc_namespace '[a-zA-Z_]\w*' contained nextgroup=joedbc_namespace_continuation
syn match joedbc_namespace_continuation '::' contained nextgroup=joedbc_namespace

syn keyword joedbc_keyword create_unique_index nextgroup=joedbc_index_table_fields skipwhite

syn keyword joedbc_keyword create_index nextgroup=joedbc_index_table_fields skipwhite

syn match joedbc_index_table_fields '[a-zA-Z_]\w*' contained nextgroup=joedbc_table_fields skipwhite
syn match joedbc_table_fields '[a-zA-Z_]\w*' contained nextgroup=joedbc_fields skipwhite
syn match joedbc_fields '[a-zA-Z_]\w*' contained nextgroup=joedbc_fields_continuation
syn match joedbc_fields_continuation ',' contained nextgroup=joedbc_fields

syn keyword joedbc_keyword set_table_null_initialization nextgroup=joedbc_table_constant skipwhite
syn match joedbc_table_constant '[a-zA-Z_]\w*' contained nextgroup=joedbc_constant
syn match joedbc_constant '.*' contained

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
hi def link joedbc_keyword            Statement
hi def link joedbc_namespace          Type
hi def link joedbc_index_table_fields Type
hi def link joedbc_table_fields       Identifier
hi def link joedbc_table_constant     Identifier
hi def link joedbc_fields             Special
hi def link joedbc_constant           Constant

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
let b:current_syntax = "joedbc"
