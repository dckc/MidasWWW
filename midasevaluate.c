#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include "midasoperand.h"
#include <string.h>

extern Widget ActiveWidget;
MidasOperand MidasGetIngotValue();
MidasOperand MidasCallFunction();
XtPointer MidasFindIngot();

static char *UnaryOperatorCharacters = "+-";
static char *BinaryOperatorCharacters = "+-*/><!=&|";

static MidasOperator BinaryOperators[] = {
                                {"==", 6,  MString,  MString, MBoolean},
                                {"!=", 6,  MString,  MString, MBoolean},
                                {"<=", 7,  MNumber,  MNumber, MBoolean},
                                {">=", 7,  MNumber,  MNumber, MBoolean},
                                {"&&", 1, MBoolean, MBoolean, MBoolean},
                                {"||", 1, MBoolean, MBoolean, MBoolean}, 
                                {"//", 9,     MInt,     MInt,     MInt},
                                {"++",10,  MString,  MString,  MString}, 
                                {"<" , 7,  MNumber,  MNumber, MBoolean},
                                {">" , 7,  MNumber,  MNumber, MBoolean},
                                {"+" , 8,  MNumber,  MNumber,  MNumber},
                                {"-" , 8,  MNumber,  MNumber,  MNumber},
                                {"*" , 9,  MNumber,  MNumber,  MNumber},
                                {"/" , 9,  MNumber,  MNumber,  MNumber}};

static char *SBool[2] = {"False" , "True"};

