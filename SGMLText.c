/*==================================================================*/
/*                                                                  */
/* SGMLTextObject                                                   */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLTextP.h"
#include "SGMLHyperP.h"

/* 
  Private functions 
*/

#define max(a,b) (a>b?a:b)
void _SGMLBreak(); 

/*
  Widget class methods
*/

static void    ClassPartInitialize();
static void    Initialize();
static void    ClassInitialize();
static void    Destroy();
static Boolean SetValues();
static void    ComputeSize();
static Boolean AdjustSize();
static void    AdjustPosition();
static void    Redisplay();
static void    Activate();
static void    Hilite();
static Widget  Contains();
static void    CallCreateCallback();
static void    CallMapCallback();
static void    MakeVisible();

#define Offset(field) XtOffsetOf(SGMLTextRec,sgml_text.field)

static XtResource resources[] = {

    {SGMLNtext, SGMLCText, XtRString, sizeof(String),
    Offset(text), XtRString, ""},

    {SGMLNparam, SGMLCParam, XtRString, sizeof(String),
    Offset(param), XtRString, ""},

    {SGMLNlength, SGMLCLength, XtRInt, sizeof(int),
    Offset(length), XtRImmediate, 0},

    {SGMLNparamLength, SGMLCParamLength, XtRInt, sizeof(int),
    Offset(param_length), XtRImmediate, 0},

    {SGMLNfont, SGMLCFont, XtRFontStruct, sizeof (XFontStruct *),
    Offset(normal_rendition.font), XtRString, SGMLINHERIT_FONT},

    {SGMLNcolor, SGMLCColor, XtRPixel, sizeof (Pixel),
    Offset(normal_rendition.color),XtRString, SGMLINHERIT_COLOR},

    {SGMLNoutline,SGMLCOutline,XtRBoolean,sizeof(Boolean),
    Offset (normal_rendition.outline),XtRImmediate,(XtPointer)SGMLINHERIT_OUTLINE},

    {SGMLNunderline,SGMLCUnderline,XtRInt,sizeof(int),
    Offset (normal_rendition.underline),XtRImmediate,(XtPointer)SGMLINHERIT_UNDERLINE},

    {SGMLNunderlineHeight, SGMLCUnderlineHeight, XtRDimension, sizeof(Dimension),
    Offset (normal_rendition.underline_height),XtRImmediate,(XtPointer)SGMLINHERIT_UNDERLINEHEIGHT},

    {SGMLNsensitiveFont, SGMLCFont, XtRFontStruct, sizeof (XFontStruct *),
    Offset(sensitive_rendition.font), XtRString, SGMLINHERIT_FONT},

    {SGMLNsensitiveColor, SGMLCColor, XtRPixel, sizeof (Pixel),
    Offset(sensitive_rendition.color),XtRString, SGMLINHERIT_COLOR},

    {SGMLNsensitiveOutline,SGMLCOutline,XtRBoolean,sizeof(Boolean),
    Offset (sensitive_rendition.outline),XtRImmediate,(XtPointer)SGMLINHERIT_OUTLINE},

    {SGMLNsensitiveUnderline,SGMLCUnderline,XtRInt,sizeof(int),
    Offset (sensitive_rendition.underline),XtRImmediate,(XtPointer)SGMLINHERIT_UNDERLINE},

    {SGMLNsensitiveUnderlineHeight, SGMLCUnderlineHeight, XtRDimension, sizeof(Dimension),
    Offset (sensitive_rendition.underline_height),XtRImmediate,(XtPointer)SGMLINHERIT_UNDERLINEHEIGHT},

    {SGMLNbreakBefore,SGMLCBreak,SGMLRBreak,sizeof(int),
    Offset (break_before),XtRImmediate,(XtPointer)FALSE},

    {SGMLNbreakAfter,SGMLCBreak,SGMLRBreak,sizeof(int),
    Offset (break_after),XtRImmediate,(XtPointer)FALSE},

    {SGMLNsensitive,SGMLCSensitive,XtRBoolean,sizeof(Boolean),
    Offset (sensitive),XtRImmediate,(XtPointer)SGMLINHERIT_SENSITIVITY},

    {SGMLNrightMargin,SGMLCMargin,XtRInt,sizeof(int),
    Offset (right_margin),XtRImmediate,(XtPointer)SGMLINHERIT_MARGIN},

    {SGMLNleftMargin,SGMLCMargin,XtRInt,sizeof(int),
    Offset (left_margin),XtRImmediate,(XtPointer)SGMLINHERIT_MARGIN},

    {SGMLNrightIndent,SGMLCMargin,XtRInt,sizeof(int),
    Offset (right_indent),XtRImmediate,(XtPointer)0},

    {SGMLNleftIndent,SGMLCMargin,XtRInt,sizeof(int),
    Offset (left_indent),XtRImmediate,(XtPointer)0},

    {SGMLNparagraphIndent,SGMLCMargin,XtRInt,sizeof(int),
    Offset (paragraph_indent),XtRImmediate,(XtPointer)SGMLINHERIT_PARAGRAPHINDENT},

    {SGMLNalignment,SGMLCAlignment,XtRInt,sizeof(int),
    Offset (alignment),XtRImmediate,(XtPointer)SGMLINHERIT_ALIGNMENT},

    {SGMLNspacing,SGMLCSpacing,XtRDimension,sizeof(Dimension),
    Offset (spacing),XtRImmediate,(XtPointer)0},

    {SGMLNspaceBefore,SGMLCSpacing,XtRDimension,sizeof(Dimension),
    Offset (space_before),XtRImmediate,0},

    {SGMLNspaceAfter,SGMLCSpacing,XtRDimension,sizeof(Dimension),
    Offset (space_after),XtRImmediate,0},

    {SGMLNdepth,SGMLCDepth,XtRDimension,sizeof(Dimension),
    Offset (depth),XtRImmediate,(XtPointer)0},

    {SGMLNascent,SGMLCAscent,XtRDimension,sizeof(Dimension),
    Offset (ascent),XtRImmediate,(XtPointer)0},

    {SGMLNuserdata,SGMLCUserdata,XtRPointer,sizeof(XtPointer),
    Offset (userdata),XtRImmediate,(XtPointer)0},

    {SGMLNactivateCallback,SGMLCCallback,XtRCallback,sizeof(caddr_t),
    Offset (activate),XtRCallback,NULL},

    {SGMLNmapCallback,SGMLCCallback,XtRCallback,sizeof(caddr_t),
    Offset (map),XtRCallback,NULL},

     {SGMLNcreateCallback, SGMLCCallback, XtRCallback, sizeof(caddr_t),
    Offset (create),XtRCallback,NULL} 
    
};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLTextClassRec  sGMLTextClassRec = {
    {
    (WidgetClass) &objectClassRec,       /* superclass            */
    "SGMLText",                          /* class_name            */
    sizeof(SGMLTextRec),                 /* widget_size           */
    ClassInitialize,                     /* class_initialize      */
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
    SetValues,                           /* set_values            */
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
    AdjustSize,          		 /* adjust_size           */
    AdjustPosition,          		 /* adjust_position       */
    Redisplay,                           /* expose                */
    Activate,                            /* activate              */
    Hilite,                              /* hilite                */
    Contains,                            /* contains              */
    CallCreateCallback,                  /* call_create_callback  */ 
    CallMapCallback,                     /* call_map_callback     */ 
    MakeVisible,                         /* make_visible          */
    NULL,                                /* extension             */
    }
};

