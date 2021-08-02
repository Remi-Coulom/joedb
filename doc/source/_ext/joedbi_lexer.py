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
            (r'comment\s+', Keyword, 'literal'),
            (r'insert_into\s+', Keyword, 'table_literal'),
            (r'update\s+', Keyword, 'table_integer_field_literal'),
            (r'delete_from\s+', Keyword, 'table_integer'),
            (r'timestamp\s+', Keyword, 'timestamp'),
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
        ],
        'literal': [
            (r'\s*\"([^\\\"]|\\.)*\"', Literal.String),
            (r'\s*\d+', Number.Integer)
        ],
        'integer': [
            (r"\d+", Number.Integer)
        ],
        'table_literal': [
            (r'[a-zA-Z_]\w*\s+', Name.Class, 'literal')
        ],
        'table_integer_field_literal': [
            (r'[a-zA-Z_]\w*\s+', Name.Class, 'integer_field_literal')
        ],
        'integer_field_literal': [
            (r"\d+\s+", Number.Integer, 'field_literal')
        ],
        'field_literal': [
            (r'[a-zA-Z_]\w*\s+', Name.Variable, 'literal')
        ],
        'table_integer': [
            (r'[a-zA-Z_]\w*\s+', Name.Class, 'integer')
        ],
        'timestamp': [
            (r'[a-zA-Z0-9\ \-\:]+', Literal.String)
        ]
    }

def setup(app):
    app.add_lexer('joedbi', JoedbiLexer())
