/*==================================================================*/
/*                                                                  */
/* SGMLPlainTextObject                                              */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a plain text segment for the SGMLHyper widget            */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLPlainTextP.h"

/* 
  Private functions 
*/

static int LineSize();

/*
  Widget class methods
*/

static void    Initialize();
static void    Destroy();
static void    ComputeSize();
static void    Redisplay();

#define Offset(field) XtOffsetOf(SGMLPlainTextRec,sgml_plain_text.field)

static XtResource resources[] = {

    {SGMLNtabs,SGMLCTabs,XtRInt,sizeof(int),
    Offset (tabs),XtRImmediate,(XtPointer)0},

};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLPlainTextClassRec  sGMLPlainTextClassRec = {
    {
    (WidgetClass) &sGMLTextClassRec,     /* superclass            */
    "SGMLPlainText",                     /* class_name            */
    sizeof(SGMLPlainTextRec),            /* widget_size           */
    NULL,                                /* class_initialize      */
    NULL,                                /* class_part_initialize */
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
    SGMLInheritContains,                 /* contains              */
    SGMLInheritCallCreateCallback,       /* call_create_callback  */
    SGMLInheritCallMapCallback,          /* call_map_callback     */
    SGMLInheritMakeVisible,              /* make_visible          */
    NULL,                                /* extension             */
    },
    {
    NULL,                                /* ignore                */
    }
};


WidgetClass sGMLPlainTextObjectClass = (WidgetClass) &sGMLPlainTextClassRec;

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLTextObject request, new;
{
}

/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory alocated              */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLTextObject w;
{
}

/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region)
SGMLPlainTextObject t;
XEvent *event;
Region region;
{
   Position y = t->sgml_text.begin.y;
   Position margin = t->sgml_text.margin;
   Dimension width  = t->sgml_text.width;
   Dimension height = t->sgml_text.height;

   if(XRectInRegion(region,margin,y,width,height) != RectangleOut)
   {
      Display *dpy = XtDisplayOfObject((Widget) t);
      Window  wind = XtWindowOfObject((Widget) t);
      GC gc =  t->sgml_text.gc;
      int tabs = t->sgml_plain_text.tabs;

      Position x = t->sgml_text.begin.x;
      Dimension depth = t->sgml_text.depth;
      Dimension ascent = t->sgml_text.ascent;
      char *p = t->sgml_text.text;
      char *line = p;

      for (;; p++)
        {
          if (*p == '\n' || *p == '\0') 
            { 
               if (p != line && XRectInRegion(region,x,y,width,depth) != RectangleOut)
                 if (tabs)
                   {
                     int nt = 0, pos=0;
                     char *q, *copy;

                     for (q = line; q < p; q++) if (*q == '\t') nt++;
        
                     copy = XtMalloc(p-line + nt * tabs);
                     
                     for (q = line; q < p; q++)
                       if (*q != '\t') copy[pos++] = *q;
                       else for ( copy[pos++] = ' '; pos%tabs ; ) copy[pos++] = ' ';
                         
                     XDrawString(dpy, wind,gc, x, y+ascent, copy, pos);
                     XtFree(copy); 
                   } 
                 else XDrawString(dpy, wind,gc, x, y+ascent, line, p-line); 

               if (*p == '\0') break;
               
               x = margin;  
               y += depth;
               line = p+1;
            }
        }
   }
}

/*--------------------------------------------------------------*/
/* Compute Size:                                                */
/*--------------------------------------------------------------*/

static void ComputeSize(w,geom)
SGMLTextObject w;
SGMLGeometry *geom;
{
   Dimension left_clearance  = w->sgml_text.left_margin;
   int nlines;

   /*
    * Break before?
    */  

   if ( w->sgml_text.break_before )
   {
     _SGMLBreak(geom,w->sgml_text.space_before);
   } 

   if (w->sgml_text.depth  > geom->coord.depth) geom->coord.depth =  w->sgml_text.depth;
   if (left_clearance > geom->coord.x    ) geom->coord.x = left_clearance;

   w->sgml_text.begin  = geom->coord;

   nlines = LineSize(w,&geom->coord.x,&geom->actual_width);

   w->sgml_text.height = w->sgml_text.depth  * nlines;   
   w->sgml_text.margin = left_clearance;
   w->sgml_text.width = geom->actual_width - left_clearance;
   
   geom->coord.y += w->sgml_text.height -  w->sgml_text.depth;
   geom->coord.depth  = w->sgml_text.depth;
   geom->space = 0;
   geom->broken = FALSE;

   w->sgml_text.end = geom->coord;

   /*
    * Break after?
    */  

   if ( w->sgml_text.break_after )
   {
     _SGMLBreak(geom,w->sgml_text.space_after);
   } 
}

/*-----------------------------------------------------------------------*/
/* Private routines                                                      */
/*-----------------------------------------------------------------------*/
/* LineSize: In the plain text case, just count the newlines             */
/*-----------------------------------------------------------------------*/

static int LineSize(w, inoutx, inoutwidth)
SGMLPlainTextObject w;
Position *inoutx;
Dimension *inoutwidth; 
{
  Position x = *inoutx;
  Dimension width, maxWidth = *inoutwidth;
  Dimension left_clearance  = w->sgml_text.left_margin;
  int nlines = 1;
  int tabs = w->sgml_plain_text.tabs;
  char *p = w->sgml_text.text;
  int size = 0;

  for (; *p != '\0'; p++)
    {
       if (*p == '\n') 
         {
           width = size * w->sgml_text.spacing;
           if (x+width > maxWidth) maxWidth = x+width;
           nlines++;  
           size = 0;
           x = left_clearance; 
         }
       else if (*p == '\t' && tabs)  size += tabs - size%tabs; 
       else size++;
    }
 width = size * w->sgml_text.spacing;
 if (x+width > maxWidth) maxWidth = x+width;
 x += width; 

 *inoutx = x; 
 *inoutwidth = maxWidth;   
 return nlines;
}
/*-----------------------------------------------------------------------*/
/* Create a new SGMLPlainTextObject                                      */
/*-----------------------------------------------------------------------*/

Widget SGMLCreatePlainText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLPlainTextObjectClass,parent,al,ac);
}
