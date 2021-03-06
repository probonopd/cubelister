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

#ifndef CSLTOOLTIP_H
#define CSLTOOLTIP_H

class CslToolTipEvent;

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(CSL_DLL_GUITOOLS, wxCSL_EVT_TOOLTIP, wxID_ANY)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*CslToolTipEventFunction)(CslToolTipEvent&);

#define CSL_EVT_TOOLTIP(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_TOOLTIP, id, wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(CslToolTipEventFunction, &fn), \
                               (wxObject*)NULL \
                             ),

class CslToolTipEvent : public wxEvent
{
    public:
        CslToolTipEvent(wxFrame *frame = NULL, const wxPoint& pos = wxDefaultPosition) :
                wxEvent(wxID_ANY, wxCSL_EVT_TOOLTIP),
                Parent(frame), Pos(pos), UserWindow(NULL)
        { }

        virtual wxEvent* Clone() const
        {
            return new CslToolTipEvent(*this);
        }

        wxFrame *Parent;
        wxPoint Pos;
        wxString Title;
        wxArrayString Text;
        wxWindow *UserWindow;

    private:
        DECLARE_DYNAMIC_CLASS_NO_ASSIGN(CslToolTipEvent)
};


class CSL_DLL_GUITOOLS CslToolTip : public wxEvtHandler
{
    private:
        CslToolTip();
        ~CslToolTip();

    public:
        static void Init(wxWindow *window, wxInt32 delay, bool top=false);
        static void Reset();

    private:
        static CslToolTip& GetInstance();

        void CreateFrame();

#ifdef __WXMSW__
        void OnEraseBackground(wxEraseEvent& event);
#else
        void OnPaint(wxPaintEvent& event);
#endif
        void OnTimer(wxTimerEvent& event);
        void OnMouseLeave(wxMouseEvent& event);
        void OnMouseButton(wxMouseEvent& event);

        DECLARE_EVENT_TABLE();

        bool m_top;
        bool m_processMouseButton;
        wxFrame *m_frame;
        wxWindow *m_parent;
        wxTimer m_timer;
};

#endif //CSLTOOLTIP_H
