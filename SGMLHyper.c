/*==================================================================*/
/*                                                                  */
/* SGMLHyperWidget                                                  */
/*                                                                  */
/* B.Raoult (mab@ecmwf.co.uk)                              Oct.91   */
/* T.Johnson - added SGML facilities                      June.92   */
/*             (Tony Johnson)                                       */
/*                                                                  */
/* Hyper text like widget.                                          */
/*                                                                  */
/*==================================================================*/

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/ScrolledW.h>
#include "SGMLTextP.h"
#include "SGMLPlainText.h"
#include "SGMLFormattedText.h"
#include "SGMLCompositeText.h"
#include "SGMLAnchorText.h"
#include "SGMLMarkerText.h"
#include "SGMLListText.h"
#include "SGMLHyperP.h"

#ifndef ABS
#define ABS(a)           ((a)>=0?(a):-(a))
#endif
#ifndef MIN
#define MIN(a,b)         ((a)>(b)?(b):(a))
#endif
#define MAX_LINE_SIZE    1024

/* 
  Private functions 
*/

static void    create_gcs();
static void    selectt();
static void    cursor();
static void    activate();
static Widget  add_to_text();
static void    set_tag();
static void    calc_new_size ();
static void    zoom_open ();
static void    show_selection();
static void    lowcase();

static int              NumberOfClasses = 0;
static WidgetClass     *ClassList = NULL; 
static XrmName          NullQuark;

/*
  Widget class methods
*/

static void    DeleteChild();
static void    InsertChild();
static void    ClassInitialize();
static void    Initialize();
static void    Redisplay();
static void    Resize();
static void    Destroy();
static Boolean SetValues();

static char defaultTranslations[] = 
"<Btn1Down>:select()\n<Btn1Up>: activate()\n<Motion>:cursor()";

static XtActionsRec actionsList[] = {
    { "select",   (XtActionProc) selectt},
    { "activate", (XtActionProc) activate},
    { "cursor",   (XtActionProc) cursor},
};

#define Offset(field) XtOffsetOf(SGMLHyperRec, sgml_hyper.field)

static XtResource resources[] = {

    {SGMLNactivateCallback,SGMLCCallback,XtRCallback,sizeof(caddr_t),
    Offset (activate),XtRCallback,NULL},

    {SGMLNcaseSensitiveTags,SGMLCCaseSensitiveTags,XtRBoolean,sizeof(Boolean),
    Offset (case_sensitive_tags),XtRImmediate,(XtPointer)FALSE},

    {SGMLNzoomEffect,SGMLCZoom,XtRBoolean,sizeof(Boolean),
    Offset (zoom),XtRImmediate,(XtPointer)TRUE},

    {SGMLNzoomSpeed,SGMLCZoomSpeed,XtRInt,sizeof(int),
    Offset (speed),XtRImmediate,(XtPointer)4},

    {SGMLNcacheSize,SGMLCCacheSize,XtRInt,sizeof(int),
    Offset (cache_size),XtRImmediate,(XtPointer)0},

    {SGMLNopenTag,SGMLCTagChar,XtRUnsignedChar,sizeof(unsigned char),
    Offset(open_tag),XtRImmediate,(XtPointer)'<'},

    {SGMLNcloseTag,SGMLCTagChar,XtRUnsignedChar,sizeof(unsigned char),
    Offset(close_tag),XtRImmediate,(XtPointer)'>'},

    {SGMLNendTag,SGMLCTagChar,XtRUnsignedChar,sizeof(unsigned char),
    Offset(end_tag),XtRImmediate,(XtPointer)'/'},

    {SGMLNparameterTag,SGMLCTagChar,XtRUnsignedChar,sizeof(unsigned char),
    Offset(parameter_tag),XtRImmediate,(XtPointer)' '},

    {SGMLNmargin,SGMLCMargin,XtRDimension,sizeof(Dimension),
    Offset (margin),XtRImmediate,(XtPointer)10},

    {SGMLNnaturalWidth,SGMLCWidth,XtRDimension,sizeof(Dimension),
    Offset (natural_width),XtRImmediate,(XtPointer)500},
};

#undef Offset

/* stack pointer */

static int sp = 0;

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/


static CompositeClassExtensionRec compositeExtension = {
    NULL,                                 /* next_extension   */
    NULLQUARK,                            /* record_type      */
    XtCompositeExtensionVersion,          /* version          */
    sizeof(CompositeClassExtensionRec),   /* record_size      */
    TRUE				  /* accepts_objects  */  
}; 

SGMLHyperClassRec  sGMLHyperClassRec = {
    {
#ifdef MOTIF
    (WidgetClass) &xmManagerClassRec,    /* superclass            */
#else
    (WidgetClass) &compositeClassRec,    /* superclass            */
#endif
    "SGMLHyper",                         /* class_name            */
    sizeof(SGMLHyperRec),                /* widget_size           */
    ClassInitialize,                     /* class_initialize      */
    NULL,                                /* class_part_initialize */
    FALSE,                               /* class_inited          */
    Initialize,                          /* initialize            */
    NULL,                                /* initialize_hook       */
    XtInheritRealize,                    /* realize               */
    actionsList,                         /* actions               */
    XtNumber(actionsList),               /* num_actions           */
    resources,                           /* resources             */
    XtNumber(resources),                 /* num_resources         */
    NULLQUARK,                           /* xrm_class             */
    TRUE,                                /* compress_motion       */
    TRUE,                                /* compress_exposure     */
    TRUE,                                /* compress_enterleave   */
    TRUE,                                /* visible_interest      */
    Destroy,                             /* destroy               */
    Resize,                              /* resize                */
    Redisplay,                           /* expose                */
    SetValues,                           /* set_values            */
    NULL,                                /* set_values_hook       */
    XtInheritSetValuesAlmost,            /* set_values_almost     */
    NULL,                                /* get_values_hook       */
    NULL,                                /* accept_focus          */
    XtVersion,                           /* version               */
    NULL,                                /* callback private      */
    defaultTranslations,                 /* tm_table              */
    NULL,                                /* query_geometry        */
    NULL,                                /* display_accelerator   */
    NULL,                                /* extension             */
    },
    
    {
    NULL,                                /* geometry manager      */
    NULL,				 /* change_managed        */
    InsertChild,                         /* insert_child          */
    DeleteChild,                         /* delete_child          */
    (XtPointer) &compositeExtension      /* extension             */
    },
#ifdef MOTIF
    {
     
    },
    {
    XtInheritTranslations,                /* translations                  */
    NULL,                                 /* syn_resources                 */
    0,                                    /* num_syn_resources             */
    NULL,                                 /* syn_constraint_resources      */
    0,                                    /* num_syn_constraint_resources  */
    NULL,                                 /* parent_process                */
    NULL,                                 /* extension                     */
    },
#endif
    {
    0,                                    /* ignore                */
    }
};