WidgetClass sGMLTextObjectClass = (WidgetClass) &sGMLTextClassRec;

static void CvtStringToBreak (args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    char *s;
    static int flag;
       
    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtStringToBreak","XtToolkitError",
                  "String to Break conversion needs no extra arguments",
                  (String *) NULL, (Cardinal *)NULL);

    s = (char *) fromVal->addr;
    flag = SGMLBREAK_NEVER;

    if      (!strcmp(s,"SGMLBREAK_NEVER" )) flag = SGMLBREAK_NEVER; 
    else if (!strcmp(s,"SGMLBREAK_ALWAYS")) flag = SGMLBREAK_ALWAYS; 
    else if (!strcmp(s,"SGMLBREAK_SOFT"))   flag = SGMLBREAK_SOFT; 
    else XtStringConversionWarning((char *) fromVal->addr, SGMLRBreak);

    toVal->size = sizeof(flag);
    toVal->addr = (XtPointer) &flag;
}

/*--------------------------------------------------------------*/
/* ClassInitialize: Register the converters                     */
/*--------------------------------------------------------------*/

static void ClassInitialize()
{
   XtAddConverter (XtRString, SGMLRBreak, CvtStringToBreak,
 	          (XtConvertArgList) NULL, (Cardinal) 0);
}
/*--------------------------------------------------------------*/
/* SetFontInfo: (private function)                              */
/*--------------------------------------------------------------*/
static void SetFontInfo(new)
SGMLTextObject new;
{
    /* if possible we will inherit graphic contexts from our parent */ 

    SGMLTextObject parent = (SGMLTextObject) XtParent((Widget) new);
    SGMLRendition  *rendition, *parent_rendition;
    int dir, ascent, descent;
    XCharStruct charinfo;

    if (parent->sgml_text.sensitive) parent_rendition = &parent->sgml_text.sensitive_rendition;
    else                             parent_rendition = &parent->sgml_text.normal_rendition;

    if (new->sgml_text.sensitive) rendition = &new->sgml_text.sensitive_rendition;
    else                          rendition = &new->sgml_text.normal_rendition;

    if (parent_rendition->color       == rendition->color &&
        parent_rendition->font        == rendition->font  &&
        parent_rendition->underline_height  
                                      == rendition->underline_height )
      {    

        new->sgml_text.gc        = parent->sgml_text.gc;
        new->sgml_text.invert_gc = parent->sgml_text.invert_gc;
        new->sgml_text.own_gcs   = FALSE;
      }
    else
      {
        XGCValues values;
        XtGCMask  valueMask;
        Pixel background, color; 
        Widget w;
        Visual *Vis; 
               
        for (w=XtParent((Widget) new); !XtIsWidget(w); w= XtParent(w));
        background = w->core.background_pixel; 
        
        valueMask = GCForeground | GCFont | GCLineWidth | GCLineStyle;
  
        if (DefaultDepthOfScreen(XtScreen(w))==1) color = 1 - background;
        else color = rendition->color;

        values.foreground = color;
        values.font       = rendition->font->fid;
        values.line_width = rendition->underline_height;
        values.line_style = LineSolid;

        new->sgml_text.gc = XtGetGC ((Widget) new, valueMask, &values);
   
        valueMask = GCForeground | GCFunction;
        values.foreground = background ^ color;
        values.function   = GXxor;
        new->sgml_text.invert_gc = XtGetGC ((Widget) new, valueMask, &values);

        new->sgml_text.own_gcs   = TRUE;
    
      }
    XTextExtents(rendition->font," ",1,&dir,&ascent,&descent,&charinfo);

    new->sgml_text.ascent  = ascent;
    new->sgml_text.depth   = ascent + max(descent,2*max(rendition->underline_height,1)*
                                                    rendition->underline);
    new->sgml_text.spacing = charinfo.width;
}

