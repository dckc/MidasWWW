/*==================================================================*/
/*                                                                  */
/* SGMLFormattedTextObject                                          */
/*                                                                  */
/* T.Johnson - (TonyJ@Slacvx.Slac.Stanford.Edu)           June.92   */
/*                                                                  */
/* Defines a formatted text segment for the SGMLHyper widget        */
/*                                                                  */
/*==================================================================*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "SGMLFormattedTextP.h"

/* 
  Private functions 
*/

#define max(a,b) (a>b?a:b)

static void WordSize();
static void LineSize();

/*
  Widget class methods
*/

static void    Initialize();
static void    Destroy();
static void    ComputeSize();
static Boolean AdjustSize();
static void    Redisplay();
static Widget  Contains();
static void    Hilite();

#define Offset(field) XtOffsetOf(SGMLFormattedTextRec,sgml_formatted_text.field)

static XtResource resources[] = {

    {SGMLNrightIndent,SGMLCMargin,XtRDimension,sizeof(int),
    Offset (right_indent),XtRImmediate,(XtPointer)0},

    {SGMLNleftIndent,SGMLCMargin,XtRDimension,sizeof(int),
    Offset (left_indent),XtRImmediate,(XtPointer)0},

};
#undef Offset

/*---------------------------------------------------------------*/
/* Static initialisation of the class record                     */
/*---------------------------------------------------------------*/

SGMLFormattedTextClassRec  sGMLFormattedTextClassRec = {
    {
    (WidgetClass) &sGMLTextClassRec,     /* superclass            */
    "SGMLFormattedText",                 /* class_name            */
    sizeof(SGMLFormattedTextRec),        /* widget_size           */
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
    ComputeSize,          		 /* compute_size          */
    AdjustSize,          		 /* adjust_size           */
    SGMLInheritAdjustPosition,   	 /* adjust_position       */
    Redisplay,                           /* expose                */
    SGMLInheritActivate,                 /* activate              */
    Hilite,                              /* hilite                */
    Contains,                            /* contains              */
    SGMLInheritCallCreateCallback,       /* call_create_callback  */
    SGMLInheritCallMapCallback,          /* call_map_callback     */
    SGMLInheritMakeVisible,              /* make_visible          */
    NULL,                                /* extension             */
    },
    {
    NULL,                                /* ignore                */
    }
};


WidgetClass sGMLFormattedTextObjectClass = (WidgetClass) &sGMLFormattedTextClassRec;

/*--------------------------------------------------------------*/
/* Initialize:                                                  */
/*--------------------------------------------------------------*/

static void Initialize (request, new)
SGMLFormattedTextObject request, new;
{
   new->sgml_formatted_text.word_info = NULL;
   new->sgml_formatted_text.line_info = NULL;
   new->sgml_formatted_text.nwords = 0; 
   new->sgml_formatted_text.nlines = 0; 
   new->sgml_formatted_text.left_indent  = new->sgml_text.left_indent;
   new->sgml_formatted_text.right_indent = new->sgml_text.right_indent;

}

/*--------------------------------------------------------------*/
/* Destroy the widget: release all memory allocated             */
/*--------------------------------------------------------------*/

static void Destroy (w)
SGMLFormattedTextObject w;
{
   XtFree((char *) w->sgml_formatted_text.word_info);
   XtFree((char *) w->sgml_formatted_text.line_info);
}

/*--------------------------------------------------------------*/
/* Contains:                                                    */
/*--------------------------------------------------------------*/

static Widget Contains(w,x,y)
SGMLFormattedTextObject w;
Position x,y;
{

  if (w->sgml_text.sensitive && 
      w->sgml_text.begin.y < y  &&  w->sgml_text.begin.y + w->sgml_text.height > y &&
      w->sgml_text.margin  < x  &&  w->sgml_text.margin  + w->sgml_text.width   > x )
    {
       if (w->sgml_text.begin.y == w->sgml_text.end.y)
         {
           return (Widget) w;
         } 
       else if (y > w->sgml_text.end.y)
         {
           return (x < w->sgml_text.end.x)?(Widget) w:NULL; 
         }
       else if (y > w->sgml_text.begin.y + w->sgml_text.begin.depth)
         {
            return (Widget) w;
         }
       else
         {
           return (x > w->sgml_text.begin.x)?(Widget) w:NULL; 
         }      
    }    

  return NULL;
}

