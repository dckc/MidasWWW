#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include <X11/CompositeP.h>     

Cardinal DXmNumChildren(w)
Widget w;
{
   CompositePart *p;
   p = &(((CompositeRec *)w)->composite);
   return p->num_children;
}
Widget *DXmChildren(w)
Widget w;
{
   CompositePart *p;
   p = &(((CompositeRec *)w)->composite);
   return p->children;
}
