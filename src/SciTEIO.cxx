// SciTE - Scintilla based Text Editor
/** @file SciTEIO.cxx
 ** Manage input and output with the system.
 **/
// Copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include "Platform.h"

#if PLAT_FOX

#include <unistd.h>

#endif

#if PLAT_GTK

#include <unistd.h>
#include <gtk/gtk.h>

#endif

#if PLAT_WIN

#define _WIN32_WINNT  0x0400
#ifdef _MSC_VER
// windows.h, et al, use a lot of nameless struct/unions - can't fix it, so allow it
#pragma warning(disable: 4201)
#endif
#include <windows.h>
#ifdef _MSC_VER
// okay, that's done, don't allow it in our code
#pragma warning(default: 4201)
#endif
#include <commctrl.h>

// For chdir
#ifdef _MSC_VER
#include <direct.h>
#endif
#ifdef __BORLANDC__
#include <dir.h>
#endif

#endif

#include "SciTE.h"
#include "PropSet.h"
#include "Accessor.h"
#include "WindowAccessor.h"
#include "Scintilla.h"
#include "Extender.h"
#include "Utf8_16.h"
#include "SciTEBase.h"

#ifdef unix
const char pathSepString[] = "/";
const char pathSepChar = '/';
const char configFileVisibilityString[] = ".";
const char propUserFileName[] = ".SciTEUser.properties";
const char fileRead[] = "rb";
const char fileWrite[] = "wb";
#endif
#ifdef __vms
const char pathSepString[] = "/";
const char pathSepChar = '/';
const char configFileVisibilityString[] = "";
const char propUserFileName[] = "SciTEUser.properties";
const char fileRead[] = "r";
const char fileWrite[] = "w";
#endif
#ifdef WIN32
// Windows
const char pathSepString[] = "\\";
const char pathSepChar = '\\';
const char configFileVisibilityString[] = "";
const char propUserFileName[] = "SciTEUser.properties";
const char fileRead[] = "rb";
const char fileWrite[] = "wb";
#endif
const char propGlobalFileName[] = "SciTEGlobal.properties";
const char propAbbrevFileName[] = "abbrev.properties";

#define PROPERTIES_EXTENSION	".properties"

static bool IsPropertiesFile(char *filename) {
	size_t nameLen = strlen(filename);
	size_t propLen = strlen(PROPERTIES_EXTENSION);
	if (nameLen <= propLen)
		return false;
	if (EqualCaseInsensitive(filename + nameLen - propLen, PROPERTIES_EXTENSION))
		return true;
	return false;
}

bool SciTEBase::IsAbsolutePath(const char *path) {
	if (!path || *path == '\0')
		return false;
#ifdef unix

	if (path[0] == '/')
		return true;
#endif
#ifdef __vms

	if (path[0] == '/')
		return true;
#endif
#ifdef WIN32

	if (path[0] == '\\' || path[1] == ':')	// UNC path or drive separator
		return true;
#endif

	return false;
}

void SciTEBase::FixFilePath() {
	// Only used on Windows to fix the case of file names
}

#ifdef __vms
const char *VMSToUnixStyle(const char *fileName) {
	// possible formats:
	// o disk:[dir.dir]file.type
	// o logical:file.type
	// o [dir.dir]file.type
	// o file.type
	// o /disk//dir/dir/file.type
	// o /disk/dir/dir/file.type

	static char unixStyleFileName[MAX_PATH + 20];

	if (strchr(fileName, ':') == NULL && strchr(fileName, '[') == NULL) {
		// o file.type
		// o /disk//dir/dir/file.type
		// o /disk/dir/dir/file.type
		if (strstr (fileName, "//") == NULL) {
			return fileName;
		}
		strcpy(unixStyleFileName, fileName);
		char *p;
		while ((p = strstr (unixStyleFileName, "//")) != NULL) {
			strcpy (p, p + 1);
		}
		return unixStyleFileName;
	}

	// o disk:[dir.dir]file.type
	// o logical:file.type
	// o [dir.dir]file.type

	if (fileName [0] == '/') {
		strcpy(unixStyleFileName, fileName);
	} else {
		unixStyleFileName [0] = '/';
		strcpy(unixStyleFileName + 1, fileName);
		char *p = strstr(unixStyleFileName, ":[");
		if (p == NULL) {
			// o logical:file.type
			p = strchr(unixStyleFileName, ':');
			*p = '/';
		} else {
			*p = '/';
			strcpy(p + 1, p + 2);
			char *end = strchr(unixStyleFileName, ']');
			if (*end != NULL) {
				*end = '/';
			}
			while (p = strchr(unixStyleFileName, '.'), p != NULL && p < end) {
				*p = '/';
			}
		}
	}
	return unixStyleFileName;
} // VMSToUnixStyle
#endif

