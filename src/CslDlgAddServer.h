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

#ifndef CSLDLGADDSERVER_H
#define CSLDLGADDSERVER_H

class CslDlgAddServer: public wxDialog
{
    public:
        // begin wxGlade: CslDlgAddServer::ids
        // end wxGlade

        CslDlgAddServer(wxWindow* parent);

        CslServerInfo* InitDlg(CslServerInfo *info);

    private:
        // begin wxGlade: CslDlgAddServer::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        void UpdatePort(wxInt32 type);

        void OnCommandEvent(wxCommandEvent& event);
        void OnSpinCtrl(wxSpinEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        // begin wxGlade: CslDlgAddServer::attributes
        wxStaticBox* sizer_address_staticbox;
        wxChoice* m_choiceGameType;
        wxTextCtrl* m_tcAddress;
        wxSpinCtrl* m_scGamePort;
        wxSpinCtrl* m_scInfoPort;
        wxSizer* m_bsDlg;
        // end wxGlade

        CslServerInfo *m_info;

}; // wxGlade: end class


#endif // CSLDLGADDSERVER_H
