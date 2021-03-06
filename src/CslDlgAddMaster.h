/***************************************************************************
 *   Copyright (C) 2007-2014 by Glen Masgai                                *
 *   mimosius@users.sourceforge.net                                        *
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

#ifndef CSLDLGADDMASTER_H
#define CSLDLGADDMASTER_H

class CslDlgAddMaster: public wxDialog
{
    public:
        // begin wxGlade: CslDlgAddMaster::ids
        // end wxGlade
        CslDlgAddMaster(wxWindow* parent);

        void InitDlg(wxUint32 *fourcc);

    private:
        // begin wxGlade: CslDlgAddMaster::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        void OnCommandEvent(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        bool IsValid();
        void SetGameDefaultValues();
        CslGame* GetSelectedGame(wxUint32 *pos = NULL);

        // begin wxGlade: CslDlgAddMaster::attributes
        wxStaticBox* sizer_master_staticbox;
        wxChoice* m_choiceGameType;
        wxChoice* m_choiceMasterType;
        wxTextCtrl* m_tcAddress;
        wxSpinCtrl* m_scPort;
        wxTextCtrl* m_tcPath;
        wxSizer* m_bsDlg;
        // end wxGlade

        bool m_modified;
        wxUint32 *m_fourcc;
}; // wxGlade: end class


#endif // CSLDLGADDMASTER_H
