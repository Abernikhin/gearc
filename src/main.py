import sys
import lexer

def compile_module(module, to): pass

def main(argv):
    compile_module(argv[1], argv[2])

def compile_module(module, to):
    with open(module, 'r') as f:
        text = f.read()
    _lexer = lexer.lexer(text)
    _tokens = _lexer()

main(sys.argv)
