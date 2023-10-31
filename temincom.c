/*	temincom.c

	Text editor -- version for use with minicom (Linux).

	Copyright (c) 2015-2021 Miguel Garcia / FloppySoftware
	Modifications by Dean Jenkins 2021

	This program is free software; you can redistribute it and/or modify it
	under the terms of the GNU General Public License as published by the
	Free Software Foundation; either version 2, or (at your option) any
	later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

	Usage:

	temincom [filename]

	Compilation:

	cc temincom
	ccopt temincom (optional, takes a long time on native CP/M)
	zsm temincom
	hextocom temincom

	Changes:

	20 Oct 2020 : 1st version from source for the Takeda Toshiya's CP/M emulator.
	14 Jan 2021 : Remove OPT_NUM.
	22 Feb 2021 : Move CRT_ROWS, CRT_COLS to assembler.

	Dean's changes:

	28 May 2021 : Initial adaption to create temincom.

	Notes:

	It is compatible with minicom (Linux).

	It needs to translate some keyboard codes.
*/

/* Options
   -------
   Set to 1 to add the following functionalities, else 0.
*/
#define OPT_LWORD 0  /* Go to word on the left */
#define OPT_RWORD 0  /* Go to word on the right */
#define OPT_FIND  1  /* Find string */
#define OPT_GOTO  1  /* Go to line # */
#define OPT_BLOCK 1  /* Block selection */
#define OPT_MACRO 1  /* Enable macros */

/* Definitions
   -----------
*/
#define CRT_NAME "ANSI Terminal"

#asm

CRT_ROWS: equ 40   ; CRT rows
CRT_COLS: equ 130  ; CRT columns

#endasm

#define RULER_TAB    '!'  /* Ruler: Tab stop character - ie: ! */
#define RULER_CHR    '.'  /* Ruler: Character - ie: . */
#define SYS_LINE_SEP '-'  /* System line separator character - ie: - */

/* Include main code
   -----------------
*/
#include "te.c"

/* ESC sequences key mappings */
#define KEY_UP		CTL_K
#define KEY_DOWN	CTL_X
#define KEY_LEFT	CTL_S
#define KEY_RIGHT	CTL_G
#define KEY_HOME	CTL_O
#define KEY_END		CTL_A
#define KEY_PGUP	CTL_R
#define KEY_PGDOWN	CTL_W

/* Setup CRT: Used when the editor starts
   --------------------------------------
   void CrtSetup(void)
*/
CrtSetup()
{
	CrtSetupEx();

	SetKey(K_UP,        KEY_UP,     '\0', "UP");
	SetKey(K_DOWN,      KEY_DOWN,   '\0', "DOWN");
	SetKey(K_LEFT,      KEY_LEFT,   '\0', "LEFT");
	SetKey(K_RIGHT,     KEY_RIGHT,  '\0', "RIGHT");
	SetKey(K_BEGIN,     KEY_HOME,   '\0', "HOME");
	SetKey(K_END,       KEY_END,    '\0', "END");
	SetKey(K_TOP,       CTL_T,      '\0', NULL);
	SetKey(K_BOTTOM,    CTL_E,      '\0', NULL);
	SetKey(K_PGUP,      KEY_PGUP,   '\0', NULL);
	SetKey(K_PGDOWN,    KEY_PGDOWN, '\0', NULL);
	SetKey(K_TAB,       CTL_I,      '\0', "TAB");
	SetKey(K_CR,        CTL_M,      '\0', "ENTER");
	SetKey(K_ESC,       ESC,        '\0', "ESC");
	SetKey(K_RDEL,      DEL,        '\0', "DEL");
	SetKey(K_LDEL,      CTL_H,      '\0', "BS");
	SetKey(K_CUT,       CTL_U,      '\0', NULL);
	SetKey(K_COPY,      CTL_C,      '\0', NULL);
	SetKey(K_PASTE,     CTL_V,      '\0', NULL);
	SetKey(K_DELETE,    CTL_D,      '\0', NULL);
	SetKey(K_CLRCLP,    CTL_P,      '\0', NULL);
#if OPT_FIND                            
	SetKey(K_FIND,      CTL_F,      '\0', NULL);
	SetKey(K_NEXT,      CTL_N,      '\0', NULL);
#endif                                  
#if OPT_GOTO                            
	SetKey(K_GOTO,      CTL_J,      '\0', NULL);
#endif
#if OPT_LWORD
	/*SetKey(K_LWORD,     '\0', '\0', NULL);*/
#endif
#if OPT_RWORD
	/*SetKey(K_RWORD,     '\0', '\0', NULL);*/
#endif
#if OPT_BLOCK
	SetKey(K_BLK_START, CTL_B,      'S', NULL);
	SetKey(K_BLK_END,   CTL_B,      'E', NULL);
	SetKey(K_BLK_UNSET, CTL_B,      'U', NULL);
#endif                                  
#if OPT_MACRO                           
	SetKey(K_MACRO,     CTL_Y,      '\0', NULL);
#endif
}

#asm
CrtSetupEx:
	ld  hl,(1)
	inc hl
	inc hl
	inc hl
	ld  de,BiosConst
	ld  bc,9
	ldir
	ret

BiosConst:  jp 0
BiosConin:  jp 0
BiosConout: jp 0
#endasm

/* Reset CRT: Used when the editor exits
   -------------------------------------
   void CrtReset(void)
*/
CrtReset()
{
}