/*--------------------------------------------------------------*/
/* FreeFontInfo: (private function)                             */
/*--------------------------------------------------------------*/
static void FreeFontInfo(new)
SGMLTextObject new;
{
    if (new->sgml_text.own_gcs)
      {
        XtReleaseGC((Widget) new, new->sgml_text.gc);
        XtReleaseGC((Widget) new, new->sgml_text.invert_gc);
        new->sgml_text.own_gcs = FALSE;
      }
}
/*--------------------------------------------------------------*/
/* ClassPartInitialize:                                         */
/*--------------------------------------------------------------*/

static void ClassPartInitialize (class)
SGMLTextObjectClass class;
{
   SGMLTextObjectClass superclass = (SGMLTextObjectClass) class->object_class.superclass; 
      
   if (class->sgml_text_class.compute_size == SGMLInheritComputeSize)
       class->sgml_text_class.compute_size = superclass->sgml_text_class.compute_size;

   if (class->sgml_text_class.adjust_size == SGMLInheritAdjustSize)
       class->sgml_text_class.adjust_size = superclass->sgml_text_class.adjust_size;

   if (class->sgml_text_class.adjust_position == SGMLInheritAdjustPosition)
       class->sgml_text_class.adjust_position = superclass->sgml_text_class.adjust_position;

  if (class->sgml_text_class.compute_size == SGMLInheritComputeSize)
       class->sgml_text_class.compute_size = superclass->sgml_text_class.compute_size;


   if (class->sgml_text_class.expose == SGMLInheritExpose)
       class->sgml_text_class.expose = superclass->sgml_text_class.expose;

   if (class->sgml_text_class.activate == SGMLInheritActivate)
       class->sgml_text_class.activate = superclass->sgml_text_class.activate;

   if (class->sgml_text_class.hilite == SGMLInheritHilite)
       class->sgml_text_class.hilite = superclass->sgml_text_class.hilite;

   if (class->sgml_text_class.contains == SGMLInheritContains)
       class->sgml_text_class.contains = superclass->sgml_text_class.contains;

   if (class->sgml_text_class.call_create_callback == SGMLInheritCallCreateCallback)
       class->sgml_text_class.call_create_callback = superclass->sgml_text_class.call_create_callback;

   if (class->sgml_text_class.call_map_callback == SGMLInheritCallMapCallback)
       class->sgml_text_class.call_map_callback = superclass->sgml_text_class.call_map_callback;

   if (class->sgml_text_class.make_visible == SGMLInheritMakeVisible)
       class->sgml_text_class.make_visible = superclass->sgml_text_class.make_visible;
}
/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLTextObject request, new;
{
   SGMLTextObject parent = (SGMLTextObject) XtParent((Widget) new);

   new->sgml_text.text   = new->sgml_text.text?XtNewString(new->sgml_text.text):NULL;
   new->sgml_text.length = new->sgml_text.text?strlen(new->sgml_text.text):0;

   new->sgml_text.param        = new->sgml_text.param?XtNewString(new->sgml_text.param):NULL;
   new->sgml_text.param_length = new->sgml_text.param?strlen(new->sgml_text.param):0;

   new->sgml_text.size_valid = FALSE;

    /*
     * Many quantities can be inherited from our parent 
     */

    if (SGMLIsText((Widget) parent))
    {
       new->sgml_text.left_indent  +=  parent->sgml_text.left_indent;
       new->sgml_text.right_indent +=  parent->sgml_text.right_indent;

       if (new->sgml_text.left_margin == SGMLINHERIT_MARGIN)
         new->sgml_text.left_margin = parent->sgml_text.left_margin;

       if (new->sgml_text.right_margin == SGMLINHERIT_MARGIN)
         new->sgml_text.right_margin = parent->sgml_text.right_margin;

       if (new->sgml_text.paragraph_indent == SGMLINHERIT_PARAGRAPHINDENT)
         new->sgml_text.paragraph_indent = parent->sgml_text.paragraph_indent;

       if (new->sgml_text.space_before == SGMLINHERIT_SPACE)
         new->sgml_text.space_before = parent->sgml_text.space_before;

       if (new->sgml_text.space_after == SGMLINHERIT_SPACE)
         new->sgml_text.space_after = parent->sgml_text.space_after;

       if (new->sgml_text.break_before == SGMLINHERIT_BREAK)
         new->sgml_text.break_before = parent->sgml_text.break_before;

       if (new->sgml_text.break_after == SGMLINHERIT_BREAK)
         new->sgml_text.break_after = parent->sgml_text.break_after;

       if (new->sgml_text.alignment == SGMLINHERIT_ALIGNMENT)
         new->sgml_text.alignment = parent->sgml_text.alignment;

       if (new->sgml_text.sensitive == SGMLINHERIT_SENSITIVITY)
         new->sgml_text.sensitive = parent->sgml_text.sensitive;

       if (new->sgml_text.normal_rendition.outline == SGMLINHERIT_OUTLINE)
         new->sgml_text.normal_rendition.outline = parent->sgml_text.normal_rendition.outline;

       if (new->sgml_text.normal_rendition.underline == SGMLINHERIT_UNDERLINE)
         new->sgml_text.normal_rendition.underline = parent->sgml_text.normal_rendition.underline;

       if (new->sgml_text.normal_rendition.underline_height == SGMLINHERIT_UNDERLINEHEIGHT)
         new->sgml_text.normal_rendition.underline_height = parent->sgml_text.normal_rendition.underline_height;

       if (new->sgml_text.normal_rendition.font == SGMLINHERIT_FONT)
         new->sgml_text.normal_rendition.font = parent->sgml_text.normal_rendition.font;

       if (new->sgml_text.normal_rendition.color == SGMLINHERIT_COLOR)
         new->sgml_text.normal_rendition.color = parent->sgml_text.normal_rendition.color;

       if (new->sgml_text.sensitive_rendition.outline == SGMLINHERIT_OUTLINE)
         new->sgml_text.sensitive_rendition.outline = parent->sgml_text.sensitive_rendition.outline;

       if (new->sgml_text.sensitive_rendition.underline == SGMLINHERIT_UNDERLINE)
         new->sgml_text.sensitive_rendition.underline = parent->sgml_text.sensitive_rendition.underline;

       if (new->sgml_text.sensitive_rendition.underline_height == SGMLINHERIT_UNDERLINEHEIGHT)
         new->sgml_text.sensitive_rendition.underline_height = parent->sgml_text.sensitive_rendition.underline_height;

       if (new->sgml_text.sensitive_rendition.font == SGMLINHERIT_FONT)
         new->sgml_text.sensitive_rendition.font = parent->sgml_text.sensitive_rendition.font;
 
       if (new->sgml_text.sensitive_rendition.color == SGMLINHERIT_COLOR)
         new->sgml_text.sensitive_rendition.color = parent->sgml_text.sensitive_rendition.color;
    }
    else
    {
       if (new->sgml_text.left_margin == SGMLINHERIT_MARGIN)
         new->sgml_text.left_margin = ((SGMLHyperWidget) parent)->sgml_hyper.margin;

       if (new->sgml_text.right_margin == SGMLINHERIT_MARGIN)
         new->sgml_text.right_margin = ((SGMLHyperWidget) parent)->sgml_hyper.margin;

       if (new->sgml_text.paragraph_indent == SGMLINHERIT_PARAGRAPHINDENT)
         new->sgml_text.paragraph_indent = 0;

       if (new->sgml_text.space_before == SGMLINHERIT_SPACE)
         new->sgml_text.space_before = 0;
         
       if (new->sgml_text.space_after == SGMLINHERIT_SPACE)
         new->sgml_text.space_after = 0;

       if (new->sgml_text.break_before == SGMLINHERIT_BREAK)
         new->sgml_text.break_before = FALSE;

       if (new->sgml_text.break_after == SGMLINHERIT_BREAK)
         new->sgml_text.break_after = FALSE;

       if (new->sgml_text.alignment == SGMLINHERIT_ALIGNMENT)
         new->sgml_text.alignment = SGMLALIGNMENT_LEFT;

       if (new->sgml_text.sensitive == SGMLINHERIT_SENSITIVITY)
         new->sgml_text.sensitive = FALSE;

       if (new->sgml_text.normal_rendition.outline == SGMLINHERIT_OUTLINE)
         new->sgml_text.normal_rendition.outline = FALSE;

       if (new->sgml_text.normal_rendition.underline == SGMLINHERIT_UNDERLINE)
         new->sgml_text.normal_rendition.underline = 0;

       if (new->sgml_text.normal_rendition.underline_height == SGMLINHERIT_UNDERLINEHEIGHT)
         new->sgml_text.normal_rendition.underline_height = 0;

       if (new->sgml_text.normal_rendition.font == SGMLINHERIT_FONT)
         new->sgml_text.normal_rendition.font = XLoadQueryFont(XtDisplayOfObject((Widget) new),"fixed");

       if (new->sgml_text.sensitive_rendition.outline == SGMLINHERIT_OUTLINE)
         new->sgml_text.sensitive_rendition.outline = FALSE;

       if (new->sgml_text.sensitive_rendition.underline == SGMLINHERIT_UNDERLINE)
         new->sgml_text.sensitive_rendition.underline = 0;

       if (new->sgml_text.sensitive_rendition.underline_height == SGMLINHERIT_UNDERLINEHEIGHT)
         new->sgml_text.sensitive_rendition.underline_height = 0;

       if (new->sgml_text.sensitive_rendition.font == SGMLINHERIT_FONT)
         new->sgml_text.sensitive_rendition.font = XLoadQueryFont(XtDisplayOfObject((Widget) new),"fixed");
    }

  SetFontInfo(new);
}
/*--------------------------------------------------------------*/
/* Call the create Callback:                                    */
/*--------------------------------------------------------------*/

