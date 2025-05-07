from pygments.lexer import RegexLexer, bygroups
from pygments.token import *

class JoedbiLexer(RegexLexer):
    name = 'joedbi'
    aliases = ['joedbi']
    filenames = ['*.joedbi']

    tokens = {
        'root': [
            (r'create_table\s+', Keyword, 'table'),
            (r'add_field\s+', Keyword, 'table_field_type_init'),
            (r'custom\s+', Keyword, 'custom'),
            (r'comment\s+', Keyword, 'literal'),
            (r'insert_into\s+', Keyword, 'table_literal'),
            (r'delete_from\s+', Keyword, 'table_integer'),
            (r'insert_vector\s+', Keyword, 'table_integer_integer'),
            (r'delete_vector\s+', Keyword, 'table_integer_integer'),
            (r'update\s+', Keyword, 'table_integer_field_literal'),
            (r'timestamp\s+', Keyword, 'timestamp'),
            (r'valid_data\s+', Keyword),
            (r'soft_checkpoint\s+', Keyword),
            (r'hard_checkpoint\s+', Keyword),
            (r'.+', Literal)
        ],
        'table': [
            (r'[a-zA-Z_]\w*', Name.Class)
        ],
        'table_field_type_init': [
            (r'[a-zA-Z_]\w*\s+', Name.Class, 'field_type_init')
        ],
        'field_type_init': [
            (r'[a-zA-Z_]\w*\s+', Name.Variable, 'type_init')
        ],
        'type_init': [
            (r'references\s+[a-zA-Z_]\w*', Keyword.Type, 'init'),
            (r'[a-zA-Z_]\w*', Keyword.Type, 'init')
        ],
        'init': [
            (r'\s*=\s*', Operator, 'literal')
        ],
        'custom': [
            (r'[a-zA-Z_]\w*', Name.Function)
        ],
        'literal': [
            (r'\s*\"([^\\\"]|\\.)*\"', Literal.String),
            (r'-?\s*\d+', Number.Integer)
        ],
        'integer': [
            (r"\d+", Number.Integer)
        ],
        'table_literal': [
            (r'[a-zA-Z_]\w*\s+', Name.Class, 'literal')
        ],
        'table_integer_integer': [
            (r'[a-zA-Z_]\w*\s+', Name.Class, 'integer_integer')
        ],
        'table_integer_field_literal': [
            (r'[a-zA-Z_]\w*\s+', Name.Class, 'integer_field_literal')
        ],
        'integer_field_literal': [
            (r"\d+\s+", Number.Integer, 'field_literal')
        ],
        'integer_integer': [
            (r"\d+\s+", Number.Integer, 'integer')
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
    app.add_lexer('joedbi', JoedbiLexer)