static Boolean IsSubClass(sub,class)
WidgetClass sub, class;
{
  for (; sub != NULL; sub = sub->core_class.superclass)
    if (sub == class) return TRUE;
   
  return FALSE;
}

WidgetClass sGMLHyperWidgetClass = (WidgetClass) &sGMLHyperClassRec;

static void CvtStringToTagList (args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    char *s, *t, *p;
    WidgetClass current_class = NULL;
       
    Cardinal num_slots = 10;
    Cardinal num_used = 0; 
    static SGMLTagList *list;
    
    list = (SGMLTagList *) XtMalloc(sizeof(SGMLTagList) * num_slots);

    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtStringToTagList","XtToolkitError",
                  "String to Tag List conversion needs no extra arguments",
                  (String *) NULL, (Cardinal *)NULL);

    s = XtNewString((char *) fromVal->addr);
    
    for (t = strtok(s," "); t != NULL; t = strtok(NULL," "))
      {
        if ((p = strchr(t,':')) == NULL)
          {
            XrmName tag = XrmStringToQuark(t);
            if (num_used == num_slots) 
              list = (SGMLTagList *) XtRealloc((XtPointer) list, sizeof(SGMLTagList) * (num_slots *= 2)); 
              
            list[num_used].name = tag;
            list[num_used].class = current_class;  
            num_used++;
          }
        else   
          {
             int i;

             *p = '\0';
             current_class = NULL; 
 
             for (i=0; i<NumberOfClasses; i++)
               if (!strcmp(t,ClassList[i]->core_class.class_name)) 
                 current_class = ClassList[i];
                 
             if (current_class == NULL)
               XtStringConversionWarning(t, SGMLRClass); 
          } 
      }
    if (num_used == num_slots) 
       list = (SGMLTagList *) XtRealloc((XtPointer) list, sizeof(SGMLTagList) * (++num_slots)); 

    list[num_used].name = 0;
    list[num_used].class = NULL;  
    num_used++; 
    toVal->size = sizeof(XtPointer);
    toVal->addr = (XtPointer) &list;
    
    XtFree(s);
}
static void CvtStringToClass (args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    char *s;
    int p;
       
    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtStringToClass","XtToolkitError",
                  "String to Class conversion needs no extra arguments",
                  (String *) NULL, (Cardinal *)NULL);

    s = (char *) fromVal->addr;
    
    for (p=0; p<NumberOfClasses; p++)
      if (!strcmp(s,ClassList[p]->core_class.class_name)) 
        {
          toVal->size = sizeof(WidgetClass);
          toVal->addr = (XtPointer) &ClassList[p];
          return;
        }     
    XtStringConversionWarning((char *) fromVal->addr, SGMLRClass);
}

/*--------------------------------------------------------------*/
/* ClassInitialize: Register the standard text classes          */
/*--------------------------------------------------------------*/
static void ClassInitialize()
{
    XtAddConverter (XtRString, SGMLRTagList, CvtStringToTagList,
		       (XtConvertArgList) NULL, (Cardinal) 0);

    XtAddConverter (XtRString, SGMLRClass, CvtStringToClass,
		        (XtConvertArgList) NULL, (Cardinal) 0);

   SGMLHyperDeclareClass(sGMLTextObjectClass);
   SGMLHyperDeclareClass(sGMLPlainTextObjectClass);
   SGMLHyperDeclareClass(sGMLFormattedTextObjectClass);
   SGMLHyperDeclareClass(sGMLAnchorTextObjectClass);
   SGMLHyperDeclareClass(sGMLMarkerTextObjectClass);
   SGMLHyperDeclareClass(sGMLCompositeTextObjectClass);
   SGMLHyperDeclareClass(sGMLListTextObjectClass);

   NullQuark = XrmStringToQuark(NULL);
}

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new) 
SGMLHyperWidget request, new;
{
    Arg arglist[10];
    int n = 0;

    /* Check the size of the widget */

    if (request->core.width == 0)
        new->core.width = 100;
    if (request->core.height == 0)
        new->core.height = 100;

/*
    new->sgml_hyper.last_selected = NULL;
    new->sgml_hyper.last_cursor   = NULL;
*/
    new->sgml_hyper.hand = XCreateFontCursor(XtDisplay(new),XC_hand2);

    /* Nothing found */
/*
    new->sgml_hyper.grep_seg = NULL;
*/
    new->sgml_hyper.grep_txt = NULL;
    new->sgml_hyper.grep_len = 0;
    new->sgml_hyper.grep_off = 0;

    new->sgml_hyper.managed = NULL;
}


/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLHyperWidget w;
{
    XtRemoveAllCallbacks ((Widget) w,SGMLNactivateCallback);
}

/*--------------------------------------------------------------*/
/* DeleteChild : deal with the case where this is managed       */
/*--------------------------------------------------------------*/

static void DeleteChild(w)
Widget w;
{
   SGMLHyperWidget parent = (SGMLHyperWidget) XtParent(w);

   if (parent->sgml_hyper.managed == w) parent->sgml_hyper.managed = NULL;
   
/* Invoke the superclass's delete_child method */   

   (*((CompositeWidgetClass)(sGMLHyperWidgetClass->core_class.superclass))->   
     composite_class.delete_child) (w);
 
}

/*--------------------------------------------------------------*/
/* InsertChild : Only accept SGMLTextObject as child            */
/*--------------------------------------------------------------*/

