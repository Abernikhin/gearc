from parser.node import*

class vector:
    def __init__(self, sig):
        self.sig = sig
        self.e = []
    
    def append(self, obj):
        self.e.append(obj)
    
    def pop(self, index):
        self.e.pop(index)
    
    def __getitem__(self, index):
        return self.e[index]
    
class parser:
    def __init__(self, tokens) -> None:
        self.tokens = tokens
        self.result = []
        self.free = 0

    def __call__(self):
        while len(self.tokens) > 0:
            self.result.append(self.rout_m(self.tokens))
            for i in range(self.free):
                self.tokens.pop(0)
        return self.result

    def rout_m(self, tokens):
        if tokens[0] != "private" and tokens[0] != "public":
            return node(token("name", "private"), self.rout(tokens))
        mod = tokens[0]
        tokens.pop(0)
        root = node(mod, self.rout(tokens))
        return root

    def rout(self, tokens):
        tokens = self.parent(tokens)
        if tokens[0] == "struct":
            self.free += 4
            root = node(tokens[0])
            tokens.pop(0)
            if type(tokens[0]) != token:
                raise SyntaxError("wrong struct construction")
            if tokens[0].type != "name":
                raise SyntaxError("wrong struct construction: struct %s"%tokens[0].value)
            root.append(node(token("name", "name"), node(tokens[0])))
            tokens.pop(0)
            if type(tokens[0]) != vector:
                raise SyntaxError("wrong struct construction: cant find {}")
            if tokens[0].sig != 0:
                raise SyntaxError("wrong struct construction: cant find {}")
            root.append(self.struct(tokens[0].e))
            return root


    def struct(self, tokens):
        root = node(token("name", "attrs"))
        obj = []
        buf = []
        for i in tokens:
            if i == ';':
                obj.append(buf)
                buf = []
                continue
            buf.append(i)
        
        breakpoint()
        for i in obj:
            buffer = []
            self.free += len(i) + 1
            for f in i.copy():
                if f == ':':
                    break
                buffer.append(f)
                i.pop(0)
            i.pop(0)
            root.append(node(token("colon", ':'), self.enum(buffer), self.type(i)))

        return root


    def enum(self, tokens):
        if len(tokens) == 1:
            return node(tokens[0])
        return node(token("name", "enum"))

    def type(self, tokens):
        if len(tokens) == 1:
            return node(tokens[0])
        return node(token("name", "type"))



    def parent(self, tokens):
        result = []
        buf    = vector(0)
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
                    buf = vector(0)
                continue
            if c != 0:
                buf.append(i)
                index += 1
                continue
            result.append(i)
            index += 1
        return result