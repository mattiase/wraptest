/* Tests of VTxxx wrapping semantics.
   A report is printed to stdout, which can be redirected
   (all tty output goes to stderr). */

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <stdarg.h>

/* Set to 1 to skip tests that require a VT220 or above. */
#define VT100_ONLY 0

/* Write to tty (stderr). */
static void wr(const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vfprintf(stderr, fmt, va);
  va_end(va);
}

/* Cursor Position */
static void cup(int row, int col) {
  wr("\33[%d;%dH", row, col);
}

/* Auto-Wrap Mode */
static void decawm(int on) {
  wr("\33[?7%c", on ? 'h' : 'l');
}

/* Request and return current cursor position. */
static void getpos(int *row, int *col) {
  wr("\33[6n");
  if (scanf("\33[%d;%dR", row, col) != 2) {
    fprintf(stderr, "\r\nerror: no cursor position reply\r\n");
    exit(1);
  }
}

typedef enum {
  no_info = -1,
  no = 0,
  yes = 1,
} tristate_t;

static tristate_t wrap_works = no_info;
static tristate_t wrap_is_deferred = no_info;
static tristate_t cpr_beyond_last_col = no_info;
static tristate_t cr_works_at_margin = no_info;
static tristate_t bs_works_at_margin = no_info;
static tristate_t tab_wraps_at_margin = no_info;
static tristate_t tab_cancels_wrap = no_info;
static tristate_t nl_cancels_wrap = no_info;
static tristate_t nul_cancels_wrap = no_info;
static tristate_t bel_cancels_wrap = no_info;
static tristate_t ri_cancels_wrap = no_info;
static tristate_t sgr_cancels_wrap = no_info;
static tristate_t sm_cancels_wrap = no_info;
static tristate_t cup_cancels_wrap = no_info;
static tristate_t cuf_cancels_wrap = no_info;
static tristate_t el_cancels_wrap = no_info;
static tristate_t ed_cancels_wrap = no_info;
static tristate_t dch_cancels_wrap = no_info;
static tristate_t ich_cancels_wrap = no_info;
static tristate_t ech_cancels_wrap = no_info;
static tristate_t cpr_cancels_wrap = no_info;
static tristate_t decsc_cancels_wrap = no_info;
static tristate_t decrc_restores_wrap = no_info;
static tristate_t decrc_restores_decawm_on = no_info;
static tristate_t decrc_restores_decawm_off = no_info;

static void do_tests(void) {
  int r, c, width, wrap_col;
  decawm(1);
  wr("\33[2J");			/* Clear screen. */
  cup(1, 999);
  getpos(&r, &width);
  
  /* Check that wrap works. */
  cup(1, width - 1);
  wr("ABC");
  getpos(&r, &c);
  wrap_works = (r == 2 && c == 2);

  /* Check that wrap is deferred after writing to the last column. */
  cup(1, width - 1);
  wr("AB");
  getpos(&r, &wrap_col);
  wrap_is_deferred = (r == 1 && wrap_col >= width);

  /* Whether CPR reports a position beyond the last column in the wrap state. */
  cpr_beyond_last_col = (wrap_col > width);

  /* Check that CR works after writing to the last column. */
  cup(1, width - 1);
  wr("AB\r");
  getpos(&r, &c);
  cr_works_at_margin = (r == 1 && c == 1);

  /* Check that BS works after writing to the last column. */
  cup(1, width - 1);
  wr("AB\b");
  getpos(&r, &c);
  bs_works_at_margin = (r == 1 && c == width - 1);

  /* Check whether TAB wraps after writing to the last column. */
  cup(1, width - 1);
  wr("AB\t");
  getpos(&r, &c);
  tab_wraps_at_margin = (r == 2);

  /* Check whether TAB cancels the wrap state. */
  cup(1, width - 1);
  wr("AB\tC");
  getpos(&r, &c);
  tab_cancels_wrap = (r == 1 && c >= width);

  /* Check that NL cancels the wrap state. */
  cup(1, width - 1);
  wr("AB\nC");
  getpos(&r, &c);
  nl_cancels_wrap = (r == 2 && c == width);

  /* Check whether NUL cancels the wrap state. */
  cup(1, width - 1);
  wr("AB%cC", 0);
  getpos(&r, &c);
  nul_cancels_wrap = (r == 1 && c >= width);

  /* Check whether BEL cancels the wrap state. */
  cup(1, width - 1);
  wr("AB\aC");
  getpos(&r, &c);
  bel_cancels_wrap = (r == 1 && c >= width);

  /* Check whether RI (Reverse Index) cancels the wrap state. */
  cup(2, width - 1);
  wr("AB\33MC");
  getpos(&r, &c);
  ri_cancels_wrap = (r == 1 && c >= width);

  /* Check whether SGR (Select Graphic Rendition) cancels the wrap state. */
  cup(1, width - 1);
  wr("AB\33[mC");
  getpos(&r, &c);
  sgr_cancels_wrap = (r == 1 && c >= width);

  /* Check whether SM (Set Mode) cancels the wrap state. */
  cup(1, width - 1);
  wr("AB\33[hC");
  getpos(&r, &c);
  sm_cancels_wrap = (r == 1 && c >= width);

  /* Check whether CUP (Set Cursor Position) cancels the wrap state. */
  cup(1, width - 1);
  wr("AB");
  cup(1, width);
  wr("C");
  getpos(&r, &c);
  cup_cancels_wrap = (r == 1 && c >= width);

  /* Check whether CUF (Cursor Forward) cancels the wrap state. */
  cup(1, width - 1);
  wr("AB\33[CC");
  getpos(&r, &c);
  cuf_cancels_wrap = (r == 1 && c >= width);

  /* Check whether EL (Erase in Line) cancels the wrap state. */
  cup(1, width - 1);
  wr("AB\33[KC");
  getpos(&r, &c);
  el_cancels_wrap = (r == 1 && c == width);

  /* Check whether ED (Erase in Display) cancels the wrap state. */
  cup(1, width - 1);
  wr("AB\33[JC");
  getpos(&r, &c);
  ed_cancels_wrap = (r == 1 && c >= width);

  /* Check whether DCH (Delete Character) cancels the wrap state. */
  cup(1, width - 1);
  wr("AB\33[PC");
  getpos(&r, &c);
  dch_cancels_wrap = (r == 1 && c >= width);

  if (!VT100_ONLY) {
    /* Check whether ICH (Insert Character) cancels the wrap state. */
    cup(1, width - 1);
    wr("AB\33[@C");
    getpos(&r, &c);
    ich_cancels_wrap = (r == 1 && c >= width);

    /* Check whether ECH (Erase Character) cancels the wrap state. */
    cup(1, width - 1);
    wr("AB\33[XC");
    getpos(&r, &c);
    ech_cancels_wrap = (r == 1 && c >= width);
  }

  /* Check whether CPR (Cursor Position Report) cancels the wrap state. */
  cup(1, width - 1);
  wr("AB");
  getpos(&r, &c);
  wr("C");
  getpos(&r, &c);
  cpr_cancels_wrap = (r == 1 && c >= width);

  /* Check whether DECSC (Save Cursor) cancels the wrap state. */
  cup(1, width - 1);
  wr("AB\33" "7" "C");
  getpos(&r, &c);
  decsc_cancels_wrap = (r == 1 && c >= width);

  /* Check whether DECRC (Restore Cursor) restores the wrap state. */
  cup(1, width - 1);
  wr("AB\33" "7");
  cup(3, 10);
  wr("Q\33" "8" "X");
  getpos(&r, &c);
  decrc_restores_wrap = (r == 2);

  /* Check whether DECRC (Restore Cursor) restores DECAWM=on. */
  cup(1, 1);
  wr("\33" "7");
  decawm(0);
  wr("\33" "8"); 
  cup(1, width - 1);
  wr("ABC");
  getpos(&r, &c);
  decrc_restores_decawm_on = (r == 2);
  decawm(1);

  /* Check whether DECRC (Restore Cursor) restores DECAWM=off. */
  cup(1, 1);
  decawm(0);
  wr("\33" "7");
  decawm(1);
  wr("\33" "8"); 
  cup(1, width - 1);
  wr("ABC");
  getpos(&r, &c);
  decrc_restores_decawm_off = (r == 1);
  decawm(1);
}

