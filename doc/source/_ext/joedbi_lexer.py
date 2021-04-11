from pygments.lexer import RegexLexer, bygroups
from pygments.token import *

class JoedbiLexer(RegexLexer):
    name = 'joedbi'
    aliases = ['joedbi']
    filenames = ['*.joedbi']

    tokens = {
        'root': [
            (r'create_table\s+', Keyword, 'table'),
            (r'add_field\s+', Keyword, 'table_field_type'),
            (r'custom\s+', Keyword, 'custom'),
            (r'.+', Literal)
        ],
        'table': [
            (r'[a-zA-Z_]\w*', Name.Class)
        ],
        'table_field_type': [
            (r'[a-zA-Z_]\w*\s+', Name.Class, 'field_type')
        ],
        'field_type': [
            (r'[a-zA-Z_]\w*\s+', Name.Variable, 'type')
        ],
        'type': [
            (r'references\s+[a-zA-Z_]\w*', Keyword.Type),
            (r'[a-zA-Z_]\w*', Keyword.Type)
        ],
        'custom': [
            (r'[a-zA-Z_]\w*', Name.Function)
        ]

    }

def setup(app):
    app.add_lexer('joedbi', JoedbiLexer())
