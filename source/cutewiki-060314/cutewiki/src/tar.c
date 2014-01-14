/*
 * tar.c - generate tar files from within the wiki
 *
 * Copyright 2003 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include "types.h"
#include "cutewiki.h"
#include "page.h"
#include "page_list.h"
#include "svr.h"

/* definitions for tar */
#define TAR_MAGIC          "ustar"	/* magic tar string */
#define TAR_VERSION        "  "	        /* version string */
#define TAR_BLOCK_SIZE 512
#define TAR_MAGIC_LEN  6
#define TAR_VERSION_LEN 2
#define NAME_SIZE 100



/* tar header block */
typedef struct TarHeader TarHeader;
struct TarHeader {		/* byte offset */
    char name[NAME_SIZE];	/*   0- 99 */
    char mode[8];		/* 100-107 */
    char uid[8];		/* 108-115 */
    char gid[8];		/* 116-123 */
    char size[12];		/* 124-135 */
    char mtime[12];		/* 136-147 */
    char chksum[8];		/* 148-155 */
    char typeflag;		/* 156-156 */
    char linkname[NAME_SIZE];	/* 157-256 */
    char magic[6];		/* 257-262 */
    char version[2];		/* 263-264 */
    char uname[32];		/* 265-296 */
    char gname[32];		/* 297-328 */
    char devmajor[8];		/* 329-336 */
    char devminor[8];		/* 337-344 */
    char prefix[155];		/* 345-499 */
    char unused[12];		/* 500-512 (fillup till TAR_BLOCK_SIZE) */
};



/*
 * Put an octal string into the specified buffer.
 *
 * The number is zero and space padded and possibly null padded.
 *
 * Returns true if successful.
 */
static int
to_octal(char *cp, int len, long value)
{
    int tempLength;
    char tempBuffer[32];
    char *tempString = tempBuffer;

    /* Create a string of the specified length with an initial space,
     * leading zeroes and the octal number, and a trailing null.  */
    sprintf(tempString, "%0*lo", len - 1, value);

    /* If the string is too large, suppress the leading space.  */
    tempLength = strlen(tempString) + 1;
    if (tempLength > len) {
        tempLength--;
        tempString++;
    }

    /* If the string is still too large, suppress the trailing null.  */
    if (tempLength > len)
        tempLength--;

    /* If the string is still too large, fail.  */
    if (tempLength > len)
        return false;

    /* Copy the string to the field.  */
    memcpy(cp, tempString, len);

    return true;
}



/* Write a header for each file */
static int
tar_write_header(Page * page)
{
    char filename[MAX_PATH];
    char fn[MAX_PATH];
    long chksum = 0;
    struct TarHeader header;
    const unsigned char *cp = (const unsigned char *) &header;
    ssize_t size = sizeof(struct TarHeader);
    struct stat fstat;

    memset(&header, 0, size);

    sprintf(filename, "%s.wik", page_get_name(page) );
    if (strlen(filename) >= NAME_SIZE) {
        return true;
    }
    /* Get size information from file entries in directory */
    page_get_textfilename(page, fn);
    stat(fn, &fstat);

    strncpy(header.name, filename, sizeof(header.name));

    to_octal(header.mode, sizeof(header.mode), fstat.st_mode);
    to_octal(header.uid, sizeof(header.uid), fstat.st_uid);
    to_octal(header.gid, sizeof(header.gid), fstat.st_gid);
    to_octal(header.size, sizeof(header.size), fstat.st_size );
    to_octal(header.mtime, sizeof(header.mtime), fstat.st_mtime );
    strncpy(header.magic, TAR_MAGIC TAR_VERSION, TAR_MAGIC_LEN + TAR_VERSION_LEN);

    /* user and group names */
    strcpy(header.uname, "wiki");
    strcpy(header.gname, "wiki");
    header.typeflag = '0';      /* we write a regular file */

    /* Calculate  the checksum */
    memset(header.chksum, ' ', sizeof(header.chksum));
    cp = (const unsigned char *) &header;
    while (size-- > 0)
        chksum += *cp++;
    to_octal(header.chksum, 7, chksum);

    /* Write the header to client */
    svr_set_contenttype(server, "application/tar");
    svr_send_binary(server, (char *) &header, sizeof(struct TarHeader));

    return (true);
}



/*
 * tar_write_file - writes a single page to the archive
 */
static void
tar_write_file(Page * page)
{
    char * text;
    bool loaded;

    if (!page_is_seen(page))
        return;

    /* archive the text as a regular file */
    page_load_text(page, &loaded);
    text = page_get_text(page);
    if (text != NULL) {
	ssize_t size = 0, blocksize = 0;

	tar_write_header(page);
	size = strlen(text);
	svr_send_binary(server, text, size);
	blocksize += size;;

	/* Pad the file up to the tar block size */
	for (; (blocksize % TAR_BLOCK_SIZE) != 0; blocksize++)
	    svr_putc(server, '\0');
    }
    page_unload_text(page, loaded);
}



/*
 * tar_create_tarfile - writes the whole tar-archive
 */
void
tar_create_tarfile(char * filename)
{
    Page** list = pagelist_alpha_sorted();
    size_t i, size;

    svr_use_utf8(false);        /* use iso-8859-1 encoding */

    /* Iterate over the pages and output one at a time */
    for (i = 0; list[i] != NULL; i++) {
        tar_write_file(list[i]);
    }

    /* Write two empty blocks to the end of the archive */
    for (size = 0; size < (2 * TAR_BLOCK_SIZE); size++)
	svr_putc(server, '\0');
    free(list);
}