/*--------------------------------------------------------------*/
/* Hilite:                                                      */
/*--------------------------------------------------------------*/

static void Hilite(t,state)
SGMLTextObject t;
Boolean state;
{
  Display *dpy = XtDisplayOfObject((Widget) t);
  Window  wind = XtWindowOfObject((Widget) t);
  GC        gc = t->sgml_text.invert_gc;
  
  if (t->sgml_text.begin.y == t->sgml_text.end.y)
    {
      Position x = t->sgml_text.begin.x; 
      Position y = t->sgml_text.begin.y; 
      Dimension width  = t->sgml_text.width;
      Dimension height = t->sgml_text.height;

      XFillRectangle(dpy,wind,gc,x,y,width,height);
    }
  else
    {
      XPoint p[8];

      p[0].x = t->sgml_text.begin.x;
      p[0].y = t->sgml_text.begin.y;
      p[1].x = t->sgml_text.margin + t->sgml_text.width;
      p[1].y = p[0].y;
      p[2].x = p[1].x;
      p[2].y = t->sgml_text.end.y;
      p[3].x = t->sgml_text.end.x;
      p[3].y = p[2].y;
      p[4].x = p[3].x;
      p[4].y = t->sgml_text.end.y + t->sgml_text.end.depth;
      p[5].x = t->sgml_text.margin;
      p[5].y = p[4].y;
      p[6].x = p[5].x;
      p[6].y = t->sgml_text.begin.y + t->sgml_text.begin.depth;
      p[7].x = p[0].x;
      p[7].y = p[6].y;
      
      XFillPolygon(dpy,wind,gc,p,XtNumber(p),Nonconvex,CoordModeOrigin);
    }
}

/*--------------------------------------------------------------*/
/* Redisplay                                                    */
/*--------------------------------------------------------------*/

static void Redisplay(t,event,region)
SGMLFormattedTextObject t;
XEvent *event;
Region region;
{
   Position y = t->sgml_text.begin.y;
   Position margin = t->sgml_text.margin;    
   Dimension width  = t->sgml_text.width;
   Dimension height = t->sgml_text .height;

   if(XRectInRegion(region,margin,y,width,height) != RectangleOut)
   {
      Display *dpy = XtDisplayOfObject((Widget) t);
      Window  wind = XtWindowOfObject((Widget) t);
      GC gc =  t->sgml_text.gc;
      XSegment *segs = NULL;

      Position x = t->sgml_text.begin.x;
      Dimension depth = t->sgml_text.begin.depth;
      Dimension ascent = t->sgml_text.begin.ascent;
      WordInfo *info = t->sgml_formatted_text.word_info;
      LineInfo *line = t->sgml_formatted_text.line_info; 
      SGMLRendition *rendition; 
      int l;

      if (t->sgml_text.sensitive) rendition = &t->sgml_text.sensitive_rendition;
      else                        rendition = &t->sgml_text.normal_rendition;

      for( l = 0 ; l < t->sgml_formatted_text.nlines; )
      {
        if (XRectInRegion(region,margin,y,width,depth) != RectangleOut &&
            line[l].nchars > 0)
        { 
          int m;
          char *p = XtMalloc(line[l].nchars);
          char *q = p;
          for ( m = line[l].start ; m < line[l].stop ; m++)
          {
            strncpy(p,info[m].word,info[m].length);
            p += info[m].length;
            *p++ = ' ';
          }  
          *--p = '\0'; 
          XDrawString(dpy, wind,gc,
                      x, y+ascent,
                      q,line[l].nchars-1);

          if (rendition->underline)
            {
              int s;
              Position yy = y+ascent; 
 
              if (!segs) segs = (XSegment *) XtMalloc(sizeof(XSegment) * rendition->underline);

              for (s=0; s < rendition->underline ; s++ )
                {
                   yy += max(rendition->underline_height,1); 
                   segs[s].x1 = x;  
                   segs[s].x2 = line[l].size;
                   segs[s].y1 = yy;  
                   segs[s].y2 = yy;  
                   yy += max(rendition->underline_height,1); 
                }
              XDrawSegments(dpy,wind,gc,segs,rendition->underline);
            }
          if (rendition->outline)
            {
              XDrawRectangle(dpy,wind,gc,x,y,line[l].size-x,depth-1);
            }
          XtFree(q); 
        }   
        x = margin;
        y += depth;   

        if (l++ == t->sgml_formatted_text.nlines) break;
        else if ( l+1 == t->sgml_formatted_text.nlines)
          {
            depth = t->sgml_text.end.depth;
            ascent = t->sgml_text.end.ascent;
          }
        else
          {
            depth = t->sgml_text.depth;
            ascent = t->sgml_text.ascent;
          }
      }
     XtFree((char *) segs);
   }
}
/*--------------------------------------------------------------*/
/* Adjust Size:                                                 */
/*--------------------------------------------------------------*/

