/*==================================================================*/
/*                                                                  */
/* SGMLListTextObject                                               */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLHyper widget                  */
/*                                                                  */
/*==================================================================*/

#ifndef SGMLLISTTEXTP_H
#define SGMLLISTTEXTP_H

#include "SGMLListText.h"
#include "SGMLCompositeTextP.h"

typedef struct _SGMLListTextClassPart{

   XtPointer	      extension;	  /* pointer to extension record     */

} SGMLListTextClassPart;

typedef struct _SGMLListTextClassRec{

    ObjectClassPart            object_class;
    SGMLTextClassPart          sgml_text_class;
    SGMLCompositeTextClassPart sgml_composite_text_class;
    SGMLListTextClassPart      sgml_list_text_class;

} SGMLListTextClassRec, *SGMLListTextObjectClass;

extern SGMLListTextClassRec sGMLListTextClassRec;

typedef struct _BulletList
{
    Position    x;
    Position    y;

} BulletListItem, *BulletList;

typedef struct _SGMLListTextPart {

    BulletList  bullets;	     /* pointer to array of bullets	     */
    Cardinal    num_bullets;	     /* total number of bullets 	     */
    Cardinal    alloc_bullets;	     /* total number of bullets allocated    */
    int         bullet_type;         /* type of bullets 		     */
    Dimension   bullet_size;         /* size of bullets                      */
    Pixel       bullet_color;        /* color of bullets                     */ 
    Dimension   bullet_indent;       /* indentation of bullet                */
     
} SGMLListTextPart;

typedef struct _SGMLListTextRec {
    ObjectPart             object;
    SGMLTextPart           sgml_text;
    SGMLCompositeTextPart  sgml_composite_text;
    SGMLListTextPart       sgml_list_text;
} SGMLListTextRec;

#endif SGMLLISTTEXTP_H