void SciTEBase::SetFileName(const char *openName, bool fixCase) {
	if (openName[0] == '\"') {
		// openName is surrounded by double quotes
		char pathCopy[MAX_PATH + 1];
		pathCopy[0] = '\0';
		strncpy(pathCopy, openName + 1, MAX_PATH);
		pathCopy[MAX_PATH] = '\0';
		if (pathCopy[strlen(pathCopy) - 1] == '\"') {
			pathCopy[strlen(pathCopy) - 1] = '\0';
		}
		AbsolutePath(fullPath, pathCopy, MAX_PATH);
	} else if (openName[0]) {
		AbsolutePath(fullPath, openName, MAX_PATH);
	} else {
		fullPath[0] = '\0';
	}

	// Break fullPath into directory and file name using working directory for relative paths
	dirName[0] = '\0';
	char *cpDirEnd = strrchr(fullPath, pathSepChar);
	if (IsAbsolutePath(fullPath)) {
		// Absolute path
		strcpy(fileName, cpDirEnd + 1);
		strcpy(dirName, fullPath);
		dirName[cpDirEnd - fullPath] = '\0';
		//Platform::DebugPrintf("SetFileName: <%s> <%s>\n", fileName, dirName);
	} else {
		// Relative path. Since we ran AbsolutePath, we probably are here because fullPath is empty.
		GetDocumentDirectory(dirName, sizeof(dirName));
		//Platform::DebugPrintf("Working directory: <%s>\n", dirName);
		if (cpDirEnd) {
			// directories and file name
			strcpy(fileName, cpDirEnd + 1);
			*cpDirEnd = '\0';
			strncat(dirName, pathSepString, sizeof(dirName));
			strncat(dirName, fullPath, sizeof(dirName));
		} else {
			// Just a file name
			strcpy(fileName, fullPath);
		}
	}

	// Rebuild fullPath from directory and name
	strcpy(fullPath, dirName);
	strcat(fullPath, pathSepString);
	strcat(fullPath, fileName);
	//Platform::DebugPrintf("Path: <%s>\n", fullPath);

	if (fixCase) {
		FixFilePath();
	}

	char fileBase[MAX_PATH];
	strcpy(fileBase, fileName);
	char *extension = strrchr(fileBase, '.');
	if (extension) {
		*extension = '\0';
		strcpy(fileExt, extension + 1);
	} else { // No extension
		fileExt[0] = '\0';
	}

	chdir(dirName);

	ReadLocalPropFile();

	props.Set("FilePath", fullPath);
	props.Set("FileDir", dirName);
	props.Set("FileName", fileBase);
	props.Set("FileExt", fileExt);
	props.Set("FileNameExt", fileName);

	SetWindowName();
	if (buffers.buffers)
		buffers.buffers[buffers.current].Set(fullPath);
	BuffersMenu();
}

bool SciTEBase::Exists(const char *dir, const char *path, char *testPath) {
	char copyPath[MAX_PATH];
	if (IsAbsolutePath(path) || !dir) {
		strcpy(copyPath, path);
	} else if (dir) {
		if ((strlen(dir) + strlen(pathSepString) + strlen(path) + 1) > MAX_PATH)
			return false;
		strcpy(copyPath, dir);
		strcat(copyPath, pathSepString);
		strcat(copyPath, path);
	}
	FILE *fp = fopen(copyPath, fileRead);
	if (!fp)
		return false;
	fclose(fp);
	if (testPath)
		AbsolutePath(testPath, copyPath, MAX_PATH);
	return true;
}

