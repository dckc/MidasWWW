#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>

#include "SGMLHyper.h"
#include "Tree.h"

void MidasClassInit()
{
  MidasDeclareClass(xmRowColumnWidgetClass);
  MidasDeclareClass(xmPushButtonWidgetClass);
  MidasDeclareClass(xmPushButtonGadgetClass);
  MidasDeclareClass(xmLabelWidgetClass);
  MidasDeclareClass(xmLabelGadgetClass);

  MidasDeclareClass(sGMLHyperWidgetClass);
  MidasDeclareClass(treeWidgetClass);
}