static Boolean AdjustSize(w,y,ascent,depth)
SGMLFormattedTextObject w;
Position y;
Dimension ascent, depth;
{
   if (w->sgml_text.end.y != y) return FALSE;
     
   w->sgml_text.end.ascent = ascent;
   w->sgml_text.height += depth - w->sgml_text.end.depth; 
   w->sgml_text.end.depth = depth;

   if (w->sgml_text.begin.y == y)
     {
       w->sgml_text.begin.ascent = ascent;
       w->sgml_text.begin.depth = depth;
       return TRUE;
     }
        
   return FALSE;
}   
/*--------------------------------------------------------------*/
/* Compute Size:                                                */
/*--------------------------------------------------------------*/

static void ComputeSize(w,geom,Adjust,Closure)
SGMLFormattedTextObject w;
SGMLGeometry *geom;
AdjustSizeProc Adjust;
Opaque Closure;
{
   Dimension left_clearance  = w->sgml_text.left_margin  + w->sgml_formatted_text.left_indent;
   Dimension right_clearance = w->sgml_text.right_margin + w->sgml_formatted_text.right_indent;
   Boolean punctuation;
   char *punc_char = ".,:;)"; 
   Position xmin, xmax;

/* If we haven't already calculated the size of the words, do so now. */

   if (w->sgml_formatted_text.word_info == NULL) WordSize(w); 
 
   /*
    * Break before?
    */  

   if ( w->sgml_text.break_before == SGMLBREAK_ALWAYS ||
       (w->sgml_text.break_before == SGMLBREAK_SOFT && geom->coord.x > left_clearance) ||
       (w->sgml_text.alignment != geom->alignment && geom->alignment))
   {
      _SGMLBreak(geom,w->sgml_text.space_before);
   } 

  
/*
 *  We will leave a space before the first word if:
 *       a) Not beginning a new line
 *       b) Not a punctuation character
 *       c) the geometry structure requests it.
 */
 
   punctuation = !w->sgml_formatted_text.word_info || strchr(punc_char,*w->sgml_formatted_text.word_info[0].word);
  
   if (!punctuation) 
     if (geom->coord.x + w->sgml_text.spacing + w->sgml_formatted_text.word_info[0].size + right_clearance > geom->natural_width)
       {
          geom->coord.x = left_clearance;
          geom->coord.y += geom->coord.depth;
          geom->coord.depth = w->sgml_text.depth;
          geom->coord.ascent = w->sgml_text.ascent;
       }
     else if (geom->leave_space) geom->coord.x += w->sgml_text.spacing;   

   if (w->sgml_text.depth  > geom->coord.depth  ||
       w->sgml_text.ascent > geom->coord.ascent )
     {  
       if (w->sgml_text.depth  > geom->coord.depth)  geom->coord.depth  =  w->sgml_text.depth;
       if (w->sgml_text.ascent > geom->coord.ascent) geom->coord.ascent =  w->sgml_text.ascent;
       
       Adjust(Closure, geom->coord.y, geom->coord.ascent, geom->coord.depth);
     }  

   if (left_clearance > geom->coord.x && geom->broken) geom->coord.x = left_clearance;

   w->sgml_text.begin = geom->coord;

   LineSize(w,geom,&xmin,&xmax);

   if (xmax > geom->actual_width) geom->actual_width = xmax;
   
   w->sgml_text.width  = xmax - xmin;
   w->sgml_text.margin = xmin;  
   w->sgml_text.height = geom->coord.y + geom->coord.depth - w->sgml_text.begin.y;

   w->sgml_text.end = geom->coord;

   /*
    * Break after?
    */  

   if (w->sgml_text.break_after)
   {
     _SGMLBreak(geom,w->sgml_text.space_after);
   } 
}

/*-----------------------------------------------------------------------*/
/* Private routines                                                      */
/*-----------------------------------------------------------------------*/
/* WordSize: Build an array giving the length of each word               */
/*-----------------------------------------------------------------------*/

