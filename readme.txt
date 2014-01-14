=== About

CuteWiki is a standalone wiki engine implemented in ANSI-C. It is just
known to run on Linux, although it was tested to run on HP-UX in an
earlier version. Although it communicates with the webbrowser via UTF-8
characterset, it is just able to store data in latin1 - unknown
characters are replaced by "_".

=== Installation

To install cutewiki, just do the following:

Switch your locale setting to iso latin1, like for example:

export LANG=de_DE.ISO-8859-1 or
export LANG=de_DE.ISO-8859-15

Create a sub-directory in the homedirectory of the user, which will run
the cutewiki:

mkdir ~/.ini

In this directory create a config file, like explained in the
Configuration section of this readme.

Create all needed directories given in the config file and put in it the
content of the directories files, images and words. Words is optional.
This is the only wiki with support in it.   :-)

=== Compilation

Go into the cutewiki/src directory and have a look at the config.h and
the Makefile. Type make.

=== Configuration

[General]
description = Martin's Wiki
hostname = stunk
port = 8090

[Files]
pagedir = /home/martin/cutewiki/mdoering
filedir = /home/martin/cutewiki/files
wordsdir = /home/martin/cutewiki/words
imagedir = /home/martin/cutewiki/images
errorlog = /home/martin/cutewiki/logs/mdoering-err.log
accesslog = /home/martin/cutewiki/logs/mdoering-acc.log

[Administration]
admin = MartinDoering
admin = WikiAdmin

The General section will give the wiki a name and configure the port.
The hostname is just for internal reference.

The Files section will provide the information, where the different
files will be found. The pagedir will hold the wiki pages and the
metainformation files. The filedir holds some things the wiki needs to
run, like css stylesheets, icons and so on.

The Administration section will tell the wiki engine, which users will
be allowed to do a password reset for others. The initial password for
each new User is "wikiwiki". Users can be created by every other user.


=== Startup

You can run cutewiki like:

./cutewiki mywiki

if the config file was called mywiki.ini or put it into the path. Cutewiki will tell you, if something is wrong.

The wiki engine will create all essential pages itself. There are no
artificial metapages. After creation all pages can be changed. If an
essential page id deleted accidently, it will be recreated. If RCS is
installed it will be sensed and will be used. You should create a RCS
subdirectory in your pagedir to hold the version information.
 
=== Security

The cutewiki software is just used inhouse in my company for now. Users
in it are just used to know, who is who. It is not safe in any kind.
Passwords are transmitted in clear text, but saved DES encypted and
sored in a cookie and in the users metafile

There may be many bugs an security hole in cuitewiki. In our company it
is used by about 50 users, 5 of them every day. It has run for many
month without a crash, but this may mean nothing. Don't drive it in an
environment, where you have to fight for your data, like the internet or
such. The wiki does not provide spam blocking, can not block ip
addresses and such. Although it may contain buffer overruns and other
shortcomings of the C language, although I did run it with tcc's memory
pointer range checker compiled in.

=== Documentation

There is some documentation, but for now just in german language. 

=== Future

Cutewiki will be developed slowly, but will never be a feature monster.
It will be kept a wiki in the original sense and not be developed in a
groupware direction. It is though for personal use of for use in your
company, not to replace your CMS, DMS or groupware. It is text only,
until somebody feels to extend the webserver library to handle
fileuploads.


=== You and me

Please report any problems you had with cutewiki to
cutewiki@datenbrei.de. I will try to fix things. Don't ask me for
features: Implement or forget them. I'm pretty satisfied with cutewiki.
:-)

In no case I will be responsable for any problems, data loss or other
bad things that may happen, driving the cutewiki engine. Do it on your
own risc. 

=== Licensing

Cutewiki is licensed under the Gnu General Public License, like given in
the doc directory.

