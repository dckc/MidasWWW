/*
 *  These are the WWW - Midas Interface routines
 */ 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#ifdef VAX
#define CADDR_T
#define __TYPES_LOADED
#include ctype
#endif

#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include "midas.h"
#include "SGMLHyper.h"
#include "SGMLAnchorText.h"
#include "SGMLCompositeText.h"
#include <string.h>

 
#ifndef VAX
#define socket_read(a,b,c) read(a,b,c)
#define socket_write(a,b,c) write(a,b,c)
#define socket_close(a) close(a)
#endif

/*
 *  Structures
 */

#define HASHSIZE 337
#define DEADNODE ((WWWNode *) -1)

typedef struct  {

  XrmQuark protocol;
  XrmQuark node;
  int      port;
  XrmQuark file;      
  XrmQuark anchor; 
  XrmQuark keyword;

} WWWFile;  

static WWWFile WWWFileDefault;

struct _WWWNode {

  struct     _WWWNode *hashclash; 
  
  struct     _WWWNode *parent;
  struct     _WWWNode *next;
  struct     _WWWNode *prev;
  struct     _WWWNode *up;
  struct     _WWWNode *down;
  struct     _WWWNode *chain;

  WWWFile    *file;
  Boolean    visited;

  WidgetList widgets;  /* current list of widgets corresponding to this node */
  int num_widgets;
  int alloc_widgets;

};
typedef struct _WWWNode WWWNode;   

struct _WWWLink {
    
  WWWNode *from;
  WWWNode *to;    

};
typedef struct _WWWLink WWWLink;   

static WWWNode *HashTable[HASHSIZE];
static WWWFile *WWWGateway; 
static WWWNode *WWWToLink = NULL;
static WWWNode *WWWFromLink = DEADNODE;

/* Ugly static variable that keeps track of where we are in processing a document */

static int nextid;
static WWWNode *lastnode;
static WWWNode *parent;

static XrmQuark localProtocol;
static XrmQuark httpProtocol;

typedef struct {

  int   socket;
  int   nleft;
  char  *next;
  
} DataSource;  

/*
 * Application resources
 * 
 */

#define WWWINHERIT_FONT    NULL
#define WWWINHERIT_UNDERLINE -999
#define WWWINHERIT_OUTLINE 2
#define WWWINHERIT_UNDERLINEHEIGHT 999
#define WWWINHERIT_COLOR NULL

#define WWWNvisitedFont            "visitedFont"
#define WWWNvisitedColor           "visitedColor"
#define WWWNvisitedUnderline       "visitedUnderline"
#define WWWNvisitedOutline         "visitedOutline"
#define WWWNvisitedUnderlineHeight "visitedUnderlineHeight"

#define WWWNdefaultHTTPPort        "defaultHTTPPort"
#define WWWCPort                   "Port"
#define WWWNhelp                   "help"
#define WWWCHelp                   "Help" 

typedef struct {

    Pixel               color;
    Boolean             outline;                 
    int                 underline;
    Dimension           underline_height;
    XFontStruct         *font;

    int                 default_HTTP_port;
    String              help; 

} WWWResources;

static WWWResources appResources;

#define Offset(field) XtOffsetOf(WWWResources,field) 

static XtResource resources[] = { 

    {WWWNvisitedFont, SGMLCFont, XtRFontStruct, sizeof (XFontStruct *),
    Offset(font), XtRString, WWWINHERIT_FONT},

    {WWWNvisitedColor, SGMLCColor, XtRPixel, sizeof (Pixel),
    Offset(color),XtRString, WWWINHERIT_COLOR},

    {WWWNvisitedOutline,SGMLCOutline,XtRBoolean,sizeof(Boolean),
    Offset(outline),XtRImmediate,(XtPointer) WWWINHERIT_OUTLINE},

    {WWWNvisitedUnderline,SGMLCUnderline,XtRInt,sizeof(int),
    Offset(underline),XtRImmediate,(XtPointer) WWWINHERIT_UNDERLINE},

    {WWWNvisitedUnderlineHeight, SGMLCUnderlineHeight, XtRDimension, sizeof(Dimension),
    Offset(underline_height),XtRImmediate,(XtPointer) WWWINHERIT_UNDERLINEHEIGHT},

    {WWWNdefaultHTTPPort, WWWCPort, XtRInt, sizeof(int),
    Offset(default_HTTP_port),XtRImmediate,(XtPointer) 80},

    {WWWNhelp, WWWCHelp, XtRString, sizeof(char *),
    Offset(help),XtRString,(XtPointer) "http://slacvx.slac.stanford.edu:80/MidasWWW/v10/"},

};    

