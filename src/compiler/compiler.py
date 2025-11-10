from parser.node import*

class output:
    def __init__(self, data, names):
        self.data = data
        self.names = names
    
    def append(self, obj):
        self.names.append(obj)

class compiler:
    def __init__(self, ast):
        self.ast: list[node] = ast

    def expr(self, obj: node, r):
        if obj == 'call()':
            return "call "+obj[0].value+'\n'
        return f"mov {r}, {obj.value}\n"

    def __call__(self):
        result = ""
        out = output("", [])
        
        for i in self.ast:
            if i == "private":
                pass


        out.data = result
        return out

