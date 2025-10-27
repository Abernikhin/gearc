from lexer import token

class node:
    def __init__(self, value, *child):
        if type(value) != token:
            raise TypeError("wrong type in node %s"%type(value))
        self.type = value.type
        self.value = value.value
        for i in child:
            if type(i) != node:
                raise TypeError("node child has wrong type %s"%type(i))
        self.child = list(child)
    
    def append(self, value):
        self.child.append(value)
    
    def __getitem__(self, index):
        return self.child[index]
    
    def __eq__(self, value):
        if type(value) != str:
            raise TypeError("wrong type in node.__eq__")
        return True if self.value == value else False

    def __ne__(self, value):
        if type(value) != str:
            raise TypeError("wrong type in node.__ne__")
        return True if self.value != value else False

    def info(self, c=0):
        print("    "*c+"|> "+self.value)
        for i in self.child:
            i.info(c+1)