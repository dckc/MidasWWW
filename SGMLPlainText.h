/*==================================================================*/
/*                                                                  */
/* SGMLPlainTextObject                                              */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines aplain  text segment for the SGMLText widget             */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLPLAINTEXT_H
#define  SGMLPLAINTEXT_H
#include "SGMLText.h"

extern WidgetClass sGMLPlainTextObjectClass;
typedef struct _SGMLPlainTextRec  *SGMLPlainTextObject;

#define SGMLNtabs            "tabs"
#define SGMLCTabs            "Tabs"

#ifdef _NO_PROTO

extern Widget  CreateSGMLPlainText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreatePlainText(Widget parent,
        char *name,
        ArgList al,
        int ac);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsPlainText(w)     XtIsSubclass(w,sGMLPlainTextObjectClass)

#endif SGMLPLAINTEXT_H

