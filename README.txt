Details on natively building TEMINCOM.COM (based on TE).

Installion of ZSM and MESCC
===========================

Needed for building TEMINCOM (based on TE).

Checkout the master branch of MESCC (contains ZSM) on the FAT
partition of the SD card.

git clone https://github.com/MiguelVis/mescc.git

Boot the SC126 into CPM 2.2
Copy the pre-built COM files into the I: drive by doing:

Drive 4 is the SD card
Drive 2 is the CF card

c>FAT COPY 4:MESCC/*.COM I:

Copying...

4:MESCC/CCOPT.COM ==> I:CCOPT.COM ... [OK]
4:MESCC/HELLO.COM ==> I:HELLO.COM ... [OK]
4:MESCC/HEXTOCOM.COM ==> I:HEXTOCOM.COM ... [OK]
4:MESCC/ZSM.COM ==> I:ZSM.COM ... [OK]
4:MESCC/CC.COM ==> I:CC.COM ... [OK]

    5 File(s) Copied

Then copy the header files into the C source code disk:
C>FAT COPY 4:MESCC/*.H J:

Copying...

4:MESCC/CCOPT.H ==> J:CCOPT.H ... [OK]
4:MESCC/ALLOC.H ==> J:ALLOC.H ... [OK]
4:MESCC/ATEXIT.H ==> J:ATEXIT.H ... [OK]
4:MESCC/BSEARCH.H ==> J:BSEARCH.H ... [OK]
4:MESCC/CLOCK.H ==> J:CLOCK.H ... [OK]
4:MESCC/CONIO.H ==> J:CONIO.H ... [OK]
4:MESCC/CPM.H ==> J:CPM.H ... [OK]
4:MESCC/CTYPE.H ==> J:CTYPE.H ... [OK]
4:MESCC/FILEIO.H ==> J:FILEIO.H ... [OK]
4:MESCC/FPRINTF.H ==> J:FPRINTF.H ... [OK]
4:MESCC/MEM.H ==> J:MEM.H ... [OK]
4:MESCC/MESCC.H ==> J:MESCC.H ... [OK]
4:MESCC/PRINTF.H ==> J:PRINTF.H ... [OK]
4:MESCC/QSORT.H ==> J:QSORT.H ... [OK]
4:MESCC/RAND.H ==> J:RAND.H ... [OK]
4:MESCC/REDIR.H ==> J:REDIR.H ... [OK]
4:MESCC/SETJMP.H ==> J:SETJMP.H ... [OK]
4:MESCC/SPRINTF.H ==> J:SPRINTF.H ... [OK]
4:MESCC/STDBOOL.H ==> J:STDBOOL.H ... [OK]
4:MESCC/STRING.H ==> J:STRING.H ... [OK]
4:MESCC/XPRINTF.H ==> J:XPRINTF.H ... [OK]
4:MESCC/Z80.H ==> J:Z80.H ... [OK]

    22 File(s) Copied


Building my version of the TE editor: TEMINCOM.
===============================================

Checkout the master branch of MESCC (contains ZSM) on the FAT
partition of the SD card.

On the SD card using Linux do:

git clone https://github.com/skullandbones/te.git
cd te
git checkout -b v1.60_plus_temincom origin/v1.60_plus_temincom

Reinsert the SD card into the SC126.

Boot into CP/M 2.2 and do:

C:
FAT COPY 4:TE/TEMINCOM.C J:
FAT COPY 4:TE/*.C J:
FAT COPY 4:TE/*.H J:
J:
I:CC TEMINCOM.C
I:ZSM TEMINCOM
I:HEXTOCOM TEMINCOM
TEMINCOM TEMINCOM.C


Build attempt of TEANSI (can be ignored)
========================================

C:
FAT COPY 4:TE/*.C J:
FAT COPY 4:TE/*.H J:
J:
I:CC TEANSI.C
I:ZSM TEANSI
I:HEXTOCOM TEANSI

If using baseline v1.70 or later then need to also build TECF.COM
J:
I:CC TECF.C
I:ZSM TECF
I:HEXTOCOM TECF

Sadly, the compilation fails because
TECF.C:#include "cfreader.h" is not found.
cfreader.h seems to be outside of the project as .gitignore excludes it.

So instead, copy the pre-built TECF.COM
C:
FAT COPY 4:TE/TECF.COM J:
FAT COPY 4:TE/TEANSI.CF J:

Then patch TEANSI
J:
TECF PATCH TEANSI.COM TEANSI.CF
Except it fails with
TECF: Can't read file.

To test,
TEANSI TEANSI.C
Note, the file is corrupted by tabs being converted to a space and hangs
when the max number of columns is exceeded.

Note that the original baseline branches won't natively build on the
SC126 because strangely some filenames have underscores which are not
supported by CP/M. Therefore, do sufficient modifications to allow
native building to work.
