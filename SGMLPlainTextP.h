/*==================================================================*/
/*                                                                  */
/* SGMLPlainTextObject                                              */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLPLAINTEXTP_H
#define SGMLPLAINTEXTP_H

#include "SGMLPlainText.h"
#include "SGMLTextP.h"

typedef struct _SGMLPlainTextClassPart{

    int ignore;   /* no new class elements */

} SGMLPlainTextClassPart;

typedef struct _SGMLPlainTextClassRec{

    ObjectClassPart        object_class;
    SGMLTextClassPart      sgml_text_class;
    SGMLPlainTextClassPart sgml_plain_text_class;

} SGMLPlainTextClassRec, *SGMLPlainTextObjectClass;

extern SGMLPlainTextClassRec sGMLPlainTextClassRec;

typedef struct _SGMLPlainTextPart {

    int tabs;  /* controls handling of tabs */ 

} SGMLPlainTextPart;

typedef struct _SGMLPlainTextRec {
    ObjectPart             object;
    SGMLTextPart           sgml_text;
    SGMLPlainTextPart      sgml_plain_text;
} SGMLPlainTextRec;

#endif SGMLPLAINTEXTP_H