static void CallCreateCallback(w,event)
SGMLTextObject w;
XEvent         *event;
{
    SGMLTextCallbackStruct cb;

    cb.reason = SGMLTEXT_CREATE;
    cb.event = event;
    cb.text   = w->sgml_text.text;
    cb.length = w->sgml_text.length;
    cb.param  = w->sgml_text.param;
    cb.param_length = w->sgml_text.param_length;
  
    XtCallCallbacks((Widget) w,SGMLNcreateCallback,(XtPointer) &cb);
}
/*--------------------------------------------------------------*/
/* Call the map Callback:                                       */
/*--------------------------------------------------------------*/

static void CallMapCallback(w,event)
SGMLTextObject w;
XEvent         *event;
{
    SGMLTextCallbackStruct cb;

    cb.reason = SGMLTEXT_MAP;
    cb.event = event;
    cb.text   = w->sgml_text.text;
    cb.length = w->sgml_text.length;
    cb.param  = w->sgml_text.param;
    cb.param_length = w->sgml_text.param_length;
  
    XtCallCallbacks((Widget) w,SGMLNmapCallback,(XtPointer) &cb);
}
/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory alocated              */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLTextObject w;
{
    FreeFontInfo(w);

    XtFree(w->sgml_text.text);
    XtFree(w->sgml_text.param);
    XtRemoveAllCallbacks ((Widget) w,SGMLNactivateCallback);
    XtRemoveAllCallbacks ((Widget) w,SGMLNcreateCallback);
}