bool BuildPath(char *path, const char *dir, const char *fileName,
               unsigned int lenPath) {
	*path = '\0';
	if ((strlen(dir) + strlen(pathSepString) + strlen(fileName)) < lenPath) {
		strcpy(path, dir);
		strcat(path, pathSepString);
		strcat(path, fileName);
		return true;
	}
	return false;
}

time_t GetModTime(const char *fullPath) {
	if (IsUntitledFileName(fullPath))
		return 0;
	struct stat statusFile;
	if (stat(fullPath, &statusFile) != -1)
		return statusFile.st_mtime;
	else
		return 0;
}

void SciTEBase::CountLineEnds(int &linesCR, int &linesLF, int &linesCRLF) {
	linesCR = 0;
	linesLF = 0;
	linesCRLF = 0;
	int lengthDoc = LengthDocument();
	char chPrev = ' ';
	WindowAccessor acc(wEditor.GetID(), props);
	for (int i = 0; i < lengthDoc; i++) {
		char ch = acc[i];
		char chNext = acc.SafeGetCharAt(i + 1);
		if (ch == '\r') {
			if (chNext == '\n')
				linesCRLF++;
			else
				linesCR++;
		} else if (ch == '\n') {
			if (chPrev != '\r') {
				linesLF++;
			}
		}
		chPrev = ch;
	}
}

