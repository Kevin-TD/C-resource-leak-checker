#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main () {
   char *str;
   int a = getchar();

   /* Initial memory allocation */
   str = (char *) malloc(15);
   strcpy(str, "helloworld");
   printf("String = %s,  Address = %u\n", str, str);

   /* Reallocating memory */
   str = (char *) realloc(str, 25);
   strcat(str, "hello");
   printf("String = %s,  Address = %u\n", str, str);

    if (a == -15) { // impossible
         free(str);
    }
    else if (a == -10) { // impossible
        char* str1; 
        str1 = (char *) malloc(15);

        if (a == -9) { // very impossible 
            free(str1);
        } 

         free(str); 

    }
    else {
      char* str2; 
        str2 = (char *) malloc(15);

        if (a == -9) { // very impossible 
            free(str2);
        } 

        free(str2);
        free(str); 
    }
   
   
   return(0);
}
