/*==================================================================*/
/*                                                                  */
/* SGMLAnchorTextObject                                             */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLAnchorText widget             */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLANCHORTEXT_H
#define  SGMLANCHORTEXT_H
#include "SGMLFormattedText.h"

extern WidgetClass sGMLAnchorTextObjectClass;
typedef struct _SGMLAnchorTextRec  *SGMLAnchorTextObject;


#define SGMLNhref        "href"
#define SGMLCHref        "Href"
#define SGMLNname        "name"
#define SGMLCName        "Name"
#define SGMLNhrefLength  "hrefLength"
#define SGMLCHrefLength  "HrefLength"
#define SGMLNnameLength  "nameLength"
#define SGMLCNameLength  "NameLength"
#define SGMLNvisited     "visited"
#define SGMLCVisited     "Visited"

#define SGMLNvisitedFont            "visitedFont"
#define SGMLNvisitedColor           "visitedColor"
#define SGMLNvisitedUnderline       "visitedUnderline"
#define SGMLNvisitedOutline         "visitedOutline"
#define SGMLNvisitedUnderlineHeight "visitedUnderlineHeight"


#ifdef _NO_PROTO

extern Widget  CreateSGMLAnchorText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateAnchorText(Widget parent,
        char *name,
        ArgList al,
        int ac);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsAnchorText(w)     XtIsSubclass(w,sGMLAnchorTextObjectClass)

#endif SGMLANCHORTEXT_H