static bool isEncodingChar(char ch) {
	return (ch == '_') || (ch == '-') || (ch == '.') ||
		(ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
		(ch >= '0' && ch <= '9');
}

static bool isSpaceChar(char ch) {
	return (ch == ' ') || (ch == '\t');
}

static SString ExtractLine(const char *buf, size_t length) {
	unsigned int endl = 0;
	if (length > 0) {
		while ((endl < length) && (buf[endl] != '\r') && (buf[endl] != '\n')) {
			endl++;
		}
		if (((endl+1) < length) && (buf[endl] == '\r') && (buf[endl+1] == '\n')) {
			endl++;
		}
		if (endl < length) {
			endl++;
		}
	}
	return SString(buf, 0, endl);
}

static const char codingCookie[] = "coding";

static UniMode CookieValue(const SString &s) {
	int posCoding = s.search(codingCookie);
	if (posCoding >= 0) {
		posCoding += strlen(codingCookie);
		if ((s[posCoding] == ':') || (s[posCoding] == '=')) {
			posCoding++;
			while ((posCoding < static_cast<int>(s.length())) &&
				(isSpaceChar(s[posCoding]))) {
				posCoding++;
			}
			size_t endCoding = static_cast<size_t>(posCoding);
			while ((endCoding < s.length()) &&
				(isEncodingChar(s[endCoding]))) {
				endCoding++;
			}
			SString code(s.c_str(), posCoding, endCoding);
			code.lowercase();
			if (code == "utf-8") {
				return uniCookie;
			}
		}
	}
	return uni8Bit;
}

void SciTEBase::OpenFile(bool initialCmdLine) {
	Utf8_16_Read convert;

	FILE *fp = fopen(fullPath, fileRead);
	if (fp || initialCmdLine) {
		// If initial run and no fp, just open an empty buffer
		// with the given name
		if (fp) {
			fileModTime = GetModTime(fullPath);
			fileModLastAsk = fileModTime;
			SendEditor(SCI_CLEARALL);
			char data[blockSize];
			size_t lenFile = fread(data, 1, sizeof(data), fp);
			SString l1 = ExtractLine(data, lenFile);
			SString l2 = ExtractLine(data+l1.length(), lenFile-l1.length());
			while (lenFile > 0) {
				lenFile = convert.convert(data, lenFile);
				SendEditorString(SCI_ADDTEXT, lenFile, convert.getNewBuf());
				lenFile = fread(data, 1, sizeof(data), fp);
			}
			fclose(fp);
			unicodeMode = static_cast<UniMode>(
				static_cast<int>(convert.getEncoding()));
			// Check the first two lines for coding cookies
			if (unicodeMode == uni8Bit) {
				unicodeMode = CookieValue(l1);
				if (unicodeMode == uni8Bit) {
					unicodeMode = CookieValue(l2);
				}
			}
			if (unicodeMode != uni8Bit) {
				// Override the code page if Unicode
				codePage = SC_CP_UTF8;
			} else {
				codePage = props.GetInt("code.page");
			}
			SendEditor(SCI_SETCODEPAGE, codePage);

			if (props.GetInt("eol.auto")) {
				int linesCR;
				int linesLF;
				int linesCRLF;
				SetEol();
				CountLineEnds(linesCR, linesLF, linesCRLF);
				if (((linesLF >= linesCR) && (linesLF > linesCRLF)) || ((linesLF > linesCR) && (linesLF >= linesCRLF)))
					SendEditor(SCI_SETEOLMODE, SC_EOL_LF);
				else if (((linesCR >= linesLF) && (linesCR > linesCRLF)) || ((linesCR > linesLF) && (linesCR >= linesCRLF)))
					SendEditor(SCI_SETEOLMODE, SC_EOL_CR);
				else if (((linesCRLF >= linesLF) && (linesCRLF > linesCR)) || ((linesCRLF > linesLF) && (linesCRLF >= linesCR)))
					SendEditor(SCI_SETEOLMODE, SC_EOL_CRLF);
			}
		}
	} else {
		SString msg = LocaliseMessage("Could not open file '^0'.", fullPath);
		WindowMessageBox(wSciTE, msg, MB_OK | MB_ICONWARNING);
	}
	SendEditor(SCI_SETUNDOCOLLECTION, 1);
	// Flick focus to the output window and back to
	// ensure palette realised correctly.
	WindowSetFocus(wOutput);
	WindowSetFocus(wEditor);
	SendEditor(SCI_SETSAVEPOINT);
	if (props.GetInt("fold.on.open") > 0) {
		FoldAll();
	}
	SendEditor(SCI_GOTOPOS, 0);
	Redraw();
}

bool SciTEBase::Open(const char *file, bool initialCmdLine,
                     bool forceLoad, bool maySaveIfDirty) {
	InitialiseBuffers();

	if (!file) {
		SString msg = LocaliseMessage("Bad file.");
		WindowMessageBox(wSciTE, msg, MB_OK | MB_ICONWARNING);
	}
#ifdef __vms
	static char fixedFileName [MAX_PATH];
	strcpy(fixedFileName, VMSToUnixStyle(file));
	file = fixedFileName;
#endif

	char absPath[MAX_PATH];
	AbsolutePath(absPath, file, MAX_PATH);
	int index = buffers.GetDocumentByName(absPath);
	if (index >= 0) {
		SetDocumentAt(index);
		DeleteFileStackMenu();
		SetFileStackMenu();
		if (!forceLoad) // Just rotate into view
			return true;
	}
	// See if we can have a buffer for the file to open
	if (!CanMakeRoom(maySaveIfDirty)) {
		return false;
	}

	if (buffers.size == buffers.length) {
		AddFileToStack(fullPath, GetSelection(), GetCurrentScrollPosition());
		ClearDocument();
	} else {
		if (index < 0 || !forceLoad) { // No new buffer, already opened
			New();
		}
	}

	//Platform::DebugPrintf("Opening %s\n", file);
	SetFileName(absPath);
	overrideExtension = "";
	ReadProperties();
	SetIndentSettings();
	UpdateBuffersCurrent();
	if (tabVisible)	// Update tab visibility if the hide one mode is on
		SizeSubWindows();

	if (initialCmdLine) {
		if (props.GetInt("save.recent", 0))
			LoadRecentMenu();
	}
	if (initialCmdLine && props.GetInt("buffers") && !fileName[0]) {
		if (props.GetInt("save.session", 0))
			LoadSession("");
	}
	if (fileName[0]) {
		SendEditor(SCI_SETREADONLY, 0);
		SendEditor(SCI_CANCEL);
		SendEditor(SCI_SETUNDOCOLLECTION, 0);

		OpenFile(initialCmdLine);

		SendEditor(SCI_EMPTYUNDOBUFFER);
		isReadOnly = props.GetInt("read.only");
		SendEditor(SCI_SETREADONLY, isReadOnly);
	}
	RemoveFileFromStack(fullPath);
	DeleteFileStackMenu();
	SetFileStackMenu();
	SetWindowName();
	UpdateStatusBar(true);
	if (extender)
		extender->OnOpen(fullPath);
	return true;
}

void SciTEBase::OpenMultiple(const char *files, bool initialCmdLine, bool forceLoad) {
	if (*files) {
		while (*files) {
			Open(files, initialCmdLine, forceLoad);
			files = files + strlen(files) + 1;
		}
	} else {
		Open("", initialCmdLine, forceLoad);
	}
}

// Returns true if editor should get the focus
bool SciTEBase::OpenSelected() {
	char selectedFilename[MAX_PATH];
	char cTag[200];
	unsigned long lineNumber = 0;

	SString selName = SelectionFilename();
	strncpy(selectedFilename, selName.c_str(), MAX_PATH);
	selectedFilename[MAX_PATH - 1] = '\0';
	if (selectedFilename[0] == '\0') {
		WarnUser(warnWrongFile);
		return false;	// No selection
	}

	if (EqualCaseInsensitive(selectedFilename, fileName) || EqualCaseInsensitive(selectedFilename, fullPath)) {
		WarnUser(warnWrongFile);
		return true;	// Do not open if it is the current file!
	}

	cTag[0] = '\0';
	if (IsPropertiesFile(fileName) &&
	        strlen(fileName) + strlen(PROPERTIES_EXTENSION) < MAX_PATH) {
		// We are in a properties file, we append the correct extension
		// to open the include
		strcat(selectedFilename, PROPERTIES_EXTENSION);
	} else {
		// Check if we have a line number (error message or grep result)
		// A bit of duplicate work with DecodeMessage, but we don't know
		// here the format of the line, so we do guess work.
		// Can't do much for space separated line numbers anyway...
		char *endPath = strchr(selectedFilename, '(');
		if (endPath) {	// Visual Studio error message: F:\scite\src\SciTEBase.h(312):	bool Exists(
			lineNumber = atol(endPath + 1);
		} else {
			endPath = strchr(selectedFilename + 2, ':');	// Skip Windows' drive separator
			if (endPath) {	// grep -n line, perhaps gcc too: F:\scite\src\SciTEBase.h:312:	bool Exists(
				lineNumber = atol(endPath + 1);
			}
		}
		if (lineNumber > 0) {
			*endPath = '\0';
		}

#if PLAT_WIN
		if (strncmp(selectedFilename, "http:", 5) == 0 ||
		        strncmp(selectedFilename, "https:", 6) == 0 ||
		        strncmp(selectedFilename, "ftp:", 4) == 0 ||
		        strncmp(selectedFilename, "ftps:", 5) == 0 ||
		        strncmp(selectedFilename, "news:", 5) == 0 ||
		        strncmp(selectedFilename, "mailto:", 7) == 0) {
			SString cmd = selectedFilename;
			AddCommand(cmd, "", jobShell);
			return false;	// Job is done
		}
#endif

		// Support the ctags format

		if (lineNumber == 0) {
			GetCTag(cTag, sizeof(cTag));
		}
	}

	char path[MAX_PATH];
	*path = '\0';
	// Don't load the path of the current file if the selected
	// filename is an absolute pathname
	if (!IsAbsolutePath(selectedFilename)) {
		GetDocumentDirectory(path, sizeof(path));
#if PLAT_WIN
		// If not there, look in openpath
		if (!Exists(path, selectedFilename, NULL)) {
			SString fn = ExtensionFileName();
			SString openPath = props.GetNewExpand("openpath.", fn.c_str());
			//Platform::DebugPrintf("openPath=%s", openPath.c_str());
			if (openPath.length()) {
				LPTSTR lp;
				if (::SearchPath(openPath.c_str(), selectedFilename,
					NULL, sizeof(path), path, &lp)) {
					lp--;
					*lp = '\0';
				}
			}
		}
#endif
	}
	if (Exists(path, selectedFilename, path)) {
		if (Open(path, false)) {
			if (lineNumber > 0) {
				SendEditor(SCI_GOTOLINE, lineNumber - 1);
			} else if (cTag[0] != '\0') {
				if (atol(cTag) > 0) {
					SendEditor(SCI_GOTOLINE, atol(cTag) - 1);
				} else {
					findWhat = cTag;
					FindNext(false);
				}
			}
			return true;
		}
	} else {
		WarnUser(warnWrongFile);
	}
	return false;
}

void SciTEBase::Revert() {
	RecentFile rf = GetFilePosition();
	OpenFile(false);
	DisplayAround(rf);
}

void SciTEBase::CheckReload() {
	if (props.GetInt("load.on.activate")) {
		// Make a copy of fullPath as otherwise it gets aliased in Open
		char fullPathToCheck[MAX_PATH];
		strcpy(fullPathToCheck, fullPath);
		time_t newModTime = GetModTime(fullPathToCheck);
		//Platform::DebugPrintf("Times are %d %d\n", fileModTime, newModTime);
		if (newModTime > fileModTime) {
			RecentFile rf = GetFilePosition();
			if (isDirty) {
				static bool entered = false; // Stop reentrancy
				if (!entered && (0 == dialogsOnScreen) && (newModTime != fileModLastAsk)) {
					entered = true;
					SString msg = LocaliseMessage(
					                  "The file '^0' has been modified. Should it be reloaded?",
					                  fullPathToCheck);
					dialogsOnScreen++;
					int decision = WindowMessageBox(wSciTE, msg, MB_YESNO);
					dialogsOnScreen--;
					if (decision == IDYES) {
						Open(fullPathToCheck, false, true, false);
						DisplayAround(rf);
					}
					fileModLastAsk = newModTime;
					entered = false;
				}
			} else {
				Open(fullPathToCheck, false, true);
				DisplayAround(rf);
			}
		}
	}
}

void SciTEBase::Activate(bool activeApp) {
	if (activeApp) {
		CheckReload();
	} else {
		if (isDirty) {
			if (props.GetInt("save.on.deactivate") && fileName[0]) {
				// Trying to save at deactivation when no file name -> dialogs
				Save();
			}
		}
	}
}

int SciTEBase::SaveIfUnsure(bool forceQuestion) {
	if (isDirty) {
		if (props.GetInt("are.you.sure", 1) ||
		        IsUntitledFileName(fullPath) ||
		        forceQuestion) {
			SString msg;
			if (fileName[0]) {
				msg = LocaliseMessage("Save changes to '^0'?", fullPath);
			} else {
				msg = LocaliseMessage("Save changes to (Untitled)?");
			}
			dialogsOnScreen++;
			int decision = WindowMessageBox(wSciTE, msg, MB_YESNOCANCEL);
			dialogsOnScreen--;
			if (decision == IDYES) {
				if (!Save())
					decision = IDCANCEL;
			}
			return decision;
		} else {
			if (!Save())
				return IDCANCEL;
		}
	}
	return IDYES;
}

int SciTEBase::SaveIfUnsureAll(bool forceQuestion) {
	if (SaveAllBuffers(forceQuestion) == IDCANCEL) {
		return IDCANCEL;
	}
	if (props.GetInt("save.recent", 0)) {
		for (int i = 0; i < buffers.length; ++i) {
			Buffer buff = buffers.buffers[i];
			AddFileToStack(buff.FullPath(),
			               buff.selection, buff.scrollPosition);
		}
		SaveRecentStack();
	}
	if (props.GetInt("buffers") && props.GetInt("save.session", 0))
		SaveSession("");

	// Definitely going to exit now, so delete all documents
	// Set editor back to initial document
	SendEditor(SCI_SETDOCPOINTER, 0, buffers.buffers[0].doc);
	// Release all the extra documents
	for (int j = 0; j < buffers.size; j++) {
		if (buffers.buffers[j].doc)
			SendEditor(SCI_RELEASEDOCUMENT, 0, buffers.buffers[j].doc);
	}
	// Initial document will be deleted when editor deleted
	return IDYES;
}

int SciTEBase::SaveIfUnsureForBuilt() {
	if (isDirty) {
		if (props.GetInt("are.you.sure.for.build", 0))
			return SaveIfUnsure(true);

		Save();
	}
	return IDYES;
}

void SciTEBase::StripTrailingSpaces() {
	int maxLines = SendEditor(SCI_GETLINECOUNT);
	for (int line = 0; line < maxLines; line++) {
		int lineStart = SendEditor(SCI_POSITIONFROMLINE, line);
		int lineEnd = SendEditor(SCI_GETLINEENDPOSITION, line);
		int i = lineEnd-1;
		char ch = static_cast<char>(SendEditor(SCI_GETCHARAT, i));
		while ((i >= lineStart) && ((ch == ' ') || (ch == '\t'))) {
			i--;
			ch = static_cast<char>(SendEditor(SCI_GETCHARAT, i));
		}
		if (i < (lineEnd-1)) {
			SendEditor(SCI_SETTARGETSTART, i + 1);
			SendEditor(SCI_SETTARGETEND, lineEnd);
			SendEditorString(SCI_REPLACETARGET, 0, "");
		}
	}
}

void SciTEBase::EnsureFinalNewLine() {
	int maxLines = SendEditor(SCI_GETLINECOUNT);
	bool appendNewLine = maxLines == 1;
	int endDocument = SendEditor(SCI_POSITIONFROMLINE, maxLines);
	if (maxLines > 1) {
		appendNewLine = endDocument > SendEditor(SCI_POSITIONFROMLINE, maxLines-1);
	}
	if (appendNewLine) {
		const char *eol = "\n";
		switch (SendEditor(SCI_GETEOLMODE)) {
			case SC_EOL_CRLF:
				eol = "\r\n";
				break;
			case SC_EOL_CR:
				eol = "\r";
				break;
		}
		SendEditorString(SCI_INSERTTEXT, endDocument, eol);
	}
}

// Writes the buffer to the given filename
bool SciTEBase::SaveBuffer(const char *saveName) {
	// Perform clean ups on text before saving
	SendEditor(SCI_BEGINUNDOACTION);
	if (props.GetInt("strip.trailing.spaces"))
		StripTrailingSpaces();
	if (props.GetInt("ensure.final.line.end"))
		EnsureFinalNewLine();
	if (props.GetInt("ensure.consistent.line.ends"))
		SendEditor(SCI_CONVERTEOLS, SendEditor(SCI_GETEOLMODE));
	SendEditor(SCI_ENDUNDOACTION);

	Utf8_16_Write convert;
	if (unicodeMode != uniCookie) {	// Save file with cookie without BOM.
		convert.setEncoding(static_cast<Utf8_16::encodingType>(
			static_cast<int>(unicodeMode)));
	}

	FILE *fp = convert.fopen(saveName, fileWrite);
	if (fp) {
		char data[blockSize + 1];
		int lengthDoc = LengthDocument();
		for (int i = 0; i < lengthDoc; i += blockSize) {
			int grabSize = lengthDoc - i;
			if (grabSize > blockSize)
				grabSize = blockSize;
			GetRange(wEditor, i, i + grabSize, data);
			convert.fwrite(data, grabSize);
		}
		convert.fclose();
		return true;
	}
	return false;
}

// Returns false if cancelled or failed to save
bool SciTEBase::Save() {
	if (fileName[0]) {
		if (props.GetInt("save.deletes.first")) {
			unlink(fullPath);
		}

		if (SaveBuffer(fullPath)) {
			fileModTime = GetModTime(fullPath);
			SendEditor(SCI_SETSAVEPOINT);
			if (IsPropertiesFile(fileName)) {
				//bool currentUseMonoFont = useMonoFont;
				ReadGlobalPropFile();
				SetImportMenu();
				ReadLocalPropFile();
				ReadAbbrevPropFile();
				ReadProperties();
				//useMonoFont = currentUseMonoFont;
				//if (useMonoFont) {
				//	SetMonoFont();
				//}
				SetWindowName();
				BuffersMenu();
				Redraw();
			}
		} else {
			SString msg = LocaliseMessage("Could not save file '^0'.", fullPath);
			dialogsOnScreen++;
			WindowMessageBox(wSciTE, msg, MB_OK | MB_ICONWARNING);
			dialogsOnScreen--;
			return false;
		}
		return true;
	} else {
		return SaveAs();
	}
}

bool SciTEBase::SaveAs(const char *file) {
	if (file && *file) {
		SetFileName(file);
		Save();
		ReadProperties();
		SendEditor(SCI_CLEARDOCUMENTSTYLE);
		SendEditor(SCI_COLOURISE, 0, -1);
		Redraw();
		SetWindowName();
		BuffersMenu();
		return true;
	} else {
		return SaveAsDialog();
	}
}
