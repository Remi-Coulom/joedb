from pygments.lexer import RegexLexer, bygroups
from pygments.token import *

class JoedbcLexer(RegexLexer):
    name = 'joedbc'
    aliases = ['joedbc']
    filenames = ['*.joedbc']

    tokens = {
        'root': [
            (r'namespace\s+', Keyword, 'namespace'),
            (r'create_unique_index\s+', Keyword, 'index_table_fields'),
            (r'create_index\s+', Keyword, 'index_table_fields'),
            (r'set_table_null_initialization\s+', Keyword, 'table_constant'),
            (r'generate_c_wrapper', Keyword)
        ],
        'namespace': [
            (r'[a-zA-Z_]\w*', Name.Namespace, 'namespace_continuation')
        ],
        'namespace_continuation': [
            (r'::', Operator, 'namespace')
        ],
        'index_table_fields': [
            (r'[a-zA-Z_]\w*\s+', Name.Variable, 'table_fields')
        ],
        'table_fields': [
            (r'[a-zA-Z_]\w*\s+', Name.Class, 'fields')
        ],
        'fields': [
            (r'[a-zA-Z_]\w*', Name.Variable, 'fields_continuation')
        ],
        'fields_continuation': [
            (r',', Operator, 'fields')
        ],
        'table_constant' : [
        ]
    }

def setup(app):
    app.add_lexer('joedbc', JoedbcLexer())
