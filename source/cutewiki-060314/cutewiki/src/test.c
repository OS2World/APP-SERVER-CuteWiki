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


    ch = 'ö';
    if ( isalpha(ch) )
        fprintf(stderr, "ö is alpha\n");
    else
        fprintf(stderr, "ö is not alpha\n");

    if ( islower(ch) )
        fprintf(stderr, "ö is lowercase\n");
    else
        fprintf(stderr, "ö is not lowercase\n");

    ch = 'Ö';
    if ( isupper(ch) )
        fprintf(stderr, "Ö is uppercase\n");
    else
        fprintf(stderr, "Ö is not uppercase\n");

    return 1;
}
