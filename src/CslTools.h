/***************************************************************************
 *   Copyright (C) 2007 by Glen Masgai                                     *
 *   mimosius@gmx.de                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef CSLTOOLS_H
#define CSLTOOLS_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

// since WX > 2.8.4 the listctrl items
// get deselected when sorting (only wxGTK ?) !?
#ifndef CSL_USE_WX_LIST_DESELECT_WORKAROUND
#if wxVERSION_NUMBER > 2804
#endif
#define CSL_USE_WX_LIST_DESELECT_WORKAROUND
#endif

#ifndef DATADIR
#undef LOCALEDIR
#ifdef __WXMAC__
#define DATADIR wxString(wxT("csl.app/Contents/Resources"))
#define LOCALEDIR DATADIR+wxT("/lang")
#else
#define DATADIR ::wxPathOnly(wxTheApp->argv[0])+PATHDIV+wxT("data")
#define LOCALEDIR ::wxPathOnly(wxTheApp->argv[0])+PATHDIV+wxT("lang")
#endif //__WXMAC__
#endif //DATADIR

#define A2U(PSZA_CHART) wxString(wxConvertMB2WX(PSZA_CHART))
#define U2A(PSZT_CHART) (char*)(const char*)wxConvertWX2MB(PSZT_CHART)

#define COLOUR2INT(col) ((col.Red()<<16)|(col.Green()<<8)|col.Blue())
#define INT2COLOUR(int) wxColour((int>>16)&0xFF,(int>>8)&0xFF,int&0xFF)

#define SYSCOLOUR(x) wxSystemSettings::GetColour(x)

#ifdef __WXMSW__
#define PATHDIV wxT("\\")
#define PATHDIVA wxT('\\')
#else
#define PATHDIV wxT("/")
#define PATHDIVA wxT('/')
#endif

#ifdef __WXDEBUG__
void Debug_Printf(const char *DbgFunc, const char *FmtStr,...);
#define LOG_DEBUG(...) Debug_Printf(__FUNCTION__,## __VA_ARGS__);
#else
#define LOG_DEBUG(...)
#endif


extern char* StripColours(char *s,wxUint32 *l,wxUint32 count);
extern bool IsIP(const wxString& s);
extern bool IP2Int(const wxString& s,wxUint32 *ip);
extern wxString FormatSeconds(wxUint32 time);


enum { CSL_SORT_ASC = 0, CSL_SORT_DSC };
enum { CSL_LIST_SORT_INT = 0, CSL_LIST_SORT_UINT, CSL_LIST_SORT_STRING };

class CslListSortHelper
{
    public:
        void Init(wxUint32 mode,wxUint32 type)
        {
            m_sortMode=mode;
            m_sortType=type;
        }

        wxInt32 m_sortMode;
        wxInt32 m_sortType;
};

extern wxUint32 GetTicks();


#endif // CSLTOOLS_H