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

#ifndef  SGMLHYPER_H
#define  SGMLHYPER_H

/* 
    If you define MOTIF, the widget will inherit proprieties 
   from the XmManager class : Help Callback, user data, ...
*/

/*
#define MOTIF
*/


extern WidgetClass sGMLHyperWidgetClass;
typedef struct _SGMLHyperRec      * SGMLHyperWidget;

/*
 * Define resource strings for the Hyper widget.
 */

#define SGMLNzoomEffect        "zoomEffect"
#define SGMLCZoom              "Zoom"
#define SGMLNopenTag           "openTag"
#define SGMLNcloseTag          "closeTag"
#define SGMLNendTag            "endTag"
#define SGMLNparameterTag      "parameterTag"
#define SGMLCTagChar           "TagChar"
#define SGMLNzoomSpeed         "zoomSpeed"
#define SGMLCZoomSpeed         "ZoomSpeed"
#define SGMLNcacheSize         "cacheSize"
#define SGMLCCacheSize         "CacheSize"
#define SGMLNmargin            "margin"
#define SGMLNnaturalWidth      "naturalWidth"
#define SGMLCWidth             "Width"
#define SGMLNcaseSensitiveTags "caseSensitiveTags"
#define SGMLCCaseSensitiveTags "CaseSensitiveTags"

/*
  Callback structure
*/

#define SGMLHYPER_REASON 1

typedef struct {

    int     reason;   /* always = SGMLHYPER_REASON                        */
    XEvent *event;    /* event                                            */
    char   *text;     /* pointer on highlighted text selected (read only) */
    int     length;   /* length of selected text                          */
    char   *hidden;   /* pointer to hidden text */
    int     hidden_length;

}  SGMLHyperCallbackStruct;


Widget  CreateSGMLHyper();
Widget  SGMLHyperLoadFile();
Widget  SGMLHyperSetText();
Widget  SGMLHyperLoadText();

#ifdef _NO_PROTO
  void    SGMLHyperSetTags();
  char    *SGMLHyperGetText();
  void    SGMLHyperDeclareClass();
#else
  void    SGMLHyperSetTags();
  char    *SGMLHyperGetText();
  void    SGMLHyperDeclareClass();
#endif  

#define SGMLIsHyper(w)     XtIsSubclass(w,sGMLHyperWidgetClass)

#endif SGMLHYPER_H
