/*==================================================================*/
/*                                                                  */
/* SGMLAnchorTextObject                                             */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a anchortext segment for the SGMLHyper widget            */
/*                                                                  */
/*==================================================================*/

#ifdef VAX
#include ctype
#endif

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLAnchorTextP.h"
#include <string.h>

/* 
  Private functions 
*/

/*
  Widget class methods
*/

static void    Initialize();
static void    Destroy();

#define Offset(field) XtOffsetOf(SGMLAnchorTextRec,sgml_anchortext.field)

static XtResource resources[] = {


    {SGMLNhref, SGMLCHref, XtRString, sizeof(String),
    Offset(href), XtRString, ""},

    {SGMLNname, SGMLCName, XtRString, sizeof(String),
    Offset(name), XtRString, ""},

    {SGMLNhrefLength, SGMLCHrefLength, XtRInt, sizeof(int),
    Offset(href_length), XtRImmediate, 0},

    {SGMLNnameLength, SGMLCNameLength, XtRInt, sizeof(int),
    Offset(name_length), XtRImmediate, 0}

};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLAnchorTextClassRec  sGMLAnchorTextClassRec = {
    {
    (WidgetClass) &sGMLFormattedTextClassRec,    /* superclass    */
    "SGMLAnchorText",                    /* class_name            */
    sizeof(SGMLAnchorTextRec),           /* widget_size           */
    NULL,                                /* class_initialize      */
    NULL,                                /* class_part_initialize */
    FALSE,                               /* class_inited          */
    Initialize,                          /* initialize            */
    NULL,                                /* initialize_hook       */
    NULL,                                /* obj1                  */
    NULL,                                /* obj2                  */
    0,                                   /* obj3                  */
    resources,                           /* resources             */
    XtNumber(resources),                 /* num_resources         */
    NULLQUARK,                           /* xrm_class             */
    0,                                   /* obj4                  */
    0,                                   /* obj5                  */
    0,                                   /* obj6                  */
    0,                                   /* obj7                  */
    Destroy,                             /* destroy               */
    NULL,                                /* obj8                  */
    NULL,                                /* obj9                  */
    NULL,                                /* set_values            */
    NULL,                                /* set_values_hook       */
    NULL,                                /* obj10                 */
    NULL,                                /* get_values_hook       */
    NULL,                                /* obj11                 */
    XtVersion,                           /* version               */
    NULL,                                /* callback private      */
    NULL,                                /* obj12                 */
    NULL,                                /* obj13                 */
    NULL,                                /* obj14                 */
    NULL,                                /* extension             */
    },
    {
    SGMLInheritComputeSize,         	 /* compute_size          */
    SGMLInheritAdjustSize,               /* adjust_size           */
    SGMLInheritAdjustPosition,   	 /* adjust_position       */
    SGMLInheritExpose,                   /* expose                */
    SGMLInheritActivate,                 /* activate              */
    SGMLInheritHilite,                   /* hilite                */
    SGMLInheritContains,                 /* contains              */
    SGMLInheritCallCreateCallback,       /* call_create_callback  */
    SGMLInheritCallMapCallback,          /* call_map_callback     */
    SGMLInheritMakeVisible,              /* make_visible          */
    NULL,                                /* extension             */
    }
};

WidgetClass sGMLAnchorTextObjectClass = (WidgetClass) &sGMLAnchorTextClassRec;

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLAnchorTextObject request, new;
{
   char *key1 = "href=";
   char *key2 = "name=";
   char *delim = " \n\t";
   char *p, *q; 
   int l;
 
   q = XtNewString(new->sgml_text.param);
   for (p = q ; *p != '\0' ; p++) if (isupper(*p)) *p = tolower(*p);

   p = strstr(q,key1);
   if (p)
     {
       p += strlen(key1);
       new->sgml_anchortext.href_length = l = strcspn(p,delim);
       new->sgml_anchortext.href = strncpy((char *)XtMalloc(l+1),new->sgml_text.param+(p-q),l);
       *(new->sgml_anchortext.href + l) = '\0';
     }
   else new->sgml_anchortext.href = XtNewString("");  

   p = strstr(q,key2);
   if (p)
     {
       p += strlen(key2);
       new->sgml_anchortext.name_length = l = strcspn(p,delim);
       new->sgml_anchortext.name = strncpy((char *)XtMalloc(l+1),new->sgml_text.param+(p-q),l);
       *(new->sgml_anchortext.name + l) = '\0';
     }   
   else new->sgml_anchortext.name = XtNewString("");
   
   XtFree(q);
}

/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLAnchorTextObject w;
{
   XtFree(w->sgml_anchortext.href);
   XtFree(w->sgml_anchortext.name);
}

/*-----------------------------------------------------------------------*/
/* Create a new SGMLAnchorTextObject                                           */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateAnchorText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLAnchorTextObjectClass,parent,al,ac);
}
