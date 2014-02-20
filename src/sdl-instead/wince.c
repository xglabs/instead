/* 
 * Copyright 2009-2014 Peter Kosyh <p.kosyh at gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "wince.h"
#include <windows.h>
#include <shlobj.h>
#include <limits.h>
#include <sys/types.h>
#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "sdl_iconv.h"
#include "internals.h"

extern char *curgame;
extern char *curgame_dir;

static char local_games_path[PATH_MAX];
static char local_themes_path[PATH_MAX];
static char local_stead_path[PATH_MAX];

static char save_path[PATH_MAX];
static char cfg_path[PATH_MAX];

void	nsleep(int u)
{
	Sleep(u);
}

char *game_locale(void)
{
	char buff[64];
	buff[0] = 0;
	if (!GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME,
        	buff,sizeof(buff) - 1))
		return NULL;
	return strdup(buff);
}

#if 0
static char *game_codepage = NULL;
#ifdef _HAVE_ICONV
static char *game_cp(void)
{
	char cpbuff[64];
	char buff[64];
	buff[0] = 0;
	if (!GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE,
        	buff,sizeof(buff) - 1))
		return NULL;
	snprintf(cpbuff, sizeof(cpbuff), "WINDOWS-%s", buff);
	return strdup(cpbuff);
}

char *mbs2utf8(const char *s)
{
	iconv_t han;
	char *str;
	if (!game_codepage)
		game_codepage = game_cp();
	if (!s)
		return NULL;
	if (!game_codepage)
		goto out0;
	han = iconv_open("UTF-8", game_codepage);
	if (han == (iconv_t)-1)
		goto out0;
	if (!(str = decode(han, s)))
		goto out1;
	iconv_close(han);
	return str;
out1:
	iconv_close(han);
out0:
	return strdup(s);
}
#else
char *mbs2utf8(const char *s)
{
	return strdup(s);
}
#endif
#endif

extern void unix_path(char *);

char *sdl_path(char *p)
{
#if 0
	char *r = mbs2utf8(p);
	if (p)
		free(p);
	unix_path(r);
	return r;
#else
	unix_path(p);
	return p;
#endif
}

char *app_dir( void );

char *game_tmp_path(void)
{
	DWORD dwRetVal = 0;
	static TCHAR lpTempPathBuffer[MAX_PATH];
	  //  Gets the temp path env string (no guarantee it's a valid path).
	dwRetVal = GetTempPath(MAX_PATH,          // length of the buffer
		lpTempPathBuffer); // buffer for path 
	if (dwRetVal > MAX_PATH || (dwRetVal == 0)) {
		return NULL;
	}
	strcat((char*)lpTempPathBuffer, "/instead-games");
	if (mkdir((char*)lpTempPathBuffer) && access((char*)lpTempPathBuffer, W_OK))
		return NULL;
	unix_path((char*)lpTempPathBuffer);
	return (char*)lpTempPathBuffer;
}

char *game_local_games_path(int cr)
{
	char *app = app_dir();
	if (!app)
		return NULL;
	snprintf(local_games_path, sizeof(local_games_path) - 1 , "%s/", app);
	if (cr) {
		if (mkdir(local_games_path) && access(local_games_path, W_OK))
			return NULL;
	}
	strcat(local_games_path,"/games");
	if (cr) {
		if (mkdir(local_games_path) && access(local_games_path, W_OK))
			return NULL;
	}
	return local_games_path;
}

char *game_local_themes_path(void)
{
	snprintf(local_themes_path, sizeof(local_themes_path) - 1 , "%s/themes", app_dir());
	return local_themes_path;
}

char *game_local_stead_path(void)
{
	snprintf(local_stead_path, sizeof(local_stead_path) - 1 , "%s/stead", app_dir());
	return local_stead_path;
}

#if 0
char *home_dir( void )
{
	static char homedir[PATH_MAX]="";
	SHGetFolderPath( NULL, 
		CSIDL_FLAG_CREATE | CSIDL_PROFILE,
		NULL,
		0, 
		(LPTSTR)homedir );
	unix_path(homedir);
	return homedir;
}
#endif
char *app_dir( void )
{
	static char appdir[PATH_MAX]="";
	if (appdata_sw)
		strcpy(appdir, appdata_sw);
	else {
		strcpy(appdir, game_cwd);
		strcat(appdir, "/appdata");
	}
	if (!access(appdir, W_OK))
		return appdir;
	return NULL;
}

char *game_cfg_path( void )
{
	char *p = app_dir();
	if (!p)
		return NULL;

	snprintf(cfg_path, sizeof(cfg_path) - 1 , "%src", p); /* appdir/insteadrc ;) */
	if (!access(cfg_path, R_OK)) 
		return cfg_path; 
