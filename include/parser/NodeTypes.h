
#ifndef __NodeTypes_h__
#define __NodeTypes_h__

enum GlobalNodes {
    Node_Function,
    Node_Statement,
};

enum StatementNodes {
    Node_Var,
    Node_Return,
    Node_Assign,
    Node_Call,
};

enum ExprNodes {
    Node_Binary_Op,
    Node_Unary_Op,
    Node_Const,
};

enum ConstNodes {
    Node_Number,
    Node_String,
    Node_Id,
};

#endif // __NodeTypes_h__
