#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv){

    char url[128]; //you could make this bigger if you want
    printf("Dawaj adres: ");
    scanf(" %s", url); // get the url from the console

    char call[256];
    strcpy(call, "xdg-open "); // web browser command
    strcat(call, url); // append url

    system(call);

    return 0;
}
