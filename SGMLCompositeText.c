/*==================================================================*/
/*                                                                  */
/* SGMLCompositeTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a composite text segment for the SGMLHyper widget        */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLCompositeTextP.h"
#include "SGMLFormattedText.h"

/* 
  Private functions 
*/

static void             DestroyChildren();

/*
  Widget class methods
*/

static void    ClassPartInitialize();
static void    Initialize();
static void    Destroy();
static void    InsertChild();
static void    DeleteChild();
static void    ComputeSize();
static Boolean AdjustSize();
static void    Redisplay();
static Widget  Contains();
static void    CallMapCallback();

#define Offset(field) XtOffsetOf(SGMLCompositeTextRec,sgml_composite_text.field)

static XtResource resources[] = {

    {XtNchildren, XtCReadOnly, XtRWidgetList, sizeof(WidgetList),
     Offset(children), XtRImmediate, NULL},
    {XtNnumChildren, XtCReadOnly, XtRCardinal, sizeof(Cardinal),
     Offset(num_children), XtRImmediate, 0},
    {XtNinsertPosition, XtCInsertPosition, XtRFunction, sizeof(XtOrderProc),
     Offset(insert_position), XtRImmediate, NULL},

    {SGMLNtagList, SGMLCTagList, SGMLRTagList, sizeof(SGMLTagList),
     Offset(tag_list), XtRImmediate, NULL},
    {SGMLNdefaultClass, SGMLCClass, SGMLRClass, sizeof(WidgetClass),
     Offset(default_class), XtRImmediate, SGMLINHERIT_CLASS}
};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLCompositeTextClassRec  sGMLCompositeTextClassRec = {
    {
    (WidgetClass) &sGMLTextClassRec,     /* superclass            */
    "SGMLCompositeText",                 /* class_name            */
    sizeof(SGMLCompositeTextRec),        /* widget_size           */
    NULL,                                /* class_initialize      */
    ClassPartInitialize,                 /* class_part_initialize */
    FALSE,                               /* class_inited          */
    Initialize,                          /* initialize            */
    NULL,                                /* initialize_hook       */
    NULL,                                /* obj1                  */
    NULL,                                /* obj2                  */
    0,                                   /* obj3                  */
    resources,                           /* resources             */
    XtNumber(resources),                 /* num_resources         */
    NULLQUARK,                           /* xrm_class             */
    0,                                   /* obj4                  */
    0,                                   /* obj5                  */
    0,                                   /* obj6                  */
    0,                                   /* obj7                  */
    Destroy,                             /* destroy               */
    NULL,                                /* obj8                  */
    NULL,                                /* obj9                  */
    NULL,                                /* set_values            */
    NULL,                                /* set_values_hook       */
    NULL,                                /* obj10                 */
    NULL,                                /* get_values_hook       */
    NULL,                                /* obj11                 */
    XtVersion,                           /* version               */
    NULL,                                /* callback private      */
    NULL,                                /* obj12                 */
    NULL,                                /* obj13                 */
    NULL,                                /* obj14                 */
    NULL,                                /* extension             */
    },
    {
    ComputeSize,          		 /* compute_size          */
    SGMLInheritAdjustSize,               /* adjust_size           */
    SGMLInheritAdjustPosition,   	 /* adjust_position       */
    Redisplay,                           /* expose                */
    SGMLInheritActivate,                 /* activate              */
    SGMLInheritHilite,                   /* hilite                */
    Contains,                            /* contains              */
    SGMLInheritCallCreateCallback,       /* call_create_callback  */
    CallMapCallback,                     /* call_map_callback     */
    SGMLInheritMakeVisible,              /* make_visible          */
    NULL,                                /* extension             */
    },
    {
    NULL,                                /* geometry_manager      */
    NULL,                                /* change_managed        */
    InsertChild,                         /* insert_child          */
    DeleteChild,                         /* delete_child          */
    NULL,                                /* ignore                */
    }
};

WidgetClass sGMLCompositeTextObjectClass = (WidgetClass) &sGMLCompositeTextClassRec;

/*--------------------------------------------------------------*/
/* ClassPartInitialize:                                         */
/*--------------------------------------------------------------*/

