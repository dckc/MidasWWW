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

#ifndef SGMLHYPERP_H
#define SGMLHYPERP_H
#include "SGMLHyper.h"

#ifdef MOTIF
#include <Xm/XmP.h>
#endif

/* SGMLHyper class : no new fields */

typedef struct _SGMLHyperClassPart{
    int ignore;
} SGMLHyperClassPart;

typedef struct _SGMLHyperClassRec{
    CoreClassPart       core_class;
    CompositeClassPart  composite_class; 
#ifdef MOTIF
    XmManagerClassPart  manager_class;
#endif
    SGMLHyperClassPart  sgml_hyper_class;
} SGMLHyperClassRec, *SGMLHyperWidgetClass;

extern SGMLHyperClassRec sGMLHyperClassRec;


typedef struct _SGMLHyperPart {

    Cursor    hand;                  /* Selecting cursor shape */
    Boolean   zoom;                  /* zoom effect when selected */
    int       speed;                 /* zoom speed                */
    char      open_tag;              /* start of highlighted text mark */
    char      close_tag;             /* end of highlighted text mark */
    char      end_tag;               /* character used to hide text in highlight */
    char      parameter_tag;         /* parameter tag */
    Boolean   case_sensitive_tags;   /* Case sensitive tags */
    Widget    managed;
    int       cache_size;            /* maximum number of cached text segments */

    Dimension margin;                /* margins size */
    Dimension natural_width;	     /* size it tries to fit text into */
    
/*
    text_segment         *grep_seg;  /* segment where found text is *
*/
    char                 *grep_txt;  /* pointer to found text */
    int                   grep_len;  /* length of found text */
    int                   grep_off;  /* offset of found text */

    Position          grep_x;        /* rectangle of found text*/
    Position          grep_y;
    Dimension         grep_width;
    Dimension         grep_height;

    SGMLTextObject    last_selected; /* last selected segment     */
    SGMLTextObject    last_cursor;   /* last under cursor segment */

    XtCallbackList    activate;      /* callback list             */

} SGMLHyperPart;

typedef struct _SGMLHyperRec {
    CorePart          core;
    CompositePart     composite; 
#ifdef MOTIF
    XmManagerPart     manager;
#endif
    SGMLHyperPart     sgml_hyper;
} SGMLHyperRec;

#endif SGMLHYPERP_H