static void rep(const char *name, tristate_t value) {
  printf("%-40s %s\n", name, value == yes ? "yes" : value == no ? "no" : "-");
}

static void report(void) {
  wr("\33[H\33[2J");
  rep("1.  wrap works", wrap_works);
  rep("2.  wrap is deferred", wrap_is_deferred);
  rep("3.  CPR beyond last column", cpr_beyond_last_col);
  rep("4.  CR works at margin", cr_works_at_margin);
  rep("5.  BS works at margin", bs_works_at_margin);
  rep("6.  TAB wraps at margin", tab_wraps_at_margin);
  rep("7.  TAB cancels wrap", tab_cancels_wrap);
  rep("8.  NL cancels wrap", nl_cancels_wrap);
  rep("9.  NUL cancels wrap", nul_cancels_wrap);
  rep("10. BEL cancels wrap", bel_cancels_wrap);
  rep("11. RI cancels wrap", ri_cancels_wrap);
  rep("12. SGR cancels wrap", sgr_cancels_wrap);
  rep("13. SM cancels wrap", sm_cancels_wrap);
  rep("14. CUP cancels wrap", cup_cancels_wrap);
  rep("15. CUF cancels wrap", cuf_cancels_wrap);
  rep("16. EL cancels wrap", el_cancels_wrap);
  rep("17. ED cancels wrap", ed_cancels_wrap);
  rep("18. DCH cancels wrap", dch_cancels_wrap);
  rep("19. ICH cancels wrap", ich_cancels_wrap);
  rep("20. ECH cancels wrap", ech_cancels_wrap);
  rep("21. CPR cancels wrap", cpr_cancels_wrap);
  rep("22. DECSC cancels wrap", decsc_cancels_wrap);
  rep("23. DECRC restores wrap", decrc_restores_wrap);
  rep("24. DECRC restores decawm=on", decrc_restores_decawm_on);
  rep("25. DECRC restores decawm=off", decrc_restores_decawm_off);
}

int main(void) {
  /* Set the tty to a fairly raw mode, to avoid interference from
     local editing, echo, or newline transformation. */
  struct termios saved, raw;
  if (tcgetattr(0, &saved) == -1) {
    perror("tcgetattr");
    return 1;
  }
  raw = saved;
  raw.c_lflag &= ~(ECHO | ECHONL | ICANON);  /* No input editing or echo. */
  raw.c_oflag &= ~ OPOST;		     /* No output post-processing. */
  if (tcsetattr(0, TCSANOW, &raw) == -1) {
    perror("tcsetattr");
    return 1;
  }

  /* Make stdin and stderr unbuffered. */
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  do_tests();

  if (tcsetattr(0, TCSANOW, &saved) == -1) {
    perror("tcsetattr");
    return 1;
  }

  report();

  return 0;
}
