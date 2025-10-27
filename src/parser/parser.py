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
        self.tokens = self.parent(self.tokens)
        while len(self.tokens) > 0:
            self.result.append(self.rout_m(self.tokens))
            for i in range(self.free):
                self.tokens.pop(0)

            self.free = 0
        return self.result


    def enum(self, tokens):
        if len(tokens) == 1:
            return node(tokens[0])
        return node(token("name", "enum"))

    def type(self, tokens):
        if len(tokens) == 1:
            return node(tokens[0])
        return node(token("name", "type"))




    def rout_m(self, tokens):
        if tokens[0] != "private" and tokens[0] != "public":
            return node(token("name", "private"), self.rout(tokens))
        mod = tokens[0]
        tokens.pop(0)
        root = node(mod, self.rout(tokens))
        self.free += 1
        return root
    
    def rout(self, tokens):
        if tokens[0] == "struct":
            if type(tokens[1]) == vector:
                raise SyntaxError("cant find name in struct")
            if tokens[1].type != "name":
                raise SyntaxError("wrong expresion: struct %s"%tokens[1].value)
            if type(tokens[2]) != vector:
                raise SyntaxError("wrong struct construction")
            if tokens[2].sig != 0:
                raise SyntaxError("cant find {}")
            root = node(
                tokens[0],
                node(
                    token("name", "name"),
                    node(tokens[1])
                    ),
                self.struct(tokens[2].e)
                )
            self.free += 3
            return root

        if tokens[0] == "var":
            name = node(tokens[1])
            if tokens[2] != ':':
                raise SyntaxError("wrong var expresion: cant find :")
            buf = []
            for i in range(3, len(tokens)):
                if tokens[i] == ';':
                    break
                buf.append(tokens[i])
            self.free += len(buf) + 4
            root = node(tokens[0])
            root.append(node(token("name", "name"), name))
            root.append(node(token("name", "type"), self.type(buf)))
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
        
        for i in obj:
            buffer = []
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
        if len(tokens) == 1 and tokens[0].type == "name":
            return node(tokens[0])
        if len(tokens) == 2 and tokens[1].type == "name":
            if tokens[0] == '&':
                return node(tokens[0], node(tokens[1]))
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
    