/*------------------------------------------------------------------*/
/* SetValues : redraw only for font or color changes                */
/*------------------------------------------------------------------*/

static Boolean SetValues (current, request, new)
SGMLTextObject current, request, new;
{
    Boolean    redraw = FALSE;

#define HAS_CHANGED(a)    (new->sgml_text.a != current->sgml_text.a)

    if (HAS_CHANGED(text))
      {
        XtFree(current->sgml_text.text);
        new->sgml_text.text   = new->sgml_text.text?XtNewString(new->sgml_text.text):NULL;
        new->sgml_text.length = new->sgml_text.text?strlen(new->sgml_text.text):0;
      }

    if (HAS_CHANGED(param))
      {
        XtFree(current->sgml_text.param);
        new->sgml_text.param  = new->sgml_text.param?XtNewString(new->sgml_text.param):NULL;
        new->sgml_text.length = new->sgml_text.param?strlen(new->sgml_text.param):0;
      }

    if (HAS_CHANGED(sensitive))
      {
          FreeFontInfo(new);
          SetFontInfo(new);
          redraw = TRUE;
      } 
    else if (new->sgml_text.sensitive)
      {
        if (HAS_CHANGED(sensitive_rendition.color) || 
            HAS_CHANGED(sensitive_rendition.font)  || 
            HAS_CHANGED(sensitive_rendition.underline_height))
        {
          FreeFontInfo(new);
          SetFontInfo(new);
          redraw = TRUE;
        }
      }  
    else 
      {
        if (HAS_CHANGED(normal_rendition.color) || 
            HAS_CHANGED(normal_rendition.font)  || 
            HAS_CHANGED(normal_rendition.underline_height))
        {
          FreeFontInfo(new);
          SetFontInfo(new);
          redraw = TRUE;
        }
      } 
    return redraw;

#undef HAS_CHANGED

}