#undef Offset

/*
 * Calculates a hash id from a WWWFile
 * -----------------------------------
 */
static int WWWHash(file)
WWWFile *file;
{
   return ((int) file->protocol + 
           (int) file->node     + 
           (int) file->file     + 
           (int) file->anchor   + 
           file->port           ) % HASHSIZE; 
}
static void WWWAsciiFile(buffer,pfile)
char *buffer;
WWWFile *pfile;
{
  sprintf(buffer,"%s:",XrmQuarkToString(pfile->protocol));
  buffer += strlen(buffer);
  
  if (pfile->node)
    {
      sprintf(buffer,"//%s",XrmQuarkToString(pfile->node));
      buffer += strlen(buffer);
    }  

  if (pfile->port) 
    {  
      sprintf(buffer,":%d",pfile->port);
      buffer += strlen(buffer);
    }
     
  sprintf(buffer,"%s",XrmQuarkToString(pfile->file));
  buffer += strlen(buffer);

  if (pfile->anchor)
    {
      sprintf(buffer,"#%s",XrmQuarkToString(pfile->anchor));
      buffer += strlen(buffer);
    }

}
/*
 *  Dump the hash table
 *  -------------------
 */
static void WWWHashDump()
{
  int i;
  for (i=0; i<HASHSIZE; i++)
    {
      WWWNode *a = HashTable[i]; 
      for (; a != NULL; a = a->hashclash) 
        {
          WWWFile *pfile = a->file; 
          WWWNode *a = HashTable[i]; 
          char p[256];

          WWWAsciiFile(p,pfile);
          
          printf("(%d,%d,%d,%d,%d,%d) %s\n",pfile->protocol,pfile->node,pfile->port,
                                            pfile->file,pfile->anchor,pfile->keyword,p);
         }                                   
     }
}
/*
 * Called to mark an anchor as visited
 */
static void WWWBeenThere(w)
Widget w;
{
  Arg arglist[10];
  int n=0;
  
  if (appResources.color!=WWWINHERIT_COLOR) 
  {
    XtSetArg(arglist[n],SGMLNsensitiveColor,appResources.color); n++;
  }
  if (appResources.font!=WWWINHERIT_FONT) 
  {
    XtSetArg(arglist[n],SGMLNsensitiveFont,appResources.font ); n++;
  }
  if (appResources.outline!=WWWINHERIT_OUTLINE) 
  {
    XtSetArg(arglist[n],SGMLNsensitiveOutline,appResources.outline); n++;
  }
  if (appResources.underline!=WWWINHERIT_UNDERLINE) 
  {
    XtSetArg(arglist[n],SGMLNsensitiveUnderline,appResources.underline); n++;
  }
  if (appResources.underline_height!=WWWINHERIT_UNDERLINEHEIGHT) 
  {
    XtSetArg(arglist[n],SGMLNsensitiveUnderlineHeight,appResources.underline_height); n++;
  }
  if (n) XtSetValues(w,arglist,n); 
}
/*
 *  Delete a file structure
 *  -----------------------
 */
static void FreeFile(file)
WWWFile *file;
{
  XtFree((char *)file);
}
/*
 *  Create a copy of a file structure
 *  ---------------------------------
 */
static WWWFile *CopyFile(file)
WWWFile *file;
{
  WWWFile *new = XtNew(WWWFile);
  
  *new = *file;
  return new; 
}
static int CompareFile(file1,file2)
WWWFile *file1;
WWWFile *file2;
{
  return memcmp(file1,file2,sizeof(WWWFile));
}
/*
 * Find (or create) a node
 * -----------------------
 */ 

