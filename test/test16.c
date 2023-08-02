#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../Annotations/Annotations.h"

struct my_struct {
    MustCall(STRUCT("my_struct") AT_FIELD("x"), METHODS("free"))
    MustCall(STRUCT("my_struct") AT_FIELD("y"), METHODS("free"))
    char* x; // MustCall("free")
    char* y; // MustCall("free")
};

Calls(FUNCTION("does_free") AT_PARAMETER("1"), METHODS("free"))
void does_free(char* s) { 
    free(s); 
}

/*
void does_free(char* s MustCall("free")) { 
    free(s); 
}
*/


Calls(FUNCTION("creates_obligation") AT_PARAMETER("1"), METHODS("free"))
Calls(FUNCTION("creates_obligation") AT_PARAMETER("2") AT_FIELD("x"), METHODS("free"))
MustCall(FUNCTION("creates_obligation") AT_RETURN, METHODS("free")) 
char* creates_obligation(char* s, struct my_struct X) {    
    free(s);  
    free(X.x); 
    char* str = (char*)malloc(15); 
    return str; 
}

/*
char* MustCall("free") creates_obligation(char* s Calls("free"), struct my_struct X Calls("free", "x")) {    
    free(s);  
    free(X.x); 
    char* str = (char*)malloc(15); 
    return str; 
}
*/

Calls(FUNCTION("does_something") AT_RETURN AT_FIELD("x"), METHODS("free"))
struct my_struct does_something(struct my_struct S) {
    free(S.x); 
    return S; 
}

/*
struct my_struct Calls("free", "x") does_something(struct my_struct S Calls("free", "x")) {
    free(S.x); 
    return S; 
}
*/

int main() {
    struct my_struct k; 
    k.x = (char*)malloc(15); 
    k.y = (char*)malloc(15); 
    // obligations to free k.x and k.y created 

    char* s = (char*)malloc(15);  
    char* y = creates_obligation(s, k);  // creates obligation to free y; the method itself also calls free on k.x 

    free(y); // y, k.x freed. now to free k.y 
    does_free(k.y); // k.y freed 
    
    // all obligations satisfied 

}