static void InsertChild(w)
Widget w;
{
   String params[2];
   Cardinal num_params;
   SGMLHyperWidget parent = (SGMLHyperWidget) XtParent(w);

   if (!SGMLIsText(w))
     {
       params[0] = XtClass(w)->core_class.class_name;
       params[1] = XtClass(parent)->core_class.class_name;
       num_params = 2;
       XtAppErrorMsg(XtWidgetToApplicationContext(w),
                     "childError","class","WidgetError",
                     "Children of class %s cannot be added to %s widgets",
                     params, &num_params);
     }
    
  if (parent->composite.num_children >= parent->sgml_hyper.cache_size)
    {
      if (parent->composite.num_children) XtDestroyWidget(*parent->composite.children);
    }

/* Invoke the superclass's insert_child method */   

   (*((CompositeWidgetClass)(sGMLHyperWidgetClass->core_class.superclass))->   
     composite_class.insert_child) (w);
 
}
/*--------------------------------------------------------------*/
/* Resize : not implemented                                     */
/*--------------------------------------------------------------*/


static void Resize (w)
SGMLHyperWidget w;
{
    /* 
       For futur implementation
       May be for text warp ...
    */
}

/*--------------------------------------------------------------*/
/* Redisplay : redraw the text                                  */
/*--------------------------------------------------------------*/


static void Redisplay (w, event, region)
SGMLHyperWidget  w;
XEvent       *event;
Region        region;
{
    if(w->core.visible && w->sgml_hyper.managed)
    {
      SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(w->sgml_hyper.managed);
      (*childClass->sgml_text_class.expose)(w->sgml_hyper.managed,event,region);
    }  
     
/*
        if(w->sgml_hyper.grep_seg)
        {
            if(XRectInRegion(region,
                w->sgml_hyper.grep_x,
                w->sgml_hyper.grep_y,
                w->sgml_hyper.grep_width,
                w->sgml_hyper.grep_height) != RectangleOut)

                XFillRectangle(XtDisplay(w),XtWindow(w),
                    (w->sgml_hyper.grep_seg->type == HIGHLIGHT) ? 
                      w->sgml_hyper.select_gc : w->sgml_hyper.xor_gc,
                    w->sgml_hyper.grep_x,
                    w->sgml_hyper.grep_y,
                    w->sgml_hyper.grep_width,
                    w->sgml_hyper.grep_height);

       }
*/
}

/*------------------------------------------------------------------*/
/* SetValues : redraw only for font or color changes                */
/*------------------------------------------------------------------*/

static Boolean SetValues (current, request, new)
SGMLHyperWidget current, request, new;
{
    Boolean    redraw = FALSE;

#define HAS_CHANGED(a)    (new->a != current->a)

    if(
        HAS_CHANGED(sgml_hyper.natural_width)
      )
    {

        /* invalidate size of all text widget children */
        
        int n = new->composite.num_children;
        SGMLTextObject *children = (SGMLTextObject *) new->composite.children;
        
        for (; n-- > 0; children++) (*children)->sgml_text.size_valid = FALSE;

        /* rebuild text */

        calc_new_size(new);
        redraw = TRUE;
    }

    return (redraw);

#undef HAS_CHANGED

}

/*------------------------------------------------------------------*/
/* Adjust the size of a child                                       */
/*------------------------------------------------------------------*/

static AdjustSizeProc adjust_size(child,y,ascent,depth)
Widget child;
Position y;
Dimension ascent, depth;
{
  return FALSE;
}
/*------------------------------------------------------------------*/
/* Calculate the size of the widget                                 */
/*------------------------------------------------------------------*/

static void calc_new_size(w)
SGMLHyperWidget  w;
{
    XtGeometryResult    result;
    Dimension           replyWidth = 0, replyHeight = 0;
    Dimension           width, height;
    SGMLGeometry        geom;

    geom.coord.x = 0;
    geom.coord.y = w->sgml_hyper.margin;
    geom.coord.depth = 0;
    geom.coord.ascent = 0;
    geom.broken = TRUE;
    geom.leave_space = FALSE;
    geom.space = 99999;
    geom.actual_width = 2 * w->sgml_hyper.margin;
    geom.natural_width = w->sgml_hyper.natural_width;

    /* Loop over all of the text segments, getting each to 
       tell us how much space it needs                     */

    if (w->sgml_hyper.managed) 
      {
        SGMLTextObject t = (SGMLTextObject) w->sgml_hyper.managed;
        
        if (t->sgml_text.size_valid)
          {
            width =  t->sgml_text.width;
            height = geom.coord.y + t->sgml_text.height;
          }
        else
          {
            SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass((Widget) t);
            (*childClass->sgml_text_class.compute_size)(t,&geom,adjust_size,w->sgml_hyper.managed);
       
            t->sgml_text.size_valid = TRUE; 
            width = geom.actual_width;
            height = geom.coord.y + geom.coord.depth;
          }
      } 
    width  += w->sgml_hyper.margin;
    height += w->sgml_hyper.margin;

    /* 
    Tell our parent we want a new size 
    */

    if(w->core.width != width || w->core.height != height)
    {
        result = XtMakeResizeRequest((Widget) w,width,height, 
            &replyWidth, &replyHeight) ;

        if (result == XtGeometryAlmost)
            XtMakeResizeRequest ((Widget) w, replyWidth, replyHeight,NULL, NULL);
        
    }

    /* 
    Redraw the window
    */
   
    if (XtIsRealized((Widget) w)) XClearArea(XtDisplay((Widget)w),XtWindow((Widget)w),0,0,0,0,True);

}

/*-----------------------------------------------------------------------*/
/* Find the "visible" part of a widget as the intersection of all the    */
/* windows of it's parents' windows                                      */
/*-----------------------------------------------------------------------*/

static void find_visible_part(w,x,y,width,height)
Widget    w;
Position  *x;
Position  *y;
Dimension *width;
Dimension *height;
{
    Position root_x,root_y;
    Widget   p = w;

    *width  = w->core.width;
    *height = w->core.height;
    XtTranslateCoords(w,0,0,&root_x,&root_y);

    *x = 0;
    *y = 0;

    while(p = XtParent(p))
    {
        Position  rx,ry;
        Dimension w,h;

        /* 
           make all computations in the root's
           coordinate system
        */

        XtTranslateCoords(p,0,0,&rx,&ry);

        w = p->core.width;
        h = p->core.height;

        /* 
            use the smallest rectangle
        */

        if(w < *width)  *width  = w;
        if(h < *height) *height = h;

        if(rx>root_x) root_x = rx;
        if(ry>root_y) root_y = ry;

        /* stop when reach a shell,
          don't go to top level shell */
        if(XtIsShell(p)) break;
    }

    /* Back to the widget's coordinate system */

    XtTranslateCoords(w,0,0,x,y);
    *x = root_x - *x;
    *y = root_y - *y;


}

