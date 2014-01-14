/*
 * out-rtf.c - output functions for richtext format
 *
 * Copyright 2002 Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "config.h"
#include "svr.h"
#include "parser.h"
#include "misc.h"


/* local Prototypes */
void rtf_HeadingBegin(int level);
void rtf_HeadingEnd(int level);


enum TextModes
{
    NORM = 1,
    PAR = 2,
    PRE  = 3,
    BLK  = 4,
    FOOT  = 5,
    LIST  = 6,
    NLIST = 7
};


static int cellcnt;             /* cell count in one table row */
static int textmode = NORM;
static int blockindent = 0;

static int listindent = 0;
static int listnum[5] = {0,0,0,0,0};         /* save state for different indent levels */
static int listmode[5] = {NORM, NORM, NORM, NORM, NORM};         /* save state for different indent levels */
static int tablehead = 0;



void
static ResetParagraph()
{
    svr_printf(server, "\\pard\\plain \\nowidctlpar\\adjustright \\fs20\\lang1031\\cgrid ");
}



/*
 * rtf_Putc - outputs a string to the server
 */
void
rtf_putc(char ch)
{
    switch (ch) {
    case '\t':
        svr_putc(server, ' ');
        break;
    case 0x0d:
    case '\n':
        break;
    case '\\':
    case '{':
    case '}':
        svr_putc(server, '\\');
    default:
        svr_putc(server, ch);
    }
}



void
rtf_puts(char * str)
{
    if (str) {
        char ch;

        while ((ch = *str++)) {
            rtf_putc(ch);
        }
    }
}
/*
 * rtf_Puts - outputs a string to the server
 */
void
rtf_Puts(char * str)
{
    if (str) {
        char ch;

        while ((ch = *str++)) {
            char ch2[2];

            ch2[0] = ch;
            ch2[1] = '\0';
            switch (ch) {
            case '\t':
                svr_puts(server, " ");
                break;
            case 0x0d:
            case '\n':
                break;
            case '\\':
            case '{':
            case '}':
                svr_puts(server, "\\");
            default:
                svr_puts(server, ch2);
                break;
            }
        }
    }
}