static void WordSize(w)
SGMLFormattedTextObject w;
{
   int nwords;
   char *p = w->sgml_text.text;
   char *delim  = " \t\n"; 
   WordInfo *info = w->sgml_formatted_text.word_info;
   XFontStruct *font;
   
   if (w->sgml_text.sensitive) font=w->sgml_text.sensitive_rendition.font;
   else                        font=w->sgml_text.normal_rendition.font; 
   
   /* count how many words */
   
   for (nwords=0; *p != '\0';nwords++) 
   {
     for (; *p != '\0' && strchr(delim,*p);  p++); /* scan white space */
     if (*p == '\0') break;
      
     for (; !strchr(delim,*p); p++); 
   }
   
   /* Allocate storage for word statistics */

   XtFree((char *) info); 
   info = nwords?(WordInfo *) XtMalloc(sizeof(WordInfo) * nwords):NULL; 

   /* Now store length of each word */
   
   p = w->sgml_text.text;   
   for (nwords=0; *p != '\0' ; nwords++) 
   {
     for (; *p != '\0' && strchr(delim,*p);  p++); /* scan white space */
     if (*p == '\0') break;
     
     info[nwords].word = p;
     for (; !strchr(delim,*p); p++);
     info[nwords].length = p - info[nwords].word;

     info[nwords].size = XTextWidth(font,info[nwords].word,info[nwords].length);
   }
   w->sgml_formatted_text.word_info = info;
   w->sgml_formatted_text.nwords = nwords;

}
/*-----------------------------------------------------------------------*/
/* LineSize: Build an array giving the length of each line               */
/*-----------------------------------------------------------------------*/

static void LineSize(w, geom, xmin, xmax)
SGMLFormattedTextObject w;
SGMLGeometry *geom;
Position *xmin, *xmax; 
{
  Position x = geom->coord.x;
  Position y = geom->coord.y; 
  Position maxX = x;
  Position minX = x; 
  int n = 0;  
  int l = 0;
  int nc = 0; 
  int lmax = 10;
  Dimension size = 0;  
  Dimension spacing = 0;
  WordInfo *info = w->sgml_formatted_text.word_info;
  Dimension left_clearance  = w->sgml_text.left_margin  + w->sgml_formatted_text.left_indent;
  Dimension right_clearance = w->sgml_text.right_margin + w->sgml_formatted_text.right_indent;
  
  LineInfo *line = (LineInfo *) XtMalloc(lmax*sizeof(LineInfo)); 

  XtFree((char *) w->sgml_formatted_text.line_info);
     
  line[l].start = n;
 
  for (; n < w->sgml_formatted_text.nwords; n++)
  {
    if (spacing>0 && x + spacing + info[n].size + right_clearance > geom->natural_width)
    {
      line[l].stop = n;
      line[l].nchars = nc;  
      line[l].size = x;
      if (x > maxX) maxX = x;

      if (++l == lmax) line = (LineInfo *) XtRealloc((char *) line, (lmax *= 2)*sizeof(LineInfo));

      line[l].start = n;
      x = left_clearance + info[n].size;
      y += geom->coord.depth;
      geom->coord.depth = w->sgml_text.depth;
      geom->coord.ascent = w->sgml_text.ascent;
      minX = left_clearance;
      nc = info[n].length + 1; 
    }  
    else 
    {
      x += spacing + info[n].size;
      nc += info[n].length + 1;
    }
    geom->alignment = w->sgml_text.alignment; 
    geom->broken = FALSE;
    geom->leave_space = TRUE;
    spacing =  w->sgml_text.spacing;
  }
  line[l].stop = n;
  line[l].nchars = nc;  
  line[l].size = x;
  if (x > maxX) maxX = x;
  
  w->sgml_formatted_text.line_info = line;
  w->sgml_formatted_text.nlines = l + 1;

  geom->coord.x = x; 
  geom->coord.y = y;  
  *xmin = minX; 
  *xmax = maxX;  

}
/*-----------------------------------------------------------------------*/
/* Create a new SGMLFormattedTextObject                                  */
/*-----------------------------------------------------------------------*/

Widget SGMLCreateFormattedText(parent,name,al,ac)
Widget parent;
char   *name;
ArgList al;
int     ac;
{
    return XtCreateWidget(name,sGMLFormattedTextObjectClass,parent,al,ac);
}