MidasOperand MidasApplyOperation(Op1,Op2,Op)
  MidasOperand  Op1; 
  MidasOperand  Op2; 
  MidasOperator *Op;
{
  MidasOperand Result;
  Boolean Float = FALSE;
  int v1,v2,v;
  float f;

  Result.Dynamic = FALSE;

  if (strcmp(Op1.Type,Op->Op1)) MidasConvertOperand(&Op1,Op->Op1);    
  if (strcmp(Op2.Type,Op->Op2)) MidasConvertOperand(&Op2,Op->Op2);

  if (strcmp(Op->Op1,MNumber) == 0)
    { 
      if      (strcmp(Op1.Type,MFloat) == 0) Float = TRUE;
      else if (strcmp(Op2.Type,MFloat) == 0) Float = TRUE;

      if (Float && strcmp(Op1.Type,MFloat)) MidasConvertOperand(&Op1,MFloat);    
      if (Float && strcmp(Op2.Type,MFloat)) MidasConvertOperand(&Op2,MFloat);
    }

  v1 = Op1.Value.I;
  v2 = Op2.Value.I;

  if      (strcmp(Op->Symbol,"==") == 0) v = strcmp((char *)v1,(char *)v2) == 0; 
  else if (strcmp(Op->Symbol,"!=") == 0) v = strcmp((char *)v1,(char *)v2) != 0; 
  else if (strcmp(Op->Symbol,"&&") == 0) v =  v1 ?   v2 : FALSE; 
  else if (strcmp(Op->Symbol,"||") == 0) v =  v1 ? TRUE :  v2; 
  else if (strcmp(Op->Symbol,"++") == 0) 
    {
      v = (int) XtMalloc(strlen((char *)v1)+strlen((char *)v2)+1);
      strcpy((char *)v,(char *)v1);
      strcat((char *)v,(char *)v2);
      Result.Dynamic = TRUE;
    } 
  else if (strcmp(Op->Symbol,"//") == 0) 
    {
      if ( v2 == 0) MidasError("Integer division by zero"); 
      v =  v1 /  v2;
    }
  else if (Float)
    {
      float f1 = Op1.Value.F;
      float f2 = Op2.Value.F;

      if      (strcmp(Op->Symbol,">=") == 0) f =  f1 >=  f2; 
      else if (strcmp(Op->Symbol,"<=") == 0) f =  f1 <=  f2; 
      else if (strcmp(Op->Symbol,">" ) == 0) f =  f1 >   f2; 
      else if (strcmp(Op->Symbol,"<" ) == 0) f =  f1 <   f2; 
      else if (strcmp(Op->Symbol,"-")  == 0) f =  f1 -   f2;
      else if (strcmp(Op->Symbol,"+")  == 0) f =  f1 +   f2;
      else if (strcmp(Op->Symbol,"*")  == 0) f =  f1 *   f2;
      else if (strcmp(Op->Symbol,"/")  == 0) 
        {
          if (f2 == 0) MidasError("Division by zero"); 
          f = f1 / f2;
        }
      else MidasError("Midas Internal Error: MidasApplyOperation");
    }
  else
    {
      if      (strcmp(Op->Symbol,">=") == 0) v =  v1 >=  v2; 
      else if (strcmp(Op->Symbol,"<=") == 0) v =  v1 <=  v2; 
      else if (strcmp(Op->Symbol,">" ) == 0) v =  v1 >   v2; 
      else if (strcmp(Op->Symbol,"<" ) == 0) v =  v1 <   v2; 
      else if (strcmp(Op->Symbol,"-")  == 0) v =  v1 -   v2;
      else if (strcmp(Op->Symbol,"+")  == 0) v =  v1 +   v2;
      else if (strcmp(Op->Symbol,"*")  == 0) v =  v1 *   v2;
      else if (strcmp(Op->Symbol,"/")  == 0) 
        {
          float r;
          if (v2 == 0) MidasError("Division by zero"); 
          if (v1 % v2 == 0) v = v1 / v2;
          else
            {
              f = (float) v1 / (float) v2;
              Float = TRUE;
            }
        }
      else MidasError("Midas Internal Error: MidasApplyOperation");
    }

  if (Op1.Dynamic) XtFree((char *)Op1.Value.P);
  if (Op2.Dynamic) XtFree((char *)Op2.Value.P); 

  if (Float) Result.Value.F = f;
  else       Result.Value.I = v;

  if (strcmp(Op->Result,MNumber) == 0) Result.Type = Float ? MFloat : MInt;
  else Result.Type = Op->Result;

  return Result; 
}
static MidasOperand MidasEvaluateFunction(in,outlen)
char *in;
int *outlen;
{
  MidasOperand Temp;
  char *q = in;

  Temp = MidasCallFunction(&q);
  *outlen = q - in + 1;
  return Temp;
}
MidasOperand MidasEvaluateExpression(in)
char *in;
{
   MidasStack Stack[10];
   MidasStack *SP = &Stack[0];
   MidasOperator Dummy, EndOfLine, *CurrentOperator;   
   char Unary = '\0';

#ifdef debug
   printf("Evaluating: %s\n",in);
#endif

   Dummy.Precedence = -1;
   EndOfLine.Precedence = 0;
   EndOfLine.Symbol = "";
   SP->Op = &Dummy; 

   for (;;)
     {
       int l1;
       if (*in != '\0' && strchr(UnaryOperatorCharacters,*in) != 0) Unary = *in++;
/*
       Tokens in expressions may be:

          a) sub-expressions (in parentheses)
          b) Function invokations
          c) Quoted strings
          d) Ingots
          e) Unquoted strings

       First deal with Sub-expressions 
*/
       if (*in == '(')
         {
           int n = 1, q = 0;
           char *p = in + 1;
           for (; *p != '\0'; p++) 
             {
               if (*p == '"') q = !q;
               else if (!q && *p == '(') n++;
               else if (!q && *p == ')') if (--n == 0) break;
             }    
           if (q != 0) MidasError("Mismatched quotes in expression: %s",in);
           if (n != 0) MidasError("Parenthesis error in expression: %s",in);
           *p = '\0';
           in++;
           SP->Operand = MidasEvaluateExpression(in); 
           l1 = p - in + 1;
         } 
/*
       Now deal with quoted strings
*/
       else if (*in == '"')
         {
           int q = 1;
           char *p = in + 1;
           for (; *p != '\0'; p++) if (*p == '"') { q = 0;  break; }
           if (q != 0) MidasError("Mismatched quotes in expression: %s",in);
           *p = '\0';
           in++;
           SP->Operand.Type = MString;
           SP->Operand.Dynamic = FALSE;
           SP->Operand.Value.P = (XtPointer) in; 
           l1 = p - in + 1;
         } 
       else
/*
       Could be a function invokation
*/
         {
           char *l2 = strchr(in,'(');
           l1 = strcspn(in,BinaryOperatorCharacters); 
           if (l2 != NULL && l2 < in+l1) 
             {
               SP->Operand = MidasEvaluateFunction(in,&l1);
             }
           else 
             {
/*
       Or it could be an ingot             
*/ 
               XtPointer Ingot;               
               char temp = *(in + l1);
               *(in + l1) = '\0';

               Ingot = MidasFindIngot(ActiveWidget,in);
               if (Ingot != 0) SP->Operand = MidasGetIngotValue(Ingot);
               else 
                 {
                   SP->Operand.Type    = MString;
                   SP->Operand.Dynamic = FALSE;
                   SP->Operand.Value.P = (XtPointer) in;
                 } 
               *(in + l1) = temp;
             }
         }      
/* 
     Now deal with any pending unary operator
*/
       if (Unary == '-') 
         {
           MidasConvertOperand(&SP->Operand,MInt);
           SP->Operand.Value.I = -(int) SP->Operand.Value.I;
         }

       if (*(in+l1) == '\0') CurrentOperator = &EndOfLine;
       else
         {
           int op; 
           for (op = 0;  op < XtNumber(BinaryOperators);  op++)
      	     if (strncmp(in+l1,BinaryOperators[op].Symbol,
                         strlen(BinaryOperators[op].Symbol)) == 0) break;
 
           if (op == XtNumber(BinaryOperators)) 
             MidasError("Unrecognised operator %s in expression",in+l1);
           CurrentOperator = &BinaryOperators[op];
         }

       *(in+l1) = '\0';
       in += l1 + strlen(CurrentOperator->Symbol);

       for (; CurrentOperator->Precedence <= SP->Op->Precedence; )
         { 
           (SP-1)->Operand = MidasApplyOperation((SP-1)->Operand,SP->Operand,SP->Op);
           SP--;
         }

       if (strcmp(CurrentOperator->Symbol,"&&") == 0)
         { 
           if (strcmp(SP->Operand.Type,MBoolean)) MidasConvertOperand(&SP->Operand,MBoolean);
           if (SP->Operand.Value.I == FALSE) break; 
         } 
       else if (strcmp(CurrentOperator->Symbol,"||") == 0)
         { 
           if (strcmp(SP->Operand.Type,MBoolean)) MidasConvertOperand(&SP->Operand,MBoolean);
           if (SP->Operand.Value.I == TRUE) break; 
         } 

       if (CurrentOperator == &EndOfLine) break;
       (++SP)->Op = CurrentOperator;
    }       
#ifdef debug
  printf ("Result is %s = ",SP->Operand.Type);
  if (strcmp(SP->Operand.Type,MString) == 0) printf("%s\n",SP->Operand.Value.I);
  else                                       printf("%d\n",SP->Operand.Value.I); 
#endif

  return SP->Operand; 
}
void MidasForceEvaluateExpression(in,out)
char **in;
char **out;
{
  MidasOperand Result;
  char temp, *p = *in+1, *pp, *buffer;
  int q = 0, m = 0;

  for (; *p != '\0' && *p != '\n'; p++)
    {
      if (*p == '"')
        {
          if (m == 0 && !q) break; 
          else q = !q;
        }
      else if (!q && *p == '(') m++;
      else if (!q && *p == ')') m--;
      else if (!q && m == 0 && *p == ' ') break;
    }
    
  temp = *p;
  *p = '\0';
  buffer = XtNewString(*in+1);
  *p = temp;

  Result = MidasEvaluateExpression(buffer);
  if (strcmp(Result.Type,MString) != 0) MidasConvertOperand(&Result,MString);
      
  for (pp = Result.Value.P; *pp != '\0';) *(*out)++ = *pp++;
  if (Result.Dynamic) XtFree((char *)Result.Value.P);

  XtFree(buffer);
  *in = p - 1;
}
int MidasConvertToInteger(Operand)
MidasOperand Operand;
{
  if (strcmp(Operand.Type,MInt)) MidasConvertOperand(&Operand,MInt);
  return Operand.Value.I;
}
char *MidasConvertToString(Operand)
MidasOperand Operand;
{
  if (strcmp(Operand.Type,MString)) MidasConvertOperand(&Operand,MString);
  return XtNewString(Operand.Value.P);
}
char *MidasConvertToBoolean(Operand)
MidasOperand Operand;
{
  if (strcmp(Operand.Type,MBoolean)) MidasConvertOperand(&Operand,MString);
  return Operand.Value.P;
}
MidasOperand MidasConvertFromString(string)
char *string;
{
  MidasOperand Temp;
  Temp.Type = MString;
  Temp.Dynamic = TRUE;
  Temp.Value.P = XtNewString(string);
  return Temp;
}
MidasOperand MidasConvertFromInteger(i)
int i;
{
  MidasOperand Temp;
  Temp.Type = MInt;
  Temp.Dynamic = FALSE;
  Temp.Value.I = i;
  return Temp;
}
MidasOperand MidasConvertFromBoolean(b)
int b;
{
  MidasOperand Temp;
  Temp.Type = MBoolean;
  Temp.Dynamic = FALSE;
  Temp.Value.I = (b != 0);
  return Temp;

}