void
rtf_page_header(Page * page, int mode)
{
    char * title = page_get_title(page);

    svr_set_contenttype(server, "text/rtf");

    svr_use_utf8(false);
    svr_puts(server,  "{\\rtf\\ansi\\deff0\n");

    /* Build the font table */
    svr_puts(server,  "{\\fonttbl");
    svr_puts(server,  "{\\f0\\fswiss Arial;}");
    svr_puts(server,  "{\\f1\\froman Times New Roman;}");
    svr_puts(server,  "{\\f2\\fmodern Courier New;}");
    svr_puts(server,  "{\\f3\\fnil\\fcharset2 Symbol;}");
    svr_puts(server,  "}\n");

    svr_printf(server, "{\\info\\title %s}{\\subject %s}{\\author %s}", title, title, page_get_ownername(page));
    svr_printf(server, "{\\keywords %s}{\\operator CuteWiki}{\\version1}", title);
    
    /* Build the colour table */
    svr_puts(server, "{\\colortbl;");
    svr_puts(server, "\\red0\\green0\\blue0;");
    svr_puts(server, "\\red0\\green0\\blue255;");
    svr_puts(server, "\\red0\\green255\\blue255;");
    svr_puts(server, "\\red0\\green255\\blue0;");
    svr_puts(server, "\\red255\\green0\\blue255;");
    svr_puts(server, "\\red255\\green0\\blue0;");
    svr_puts(server, "\\red255\\green255\\blue0;");
    svr_puts(server, "\\red255\\green255\\blue255;");
    svr_puts(server, "\\red0\\green0\\blue128;");
    svr_puts(server, "\\red0\\green128\\blue128;");
    svr_puts(server, "\\red0\\green128\\blue0;");
    svr_puts(server, "\\red128\\green0\\blue128;");
    svr_puts(server, "\\red128\\green0\\blue0;");
    svr_puts(server, "\\red128\\green128\\blue0;");
    svr_puts(server, "\\red128\\green128\\blue128;");
    svr_puts(server, "\\red192\\green192\\blue192;");
    svr_puts(server, "\\red66\\green105\\blue115;");
    svr_puts(server, "\\red222\\green231\\blue239;");
    svr_puts(server, "}\n");

    /* Build the style sheet */
    svr_puts(server, "{\\stylesheet");

    svr_puts(server, "{\\widctlpar\\adjustright \\fs20\\lang1031\\cgrid \\snext0 Normal;}\n");

    svr_puts(server, "{\\s1\\sb240\\sa60\\keepn\\widctlpar");
    svr_puts(server, "\\brdrb\\brdrs\\brdrw10\\brsp20\\outlinelevel0");
    svr_puts(server, "\\shading1000\\cbpat8\\b\\f0\\fs32\\lang1031\\kerning32");
    svr_puts(server, "\\sbasedon0 \\snext0 heading 1;}");

    svr_puts(server, "{\\s2\\sb240\\sa60\\keepn\\widctlpar");
    svr_puts(server, "\\brdrb\\brdrs\\brdrw10\\brsp20\\outlinelevel0");
    svr_puts(server, "\\b\\f0\\fs28\\lang1031");
    svr_puts(server, "\\sbasedon0 \\snext0 heading 2;}");

    svr_puts(server, "{\\s3\\sb240\\sa60\\keepn\\widctlpar");
    svr_puts(server, "\\brdrb\\brdrs\\brdrw10\\brsp20\\outlinelevel0");
    svr_puts(server, "\\b\\f0\\fs24\\lang1031");
    svr_puts(server, "\\sbasedon0 \\snext0 heading 3;}");

    svr_puts(server, "{\\s4\\sb240\\sa60\\keepn\\widctlpar");
    svr_puts(server, "\\brdrb\\brdrs\\brdrw10\\brsp20\\outlinelevel0");
    svr_puts(server, "\\b\\f0\\fs20\\lang1031");
    svr_puts(server, "\\sbasedon0 \\snext0 heading 4;}");

    svr_puts(server, "{\\*\\cs10 \\additive Default Paragraph Font;}");
    svr_puts(server, "{\\*\\cs15 \\additive \\ul\\cf2 \\sbasedon10 Hyperlink;}");
    svr_puts(server, "{\\s16\\widctlpar\\adjustright \\fs20\\lang1031\\cgrid \\sbasedon0 \\snext16 footnote text;}");
    svr_puts(server, "{\\*\\cs17 \\additive \\super \\sbasedon10 footnote reference;}");

    svr_puts(server, "{\\s21\\widctlpar\\box\\brdrs\\brdrw10 \\adjustright \\shading500\\cbpat8");
    svr_puts(server, "\\f0\\fs20\\lang1031\\cgrid \\sbasedon0 \\snext21 Fixed;}");
    svr_puts(server, "}\n");

    svr_puts(server,  "\\margl900\\margr900\\margt1100\\margb1100\n");
    svr_puts(server,  "\\paperw11906\\paperh16838\n");
    svr_puts(server,  "\\sectd\\pard\\plain\\fs20\n");

    // Header : name of the file, centered
  
    svr_puts(server,  "{\\header \\pard\\plain \\qr\\sl240\\slmult0\\nowidctlpar\\adjustright");
    svr_puts(server,  " \\f1\\fs20\\lang1031\\cgrid");
    svr_printf(server,  "{\\cgrid0 %s, }", title);
    svr_puts(server,  "{\\field{\\*\\fldinst{\\cgrid0  DATE \\\\@ \"dd.MM.yy\" }}");
    //svr_puts(server,  "{\\fldrslt {\\lang1024\\cgrid0 01.07.03}}}");
    svr_puts(server,  "{\\fldrslt }}");
    svr_puts(server,  "{\\cgrid0 \\par }}");


#if 0
    svr_puts(server,  "{\\header\\fs20\\qc\\slmult2");
    svr_printf(server,  "{%s \\par}\n", title);
    svr_puts(server,  "}\n");
#endif

    // Footer : Page n/p
#if GERMAN
    svr_puts(server,  "{\\footer\\f0\\fs20\\qc Seite\n");
#else
    svr_puts(server,  "{\\footer\\f0\\fs20\\qc Page\n");
#endif
    svr_puts(server,  "{\\field{\\*\\fldinst { PAGE }}{\\fldrslt 1}}/\n");
    svr_puts(server,  "{\\field{\\*\\fldinst { NUMPAGES }}{\\fldrslt 1}}\n");
    svr_puts(server,  "{\\par }\n");
    svr_puts(server,  "}\n");        /* end header */

    /* Print the title */
    rtf_HeadingBegin(0);
    svr_printf(server,  "%s\n", title);
    rtf_HeadingEnd(0);
}



/*
 * WriteFooter
 *
 * If title is NULL, then it is a meta page!
 */
