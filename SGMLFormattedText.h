/*==================================================================*/
/*                                                                  */
/* SGMLFormattedTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a formatted  text segment for the SGMLText widget         */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLFORMATTEDTEXT_H
#define  SGMLFORMATTEDTEXT_H
#include "SGMLText.h"

extern WidgetClass sGMLFormattedTextObjectClass;
typedef struct _SGMLFormattedTextRec  *SGMLFormattedTextObject;

#ifdef _NO_PROTO

extern Widget  CreateSGMLFormattedText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateFormattedText(Widget parent,
        char *name,
        ArgList al,
        int ac);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsFormattedText(w)     XtIsSubclass(w,sGMLFormattedTextObjectClass)

#endif SGMLFORMATTEDTEXT_H

