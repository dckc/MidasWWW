/*==================================================================*/
/*                                                                  */
/* SGMLMarkerTextObject                                             */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a marker text segment for the SGMLHyper widget           */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLMarkerTextP.h"

/* 
  Private functions 
*/


/*
  Widget class methods
*/

static void ComputeSize();

#define Offset(field) XtOffsetOf(SGMLMarkerTextRec,sgml_marker_text.field)

static XtResource resources[] = {

    {SGMLNcolumnAlign,SGMLCColumnAlign,XtRDimension,sizeof(Dimension),
    Offset (column_align),XtRImmediate,(XtPointer)0},

    {SGMLNposition,SGMLCPosition,XtRPosition,sizeof(Position),
    Offset (position),XtRImmediate,(XtPointer)-1},

};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLMarkerTextClassRec  sGMLMarkerTextClassRec = {
    {
    (WidgetClass) &sGMLTextClassRec,     /* superclass            */
    "SGMLMarkerText",                    /* class_name            */
    sizeof(SGMLMarkerTextRec),           /* widget_size           */
    NULL,                                /* class_initialize      */
    NULL,                                /* class_part_initialize */
    FALSE,                               /* class_inited          */
    NULL,                                /* initialize            */
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
    NULL,	                         /* destroy               */
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
    ComputeSize,	    		 /* compute_size          */
    SGMLInheritAdjustSize,               /* adjust_size           */
    SGMLInheritAdjustPosition,   	 /* adjust_position       */
    SGMLInheritExpose,                   /* expose                */
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


WidgetClass sGMLMarkerTextObjectClass = (WidgetClass) &sGMLMarkerTextClassRec;

/*--------------------------------------------------------------*/
/* Compute Size:                                                */
/*--------------------------------------------------------------*/
static void ComputeSize(w,geom)
SGMLMarkerTextObject w;
SGMLGeometry *geom;
{
  Dimension left_clearance  = w->sgml_text.left_margin  + w->sgml_text.left_indent;
  Dimension right_clearance = w->sgml_text.right_margin + w->sgml_text.right_indent;

   /*
    * Break?
    */  

  if (w->sgml_text.break_before || w->sgml_text.break_after)
  {
    _SGMLBreak(geom,w->sgml_text.space_before);
    _SGMLBreak(geom,w->sgml_text.space_after);
  } 
  else if (w->sgml_marker_text.column_align)
  {
    Dimension x = geom->coord.x;
    
    if (x < left_clearance) x = 0;
    else x -= left_clearance;
 
    if (x%w->sgml_marker_text.column_align)
      x += w->sgml_marker_text.column_align - x%w->sgml_marker_text.column_align;
  
    x += left_clearance;
    geom->coord.x = x; 
    geom->leave_space = FALSE;

    if (x + w->sgml_marker_text.column_align + right_clearance > 
        geom->natural_width)
    {
      _SGMLBreak(geom,0);
    }             
  }
  if (w->sgml_marker_text.position >= 0)
  {
    if (geom->coord.x > w->sgml_text.left_margin+w->sgml_marker_text.position)
      {
        _SGMLBreak(geom,0);
      }
    geom->coord.x = w->sgml_text.left_margin+w->sgml_marker_text.position;
    geom->leave_space = FALSE;
    geom->broken = FALSE;
  }
}

/*-----------------------------------------------------------------------*/
/* Create a new SGMLMarkerTextObject                                      */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateMarkerText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLMarkerTextObjectClass,parent,al,ac);
}
