/*==================================================================*/
/*                                                                  */
/* SGMLCompositeTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a composite  text segment for the SGMLText widget         */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLCOMPOSITETEXT_H
#define  SGMLCOMPOSITETEXT_H
#include "SGMLText.h"

#define SGMLNtagList         "tagList"
#define SGMLCTagList         "TagList"
#define SGMLRTagList         "TagList"
#define SGMLNdefaultClass    "defaultClass"
#define SGMLCClass           "Class"
#define SGMLRClass           "Class"

#define SGMLINHERIT_CLASS NULL 

extern WidgetClass sGMLCompositeTextObjectClass;
typedef struct _SGMLCompositeTextRec  *SGMLCompositeTextObject;

typedef struct _TagList {

      XrmName     name; 
      WidgetClass class;

} SGMLTagList;

#ifdef _NO_PROTO

extern void    SGMLCompositeTextInsertChild();
extern Widget  CreateSGMLCompositeText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateCompositeText(Widget parent,
        char *name,
        ArgList al,
        int ac);

    extern void SGMLCompositeTextInsertChild(Widget parent, Widget child);
    
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsCompositeText(w)     XtIsSubclass(w,sGMLCompositeTextObjectClass)

#endif SGMLCOMPOSITETEXT_H