static void ClassPartInitialize(widgetClass)
	WidgetClass widgetClass;
{
    register SGMLCompositeTextClassPart *wcPtr;
    register SGMLCompositeTextClassPart *superPtr;

    wcPtr = (SGMLCompositeTextClassPart *)
	&(((SGMLCompositeTextObjectClass)widgetClass)->sgml_composite_text_class);

    if (widgetClass != sGMLCompositeTextObjectClass)
	/* don't compute possible bogus pointer */
	superPtr = (SGMLCompositeTextClassPart *)&(((SGMLCompositeTextObjectClass)widgetClass
			->core_class.superclass)->sgml_composite_text_class);
#ifdef lint
    else
	superPtr = NULL;
#endif

    /* We don't need to check for null super since we'll get to composite
       eventually, and it had better define them!  */

    if (wcPtr->geometry_manager == XtInheritGeometryManager) {
	wcPtr->geometry_manager =
		superPtr->geometry_manager;
    }

    if (wcPtr->change_managed == XtInheritChangeManaged) {
	wcPtr->change_managed =
		superPtr->change_managed;
    }

    if (wcPtr->insert_child == XtInheritInsertChild) {
	wcPtr->insert_child = superPtr->insert_child;
    }

    if (wcPtr->delete_child == XtInheritDeleteChild) {
	wcPtr->delete_child = superPtr->delete_child;
    }

}

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLCompositeTextObject request, new;
{
    register SGMLCompositeTextObject cw;
    SGMLCompositeTextObject parent = (SGMLCompositeTextObject) XtParent((Widget) new);

    cw = (SGMLCompositeTextObject) new;
    cw->sgml_composite_text.num_children = 0;
    cw->sgml_composite_text.children = NULL;
    cw->sgml_composite_text.num_slots = 0;

    if (SGMLIsCompositeText((Widget) parent))
    {
       if (new->sgml_composite_text.default_class == SGMLINHERIT_CLASS)
         new->sgml_composite_text.default_class = parent->sgml_composite_text.default_class;
    }
    else
    {
       if (new->sgml_composite_text.default_class == SGMLINHERIT_CLASS)
         new->sgml_composite_text.default_class = sGMLFormattedTextObjectClass;
     }

}

/*--------------------------------------------------------------*/
/* DestroyChildnen:                                             */
/*--------------------------------------------------------------*/

static void DestroyChildren(w)
SGMLCompositeTextObject w;
{
    int n = w->sgml_composite_text.num_children;
    WidgetList children = w->sgml_composite_text.children;

    /*
     * This horrible fix is to work around a bug in some versions
     * of the intrinsics where XtPhase@Destroy attempts to remove a widget
     * from its (non-widget) parent's popup list, (which doesn't exist)  
     */

    ((Widget) w)->core.num_popups = 0;
    
    for (; n-- > 0; ) XtDestroyWidget(*children++);
}

/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLCompositeTextObject w;
{
/*
 * We have to explicitly delete all of out children since the 
 * intrincics dont know to do it for us.  
 */
    DestroyChildren(w);
       
    XtFree((char *) w->sgml_composite_text.children);
}

/*--------------------------------------------------------------*/
/* InsertChild: adds a new child                                */
/*--------------------------------------------------------------*/

static void InsertChild(w)
    Widget	w;
{
    register Cardinal	             position;
    register Cardinal                i;
    register SGMLCompositeTextObject cw;
    register WidgetList              children;

    cw = (SGMLCompositeTextObject) w->core.parent;
    children = cw->sgml_composite_text.children;

    if (cw->sgml_composite_text.insert_position != NULL)
	position = (*(cw->sgml_composite_text.insert_position))(w);
    else
	position = cw->sgml_composite_text.num_children;

    if (cw->sgml_composite_text.num_children == cw->sgml_composite_text.num_slots) {
	/* Allocate more space */
	cw->sgml_composite_text.num_slots +=  (cw->sgml_composite_text.num_slots / 2) + 2;
	cw->sgml_composite_text.children = children = 
	    (WidgetList) XtRealloc((XtPointer) children,
	    (unsigned) (cw->sgml_composite_text.num_slots) * sizeof(Widget));
    }

    /* Ripple children up one space from "position" */
    for (i = cw->sgml_composite_text.num_children; i > position; i--) {
	children[i] = children[i-1];
    }
    children[position] = w;
    cw->sgml_composite_text.num_children++;
}

/*--------------------------------------------------------------*/
/* DeleteChild: delete a child                                  */
/*--------------------------------------------------------------*/

static void DeleteChild(w)
    Widget	w;
{
    register Cardinal	             position;
    register Cardinal	             i;
    register SGMLCompositeTextObject cw;

    cw = (SGMLCompositeTextObject) w->core.parent;

    for (position = 0; position < cw->sgml_composite_text.num_children; position++) {
        if (cw->sgml_composite_text.children[position] == w) {
	    break;
	}
    }
    if (position == cw->sgml_composite_text.num_children) return;

    /* Ripple children down one space from "position" */
    cw->sgml_composite_text.num_children--;
    for (i = position; i < cw->sgml_composite_text.num_children; i++) {
        cw->sgml_composite_text.children[i] = cw->sgml_composite_text.children[i+1];
    }
}

