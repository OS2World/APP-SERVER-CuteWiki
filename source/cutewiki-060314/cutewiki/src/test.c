#include <ctype.h>
#include <stdio.h>
#include <locale.h>

int main(int argc, char *argv[])
{
    int ch ;

    /* Set the specific locale environment */
    if (!setlocale(LC_ALL, "de_DE.iso88591")) {
        perror("setlocale fails");
        return 1;
    }


    ch = '�';
    if ( isalpha(ch) )
        fprintf(stderr, "� is alpha\n");
    else
        fprintf(stderr, "� is not alpha\n");

    if ( islower(ch) )
        fprintf(stderr, "� is lowercase\n");
    else
        fprintf(stderr, "� is not lowercase\n");

    ch = '�';
    if ( isupper(ch) )
        fprintf(stderr, "� is uppercase\n");
    else
        fprintf(stderr, "� is not uppercase\n");

    return 1;
}