static WWWNode *WWWCreateNode(file,parent)
WWWFile *file;
WWWNode *parent;
{
  int hashid = WWWHash(file);
  WWWNode *a = HashTable[hashid]; 

  for (; a != NULL; a = a->hashclash) if (!CompareFile(a->file,file)) return a;

  a = XtNew(WWWNode);
        
  a->file = CopyFile(file);

  a->parent = parent ? parent : a;
  a->prev = NULL; 
  a->next = NULL;
  a->up   = NULL;
  a->down = NULL;
  a->chain= NULL;
  a->visited = FALSE;

  a->widgets = NULL;
  a->num_widgets = 0;
  a->alloc_widgets = 0;
   
  a->hashclash = HashTable[hashid];
  HashTable[hashid] = a;

  return a; 
}
/*
 * Find (or create) a node and if necessary its parent
 * ---------------------------------------------------
 */ 
static WWWNode *WWWCreateNodeAndParent(file)
WWWFile *file;
{
   WWWNode *parent;

   if (file->anchor) 
     {
       XrmQuark temp = file->anchor;
       file->anchor = NULL;      
       parent = WWWCreateNode(file,NULL);
       file->anchor = temp;
     }
   else parent = NULL;
       
   return WWWCreateNode(file,parent);
}
/*
 * Mark a node as visited
 * ----------------------
 */
WWWVisitNode(node)
WWWNode *node;
{
  if (!node->visited)
    {
      WWWNode *src = node->up;  

      for ( ; src != NULL; src = src->chain)
        {
           WidgetList w = src->widgets;
           int nw = src->num_widgets; 
           for (; nw-- > 0; w++) WWWBeenThere(*w);
        }
      node->visited = TRUE;
     } 
}
static void WWWDetachWidgetFromNode(w,node)
Widget w;
WWWNode *node;
{
  int i;
  for (i=0 ; i < node->num_widgets; i++)
    if (node->widgets[i] == w) node->widgets[i] = node->widgets[--node->num_widgets];
}
static void WWWAttachWidgetToNode(node,w)
WWWNode *node;
Widget w;
{
   if (node->num_widgets == node->alloc_widgets) 
     node->widgets = (Widget *) XtRealloc((char *) node->widgets, (node->alloc_widgets += 5) * sizeof(Widget));
   node->widgets[node->num_widgets++] = w;
   XtAddCallback(w,XtNdestroyCallback,WWWDetachWidgetFromNode,(XtPointer)node);
}
/*
 * Provides buffering for the TCP/IP packets
 * -----------------------------------------
 */
static char GetCharacter(data)
DataSource *data;
{
  static char response[1024];

  if (data->nleft == 0)
    {
      data->nleft = socket_read(data->socket,response,sizeof(response));
      data->next = response;
    }
  if (data->nleft-- > 0) return *(data->next++); 

  return '\0'; 
}
static char GetFileCharacter(f)
FILE **f;
{
    int n =  getc(*f);
    return (n==EOF?0:(char)n);
}
/*
 * Fetch the local document
 * ------------------------
 */

static Widget WWWFetchDocumentLOCAL(w,file)
Widget w;
WWWFile *file;
{
  Widget result;
  char *name = XrmQuarkToString(file->file); 

  FILE *f = fopen(name,"r");
  if (f == NULL)   
    {
      char command[256]; 
      sprintf(command,"<h2>Error:</h2>Cannot open file %s<p>For more information see <a href=%sfile_open_failed.html>help</a>.",
              name,appResources.help);
      return SGMLHyperSetText(w,command);
    }  

  result = SGMLHyperLoadText(w,GetFileCharacter,&f);     
  
  fclose(f);
  return result;
}  
/*
 * Fetch the document using TCP/IP
 * -------------------------------
 */

static Widget WWWFetchDocumentHTTP(w,file)
Widget w;
WWWFile *file;
{
  struct sockaddr_in server;
  int n, s, status;
  char command[256]; 
  char *node = XrmQuarkToString(file->node);
  struct hostent *hp;
  DataSource data;
  Widget result;

  hp = gethostbyname(node);
  if (hp == 0)
    {
      sprintf(command,"<h2>Error:</h2>Host %s unknown<p>For more information see <a href=%sunknown_host.html>help</a>.",
              node,appResources.help);
      return SGMLHyperSetText(w,command);
    }
  server.sin_family = AF_INET;
  server.sin_port = htons(file->port?file->port:appResources.default_HTTP_port);
  bcopy(hp->h_addr,&server.sin_addr, hp->h_length);
  
  s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  status = connect(s,&server,sizeof(server));
  
  if (status<0) 
    {
      sprintf(command,"<h2>Error:</h2>Cannot connect to %s port %d<p>For more information see <a href=%scannot_connect.html>help</a>.",
              node,file->port,appResources.help);
      socket_close(s);
      return SGMLHyperSetText(w,command);
    }
  
  strcpy(command,"GET ");
  strcat(command,XrmQuarkToString(file->file)); 
  strcat(command, "\r\n");

  socket_write(s,command,strlen(command));
  
  data.socket = s;
  data.nleft = 0;
  
  result = SGMLHyperLoadText(w,GetCharacter,&data);     
  
  socket_close(s);
  return result;
}  
/*
 * Fetch the document using appropriate protocol
 * ---------------------------------------------
 */

