from pygments.lexer import RegexLexer, bygroups
from pygments.token import *

class JoedbConsoleLexer(RegexLexer):
    name = 'joedb_console'
    aliases = ['joedb_console']
    filenames = ['*.joedb_console']

    tokens = {
        'root': [
            (r'^.*\$', Keyword.Type),
            (r'^.*\>', Name.Class),
            (r'.+', Literal)
        ]
    }

def setup(app):
    app.add_lexer('joedb_console', JoedbConsoleLexer)