void
rtf_page_footer(Page * page, int mode)
{
    svr_puts(server,  "}\n");        /* end file */
}


void
rtf_ParaBegin()
{
    ResetParagraph();
    textmode = PAR;
}

void
rtf_ParaEnd()
{
    svr_printf(server, "\\par\n");
    svr_printf(server, "\\par\n");
    textmode = NORM;
}

void
rtf_PreBegin()
{
    ResetParagraph();
    svr_printf(server, "\\s21\\widctlpar\\box\\brdrs\\brdrw10 \\adjustright\n");
    svr_printf(server, "\\shading500\\cbpat8 \\f2\\fs20\\lang1031\\cgrid ");
    textmode = PRE;
}

void
rtf_PreEnd()
{
    ResetParagraph();
    svr_printf(server, "\\par\n");
    textmode = NORM;
}

void
rtf_BlockquoteBegin()
{
    blockindent++;
    ResetParagraph();
    svr_printf(server, "\\fi%d ", blockindent*100);
    svr_printf(server, "\\i ");
    textmode = BLK;
}

void
rtf_BlockquoteEnd()
{
    blockindent--;
    svr_printf(server, "\\par\n");
    textmode = NORM;
}

void
rtf_RulerBegin()
{
   ResetParagraph();
    //svr_printf(server, "\\emdash\n");
}
void
rtf_RulerEnd()
{
    svr_printf(server, "\\nowidctlpar\\brdrt\\brdrs\\brdrw10\\brsp20 \\adjustright\n");
    svr_printf(server, "\\par\n");
    ResetParagraph();
}

void
rtf_ListBegin()
{
    //ResetParagraph();
    listmode[listindent]=textmode;
    listindent++;
    textmode=LIST;
    svr_printf(server, "\\pard\\fi-320\\li%d", listindent*320);
    svr_printf(server, "{\\*\\pn\\pnlvlblt\\pnf3\\pnindent%d{\\pntxtb\\'B7}}\n", listindent-1);
}

void
rtf_ListEnd()
{
    listindent--;
    textmode=listmode[listindent];
    svr_printf(server, "\\pard\\fi-320\\li%d", listindent*320);
    if (listindent == 0) {
        ResetParagraph();
        svr_printf(server, "\\par\n");
    }
    else {
        if (textmode == LIST) {
            svr_printf(server, "{\\*\\pn\\pnlvlblt\\pnf3\\pnindent%d{\\pntxtb\\'B7}}\n", listindent-1);
        }
        else {
	    svr_printf(server, "{\\*\\pn\\pnlvl%d\\pnf1\\pnindent%d\\pnstart1\\pnqr\\pndec{\\pntxta . }}\n",
		       listindent, listindent-1, listnum[listindent]);
        }
    }
}

void
rtf_NumListBegin()
{
    //ResetParagraph();
    listmode[listindent]=textmode;
    listindent++;
    textmode=NLIST;
    svr_printf(server, "\\pard\\fi-320\\li%d", listindent*320);
    svr_printf(server, "{\\*\\pn\\pnlvl%d\\pnf1\\pnindent%d\\pnstart1\\pnqr\\pndec{\\pntxta . }}\n",
                listindent, listindent-1, listnum[listindent]);
}

void
rtf_NumListEnd()
{
    listnum[listindent] = 0;
    listindent--;
    textmode=listmode[listindent];
    svr_printf(server, "\\pard\\fi-320\\li%d", listindent*320);
    if (listindent == 0) {
        ResetParagraph();
        svr_printf(server, "\\par\n");
    }
    else {
        if (textmode == LIST) {
            svr_printf(server, "{\\*\\pn\\pnlvlblt\\pnf3\\pnindent%d{\\pntxtb\\'B7}}\n", listindent-1);
        }
        else {
	    svr_printf(server, "{\\*\\pn\\pnlvl%d\\pnf1\\pnindent%d\\pnstart1\\pnqr\\pndec{\\pntxta . }}\n",
		       listindent, listindent-1, listnum[listindent]);
        }
    }
}

void
rtf_ListItemBegin()
{
    if (textmode == LIST) {
        svr_printf(server, "{\\pntext\\f3\\bullet\\tab}", listnum[listindent]);
    }
    else {
        listnum[listindent]++;
        svr_printf(server, "{\\pntext\\f1 %d. \\tab}", listnum[listindent]);
    }
}

void
rtf_ListItemEnd()
{
    svr_printf(server, "\\par\n");
}

