#include <stdio.h>                              /* For printf and so on. */
#include <string.h>
#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include <Xm/Text.h>
#include <Xm/ToggleBG.h>
#include <Xm/PushBG.h>
#include <Xm/CascadeBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/RowColumn.h>
#include "midaslist.h"
#include "midasoperand.h"
#include "midasshell.h"

XmString MidasCharToString();
MidasShell *MidasGetShell();
Widget MidasFetch();

#ifndef VAX
WidgetList DXmChildren();
int DXmNumChildren();
#endif

MidasOperand MidasGetAppResource();

struct VerbInMenu {
    int     Verb;
    int     Menu;
};

typedef struct VerbInMenu VerbInMenu;

struct VerbStruct {
    char *Command;
    Boolean IsToggle;
};
    
typedef struct VerbStruct VerbStruct; 

struct CustomHeader {
    int    Modified;
    int    Changed;
    Widget Fetched;
    Widget Parent;

    Widget CustomCommandText;
    Widget CustomVerbAddButton;
    Widget CustomVerbDeleteButton;
    Widget CustomVerbList;
    Widget CustomVerbText;
    Widget CustomVerbIsToggle;
    Widget CustomMenuAddButton;
    Widget CustomMenuDeleteButton;
    Widget CustomMenuList;
    Widget CustomMenuText;
    Widget CustomVMUpButton;
    Widget CustomVMDownButton;
    Widget CustomVMAddVerbButton;
    Widget CustomVMAddMenuButton;
    Widget CustomVMSeparatorButton;
    Widget CustomVMRemoveButton;
    Widget CustomVMList;
    Widget CustomVMActiveLabel;
    Widget CustomOKButton;
    Widget CustomApplyButton;
    Widget CustomCancelButton;
    Widget CustomResetButton;
    
    List TempVerbList;
    List TempMenuList;
    List VerbList;
    List MenuList;
    List *VMList;
    List ConnectList;
};

typedef struct CustomHeader CustomHeader;

struct Link  {
    Widget        Widget;
    CustomHeader *Header;
};

typedef struct Link Link;

static List CustomHeaders; 
static int  ExitResponse; 

static void CustomSetActiveMenu(Header,Label)
      CustomHeader *Header;
      XmString Label;
{
    int n = 0;
    Arg args[10];
    XtSetArg(args[n],XmNlabelString,Label); n++;
    XtSetValues(Header->CustomVMActiveLabel,args,n);
}
static char *CustomAppendArrow(name)
    char *name;
{
    char *result;
    result = strcpy(XtMalloc(strlen(name)+4),name);
    return strcat(result," =>");
}
static Boolean CustomIsChild(Header,parent,child)
      CustomHeader *Header;
      List         *parent;
      List         *child;