/* Output character to the CRT
   ---------------------------
   All program output is done with this function.

   On '\n' outputs '\n' + '\r'.

   void CrtOut(int ch)
*/
#asm
CrtOut:
	ld   a,l
	cp   10
	jr   nz,CrtOutRaw
	ld   c,13
	call BiosConout
	ld   l,10
CrtOutRaw:
	ld   c,l
	jp   BiosConout
#endasm

int newkey = 0;
/* 0 = not received an ESC code
   1 = got an ESC code
   2 = got a ESC [ code sequence
   3 = wait for ESC [ <code> ~ end of sequence
   4 = got ESC O code sequence
*/
int escape_seq = 0;
CrtInEsc()
{
	int ch;
	int code_key;
	int wait_char;

	switch(escape_seq)
	{
		case 0:
			/* Wait for a key via BIOS CONIN */
			ch = CrtInEx();

			if(ch == ESC)
			{
				/* Got an ESC code */
				escape_seq = 1;
				return 0;
			}
			else
			{
				/* Normal key handling */
				return ch;
			}
			break;
		case 1:
			wait_char = 100;

			/* Use BIOS CONST, returns 0 if no following key, else 0xFF */
			while(wait_char && !CrtInSt())
			{
				/* Avoid false ESC detection, wait a little while */
				wait_char--;
			}

			if(!wait_char)
			{
				/* No code followed the ESC so declare key is ESC */
				escape_seq = 0;
				return ESC;
			}

			/* Process the code after the ESC code */
			ch = CrtInEx();
			switch(ch)
			{
				case ESC:
					/* Detected ESC ESC sequence so declare key is ESC */
					escape_seq = 0;
					return ESC;
				case '[':
					/* Detected ESC [ sequence */
					escape_seq = 2;
					return 0;
				case 'O':
					/* Detected ESC O sequence */
					escape_seq = 4;
					return 0;
				default:
					/* Unhandled ESC <char> sequence */
					break;
			}
			break;
		case 2:
			/* Get the code after the ESC [ */
			code_key = CrtInEx();

			switch(code_key)
			{
				/* https://en.wikipedia.org/wiki/ANSI_escape_code#Terminal_input_sequences */
				/* VT Escape sequence handling */
				case '1' : /* HOME (start of line) */
					newkey = KEY_HOME;
					escape_seq = 3;
					return 0;
				case '3' : /* DEL */
					newkey = DEL;
					escape_seq = 3;
					return 0;
				case '5' : /* PAGE UP */
					newkey = KEY_PGUP;
					escape_seq = 3;
					return 0;
				case '6' : /* PAGE DOWN */
					newkey = KEY_PGDOWN;
					escape_seq = 3;
					return 0;
				/* xterm handling */
				case 'A' : /* UP */
					escape_seq = 0;
					return KEY_UP;
				case 'B' : /* DOWN */
					escape_seq = 0;
					return KEY_DOWN;
				case 'C' : /* RIGHT */
					escape_seq = 0;
					return KEY_RIGHT;
				case 'D' : /* LEFT */
					escape_seq = 0;
					return KEY_LEFT;
				case 'H' : /* HOME */
					escape_seq = 0;
					return KEY_HOME;
				case 'F' : /* END */
					escape_seq = 0;
					return KEY_END;
			}
			break;
		case 3:
			if(CrtInEx() == '~')
			{
				/* Detected ESC [ <code> ~ end of sequence */
				escape_seq = 0;
				return newkey;
			}
			break;
		case 4:
			if(CrtInEx() == 'F')
			{
				/* Detected ESC OF for the END key */
				escape_seq = 0;
				return K_END;
			}
			break;
	}

	escape_seq = 0;
	return 0;
}

/* Input character from the keyboard
   ---------------------------------
   All program input is done with this function.

   Translates the ANSI key codes into single characters.

   int CrtIn(void)
*/
CrtIn()
{
	int ch;

	ch = 0;

	while(!ch)
	{
		ch = CrtInEsc();
	}

	return ch;
}

#asm
CrtInSt:
	call BiosConst
	ld h,0
	ld l,a
	ret
	
CrtInEx:
	call BiosConin
	ld h,0
	ld l,a
	ret
#endasm

/* Clear screen and send cursor to 0,0
   -----------------------------------
   void CrtClear(void)
*/
CrtClear()
{
	CrtOut(27); putstr("[1;1H"); /* Cursor to 0,0 */
	CrtOut(27); putstr("[2J");   /* Clear CRT */
}

/* Locate the cursor (HOME is 0,0)
   -------------------------------
   void CrtLocate(int row, int col)
*/
CrtLocate(row, col)
int row, col;
{
	CrtOut(27); CrtOut('[');
	putint("%d", row + 1); CrtOut(';');
	putint("%d", col + 1); CrtOut('H');
}

/* Erase line and cursor to row,0
   ------------------------------
   void CrtClearLine(int row)
*/
CrtClearLine(row)
int row;
{
	CrtLocate(row, 0); CrtClearEol();
}

/* Erase from the cursor to the end of the line
   --------------------------------------------
*/
CrtClearEol()
{
	CrtOut(27); putstr("[K");
}

/* Turn on / off reverse video
   ---------------------------
*/
CrtReverse(on)
int on;
{
	CrtOut(27); CrtOut('['); CrtOut(on ? '7' : '0'); CrtOut('m');
}