void
rtf_LineBegin()
{
}

void
rtf_LineEnd()
{
    switch (textmode) {
    case BLK:
    case PAR:
        svr_printf(server, " ");
        break;
    case PRE:
        svr_printf(server, "\\par\n");
        break;
    case FOOT:
    case LIST:
    case NLIST:
        break;
    }
}

void
rtf_HeadingBegin(int level)
{
    ResetParagraph();
    svr_printf(server, "\\pard\\keepn\\sb240\\sa60\\f0");
    svr_puts(server, "\\brdrb\\brdrs\\brdrw10\\brsp20\\outlinelevel0");
    switch (level) {
    case 0:
        svr_printf(server, "\\shading1000\\cbpat8\\s1\\fs32\\kerning32\\b ");
        break;
    case 1:
        svr_printf(server, "\\s2\\fs28\\b ");
        break;
    case 2:
        svr_printf(server, "\\s3\\fs24\\b ");
        break;
    default:
        svr_printf(server, "\\s4\\fs20\\b ");
    }
}

void
rtf_HeadingEnd(int level)
{
    svr_printf(server, "\\par\n");
    ResetParagraph();
}

/*
 * This link should also set the referenced text later. Word will show it
 * automatically at the bottom of the page. No idea, how to handle this
 * easily...
 */
void
rtf_Footnote(char * footnote)
{
    svr_printf(server, "{\\cs17\\f1\\super\\cgrid0 \\chftn ");
    svr_printf(server, "{\\footnote \\pard\\plain \\s16\\widctlpar\\adjustright ");
    svr_printf(server, "\\fs20\\lang1031\\cgrid {\\cs17\\super \\chftn }{ %s}}}", footnote);
}

void
rtf_BoldBegin()
{
    svr_printf(server, "{\\b ");
}

void
rtf_BoldEnd()
{
    svr_printf(server, "}");
}

void
rtf_ItalicBegin()
{
    svr_printf(server, "{\\i ");
}

void
rtf_ItalicEnd()
{
    svr_printf(server, "}");
}

void
rtf_InternalLink(char * title, char * atitle, Pagetype type)
{
    svr_printf(server, "%s", atitle);
}

void
rtf_BrokenLink(char * title)
{
    svr_printf(server, "%s", title);
}

void
rtf_TableBegin(int cells)
{
    cellcnt = cells;
    //svr_printf(server, "\\par\n");
    ResetParagraph();
}

void
rtf_TableEnd()
{
    svr_printf(server, "\\pard \\widctlpar\\adjustright\n");
    svr_printf(server, "\\par\n");
}

void
rtf_TableRowBegin()
{
    int i;

    //svr_printf(server, "\\trowd \\trgaph50\\trleft-50\n");
    svr_printf(server, "\\trowd \\trgaph50\n");
    svr_printf(server, "\\trbrdrt\\brdrs\\brdrw10\n");
    svr_printf(server, "\\trbrdrl\\brdrs\\brdrw10\n");
    svr_printf(server, "\\trbrdrb\\brdrs\\brdrw10\n");
    svr_printf(server, "\\trbrdrr\\brdrs\\brdrw10\n");
    svr_printf(server, "\\trbrdrh\\brdrs\\brdrw10\n");
    svr_printf(server, "\\trbrdrv\\brdrs\\brdrw10\n");

    for (i=1; i <= cellcnt; i++) {
        svr_printf(server, "\\clvertalt\n");
        svr_printf(server, "\\clbrdrt\\brdrs\\brdrw10\n");
        svr_printf(server, "\\clbrdrl\\brdrs\\brdrw10\n");
        svr_printf(server, "\\clbrdrb\\brdrs\\brdrw10\n");
        svr_printf(server, "\\clbrdrr\\brdrs\\brdrw10\n");
        svr_printf(server, "\\cltxlrtb \\cellx%d", 10110*i/cellcnt);
    }
    svr_printf(server, "\n\\pard \\widctlpar\\intbl\\adjustright {\n");
}

void
rtf_TableRowEnd()
{
    svr_printf(server, "\n}\\pard \\widctlpar\\intbl\\adjustright \\row\n");
}

