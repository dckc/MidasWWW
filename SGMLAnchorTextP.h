/*==================================================================*/
/*                                                                  */
/* SGMLAnchorTextObject                                             */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a anchortext segment for the SGMLHyper widget            */   
/*                                                                  */
/*==================================================================*/

#ifndef SGMLANCHORTEXTP_H
#define SGMLANCHORTEXTP_H

#include "SGMLAnchorText.h"
#include "SGMLFormattedTextP.h"

typedef struct _SGMLAnchorTextClassPart{

  int ignore;  /* no new class elements */
  
} SGMLAnchorTextClassPart;

typedef struct _SGMLAnchorTextClassRec{

    ObjectClassPart     object_class;
    SGMLTextClassPart          sgml_text_class;
    SGMLFormattedTextClassPart sgml_formatted_text_class;
    SGMLAnchorTextClassPart   sgml_anchortext_class;

} SGMLAnchorTextClassRec, *SGMLAnchorTextObjectClass;

extern SGMLAnchorTextClassRec sGMLAnchorTextClassRec;

typedef struct _SGMLAnchorTextPart {

    String              href;    /* pointer to href */
    String              name;    /* pointer to name */
    int                 href_length;   /* length of href */
    int                 name_length ;  /* length of name */
    SGMLRendition       visited_rendition;
    Boolean             visited;

} SGMLAnchorTextPart;

typedef struct _SGMLAnchorTextRec {
    ObjectPart        object;
    SGMLTextPart           sgml_text;
    SGMLFormattedTextPart  sgml_formatted_text;
    SGMLAnchorTextPart     sgml_anchortext;
} SGMLAnchorTextRec;

#endif SGMLANCHORTEXTP_H
