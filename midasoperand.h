#ifndef MIDASOPERAND
#define MIDASOPERAND

typedef char *MidasType;

typedef struct {

  char *Symbol;
  int  Precedence;
  MidasType Op1;
  MidasType Op2;
  MidasType Result; 

} MidasOperator;

typedef struct {

  MidasType Type;
  Boolean   Dynamic;
  union {
    XtPointer P;
    int I;
    float F;
    short S;
    Boolean B;
  } Value;

} MidasOperand;

typedef struct {

  MidasOperand   Operand;
  MidasOperator *Op;

} MidasStack;

#define MString  "String"
#define MInt     "Int"
#define MBoolean "Boolean"
#define MFloat   "Float"
#define MWidget  "Widget"
#define MNumber  "Number"
#endif