/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region)
SGMLTextObject t;
XEvent *event;
Region region;
{
}

/*--------------------------------------------------------------*/
/* Compute Size:                                                */
/*--------------------------------------------------------------*/

static void ComputeSize(w,geom,Adjust,Closure)
SGMLTextObject w;
SGMLGeometry *geom;
AdjustSizeProc Adjust;
Opaque Closure;
{
}

/*--------------------------------------------------------------*/
/* Adjust Size:                                                 */
/*--------------------------------------------------------------*/

static Boolean AdjustSize(w,y,ascent,depth)
SGMLTextObject w;
Position y;
Dimension ascent, depth;
{
  return TRUE;
}

/*--------------------------------------------------------------*/
/* Adjust Position:                                             */
/*--------------------------------------------------------------*/

static void AdjustPosition(w)
SGMLTextObject w;
{
}

/*--------------------------------------------------------------*/
/* Hilite:                                                      */
/*--------------------------------------------------------------*/

static void Hilite(w,state)
SGMLTextObject w;
Boolean state;
{
}

/*--------------------------------------------------------------*/
/* MakeVisible:                                                 */
/*--------------------------------------------------------------*/

static void MakeVisible(w)
SGMLTextObject w;
{
}

/*--------------------------------------------------------------*/
/* Activate:                                                    */
/*--------------------------------------------------------------*/