/* no at home? Try in dir */
	snprintf(cfg_path, sizeof(cfg_path) - 1 , "%s", p);
	if (mkdir(cfg_path) && access(cfg_path, W_OK)) {
		snprintf(cfg_path, sizeof(cfg_path) - 1 , "%src", p); /* appdir/insteadrc ;) */
		return cfg_path;
	}
	snprintf(cfg_path, sizeof(cfg_path) - 1 , "%s/insteadrc", p);
	return cfg_path;
}

char *game_save_path( int cr, int nr )
{
	char appdir[PATH_MAX];
	char *p = app_dir();

	if (!curgame_dir)
		return NULL;

	if (!access("saves", R_OK)) {
		if (nr)
			snprintf(save_path, sizeof(save_path) - 1, "saves/save%d", nr);
		else
			snprintf(save_path, sizeof(save_path) - 1, "saves/autosave");
		return save_path;
	}
	if (!p)
		return NULL;

	strcpy( appdir, p );

	if (cr && mkdir(appdir) && access(appdir, W_OK))
		return NULL;

	snprintf(save_path, sizeof(save_path) - 1 , "%s/saves", appdir);

	if (cr && mkdir(save_path) && access(save_path, W_OK))
		return NULL;
	snprintf(save_path, sizeof(save_path) - 1, "%s/saves/%s", appdir, curgame_dir);

	if (cr && mkdir(save_path) && access(save_path, W_OK))
		return NULL;

	if (nr)
		snprintf(save_path, sizeof(save_path) - 1, "%s/saves/%s/save%d", appdir, curgame_dir, nr);
	else
		snprintf(save_path, sizeof(save_path) - 1, "%s/saves/%s/autosave", appdir, curgame_dir);

	return save_path; 
}

int debug_init(void)
{
	return 0;
}

void debug_done()
{
}
#ifdef _USE_BROWSE
char *open_file_dialog(void)
{
	OPENFILENAME ofn;
	static char szFile[MAX_PATH];
	static char szOldDir[MAX_PATH];
	static int old_dir_set = 0;
	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL ;
	ofn.lpstrFile = szFile ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	ofn.lpstrFilter = "*.*\0*.*\0main.lua;*.zip\0main.lua;*.zip\0\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	if (!old_dir_set)
		ofn.lpstrInitialDir = NULL;
	else
		ofn.lpstrInitialDir = szOldDir;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_READONLY;
	if (!GetOpenFileName(&ofn))
		return NULL;
	old_dir_set = 1;
	strcpy(szOldDir, ofn.lpstrFile);
	dirname(szOldDir);
	unix_path(ofn.lpstrFile);
	return ofn.lpstrFile;
}
#endif

static char curdir[PATH_MAX];

int setdir(const char *path)
{
	strncpy(curdir, path, sizeof(curdir) - 1);
	return 0;
}

char *getdir(char *path, size_t size)
{
	strncpy(path, curdir, size - 1);
	return path;
}

char *dirpath(const char *path)
{
	static char fp[PATH_MAX * 4];
	if (path[0] == '/')
		return (char*)path;
	strcpy(fp, curdir);
	strcat(fp, "/");
	strcat(fp, path);
	unix_path(fp);
	return fp;
}

int is_absolute_path(const char *path)
{
	if (!path || !*path)
		return 0;
	return (*path == '/' || *path == '\\');
}
