/*
 * html.c - output functions for HTML
 *
 * Copyright 2005 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include "config.h"
#include "svr.h"



static void
html_login_header()
{
    svr_use_utf8(true);
    svr_puts(server, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
    
    svr_puts(server, "<html>\n");
    svr_puts(server, "  <head>\n");
    svr_puts(server, "    <link rel=\"stylesheet\" type=\"text/css\" href=\"/Files/cwlogin.css\">\n");
    svr_puts(server, "    <link rel=\"icon\" href=\"/Files/cutewiki.ico\" type=\"image/ico\">\n");
    svr_puts(server, "    <link rel=\"shortcut icon\" href=\"/Files/cutewiki.ico\">\n");
    svr_puts(server, "    <meta http-equiv=\"content-type\" content=\"text/html;charset=UTF-8\">\n");
    svr_puts(server, "    <meta http-equiv=\"cache-control\" content=\"no-store\" >\n");
    svr_puts(server, "    <meta http-equiv=\"pragma\" content=\"no-cache\" >\n");
    svr_puts(server, "    <meta http-equiv=\"expires\" content=\"0\" >\n");
    svr_puts(server, "    <meta name=\"robots\" content=\"noindex\">\n");
    svr_puts(server, "    <meta name=\"generator\" content=\"CuteWiki\">\n");
    svr_puts(server, "    <title>Login</title>\n");
    svr_puts(server, "  </head>\n\n\n");
    svr_puts(server, "  <body onLoad=\"document.login.username.focus();\">\n");
}



/*
 * WriteFooter
 */
static void
html_login_footer()
{
    svr_puts(server, "  </body>\n");
    svr_puts(server, "</html>\n");
}



/*
 * html_login_page
 *
 * Print all the fields we need for to authenticate the user
 */
void
html_login_page()
{
    html_login_header();

    /* now really print all the form */

    svr_puts(server, "    <div class=\"login\">\n");
    svr_puts(server, "      <form method=\"post\" accept-charset=\"UTF-8\" "
                            "action=\"/Wiki/StartPage\" name=\"login\">\n");

#if GERMAN
    svr_puts(server, "        CuteWiki - Bitte melden Sie sich an!\n");
#else
    svr_puts(server, "        CuteWiki - Please log in!\n");
#endif

    svr_puts(server, "        <table>\n");
    svr_puts(server, "          <tr>\n");
    svr_puts(server, "            <td>Username:</td>\n");
    svr_puts(server, "            <td><input name=\"username\" type=\"text\" "
	                         "size=\"40\" maxlength=\"40\"></td>\n");
    svr_puts(server, "          </tr>\n");
    svr_puts(server, "          <tr>\n");
    svr_puts(server, "            <td>Password:</td>\n");
    svr_puts(server, "            <td><input name=\"password\" type=\"password\" "
	                         "size=\"40\" maxlength=\"40\"></td>\n");
    svr_puts(server, "          </tr>\n");
    svr_puts(server, "        </table>\n");
    svr_puts(server, "        <input type=\"submit\" class=\"button\" "
                              "value=\" Login \">\n");
    svr_puts(server, "        <input type=\"reset\" class=\"button\" "
                              "value=\" Clear \">\n");
    svr_puts(server, "      </form>\n");
    svr_puts(server, "    </div>\n");

    html_login_footer();
}



