/*==================================================================*/
/*                                                                  */
/* SGMLListTextObject                                               */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a list text segment for the SGMLText widget              */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLLISTTEXT_H
#define  SGMLLISTTEXT_H
#include "SGMLCompositeText.h"

#define SGMLNnumBullets   "numBullets"
#define SGMLNallocBullets "allocBullets"
#define SGMLNbulletType   "bulletType"
#define SGMLCBulletType   "BulletType"
#define SGMLRBulletType   "BulletType"
#define SGMLNbulletSize   "bulletSize"
#define SGMLCBulletSize   "BulletSize"
#define SGMLNbulletColor  "bulletColor"
#define SGMLCBulletColor  "BulletColor"
#define SGMLNbulletIndent "bulletIndent"
#define SGMLCBulletIndent "BulletIndent"

#define SGMLBULLET_DEFAULT -1
#define SGMLBULLET_BULLET  0
#define SGMLBULLET_DIAMOND 1
#define SGMLBULLET_SQUARE  2
#define SGMLBULLET_NUMERIC 3
#define SGMLBULLET_ALPHA   4

#define SGMLINHERIT_CLASS NULL 

extern WidgetClass sGMLListTextObjectClass;
typedef struct _SGMLListTextRec  *SGMLListTextObject;

#ifdef _NO_PROTO

extern Widget  CreateSGMLListText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateListText(Widget parent,
        char *name,
        ArgList al,
        int ac);

    extern void SGMLListTextInsertChild(Widget parent, Widget child);
    
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsListText(w)     XtIsSubclass(w,sGMLListTextObjectClass)

#endif SGMLLISTTEXT_H

