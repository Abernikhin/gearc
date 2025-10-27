import sys
import lexer
import parser

def compile_module(module, to): pass

def main(argv):
    compile_module(argv[1], argv[2])

def compile_module(module, to):
    with open(module, 'r') as f:
        text = f.read()
    _lexer = lexer.lexer(text)
    _tokens = _lexer()
    _parser = parser.parser(_tokens)
    _ast = _parser()
    for i in _ast:
        i.info()

main(sys.argv)
