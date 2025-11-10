import sys
import lexer
import parser
import compiler
import os

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
    _compiler = compiler.compiler(_ast)
    _result = _compiler()
    _module = module.split('/')[-1]
    _module = _module.split('.')[0]
    _module = _module+".s"
    if not os.path.isdir(to):
        os.mkdir(to)
    with open(to+'/'+_module, 'w')as f:
        f.write(_result.data)
    

main(sys.argv)