static Widget WWWFetchDocument(w,file)
Widget w;
WWWFile *file;
{

  if      (file->protocol == httpProtocol ) return WWWFetchDocumentHTTP(w,file);
  else if (file->protocol == localProtocol) return WWWFetchDocumentLOCAL(w,file);
  else 
    {
      WWWFile actual;   
      char p[256]; 

      *p = '/';
      WWWAsciiFile(p+1,file);
      
      actual.protocol = WWWGateway->protocol;
      actual.node     = WWWGateway->node;
      actual.port     = WWWGateway->port;
      actual.file = XrmStringToQuark(p); 
      actual.keyword = NULL;
      actual.anchor = NULL; 
      
      return WWWFetchDocumentHTTP(w,&actual);
    }
}  
static void WWWZapNode(w,node)
Widget w;
WWWNode *node;
{
   int n=0;
/* 
 * Remove a cached document
 */

  node = node->parent;
  
  for (n = 0; n < node->num_widgets; n++)
    {
      Widget t = node->widgets[n];
      if (SGMLIsCompositeText(t) && XtParent(t) == w)
        {     
           WWWDetachWidgetFromNode(t,node);  
           XtDestroyWidget(t);
           break;   
        }
    }    
}
/*
 * Display a new node in a widget
 * ------------------------------
 */ 
static void WWWDisplayNode(w,node)
Widget w;
WWWNode *node;
{
   Widget new;
   int n=0;
/*
 *  The document may already be cached in the widget?
 */    
  
  for (n = 0; n < node->num_widgets; n++)
    {
      Widget t = node->widgets[n];
      if (SGMLIsCompositeText(t) && XtParent(t) == w)
        {     
           SGMLHyperManageChild(t);
           return;   
        }
    }    
/*
 *  Otherwise we have to fetch it.
 */
 
  nextid = 0;
  lastnode = NULL;
  parent = node;

  new = WWWFetchDocument(w,node->file);
  WWWVisitNode(node);
  SGMLHyperManageChild(new);
  WWWAttachWidgetToNode(node,new);
}
/*
 *  parse a character file specification to create a file structure
 * ----------------------------------------------------------------
 */  
