/*
 * rcs.h - headers for rcs integration for CuteWiki
 *
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#ifndef RCS_H
#define RCS_H

void	rcs_init ();
bool	rcs_checkin (char*, char*);
bool	rcs_log (char*);
bool    rcs_diff (char*, char*, char*);
bool	rcs_available ();

#endif
