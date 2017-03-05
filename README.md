# DEC VT terminal line-wrapping semantics

One aspect of [DEC VT terminals](http://vt100.net), such as the
[VT100](https://en.wikipedia.org/wiki/VT100) and
[VT220](https://en.wikipedia.org/wiki/VT220), that keeps puzzling
users is what happens when text goes beyond the edge of a line into
the next. It turns out that the exact details are not common
knowledge.

Terminal emulators, like
[XTerm](http://invisible-island.net/xterm/xterm.html) and
[PuTTY](http://www.putty.org), usually try to follow the behaviour of
the original hardware as closely as possible. Since the rules are
rarely documented and genuine hardware sometimes difficult to get hold of,
emulator authors resort to guessing or inventing their own rules,
something that does not necessarily serve the best interest of their
users.

The sources for the following text are the actual terminals and DEC's
internal reference documentation [DEC STD-070 Video Systems Reference
Manual](http://bitsavers.trailing-edge.com/pdf/dec/standards/EL-SM070-00_DEC_STD_070_Video_Systems_Reference_Manual_Dec91.pdf)
(71 MB).

## Basic VT line-wrapping rules

DEC VT terminals use a mechanism sometimes called "soft-wrap",
"deferred wrap" or the "VT100 glitch", mainly to permit full-length
newline-terminated text lines to be written without being followed by
unsightly blank lines. This is what complicates matters.

The auto-wrap mode flag, DECAWM, controls whether text will wrap
around the edges at all. It can be set and cleared using the DECSET
and DECRST control sequences; terminal emulators usually get this
bit right. In the following text, we assume that auto-wrap is enabled
(DECAWM set).

When the terminal receives a printing character and the cursor is in
the rightmost column, what happens depends on the hidden Last Column
Flag:

* If the LCF is 0, then the character is drawn at the cursor position
  as usual, but instead of advancing the cursor afterwards, the LCF is
  set to 1.
* If the LCF is 1, then the cursor is moved to the first column of the
  next line, scrolling if necessary. Then LCF is set to 0, the
  character is drawn, and the cursor advanced as usual.

The Last Column flag starts out as 0 when the terminal is powered on.

## Last Column Flag specifics

Only plain character output in the rightmost column is affected by the
LCF; no other operation is. In particular, the Cursor Position Report
(CPR) sends the same coordinates regardless of the LCF.

The following rules are all according to DEC's documentation, but not
all DEC terminals obey them completely.

### Save/Restore Cursor

The LCF is saved/restored by the Save/Restore Cursor (DECSC/DECRC)
control sequences. The DECAWM flag is _not_ included in the state
managed by these operations.

### Resetting operations

The LCF is reset by the following operations (from STD-070):

* Plain character output, as describe above
* Set top and bottom margins (DECSTBM)
* Change line width/height (DECSWL, DECDWL, DECDHL)
* Set column mode, origin mode (DECSET DECCOLM, DECOM)
* Reset column mode, origin mode, auto-wrap mode
  (DECRST DECCOLM, DECOM, DECAWM)
* Cursor up, down, forward, backward (CUU, CUD, CUF, CUB)
* Cursor position (CUP, HVP)
* Backspace (BS)
* Horizontal tab, vertical tab (HT, VT)
* Carriage return, line feed (CR, LF)
* Form feed (FF)
* Substitute (SUB)
* Index, reverse index (IND, RI)
* Next line (NEL)
* Erase, delete, insert character (ECH, DCH, ICH)
* Erase in line, in display (EL, ED)
* Selective erase in line, in display (DECSEL, DECSED)

Since this part of STD-070 is from 1985, operations introduced in
newer terminals are not present in this list.

### Problematic operations

The fact that TAB is an LCF-resetting operation can have negative
consequences.  Consider a text file containing a line longer than the
width of the terminal.  If the file is sent unmodified to the
terminal, the long line will wrap, but if it contains an ASCII TAB
character at an unfortunate position, the result can be the visual
disappearance of a printing character.

GNU Grep uses EL (Erase in Line) in an attempt to control the
background colour. Since EL resets LCF, this can lead to characters
going missing (see bug reports
[here](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=712024),
[here](https://bugzilla.redhat.com/show_bug.cgi?id=1006310) and
[here](https://debbugs.gnu.org/cgi/bugreport.cgi?bug=15444)).

## Actual hardware and emulators

The VT100 predates the rules laid out in STD-070, so it diverges from
them considerably. The VT220 appears to follow the specification to
the letter, whereas the much more recent VT510 differs on a few
points, perhaps on purpose.

No emulator tested so far matches either actual hardware or the
specification in STD-070, but some are clearly better than others.

# Test program

[This little test program](wraptest.c) can be used to examine how a
terminal or emulator behaves with respect to line-wrapping. Run it in
the terminal, and it will spit out a report. (Stdout can be redirected
for the report; real terminals usually do not have text copy-paste
functions.)

Currently known results are [here](results.txt). If you have results
for terminals or emulators not mentioned in that file, please send
them to me.