/*-----------------------------------------------------------------------*/
/* Do an "zoom" effect animation, from the selected text segment to the  */
/* visible part of the widget                                            */
/*-----------------------------------------------------------------------*/

static void zoom_open(w)
SGMLHyperWidget   w;
/*
text_segment *s;
*/
{
/*
    int dx1,dx2,dy1,dy2;

    Position x ;
    Position y ;
    Dimension width  ;
    Dimension height ;

    /* selected rectangle * 

    Position  xs = s->x;
    Position  ys = s->y;
    Dimension ws = s->width;
    Dimension hs = s->height;


    /* get the rectangle we want to zoom to *

    find_visible_part(w,&x,&y,&width,&height);

    /* make sure selected rectangle in visible *

    if(xs<x) xs = x;
    if(ys<y) ys = y;
    if(xs+ws > x+width)  ws = x+width-xs;
    if(ys+hs > y+height) hs = y+height-ys;

    /* get the offsets in each directions *

    dx1 = x-xs;
    dy1 = y-ys;
    dx2 = ((x+width)-(xs+ws));
    dy2 = ((y+height)-(ys+hs));

    /* in the rectangles are differents *

    if(dx1 || dy1 || dx2 || dy2)
    {
        int min = 32000; /* <-- Can be buggy *

        /* 
          work in "left,top,bottom,right" rectangles (Mac)
          rather than "x,y,width,height" (X)
          It's easier for the animation 
        *

        int xws = xs+ws;
        int yhs = ys+hs;
        int xw  = x + width;
        int yh  = y + height;


        /* Get smallest non-null offset *

        if(dx1) min = MIN(min,ABS(dx1));
        if(dx2) min = MIN(min,ABS(dx2));
        if(dy1) min = MIN(min,ABS(dy1));
        if(dy2) min = MIN(min,ABS(dy2));

        /* Scale offsets so minimun offset is 1 pixel *

        dx1 /= min;
        dx2 /= min;
        dy1 /= min;
        dy2 /= min;

        /* Use speed .. *

        dx1 *= w->sgml_hyper.speed;
        dx2 *= w->sgml_hyper.speed;
        dy1 *= w->sgml_hyper.speed;
        dy2 *= w->sgml_hyper.speed;

        /* Animate *
 
        while(min--)
        {
            XDrawRectangle(XtDisplay(w),XtWindow(w),
                w->sgml_hyper.xor_gc,xs,ys,xws-xs,yhs-ys);

            /* Needed, otherwise X calls are buffered *

            XSync(XtDisplay(w),False);

            XDrawRectangle(XtDisplay(w),XtWindow(w),
                w->sgml_hyper.xor_gc,xs,ys,xws-xs,yhs-ys);

            xs += dx1;
            ys += dy1;

            xws += dx2;
            yhs += dy2;

        }

    }
*/
}

/*----------------------------------------------------------------------*/
/* Find the Widget (Gadget) at point (x,y)                              */
/*----------------------------------------------------------------------*/
SGMLTextObject find_segment(w,x,y)
SGMLHyperWidget w;
int x,y;
{
   if (w->sgml_hyper.managed)
      {
         SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(w->sgml_hyper.managed);
         Widget result = (*childClass->sgml_text_class.contains)(w->sgml_hyper.managed,x,y);
         return (SGMLTextObject) result;  
      }         

    return NULL;
}
/*-----------------------------------------------------------------------*/
/* Check for mouse down                                                  */
/*-----------------------------------------------------------------------*/

static void selectt (w, event, args, n_args)
SGMLHyperWidget   w;
XEvent        *event;
char          *args[];
int            n_args;
{
    /* 
       Find if the user clicked in a (sensitive) text object 
    */

    SGMLTextObject s = find_segment(w,event->xbutton.x,event->xbutton.y);
    w->sgml_hyper.last_selected = s;

    if (s != NULL) 
      {
         SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass((Widget) s);
         (*childClass->sgml_text_class.hilite)(s,TRUE);  
      }
}

/*-----------------------------------------------------------------------*/
/* Check for mouse up                                                    */
/*-----------------------------------------------------------------------*/

static void activate (w, event, args, n_args)
SGMLHyperWidget   w;
XEvent        *event;
char          *args[];
int            n_args;
{
    SGMLTextObject s = find_segment(w,event->xbutton.x,event->xbutton.y);

    /* 
       Find if the user clicked in a highlighted text 
    */

    if(s != NULL && s == w->sgml_hyper.last_selected)
    {
         SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass((Widget) s);
         (*childClass->sgml_text_class.hilite)(s,FALSE);  

         /* zoom if required *
          *
          * if(w->sgml_hyper.zoom) zoom_open(w,s); 
          */
 
         (*childClass->sgml_text_class.activate)(s,event);  

    }
    w->sgml_hyper.last_selected = NULL;
}

/*-----------------------------------------------------------------------*/
/* Check for mouse moves                                                 */
/*-----------------------------------------------------------------------*/

static void cursor (w, event, args, n_args)
SGMLHyperWidget   w;
XEvent        *event;
char          *args[];
int            n_args;
{
    SGMLTextObject s = find_segment(w,event->xbutton.x,event->xbutton.y);
    SGMLTextObject l = w->sgml_hyper.last_selected;
 
    if(s != w->sgml_hyper.last_cursor)
    {
        if ( l != NULL && (s == l || l == w->sgml_hyper.last_cursor))
          {
            SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass((Widget) l);
            (*childClass->sgml_text_class.hilite)(l,s == l);
          }
           
        if(s) XDefineCursor(XtDisplay(w),XtWindow(w),w->sgml_hyper.hand);
        else  XUndefineCursor(XtDisplay(w),XtWindow(w));
        
        w->sgml_hyper.last_cursor = s;
    }
}

/*-----------------------------------------------------------------------*/
/* Add text to a text segment                                            */
/*-----------------------------------------------------------------------*/

static Widget add_to_text(name,parent,class,text,param)
Widget parent;
SGMLTextObjectClass class;
char *text, *param, *name;
{
    Arg arglist[10];
    int n=0;  
    Widget new;
    
    XtSetArg(arglist[n],SGMLNtext,  text);  n++;
    XtSetArg(arglist[n],SGMLNparam, param); n++;
    new = XtCreateWidget(name,(WidgetClass) class,parent,arglist,n);

    if (SGMLIsCompositeText(parent)) SGMLCompositeTextInsertChild(parent,new);
    
    /*
     *  Invoke the CreateCallback
     */  

    (*class->sgml_text_class.call_create_callback)(new,NULL);
     
    return new;
}    

