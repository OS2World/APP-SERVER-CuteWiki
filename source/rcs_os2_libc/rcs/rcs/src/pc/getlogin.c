/* getlogin.c
 *
 * Author:   <rommel@ars.de>
 * Created: Wed Sep 30 1998
 */
 
static char *rcsid =
"$Id$";
static char *rcsrev = "$Revision$";


#include <stdlib.h>

/*
 * $Log$ 
 */

char *getlogin(void)
{
  return getenv("RCS_USER");
}


