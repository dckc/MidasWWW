#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include "midaslist.h"     
#include "midasoperand.h"
#include "midaswidget.h"
#include "midasshell.h"
#include <string.h>

MidasOperand MidasConvertFromInteger();
MidasOperand MidasConvertFromString();
MidasOperand MidasConvertFromBoolean();
MidasWidget *MidasWidgetToMW();
MidasShell  *MidasGetShell();
char *MidasGetWidgetName();
char *MidasConvertToString();

static char *Option = "ExecuteWhenCreated";

typedef struct _IR {char *Command; Boolean ExecuteWhenCreated; } IngotParm;

#define Offset(field) XtOffsetOf(IngotParm,field)

static XtResource resources[] = {
      {"midasIngots","MidasIngots",XtRString ,sizeof(char *) ,Offset(Command)           ,XtRImmediate,(XtPointer)0},
      {"midasIngots","MidasIngots",XtRBoolean,sizeof(Boolean),Offset(ExecuteWhenCreated),XtRImmediate,(XtPointer)True}};

#undef Offset

static List *IngotRList;

struct Ingot {
   Boolean              Valued;
   MidasOperand         Operand;
   char                *Command;
   Widget               Parent;
   struct Ingot        *ValueOwner;
   List                *CallbackList;
   Boolean              ExecuteWhenCreated;
};          
typedef struct Ingot Ingot;


static XtConvertArgRec ConvertWidgetArg[] = {{XtBaseOffset,0,sizeof(Widget)}};

