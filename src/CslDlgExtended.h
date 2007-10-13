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

#ifndef CSLDLGEXTENDED_H
#define CSLDLGEXTENDED_H

#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/imaglist.h>
// begin wxGlade: ::dependencies
#include <wx/listctrl.h>
// end wxGlade
#include "CslEngine.h"
#include "CslTools.h"


WX_DEFINE_ARRAY_PTR(wxStaticText*,t_aLabel);

class CslDlgExtended: public wxDialog
{
    public:
    // begin wxGlade: CslDlgExtended::ids
    // end wxGlade

        CslDlgExtended(wxWindow* parent,int id=wxID_ANY,const wxString& title=wxEmptyString,
                       const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize,
                       long style=wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

        void ListInit(CslEngine *engine);
        void DoShow(CslServerInfo *info);
        CslServerInfo* GetInfo() { return m_info; }

    private:
    // begin wxGlade: CslDlgExtended::methods
    void set_properties();
    void do_layout();
    // end wxGlade

        CslEngine *m_engine;
        CslServerInfo *m_info;

        wxFont m_labelFont;
        t_aLabel m_teamLabel;
                
        wxImageList m_imageList;
        CslListSortHelper m_sortHelper;

        void OnClose(wxCloseEvent& event);
        void OnSize(wxSizeEvent& event);
        void OnColumnLeftClick(wxListEvent& event);
        void OnTimer(wxTimerEvent& event);
        void OnCommandEvent(wxCommandEvent& event);
        void OnPong(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
    // begin wxGlade: CslDlgExtended::attributes
    wxStaticBox* sizer_team_score_staticbox;
    wxListCtrl* list_ctrl_extended;
    wxStaticText* label_team1;
    wxStaticText* label_team2;
    wxStaticText* label_team3;
    wxStaticText* label_team4;
    wxStaticText* label_status;
    wxCheckBox* checkbox_update;
    wxCheckBox* checkbox_update_end;
    wxButton* button_update;
    wxButton* button_close;
    // end wxGlade

        void SetTeamScore();
        void QueryInfo(wxInt32 pid=-1);
        void ListAdjustSize(wxSize size);
        void ListSort(wxInt32 column);
        void ToggleSortArrow();

        static int wxCALLBACK ListSortCompareFunc(long item1,long item2,long data);
}; // wxGlade: end class


#endif // CSLDLGEXTENDED_H