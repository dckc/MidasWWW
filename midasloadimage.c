#ifdef VAX
#include <lnmdef.h>
#include descrip
typedef struct dsc$descriptor_s string;
#endif

MidasLoadImage(name)
char *name;
/*
    The VAX version of MidasLoadImage looks for a shareable using the SLD
    dynamic loading convention.
*/
{
#ifdef VAX
   int Iss, lr;
   string LogicalName, RoutineName = {0,0,0,0}, ImageName = {0,0,0,0};
   char *p;
   $DESCRIPTOR(FileDev,"LNM$FILE_DEV");

   void (*Routine)();

   char result[80] = "SHR_";   
   struct {
     short BufLen;
     short ItmCod;
     int   BufAdr;
     int   RetAdr;  
   } ItmLst[] = {{80, LNM$_STRING, result, 0},
                 {4 , LNM$_LENGTH,  &lr, 0}, 
                 {0,0,0,0}};

   strcat(result,name);

   LogicalName.dsc$a_pointer = result;
   LogicalName.dsc$w_length = strlen(result);
   
   Iss = Sys$TrnLnm(&LNM$M_CASE_BLIND,&FileDev,&LogicalName,0,ItmLst);
   if ((Iss & 1) == 0) MidasError("Could not load image %s",name);
 
   p = &result + lr;
   *p = '\0';
   
   p = strstr(result,"/IMAGE=");
   if (p == 0) MidasError("Could not load image %s",name);

   *p = '\0';
   p += 7;
   
   ImageName.dsc$a_pointer = p;
   ImageName.dsc$w_length = strlen(p);

   RoutineName.dsc$a_pointer = &result;
   RoutineName.dsc$w_length = strlen(result);

   Iss = Lib$Find_Image_Symbol(&ImageName,&RoutineName,&Routine);
   if ((Iss & 1) == 0) MidasError("Could not load image %s",name);
 
   Routine();
#endif
}