{
    ListItem *i;

    for (i = parent->First; i != 0; i = i->Next)
      {
        VerbInMenu *vm = ((VerbInMenu *) i->Pointer);
        if (vm->Menu != 0)
          { 
            ListItem *menu = MidasFindItemInListPos(&Header->TempMenuList,vm->Menu);
            List *NewList = ((List *) menu->Pointer);
            if (NewList = child) return 1;
            else if (CustomIsChild(Header,NewList,child)) return 1;
          }
      }
    return 0;
}             
static void CustomUpdatePositionsAdd(L,pos,verb)
    List         *L;
    int           pos;
    int           verb;
{
    ListItem *i , *j;

    for (i = L->First; i != 0; i = i->Next)
      {
        List *menu = ((List *) i->Pointer); 
        if (menu != 0)
          for (j = menu->First; j != 0; j = j->Next)
            {
              VerbInMenu *vm = j->Pointer;
              if (verb)
                {
                  if (vm->Verb >= pos) vm->Verb += 1;
                }
              else
                {
                  if (vm->Menu >= pos) vm->Menu += 1;
                }
            }
        }
};
static void CustomUpdatePositionsRemove(L,pos,verb)
    List         *L;
    int           pos;
    int           verb;
{
    ListItem *i, *j, *next;

    for (i = L->First; i != 0; i = i->Next)
      {
        List *menu = ((List *) i->Pointer); 
        for (j = menu->First; j != 0; j = next)
          {
            VerbInMenu *vm = j->Pointer;
            next = j->Next;
            if (verb)
              {
                if      (vm->Verb == pos) MidasRemoveItemFromList(menu,j);
                else if (vm->Verb >  pos) vm->Verb -= 1;
              }
            else
              {
                if      (vm->Menu == pos) MidasRemoveItemFromList(menu,j);
                else if (vm->Menu >  pos) vm->Menu -= 1;
              }
          }
      }
};
static void CustomUpdateSensitivity(Header) 
                                      /*----Figures out which keys should be 
                                            sensitive at the current time    */
    CustomHeader *Header;
{

    char *menu;
    char *verb;
    char *command;
    Boolean sensitive;
   
    XtSetSensitive(Header->CustomApplyButton,Header->Changed != 0);
    XtSetSensitive(Header->CustomResetButton,Header->Changed != 0);

    XtSetSensitive(Header->CustomVerbDeleteButton,
                   Header->TempVerbList.Selected != 0);
    XtSetSensitive(Header->CustomMenuDeleteButton,
                   Header->TempMenuList.Selected != 0);

    verb    = XmTextGetString(Header->CustomVerbText);
    command = XmTextGetString(Header->CustomCommandText);
    sensitive = (strcmp(verb,"") != 0 && strcmp(command,"") != 0 );
    XtSetSensitive(Header->CustomVerbAddButton,sensitive);
    XtFree(command);
    XtFree(verb);

    menu = XmTextGetString(Header->CustomMenuText);
    sensitive = (strcmp(menu,"")!=0);
    XtSetSensitive(Header->CustomMenuAddButton,sensitive);
    XtFree(menu);

    if (Header->VMList == 0)
      {
        XtSetSensitive(Header->CustomVMAddMenuButton,0);
        XtSetSensitive(Header->CustomVMAddVerbButton,0);
        XtSetSensitive(Header->CustomVMSeparatorButton,0);
        XtSetSensitive(Header->CustomVMRemoveButton,0);
        XtSetSensitive(Header->CustomVMUpButton,0);
        XtSetSensitive(Header->CustomVMDownButton,0);
      }
    else                
      {
        ListItem *item = Header->VMList->Selected;
        XtSetSensitive(Header->CustomVMSeparatorButton,1);
        XtSetSensitive(Header->CustomVMRemoveButton,(item!=0));
        XtSetSensitive(Header->CustomVMUpButton,
                       (item!=0 && item!=Header->VMList->First));
        XtSetSensitive(Header->CustomVMDownButton,
                       (item!=0 && item!=Header->VMList->Last));
        sensitive = (Header->TempVerbList.Selected!=0);
        if (sensitive)
          sensitive=(MidasFindItemInList(Header->VMList,
                     Header->TempVerbList.Selected->Entry)==0);
        XtSetSensitive(Header->CustomVMAddVerbButton,sensitive);
        sensitive = (Header->TempMenuList.Selected!=0);
        if (sensitive)
          {
            char *entry = CustomAppendArrow(Header->TempMenuList.Selected->Entry);
            sensitive=(MidasFindItemInList(Header->VMList,entry)==0);
            XtFree(entry);
          }
        if (sensitive)
          {
            List *parent = ((List *) Header->TempMenuList.Selected->Pointer);
            if (parent==0 || parent == Header->VMList) sensitive = 0;
            else sensitive = (CustomIsChild(Header,parent,Header->VMList)==0);
          }
        XtSetSensitive(Header->CustomVMAddMenuButton,sensitive);
      }
}
static CustomHeader *CustomFindHeader(w)
      Widget w;
{
      Widget main;
      CustomHeader *header;
      int n = 0;
      Arg args[10];
      XtSetArg(args[n],XmNuserData , &header); n++;

      for (main=w;; main=XtParent(main)) 
        {
          XtGetValues(main,args,n);
          if (header!=0 && header != (CustomHeader *) -1) return header;
        }
}
static void CustomUpdateSensitivityCallback(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    CustomHeader *Header = CustomFindHeader(w);
    CustomUpdateSensitivity(Header);
}
static void CustomAddVerb(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    int pos;
    char *verb;
    char *command;
    ListItem *item;
    CustomHeader *Header = CustomFindHeader(w);
    Boolean istoggle;
    VerbStruct *Verb;  


    verb = XmTextGetString(Header->CustomVerbText);
    command = XmTextGetString(Header->CustomCommandText);
    istoggle = XmToggleButtonGetState(Header->CustomVerbIsToggle); 

    if (item = MidasFindItemInList(&Header->TempVerbList,verb)) 
      {
        Verb = (VerbStruct *) item->Pointer;  
        XtFree(Verb->Command);
        XtFree((char *) Verb); 
      }
    else
      {
        item = MidasAddItemToList(&Header->TempVerbList,verb);
        pos = MidasFindItemPosInList(&Header->TempVerbList,item);
        MidasSelectItemPosInList(&Header->TempVerbList,pos,FALSE);
        CustomUpdatePositionsAdd(&Header->TempMenuList,pos,1);
      }
    XtFree(verb);
 
    Verb = XtNew(VerbStruct);
    Verb->Command = command;
    Verb->IsToggle = istoggle;

    Header->TempVerbList.Selected = item;
    item->Pointer = Verb;
    Header->Changed++;
    CustomUpdateSensitivity(Header);
}
static void CustomSelVerb(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmListCallbackStruct *cbDataP;
{
    ListItem *item;
    CustomHeader *Header = CustomFindHeader(w);
    VerbStruct *Verb;
    
    item = MidasFindItemInListPos(&Header->TempVerbList,cbDataP->item_position); 
    Header->TempVerbList.Selected = item;
    XmTextSetString(Header->CustomVerbText,item->Entry);

    Verb = (VerbStruct *) item->Pointer;

    XmToggleButtonSetState(Header->CustomVerbIsToggle,Verb->IsToggle,FALSE);
    XmTextSetString(Header->CustomCommandText,Verb->Command);

    CustomUpdateSensitivity(Header);
}
static void CustomDelVerb(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    char *verb;
    CustomHeader *Header = CustomFindHeader(w);
    ListItem  *item = Header->TempVerbList.Selected; 
    VerbStruct *Verb = (VerbStruct *) item->Pointer;

    int pos = MidasFindItemPosInList(&Header->TempVerbList,item);
                           
    XtFree(Verb->Command);
  
    CustomUpdatePositionsRemove(&Header->TempMenuList,pos,1);
    MidasRemoveItemFromList(&Header->TempVerbList,item);
                            
    XmTextSetString(Header->CustomVerbText,"");
    XmTextSetString(Header->CustomCommandText,"");
    XmToggleButtonSetState(Header->CustomVerbIsToggle,FALSE,FALSE);
    Header->TempVerbList.Selected = 0;
    Header->Changed++;
    CustomUpdateSensitivity(Header);
}
static void CustomAddMenu(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    int pos;
    char *menu;
    ListItem *item;
    CustomHeader *Header = CustomFindHeader(w);

    menu = XmTextGetString(Header->CustomMenuText);
    XmTextSetString(Header->CustomMenuText,"");

    item = MidasAddItemToList(&Header->TempMenuList,menu);
    pos = MidasFindItemPosInList(&Header->TempMenuList,item);
    MidasSelectItemPosInList(&Header->TempMenuList,pos,FALSE);
    CustomUpdatePositionsAdd(&Header->TempMenuList,pos,0);

    item->Pointer = MidasCreateEmptyList(menu);
    Header->TempMenuList.Selected = item;
    Header->Changed++;
    CustomUpdateSensitivity(Header);
    XtFree(menu);
}
static void CustomSelConfMenu(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    char     label[100];
    XmString string;
    CustomHeader *Header = CustomFindHeader(w);

    strcpy(label,"Menu: "); 
    if (Header->VMList!=0) MidasUnrealizeList(Header->VMList);  
    Header->VMList = ((List *) Header->TempMenuList.Selected->Pointer);
    MidasRealizeList(Header->VMList,Header->CustomVMList);
    string = MidasCharToString(strcat(label,Header->TempMenuList.Selected->Entry));
    CustomSetActiveMenu(Header,string);
    XmStringFree(string);
    CustomUpdateSensitivity(Header);
}
static void CustomSelMenu(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmListCallbackStruct *cbDataP;
{
    ListItem *item;
    CustomHeader *Header = CustomFindHeader(w);
    item = MidasFindItemInListPos(&Header->TempMenuList,cbDataP->item_position); 
    Header->TempMenuList.Selected = item;
    CustomUpdateSensitivity(Header);
}
static void CustomDelMenu(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    char *menu;
    CustomHeader *Header = CustomFindHeader(w);
    int pos = MidasFindItemPosInList(&Header->TempMenuList,
                                      Header->TempMenuList.Selected); 
                            
    CustomUpdatePositionsRemove(&Header->TempMenuList,pos,0);

    if (((List *) Header->TempMenuList.Selected->Pointer) == Header->VMList) 
      {
        XmString string;
        MidasUnrealizeList(Header->VMList);  
        Header->VMList=0;
        string = MidasCharToString("No Menu Selected");
        CustomSetActiveMenu(Header,string);
        XmStringFree(string);
      }
    MidasDestroyList(Header->TempMenuList.Selected->Pointer);
    Header->TempMenuList.Selected->Pointer = 0;
    MidasRemoveItemFromList(&Header->TempMenuList,Header->TempMenuList.Selected); 
    Header->TempMenuList.Selected = 0;
    Header->Changed++;
    CustomUpdateSensitivity(Header);
}
static void CustomVMAddSeparator(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    ListItem *item; 
    VerbInMenu *vm; 
    CustomHeader *Header = CustomFindHeader(w);

    item = MidasAddItemToListPos(Header->VMList,"......",0);
    MidasSelectItemPosInList(Header->VMList,0,FALSE);

    vm = XtNew(VerbInMenu);
    vm->Menu = 0;
    vm->Verb = 0;

    item->Pointer = vm;
    Header->VMList->Selected = item;
    Header->Changed++;
    CustomUpdateSensitivity(Header);
}
static void CustomVMAddVerb(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    char *verb; 
    VerbInMenu *vm; 
    ListItem *item; 
    CustomHeader *Header = CustomFindHeader(w);

    verb = Header->TempVerbList.Selected->Entry;
    item = MidasAddItemToListPos(Header->VMList,verb,0);
    MidasSelectItemPosInList(Header->VMList,0,FALSE);

    vm = XtNew(VerbInMenu);
    vm->Menu = 0;
    vm->Verb = MidasFindItemPosInList(&Header->TempVerbList,
                                       Header->TempVerbList.Selected);
    item->Pointer = vm;

    Header->VMList->Selected = item;
    Header->Changed++;
    CustomUpdateSensitivity(Header);
}
static void CustomSelConfVerb(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    CustomHeader *Header = CustomFindHeader(w);
    if (XtIsSensitive(Header->CustomVMAddVerbButton)) 
      CustomVMAddVerb(w,closure,(XmAnyCallbackStruct *) cbDataP);
}
static void CustomVMAddMenu(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    char *menu, *arrow; 
    VerbInMenu *vm; 
    ListItem *item; 
    CustomHeader *Header = CustomFindHeader(w);

    menu = Header->TempMenuList.Selected->Entry;
    arrow = CustomAppendArrow(menu);
    item = MidasAddItemToListPos(Header->VMList,arrow,0);
    MidasSelectItemPosInList(Header->VMList,0,FALSE);
    XtFree(arrow);
   
    vm = XtNew(VerbInMenu);
    vm->Menu = MidasFindItemPosInList(&Header->TempMenuList,
                                       Header->TempMenuList.Selected);
    vm->Verb = 0;
   
    item->Pointer = vm;

    Header->VMList->Selected = item;
    Header->Changed++;
    CustomUpdateSensitivity(Header);
}
static void CustomVMSel(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmListCallbackStruct *cbDataP;
{
    ListItem *item;
    CustomHeader *Header = CustomFindHeader(w);

    item = MidasFindItemInListPos(Header->VMList,cbDataP->item_position);
    Header->VMList->Selected = item;
    CustomUpdateSensitivity(Header);
}
static void CustomVMSelConf(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmListCallbackStruct *cbDataP;
{
    VerbInMenu *vm;
    ListItem *item;
    CustomHeader *Header = CustomFindHeader(w);

    item = MidasFindItemInListPos(Header->VMList,cbDataP->item_position);
    vm = ((VerbInMenu *) item->Pointer);
    if      (vm->Verb != 0) 
      { 
        ListItem *verb = MidasFindItemInListPos(&Header->TempVerbList,vm->Verb);
        XmString string = MidasCharToString(verb->Entry);
        XmListSelectItem(Header->CustomVerbList,string,1);
        XmStringFree(string);
      }
    else if (vm->Menu != 0) 
      {
        ListItem *menu = MidasFindItemInListPos(&Header->TempMenuList,vm->Menu);
        XmString string = MidasCharToString(menu->Entry);
        XmListSelectItem(Header->CustomMenuList,string,1);
        XmStringFree(string);
      }
}
static void CustomVMDel(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    char *menu;
    ListItem *item;
    CustomHeader *Header = CustomFindHeader(w);

    MidasRemoveItemFromList(Header->VMList,Header->VMList->Selected); 
    Header->VMList->Selected = 0;
    Header->Changed++;
    CustomUpdateSensitivity(Header);
}
static void CustomMoveUp(Header,item,dir)
      CustomHeader *Header;
      ListItem     *item;
      int           dir;
{
    XmString NewItems[2];

    ListItem *i;
    ListItem *swap = item->Prev;
    int position=0;

    for (i = Header->VMList->First; i != 0; i = i->Next) 
      {
        position++;
        if (i == item) break;
      }

    NewItems[0] = MidasCharToString(item->Entry); 
    NewItems[1] = MidasCharToString(swap->Entry); 
    
    XmListReplaceItemsPos(Header->VMList->W,NewItems,2,--position);

    XmStringFree(NewItems[0]);
    XmStringFree(NewItems[1]);

    if (dir) 
      {
        XmListDeselectPos(Header->VMList->W,position);
        XmListSelectPos(Header->VMList->W,position+1,0);
      }
    else 
      {
        XmListSelectPos(Header->VMList->W,position,0);
        XmListDeselectPos(Header->VMList->W,position+1);
      }

    if (Header->VMList->First == swap) Header->VMList->First = item; 
    else                               swap->Prev->Next = item;
    if (Header->VMList->Last  == item) Header->VMList->Last  = swap; 
    else                               item->Next->Prev = swap;
    item->Prev = swap->Prev;
    swap->Next = item->Next;
    item->Next = swap;
    swap->Prev = item;
    Header->Changed++;
    CustomUpdateSensitivity(Header);
}
static void CustomVMUp(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    CustomHeader *Header = CustomFindHeader(w);
    CustomMoveUp(Header,Header->VMList->Selected,0);
}
static void CustomVMDown(w,closure,cbDataP)
      Widget w; 
      Opaque closure; 
      XmAnyCallbackStruct *cbDataP;
{
    CustomHeader *Header = CustomFindHeader(w);
    CustomMoveUp(Header,Header->VMList->Selected->Next,1);
}
static void CustomDispatchCommand(w,tag,cbDataP)
      Widget w; 
      Opaque tag; 
      XmAnyCallbackStruct *cbDataP;
{
   MidasDispatchCommandCallback(w,tag,cbDataP);
}
static void CustomCreateCallback(w,tag,cbDataP)
      Widget w; 
      int *tag; 
      XmAnyCallbackStruct *cbDataP;
{
    ListItem *item = CustomHeaders.Selected;
    CustomHeader *Header = item->Pointer;

    if      (*tag ==  1) Header->CustomCommandText       = w; 
    else if (*tag ==  2) Header->CustomVMUpButton        = w;
    else if (*tag ==  3) Header->CustomVMDownButton      = w;
    else if (*tag ==  4) Header->CustomVMAddVerbButton   = w;
    else if (*tag ==  5) Header->CustomVMAddMenuButton   = w;
    else if (*tag ==  6) Header->CustomVMSeparatorButton = w;
    else if (*tag ==  7) Header->CustomVMRemoveButton    = w;
    else if (*tag ==  8) Header->CustomVMList            = w;
    else if (*tag ==  9) Header->CustomOKButton          = w;
    else if (*tag == 10) Header->CustomApplyButton       = w;
    else if (*tag == 11) Header->CustomCancelButton      = w;
    else if (*tag == 12) Header->CustomResetButton       = w;
    else if (*tag == 13) Header->CustomVerbList          = w;
    else if (*tag == 14) Header->CustomVerbText          = w;
    else if (*tag == 15) Header->CustomVerbAddButton     = w;
    else if (*tag == 16) Header->CustomVerbDeleteButton  = w;
    else if (*tag == 17) Header->CustomMenuList          = w;
    else if (*tag == 18) Header->CustomMenuText          = w;
    else if (*tag == 19) Header->CustomMenuAddButton     = w;
    else if (*tag == 20) Header->CustomMenuDeleteButton  = w;
    else if (*tag == 21) Header->CustomVMActiveLabel     = w;
    else if (*tag == 22) Header->CustomVerbIsToggle      = w;
}                      
static void CustomDestroyMenu(w)
      Widget w;
{
      Widget *list = DXmChildren(w);
      int n = DXmNumChildren(w);

      for (; n > 0; n--) 
        { 
          XtUnmanageChild(*list);
          XtDestroyWidget(*list);
          list++;
        }
}
static char *strtrn(in,from,to)
char *in; 
char from; 
char to;
{
      char *p = in;
      for (; *p != 0; p++) if (*p == from) *p = to;
      return in;
}
static void CustomBuildMenu(Header,w,menu)
      CustomHeader *Header;
      Widget w; 
      List   *menu;
{
      Widget child[4000];
      ListItem *j;
      int m=0;

      for (j = menu->First; j != 0; j = j->Next)
        {
          VerbInMenu *vm = ((VerbInMenu *) j->Pointer);
          if (vm->Verb != 0) 
            {
              ListItem *verb = MidasFindItemInListPos(&Header->VerbList,vm->Verb);
              XmString verbname = MidasCharToString(verb->Entry);
              VerbStruct *Verb = (VerbStruct *) verb->Pointer;

              Arg args[10];
              int n = 0;          
              char *name = strtrn(XtNewString(verb->Entry),' ','_');              

              strtrn(name,'.','_');          

              if (Verb->IsToggle)
                {
                  XtSetArg(args[n],XmNlabelString, verbname); n++;
                  child[m] = XmCreateToggleButtonGadget(w,name,args,n);
                  MidasSetupWidget(child[m]);
                  XtAddCallback(child[m++],XmNvalueChangedCallback,
                                CustomDispatchCommand,Verb->Command);
                 }
              else
                {
                  XtSetArg(args[n],XmNlabelString, verbname); n++;
                  child[m] = XmCreatePushButtonGadget(w,name,args,n);
                  MidasSetupWidget(child[m]);
                  XtAddCallback(child[m++],XmNactivateCallback,
                                CustomDispatchCommand,Verb->Command);
                } 
              XtFree(name);
              XmStringFree(verbname);
            }
          else if (vm->Menu != 0) 
            {
              ListItem *menu = MidasFindItemInListPos(&Header->MenuList,vm->Menu);
              XmString menuname = MidasCharToString(menu->Entry);
              char *name = strtrn(XtNewString(menu->Entry),' ','_');
              Widget submenu = XmCreatePulldownMenu(w,name,NULL,0);
              int n = 0;
              Arg args[10];
              XtSetArg(args[n],XmNlabelString , menuname ); n++;
              XtSetArg(args[n],XmNsubMenuId   , submenu  ); n++;
              child[m] = XmCreateCascadeButtonGadget(w,name,args,n);
/*
 *  If the last item in a menu is a HELP menu, force it to be right aligned
 */

              if (j->Next == NULL && strcmp(name,"Help")==0)
                {
                  n = 0;
                  XtSetArg(args[n],XmNmenuHelpWidget,child[m]); n++;
                  XtSetValues(w,args,n);
                }

              MidasSetupWidget(submenu);
              MidasSetupWidget(child[m++]);
              XtFree(name);
              XmStringFree(menuname);

              CustomBuildMenu(Header,submenu,menu->Pointer); 
            }
          else 
            {
              child[m++] = XmCreateSeparatorGadget(w," ",NULL,0);
            }
        }
      XtManageChildren(child,m);
}
static void CustomUpdateMenus(Header)
    CustomHeader *Header;
{
    ListItem *connect = Header->ConnectList.First;
    for (; connect != 0 ; connect = connect->Next )
      {
        Link *link = connect->Pointer;
        ListItem *menu = MidasFindItemInList(&Header->MenuList,connect->Entry);
        CustomDestroyMenu(link->Widget);
        CustomBuildMenu(Header,link->Widget,menu->Pointer);
      }
}
static VerbStruct *CustomCopyVerbListElement(Old)
    VerbStruct *Old;
{ 
    VerbStruct *New = XtNew(VerbStruct);

    New->IsToggle = Old->IsToggle;
    New->Command = XtNewString(Old->Command);  

    return New;
}
static VerbInMenu *CustomCopyVMListElement(vm)
    VerbInMenu *vm;
{
    return (VerbInMenu *) memcpy(XtMalloc(sizeof(VerbInMenu)),vm,sizeof(VerbInMenu));
}
static List *CustomCopyMenuListElement(L)
    List *L;
{
    List *New = MidasCreateEmptyList("");
    MidasCopyList(L,New,(MidasCopyListRtn)CustomCopyVMListElement);
    return New;
}
static void CustomCopyVerbList(Old,New)
    List *Old;
    List *New;
{
    MidasCopyList(Old,New,(MidasCopyListRtn)CustomCopyVerbListElement);
}
static void CustomCopyMenuList(Old,New)
    List *Old;
    List *New;
{
    MidasCopyList(Old,New,(MidasCopyListRtn)CustomCopyMenuListElement);
}
static void CustomCleanup(Header)
   CustomHeader *Header;
{
    XmString string;
    if (Header->VMList != 0) MidasUnrealizeList(Header->VMList);  
    MidasUnrealizeList(&Header->TempVerbList);
    MidasUnrealizeList(&Header->TempMenuList);
    Header->VMList=0;
    string = MidasCharToString("No Menu Selected");
    CustomSetActiveMenu(Header,string);
    XmStringFree(string);
    XmTextSetString(Header->CustomVerbText,"");
    XmTextSetString(Header->CustomMenuText,"");
    XmTextSetString(Header->CustomCommandText,"");
}
static void CustomApply(w,tag,cbDataP)
      Widget w; 
      int    *tag; 
      XmSelectionBoxCallbackStruct *cbDataP;
{
    ListItem *i;
    CustomHeader *Header = CustomFindHeader(w);
    Header->Changed = 0;
    Header->Modified++;

    for (i = Header->MenuList.First ; i != 0 ; i = i->Next )
      {
        MidasDestroyList(i->Pointer);
        i->Pointer = 0;
      }
    for (i = Header->VerbList.First ; i != 0 ; i = i->Next )
      {
        VerbStruct *Verb = (VerbStruct *) i->Pointer;
        XtFree(Verb->Command);
        XtFree((char *) Verb);
        i->Pointer = 0;
      }
    MidasEmptyList(&Header->VerbList);
    MidasEmptyList(&Header->MenuList);

    Header->VerbList = NullList;
    Header->MenuList = NullList;

    CustomCopyVerbList(&Header->TempVerbList,&Header->VerbList);
    CustomCopyMenuList(&Header->TempMenuList,&Header->MenuList);

    CustomUpdateMenus(Header);

    CustomUpdateSensitivity(Header);
}
static void CustomReset(w,tag,cbDataP)
      Widget w; 
      int    *tag; 
      XmSelectionBoxCallbackStruct *cbDataP;
{
    CustomHeader *Header = CustomFindHeader(w);
    ListItem *i;

    CustomCleanup(Header);

    for (i = Header->TempMenuList.First ; i != 0 ; i = i->Next )
      {
        MidasDestroyList(i->Pointer);
        i->Pointer = 0;
      }
    for (i = Header->TempVerbList.First ; i != 0 ; i = i->Next )
      {
        VerbStruct *Verb = (VerbStruct *) i->Pointer;
        XtFree(Verb->Command);
        XtFree((char *) Verb);
        i->Pointer = 0;
      }

    MidasEmptyList(&Header->TempVerbList);
    MidasEmptyList(&Header->TempMenuList);

    Header->TempVerbList = NullList;
    Header->TempMenuList = NullList;

    CustomCopyVerbList(&Header->VerbList,&Header->TempVerbList);
    CustomCopyMenuList(&Header->MenuList,&Header->TempMenuList);

    MidasRealizeList(&Header->TempVerbList,Header->CustomVerbList);
    MidasRealizeList(&Header->TempMenuList,Header->CustomMenuList);
    Header->Changed = 0;
    CustomUpdateSensitivity(Header);
}
static void CustomCancel(w,tag,cbDataP)
      Widget w; 
      int    *tag; 
      XmSelectionBoxCallbackStruct *cbDataP;
{
    ListItem *i;
    CustomHeader *Header = CustomFindHeader(w);

    XtUnmanageChild(Header->Fetched);
    CustomCleanup(Header);

    for (i = Header->TempMenuList.First ; i != 0 ; i = i->Next )
      {
        MidasDestroyList(i->Pointer);
        i->Pointer = 0;
      }
    for (i = Header->TempVerbList.First ; i != 0 ; i = i->Next )
      {
        VerbStruct *Verb = (VerbStruct *) i->Pointer;
        XtFree(Verb->Command);
        XtFree((char *) Verb);
        i->Pointer = 0;
      }
    MidasEmptyList(&Header->TempVerbList);
    MidasEmptyList(&Header->TempMenuList);

    Header->TempVerbList = NullList;
    Header->TempMenuList = NullList;

}
static void CustomOk(w,tag,cbDataP)
      Widget w; 
      int    *tag; 
      XmSelectionBoxCallbackStruct *cbDataP;
{
    CustomHeader *Header = CustomFindHeader(w);
    ListItem *i;

    CustomCleanup(Header);

    if (Header->Changed != 0)
      {  
        for (i = Header->MenuList.First ; i != 0 ; i = i->Next )
          {
            MidasDestroyList(i->Pointer);
            i->Pointer = 0;
          }
        for (i = Header->VerbList.First ; i != 0 ; i = i->Next )
          {
            VerbStruct *Verb = (VerbStruct *) i->Pointer;
            XtFree(Verb->Command);
            XtFree((char *) Verb);
            i->Pointer = 0;
          }
        MidasEmptyList(&Header->VerbList);
        MidasEmptyList(&Header->MenuList);

        Header->VerbList = Header->TempVerbList;
        Header->MenuList = Header->TempMenuList;

        CustomUpdateMenus(Header);
        Header->Modified++;
      }
    else 
      {
        for (i = Header->TempMenuList.First ; i != 0 ; i = i->Next )
          {
            MidasDestroyList(i->Pointer);
            i->Pointer = 0;
          }
        for (i = Header->TempVerbList.First ; i != 0 ; i = i->Next )
          {
            VerbStruct *Verb = (VerbStruct *) i->Pointer;
            XtFree(Verb->Command);
            XtFree((char *) Verb);
            i->Pointer = 0;
          }
        MidasEmptyList(&Header->TempVerbList);
        MidasEmptyList(&Header->TempMenuList);
      }
    Header->TempVerbList = NullList;
    Header->TempVerbList = NullList;

    XtUnmanageChild(Header->Fetched);
}
static void CustomWrite(Header,Name)
    CustomHeader *Header;
    char *Name;
{
    ListItem *i,*j;
    List *menu;
    char list[4096];
    char value[4096];
    char specifier[80];
    char *l = list;
    MidasShell *ms = MidasGetShell(Header->Parent);
    XrmDatabase *database = &ms->Database;
    int n= 0 ;

    for (i = Header->VerbList.First; i != 0; i = i->Next)
      {
        VerbStruct *Verb = (VerbStruct *) i->Pointer; 
        strcpy(l,i->Entry);

        l += strlen(i->Entry);
        *l++ = '#';

        sprintf(specifier,"CUSTOM.%s.Verb.%d",Name,++n);

        if (Verb->IsToggle) sprintf(value,"#%s",Verb->Command);
        else                sprintf(value,"%s" ,Verb->Command);

        XrmPutStringResource(database,specifier,value);
      }

    *(--l) = '\0';
    sprintf(specifier,"CUSTOM.%s.Verbs",Name);
    XrmPutStringResource(database,specifier,list);

    l = list;
    n = 0; 
    
    for (i = Header->MenuList.First; i != 0; i = i->Next)
      {
        char *v = value;

        menu = i->Pointer;

        strcpy(l,i->Entry);
        l += strlen(i->Entry);
        *l++ = '#';

        for (j = menu->First; j != 0; j = j->Next)
          {
            VerbInMenu *vm = ((VerbInMenu *) j->Pointer);
            int pos = 0;

            if      (vm->Verb != 0) pos = vm->Verb;
            else if (vm->Menu != 0) pos = -vm->Menu;

            sprintf(v,"%s\\%d#",j->Entry,pos);
            v += strlen(v);
          }
        *(--v) = '\0';
        sprintf(specifier,"CUSTOM.%s.Menu.%d",Name,++n);
        XrmPutStringResource(database,specifier,value);
      }

    *(--l) = '\0';
    sprintf(specifier,"CUSTOM.%s.Menus",Name);
    XrmPutStringResource(database,specifier,list);

    ms->Changes++;
    Header->Modified = 0;
}
static void CustomReadOld(Header,Name,UserDefaults)
    CustomHeader *Header;
    char         *Name;
    int          UserDefaults;
{
    char buffer[4000] , name[80];
    FILE *file;

    if (UserDefaults)
      {
#ifdef VAX
        sprintf(name,"DECW$USER_DEFAULTS:MIDAS_%s.DAT",Name);
#else
        sprintf(name,".Midas_%src",Name);
#endif
        file = fopen(name,"r");
      }
    if (file == 0)
      {
        sprintf(name,"DECW$SYSTEM_DEFAULTS:MIDAS_%s.DAT",Name);
        file = fopen(name,"r");
      }
    if (file == 0)
       file = fopen("DECW$SYSTEM_DEFAULTS:MIDAS_CUSTOM.DAT","r");

    if (file == 0) MidasError("Could not open customization file for %s",Name);
    
    for (;;) 
      {
        int c = fgetc(file);
        if (c == EOF) break;
        if (c == '$')
          {
            int i,j,l;
            char type[6],*p;

            p = type;
            *p++ = fgetc(file);
            *p++ = fgetc(file);
            *p++ = fgetc(file);
            *p++ = fgetc(file);
            *p++ = fgetc(file);
            *p = '\0';

            for (l=0;;l++)
              {
                 c = fgetc(file);
                 buffer[l] = c;

                 if (c == '{') j=l;
                 if (c == '}' | c == EOF) break;
              }

            i = 1;
            buffer[j-1] = '\0';
            j += 2;

            if (strcmp(type,"Verb:") == 0)
              {
                VerbStruct *Verb = XtNew(VerbStruct); 
                ListItem *item; 
                if (buffer[i]=='#') 
                  {
                    i++;
                    Verb->IsToggle = TRUE;
                  }
                else Verb->IsToggle = FALSE;       
                item = MidasAddItemToList(&Header->VerbList,&buffer[i]);
                buffer[l-1] = '\0';
                item->Pointer = Verb;
                Verb->Command = XtNewString(&buffer[j]);
              }         
            else if (strcmp(type,"Menu:") == 0)
              {
                int k;
                List *list;

                ListItem *item = MidasAddItemToListPos(&Header->MenuList,&buffer[i],0);

                list = XtNew(List);
                list->First    = 0;
                list->Last     = 0;
                list->Selected = 0;
                list->W        = 0;
                item->Pointer = list;
                for (;;)
                  {
                    VerbInMenu *vm;
                    int pos;

                    for (;buffer[j]==' ';j++);
                    for (k=j; buffer[k]!='\\' && buffer[k]!='}'; k++);
                    if (buffer[k]=='}') break;
                    buffer[k]='\0';
                    item = MidasAddItemToListPos(list,&buffer[j],0);
                    vm = XtNew(VerbInMenu);
                    item->Pointer = vm;
                    vm->Menu = 0; 
                    vm->Verb = 0; 
                    j = k+1;
                    for (k=j; buffer[k]!='\n'; k++);
                    buffer[k]='\0';
                    pos = atoi(&buffer[j]);
                    if      (pos > 0) vm->Verb = pos;
                    else if (pos < 0) vm->Menu = -pos;
                    j = k+1;
                  }
              }
          }
      }
    fclose(file);
}
static void CustomRead(Header,Name,UserDefaults) 
    CustomHeader *Header;
    char         *Name;
    int          UserDefaults;
{
    XrmDatabase database = XtDatabase(XtDisplay(Header->Parent));
    char specifier[80];
    char *type;
    XrmRepresentation rep;
    char *Verbs, *verb;
    char *Menus, *menu;
    XrmValue value;
    Boolean found;
    int n = 0;
    XrmHashTable root[10]; 
    XrmQuark     quarks[3]; 


/*
 * Start by reading the lists of verbs and menus
 */
    sprintf(specifier,"CUSTOM.%s.Verbs",Name);

    found = XrmGetResource(database,specifier,specifier,&type,&value);
    
    if (!found)
      {
        CustomReadOld(Header,Name,UserDefaults);
        return;
      }
    Verbs = XtNewString(value.addr);

    sprintf(specifier,"CUSTOM.%s.Menus",Name);
    found = XrmGetResource(database,specifier,specifier,&type,&value);
    Menus = XtNewString(value.addr);
    
    sprintf(specifier,"CUSTOM.%s.Verb",Name);
    XrmStringToNameList(specifier,quarks);
    
    XrmQGetSearchList(database,quarks,quarks,root,XtNumber(root)); 

    for (verb = strtok(Verbs,"#"); verb != NULL; verb = strtok(NULL,"#"))
      {
        VerbStruct *Verb = XtNew(VerbStruct); 
        ListItem *item = MidasAddItemToListPos(&Header->VerbList,verb,0); 
        char name[8];
        char *command;
        
        sprintf(name,"%d",++n);
        XrmQGetSearchResource(root,XrmStringToName(name),XrmStringToClass(name),&rep,&value);
        command = value.addr;

        if (Verb->IsToggle = (*command == '#')) command++; 

        item->Pointer = Verb;
        Verb->Command = XtNewString(command);
      } 
    
    n = 0;
    sprintf(specifier,"CUSTOM.%s.Menu",Name);
    XrmStringToNameList(specifier,quarks);
    
    XrmQGetSearchList(database,quarks,quarks,root,XtNumber(root)); 

    for (menu = strtok(Menus,"#"); menu != NULL; menu = strtok(NULL,"#"))
      {
        char *menuitems, *p;
        char name[8];
        List *list = MidasCreateEmptyList("");
        ListItem *item = MidasAddItemToListPos(&Header->MenuList,menu,0);
        item->Pointer = list;

        sprintf(name,"%d",++n);
        XrmQGetSearchResource(root,XrmStringToName(name),XrmStringToClass(name),&rep,&value);
        menuitems = XtNewString(value.addr);

        for (p = menuitems; *p != '\0'; p++)
          {
            VerbInMenu *vm = XtNew(VerbInMenu);
            int pos;
            ListItem *item;
 
            for (verb = p; *p != '\\'; p++);
            *p = '\0';

            item = MidasAddItemToListPos(list,verb,0);
            item->Pointer = vm;
 
            vm->Menu = 0; 
            vm->Verb = 0; 

            for (verb = ++p; *p != '#' && *p != '\0'; p++);

            pos = atoi(verb);
            if      (pos > 0) vm->Verb = pos;
            else if (pos < 0) vm->Menu = -pos;

            if (*p == '\0') break;
          }
        XtFree(menuitems);
      } 

    XtFree(Verbs);
    XtFree(Menus);
}
static void CustomDisconnect(w,connect,cbDataP)
      Widget w; 
      ListItem *connect; 
      XmSelectionBoxCallbackStruct *cbDataP;
{
  Link *link = connect->Pointer;
  CustomHeader *Header = link->Header;
  MidasRemoveItemFromList(&Header->ConnectList,connect);
}
void CustomWriteUser(name,entry)
      char   *name;
      char   *entry;
{
    CustomHeader *Header;
    ListItem *item = MidasFindItemInList(&CustomHeaders,name);
    char *fname;

    if (item == 0) return;

    Header = item->Pointer;
    if (entry==0) fname=name;
    else          fname=entry;

    CustomWrite(Header,fname);
}
void CustomReadUser(name,entry)
      char   *name;
      char   *entry;
{
    CustomHeader *Header;
    ListItem *item = MidasFindItemInList(&CustomHeaders,name);
    char *fname;

    if (item == 0) return;

    Header = item->Pointer;
    if (entry==0) fname=name;
    else          fname=entry;

    CustomRead(Header,fname,1);
}
void CustomReadSystem(name,entry)
      char   *name;
      char   *entry;
{
    CustomHeader *Header;
    ListItem *item = MidasFindItemInList(&CustomHeaders,name);
    char *fname;

    if (item == 0) return;

    Header = item->Pointer;
    if (entry==0) fname=name;
    else          fname=entry;

    CustomRead(Header,fname,0);
}
void CustomConnect(name,entry,w)
      char   *name;
      char   *entry;
      Widget w;
{
    CustomHeader *Header;
    ListItem *item = MidasFindItemInList(&CustomHeaders,name);
    ListItem *menu, *connect; 
    Link *link;
    int pos;

    if (item == 0) return;

    Header = item->Pointer;
    menu = MidasFindItemInList(&Header->MenuList,entry);
    if (menu==0)
      {
        menu = MidasAddItemToList(&Header->MenuList,entry);
        pos = MidasFindItemPosInList(&Header->MenuList,menu);
        CustomUpdatePositionsAdd(&Header->MenuList,pos,0);
        menu->Pointer = MidasCreateEmptyList("");
      }
    else
      {
        CustomBuildMenu(Header,w,menu->Pointer);
      }

    connect = MidasAddItemToListPos(&Header->ConnectList,entry,0);
    link = XtNew(Link);
    connect->Pointer = link;
    link->Widget = w;
    link->Header = Header;
    XtAddCallback(w,XmNdestroyCallback,CustomDisconnect,(XtPointer) connect);
}
void CustomPopup(name)
      char   *name;
{
    CustomHeader *Header;
    ListItem *item = MidasFindItemInList(&CustomHeaders,name);

    if (item == 0) return;

    Header = item->Pointer;

    if (Header->Fetched == 0)
      {
        int n = 0;
        Arg args[10];
        XtSetArg(args[n],XmNuserData , Header); n++;
        CustomHeaders.Selected = item;
        Header->Fetched = MidasFetch("Midas_Custom",Header->Parent);
        XtSetValues(Header->Fetched,args,n); 
      }
    if (XtIsManaged(Header->Fetched) == 0)
      {
        Header->TempVerbList = NullList;
        Header->TempMenuList = NullList;
        CustomCopyVerbList(&Header->VerbList,&Header->TempVerbList);
        CustomCopyMenuList(&Header->MenuList,&Header->TempMenuList);
        MidasRealizeList(&Header->TempVerbList,Header->CustomVerbList);
        MidasRealizeList(&Header->TempMenuList,Header->CustomMenuList);
        Header->Changed = 0;
        XtManageChild(Header->Fetched);
      }
    CustomUpdateSensitivity(Header);
}
void CustomCreate(name,parent)
      char   *name;
      Widget parent;
{
    CustomHeader *NewHeader;
    ListItem *NewItem;

    if (MidasFindItemInList(&CustomHeaders,name)) return;

    NewItem   =  MidasAddItemToList(&CustomHeaders,name);

    NewHeader = XtNew(CustomHeader);
    NewItem->Pointer = NewHeader;

    NewHeader->Modified     = 0;
    NewHeader->Fetched      = 0;
    NewHeader->Parent       = parent;
    NewHeader->MenuList     = NullList;
    NewHeader->VerbList     = NullList;
    NewHeader->TempMenuList = NullList;
    NewHeader->TempVerbList = NullList;
    NewHeader->VMList       = 0;
    NewHeader->ConnectList  = NullList;
    CustomRead(NewHeader,name,1);
}
int CustomExit()
{
    ListItem *i;
    int mods = 0;
    Widget w;

    for (i = CustomHeaders.First; i != 0; i = i->Next)
      {
        CustomHeader *Header = i->Pointer;
        mods += Header->Modified;
      }

    if (mods>0)
      {
        w = MidasFetch("MidasCustomExitDialog",0);
        XtManageChild(w);
        ExitResponse = -1;
        for (;ExitResponse == -1;) MidasFetchDispatchEvent();
      }
    if (ExitResponse == 2 )              /* Cancel */   
      {
        XtUnmanageChild(w);
        return 0;    
      }
    if (ExitResponse == 1 )              /* Yes    */
      for (i = CustomHeaders.First; i != 0; i = i->Next)
        {
          CustomHeader *Header = i->Pointer;
          if (Header->Modified != 0)
            CustomWrite(Header,i->Entry);
        }
    return 1;
}
static void CustomExitCallback(w,tag,cbDataP)
      Widget w; 
      int    *tag; 
      XmAnyCallbackStruct *cbDataP;
{
    ExitResponse = *tag;
}
void CustomInit()
{
    static MrmRegisterArg reglist[] = {
        {"CustomAddVerb",             (caddr_t)   CustomAddVerb},
        {"CustomDelVerb",             (caddr_t)   CustomDelVerb},
        {"CustomSelVerb",             (caddr_t)   CustomSelVerb},
        {"CustomSelConfVerb",         (caddr_t)   CustomSelConfVerb},
        {"CustomAddMenu",             (caddr_t)   CustomAddMenu},
        {"CustomDelMenu",             (caddr_t)   CustomDelMenu},
        {"CustomSelMenu",             (caddr_t)   CustomSelMenu},
        {"CustomSelConfMenu",         (caddr_t)   CustomSelConfMenu},
        {"CustomVMAddSeparator",      (caddr_t)   CustomVMAddSeparator},
        {"CustomVMAddVerb",           (caddr_t)   CustomVMAddVerb},
        {"CustomVMAddMenu",           (caddr_t)   CustomVMAddMenu},
        {"CustomVMDel",               (caddr_t)   CustomVMDel},
        {"CustomVMSel",               (caddr_t)   CustomVMSel},
        {"CustomVMSelConf",           (caddr_t)   CustomVMSelConf},
        {"CustomVMUp",                (caddr_t)   CustomVMUp},
        {"CustomVMDown",              (caddr_t)   CustomVMDown},
        {"CustomUpdateSensitivity",   (caddr_t)   CustomUpdateSensitivityCallback},
        {"CustomCreate",              (caddr_t)   CustomCreateCallback },
        {"CustomApply",               (caddr_t)   CustomApply },
        {"CustomReset",               (caddr_t)   CustomReset },
        {"CustomOk",                  (caddr_t)   CustomOk },
        {"CustomCancel",              (caddr_t)   CustomCancel },
        {"CustomExit",                (caddr_t)   CustomExitCallback }
    };
    MrmRegisterNames(reglist, XtNumber (reglist));

    CustomHeaders = NullList;

    MidasDeclareVerb("CUSTOM CREATE      upname Widget"     ,CustomCreate);
    MidasDeclareVerb("CUSTOM POPUP       upname"            ,CustomPopup);
    MidasDeclareVerb("CUSTOM CONNECT     upname name Widget",CustomConnect);
    MidasDeclareVerb("CUSTOM WRITE       upname {name}"     ,CustomWriteUser);
    MidasDeclareVerb("CUSTOM READ USER   upname {name}"     ,CustomReadUser);
    MidasDeclareVerb("CUSTOM READ SYSTEM upname {name}"     ,CustomReadSystem);
}