static void Activate(w,event)
SGMLTextObject w;
XEvent *event;
{
    SGMLTextCallbackStruct cb;

    /* Fill callback struct */

    cb.text          = w->sgml_text.text;
    cb.length        = w->sgml_text.length;
    cb.param         = w->sgml_text.param;
    cb.param_length  = w->sgml_text.param_length;
    cb.reason        = SGMLTEXT_ACTIVATE;
    cb.event         = event;

    XtCallCallbacks ((Widget) w, SGMLNactivateCallback,(XtPointer) &cb);
}

/*--------------------------------------------------------------*/
/* Contains:                                                    */
/*--------------------------------------------------------------*/

static Widget Contains(w,x,y)
SGMLTextObject w;
Position x,y;
{
  return NULL;
}

/*--------------------------------------------------------------*/
/* _SGMLBreak:                                                  */
/*--------------------------------------------------------------*/

void _SGMLBreak(geom,space)
SGMLGeometry *geom;
Dimension space; 
{
  if (!geom->broken)
  {
    geom->coord.y += geom->coord.depth;
    geom->coord.y += space;
    geom->space = space;
    geom->broken = TRUE;
  }
  else
  {
    if (space > geom->space) 
    {  
      geom->coord.y += space - geom->space;
      geom->space = space;
    }
  }
  geom->coord.x = 0;
  geom->coord.depth = 0;
  geom->coord.ascent = 0;
  geom->alignment = SGMLALIGNMENT_NONE; 
  geom->leave_space = FALSE; 
}
/*-----------------------------------------------------------------------*/
/* Create a new SGMLTextObject                                           */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLTextObjectClass,parent,al,ac);
}
