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
            impl = node(token("name", "impl"))
            _type = node(token("name", "type"))
            root = node(tokens[0])
            root.append(name)
            if type(tokens[2]) == vector:
                if tokens[2].sig != 1:
                    raise
                _args = self.args(tokens[2].e)
            if type(tokens[3]) == vector:
                if tokens[3].sig != 0:
                    raise
                impl = self.function(tokens[3].e)
                _type.append(node(token("name", "void")))
                self.free += 4
                root.append(_type)
                root.append(impl)
                root.append(_args)
                return root


            return root
            
        
        if tokens[0] == "using":
            pass

        else:
            raise SyntaxError(f"wrong global expression: {tokens[0].value}")
        
    
    def function(self, tokens):
        root = node(token("name", "impl"))
        obj = []
        buffer = []
        for i in tokens:
            if i == ";":
                obj.append(buffer)
                buffer = []
                continue
            buffer.append(i)
        

        for i in obj:
            if i[0] == "var":
                if len(i) < 4:
                    raise
                if i[1].type != "name":
                    raise
                
                name = node(token("name", "name"), node(i[1]))
                type = node(token("type", "type"))
                mode = False
                a_buffer = []
                type_buffer = []
                for j in i[3:]:
                    if j == '=':
                        mode = True
                        continue
                    if mode:
                        a_buffer.append(j)
                        continue
                    type_buffer.append(j)

                type.append(self.type(type_buffer))
                value = node(token("name", "value"))
                value.append(self.expr(a_buffer))
                root.append(node(i[0], name, type, value))

            elif i[0] == "return":
                root.append(node(i[0], self.expr(i[1:])))
            
            else:
                root.append(self.expr(i))
        
        return root


    def args(self, tokens):
        buffer = []
        type_buffer = []
        root = node(token("name", "args"))
        index = 0
        while index < len(tokens):
            i = tokens[index]
            if i == ':':
                index += 1
                
                n = node(token("colon", ":"))

                while i != ',' and index < len(tokens):
                    i = tokens[index]
                    type_buffer.append(i)
                    index += 1
                
                n.append(self.type(type_buffer))
                n.append(node(token("name", "enum")))
                for i in buffer:
                    if i == ',':
                        continue
                    n["enum"].append(node(i))
                
                root.append(n)
                buffer = []
                type_buffer = []

                continue
            buffer.append(i)
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
        if len(tokens) == 0:
            return tokens

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
        tokens.reverse()
        reverse = tokens
        tokens.reverse()
        for i in reverse:
            if type(i) == vector:
                index += 1
                continue
            if index == 0 and (i == '+' or i == '-'):
                index += 1
                continue
            if i == '+' or i == '-':
                return node(i, self.expr(tokens[:index]), self.expr(tokens[index+1:]))
            index += 1
        
        index = 0
        for i in tokens:
            if type(i) == vector:
                index += 1
                continue
            if index == 0 and i == '*':
                index += 1
                continue
            if i == '*' or i == '+':
                return node(i, self.expr(tokens[:index]), self.expr(tokens[index+1:]))
            index += 1
        
        return self.factor(tokens)

    def factor(self, tokens):
        if len(tokens) == 1:
            tokens = tokens[0]
            if type(tokens) == vector:
                if tokens.sig == 1:
                    return self.expr(tokens.e)
                if tokens.sig == 2:
                    return node(token("module", "[]"), self.expr(tokens.e))
            if tokens.type == "name":
                return node(tokens)
            if tokens.type == "number":
                return node(tokens)

        if len(tokens) == 2:
            first = tokens[0]
            second = tokens[1]
            if type(first) == vector:
                pass
            elif first.type == "name":
                if type(second) == vector:
                    if second.sig == 1:
                        root = node(first)
                        buffer = []
                        obj = []
                        for i in second.e:
                            if i == ',':
                                obj.append(buffer)
                                buffer = []
                                continue
                            buffer.append(i)
                        
                        if buffer != []:
                            obj.append(buffer)
                        
                        for i in obj:
                            root.append(self.expr(i))
                        
                        return node(token("call()", "call()"), root)

        return node(token("null", "null"))