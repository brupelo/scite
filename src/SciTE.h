// SciTE - small text editor to debug Scintilla
// SciTE.h - define command IDs used within SciTE
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCITE_H
#define SCITE_H

// menu defines
#define IDM_BUFFER 70
#define IDM_MRUFILE	90

#define IDM_NEW		101
#define IDM_OPEN    102
#define IDM_CLOSE    103
#define IDM_SAVE    104
#define IDM_SAVEAS  105
#define IDM_REVERT	106
#define IDM_PRINT   107
#define IDM_PRINTSETUP   108
#define IDM_ABOUT   109
#define IDM_QUIT    110
#define IDM_NEXTFILE    111
#define IDM_MRU_SEP	112
#define IDM_SAVEASHTML 113
#define IDM_PREVFILE    114
// 115 has been recycled.
// 116 has been recycled.
#define IDM_BUFFERSEP 117
#define IDM_CLOSEALL   118
#define IDM_SAVEASRTF 119

#define IDM_UNDO	120
#define IDM_REDO	121
#define IDM_CUT		122
#define IDM_COPY	123
#define IDM_PASTE	124
#define IDM_CLEAR	125
#define IDM_SELECTALL 127
#define IDM_FIND 128
#define IDM_FINDNEXT 129
#define IDM_FINDNEXTBACK 144
#define IDM_FINDNEXTSEL 145
#define IDM_FINDNEXTBACKSEL 146
#define IDM_FINDINFILES 130
#define IDM_REPLACE 131
#define IDM_GOTO 132
#define IDM_UPRCASE 133
#define IDM_LWRCASE 134
#define IDM_VIEWSPACE 135
#define IDM_SPLITVERTICAL 136
#define IDM_NEXTMSG 137
#define IDM_PREVMSG 138
#define IDM_MATCHBRACE 139
#define IDM_COMPLETE 140
#define IDM_EXPAND 141
#define IDM_TABSIZE 142
#define IDM_SELECTTOBRACE 143
#define IDM_COMPLETEWORD 147
#define IDM_SHOWCALLTIP 148
#define IDM_TOGGLE_FOLDALL 149
#define IDM_SAVEASPDF 150
#define IDM_OPENSELECTED 151
#define IDM_FIXED_FONT 152

#define IDM_COMPILE 200
#define IDM_BUILD 201
#define IDM_GO 202
#define IDM_STOPEXECUTE 203
#define IDM_FINISHEDEXECUTE 204

#define IDM_OPENLOCALPROPERTIES 210
#define IDM_OPENUSERPROPERTIES 211
#define IDM_OPENGLOBALPROPERTIES 212

// Dialog control IDs
#define IDGOLINE 220
#define IDABOUTSCINTILLA 221
#define IDFINDWHAT 222
#define IDFILES 223
#define IDDIRECTORY 224
#define IDCURRLINE 225
#define IDLASTLINE 226
#define IDEXTEND 227
#define IDTABSIZE 228
#define IDINDENTSIZE 229
#define IDUSETABS 230

#define IDREPLACEWITH 231
#define IDWHOLEWORD 232
#define IDMATCHCASE 233
#define IDDIRECTIONUP 234
#define IDDIRECTIONDOWN 235
#define IDREPLACE 236
#define IDREPLACEALL 237
#define IDREGEXP 238

#define IDM_VIEWGUIDES 240
#define IDM_SELECTIONMARGIN  241
#define IDM_BUFFEREDDRAW  242
#define IDM_LINENUMBERMARGIN  243
#define IDM_USEPALETTE  244
#define IDM_SELMARGIN 245
#define IDM_FOLDMARGIN 246
#define IDM_VIEWEOL 247
#define IDM_VIEWTOOLBAR 248
#define IDM_VIEWTABBAR 255
#define IDM_VIEWSTATUSBAR 249

#define IDM_ACTIVATE    250
#define IDM_TOOLS 260

#define IDM_EOL_CRLF 270
#define IDM_EOL_CR 271
#define IDM_EOL_LF 272
#define IDM_EOL_CONVERT 273

#define IDM_WORDPARTLEFT 280
#define IDM_WORDPARTLEFTEXTEND 281
#define IDM_WORDPARTRIGHT 282
#define IDM_WORDPARTRIGHTEXTEND 283

#define IDM_SRCWIN 300
#define IDM_RUNWIN 301
#define IDM_TOOLWIN 302
#define IDM_STATUSWIN 303
#define IDM_TABWIN 304

#define IDM_BOOKMARK_TOGGLE 302
#define IDM_BOOKMARK_NEXT 303

#define IDM_HELP 320

// Dialog IDs
#define IDD_FIND 400
#define IDD_REPLACE 401

#define LEXER_BASE	400

#define IDM_LEXER_NONE		LEXER_BASE
#define IDM_LEXER_CPP		LEXER_BASE+1
#define IDM_LEXER_VB		LEXER_BASE+2
#define IDM_LEXER_RC		LEXER_BASE+3
#define IDM_LEXER_HTML		LEXER_BASE+4
#define IDM_LEXER_XML		LEXER_BASE+5
#define IDM_LEXER_JS		LEXER_BASE+6
#define IDM_LEXER_WSCRIPT	LEXER_BASE+7
#define IDM_LEXER_PROPS		LEXER_BASE+8
#define IDM_LEXER_BATCH		LEXER_BASE+9
#define IDM_LEXER_MAKE		LEXER_BASE+10
#define IDM_LEXER_ERRORL	LEXER_BASE+11
#define IDM_LEXER_JAVA		LEXER_BASE+12
#define IDM_LEXER_LUA	LEXER_BASE+13
#define IDM_LEXER_PYTHON	LEXER_BASE+14
#define IDM_LEXER_PERL		LEXER_BASE+15
#define IDM_LEXER_SQL		LEXER_BASE+16
#define IDM_LEXER_PLSQL		LEXER_BASE+17
#define IDM_LEXER_PHP		LEXER_BASE+18
#define IDM_LEXER_LATEX		LEXER_BASE+19
#define IDM_LEXER_DIFF		LEXER_BASE+20
#define IDM_LEXER_CS			LEXER_BASE+21
#define IDM_LEXER_CONF		LEXER_BASE+22
#define IDM_LEXER_PASCAL		LEXER_BASE+23

#endif
