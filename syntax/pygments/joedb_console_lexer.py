from pygments.lexer import RegexLexer, bygroups
from pygments.token import *

class JoedbConsoleLexer(RegexLexer):
    name = 'joedb_console'
    aliases = ['joedb_console']
    filenames = ['*.joedb_console']

    tokens = {
        'root': [
            (r'^.*\$\s+', Keyword.Type, 'command'),
            (r'^.*\>\s+', Name.Class, 'command'),
            (r'.+', Literal)
        ],

        'command': [
            (r'\w.*$', Keyword, 'root'),
        ],
    }

def setup(app):
    app.add_lexer('joedb_console', JoedbConsoleLexer)
