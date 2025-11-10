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
        return node(token("name", "private"), self.rout(tokens))
    

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

        if tokens[0] == "let":
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
    
        if tokens[0] == "def":
            name = node(token("name", "name"), node(tokens[1]))
            _args = node(token("name", "args"))
            type_buffer = []
            impl = None
            for i in range(4, len(tokens)):
                if type(tokens[i]) == vector:
                    impl = self.function(tokens[i].e, tokens[5])
                    break
                type_buffer.append(tokens[i])
            _type = node(token("name", "type"), self.type(type_buffer))
            self.free += 5 + len(type_buffer)
            return node(tokens[0], name, _args, _type, impl)
        
        if tokens[0] == "using":
            pass

        else:
            raise SyntaxError(f"wrong global expression: {tokens[0].value}")
        
    
    def function(self, tokens, t):
        root = node(token("name", "impl"))
        obj = []
        buf = []
        tokens = self.parent(tokens)
        for i in tokens:
            if i == ';':
                obj.append(buf)
                buf = []
                continue
            buf.append(i)
        if len(buf) != 0:
            obj.append(buf)
        
        index = 0
        for i in obj:
            if i[0] == "return":
                i.pop(0)
                root.append(node(token("name", "return"), self.expr(i)))
                break
            elif len(obj)-1 == index and t != 'void':
                root.append(node(token("name", "return"), self.expr(i)))
                break
            else:
                root.append(self.factor(i))
            index += 1
        
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
        for i in tokens:
            if type(i) != token:
                raise SyntaxError("wrong enum")
            if i.type != 'comma' and i.type != "name":
                raise SyntaxError("wrong enum 2")
        if len(tokens) == 1:
            return node(tokens[0])
        if len(tokens) %2 == 0 and tokens[len(tokens)-1] == ',':
            tokens.pop()
        root = node(token("name", "enum"))
        for i in tokens:
            if i.type == "name":
                root.append(node(i))

        return root


    def type(self, tokens):
        if len(tokens) == 1 and tokens[0].type == "name":
            return node(tokens[0])
        if len(tokens) == 2 and tokens[1].type == "name":
            if tokens[0] == '&':
                return node(tokens[0], node(tokens[1]))
        return node(token("name", "type"))


    def parent(self, tokens):
        result = []

        index = 0
        while index < len(tokens):
            i = tokens[index]
            if i == '{':
                buffer = vector(0)
                index += 1
                c = 0
                while True:
                    i = tokens[index]
                    if i == '{':
                        buffer.append(i)
                        c += 1
                        continue
                    if i == '}':
                        if c != 0:
                            buffer.append(i)
                            c -= 1
                            index += 1
                            continue
                        break
                    
                    buffer.append(i)
                    index += 1
                index += 1
                buffer.e = self.parent(buffer.e)
                result.append(buffer)
                continue


            if i == '(':
                buffer = vector(1)
                index += 1
                c = 0
                while True:
                    i = tokens[index]
                    if i == '(':
                        buffer.append(i)
                        c += 1
                        continue
                    if i == ')':
                        if c != 0:
                            buffer.append(i)
                            c -= 1
                            index += 1
                            continue
                        break
                    
                    buffer.append(i)
                    index += 1
                index += 1
                buffer.e = self.parent(buffer.e)
                result.append(buffer)
                continue
            index += 1
            result.append(i)

        return result


    def expr(self, tokens):
        index = 0
        factor_t = tokens
        tokens.reverse()
        for i in tokens:
            if type(i) == vector:
                index += 1
                continue
            if (i == '+' or i == '-') and index == 0:
                index += 1
                continue
            if i == '+' or i == '-':
                return node(i, self.expr(tokens[0:index]), self.expr(tokens[index+1:]))
            index += 1
        
        index = 0
        for i in factor_t:
            if type(i) == vector:
                index += 1
                continue
            if i == '*' and index == 0:
                index += 1
                continue
            if i == '*' or i == '/':
                return node(i, self.expr(factor_t[index+1:]), self.expr(factor_t[0:index]))
            index += 1
        return self.factor(factor_t)

    def factor(self, tokens):
        if len(tokens) == 1:
            if type(tokens[0]) == vector:
                return self.expr(tokens[0].e)
            if tokens[0].type == "number":
                return node(tokens[0])
            if tokens[0].type == "name":
                return node(tokens[0])