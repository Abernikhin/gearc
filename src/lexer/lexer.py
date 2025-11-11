import re
from lexer.token import token

class wrong_token_error(Exception):
    pass

class lexer():
    def __init__(self, source: str):
        self.code: str = source
        self.rules: dict[str, str] = {
            "skip": r"(\s+)",
            "ignore": r"//([.\\ ]*)",
            "number": r"([123456789]\d+)",
            "name": r"([\w\_]+[\d\w\_]*)",
            "addres": r"(\&)",
            "begin": r"(\{)",
            "end": r"(\})",
            "right": r"(\))",
            "left": r"(\()",
            "arrow": r"(->)",
            "plus": r"(\+)",
            "minus": r"(\-)",
            "star": r"(\*)",
            "div": r"(\/)",
            "eq": "(\=)",
            "colon": r"(\:)",
            "comma": r"(\,)",
            "semicolon": r"(\;)"
        }

    def reline(self, obj: str):
        result = ""
        for i in range(len(obj), len(self.code)):
            result += self.code[i]
        
        self.code = result

    def __call__(self):
        result: list[token] = []
        while 0 < len(self.code):
            for i in self.rules:
                mo = re.match(self.rules[i], self.code)
                if mo:
                    if i != "skip" and i != "ignore":
                        result.append(token(i, mo.group(1)))
                        self.reline(mo.group(0))
                        break
                    else:
                        self.reline(mo.group(0))
                        break
            else:
                raise wrong_token_error(self.code[0])
        return result