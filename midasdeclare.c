#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include "midaslist.h"
#include "midaserrors.h"
#include "midasoperand.h"
#include "midasconvert.h"
#include <ctype.h>

MidasOperand MidasEvaluateExpression();
ConvertBlock *MidasFindStringConverter();

struct ActionBlock {
    void (*ActionRtn)();
};
                    
typedef struct ActionBlock ActionBlock;

                  
struct ConvertLink {
   List         *CList;
   ConvertBlock *CLink;
};

typedef struct ConvertLink ConvertLink;

static List TopLevelCommandList;
static List FunctionList;
static List QualifierList;

static List *QualifierLists[20];
static List **CurrentQualifierList = QualifierLists;

static char *MidasRemoveSpuriousJunk(in)
    char *in;
{
/*
 *   BUG: This should also support {&*aaa}
 */
    int l = strlen(in);
    if (*in == '&') { in++; l--; }
    if (*in == '*') { in++; l--; }
    
    if (*in == '{')
      {
        char *p = strncpy(XtMalloc(l-1),++in,l-2);
        *(p+l-2) = '\0';
        return p;
      }
    else if (l > 3 && strcmp("...",in+l-3) == 0)
      {
        char *p = strncpy(XtMalloc(l-2),in,l-3);
        *(p+l-3) = '\0';
        return p;
      }
    else return XtNewString(in);
}
static char *MidasRestOfLine(i,VerbVector,VerbCount)
int i;
char *VerbVector[];
int VerbCount;
{
    char *p;
    for (p = VerbVector[i]; p < VerbVector[VerbCount-1]; p++) if (*p == '\0') *p = ' ';
    return VerbVector[i];
}
void MidasTokenizeCommand(command,verbVector,verbCount,maxVerbs,delimiters)
    char  *command;
    char  *verbVector[];
    int   *verbCount;
    int   maxVerbs;
    char  *delimiters;
{
    char *p;
    *verbCount = 0;

    for (p=command;;)
      {
        int quotes = 0, paren = 0;
        for (; *p == ' '; p++);
        if (*p == '\0') break;
        if (*verbCount == maxVerbs) MidasError("Too many tokens found in line");
        verbVector[(*verbCount)++] = p;
        for (; (paren > 0 || quotes || strchr(delimiters,*p) == 0) && *p != '\0'; p++)
          if      (*p == '"') quotes = !quotes;
          else if (*p == '(') paren++;
          else if (*p == ')') paren--;
        if (*p == '\0') break;
        *p++ = '\0';
      }
}
void MidasDeclareInit()
{
    TopLevelCommandList = NullList;
    FunctionList = NullList;
    QualifierList = NullList;
}
void MidasDeclare(CList,command,ActionRtn,delimiters)
    List *CList;
    char *command;
    void (*ActionRtn)();
    char *delimiters;
{
    ActionBlock *ab;
    ConvertLink *cl;
    ListItem *item;
    int verbCount , i;
    char *verbVector[20];

    MidasTokenizeCommand(command,verbVector,&verbCount,
                         XtNumber(verbVector),delimiters);

    for (i=0; i < verbCount; i++)
      {
        char *token = verbVector[i];
        Boolean allupper = TRUE; 
        char *p = token;

        if (*token == '\0') continue;

        for (; *p != '\0'; p++) if (islower(*p)) { allupper = FALSE; break; }

        item = MidasFindItemInList(CList,token);
        if (item == 0) 
          {
            item = MidasAddItemToList(CList,token);
            cl = XtNew(ConvertLink);
            item->Pointer = cl;
            cl->CList = MidasCreateEmptyList("");

            if (allupper) cl->CLink = 0;
            else
              {
                MidasType type = MidasRemoveSpuriousJunk(token);
                cl->CLink = MidasFindStringConverter(type);
                XtFree(type);
              }
          }
        cl = item->Pointer;
        CList = cl->CList;
      }

     item = MidasFindItemInList(CList,"endofcommand");
     if (item == 0) 
       {
         item = MidasAddItemToList(CList,"endofcommand");
         item->Pointer = XtNew(ActionBlock);
       }
     ab = item->Pointer;
     ab->ActionRtn = ActionRtn;
}
void MidasDeclareVerb(command,ActionRtn)
    char *command;
    void (*ActionRtn)();
{
    char *buffer = XtNewString(command);
    MidasDeclare(&TopLevelCommandList,buffer,ActionRtn," ");
    XtFree(buffer);
}
void MidasDeclareFunction(command,ActionRtn)
    char *command;
    void (*ActionRtn)();
{
    char *buffer = XtNewString(command);
    MidasDeclare(&FunctionList,buffer,ActionRtn,"(,)");
    XtFree(buffer);
}
MidasDeclareQualifier(qualifier,type)
     char *qualifier;
     MidasType type;
{
     ListItem *item;
     ConvertBlock *ab,*cab;

     item = MidasAddItemToList(&QualifierList,qualifier);

     if (*type == '\0') item->Pointer = 0;
     else
       {
         cab = MidasFindStringConverter(type);
         ab = XtNew(ConvertBlock);
         *ab = *cab;
         item->Pointer = ab;
       } 
}
ActionBlock *MidasPrepareToDispatch(CList,command,delimiters,Argv,Argi,ArgC)
     List *CList;
     char *command;
     char *delimiters;
     MidasOperand Argv[10];
     Boolean Argi[10];
     int  *ArgC;
{
     int verbCount , i, Argc = 0;
     char *verbVector[100];
     ListItem *item = 0;
     void *result = 0;
     char *rest = 0;

     MidasTokenizeCommand(command,verbVector,&verbCount,
                          XtNumber(verbVector),delimiters);

     for (i=0; i < verbCount; i++)
       {
         char *c , *token = XtNewString(verbVector[i]);

         for (c=token ; *c != '\0' ; c++ ) *c = toupper(*c);

         item = MidasFindItemInList(CList,token);
         XtFree(token);

         if (item != 0) 
           {
             ConvertLink *cl = item->Pointer;
             CList = cl->CList;
           }
         else
           {
             for (item = CList->First; item != 0; item = item->Next )
               {
                 ConvertLink *cl = item->Pointer;
                 ConvertBlock *ab = cl->CLink;
                 char *entry = item->Entry;
                 Boolean indirect = (*entry == '&');
 
                 if (indirect) entry++;
                
                 if (ab != 0)
                   {
                     MidasOperand Temp;
                     if (strcmp("...",item->Entry+strlen(entry)-3))
                       {
                         if (*entry == '*')
                           {
                             Temp.Value.P = verbVector[i];
                             if (*Temp.Value.P == '"')
                               {
                                 Temp.Value.P++;
                                 *(Temp.Value.P+strlen(Temp.Value.P)-1) = '\0';
                               }
                             Temp.Dynamic = FALSE;
                             Temp.Type = MString;
                           }
                         else Temp = MidasEvaluateExpression(verbVector[i]);
                       }
                     else
                       {
                         rest = MidasRestOfLine(i,verbVector,verbCount);
                         if (*entry == '*')
                           {
                             Temp.Value.P = rest;
                             if (*Temp.Value.P == '"')
                               {
                                 Temp.Value.P++;
                                 *(Temp.Value.P+strlen(Temp.Value.P)-1) = '\0';
                               }
                             Temp.Dynamic = FALSE;
                             Temp.Type = MString;
                           }
                         else Temp = MidasEvaluateExpression(rest);
                         i = verbCount;
                       }
/*
 *                   if (strcmp(ab->FromType,Temp.Type) == 0) ab->ConvertRtn(&Temp); 
 *
 */
                     if (strcmp(ab->ToType,Temp.Type)) MidasConvertOperand(&Temp,ab->ToType);
                     Argv[Argc  ] = Temp;
                     Argi[Argc++] = indirect;
                     CList = cl->CList;
                     break;
                   }
               }
           }
       }

     /* Skip optional arguments until end of command */

     if (item != 0) 
       {
         for (; (item = MidasFindItemInList(CList,"endofcommand")) == 0;)
           {
             for (item = CList->First; item != 0; item = item->Next )
               if (*item->Entry == '{') 
                 {
                   ConvertLink *cl = item->Pointer;
                   Argv[Argc].Value.I = 0;
                   Argv[Argc].Dynamic = FALSE;
                   Argv[Argc].Type = MInt;
                   Argi[Argc] = FALSE;
                   Argc++;
                   CList = cl->CList;
                   break;    
                 }
             if (item == 0) MidasError("Premature end of command found");
           }
       }
     if (item == 0) 
       {
         for (i=0; i<Argc; i++) if (Argv[i].Dynamic) XtFree((char *)Argv[i].Value.P);
         MidasError("Syntax error in expression");
       }
     *ArgC = Argc;
     return item->Pointer;
}
MidasOperand MidasCallFunction(in)
     char **in;
{
     MidasOperand result;
     if (setjmp(JmpEnv[NumJump++]) == 0)
       {
         char buffer[2000];
         char *p = *in, *q = buffer;
         int n=0 , m=0;
         for (; *p != '\0'; p++)
           {
             *q = *p;
             if      (*p == '"') m = !m;
             else if (!m && *p == '(') { if (++n == 1) *q = ' '; }
             else if (!m && *p == ')') { if (--n == 0) break;    }
             q++;
           }
         if (n != 0) MidasError("Parentheses error in expression: %s",*in+1);
         if (m != 0) MidasError("Mismatched quotes in expression: %s",*in+1);
         *q = '\0';
         {
           MidasOperand Argv[10];
           Boolean Argi[10];
           int Argc, i;

           ActionBlock *ab = MidasPrepareToDispatch(&FunctionList,buffer," ,",Argv,Argi,&Argc);
           MidasOperand (*routine)() = (MidasOperand (*)()) ab->ActionRtn; 
           int A[10];
           for (i=0; i<Argc; i++) 
             if (Argi[i]) A[i]=(int) &Argv[i].Value.I;
             else         A[i]=Argv[i].Value.I;


           switch (Argc)
             {
               case  0: result = routine();
                        break;
               case  1: result = routine(A[0]);
                        break;
               case  2: result = routine(A[0],A[1]);
                        break;
               case  3: result = routine(A[0],A[1],A[2]);
                        break;
               case  4: result = routine(A[0],A[1],A[2],A[3]);
                        break;
               case  5: result = routine(A[0],A[1],A[2],A[3],A[4]);
                        break;
               case  6: result = routine(A[0],A[1],A[2],A[3],A[4],A[5]);
                        break;
               case  7: result = routine(A[0],A[1],A[2],A[3],A[4],A[5],A[6]);
                        break;
               case  8: result = routine(A[0],A[1],A[2],A[3],A[4],A[5],A[6],A[7]);
                        break;
               case  9: result = routine(A[0],A[1],A[2],A[3],A[4],A[5],A[6],A[7],A[8]);
                        break;
               case 10: result = routine(A[0],A[1],A[2],A[3],A[4],A[5],A[6],A[7],A[8],A[9]);
           }
           for (i=0; i<Argc; i++) if (Argv[i].Dynamic) XtFree((char *)Argv[i].Value.P);
         }
         *in = p;
         NumJump--;
       }
     else
       {
         MidasError("Error invoking function: %s",*in);
       }
  return result;
}
static List *MidasFindCommandQualifiers(QL,command)
List *QL;
char *command;
{ 
  char *p, *q, *r, delim;
  int Quotes = 0, paren = 0;
  ListItem *item;
  ListItem *new;
  List *CurrentQualifierList = MidasCreateEmptyList("CurrentQualifiers");

  for (p=command; *p != '\0'; p++)
    if (*p == '"') Quotes = 1 - Quotes;
    else if (!Quotes && *p == '(') paren++;
    else if (!Quotes && *p == ')') paren--; 
    else if (!Quotes && paren == 0 && *p == ':')
      {
        for (q = p+1 ; *q != ' ' && *q != ':' && *q != '=' && *q != '\0'; q++)
          *q = toupper(*q);

        delim = *q;
        *q = '\0';

        item = MidasFindItemInList(QL,p+1);
        if (item == 0) MidasError("Unrecognized qualifier %s",p+1);

        new = MidasAddItemToList(CurrentQualifierList,p+1);

        if (delim == '=')
          {
            MidasOperand *Temp = XtNew(MidasOperand);
            for (r = q + 1; *r != ':' && *r != '\0' && *r != ' '; r++);
            delim = *r;
            *r = '\0';
            new->Pointer = Temp;
            Temp->Type = MString;
            Temp->Dynamic = TRUE;
            Temp->Value.P = (XtPointer) XtNewString(q+1);
          }
        else 
          {
            r = q;
            new->Pointer = 0;
          }
        for (; p < r ; p++) *p=' ';
        *p = delim;
        p--;
      }
  return CurrentQualifierList;
}
void MidasDispatchCommand(command)
     char *command;
{
     char *p;
     char *buffer = XtNewString(command);
     List *QL = 0;
     ListItem *item;  

     if (setjmp(JmpEnv[NumJump++]) == 0)
       {
         QL = MidasFindCommandQualifiers(&QualifierList,buffer);
         *++CurrentQualifierList = QL;
         {
           MidasOperand Argv[10];
           Boolean Argi[10];
           int Argc, i;
           ActionBlock *ab = MidasPrepareToDispatch(&TopLevelCommandList,buffer," ",Argv,Argi,&Argc);
           int A[10];
           for (i=0; i<Argc; i++) 
             if (Argi[i]) A[i]=(int) &Argv[i].Value.I;
             else         A[i]=Argv[i].Value.I;

           switch (Argc)
             {
               case  0: ab->ActionRtn();
                        break;
               case  1: ab->ActionRtn(A[0]);
                        break;
               case  2: ab->ActionRtn(A[0],A[1]);
                        break;
               case  3: ab->ActionRtn(A[0],A[1],A[2]);
                        break;
               case  4: ab->ActionRtn(A[0],A[1],A[2],A[3]);
                        break;
               case  5: ab->ActionRtn(A[0],A[1],A[2],A[3],A[4]);
                        break;
               case  6: ab->ActionRtn(A[0],A[1],A[2],A[3],A[4],A[5]);
                        break;
               case  7: ab->ActionRtn(A[0],A[1],A[2],A[3],A[4],A[5],A[6]);
                        break;
               case  8: ab->ActionRtn(A[0],A[1],A[2],A[3],A[4],A[5],A[6],A[7]);
                        break;
               case  9: ab->ActionRtn(A[0],A[1],A[2],A[3],A[4],A[5],A[6],A[7],A[8]);
                        break;
               case 10: ab->ActionRtn(A[0],A[1],A[2],A[3],A[4],A[5],A[6],A[7],A[8],A[9]);
           }
           for (i=0; i<Argc; i++) if (Argv[i].Dynamic) XtFree((char *) Argv[i].Value.P);
         }
       for (item = QL->First; item != 0; item = item->Next)
         {
           MidasOperand *Temp = item->Pointer;
           if (Temp != 0 && Temp->Dynamic) XtFree((char *) Temp->Value.P);
         }
       MidasDestroyList(*CurrentQualifierList--);
       XtFree(buffer);
       NumJump--;
       }
     else
       {
         if (QL != 0) 
           {  
             for (item = QL->First; item != 0; item = item->Next)
               {
                 MidasOperand *Temp = item->Pointer;
                 if (Temp != 0 && Temp->Dynamic) XtFree((char *) Temp->Value.P);
               }
             MidasDestroyList(*CurrentQualifierList--);
           } 
         XtFree(buffer);
         MidasError("Illegal command");
       }
}
Boolean MidasGetQualifier(Qualifier,Result)
char *Qualifier;
XtPointer *Result;
{
   ConvertBlock *ab;
   ListItem *qitem, *item;

   if (*CurrentQualifierList == 0) return FALSE;

   item = MidasFindItemInList(*CurrentQualifierList, Qualifier);
   if (item == 0) return FALSE;

   qitem = MidasFindItemInList(&QualifierList,Qualifier);
   ab = qitem->Pointer;
   if (ab != 0) 
     { 
       MidasOperand *Temp = item->Pointer;
/*
 * Speed up temporarily removed.   
 *      if (strcmp(Temp->Type,ab->FromType) == 0) ab->ConvertRtn(Temp);
 *
 */
       if (strcmp(Temp->Type,ab->ToType)) MidasConvertOperand(Temp,ab->ToType);
       *Result = Temp->Value.P; 
     }
   return TRUE;
}
void MidasSetQualifier(Qualifier,Value)
char *Qualifier;
char *Value;
{
   ActionBlock *ab;
   ListItem *qitem, *item;

   if (*CurrentQualifierList == 0) 
     *CurrentQualifierList = MidasCreateEmptyList("CurrentQualifiers");   

   item = MidasFindItemInList(*CurrentQualifierList, Qualifier);
   if (item == 0)
     {
       item = MidasAddItemToList(*CurrentQualifierList, Qualifier);
     }
   else
     {
       if (item->Pointer != 0) XtFree(item->Pointer);
     }

   if (Value == NULL) item->Pointer = NULL;
   else 
     {
       MidasOperand *Operand = XtNew(MidasOperand);
       item->Pointer = Operand;
       Operand->Type = MString;
       Operand->Dynamic = TRUE;
       Operand->Value.P = (XtPointer) XtNewString(Value);
     }
}