static WWWFile *ParseFile(infile,def)
char *infile;
WWWFile *def; 
{
  char *node, *oldfile, *newfile, *anchor, *protocol;
  char *directory, *port;
  WWWFile *result = CopyFile(def);
  char *file = XtNewString(infile); 
  char *orig = file;
   
  result->anchor = NULL; /* NOT inherited from default */

  node = strstr(file,"/");
  protocol = strchr(file,':');
  if (protocol && (protocol < node || node == 0)) 
    {
      *protocol = '\0';
      result->protocol = XrmStringToQuark(file);
      if (result->protocol != def->protocol)
        {    
          result->node = NULL;
          result->port = 0;
        }  
      file = protocol + 1;
    } 
  else protocol = NULL; 
    
  if (strncmp(file,"//",2)) 
    { 
      if (*file == '/' || protocol) 
        {
          result->file = XrmStringToQuark(file); 
        }
      else
        {
          Boolean absolute;

          oldfile = XtNewString(XrmQuarkToString(result->file));
          absolute = (*oldfile == '/');

          if (anchor = strchr(file,'#')) 
          {
            *anchor++ = '\0';
            result->anchor = XrmStringToQuark(anchor);
          }

          for (;*file;file += 3)
            {
              char *p = strrchr(oldfile,'/'); 
              if (!p)  p = oldfile; 
              *p = '\0';
              if (strncmp(file,"../",3)) break;
            }
          newfile = strcpy((char *) XtMalloc(strlen(oldfile) + strlen(file) + 2),oldfile);
          if ((absolute || *oldfile) && *file)strcat(newfile,"/");
          strcat(newfile,file);
          
          result->file = XrmStringToQuark(newfile);
          XtFree(newfile);
          XtFree(oldfile);
        }
    }
  else
    {
      char *p;

      node = file + 2;
     
      directory = strchr(node,'/');  
      if (directory) *(directory) = '\0';

      port = strchr(node,':');
      if (port)
        {
          *(port++) = '\0';
          result->port = atoi(port);   
        }

      for (p=node; *p != '\0'; p++) if (isupper(*p)) *p = tolower(*p);  
      result->node = XrmStringToQuark(node);

      if (directory)
        {
          *(directory) = '/';

          if (anchor = strchr(directory,'#')) 
            {
              *anchor++ = '\0';
              result->anchor = XrmStringToQuark(anchor);
            }
        }
      result->file = XrmStringToQuark(directory); 
      
    }

  XtFree(orig);
  return result;
}
static void WWWGet(ww,node)
Widget ww;
WWWNode *node;
{
  WWWLink *link;
  
  WWWDisplayNode(ww,node->parent); 
  WWWToLink = node;
   
  if (node != node->parent) 
    {
       WidgetList w = node->widgets;
       int nw = node->num_widgets; 
       for (; nw-- > 0; w++) 
         {
            Widget p;
            for (p = *w; p != NULL; p = XtParent(p)) 
              if (p == ww) { SGMLHyperShowObject(ww,*w); return; }
         }
    }
  else SGMLHyperShowObject(ww,NULL);
}
static void WWWDeleteHistory(list)
List *list;
{
  ListItem *item = MidasFindItemInListPos(list,0);
  if (item) MidasRemoveItemFromList(list,item);
}
static void WWWAddHistory(list,title)
List *list;
char *title;
{
  ListItem *item;
  WWWLink *link = XtNew(WWWLink);
  link->from = (WWWFromLink == DEADNODE) ? NULL : WWWFromLink;
  link->to = WWWToLink;

  if (WWWFromLink == DEADNODE) MidasEmptyList(list);
  if (WWWFromLink)
    {
      item = MidasAddItemToListPos(list,title,0);
      item->Pointer = (XtPointer) link; 
    }   

  WWWFromLink = DEADNODE;
  WWWToLink = NULL;
}
static void WWWLinkHistory(node)
WWWNode *node;
{
  WWWFromLink = node;
}
static void WWWPut(w,stuff)
Widget w;
char *stuff;
{
  Widget t = SGMLHyperSetText(w,stuff);
  SGMLHyperManageChild(t); 
}
static void WWWDump(w,file)
Widget w;
char *file;
{
  char *dump;
  FILE *out = fopen(file,"w");
  if (out==NULL) MidasError("Could not open file %s",file);

  dump = SGMLHyperGetText(w,TRUE);
  fputs(dump,out);
  fclose(out); 
  XtFree(dump);
}
/*
 *  Called when a new anchor is created
 *  ----------------------------------- 
 */
static void WWWCreateAnchor(w,doc)
Widget w;
WWWFile *doc;
{
  char *href, *name, buffer[12];
  WWWNode *src  = NULL;
  WWWNode *dest = NULL;
  WWWFile *here = CopyFile(doc); 

  Arg arglist[10];
  int n=0;
  
  XtSetArg(arglist[n],SGMLNhref,&href); n++;
  XtSetArg(arglist[n],SGMLNname,&name); n++;
  XtGetValues(w,arglist,n); 

  /*
   *  Create a source node, if the node has no name then make
   *  one up.
   */

  if (!name || !(*name))
    {
      name = buffer;
      sprintf(buffer,"@@%d",nextid++);  
    } 

  here->anchor = XrmStringToQuark(name); 
  
  src = WWWCreateNode(here,parent);
  FreeFile(here);
    
  if (lastnode) lastnode->next = src; 
  src->prev = lastnode;
  lastnode = src; 
  WWWAttachWidgetToNode(src,w);

  n=0;
  XtSetArg(arglist[n],SGMLNuserdata,(XtPointer) src); n++;
  XtSetValues(w,arglist,n);
  
  /*
   * If this node points somewhere then create a destination node
   */  

  if (href && *href)
    { 
      WWWNode *p;  
      here = ParseFile(href,doc);

      dest = WWWCreateNodeAndParent(here);
      FreeFile(here); 

      src->down = dest;

      p = dest->up; 
      for (; p != NULL; p  = p->chain) if (p==src) break;
      if (p == NULL) 
        {
          src->chain = dest->up; 
          dest->up = src;
        } 
      if (dest->visited) WWWBeenThere(w);
    }
}
/*
 * It should be possible to do this directly from MIDAS so this routine
 * should go away as soon as it actually is.
 */ 
