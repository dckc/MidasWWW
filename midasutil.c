#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include <string.h>
#include "midasoperand.h"

/*
 *  General utility functions
 */
 
static MidasOperand Locate(needle,haystack)
char *needle;
char *haystack;
{  
  MidasOperand Temp;
  char *p = strstr(haystack,needle);
  
  Temp.Value.I = p ? p-haystack : strlen(haystack);  
  Temp.Type = MInt;
  Temp.Dynamic = FALSE;
  
  return Temp;
}
static MidasOperand Extract(start,length,string)
int start;
int length;
char *string;
{ 
  MidasOperand Temp;
  int len = strlen(string);
  char *p;
 
  if (start>=len) p = NULL;
  else 
  {   
    p = string + start; 
    if (start+length < len) *(p+length) = '\0'; 
  }
  
  Temp.Value.P = XtNewString(p);
  Temp.Type = MString;
  Temp.Dynamic = TRUE;

  return Temp;
}
static MidasOperand Length(string)
char *string;
{
  MidasOperand Temp;

  Temp.Value.I = strlen(string);  
  Temp.Type = MInt;
  Temp.Dynamic = FALSE;
  
  return Temp;
}
void MidasUtilInit()
{
   MidasDeclareFunction("LENGTH(name)"         ,Length);
   MidasDeclareFunction("EXTRACT(Int,Int,name)",Extract);
   MidasDeclareFunction("LOCATE(name,name)"    ,Locate);
}
