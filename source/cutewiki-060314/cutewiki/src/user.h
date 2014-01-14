/*
 * user.h - all the user related function headers
 *
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



void	user_set_userid(Page * page, const char * userid);
char *  user_get_userid(Page * userpage);
char *  user_get_logname();
//bool    user_check_authentication(httpd * server);
bool    user_logoff(httpd * server);
bool	user_is_admin();
bool    user_is_authenticated();

void	user_set_password(Page * page, const char * password);
void	user_set_own_password(Page * page, const char * password);
bool	user_reset_password(char * username);

void	user_init();
void	user_exit();