/*-----------------------------------------------------------------------*/
/* check a tag: returns TRUE if tag accepted                             */    
/*                      FALSE if tag rejected                            */
/*-----------------------------------------------------------------------*/

static Boolean check_tag(w,tag,Text,Name)
SGMLHyperWidget w;
char *tag;
XrmName Name;
Widget Text;
{
  XrmName tagQuark;
  char et = w->sgml_hyper.end_tag;
  Arg arglist[10];
  int n = 0;
  int i;
  SGMLTagList *tagList;

  Boolean end = (*tag == et);
  if (end) tag++;  

  if (!w->sgml_hyper.case_sensitive_tags) lowcase(tag);
  tagQuark = XrmStringToQuark(tag);  

  if (Name != NullQuark)
    if (end) return (tagQuark == Name);
    else     return FALSE;

  if (end)
    if (tagQuark == Text->core.xrm_name) return TRUE;
    else return FALSE;

  XtSetArg(arglist[n],SGMLNtagList,&tagList); n++;
  XtGetValues(Text,arglist,n);

  if (tagList)
    for (i=0; tagList[i].class != NULL; i++)
      if (tagList[i].name == tagQuark) return TRUE;
    
  return FALSE;  
}

/*-----------------------------------------------------------------------*/
/* set a new tag: the tag must previously have been accepted by          */    
/* check tag                                                             */
/*-----------------------------------------------------------------------*/


static void set_tag(w,tag,inoutText,inoutClass,inoutName,param)
SGMLHyperWidget w;
char *tag, *param; 
XrmName *inoutName;
Widget *inoutText;
WidgetClass *inoutClass;
{ 
  XrmName tagQuark;
  char et = w->sgml_hyper.end_tag;
  Widget text = *inoutText;
  WidgetClass class = *inoutClass;
  XrmName name = *inoutName; 

  Boolean end = (*tag == et);
  if (end) tag++;  

  if (!w->sgml_hyper.case_sensitive_tags) lowcase(tag);
  tagQuark = XrmStringToQuark(tag);  

  if (name != NullQuark)
    { 
       Arg arglist[10];
       int n = 0;

       *inoutName = NullQuark;

       XtSetArg(arglist[n],SGMLNdefaultClass,&class); n++;
       XtGetValues(text,arglist,n);
       *inoutClass = class;

       return;
    }   

  if (end) text = XtParent(text);
  else
  {
    Arg arglist[10];
    int n = 0;
    int i;
    SGMLTagList *tagList;

    XtSetArg(arglist[n],SGMLNtagList,&tagList); n++;
    XtGetValues(text,arglist,n);
    
    for (i=0; tagList[i].class != NULL; i++)
      if (tagList[i].name == tagQuark)
      { 
         class = tagList[i].class;
         if (IsSubClass(class,sGMLMarkerTextObjectClass))
         {
           add_to_text(tag,text,class,NULL,param);
           class = *inoutClass;
         }
         else if (IsSubClass(class,sGMLCompositeTextObjectClass))
         {
           text = add_to_text(tag,text,class,NULL,param);
         }  
         else
         {
           *inoutName = tagQuark; 
         }
         break;
      } 
    }    
  if (text != *inoutText)
  {
    Arg arglist[10];
    int n = 0;
       
    XtSetArg(arglist[n],SGMLNdefaultClass,&class); n++;
    XtGetValues(text,arglist,n);
  }
  *inoutText = text;
  *inoutClass = class;
}
/*-----------------------------------------------------------------------*/
/* Rebuild the text structure. Called when the font changes              */
/*-----------------------------------------------------------------------*/

void SGMLHyperManageChild(w)
Widget w;
{
    SGMLTextObjectClass class = (SGMLTextObjectClass) XtClass(w);
    Widget parent = XtParent(w); 

    XUndefineCursor(XtDisplay(parent),XtWindow(parent)); /* In case cursor was set */

    ((SGMLHyperWidget) parent)->sgml_hyper.managed = w;

    (*class->sgml_text_class.call_map_callback)(w,NULL);

    calc_new_size(parent);
}
/*-----------------------------------------------------------------------*/
/* Build the text. Gets the chars from the funtion "get_next_char"       */
/* using "data" as a parameter                                           */
/*-----------------------------------------------------------------------*/

