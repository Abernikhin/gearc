
#ifndef _Token_Type_
#define _Token_Type_

enum TokenType {
    
    Token_EOF,

    Token_Def_Kw, // def
    Token_Var_Kw, // var
    Token_Return_Kw, // return

    Token_Id,
    Token_Number,
    Token_String,

    Token_Open, // (
    Token_Close, // )
    Token_Begin, // {
    Token_End, // }

    Token_Colon, // :
    Token_Semicolon, // ;
    Token_Comma, // ,

    Token_Assign, // =
    Token_Plus, // +
    Token_Minus, // -
    Token_Multiply, // *
    Token_Divide, // /    

};

#endif // _Token_Type_
