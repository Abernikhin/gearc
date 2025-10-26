import lexer

class vector:
    def __init__(self, _values, type):
        if type(_values) != list:
            raise TypeError("wrong type in vector")
        for i in _values:
            if type(i) != lexer.token or type(i) != vector:
                raise TypeError("wrong type elements in list")
        self.type = type
        self.v = self.parent(_values)

    def append(self, obj):
        if type(obj) != lexer.token or type(obj) != vector:
            raise TypeError("wrong type in vector.append")
        self.v.append(obj)
    
    def parent(self, tokens):
        result = []
        buf    = vector([], 0)
        index  = 0
        c      = 0
        while index < len(tokens):
            i = tokens[index]
            if i == '{':
                if c != 0:
                    buf.append(i)
                c += 1
                index += 1
                continue
            if i == '}':
                c -= 1
                index += 1
                if c != 0:
                    buf.append(i)
                if c == 0:
                    result.append(buf)
                    buf = vector([], 0)
                continue
            if c != 0:
                buf.append(i)
                index += 1
                continue
            result.append(i)
            index += 1
        return result