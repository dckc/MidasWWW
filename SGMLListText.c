/*==================================================================*/
/*                                                                  */
/* SGMLListTextObject                                               */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a list text segment for the SGMLHyper widget             */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "SGMLListTextP.h"

/* 
  Private functions 
*/


/*
  Widget class methods
*/

static void         	ClassInitialize();
static void    		Initialize();
static void    		Destroy();
static void             ComputeSize();
static Boolean          AdjustSize();
static void             Redisplay();

#define Offset(field) XtOffsetOf(SGMLListTextRec,sgml_list_text.field)

static XtResource resources[] = {

    {SGMLNnumBullets, XtCReadOnly, XtRInt, sizeof(int),
     Offset(num_bullets), XtRImmediate, (XtPointer) 0},
    {SGMLNallocBullets, XtCReadOnly, XtRInt, sizeof(int),
     Offset(alloc_bullets), XtRImmediate, (XtPointer) 0},

    {SGMLNbulletType, SGMLCBulletType, SGMLRBulletType, sizeof(int),
     Offset(bullet_type), XtRImmediate, (XtPointer) SGMLBULLET_DEFAULT},
    {SGMLNbulletSize, SGMLCBulletSize, XtRDimension, sizeof(Dimension),
     Offset(bullet_size), XtRImmediate, (XtPointer) 10},
    {SGMLNbulletIndent, SGMLCBulletIndent, XtRDimension, sizeof(Dimension),
     Offset(bullet_indent), XtRImmediate, (XtPointer) 5},
    {SGMLNbulletColor, SGMLCBulletColor, XtRPixel, sizeof(Pixel),
     Offset(bullet_color), XtRImmediate, (XtPointer) 0}
};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLListTextClassRec  sGMLListTextClassRec = {
    {
    (WidgetClass) &sGMLCompositeTextClassRec,
                                         /* superclass            */
    "SGMLListText",                      /* class_name            */
    sizeof(SGMLListTextRec),             /* widget_size           */
    ClassInitialize,                     /* class_initialize      */
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
    NULL,                                /* geometry_manager      */
    NULL,                                /* change_managed        */
    XtInheritInsertChild,                /* insert_child          */
    XtInheritDeleteChild,                /* delete_child          */
    NULL,                                /* ignore                */
    },
    {
    NULL                                 /* ignore                */
    }
};

WidgetClass sGMLListTextObjectClass = (WidgetClass) &sGMLListTextClassRec;

/*--------------------------------------------------------------*/
/* ClassInitialize:                                             */
/*--------------------------------------------------------------*/

static void ClassInitialize()
{
}

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLListTextObject request, new;
{
   new->sgml_list_text.bullets = NULL;
   new->sgml_list_text.num_bullets = 0;
   new->sgml_list_text.alloc_bullets = 0;
}

/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLListTextObject w;
{
    XtFree((char *) w->sgml_list_text.bullets);
}

/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region)
SGMLListTextObject t;
XEvent *event;
Region region;
{
/*
 *   Invoke our superclasses ReDisplay routine ... then draw bullets
 */ 
 
 Display *dpy = XtDisplayOfObject((Widget) t);
 Window  wind = XtWindowOfObject((Widget) t);
 GC gc =  t->sgml_text.gc;
 int i;
  
 SGMLTextObjectClass superclass = (SGMLTextObjectClass) XtSuperclass((Widget) t);
 
 (*superclass->sgml_text_class.expose)(t,event,region); 
  
 for (i = 0; i < t->sgml_list_text.num_bullets; i++ )
   {
     Dimension width = t->sgml_list_text.bullet_size;
     Dimension height = t->sgml_list_text.bullet_size;
     Position x = t->sgml_list_text.bullets[i].x - width;
     Position y = t->sgml_list_text.bullets[i].y;
     
     if(XRectInRegion(region,x,y,width,height) != RectangleOut)
       XFillRectangle(dpy,wind,gc,x,y,width,height);
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
   SGMLListTextObject w = (SGMLListTextObject) XtParent(child);
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
/* Compute Size:                                                */
/*--------------------------------------------------------------*/

static void ComputeSize(w,geom,Adjust,Closure)
SGMLListTextObject w;
SGMLGeometry *geom;
AdjustSizeProc Adjust;
Opaque Closure;
{
   int n = w->sgml_composite_text.num_children;
   WidgetList children = w->sgml_composite_text.children;
   Dimension maxWidth;
   Position x  = w->sgml_text.left_margin + w->sgml_text.left_indent - w->sgml_list_text.bullet_indent;

   XtFree((char *) w->sgml_list_text.bullets);
   w->sgml_list_text.bullets = NULL;
   w->sgml_list_text.num_bullets = 0;
   w->sgml_list_text.alloc_bullets = 0;
   
   /*
    * Break before?
    */  

   if ( w->sgml_text.break_before )
   {
     _SGMLBreak(geom,w->sgml_text.space_before);
   } 

   /*
    * Query children?
    */  

   for (; n-- > 0; children++)
     {
       SGMLTextObjectClass childClass = (SGMLTextObjectClass) XtClass(*children);

       if (geom->broken)
         {
           if (w->sgml_list_text.num_bullets == w->sgml_list_text.alloc_bullets)
             w->sgml_list_text.bullets = (BulletList) XtRealloc((char *)w->sgml_list_text.bullets,
                                                                (w->sgml_list_text.alloc_bullets += 10) * sizeof(BulletListItem));
             
           w->sgml_list_text.bullets[w->sgml_list_text.num_bullets].x = x;  
           w->sgml_list_text.bullets[w->sgml_list_text.num_bullets].y = geom->coord.y;  
           w->sgml_list_text.num_bullets++;

           geom->coord.depth  = w->sgml_list_text.bullet_size;
           geom->coord.ascent = w->sgml_list_text.bullet_size;
        }    

       (*childClass->sgml_text_class.compute_size)(*children,geom,AdjustSize,*children);
     }  

   /*
    * Break after?
    */  

   if ( w->sgml_text.break_after )
   {
     _SGMLBreak(geom,w->sgml_text.space_after);
   } 

}

/*-----------------------------------------------------------------------*/
/* Create a new SGMLListTextObject                                  */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateListText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLListTextObjectClass,parent,al,ac);
}