static void WWWCopyBackgroundColor(src,dest)
Widget src;
Widget dest;
{ 
  Pixel bg;
  Arg arglist[1];

  XtSetArg(arglist[0],XmNbackground,&bg);
  XtGetValues(src,arglist,1);
  XtSetArg(arglist[0],XmNbackground,bg);
  XtSetValues(dest,arglist,1);
}
static MidasOperand WWWSource(w)
Widget w;
{
  MidasOperand Temp;
  char *dump = SGMLHyperGetText(w,TRUE);

  Temp.Value.P = dump;
  Temp.Dynamic = TRUE;
  Temp.Type = MString;
  
  return Temp; 
}
static MidasOperand WWWBack(list)
List *list;
{
  MidasOperand Temp;

  ListItem *item = MidasFindItemInListPos(list,0);
  WWWLink *link = (WWWLink *) item?item->Pointer:NULL;
  Temp.Value.P = link ? (XtPointer) link->from : NULL;
  
  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
  
  return Temp;
}
static MidasOperand WWWPrev(node)
WWWNode *node;
{
  MidasOperand Temp;

  Temp.Value.P = node ? (XtPointer) node->prev : NULL;
  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
  
  return Temp; 

}
static MidasOperand WWWNext(node)
WWWNode *node;
{
  MidasOperand Temp;

  Temp.Value.P = node ? (XtPointer) node->next : NULL;
  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
  
  return Temp; 

}
static MidasOperand WWWDest(node)
WWWNode *node;
{
  MidasOperand Temp;

  Temp.Value.P = node ? (XtPointer) node->down : NULL;
  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
  
  return Temp; 

}
static MidasOperand WWWGetNode(w)
Widget w;
{
  MidasOperand Temp;
  Arg arglist[10];
  int n=0;
  
  XtSetArg(arglist[n],SGMLNuserdata,&Temp.Value.P); n++;
  XtGetValues(w,arglist,n);

  Temp.Dynamic = FALSE;
  Temp.Type = "WWWNode";
  
  return Temp; 

}
static MidasOperand WWWParse(file,def)
char *file;
WWWFile *def;
{
  MidasOperand Temp;
  
  WWWFile *pfile = ParseFile(file,def);

  Temp.Value.P = (XtPointer) pfile;
  Temp.Dynamic = TRUE;
  Temp.Type = "WWWFile";
       
  return Temp;
}

/* 
 *  Converters
 *  ----------
 */
static Boolean WWWConvertStringFile(In,Out)
MidasOperand *In;
MidasOperand *Out;  
{ 
  WWWFile *pfile = ParseFile(In->Value.P,&WWWFileDefault);  

  Out->Value.P = (XtPointer) pfile;
  Out->Dynamic = TRUE;
       
  return TRUE;
}
static Boolean WWWConvertStringNode(In,Out)
MidasOperand *In;
MidasOperand *Out;  
{ 
  WWWFile *pfile = ParseFile(In->Value.P,&WWWFileDefault);  
  WWWNode *node = WWWCreateNodeAndParent(pfile);
  
  FreeFile(pfile); 
 
  Out->Value.P = (XtPointer) node;
  Out->Dynamic = FALSE;
       
  return TRUE;
}
static Boolean WWWConvertFileNode(In,Out)
MidasOperand *In;
MidasOperand *Out;  
{ 
  if (In->Value.P)
    {
      WWWNode *node = WWWCreateNodeAndParent((WWWFile *) In->Value.P);  
      Out->Value.P = (XtPointer) node;
    }
  else Out->Value.P = NULL;   
  Out->Dynamic = FALSE;
       
  return TRUE;
}
static Boolean WWWConvertNodeFile(In,Out)
MidasOperand *In;
MidasOperand *Out;  
{ 
  WWWNode *node = (WWWNode *) In->Value.P;
 
  Out->Value.P = node ? (XtPointer) node->file : NULL;
  Out->Dynamic = FALSE;
       
  return TRUE;
}
static Boolean WWWConvertFileString(In,Out)
MidasOperand *In;
MidasOperand *Out;
{
  char *p = (char *) XtMalloc(256); /* BUG */
  
  WWWFile *pfile = (WWWFile *) In->Value.P;
  WWWAsciiFile(p,pfile);  

  Out->Value.P = (XtPointer) p;
  Out->Dynamic = TRUE;
       
  return TRUE;
}
static void WWWSetGateway(gateway)
char *gateway;
{
  WWWGateway = ParseFile(gateway,&WWWFileDefault);
}
static void WWWInit(shell)
Widget shell;
{
  XtGetApplicationResources(shell,(XtPointer) &appResources,resources,XtNumber(resources),NULL,0);
}
/*
 * Initialization
 */ 
