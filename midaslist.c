#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include "midaslist.h"
#include "midasoperand.h"

XmString MidasCharToString();

static List MasterList = {0,0,0,0,0};

Boolean MidasConvertStringList(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
    ListItem *item = MidasFindItemInList(&MasterList,In->Value.P);

    if (item == 0) Out->Value.P = (void *) MidasCreateEmptyList(In->Value.P);
    else           Out->Value.P = item->Pointer;

    return TRUE;
}    
List *MidasCreateEmptyList(name)
    char *name;
{
    ListItem *item;
    List *list = XtNew(List);
    *list = NullList;
    item = MidasAddItemToList(&MasterList,name);
    item->Pointer = list;
    list->Name = item->Entry;
    return list;
}
void MidasEmptyList(L)
    List *L;
{
    ListItem *i, *next;
    for (i = L->First; i != 0; i = next)
      {
        next = i->Next;
        XtFree(i->Entry);
        XtFree(i->Pointer);
        XtFree((char *)i);
      }

    L->First = 0;
    L->Last  = 0;
    L->Selected = 0;
    if (L->W != 0) XmListDeleteAllItems(L->W);
}
ListItem *MidasFindItemInList(L,Entry)
      List     *L;
      char     *Entry;
{
    ListItem *i;

    for (i = L->First; i != 0; i = i->Next)
        if (strcmp(i->Entry,Entry) == 0) return i; 

    return 0;
}
int MidasFindItemPosInList(L,Item)
      List     *L;
      ListItem *Item;
{
    ListItem *i;
    int p = 1;

    for (i = L->First; i != 0; i = i->Next, p++)
        if (i == Item) return p; 

    return 0;
}
ListItem *MidasFindItemInListPos(L,Pos)
      List     *L;
      int       Pos;
{
    ListItem *i;

    if (Pos == 0) return L->Last;
    for (i = L->First; i != 0; i = i->Next) if (--Pos == 0) return i; 
    return 0;
}
ListItem *MidasAddItemToListPos(L,Entry,position)
      List     *L;
      char     *Entry;
      int       position;
{
    XmString string;
    ListItem *i;
    ListItem *new = XtNew(ListItem);

    new->Entry = XtNewString(Entry);
    new->Pointer = 0; 

    if (L->W != 0) 
      {
        string = MidasCharToString(Entry);
        XmListAddItem(L->W,string,position);
        XmStringFree(string);
      }
    if (position == 0) i = 0;
    else for (i = L->First; --position; i = i->Next);

    if (L->First == 0)
      {
        new->Next = 0;
        new->Prev = 0;
        L->Last = new;
        L->First = new;
      }
    else if (i == 0 ) 
      { 
        new->Next = 0;
        L->Last->Next = new;
        new->Prev = L->Last;
        L->Last = new;
      }
    else if (i == L->First)
      {
        new->Prev = 0;
        L->First->Prev = new;      
        new->Next = L->First;
        L->First = new;
      }
    else
      {
        new->Next = i;
        new->Prev = i->Prev;
        i->Prev->Next = new;
        i->Prev = new;
      }

    return new;
}
ListItem *MidasAddItemToList(L,Entry)
      List     *L;
      char     *Entry;
{
    XmString string;
    int position=1;
    ListItem *i;

    for (i = L->First; i != 0; i = i->Next, position++)
      {
        int compare = strcmp(i->Entry,Entry);
        if (compare >= 0) break; 
      }
    if (i==0) position=0;
    return MidasAddItemToListPos(L,Entry,position);
}
void *MidasCopyList(Old,New,CopyRtn)
    List *Old;
    List *New;
    XtPointer (*CopyRtn)();
{
    ListItem *iold, *inew;

    for (iold = Old->First; iold != 0; iold = iold->Next)
      {
        inew = MidasAddItemToListPos(New,iold->Entry,0);
        inew->Pointer = CopyRtn(iold->Pointer);
      }
}
void MidasRealizeList(L,W)
      List     *L;
      Widget    W;
{
    XmString list[4000];
    ListItem *i;
    int list_size=0;

    for (i = L->First;  i != 0; i = i->Next) 
      list[list_size++] = MidasCharToString(i->Entry); 

    {
      int n=0;
      Arg args[10];
      XtSetArg(args[n],XmNitemCount,list_size               ); n++;
      XtSetArg(args[n],XmNselectedItemCount, (XtArgVal) 0   ); n++;
      XtSetArg(args[n],XmNselectedItems,     NULL           ); n++;
      XtSetArg(args[n],XmNitems,             (XtArgVal) list); n++;  
      XtSetValues(W,args,n);
    }
    for (; list_size > 0; ) XmStringFree(list[--list_size]);

    L->Selected = 0;
    L->W = W;
}
void MidasUnrealizeList(L)
      List     *L;
{
    if (L->W != 0)
      {
        XmListDeleteAllItems(L->W);
        L->Selected = 0;
        L->W = 0;
      }
}
void MidasRemoveItemFromList(L,item)
      List     *L;
      ListItem *item;
{
    int position=1;
    ListItem *i;

    for (i = L->First; i != 0; i = i->Next, position++)
        if (i == item) break; 

    if (i == 0) return;

    if (L->W != 0) XmListDeletePos(L->W,position);

    if (i->Next == 0) L->Last = i->Prev;
    else              i->Next->Prev = i->Prev;
    if (i->Prev == 0) L->First = i->Next;
    else              i->Prev->Next = i->Next;

    if (i->Pointer != 0) XtFree(i->Pointer);
    XtFree(i->Entry);
    XtFree((char *)i);
}
void MidasSelectItemPosInList(L,Pos,Notify)
      List     *L;
      int      Pos;
      Boolean  Notify;
{
  XmListSelectPos(L->W,Pos,Notify);
}
void MidasDestroyList(L)
    List *L;
{
    ListItem *i;

    MidasEmptyList(L);
    for (i = MasterList.First; i != 0; i = i->Next)
      if (i->Pointer == L)
        {
          MidasRemoveItemFromList(&MasterList,i);
          return;
        }
}
static MidasOperand MidasFindNameInListPos(L,pos)
List *L;
int pos;
{
    MidasOperand Temp;
    ListItem *item = MidasFindItemInListPos(L,pos);

    if   (item != NULL) Temp.Value.P = item->Entry; 
    else                Temp.Value.P = "";
    Temp.Dynamic = FALSE;
    Temp.Type = MString;
    return Temp;  
}
static MidasOperand MidasCreateList(name)
char *name;
{
    MidasOperand Temp;

    Temp.Value.P = (XtPointer) MidasCreateEmptyList(name);
    Temp.Dynamic = False; /* A lie...but what is one to do */
    Temp.Type = "list";
    return Temp;
}
MidasListInit()
{

    MidasDeclareVerb("ADD LIST POSITION list name Int"      ,MidasAddItemToListPos);
    MidasDeclareVerb("ADD LIST list name"                   ,MidasAddItemToList);
    MidasDeclareVerb("REALIZE LIST list Widget"             ,MidasRealizeList);
    MidasDeclareVerb("UNREALIZE LIST list"                  ,MidasUnrealizeList);
    MidasDeclareVerb("EMPTY LIST list"                      ,MidasEmptyList);

    MidasDeclareFunction("LISTNAMEPOS(list,Int)"            ,MidasFindNameInListPos);
    MidasDeclareFunction("CREATELIST(name)"                 ,MidasCreateList);
}    
