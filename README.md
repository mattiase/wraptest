# wraptest

Nobody seems to know exactly how line-wrapping works in actual text
terminals of the VT100/VT220 kind, and terminal emulators disagree
with one another.

This little test program can be used to find out some of the details.
Run it on a terminal or terminal emulator, and it will spit out a
report. (Stdout can be redirected for the report; real terminals
usually do not have text copy-paste functions.)

Currently known results are [here](results.txt). If you have results
for terminals or emulators not mentioned in that file, please send
them to me.