Ingot *MidasFindIngotHere(w,Name)
Widget w;
char *Name;
{
    MidasWidget *mw = MidasWidgetToMW(w);
    ListItem *i;
    List *TList = mw->IngotList; 

    if (TList != NULL)
      {
        i = MidasFindItemInList(TList,Name);
        if (i != 0) return i->Pointer;
      }
    return 0;

}
Ingot *MidasFindIngot(w,Name)
Widget w;
char *Name;
{
    MidasShell *ms = MidasGetShell(w);

    for (;; w = XtParent(w))
      {
        Ingot *T = MidasFindIngotHere(w,Name);
        if (T != 0) return T;
        if (w == ms->Widget) break; 
      }      
    return 0;
}
static void MidasDestroyIngot(T)
Ingot *T;
/*
    This routine is called from MidasDestroyIngots during the destruction of
    a widget that has ingots. It assumes that all child widgets are also being 
    destroyed and therefore does not worry about them.
*/
{
    if (!T->Valued) 
      {
        Ingot *ValueOwner = T->ValueOwner;
        List *CallbackList = ValueOwner->CallbackList;
        if (CallbackList != 0) 
          {
            ListItem *item;
            for (item = CallbackList->First; item != 0 ; item = item->Next)
              {
                if (item->Pointer == T) 
                  {
                   item->Pointer = 0; 
                   MidasRemoveItemFromList(CallbackList,item);
                   break;
                  }
              }
          } 
      }
    else if (T->Operand.Dynamic) XtFree((char *)T->Operand.Value.P);
    if (T->CallbackList != 0) MidasDestroyList(T->CallbackList);
    XtFree(T->Command);
}
void MidasDestroyIngots(mw)
MidasWidget *mw;
/*
 *  This routine is called whenever a widget that has ingots
 *  goes and gets itself destroyed.
 */
{
    ListItem *item;
    List *TList = mw->IngotList;

    for (item = TList->First; item != 0; item = item->Next)
      {
        Ingot *T = item->Pointer;
        MidasDestroyIngot(T);
        item->Pointer = 0;
      } 
    MidasDestroyList(TList);
}
Ingot *MidasCreateIngot(w,Name)
Widget w;
char *Name;
/*
    If this ingot does not already have an explicit ingot of this name it is created.
    A pointer to the (newly created) ingot is returned;
    A created ingot initially has no value and no callbacks;
*/
{
    MidasWidget *mw = MidasWidgetToMW(w);
    Ingot *T;
    ListItem *item;
    List *TList = mw->IngotList;

    if (TList == NULL) 
      {
        mw->IngotList = TList = MidasCreateEmptyList(XtName(w));
      }

    item = MidasFindItemInList(TList,Name);
    if (item == 0)
      {
        T = XtNew(Ingot);
        T->CallbackList = 0;
        T->Command = 0;
        T->Parent = w;
        T->ValueOwner = 0;
        T->Valued = FALSE;
        T->ExecuteWhenCreated = TRUE;

        item = MidasAddItemToList(TList,Name);
        item->Pointer = T;
      }
    else T = item->Pointer;
    return T;
}
void MidasInheritIngotValue(T,Name)
Ingot *T;
char *Name;
{
    Ingot *ValueOwner;
    MidasShell *ms = MidasGetShell(T->Parent);
    if (strncmp(Name,"Midas",5) && ms->Widget != T->Parent) 
      ValueOwner = MidasFindIngot(XtParent(T->Parent),Name);
    else ValueOwner = 0;

    if (ValueOwner == 0)
      {
        T->Valued = TRUE;
        T->Operand.Value.P   = (XtPointer)"";
        T->Operand.Dynamic = FALSE;
        T->Operand.Type    = MString;
      }
    else 
      {
        if (ValueOwner->Valued == 0) ValueOwner = ValueOwner->ValueOwner;
        T->ValueOwner = ValueOwner; 
      }

#ifdef debug
    if (T->Valued) printf("Ingot %s in %s did not ingerit a value\n",Name,XtName(T->Parent));
    else           printf("Ingot %s inherited value %s from %s\n",Name,ValueOwner->Operand.Value,
                                                                  XtName(ValueOwner->Parent));
#endif
}
void MidasSetIngotValue(T,Operand)
Ingot *T;
MidasOperand Operand;
/*
    This routine sets the value of the specified ingot
    and calls the value changed callbacks belonging to the ingot
*/
{ 
    if (T->Valued && T->Operand.Dynamic) XtFree((char *)T->Operand.Value.P);
    T->Operand = Operand;
/*
 *  BIG KLUGE .. must be fixed soon
 */
    if (strcmp(Operand.Type,MString)==0 && Operand.Dynamic==FALSE)
      {
        T->Operand.Value.P = XtNewString(Operand.Value.P);
        T->Operand.Dynamic = TRUE;
      }
    T->Valued = TRUE;

    if (T->Command != 0) MidasDispatchCommandCallback(T->Parent,T->Command,0);
  
    if (T->CallbackList != 0)
      {
        ListItem *item;
        for (item = T->CallbackList->First; item != 0; item = item->Next)
          {
            Ingot *T = item->Pointer;
            MidasDispatchCommandCallback(T->Parent,T->Command,0);
          }
      }
}
void MidasSetCallbackValue(T,Command)
Ingot *T;
char *Command;
{
    Widget SW, w = T->Parent;
    MidasShell *ms = MidasGetShell(w);

    if (T->Command == 0)
      {
        if (T->Valued == 0)
          {
            ListItem *item;
            Ingot *ValueOwner = T->ValueOwner;
            if (ValueOwner->CallbackList == 0) ValueOwner->CallbackList = MidasCreateEmptyList("");
            item = MidasAddItemToListPos(ValueOwner->CallbackList,XtName(w),0);
            item->Pointer = T;
          } 
      }   
    else XtFree(T->Command);
    T->Command = XtNewString(Command);
}
MidasOperand MidasGetIngotValue(T)
Ingot *T;
{
   MidasOperand Temp;
   if (T->Valued == 0) T = T->ValueOwner;
   Temp = T->Operand;
   Temp.Dynamic = FALSE; /* The ingot retains ownership of the operand */
   return Temp;
}
char *MidasGetCallbackValue(T)
Ingot *T;
{
   if (T->Command == 0) return XtNewString("");
   else                 return XtNewString(T->Command);
}
MidasOperand MidasGetCallback(w,Name)
Widget w;
char *Name;
{
   MidasOperand Temp;
   Ingot *T = MidasFindIngotHere(w,Name);
   if (T == 0) 
     {
       Temp.Value.P = "";
       Temp.Dynamic = FALSE;
     }
   else 
     {
       Temp.Value.P = MidasGetCallbackValue(T);
       Temp.Dynamic = TRUE;
     }
   Temp.Type = MString;
   return Temp;
}
MidasOperand MidasGetCreateCallback(w)
Widget w;
{
   Ingot *T = MidasFindIngotHere(w,"midasCreateCallback");
   if (T == 0) 
     {
       MidasOperand Temp;
       Temp.Value.P = "";
       Temp.Dynamic = FALSE;
       Temp.Type = MString;
       return Temp;
     } 
   else return MidasGetIngotValue(T);   
}
MidasOperand MidasGetIngot(w,Name)
Widget w;
char *Name;
{
   Ingot *T = MidasFindIngot(w,Name);
   if (T == 0) MidasError("Can not find ingot %s",Name);
   return MidasGetIngotValue(T);   
}
MidasOperand MidasGetValueOwner(w,Name)
Widget w;
char *Name;
{
   MidasOperand Temp;
   Ingot *T = MidasFindIngot(w,Name);
   if (T == 0) MidasError("Can not find ingot %s",Name);

   if (T->Valued) Temp.Value.P = (XtPointer) T->Parent;
   else           Temp.Value.P = (XtPointer) T->ValueOwner->Parent;
   Temp.Type = MWidget;
   Temp.Dynamic = FALSE;

   return Temp;   
}
MidasOperand MidasGetIngotOption(w,Name)
Widget w;
char *Name;
{
   MidasOperand Temp;
   Ingot *T = MidasFindIngot(w,Name);
   if (T == 0) MidasError("Can not find ingot %s",Name);

   Temp.Value.I = T->ExecuteWhenCreated;
   Temp.Type = MBoolean;
   Temp.Dynamic = FALSE;

   return Temp;   
}
void MidasSetIngotOption(w,Name,Opt)
Widget w;
char *Name;
Boolean Opt;
{
   MidasOperand Temp;
   Ingot *T = MidasFindIngot(w,Name);
   if (T == 0) MidasError("Can not find ingot %s",Name);

   T->ExecuteWhenCreated = Opt;  
}
void MidasSetIngot(w,Name,Temp)
Widget w;
char *Name;
MidasOperand Temp;
{
   Ingot *T = MidasCreateIngot(w, Name);
   MidasSetIngotValue(T, Temp);
#ifdef debug
   printf("Set Ingot %s in %s to value %s\n",Name,XtName(w),Temp.Value.I);
#endif
}
void MidasSetIngotString(w,Name,Value)
Widget w; 
char *Name;
char *Value;
{
   MidasSetIngot(w,Name,MidasConvertFromString(Value));
}
void MidasSetIngotP(w,Name,Temp)
Widget w;
char *Name;
MidasOperand *Temp;
{
   MidasSetIngot(w,Name,*Temp);
}
void MidasSetCallback(w,Name,Command)
Widget w;
char *Name;
char *Command;
{
   Ingot *T = MidasCreateIngot(w, Name);
   MidasInheritIngotValue(T,Name);
   MidasSetCallbackValue(T,Command);
}
void MidasGetIngots(L,w)
List *L;
Widget w;
{
    MidasShell *ms = MidasGetShell(w);

    MidasEmptyList(L);

    for (;; w = XtParent(w))
      {
        MidasWidget *mw = MidasWidgetToMW(w);
        ListItem *i;
        List *TList = mw->IngotList; 
   
        if (TList != NULL)
          for (i = TList->First; i != 0; i = i->Next)
            if (strncmp(i->Entry,"midas",5))
              if (MidasFindItemInList(L,i->Entry) == 0)
                MidasAddItemToList(L,i->Entry);

        if (w == ms->Widget) break; 
      }
}
static void MidasStoreCallback(w,cname)
Widget w;
char *cname;
{
    char *result;
    char *name = MidasGetWidgetName(w);
    char *value = MidasConvertToString(MidasGetCallback(w,cname));
    char *new = XtMalloc(strlen(name)+strlen(cname)+strlen(Option)+2);
    MidasShell *ms = MidasGetShell(w);

    XtGetApplicationResources(w,(XtPointer) &result,resources,1,NULL,0);
    strcpy(new,name);
    strcat(new,".");
    strcat(new,cname);
    XrmPutStringResource(&ms->Database,new,value);

    strcat(new,Option);
    value = MidasConvertToString(MidasGetIngotOption(w,cname));
    XrmPutStringResource(&ms->Database,new,value);

    ms->Changes++;

    XtFree(new);
    XtFree(value);

    if (result != 0)
      {
        Boolean found = FALSE;
        char *token, *old = XtNewString(result);

        for (token = strtok(old," "); token != NULL; token = strtok(NULL," "))
          if (strcmp(token,cname)==0) found = TRUE;

        XtFree(old);
        if (found) { XtFree(name); return; }
        value = XtMalloc(strlen(result)+strlen(cname)+2);
        strcpy(value,result);
        strcat(value," ");
        strcat(value,cname);
      }
    else value = XtNewString(cname);
      
    new = XtMalloc(strlen(name)+strlen(".midasIngots")+1);
    strcpy(new,name);
    strcat(new,".midasIngots");
  
    XrmPutStringResource(&ms->Database,new,value);

    XtFree(new);
    XtFree(value);
    XtFree(name);
}
void MidasSetupIngots(mw,IngotNames)
MidasWidget *mw;
char *IngotNames;
{
    List *TList;
    ListItem *item;
    char *Input = XtNewString(IngotNames);
    char *VerbVector[100];
    int VerbCount, i;
    Widget w = mw->Widget;
    MidasShell *ms = MidasGetShell(w);

    mw->IngotList = TList = MidasCreateEmptyList(XtName(w));

    MidasTokenizeCommand(Input,VerbVector,&VerbCount,XtNumber(VerbVector)," ");
/*
    This could be made more efficient by fetching all the ingots at once
*/
    for (i = 0; i < VerbCount; i++)
      {
        IngotParm result;
        Ingot *T = XtNew(Ingot);
        XtResource *temp;
        ListItem *item = MidasFindItemInList(IngotRList,VerbVector[i]);
        if (item == 0)
          {
            temp = (XtResource *) XtMalloc(sizeof(XtResource)*2);
            temp[0] = resources[0];
            temp[1] = resources[1]; 
            temp[0].resource_name = XtNewString(VerbVector[i]);
            temp[1].resource_name = strcpy(XtMalloc(strlen(VerbVector[i])+strlen(Option)+1),VerbVector[i]);
            strcat(temp[1].resource_name,Option);

            item = MidasAddItemToList(IngotRList,temp->resource_name);
            item->Pointer = temp;
          }
        else temp = item->Pointer;
     
        XtGetApplicationResources(w,(XtPointer) &result,temp,XtNumber(resources),
                                  NULL,0);

        if (result.Command == 0 || *result.Command == '\0') T->Command = 0;
        else T->Command = XtNewString(result.Command);
        T->Valued = 0;
        T->Parent = w;
        T->ValueOwner = 0;
        T->CallbackList = 0;
        T->ExecuteWhenCreated = result.ExecuteWhenCreated;

        item = MidasAddItemToList(TList,VerbVector[i]);
        item->Pointer = T;
      }
    XtFree(Input);

    for (item = TList->First; item != 0; item = item->Next)
      {
        Widget SW;
        Ingot *T = item->Pointer;

        if (T->Valued == 0) MidasInheritIngotValue(T,item->Entry);
        if (T->Command != 0) 
          {
            if (T->Valued == 0)
              {
                ListItem *item;
                Ingot *ValueOwner = T->ValueOwner;
                if (ValueOwner->CallbackList == 0) ValueOwner->CallbackList = MidasCreateEmptyList("");
                item = MidasAddItemToListPos(ValueOwner->CallbackList,XtName(w),0);
                item->Pointer = T;
              } 
            if (T->ExecuteWhenCreated) MidasQueueCommand(w,T->Command);
          }
      }
}
void MidasInitializeIngots()
{
    IngotRList = MidasCreateEmptyList("IngotResourceList");
    MidasDeclareFunction("GETINGOT(Widget,name)"           ,MidasGetIngot);
    MidasDeclareFunction("GETCALLBACK(Widget,name)"        ,MidasGetCallback);
    MidasDeclareFunction("GETVALUEOWNER(Widget,name)"      ,MidasGetValueOwner);
    MidasDeclareFunction("GETINGOTOPTION(Widget,name)"     ,MidasGetIngotOption);
    MidasDeclareVerb("SET INGOT Widget *name any..."       ,MidasSetIngotP);   
    MidasDeclareVerb("GET INGOTS list Widget"              ,MidasGetIngots);
    MidasDeclareVerb("SET CALLBACK Widget name name...",    MidasSetCallback);
    MidasDeclareVerb("SET INGOT OPTION Widget name Boolean",MidasSetIngotOption);
    MidasDeclareVerb("STORE CALLBACK Widget name",          MidasStoreCallback);
}
