/*==================================================================*/
/*                                                                  */
/* SGMLTextObject                                                   */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a text segment for the SGMLText widget                   */
/*                                                                  */
/*==================================================================*/

#ifndef  SGMLTEXT_H
#define  SGMLTEXT_H

extern WidgetClass sGMLTextObjectClass;
typedef struct _SGMLTextRec  *SGMLTextObject;

#define SGMLALIGNMENT_NONE   0
#define SGMLALIGNMENT_LEFT   1
#define SGMLALIGNMENT_RIGHT  2
#define SGMLALIGNMENT_CENTER 3

#define SGMLBREAK_NEVER 0
#define SGMLBREAK_ALWAYS 2
#define SGMLBREAK_SOFT 1

#define SGMLINHERIT_ALIGNMENT -999
#define SGMLINHERIT_FONT    NULL
#define SGMLINHERIT_MARGIN -1
#define SGMLINHERIT_PARAGRAPHINDENT -999
#define SGMLINHERIT_SPACE 32767
#define SGMLINHERIT_BREAK -999
#define SGMLINHERIT_SENSITIVITY 2
#define SGMLINHERIT_UNDERLINE -999
#define SGMLINHERIT_OUTLINE 2
#define SGMLINHERIT_UNDERLINEHEIGHT 999
#define SGMLINHERIT_COLOR NULL

#define SGMLNfont                     "font"
#define SGMLNsensitiveFont            "sensitiveFont"
#define SGMLCFont                     "Font"
#define SGMLNcolor                    "color"
#define SGMLNsensitiveColor           "sensitiveColor"
#define SGMLCColor                    "Color"
#define SGMLNunderline                "underline"
#define SGMLNsensitiveUnderline       "sensitiveUnderline"
#define SGMLCUnderline                "underline"
#define SGMLNoutline                  "outline"
#define SGMLNsensitiveOutline         "sensitiveOutline"
#define SGMLCOutline                  "Outline"
#define SGMLNunderlineHeight          "underlineHeight"
#define SGMLNsensitiveUnderlineHeight "sensitiveUnderlineHeight"
#define SGMLCUnderlineHeight          "underlineHeight"
#define SGMLNbreakBefore              "breakBefore"
#define SGMLNbreakAfter               "breakAfter"
#define SGMLCBreak                    "Break"
#define SGMLRBreak                    "Break"
#define SGMLNsensitive                "sensitive"
#define SGMLCSensitive                "Sensitive"
#define SGMLNrightIndent              "rightIndent"
#define SGMLNleftIndent               "leftIndent"
#define SGMLNrightMargin              "rightMargin"
#define SGMLNleftMargin               "leftMargin"
#define SGMLNparagraphIndent          "paragraphIndent"
#define SGMLNspaceBefore              "spaceBefore"
#define SGMLNspaceAfter               "spaceAfter"
#define SGMLCMargin                   "Margin"
#define SGMLNalignment                "alignment"
#define SGMLCAlignment                "Alignment"
#define SGMLNspacing                  "spacing"
#define SGMLCSpacing                  "Spacing" 
#define SGMLNdepth                    "depth"
#define SGMLCDepth                    "Depth" 
#define SGMLNascent                   "ascent"
#define SGMLCAscent                   "Ascent" 
#define SGMLNuserdata                 "userdata"
#define SGMLCUserdata                 "Userdata" 

#define SGMLNtext        "text"
#define SGMLCText        "Text"
#define SGMLNparam       "param"
#define SGMLCParam       "param"
#define SGMLNlength      "length"
#define SGMLCLength      "Length"
#define SGMLNparamLength "paramLength"
#define SGMLCParamLength "ParamLength"

#define SGMLNactivateCallback "activateCallback"
#define SGMLNcreateCallback   "createCallback"
#define SGMLNmapCallback      "mapCallback"
#define SGMLCCallback         "Callback" 

/*
  Callback structure
*/

#define SGMLTEXT_ACTIVATE 1
#define SGMLTEXT_CREATE 2
#define SGMLTEXT_MAP 2

typedef struct {

    int     reason;   /* always = HYPER_REASON                            */
    XEvent *event;    /* event                                            */
    char   *text;     /* pointer on highlighted text selected (read only) */
    int     length;   /* length of selected text                          */
    char   *param;    /* pointer to hidden text */
    int     param_length;

}  SGMLTextCallbackStruct;

/*
   Other exported data structures
*/

typedef struct _SGMLCoord {

    Position            x,y;
    Dimension           depth;
    Dimension           ascent;

} SGMLCoord;     

typedef struct {

   SGMLCoord coord;
   int       alignment;
   Boolean   leave_space;
   Boolean   broken;
   Dimension space;
   Dimension natural_width;
   Dimension actual_width; 

} SGMLGeometry;  


#ifdef _NO_PROTO

extern Widget  CreateSGMLText();
 
#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern Widget SGMLCreateText(Widget parent,
        char *name,
        ArgList al,
        int ac);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

#define SGMLIsText(w)     XtIsSubclass(w,sGMLTextObjectClass)

#endif SGMLTEXT_H
