class vector:
    def __init__(self, _values):
        if type(_values) != list:
            raise TypeError("wrong type in vector")
        self.v = _values