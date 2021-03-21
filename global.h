#ifndef __GLOBAL_H__
#define __GLOBAL_H__

/* screen display info */
extern long first_par;     /* text offset of start of screen */
extern long first_line;    /* line number in first_par at top of window */

/* cursor stuff */
extern int  cur_row;       /* cursor's line on screen */
extern long cur_line;      /* text offset of start of cursor's line */
extern int  cur_col;       /* position of cursor in that line */
extern int  cur_len;       /* length of that line */
extern long cur_index;     /* position of cursor in document */

/* selected text region; if equal, none selected */
extern long sel_start;     /* the first character in the range */
extern long sel_end;       /* the first character out of the range */

#endif