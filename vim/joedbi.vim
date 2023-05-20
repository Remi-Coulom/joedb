"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" vim syntax file for the joedb interpreter
"
" copy or link into ~/.vim/syntax, and add the following line to .vimrc:
" au BufNewFile,BufRead *.joedbi set filetype=joedbi
"
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
if exists("b:current_syntax")
 finish
endif

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
syn keyword joedbi_keyword valid_data schema dump sql help about quit

syn keyword joedbi_keyword json echo blob nextgroup=joedbi_constant skipwhite

syn keyword joedbi_keyword update_vector nextgroup=joedbi_table_integer_field_constant skipwhite

syn keyword joedbi_keyword insert_vector nextgroup=joedbi_table_integer_integer skipwhite
syn match joedbi_table_integer_integer '[a-zA-Z_]\w*' contained nextgroup=joedbi_integer_integer skipwhite
syn match joedbi_integer_integer '\d\+' contained nextgroup=joedbi_integer skipwhite

syn keyword joedbi_keyword rename_field nextgroup=joedbi_table_field_field skipwhite
syn match joedbi_table_field_field '[a-zA-Z_]\w*' contained nextgroup=joedbi_field_field skipwhite
syn match joedbi_field_field '[a-zA-Z_]\w*' contained nextgroup=joedbi_field skipwhite

syn keyword joedbi_keyword rename_table nextgroup=joedbi_table_table skipwhite
syn match joedbi_table_table '[a-zA-Z_]\w*' contained nextgroup=joedbi_table skipwhite

syn keyword joedbi_keyword table nextgroup=joedbi_table_constant skipwhite
syn keyword joedbi_keyword create_table nextgroup=joedbi_table skipwhite
syn keyword joedbi_keyword drop_table nextgroup=joedbi_table skipwhite
syn match joedbi_table '[a-zA-Z_]\w*' contained

syn keyword joedbi_keyword add_field nextgroup=joedbi_table_field_type skipwhite
syn match joedbi_table_field_type '[a-zA-Z_]\w*' contained nextgroup=joedbi_field_type skipwhite
syn match joedbi_field_type '[a-zA-Z_]\w*' contained nextgroup=joedbi_type skipwhite

syn keyword joedbi_keyword drop_field nextgroup=joedbi_table_field skipwhite
syn match joedbi_table_field '[a-zA-Z_]\w*' contained nextgroup=joedbi_field skipwhite
syn match joedbi_field '[a-zA-Z_]\w*' contained

syn keyword joedbi_keyword custom nextgroup=joedbi_function skipwhite
syn match joedbi_function '[a-zA-Z_]\w*' contained

syn keyword joedbi_type string int8 int16 int32 int64 float32 float64 boolean blob contained
syn match joedbi_type 'references\s\+[a-zA-Z_]\w*' contained

syn match joedbi_comment "#.*$"

syn match joedbi_constant '.*' contained

syn keyword joedbi_keyword comment nextgroup=joedbi_constant skipwhite

syn keyword joedbi_keyword timestamp nextgroup=joedbi_constant skipwhite

syn keyword joedbi_keyword insert_into nextgroup=joedbi_table_constant skipwhite
syn match joedbi_table_constant '[a-zA-Z_]\w*' contained nextgroup=joedbi_constant skipwhite

syn keyword joedbi_keyword delete_from nextgroup=joedbi_table_integer skipwhite
syn match joedbi_table_integer '[a-zA-Z_]\w*' contained nextgroup=joedbi_constant skipwhite
syn match joedbi_integer '\d\+' contained

syn keyword joedbi_keyword update nextgroup=joedbi_table_integer_field_constant skipwhite
syn match joedbi_table_integer_field_constant '[a-zA-Z_]\w*' contained nextgroup=joedbi_integer_field_constant skipwhite
syn match joedbi_integer_field_constant '\d\+' contained nextgroup=joedbi_field_constant skipwhite
syn match joedbi_field_constant '[a-zA-Z_]\w*' contained nextgroup=joedbi_constant skipwhite

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
hi def link joedbi_keyword                      Statement
hi def link joedbi_comment                      Comment
hi def link joedbi_table                        Identifier
hi def link joedbi_table_field_field            Identifier
hi def link joedbi_table_table                  Identifier
hi def link joedbi_table_integer                Identifier
hi def link joedbi_table_integer_integer        Identifier
hi def link joedbi_table_constant               Identifier
hi def link joedbi_table_field                  Identifier
hi def link joedbi_table_field_type             Identifier
hi def link joedbi_table_integer_field_constant Identifier
hi def link joedbi_field                        Special
hi def link joedbi_field_field                  Special
hi def link joedbi_field_type                   Special
hi def link joedbi_field_constant               Special
hi def link joedbi_type                         Type
hi def link joedbi_function                     Function
hi def link joedbi_constant                     Constant
hi def link joedbi_integer                      Constant
hi def link joedbi_integer_integer              Constant
hi def link joedbi_integer_field_constant       Constant
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
let b:current_syntax = "joedbi"