void WWWMidasInit(argc,argv)
int argc;
char *argv[];
{
  int i;
  
  for (i=0; i < HASHSIZE; i++) HashTable[i] = NULL; 

  localProtocol = XrmStringToQuark("local");
  httpProtocol = XrmStringToQuark("http");

  WWWFileDefault.protocol = httpProtocol;
  WWWFileDefault.port   = 0;
  WWWFileDefault.node   = NULL; 
  WWWFileDefault.file   = NULL; 
  WWWFileDefault.anchor = NULL; 

/*
 * MIDAS BUG confusion between name and String
 */

  MidasDeclareConverter("String","WWWFile", WWWConvertStringFile);
  MidasDeclareConverter("name","WWWFile", WWWConvertStringFile);
  MidasDeclareConverter("String","WWWNode", WWWConvertStringNode);
  MidasDeclareConverter("name","WWWNode", WWWConvertStringNode);
  MidasDeclareConverter("WWWFile","String", WWWConvertFileString);
  MidasDeclareConverter("WWWFile","name", WWWConvertFileString);
  MidasDeclareConverter("WWWFile","WWWNode",WWWConvertFileNode);
  MidasDeclareConverter("WWWNode","WWWFile",WWWConvertNodeFile);
  

  MidasDeclareVerb("WWW HASH DUMP"                ,WWWHashDump);
  MidasDeclareVerb("WWW INIT Widget"              ,WWWInit);
  MidasDeclareVerb("WWW SET GATEWAY name"         ,WWWSetGateway);
  MidasDeclareVerb("WWW DUMP Widget name"         ,WWWDump); 
  MidasDeclareVerb("WWW ADD HISTORY list name"    ,WWWAddHistory); 
  MidasDeclareVerb("WWW DELETE HISTORY list"      ,WWWDeleteHistory); 
  MidasDeclareVerb("WWW GET Widget WWWNode"       ,WWWGet); 
  MidasDeclareVerb("WWW ZAP Widget WWWNode"       ,WWWZapNode); 
  MidasDeclareVerb("WWW LINK {WWWNode}"           ,WWWLinkHistory);
  MidasDeclareVerb("WWW PUT Widget name"          ,WWWPut); 
  MidasDeclareVerb("WWW ANCHOR Widget WWWFile"    ,WWWCreateAnchor); 
  MidasDeclareVerb("WWW COPYB Widget Widget"      ,WWWCopyBackgroundColor); 

  MidasDeclareFunction("WWWPARSE(name,WWWFile)"   ,WWWParse); 
  MidasDeclareFunction("WWWSOURCE(Widget)"        ,WWWSource); 
  MidasDeclareFunction("WWWBACK(list)"            ,WWWBack); 
  MidasDeclareFunction("WWWPREV(WWWNode)"         ,WWWPrev); 
  MidasDeclareFunction("WWWNEXT(WWWNode)"         ,WWWNext); 
  MidasDeclareFunction("WWWDEST(WWWNode)"         ,WWWDest); 
  MidasDeclareFunction("WWWNODE(Widget)"          ,WWWGetNode);
}

/* 
 * Main routine for MidasWWW
 */

int main(argc,argv)
int argc;
char *argv[];
{ 
  MidasInitialize(argc,argv);
  WWWMidasInit(argc,argv);
  MidasMainLoop();
}
