#ifndef MIDASLIST
#define MIDASLIST
#include "midasoperand.h"

struct ListItem  {
    struct ListItem     *Next;
    struct ListItem     *Prev;
    char                *Entry;
    void                *Pointer;
};

typedef struct ListItem  ListItem ;

struct List {
    struct ListItem     *First;
    struct ListItem     *Last;
    Widget               W;
    struct ListItem     *Selected;
    char                *Name;
};

typedef struct List List;

static List NullList={0,0,0,0,0}; 

typedef XtPointer (*MidasCopyListRtn)();

#ifdef _NO_WAY

List *MidasCreateEmptyList(char *name);
Boolean MidasConvertStringList(MidasOperand *In, MidasOperand *Out);
void MidasEmptyList(List *L);
void MidasDestroyList(List *L);
ListItem *MidasFindItemInList(List *L, char     *Entry);
int MidasFindItemPosInList(List *L, ListItem *Item);
ListItem *MidasFindItemInListPos(List *L,int Pos);
ListItem *MidasAddItemToListPos(List *L,char *Entry,int position);
ListItem *MidasAddItemToList(List *L, char *Entry);
void *MidasCopyList(List *Old, List *New, MidasCopyListRtn CopyRtn);
void MidasRealizeList(List *L, Widget W);
void MidasUnrealizeList(List *L);
void MidasSelectItemPosInList(List *L, int Pos, Boolean Notify);

#else

List *MidasCreateEmptyList();
Boolean MidasConvertStringList();
void MidasEmptyList();
void MidasDestroyList();
ListItem *MidasFindItemInList();
int MidasFindItemPosInList();
ListItem *MidasFindItemInListPos();
ListItem *MidasAddItemToListPos();
ListItem *MidasAddItemToList();
void *MidasCopyList();
void MidasRealizeList();
void MidasUnrealizeList();
void MidasSelectItemPosInList();

#endif 

#endif 
