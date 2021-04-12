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
syn keyword joedbi_keyword rename_table rename_field timestamp valid_data insert_into insert_vector update update_vector delete_from

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

syn keyword joedbi_type string int8 int16 int32 int64 float32 float64 bool contained
syn match joedbi_type 'references\s\+[a-zA-Z_]\w*' contained

syn match joedbi_comment "#.*$"

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
hi def link joedbi_keyword              Statement
hi def link joedbi_comment              Comment
hi def link joedbi_table                Identifier
hi def link joedbi_table_field          Identifier
hi def link joedbi_table_field_type     Identifier
hi def link joedbi_field                Special
hi def link joedbi_field_type           Special
hi def link joedbi_type                 Type
hi def link joedbi_function             Function

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
let b:current_syntax = "joedbi"
