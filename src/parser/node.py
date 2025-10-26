import lexer
from parser.vector import vector

class node:
    def init(self, _value, *child):
        if type(_value) != lexer.token:
            raise TypeError("in node init")
        self.type = _value.type
        self.value = _value.value
        self.vector = vector(list(child))

    
    def __eq__(self, value):
        if type(value) != str:
            raise TypeError(f"wrong value type in node.__eq__ - {type(value)}")
        return True if self.value == value else False

    def __ne__(self, value):
        if type(value) != str:
            raise TypeError(f"wrong value type in node.__ne__ - {type(value)}")
        return True if self.value != value else False

    def info():
        pass
