class token:
    def __init__(self, _type, _value):
        if type(_type) != str or type(_value) != str:
            raise TypeError("wrong token init arguments")
        self.type = _type
        self.value = _value

    def __eq__(self, value):
        if type(value) != str:
            raise TypeError(f"wrong value type in token.__eq__ - {type(value)}")
        return True if self.value == value else False

    def __ne__(self, value):
        if type(value) != str:
            raise TypeError(f"wrong value type in token.__ne__ - {type(value)}")
        return True if self.value != value else False

    def __str__(self):
        return f"token(type: {self.type}, value: {self.value});"