static Widget set_text(w,get_next_char,data)
SGMLHyperWidget   w;
char (*get_next_char)();
XtPointer data;
{
#define NORMAL 0
#define TAG    1
#define PARAM  2 

    char *tag, *word, *param = NULL;
    int wordsize; 
    int  i = 0;
    char ot = w->sgml_hyper.open_tag;
    char ct = w->sgml_hyper.close_tag;
    char pt = w->sgml_hyper.parameter_tag;
    Widget parentText, topText;
    WidgetClass currentClass;
    XrmName currentName = NullQuark;
    char c;
    int mode = NORMAL;
    Arg arglist[10];
    int n = 0;

    /* Create the base text object */
 
    topText = parentText = XtCreateWidget("default",sGMLCompositeTextObjectClass,(Widget)w,NULL,0);
    XtSetArg(arglist[n],SGMLNdefaultClass,&currentClass); n++;
    XtGetValues(parentText,arglist,n);
/*
    w->sgml_hyper.first_seg     = NULL;
    w->sgml_hyper.last_selected = NULL;
    w->sgml_hyper.last_cursor   = NULL;
    w->sgml_hyper.grep_seg = NULL;
*/
    w->sgml_hyper.grep_txt = NULL;
    w->sgml_hyper.grep_len = 0;
    w->sgml_hyper.grep_off = 0;

    wordsize = MAX_LINE_SIZE;
    word = XtMalloc(wordsize);
 
    while(c = (get_next_char)(data))
    {
        /* kludge */
        
        if (mode == TAG && (c == '\n' || c == '\t')) c = ' '; 

        /* Open Tag */
    
        if (mode == NORMAL && c == ot)
        {
            word[i++] = c;
            if(i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
            tag = &word[i];    
            mode = TAG;
        }

        /* Parameter */

        else if (mode == TAG && c == pt)
        {
            word[i] = '\0';
            if (check_tag(w,tag,parentText,currentName))
            {
              *(tag-1) = '\0';
              if (*word) add_to_text(XrmQuarkToString(currentName),parentText,currentClass,word,param);
              i++;
              if(i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
              XtFree(param);
              param = &word[i];
              mode = PARAM;
            }
            else 
            {
              word[i++] = pt;
              if(i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
              mode = NORMAL;
            } 
       }

        /* Close Tag */
        
        else if (mode == TAG && c == ct)
        {
            word[i] = '\0';
            if (check_tag(w,tag,parentText,currentName))
            {
              *(tag-1) = '\0';
              if (*word) add_to_text(XrmQuarkToString(currentName),parentText,currentClass,word,param);
              set_tag(w,tag,&parentText,&currentClass,&currentName,param);
              i = 0;     
              mode = NORMAL;
              XtFree(param); 
              param = NULL;   
            }
            else 
            {
              word[i++] = ct;
              if(i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
              mode = NORMAL;
            } 
        }
         
        else if (mode == PARAM && c == ct)
        {
            word[i] = '\0';
            set_tag(w,tag,&parentText,&currentClass,&currentName,param);
            param = XtNewString(param); 
            mode = NORMAL;
            i = 0;     
        }

        else 

        {
            word[i++] = c;
            if(i == wordsize) word = XtRealloc(word, (wordsize *= 2)); 
        }
    }

    /* flush .. */

    if(i)
    {
        word[i] = '\0';
        if (i) add_to_text(XrmQuarkToString(currentName),parentText,currentClass,word,param);
    }

    if (mode != PARAM) XtFree(param);  
    XtFree(word);

    return topText;
}

/*-----------------------------------------------------------------------*/
/* Declare a Widget class for the converter                              */
/*-----------------------------------------------------------------------*/

void SGMLHyperDeclareClass(class)
WidgetClass class;
{
   static int MaxClasses = 0;  
   if (NumberOfClasses == MaxClasses) 
     ClassList = (WidgetClass *)  XtRealloc((XtPointer) ClassList, (MaxClasses += 10) * sizeof(WidgetClass));
   ClassList[NumberOfClasses++] = class;
}

/*-----------------------------------------------------------------------*/
/* Create a new HyperWidget                                              */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateHyper(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLHyperWidgetClass,parent,al,ac);
}


/*-----------------------------------------------------------------------*/
/* Load the text from a file                                             */
/*-----------------------------------------------------------------------*/

/* provides chars to "set_text" routine */

static char get_from_file(f)
FILE **f;
{
    int n =  getc(*f);
    return (n==EOF?0:(char)n);
}

/* Public routine */

Widget SGMLHyperLoadFile(widget,fname)
Widget widget;
char   *fname;
{
    extern const char *const sys_errlist[];
    Widget result = NULL;

    FILE *f = fopen(fname,"r");
    if(f)
    {
        result = set_text(widget,get_from_file,&f);
        fclose(f);
    }
    else
    {
        char msg[1024];
        sprintf(msg,"%s: %s",fname,sys_errlist[errno]);
        XtWarning(msg);
    }
    return result;  
}

/*-----------------------------------------------------------------------*/
/* Load text using a getchar like routine                                */
/*-----------------------------------------------------------------------*/

Widget SGMLHyperLoadText(widget,get_character,closure)
Widget widget;
char (*get_character)();
XtPointer closure;
{
   return set_text(widget,get_character,closure);
}

/*-----------------------------------------------------------------------*/
/* Load text from memory buffer                                          */
/*-----------------------------------------------------------------------*/

/* provides chars to "set_text" routine */

static char get_from_buffer(buffer)
char **buffer;
{
    char c = **buffer;
    (*buffer)++;
    return c;
}

/* Public routine */

Widget SGMLHyperSetText(widget,text)
Widget  widget;
char *text;
{
    return set_text(widget,get_from_buffer,&text);
}

/*-----------------------------------------------------------------------*/
/* Specifies start and end of highlignt chars                            */
/*-----------------------------------------------------------------------*/

 
void SGMLHyperSetTags(widget,open_tag,close_tag,end_tag,parameter_tag)
Widget widget;
unsigned char open_tag;
unsigned char close_tag;
unsigned char end_tag;
unsigned char parameter_tag;
{
    ((SGMLHyperWidget)widget)->sgml_hyper.open_tag      = open_tag;
    ((SGMLHyperWidget)widget)->sgml_hyper.close_tag     = close_tag;
    ((SGMLHyperWidget)widget)->sgml_hyper.end_tag       = end_tag;
    ((SGMLHyperWidget)widget)->sgml_hyper.parameter_tag = parameter_tag;
}


/*-----------------------------------------------------------------------*/
/* convert a string to lower case                                        */
/*-----------------------------------------------------------------------*/

static void lowcase(p)
register char *p;
{
    while(*p)
    {
        *p = tolower(*p); 
        p++;
    }
}

/*-----------------------------------------------------------------------*/
/* Returns the text of the widget                                        */
/* the memory is allocated. It must be freed by the application          */
/* If include_tags is FALSE, the special characters are not returned     */
/*-----------------------------------------------------------------------*/

static void dump_text(hyper,w,include_tags,calc_len,pp)
SGMLHyperWidget hyper;
SGMLTextObject w;
Boolean include_tags, calc_len;
char **pp;
{
  
  char ot = hyper->sgml_hyper.open_tag;
  char ct = hyper->sgml_hyper.close_tag;
  char et = hyper->sgml_hyper.end_tag;
  char pt = hyper->sgml_hyper.parameter_tag;

  char *p;
  int len; 
  char *name = XtName((Widget)w);

  if (calc_len) len = (int) *pp;
  else          p   = *pp;   
 
  if (!strcmp(name,"default")) name = "";
  
  if (*name && include_tags)
  {
    char *q = name;
    if (calc_len) len++;
    else          *p++ = ot; 
    if (calc_len) len += strlen(q);
    else          for (; *q != '\0'; ) *p++ = *q++; 
    if (w->sgml_text.param_length)
      {  
        char *param = w->sgml_text.param;
        if (calc_len) len++;
        else          *p++ = pt;
        if (calc_len) len += strlen(param);
        else          for (; *param != '\0';) *p++ = *param++;
      }               
    if (calc_len) len++;
    else          *p++ = ct;
  }  
  if (w->sgml_text.text)
  {
    char *text = w->sgml_text.text; 
    if (calc_len) len += strlen(text);
    else          for (; *text != '\0';) *p++ = *text++; 
  }
  if (SGMLIsCompositeText((Widget)w))
  {
    int n; 
    WidgetList children;
    int nn = 0;
    Arg arglist[10];
    
    XtSetArg(arglist[nn],XtNnumChildren,&n); nn++;
    XtSetArg(arglist[nn],XtNchildren,&children); nn++;
    XtGetValues((Widget)w,arglist,nn); 
    
    if (calc_len) for (; n-- > 0; children++) dump_text(hyper,*children,include_tags,calc_len,&len);
    else          for (; n-- > 0; children++) dump_text(hyper,*children,include_tags,calc_len,&p);
  }
  if (!SGMLIsMarkerText((Widget) w) && include_tags && *name)
  {
    char *q = name;
    if (calc_len) len ++;
    else          *p++ = ot;
    if (calc_len) len ++;
    else          *p++ = et;
    if (calc_len) len += strlen(q);
    else          for (; *q != '\0'; ) *p++ = *q++; 
    if (calc_len) len ++; 
    else          *p++ = ct;
  }  
  if (calc_len) *pp = (char *) len;
  else          *pp = p; 
}

char *SGMLHyperGetText(widget, include_tags)
Widget  widget;
Boolean include_tags;
{

    SGMLHyperWidget w = (SGMLHyperWidget)widget;
    char  *result, *p;
    int   len = 1;
     
    if (w->sgml_hyper.managed)
      {
        dump_text(w,w->sgml_hyper.managed,include_tags,TRUE,&len);
        result = p = XtMalloc(len);
        dump_text(w,w->sgml_hyper.managed,include_tags,FALSE,&p);

        *p++ = '\0';
        return result;
      }
    else return NULL; 
}

/*-----------------------------------------------------------------------*/
/* Only for Motif                                                        */
/* If the widget is in a XmScrolledWindow, scroll it so the selection is */
/* visible                                                               */
/*-----------------------------------------------------------------------*/

void SGMLHyperShowObject(h,object)
SGMLHyperWidget h;
SGMLTextObject object;
{
#define SetArg(a,b)  XtSetArg(al[ac],a,b);ac++
#define GetValues(w) XtGetValues(w,al,ac);ac=0
#define SetValues(w) XtSetValues(w,al,ac);ac=0

    Widget clip = XtParent(h);
    Widget swin;

    Widget h_scroll;
    Widget v_scroll;

    int ac = 0;

    Position    x_parent,y_parent;
    Position    x_grep,y_grep;
    Dimension   h_grep,w_grep;
    Position    x_clip,y_clip;
    Dimension   h_clip,w_clip;
    Position    dv=0,dh=0;
    int min,max;
    int v_val,v_size,v_inc,v_page;
    int h_val,h_size,h_inc,h_page;
    Position x,y;

    Arg al[5];

    /* check if selection exists */

    /* check if the widget is in a scrolled window */
    /* the XmScrolledWindow creates a clip window  */
    /* The widget's parent is the clip window      */


    if(!clip) return;
    swin = XtParent(clip);

    if(!swin || !XmIsScrolledWindow(swin)) return;


    /* Get window scroll bars */

    SetArg(XmNhorizontalScrollBar, &h_scroll);
    SetArg(XmNverticalScrollBar  , &v_scroll);
    GetValues(swin);

    /* Get size of clip window and selection rect */

    w_clip = clip->core.width;
    h_clip = clip->core.height;

    /* If no object specified then position at top */
    
    if (!object)
    {
        SetArg(XmNminimum,&min);
        GetValues(v_scroll);
        XmScrollBarGetValues(v_scroll,&v_val,&v_size,&v_inc,&v_page);
        dv = min - v_val;

        SetArg(XmNminimum,&min);
        GetValues(h_scroll);
        XmScrollBarGetValues(h_scroll,&h_val,&h_size,&h_inc,&h_page);
        dh = min - h_val;
    }
    else 
    {
      w_grep = object->sgml_text.width;
      h_grep = object->sgml_text.height;

      /* Get global coordinates of clip and selection rect */

      XtTranslateCoords(clip,0,0,&x_clip,&y_clip);
      XtTranslateCoords((Widget)h,object->sgml_text.margin,object->sgml_text.begin.y,&x_grep,&y_grep);
  
      /* offset of selection within clip window */

      x = x_grep - x_clip;
      y = y_grep - y_clip;


      /* selection y coordinate is not visible */

      if( y < 0 || y + h_grep > h_clip)
      {
          /* the widget must be moved verticaly by dv pixels */

          dv = (y + h_grep / 2)  - h_clip / 2;

          SetArg(XmNminimum,&min);
          SetArg(XmNmaximum,&max);

          GetValues(v_scroll);

          XmScrollBarGetValues(v_scroll,&v_val,&v_size,&v_inc,&v_page);

          max -= v_size;
  
          if( dv + v_val > max ) dv = max - v_val;
          if( dv + v_val < min ) dv = min - v_val;
  
  
      }

      /* selection x coordinate is not visible */

      if( x < 0 || x + w_grep > w_clip)
      {
          /* the widget must be moved horizontaly by dh pixels */

          dh = (x + w_grep / 2)  - w_clip / 2;

          SetArg(XmNminimum,&min);
          SetArg(XmNmaximum,&max);
          GetValues(h_scroll);

          XmScrollBarGetValues(h_scroll,&h_val,&h_size,&h_inc,&h_page);

          max -= h_size;

          if( dh + h_val > max ) dh = max - h_val;
          if( dh + h_val < min ) dh = min - h_val;

      }
    
    }
    /* if the widget must be moved */

    if(dv || dh)
    {
        Position x = h->core.x-dh;
        Position y = h->core.y-dv;

        /* move it */

        SetArg(XmNx,x);
        SetArg(XmNy,y);
        SetValues((Widget) h);

        /* update scroll bars */

        if(dv) XmScrollBarSetValues(v_scroll,v_val+dv,v_size,v_inc,
            v_page,TRUE);
        if(dh) XmScrollBarSetValues(h_scroll,h_val+dh,h_size,h_inc,
            h_page,TRUE);


    }

}

/*-----------------------------------------------------------------------*/
/* Clear previous selection                                              */
/*-----------------------------------------------------------------------*/

static void clear_selection(w)
SGMLHyperWidget w;
{
/*
    if(w->sgml_hyper.grep_seg)
    {
        if(XtIsRealized(w))
*/
            /* force a redraw */
/*
            XClearArea(XtDisplay(w),XtWindow(w),
                w->sgml_hyper.grep_x,
                w->sgml_hyper.grep_y,
                w->sgml_hyper.grep_width,
                w->sgml_hyper.grep_height,
                TRUE);

    }
    w->sgml_hyper.grep_seg = NULL;
*/
}

/*-----------------------------------------------------------------------*/
/* Set the new selection                                                 */
/*-----------------------------------------------------------------------*/

static void set_selection(w)
SGMLHyperWidget w;
{
/*
    if(w->sgml_hyper.grep_seg)
    {
        text_segment *s = w->sgml_hyper.grep_seg;
        char *p = s->text;
        XCharStruct   char_info;
        int dir,ascent,desc;
*/
        /* get size of the begining of
           the segment, up to the found string */
/*
        XTextExtents(
            s->style->sgml_style.font,
            s->text,
            w->sgml_hyper.grep_off,
            &dir,&ascent,&desc,&char_info);

        w->sgml_hyper.grep_x      = s->x + char_info.width;
        w->sgml_hyper.grep_y      = s->y + desc;
        w->sgml_hyper.grep_height = s->height;
*/
        /* Get size of the selection */
/*
        XTextExtents(
            s->style->sgml_style.font,
            w->sgml_hyper.grep_txt,
            w->sgml_hyper.grep_len,
            &dir,&ascent,&desc,&char_info);


        w->sgml_hyper.grep_width  = char_info.width;
*/
        /* force update */
/*
        if(XtIsRealized(w))
            XClearArea(XtDisplay(w),XtWindow(w),
                w->sgml_hyper.grep_x,
                w->sgml_hyper.grep_y,
                w->sgml_hyper.grep_width,
                w->sgml_hyper.grep_height,
                TRUE);
    }
*/
}

/* size of regexp buffer */

#define ESIZE 1024

/*-----------------------------------------------------------------------*/
/* if you have regexp, define USE_REGEXP                                 */
/* NOTE : since regexp variables and functions are not static            */
/* you can have some problems if you use the same names or include       */
/* regexp somewhere else                                                 */
/*-----------------------------------------------------------------------*/
#ifdef USE_REGEXP

/* regexp macros ... see "man regexp" */

#define INIT        register char *sp = instring;
#define GETC()      (*sp++)
#define PEEKC()     (*sp)
#define UNGETC(c)   (--sp)
#define RETURN(c)   return;
#define ERROR(c)    fprintf(stderr,"Warning regexp error %d\n",c)


#include <regexp.h>


#else 

/*-----------------------------------------------------------------------*/
/* If we don't have regexp mimic it.                                     */
/* Just find plain text using strncmp. no regular expression matching    */
/*-----------------------------------------------------------------------*/

static char *loc1,*loc2;
static int len;

static compile(w,buf,end,dummy)
char *w,*buf;
int end;
int dummy;
{
    strcpy(buf,w);
    len = strlen(w);
}

static step(w,buf)
char *w;
char *buf;
{
    loc1 = w;
    while(*loc1)
    {
        if(strncmp(loc1,buf,len) == 0)
        {
            loc2 = loc1+len;
            return TRUE;
        }
        loc1++;
    }
    return FALSE;
}


#endif

/*-----------------------------------------------------------------------*/
/* Select a word in the hyper widget                                     */
/* word : word to find ( or regular expression if USE_REGEX is defined)  */
/* ignore_case : if TRUE ignore case in comparaison                      */
/* from_start : if TRUE search from start of text, else search from      */
/* current selection                                                     */
/* wrap: if TRUE, continue search from the begining of text if the end   */
/* is reached                                                            */
/*-----------------------------------------------------------------------*/

#ifdef _NO_PROTO

Boolean SGMLHyperGrep(widget,word,ignore_case,from_start,wrap)
Widget   widget;
char     *word;
Boolean  ignore_case;
Boolean  from_start;
Boolean  wrap;

#else

Boolean SGMLHyperGrep(Widget widget,
		      char *word,
     		      Boolean ignore_case,
		      Boolean from_start,
		      Boolean wrap)

#endif

{
#ifdef dummy
    SGMLHyperWidget  h = (SGMLHyperWidget)widget;
    char         *w = word;
    char         *p;
    int          offset,from,to;
    text_segment *s;
    char          expbuf[ESIZE];

    if(!h->sgml_hyper.first_seg) return;

    if(ignore_case)
    {
        /* if ignore case, change word to lower case */ 
        w = XtNewString(word);
        lowcase(w);
    }

    /* compile the regular expression */
    compile(w,expbuf,&expbuf[ESIZE],'\0');


    if(ignore_case) XtFree(w);

    /* if from_start or no previous selection, 
       start from first segment */

    if(from_start || h->sgml_hyper.grep_seg == NULL)
    {
        offset=0;
        wrap = FALSE;
        s = h->sgml_hyper.first_seg;
    }
    else 
    {
        /* start from last selection */

        offset = h->sgml_hyper.grep_off + h->sgml_hyper.grep_len;
        s = h->sgml_hyper.grep_seg;
    }

    for(;;)
    {
        if(s->text)
        {
            if(ignore_case)
            {
                /* if ignore case, change segment to lower case */
                p = XtNewString(s->text);
                lowcase(p);
            }

            /* search the string */

            if(step(p+offset,expbuf))
            {
                /* if found ...*/

                /* clear previous selection */
                clear_selection(h);

                h->sgml_hyper.grep_seg = s;
                h->sgml_hyper.grep_off = offset + (loc1-(p+offset));
                h->sgml_hyper.grep_txt = s->text + h->sgml_hyper.grep_off;
                h->sgml_hyper.grep_len = loc2-loc1;

                /* set new selection */

                set_selection(h);

                /* make it visible */

                show_selection(h);

                if(ignore_case) XtFree(p);

                return TRUE;
            }

            if(ignore_case) XtFree(p);
        }

        offset = 0;
        s = s->next;

        /* if end of text and wrap mode, go to start of text */
        if(!s)
            if(wrap)
            {
                wrap = FALSE;
                s = h->sgml_hyper.first_seg;
            }
            else break;

    }


    return FALSE;
#endif
}