/*--------------------------------------------------------------*/
/* Contains:                                                    */
/*--------------------------------------------------------------*/

static Widget Contains(w,x,y)
SGMLCompositeTextObject w;
Position x,y;
{
  int n = w->sgml_composite_text.num_children;
  WidgetList children = w->sgml_composite_text.children;

  for (; n-- > 0; children++)
    {
       SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
       Widget result = (*childClass->sgml_text_class.contains)(*children,x,y);
       if (result) return result;  
    }         
  return NULL;
}

/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region)
SGMLCompositeTextObject t;
XEvent *event;
Region region;
{
   int n = t->sgml_composite_text.num_children;
   WidgetList children = t->sgml_composite_text.children;

   for (; n-- > 0; children++)
     {
       SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
      (*childClass->sgml_text_class.expose)(*children,event,region);
     }  
}


/*------------------------------------------------------------------*/
/* Adjust the size of a child                                       */
/*------------------------------------------------------------------*/

static Boolean AdjustSize(child,y,ascent,depth)
Widget child;
Position y;
Dimension ascent, depth;
{
   SGMLCompositeTextObject w = (SGMLCompositeTextObject) XtParent(child);
   int n = w->sgml_composite_text.num_children;
   WidgetList children = w->sgml_composite_text.children;
   WidgetList p = children; 
   Boolean cont;
  
   for (; n-- > 0; p++)
     if (*p == child)
       {
         for (;  p-- != children ; ) 
           {
             SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*p);
             cont =  (*childClass->sgml_text_class.adjust_size)(*p,y,ascent,depth);
             if (!cont) return FALSE; 
           }
         return TRUE;
       }      
  return TRUE;
}

/*--------------------------------------------------------------*/
/* Call the map Callback:                                       */
/*--------------------------------------------------------------*/

static void CallMapCallback(w,event)
SGMLCompositeTextObject w;
XEvent         *event;
{
   /*
    * Invoke our childrens map callbacks 
    */

    SGMLTextCallbackStruct cb;
    int n = w->sgml_composite_text.num_children;
    WidgetList children = w->sgml_composite_text.children;
 
    for (; n-- > 0; children++)
      {
        SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
        (*childClass->sgml_text_class.call_map_callback)(*children,event);
      }  

   /*
    * Then invoke our superclasses map callback 
    */

    cb.reason = SGMLTEXT_MAP;
    cb.event = event;
    cb.text   = w->sgml_text.text;
    cb.length = w->sgml_text.length;
    cb.param  = w->sgml_text.param;
    cb.param_length = w->sgml_text.param_length;
  
    XtCallCallbacks((Widget) w,SGMLNmapCallback,(XtPointer) &cb);

}

/*--------------------------------------------------------------*/
/* Compute Size:                                                */
/*--------------------------------------------------------------*/

static void ComputeSize(w,geom,Adjust,Closure)
SGMLCompositeTextObject w;
SGMLGeometry *geom;
AdjustSizeProc Adjust;
Opaque Closure;
{
   int n = w->sgml_composite_text.num_children;
   WidgetList children = w->sgml_composite_text.children;
   Position xmin, xmax;

   /*
    * Break before?
    */  

   if ( w->sgml_text.break_before )
   {
      _SGMLBreak(geom, w->sgml_text.space_before);
   } 

   /*
    * Query children?
    */  
  
   w->sgml_text.begin = geom->coord;

   for (; n-- > 0; children++)
     {
       SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);
       (*childClass->sgml_text_class.compute_size)(*children,geom,AdjustSize,*children);
     }  

   w->sgml_text.end = geom->coord;
   w->sgml_text.height = geom->coord.y + geom->coord.depth - w->sgml_text.begin.y;
   w->sgml_text.width  = geom->actual_width;

   /*
    * Break after?
    */  

   if ( w->sgml_text.break_after )
   {
      _SGMLBreak(geom,w->sgml_text.space_after);
   } 

}

/*-----------------------------------------------------------------------*/
/* Create a new SGMLCompositeTextObject                                  */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateCompositeText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLCompositeTextObjectClass,parent,al,ac);
}

/*-----------------------------------------------------------------------*/
/* Public routine to add a child                                         */
/*-----------------------------------------------------------------------*/

void SGMLCompositeTextInsertChild(parent,child)
Widget parent;
Widget child;
{ 
    SGMLCompositeTextObjectClass parentClass = (SGMLCompositeTextObjectClass) XtClass(parent);
    (*parentClass->sgml_composite_text_class.insert_child)(child);
}