void
rtf_TableHeadBegin()
{
    int i;

    svr_printf(server, "\\trowd \\trgaph50\n");
    svr_printf(server, "\\trbrdrt\\brdrs\\brdrw10\n");
    svr_printf(server, "\\trbrdrl\\brdrs\\brdrw10\n");
    svr_printf(server, "\\trbrdrb\\brdrs\\brdrw10\n");
    svr_printf(server, "\\trbrdrr\\brdrs\\brdrw10\n");
    svr_printf(server, "\\trbrdrh\\brdrs\\brdrw10\n");
    svr_printf(server, "\\trbrdrv\\brdrs\\brdrw10\n");

    for (i=1; i <= cellcnt; i++) {
        svr_printf(server, "\\clvertalt\n");
        svr_printf(server, "\\clbrdrt\\brdrs\\brdrw10\n");
        svr_printf(server, "\\clbrdrl\\brdrs\\brdrw10\n");
        svr_printf(server, "\\clbrdrb\\brdrs\\brdrw10\n");
        svr_printf(server, "\\clbrdrr\\brdrs\\brdrw10\n");
        svr_printf(server, "\\clcbpat16\\cltxlrtb \\cellx%d", 10110*i/cellcnt);
    }
    svr_printf(server, "\n\\pard \\widctlpar\\intbl\\adjustright{\n");
    tablehead = 1;
}

void
rtf_TableHeadEnd()
{
    svr_printf(server, "\n}\\pard \\widctlpar\\intbl\\adjustright\\row\n");
    tablehead = 0;
}

void
rtf_TableCellBegin()
{
    svr_printf(server, "\\pard \\widctlpar\\intbl\\adjustright{");
    if (tablehead)
        svr_printf(server, "\\b ");
}

void
rtf_TableCellEnd()
{
    svr_printf(server, "\\cell }");
}

void
rtf_TableNumberBegin()
{
    svr_printf(server, "\\pard \\qr\\widctlpar\\intbl\\adjustright{\\cf2 ");
}

void
rtf_TableNumberEnd()
{
    svr_printf(server, "\\cell }");
}



void
rtf_url(char * url)
{
    if (!memcmp(url, "mailto:", 7)) {
	svr_printf(server, "{\\field\\fldedit{\\*\\fldinst {\\f0\\cgrid0 HYPERLINK \"%s\"}}", url);
	svr_printf(server, "{\\fldrslt {\\ul\\cf2 %s}}}", url + 7);
    }
    else {
	svr_printf(server, "{\\field\\fldedit{\\*\\fldinst {\\f0\\cgrid0 HYPERLINK \"%s\" \\\\*\\f0\\cgrid0 MERGEFORMAT }}}", url);
    }
}



void
rtf_external_link(char * url, char * text)
{
    svr_printf(server, "{\\field\\fldedit{\\*\\fldinst {\\f0\\cgrid0 HYPERLINK \"%s\"}}", url);
    svr_printf(server, "{\\fldrslt {\\ul\\cf2 %s}}}", text);
}



void
rtf_image_url(char * url)
{
    svr_printf(server, "{\\field\\fldedit{\\*\\fldinst {\\f0\\cgrid0 HYPERLINK \"%s\" \\\\*\\f0\\cgrid0 MERGEFORMAT }}}", url);
}

void
rtf_image(char * image)
{
}



/* pluggable output module for HTML */
Output rtf = {
    rtf_putc,
    rtf_puts,
    rtf_page_header,
    rtf_page_footer,
    rtf_ParaBegin,
    rtf_ParaEnd,
    rtf_PreBegin,
    rtf_PreEnd,
    rtf_BlockquoteBegin,
    rtf_BlockquoteEnd,
    rtf_RulerBegin,
    rtf_RulerEnd,
    rtf_ListBegin,
    rtf_ListEnd,
    rtf_NumListBegin,
    rtf_NumListEnd,
    rtf_ListItemBegin,
    rtf_ListItemEnd,
    rtf_LineBegin,
    rtf_LineEnd,
    rtf_HeadingBegin,
    rtf_HeadingEnd,
    rtf_Footnote,
    rtf_BoldBegin,
    rtf_BoldEnd,
    rtf_ItalicBegin,
    rtf_ItalicEnd,
    rtf_InternalLink,
    rtf_BrokenLink,
    rtf_TableBegin,
    rtf_TableEnd,
    rtf_TableHeadBegin,
    rtf_TableHeadEnd,
    rtf_TableRowBegin,
    rtf_TableRowEnd,
    rtf_TableCellBegin,
    rtf_TableCellEnd,
    rtf_TableNumberBegin,
    rtf_TableNumberEnd,
    rtf_image,
    rtf_url,
    rtf_image_url,
    rtf_external_link
};


