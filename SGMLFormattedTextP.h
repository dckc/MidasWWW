/*==================================================================*/
/*                                                                  */
/* SGMLFormattedTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLFORMATTEDTEXTP_H
#define SGMLFORMATTEDTEXTP_H

#include "SGMLFormattedText.h"
#include "SGMLTextP.h"

typedef struct _SGMLFormattedTextClassPart{

    int ignore;   /* no new class elements */

} SGMLFormattedTextClassPart;

typedef struct _SGMLFormattedTextClassRec{

    ObjectClassPart            object_class;
    SGMLTextClassPart          sgml_text_class;
    SGMLFormattedTextClassPart sgml_formatted_text_class;

} SGMLFormattedTextClassRec, *SGMLFormattedTextObjectClass;

extern SGMLFormattedTextClassRec sGMLFormattedTextClassRec;

typedef struct _WordInfo {

    char      *word;
    int       length;
    Dimension size;

} WordInfo;

typedef struct _LineInfo {

    int       start;
    int       stop;
    int       nchars; 
    Dimension size;

} LineInfo;

typedef struct _SGMLFormattedTextPart {

    WordInfo           *word_info;
    LineInfo           *line_info;
    int                 nwords;
    int                 nlines;  
    Dimension           left_indent;    /* left margin */
    Dimension           right_indent;   /* right margin */   

} SGMLFormattedTextPart;

typedef struct _SGMLFormattedTextRec {
    ObjectPart             object;
    SGMLTextPart           sgml_text;
    SGMLFormattedTextPart  sgml_formatted_text;
} SGMLFormattedTextRec;

#endif SGMLFORMATTEDTEXTP_H
