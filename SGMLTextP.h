/*==================================================================*/
/*                                                                  */
/* SGMLTextObject                                                   */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLTEXTP_H
#define SGMLTEXTP_H

#include "SGMLText.h"

/* SGMLText class */

typedef void    (*ComputeSizeProc)();
typedef Boolean (*AdjustSizeProc)();
typedef void    (*AdjustPositionProc)();
typedef void    (*ExposeProc)();
typedef void    (*ActivateProc)();
typedef void    (*HiliteProc)();
typedef Widget  (*ContainsProc)();
typedef void    (*CallCreateCallbackProc)();
typedef void    (*CallMapCallbackProc)();
typedef void    (*MakeVisibleProc)();

#define SGMLInheritComputeSize        ((ComputeSizeProc)        _XtInherit) 
#define SGMLInheritAdjustSize         ((AdjustSizeProc)         _XtInherit) 
#define SGMLInheritAdjustPosition     ((AdjustPositionProc)     _XtInherit) 
#define SGMLInheritExpose             ((ExposeProc)             _XtInherit) 
#define SGMLInheritActivate           ((ActivateProc)           _XtInherit) 
#define SGMLInheritHilite             ((HiliteProc)             _XtInherit) 
#define SGMLInheritContains           ((ContainsProc)           _XtInherit) 
#define SGMLInheritCallCreateCallback ((CallCreateCallbackProc) _XtInherit) 
#define SGMLInheritCallMapCallback    ((CallMapCallbackProc)    _XtInherit) 
#define SGMLInheritMakeVisible        ((MakeVisibleProc)        _XtInherit) 

typedef struct _SGMLTextClassPart{

   ComputeSizeProc        compute_size;
   AdjustSizeProc         adjust_size;
   AdjustPositionProc     adjust_position;
   ExposeProc             expose;
   ActivateProc           activate;
   HiliteProc             hilite;
   ContainsProc           contains;
   CallCreateCallbackProc call_create_callback;
   CallMapCallbackProc    call_map_callback;
   MakeVisibleProc        make_visible;
   XtPointer              extension;

} SGMLTextClassPart;

typedef struct _SGMLTextClassRec{

    ObjectClassPart     object_class;
    SGMLTextClassPart   sgml_text_class;

} SGMLTextClassRec, *SGMLTextObjectClass;

extern SGMLTextClassRec sGMLTextClassRec;

typedef struct _SGMLRendition {

    Pixel               color;
    Boolean             outline;                 
    int                 underline;
    Dimension           underline_height;
    XFontStruct         *font;

} SGMLRendition;

typedef struct _SGMLTextPart {

    String              text;    /* pointer to text */
    String              param;   /* pointer to parameters */
    int                 length;         /* length of text */
    int                 param_length ;  /* length of parameters */

    SGMLCoord           begin;
    SGMLCoord           end;
    Dimension           width,height;   /* Size of drawn text */
    Position            margin;

    int                 right_margin;
    int                 left_margin;
    int                 right_indent;
    int                 left_indent;
    int                 paragraph_indent;
    Dimension           space_before;
    Dimension           space_after;
    int                 break_before;
    int                 break_after;  
    int                 alignment;
    Boolean             sensitive;
    SGMLRendition       normal_rendition;
    SGMLRendition       sensitive_rendition;
    Dimension           ascent;
    Dimension           depth;
    Dimension           spacing;
    Boolean             size_valid;
    XtPointer           userdata;

    GC                  gc;
    GC                  invert_gc;
    Boolean             own_gcs;

    XtCallbackList      activate;       /* callback list */
    XtCallbackList      create;         /* callback list */
    XtCallbackList      map;            /* callback list */

} SGMLTextPart;

typedef struct _SGMLTextRec {
    ObjectPart        object;
    SGMLTextPart      sgml_text;
} SGMLTextRec;

#endif SGMLTEXTP_H
