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

#include <wx/wupdlock.h>
#include <wx/clipbrd.h>
#include "engine/CslTools.h"
#include "CslDlgAddServer.h"
#include "CslDlgOutput.h"
#include "CslListCtrlServer.h"
#include "CslStatusBar.h"
#include "CslArt.h"
#include "CslMenu.h"
#include "CslGameConnection.h"
#include "CslSettings.h"
#include "CslIPC.h"

#include "img/ext_green_8.xpm"
#include "img/sortasc_16.xpm"
#include "img/sortdsc_16.xpm"
#include "img/sortasclight_16.xpm"
#include "img/sortdsclight_16.xpm"
#include "img/red_list_16.xpm"
#include "img/green_list_16.xpm"
#include "img/yellow_list_16.xpm"
#include "img/grey_list_16.xpm"
#include "img/red_ext_list_16.xpm"
#include "img/green_ext_list_16.xpm"
#include "img/yellow_ext_list_16.xpm"

enum
{
    SORT_HOST = 0, SORT_DESCRIPTION, SORT_VER,
    SORT_PING, SORT_MODE, SORT_MAP, SORT_TIME,
    SORT_PLAYER, SORT_MM, SORT_UNKNOWN
};


BEGIN_EVENT_TABLE(CslListCtrlServer, CslListCtrl)
    #ifdef __WXMSW__
    EVT_LIST_COL_BEGIN_DRAG(wxID_ANY,CslListCtrlServer::OnColumnDragStart)
    EVT_LIST_COL_END_DRAG(wxID_ANY,CslListCtrlServer::OnColumnDragEnd)
    #endif
    EVT_SIZE(CslListCtrlServer::OnSize)
    EVT_MENU(wxID_ANY,CslListCtrlServer::OnMenu)
    EVT_LIST_COL_CLICK(wxID_ANY,CslListCtrlServer::OnColumnLeftClick)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,CslListCtrlServer::OnItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,CslListCtrlServer::OnItemSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY,CslListCtrlServer::OnItemDeselected)
    EVT_CONTEXT_MENU(CslListCtrlServer::OnContextMenu)
    EVT_KEY_DOWN(CslListCtrlServer::OnKeyDown)
END_EVENT_TABLE()


enum
{
    LIST_IMG_GREEN,
    LIST_IMG_YELLOW,
    LIST_IMG_RED,
    LIST_IMG_GREY,
    LIST_IMG_GREEN_EXT,
    LIST_IMG_YELLOW_EXT,
    LIST_IMG_RED_EXT,
    LIST_IMG_SORT_ASC,
    LIST_IMG_SORT_DSC,
    LIST_IMG_SORT_ASC_LIGHT,
    LIST_IMG_SORT_DSC_LIGHT,
    LIST_IMG_GAMES_START,
};


CslListCtrlServer::CslListCtrlServer(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                     const wxSize& size,long style,
                                     const wxValidator& validator, const wxString& name) :
        CslListCtrl(parent,id,pos,size,style,validator,name),
        m_id(id),m_engine(NULL),m_masterSelected(false),
        m_sibling(NULL),m_dontUpdateInfo(false),m_dontRemoveOnDeselect(false),
#ifdef __WXMSW__
        m_dontAdjustSize(false),
#endif
        m_filterFlags(0)
{
    wxArtProvider::Push(new CslArt);
}

CslListCtrlServer::~CslListCtrlServer()
{
// avoids an assertion
#ifndef __WXMAC__
    ListClear();
#endif
}

#ifdef __WXMSW__
void CslListCtrlServer::OnColumnDragStart(wxListEvent& WXUNUSED(event))
{
    m_dontAdjustSize=true;
}

void CslListCtrlServer::OnColumnDragEnd(wxListEvent& WXUNUSED(event))
{
    m_dontAdjustSize=false;
}
#endif

void CslListCtrlServer::OnSize(wxSizeEvent& event)
{
    event.Skip();

#ifdef __WXMSW__
    if (m_dontAdjustSize)
        return;
#endif

    Freeze();
    ListAdjustSize(event.GetSize());
    Thaw();
}

void CslListCtrlServer::OnKeyDown(wxKeyEvent &event)
{
    wxInt32 i;
    wxInt32 code=event.GetKeyCode();

    if (event.ControlDown() && code=='A')
    {
        m_dontUpdateInfo=true;
        for (i=0;i<GetItemCount();i++)
            SetItemState(i,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
        m_dontUpdateInfo=false;
    }
    else if (code==WXK_DELETE || code == WXK_NUMPAD_DELETE)
    {
        //TODO multiple events?
        if (event.ShiftDown())
            ListDeleteServers();
        else if (m_id!=CSL_LIST_MASTER)
            ListRemoveServers();
    }

    event.Skip();
}

void CslListCtrlServer::OnColumnLeftClick(wxListEvent& event)
{
    ListSort(event.GetColumn());
}

void CslListCtrlServer::OnItemActivated(wxListEvent& event)
{
    wxListItem item;

    item.SetId(event.GetIndex());
    CslListServerData *server=(CslListServerData*)GetItemData(item);
    CslGameConnection::Prepare(server->Info);

    event.Skip();
}

void CslListCtrlServer::OnItemSelected(wxListEvent& event)
{
    wxListItem item;
    item.SetId(event.GetIndex());
    GetItem(item);

    CslListServerData *server=(CslListServerData*)GetItemData(item);
    m_selected.Add(server);

    if (m_dontUpdateInfo)
        return;

    event.SetClientData((void*)server->Info);
    event.Skip();
}

void CslListCtrlServer::OnItemDeselected(wxListEvent& event)
{
    if (m_dontRemoveOnDeselect)
        return;

    wxListItem item;
    item.SetId(event.GetIndex());
    GetItem(item);

    CslListServerData *server=(CslListServerData*)GetItemData(item);
    if (m_selected.Index(server)!=wxNOT_FOUND)
        m_selected.Remove(server);

    event.Skip();
}

void CslListCtrlServer::OnContextMenu(wxContextMenuEvent& event)
{
    wxInt32 selected;
    wxMenu menu,*sub,*filter=NULL;
    wxMenuItem *item;
    CslServerInfo *info=NULL;
    wxPoint point=event.GetPosition();

    //from keyboard
    if (point==wxDefaultPosition)
        point=wxGetMousePosition();

    if ((selected=m_selected.GetCount())>0)
        info=m_selected.Item(0)->Info;

    if (selected==1)
    {
        wxUint32 caps=info->GetGame().GetCapabilities();

        CslMenu::AddItem(&menu,MENU_SERVER_CONNECT,MENU_SERVER_CONN_STR,wxART_CONNECT);
        if (CSL_CAP_CONNECT_PASS(caps))
            CslMenu::AddItem(&menu,MENU_SERVER_CONNECT_PW,MENU_SERVER_CONN_PW_STR,wxART_CONNECT_PW);

        sub=new wxMenu();
        item=menu.AppendSubMenu(sub,MENU_SERVER_EXT_STR);
        item->SetBitmap(GET_ART_MENU(wxART_ABOUT));
        if (info->ExtInfoStatus!=CSL_EXT_STATUS_OK || !CslEngine::PingOk(*info,g_cslSettings->updateInterval))
            item->Enable(false);
        else
        {
            CslMenu::AddItem(sub,MENU_SERVER_EXT_FULL,MENU_SERVER_EXT_FULL_STR,wxART_ABOUT);
            sub->AppendSeparator();
            CslMenu::AddItem(sub,MENU_SERVER_EXT_MICRO,MENU_SERVER_EXT_MICRO_STR,wxART_EXTINFO_MICRO);
            CslMenu::AddItem(sub,MENU_SERVER_EXT_MINI,MENU_SERVER_EXT_MINI_STR,wxART_EXTINFO_MINI);
            CslMenu::AddItem(sub,MENU_SERVER_EXT_DEFAULT,MENU_SERVER_EXT_DEFAULT_STR,wxART_EXTINFO_DEFAULT);
        }

        menu.AppendSeparator();

        if (CslGameConnection::IsPlaying())
        {
            menu.Enable(MENU_SERVER_CONNECT,false);
            if (CSL_CAP_CONNECT_PASS(caps))
                menu.Enable(MENU_SERVER_CONNECT_PW,false);
        }
    }

    if (m_id==CSL_LIST_MASTER && selected)
        CslMenu::AddItem(&menu,MENU_SERVER_ADD,MENU_SERVER_MAS_ADD_STR,wxART_ADD_BOOKMARK);

    else if (m_id==CSL_LIST_FAVOURITE)
    {
        CslMenu::AddItem(&menu,MENU_SERVER_ADD,MENU_SERVER_FAV_ADD_STR,wxART_ADD_BOOKMARK);
        if (selected)
            CslMenu::AddItem(&menu,MENU_SERVER_REM,MENU_SERVER_FAV_REM_STR,wxART_DEL_BOOKMARK);
    }

    if (selected>0)
    {
        CslMenu::AddItem(&menu,MENU_SERVER_DEL,selected>1 ?
                         MENU_SERVER_DELM_STR:MENU_SERVER_DEL_STR,wxART_DELETE);
        menu.AppendSeparator();

        sub=new wxMenu();
        menu.AppendSubMenu(sub,MENU_SERVER_COPY_STR);
        CslMenu::AddItem(sub,MENU_SERVER_COPY_CON,MENU_SERVER_COPY_CON_STR,wxART_CSL);
        CslMenu::AddItem(sub,MENU_SERVER_COPY_FAV,MENU_SERVER_COPY_FAV_STR,wxART_CSL);
        CslMenu::AddItem(sub,MENU_SERVER_COPY_CONFAV,MENU_SERVER_COPY_CONFAV_STR,wxART_CSL);
        CslMenu::AddItem(&menu,MENU_SERVER_COPY_SERVER,MENU_SERVER_COPY_SERVER_STR,wxART_COPY);
        menu.AppendSeparator();

        filter=(wxMenu*)1;
    }
    else if (m_id==CSL_LIST_FAVOURITE)
    {
        filter=(wxMenu*)1;
        menu.AppendSeparator();
    }

    if (filter)
    {
        filter=new wxMenu();
        item=menu.AppendSubMenu(filter,MENU_SERVER_FILTER_STR);
    }
    else
        filter=&menu;

    item=&CslMenu::AddItem(filter,MENU_SERVER_FILTER_OFF,
                           MENU_SERVER_FILTER_OFF_STR,wxART_NONE,wxITEM_CHECK);
    item->Check(*m_filterFlags&CSL_FILTER_OFFLINE);
    item=&CslMenu::AddItem(filter,MENU_SERVER_FILTER_FULL,
                           MENU_SERVER_FILTER_FULL_STR,wxART_NONE,wxITEM_CHECK);
    item->Check((*m_filterFlags&CSL_FILTER_FULL)!=0);
    item=&CslMenu::AddItem(filter,MENU_SERVER_FILTER_EMPTY,
                           MENU_SERVER_FILTER_EMPTY_STR,wxART_NONE,wxITEM_CHECK);
    item->Check((*m_filterFlags&CSL_FILTER_EMPTY)!=0);
    item->Enable(!(*m_filterFlags&CSL_FILTER_NONEMPTY));
    item=&CslMenu::AddItem(filter,MENU_SERVER_FILTER_NONEMPTY,
                           MENU_SERVER_FILTER_NONEMPTY_STR,wxART_NONE,wxITEM_CHECK);
    item->Check((*m_filterFlags&CSL_FILTER_NONEMPTY)!=0);
    item->Enable(!(*m_filterFlags&CSL_FILTER_EMPTY));
    item=&CslMenu::AddItem(filter,MENU_SERVER_FILTER_MM2,
                           MENU_SERVER_FILTER_MM2_STR,wxART_NONE,wxITEM_CHECK);
    item->Check((*m_filterFlags&CSL_FILTER_MM2)!=0);
    item=&CslMenu::AddItem(filter,MENU_SERVER_FILTER_MM3,
                           MENU_SERVER_FILTER_MM3_STR,wxART_NONE,wxITEM_CHECK);
    item->Check((*m_filterFlags&CSL_FILTER_MM3)!=0);

    if (m_id==CSL_LIST_MASTER)
    {
        filter->AppendSeparator();
        item=&CslMenu::AddItem(filter,MENU_SERVER_FILTER_VER,MENU_SERVER_FILTER_VER_STR,wxART_NONE,wxITEM_CHECK);
        item->Enable(selected>0 || m_filterVersion>-1);
        item->Check(m_filterVersion>-1);
    }

    point=ScreenToClient(point);
    PopupMenu(&menu,point);
}

void CslListCtrlServer::ListRemoveServers()
{
    wxInt32 i;
    CslServerInfo *info;
    wxListItem item;

    for (i=m_selected.GetCount()-1;i>=0;i--)
    {
        CslListServerData *ls=m_selected.Item(i);
        info=ls->Info;

        if (ListFindItem(info,item)==wxNOT_FOUND)
            continue;

        ListDeleteItem(&item);
        m_selected.RemoveAt(i);
        m_servers.Remove(ls);

        switch (m_id)
        {
            case CSL_LIST_FAVOURITE:
                info->RemoveFavourite();
                break;
            default:
                break;
        }
    }
}

#define CSL_DELETE_YESNOCANCEL_STR _("\nChoose Yes to keep these servers, " \
                                     _L_"No to delete them or\nCancel the operation.")

void CslListCtrlServer::ListDeleteServers()
{
    wxInt32 i;
    wxString msg,s;
    CslServerInfo *info;
    wxInt32 skipFav=wxCANCEL,skipStats=wxCANCEL;

    for (i=m_selected.GetCount()-1;i>=0;i--)
    {
        msg.Empty();
        CslListServerData *ls=m_selected.Item(i);
        info=ls->Info;

        if (skipFav==wxCANCEL && info->IsFavourite())
        {
            msg=_("You are about to delete servers which are also favourites!\n");
            msg+=CSL_DELETE_YESNOCANCEL_STR;
            skipFav=wxMessageBox(msg,_("Warning"),wxYES_NO|wxCANCEL|wxICON_WARNING,this);
            if (skipFav==wxCANCEL)
                return;
            if (skipFav!=wxNO)
                continue;
        }
        if (skipStats==wxCANCEL && info->HasStats())
        {
            msg=_("You are about to delete servers which have statistics!\n");
            msg+=CSL_DELETE_YESNOCANCEL_STR;
            skipStats=wxMessageBox(msg,_("Warning"),wxYES_NO|wxCANCEL|wxICON_WARNING,this);
            if (skipStats==wxCANCEL)
                return;
            if (skipStats!=wxNO)
                continue;
        }
        if (CslGameConnection::IsWaiting() && CslGameConnection::GetInfo()==info)
            CslGameConnection::Reset();
    }

    for (i=m_selected.GetCount()-1;i>=0;i--)
    {
        wxListItem item;
        CslListServerData *ls=m_selected.Item(i);
        info=ls->Info;

        if (ListFindItem(info,item)==wxNOT_FOUND)
            continue;

        if (ls->Info->IsLocked())
        {
            msg=wxString::Format(_("Server \"%s\" is currently locked,\nso deletion is not possible!"),
                                 info->GetBestDescription().c_str());
            wxMessageBox(msg,_("Error"),wxICON_ERROR,this);
            continue;
        }

        if (info->IsFavourite() && skipFav!=wxNO)
            continue;

        if (info->HasStats() && skipStats!=wxNO)
            continue;

        RemoveServer(ls,info,i);

        if (m_id==CSL_LIST_MASTER && info->IsFavourite())
            m_sibling->RemoveServer(NULL,info,-1);
        else if (m_id==CSL_LIST_FAVOURITE)
            m_sibling->RemoveServer(NULL,info,-1);

        wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED,MENU_SERVER_DEL);
        event.SetClientData((void*)info);
        wxPostEvent(m_parent,event);
    }
}
#undef CSL_DELETE_YESNOCANCEL_STR

void CslListCtrlServer::OnMenu(wxCommandEvent& event)
{
    CslServerInfo *info;
    wxListItem item;
    wxString s;
    wxUint32 i=0;
    wxInt32 c=0;

    wxInt32 id=event.GetId();

    switch (id)
    {
        case MENU_SERVER_CONNECT:
        case MENU_SERVER_CONNECT_PW:
        {
            wxInt32 pass=id==MENU_SERVER_CONNECT ?
                         CslGameConnection::NO_PASS :
                         CslGameConnection::ASK_PASS;
            CslGameConnection::Prepare(m_selected.Item(0)->Info,pass);
            event.Skip();
            break;
        }

        case MENU_SERVER_EXT_MICRO:
        case MENU_SERVER_EXT_MINI:
        case MENU_SERVER_EXT_DEFAULT:
        case MENU_SERVER_EXT_FULL:
        {
            info=m_selected.Item(0)->Info;
            event.SetClientData((void*)info);
            event.Skip();
            break;
        }

        case MENU_SERVER_ADD:
        {
            switch (m_id)
            {
                case CSL_LIST_MASTER:
                {
                    bool sort=false;
                    for (i=0;i<m_selected.GetCount();i++)
                    {
                        info=m_selected.Item(i)->Info;

                        if (ListFindItem(info,item)==wxNOT_FOUND)
                            continue;
                        info->SetFavourite();
                        sort=true;
                        m_sibling->ListUpdateServer(info);
                        /*if (!info->IsDefault())
                        {
                            item.SetId(c);
                            DeleteItem(item);
                        }*/
                    }
                    if (sort)
                        m_sibling->ListSort();
                    break;
                }
                case CSL_LIST_FAVOURITE:
                {
                    CslDlgAddServer dlg(wxTheApp->GetTopWindow());
                    info=new CslServerInfo;
                    dlg.InitDlg(m_engine,info);
                    if (dlg.ShowModal()!=wxID_OK)
                    {
                        delete info;
                        break;
                    }
                    ListUpdateServer(info);
                    ListSort();
                    break;
                }
            }
            break;
        }

        case MENU_SERVER_REM:
        {
            ListRemoveServers();
            break;
        }

        case MENU_SERVER_DEL:
        {
            ListDeleteServers();
            break;
        }

        case MENU_SERVER_COPY_CON:
        case MENU_SERVER_COPY_CONFAV:
        case MENU_SERVER_COPY_FAV:
        {
            wxString s1;
            bool pass=false;

            for (i=0;i<m_selected.GetCount();i++)
            {
                info=m_selected.Item(i)->Info;

                if (ListFindItem(info,item)==wxNOT_FOUND)
                    continue;

                if (!info->Password.IsEmpty())
                {
                    pass=true;
                    break;
                }
            }

            if (pass)
            {
                pass=wxMessageBox(_("Copy server passwords too?"),_("Warning"),
                                  wxYES_NO|wxICON_WARNING,wxTheApp->GetTopWindow())==wxYES;
            }

            for (i=0;i<m_selected.GetCount();i++)
            {
                info=m_selected.Item(i)->Info;

                if (ListFindItem(info,item)==wxNOT_FOUND)
                    continue;

                if (!s.IsEmpty())
                    s<<wxT("\r\n");

                s1.Empty();

                s<<CSL_URI_SCHEME_STR;

                if (pass && !info->Password.IsEmpty())
                {
                    s1<<info->Password<<wxT("@");
                    s1.Replace(wxT(" "),wxT("%20"));
                    s<<s1;
                    s1.Empty();
                }

                s<<info->Host;
                if (info->GetGame().GetDefaultPort()!=info->Port)
                    s<<wxT(":")<<info->Port;

                s<<wxT("?")<<CSL_URI_GAME_STR;
                s1<<info->GetGame().GetName();
                s1.Replace(wxT(" "),wxT("%20"));
                s<<wxT("=")<<s1<<wxT("&");
                s1.Empty();

                if (id==MENU_SERVER_COPY_CON || id==MENU_SERVER_COPY_CONFAV)
                    s1<<CSL_URI_ACTION_STR<<wxT("=")<<CSL_URI_ACTION_CONNECT_STR;
                if (id==MENU_SERVER_COPY_FAV || id==MENU_SERVER_COPY_CONFAV)
                {
                    if (!s1.IsEmpty())
                        s1<<wxT("&");
                    s1<<CSL_URI_ACTION_STR<<wxT("=")<<CSL_URI_ACTION_ADDFAV_STR;
                }
                s<<s1;
            }

            if (s.IsEmpty())
                break;

            if (wxTheClipboard->Open())
            {
                wxTheClipboard->SetData(new wxTextDataObject(s));
                wxTheClipboard->Close();
            }
            break;
        }

        case MENU_SERVER_COPY_SERVER:
        {
            wxString s1;
            wxUint16 port;

            item.SetMask(wxLIST_MASK_TEXT);

            for (i=0;i<m_selected.GetCount();i++)
            {
                info=m_selected.Item(i)->Info;

                if (ListFindItem(info,item)==wxNOT_FOUND)
                    continue;

                port=info->GetGame().GetDefaultPort();
                if (info->Port!=port)
                    port=info->Port;
                else
                    port=0;

                for (c=0;c<GetColumnCount();c++)
                {
                    item.SetColumn(c);
                    GetItem(item);
                    s1=item.GetText();
                    if (!s1.IsEmpty())
                    {
                        s<<s1;
                        if (port)
                        {
                            GetColumn(c,item);
                            if (item.GetText()==_("Host"))
                            {
                                s<<wxT(":")<<port;
                                port=0;
                            }
                        }
                        s<<wxT("  ");
                    }
                }
                if (!s.IsEmpty())
                    s<<wxT("\r\n");
            }

            if (s.IsEmpty())
                break;

            if (wxTheClipboard->Open())
            {
                wxTheClipboard->SetData(new wxTextDataObject(s));
                wxTheClipboard->Close();
            }
            break;
        }

        case MENU_SERVER_FILTER_OFF:
            i=CSL_FILTER_OFFLINE;
        case MENU_SERVER_FILTER_FULL:
            if (i==0)
                i=CSL_FILTER_FULL;
        case MENU_SERVER_FILTER_EMPTY:
            if (i==0)
                i=CSL_FILTER_EMPTY;
        case MENU_SERVER_FILTER_NONEMPTY:
            if (i==0)
                i=CSL_FILTER_NONEMPTY;
        case MENU_SERVER_FILTER_MM2:
            if (i==0)
                i=CSL_FILTER_MM2;
        case MENU_SERVER_FILTER_MM3:
            if (i==0)
                i=CSL_FILTER_MM3;
        case MENU_SERVER_FILTER_VER:
        {
            if (event.IsChecked())
            {
                if (i==0)
                {
                    if (m_selected.GetCount())
                        m_filterVersion=m_selected.Item(0)->Info->Protocol;
                    break;
                }
                *m_filterFlags|=i;
            }
            else
            {
                if (i==0)
                {
                    m_filterVersion=-1;
                    break;
                }
                *m_filterFlags&=~i;
            }
            break;
        }

        default:
            break;
    }

    if (id>=MENU_SERVER_FILTER_OFF && id<=MENU_SERVER_FILTER_VER)
    {
        ListFilter();
        event.SetClientData((void*)m_id);
        event.Skip();
    }
}

void CslListCtrlServer::ListInit(CslEngine *engine,CslListCtrlServer *sibling)
//CslDlgExtended *extendedDlg);
{
    m_engine=engine;
    m_sibling=sibling;
    m_filterFlags=m_id==CSL_LIST_MASTER ? &g_cslSettings->filterMaster:&g_cslSettings->filterFavourites;
    m_filterVersion=-1;

    ListCreateGameBitmaps();

    wxListItem item;

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_FORMAT);
    item.SetImage(-1);

    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Host"));
    InsertColumn(0,item);
    SetColumn(0,item);

    item.SetText(_("Description"));
    InsertColumn(1,item);
    SetColumn(1,item);

    item.SetText(_("Version"));
    InsertColumn(2,item);
    SetColumn(2,item);

    item.SetAlign(wxLIST_FORMAT_RIGHT);
    item.SetText(_("Ping"));
    InsertColumn(3,item);
    SetColumn(3,item);

    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Mode"));
    InsertColumn(4,item);
    SetColumn(4,item);

    item.SetText(_("Map"));
    InsertColumn(5,item);
    SetColumn(5,item);

    item.SetAlign(wxLIST_FORMAT_RIGHT);
    item.SetText(_("Time"));
    InsertColumn(6,item);
    SetColumn(6,item);

    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Player"));
    InsertColumn(7,item);
    SetColumn(7,item);

    item.SetText(_("MM"));
    InsertColumn(8,item);
    SetColumn(8,item);

    m_sortHelper.Init(CSL_SORT_DSC,SORT_PLAYER);
    ToggleSortArrow();
}

void CslListCtrlServer::ListAdjustSize(const wxSize& size)
{
    wxInt32 w=size.x-8;

    SetColumnWidth(0,(wxInt32)(w*g_cslSettings->colServerS1));
    SetColumnWidth(1,(wxInt32)(w*g_cslSettings->colServerS2));
    SetColumnWidth(2,(wxInt32)(w*g_cslSettings->colServerS3));
    SetColumnWidth(3,(wxInt32)(w*g_cslSettings->colServerS4));
    SetColumnWidth(4,(wxInt32)(w*g_cslSettings->colServerS5));
    SetColumnWidth(5,(wxInt32)(w*g_cslSettings->colServerS6));
    SetColumnWidth(6,(wxInt32)(w*g_cslSettings->colServerS7));
    SetColumnWidth(7,(wxInt32)(w*g_cslSettings->colServerS8));
    SetColumnWidth(8,(wxInt32)(w*g_cslSettings->colServerS9));
}

void CslListCtrlServer::ListCreateGameBitmaps()
{
    wxInt32 width;

#ifdef __WXMSW__
    width=18;
    m_imageList.Create(18,16,true);

    wxIcon icon;
    m_imageList.Add(AdjustIconSize(green_list_16_xpm,wxNullIcon,wxSize(18,16),wxPoint(0,0)));
    m_imageList.Add(AdjustIconSize(yellow_list_16_xpm,wxNullIcon,wxSize(18,16),wxPoint(0,0)));
    m_imageList.Add(AdjustIconSize(red_list_16_xpm,wxNullIcon,wxSize(18,16),wxPoint(0,0)));
    m_imageList.Add(AdjustIconSize(grey_list_16_xpm,wxNullIcon,wxSize(18,16),wxPoint(0,0)));
    m_imageList.Add(AdjustIconSize(green_ext_list_16_xpm,wxNullIcon,wxSize(18,16),wxPoint(0,0)));
    m_imageList.Add(AdjustIconSize(yellow_ext_list_16_xpm,wxNullIcon,wxSize(18,16),wxPoint(0,0)));
    m_imageList.Add(AdjustIconSize(red_ext_list_16_xpm,wxNullIcon,wxSize(18,16),wxPoint(0,0)));
    m_imageList.Add(AdjustIconSize(sortasc_16_xpm,wxNullIcon,wxSize(18,16),wxPoint(0,0)));
    m_imageList.Add(AdjustIconSize(sortdsc_16_xpm,wxNullIcon,wxSize(18,16),wxPoint(0,0)));
    m_imageList.Add(AdjustIconSize(sortasclight_16_xpm,wxNullIcon,wxSize(18,16),wxPoint(0,0)));
    m_imageList.Add(AdjustIconSize(sortdsclight_16_xpm,wxNullIcon,wxSize(18,16),wxPoint(0,0)));
#else
    width=16;
    m_imageList.Create(16,16,true);

    m_imageList.Add(wxBitmap(green_list_16_xpm));
    m_imageList.Add(wxBitmap(yellow_list_16_xpm));
    m_imageList.Add(wxBitmap(red_list_16_xpm));
    m_imageList.Add(wxBitmap(grey_list_16_xpm));
    m_imageList.Add(wxBitmap(green_ext_list_16_xpm));
    m_imageList.Add(wxBitmap(yellow_ext_list_16_xpm));
    m_imageList.Add(wxBitmap(red_ext_list_16_xpm));
    m_imageList.Add(wxBitmap(sortasc_16_xpm));
    m_imageList.Add(wxBitmap(sortdsc_16_xpm));
    m_imageList.Add(wxBitmap(sortasclight_16_xpm));
    m_imageList.Add(wxBitmap(sortdsclight_16_xpm));
#endif

    //now create the icons for favourites list
    if (m_id==CSL_LIST_FAVOURITE)
    {
        wxBitmap bmpExt(ext_green_8_xpm);

        //magic colour and brush for transparency
        wxColour magicColour(255,0,255);
        wxBrush magicBrush(magicColour);

        wxMemoryDC dc;

        vector<CslGame*>& games=m_engine->GetGames();
        loopv(games)
        {
            const char **icon=games[i]->GetIcon(16);
#ifdef __WXMSW__
            wxBitmap bmpGame=icon ? (AdjustIconSize(icon,wxNullIcon,wxSize(18,16),wxPoint(0,0))):wxBitmap(width,16);
#else
            wxBitmap bmpGame=icon ? wxBitmap(icon):wxBitmap(16,width);
#endif
            m_imageList.Add(bmpGame);

            wxBitmap bmp(width,16);
            dc.SelectObject(bmp);
            //draw transparent background
            dc.SetBrush(magicBrush);
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(0,0,width,16);
            //draw the bitmaps
            dc.DrawBitmap(bmpGame,0,0,true);
            dc.DrawBitmap(bmpExt,8,8,true);
            //unref the bitmap and set the mask
            dc.SelectObject(wxNullBitmap);
            bmp.SetMask(new wxMask(bmp,magicColour));

            m_imageList.Add(bmp);
        }
    }

    SetImageList(&m_imageList,wxIMAGE_LIST_SMALL);
}

void CslListCtrlServer::Highlight(wxInt32 type,bool high,bool sort,CslServerInfo *info,wxListItem *item)
{
    wxUint32 i,t;
    wxListItem it;

    wxColour colour;

    if (info)
    {
        if (!item)
        {
            if (ListFindItem(info,it)==wxNOT_FOUND)
                return;
            item=&it;
        }

        CslListServerData *server=(CslListServerData*)GetItemData(*item);
        t=server->SetHighlight(type,high);

        if (t&CSL_HIGHLIGHT_LOCKED)
            colour=g_cslSettings->colServerPlay;
        else if (t&CSL_HIGHLIGHT_FOUND_SERVER || t&CSL_HIGHLIGHT_FOUND_PLAYER)
            colour=g_cslSettings->colServerHigh;
        else if (t&CSL_HIGHLIGHT_SEARCH_PLAYER)
            colour=wxColour(g_cslSettings->colServerOff);
        else
            colour=GetBackgroundColour();

        SetItemBackgroundColour(*item,colour);
    }
    else
    {
        for (i=0;i<m_servers.GetCount();i++)
        {
            info=m_servers.Item(i)->Info;

            if (ListFindItem(info,it)==wxNOT_FOUND)
                continue;

            t=m_servers.Item(i)->SetHighlight(type,high);

            if (t&CSL_HIGHLIGHT_FOUND_SERVER || t&CSL_HIGHLIGHT_FOUND_PLAYER)
                colour=g_cslSettings->colServerHigh;
            else if (t&CSL_HIGHLIGHT_LOCKED)
                colour=g_cslSettings->colServerPlay;
            else
                colour=GetBackgroundColour();

            SetItemBackgroundColour(it,colour);
        }
    }

    if (sort)
    {
        ListSort();

#ifndef __WXMSW__
        //removes flicker after sorting
        wxIdleEvent idle;
        wxTheApp->SendIdleEvents(this,idle);
#endif
    }
}

wxUint32 CslListCtrlServer::GetPlayerCount()
{
    wxUint32 c=0,i;

    for (i=0;i<m_servers.GetCount();i++)
        if (m_servers.Item(i)->Players>0 &&
            CslEngine::PingOk(*m_servers.Item(i)->Info,g_cslSettings->updateInterval))
            c+=m_servers.Item(i)->Players;

    return c;
}

wxInt32 CslListCtrlServer::ListFindItem(CslServerInfo *info,wxListItem& item)
{
    wxInt32 i;

    for (i=0;i<GetItemCount();i++)
    {
        item.SetId(i);
        CslServerInfo *infoCmp=((CslListServerData*)GetItemData(item))->Info;
        if (info==infoCmp)
            return i;
    }

    return wxNOT_FOUND;
}

bool CslListCtrlServer::ListSearchItemMatches(CslServerInfo *info)
{
    wxString s;

    s=info->Host.Lower();
    if (s.Find(m_searchString)!=wxNOT_FOUND)
        return true;
    else
    {
        s=info->Description.Lower();
        if (s.Find(m_searchString)!=wxNOT_FOUND)
            return true;
        else
        {
            s=wxString(info->Version.Lower());
            if (s.Find(m_searchString)!=wxNOT_FOUND)
                return true;
            else
            {
                s=info->GameMode.Lower();
                if (s.Find(m_searchString)!=wxNOT_FOUND)
                    return true;
                else
                {
                    s=info->Map.Lower();
                    if (s.Find(m_searchString)!=wxNOT_FOUND)
                        return true;
                }
            }
        }
    }

    return false;
}

bool CslListCtrlServer::ListFilterItemMatches(CslServerInfo *info)
{
    if (m_filterVersion>-1 && info->Protocol!=m_filterVersion)
        return true;
    else if (*m_filterFlags&CSL_FILTER_OFFLINE && !CslEngine::PingOk(*info,g_cslSettings->updateInterval))
        return true;
    else if (*m_filterFlags&CSL_FILTER_FULL && info->PlayersMax>0 &&
             info->Players>=info->PlayersMax)
        return true;
    else if (*m_filterFlags&CSL_FILTER_EMPTY && info->Players==0)
        return true;
    else if (*m_filterFlags&CSL_FILTER_NONEMPTY && info->Players>0)
        return true;
    else if (*m_filterFlags&CSL_FILTER_MM2 && info->MM==2)
        return true;
    else if (*m_filterFlags&CSL_FILTER_MM3 && info->MM==3)
        return true;

    return false;
}

void CslListCtrlServer::GetToolTipText(wxInt32 row,wxString& title,wxArrayString& text)
{
    if (row<GetItemCount())
    {
        wxInt32 i;
        wxListItem item,column;

        column.SetMask(wxLIST_MASK_TEXT);
        item.SetMask(wxLIST_MASK_TEXT);
        item.SetId(row);

        for (i=0;i<GetColumnCount();i++)
        {
            item.SetColumn(i);
            GetItem(item);
            GetColumn(i,column);

            const wxString& s=item.GetText();

            if (!s.IsEmpty())
            {
                text.Add(column.GetText());
                text.Add(s);
            }
        }

        title=_("Server information");
    }
}

bool CslListCtrlServer::ListUpdateServer(CslServerInfo *info)
{
    if (!info)
    {
        wxASSERT_MSG(info,wxT("invalid info"));
        return false;
    }

    wxInt32 i,j;
    wxString s;
    wxListItem item;
    bool found=false;
    CslListServerData *infoCmp=NULL;

    i=m_servers.GetCount();
    for (j=0;j<i;j++)
    {
        if (m_servers.Item(j)->Info==info)
        {
            infoCmp=m_servers.Item(j);
            break;
        }
    }

    j=ListFindItem(info,item);
    if (j==wxNOT_FOUND)
        i=GetItemCount();
    else
        i=j;

    item.SetId(i);
    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_DATA);

    // filter
    found=m_filterFlags||m_filterVersion>-1 ? ListFilterItemMatches(info) : false;
    if (found && j!=wxNOT_FOUND)
    {
        ListDeleteItem(&item);
        wxInt32 pos=m_selected.Index(infoCmp);
        if (pos!=wxNOT_FOUND)
            m_selected.RemoveAt(pos);
    }

    if (j==wxNOT_FOUND)
    {
        if (!infoCmp)
        {
            infoCmp=new CslListServerData(info);
            m_servers.Add(infoCmp);
        }
        else
            infoCmp->Reset();

        if (found)
        {
            if (!m_searchString.IsEmpty())
                return ListSearchItemMatches(info);
            return true;
        }

        item.SetData((long)infoCmp);
        InsertItem(item);

        SetItem(i,0,info->Host);
    }
    else
    {
        if (found)
        {
            if (!m_searchString.IsEmpty())
                return ListSearchItemMatches(info);
            return true;
        }
        infoCmp=(CslListServerData*)GetItemData(item);
    }

    if (CslEngine::PingOk(*info,g_cslSettings->updateInterval) || info->PingResp)
    {
        if (infoCmp->Description!=info->Description)
            SetItem(i,1,info->Description);

        if (infoCmp->Protocol!=info->Protocol)
            SetItem(i,2,info->Version);

        if (infoCmp->Ping!=info->Ping)
            SetItem(i,3,wxString::Format(wxT("%d"),info->Ping));

        if (infoCmp->GameMode!=info->GameMode)
            SetItem(i,4,info->GameMode);

        if (infoCmp->Map!=info->Map)
            SetItem(i,5,info->Map);

        if (infoCmp->TimeRemain!=info->TimeRemain)
        {
            if (info->TimeRemain<0)
                s=_("no limit");
            else
                s=s.Format(wxT("%d"),info->TimeRemain);
            SetItem(i,6,s);
        }

        if (infoCmp->Players!=info->Players ||
            infoCmp->PlayersMax!=info->PlayersMax)
        {
            if (info->PlayersMax<=0)
                s=s.Format(wxT("%d"),info->Players);
            else
                s=s.Format(wxT("%d/%d"),info->Players,info->PlayersMax);
            SetItem(i,7,s);
        }

        if (infoCmp->MM!=info->MM)
            SetItem(i,8,info->MMDescription);
    }
    else
    {
        if (info->Description.IsEmpty() && !info->DescriptionOld.IsEmpty())
            SetItem(i,1,info->DescriptionOld);
    }

    wxColour colour(GetTextColour());

    if (!CslEngine::PingOk(*info,g_cslSettings->updateInterval))
        colour=g_cslSettings->colServerOff;
    else if (CSL_SERVER_IS_BAN(info->MM) ||
             CSL_SERVER_IS_PASSWORD(info->MM) ||
             CSL_SERVER_IS_BLACKLIST(info->MM))
        colour=g_cslSettings->colServerMM3;
    else
    {
        if (info->Players>0 && info->Players>=info->PlayersMax && info->PlayersMax>-1)
            colour=g_cslSettings->colServerFull;
        else if (CSL_SERVER_IS_PRIVATE(info->MM))
            colour=g_cslSettings->colServerMM3;
        else if (CSL_SERVER_IS_LOCKED(info->MM))
            colour=g_cslSettings->colServerMM2;
        else if (CSL_SERVER_IS_VETO(info->MM))
            colour=g_cslSettings->colServerMM1;
        else if (info->Players==0)
            colour=g_cslSettings->colServerEmpty;
    }

    SetItemTextColour(i,colour);

    // search
    found=m_searchString.IsEmpty() ? false : ListSearchItemMatches(info);
    Highlight(CSL_HIGHLIGHT_FOUND_SERVER,found,false,info,&item);

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_IMAGE|wxLIST_MASK_DATA);
    if (m_id==CSL_LIST_MASTER)
    {
        if (!CslEngine::PingOk(*info,g_cslSettings->updateInterval))
            i=LIST_IMG_GREY;
        else if (info->Ping>(wxInt32)g_cslSettings->pingbad)
            i=info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE ? LIST_IMG_RED_EXT : LIST_IMG_RED;
        else if (info->Ping>(wxInt32)g_cslSettings->pinggood)
            i=info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE ? LIST_IMG_YELLOW_EXT : LIST_IMG_YELLOW;
        else
            i=info->ExtInfoStatus!=CSL_EXT_STATUS_FALSE ? LIST_IMG_GREEN_EXT : LIST_IMG_GREEN;

        if (infoCmp->ImgId!=i)
        {
            SetItemImage(item,i);
            infoCmp->ImgId=i;
        }
    }
    else
    {
        i=LIST_IMG_GAMES_START+info->GetGame().GetId()*2-
          (info->ExtInfoStatus==CSL_EXT_STATUS_OK ? 1:2);
        if (infoCmp->ImgId!=i)
        {
            SetItemImage(item,i);
            infoCmp->ImgId=i;
        }
    }

    *infoCmp=*info;

    return found;
}

void CslListCtrlServer::ListDeleteItem(wxListItem *item)
{
    m_dontRemoveOnDeselect=true;
    DeleteItem(*item);
    m_dontRemoveOnDeselect=false;
}

void CslListCtrlServer::RemoveServer(CslListServerData *server,CslServerInfo *info,wxInt32 selId)
{
    wxInt32 i,l,c;
    wxListItem item;

    if (ListFindItem(info,item)!=wxNOT_FOUND)
        ListDeleteItem(&item);

    if (!server)
    {
        l=m_servers.GetCount();
        for (i=0;i<l;i++)
            if (m_servers.Item(i)->Info==info)
            {
                server=m_servers.Item(i);
                break;
            }
    }

    if (!server)
        return;

    if (selId<0)
    {
        if ((c=m_selected.Index(server))!=wxNOT_FOUND)
            m_selected.RemoveAt(c);
    }
    else
        m_selected.RemoveAt(selId);

    m_servers.Remove(server);
    delete server;
}

wxUint32 CslListCtrlServer::ListUpdate(vector<CslServerInfo*>& servers)
{
    bool sort=false;
    wxUint32 c=0;

    wxWindowUpdateLocker locker(this);

    loopv(servers)
    {
        CslServerInfo *info=servers[i];
        switch (m_id)
        {
            case CSL_LIST_MASTER:
                if (m_masterSelected && !info->IsDefault() && !info->IsUnused())
                    continue;
                break;
            case CSL_LIST_FAVOURITE:
                if (!info->IsFavourite())
                    break;
        }
        sort=true;
        if (ListUpdateServer(info))
            c++;
    }

    if (!sort || !g_cslSettings->autoSortColumns)
        return c;

    ListSort();

#ifndef __WXMSW__
    //removes flicker after sorting
    wxIdleEvent idle;
    wxTheApp->SendIdleEvents(this,idle);
#endif

    return c;
}

void CslListCtrlServer::ListClear()
{
    DeleteAllItems();
    m_selected.Clear();
    WX_CLEAR_ARRAY(m_servers);
    m_filterVersion=-1;
}

wxUint32 CslListCtrlServer::ListSearch(const wxString& search)
{
    m_searchString=search.Lower();

    if (m_searchString.IsEmpty())
        return 0;

    wxUint32 i,c=0;

    for (i=0;i<m_servers.GetCount();i++)
    {
        CslServerInfo *info=m_servers.Item(i)->Info;
        if (ListUpdateServer(info))
            c++;
    }

    return c;
}

wxUint32 CslListCtrlServer::ListFilter()
{
    wxUint32 i,l,c=0;

    wxWindowUpdateLocker locker(this);

    l=m_servers.GetCount();
    for (i=0;i<l;i++)
    {
        CslServerInfo *info=m_servers.Item(i)->Info;
        if (ListUpdateServer(info))
            c++;
    }

    ListSort();

    return c;
}

void CslListCtrlServer::ToggleSortArrow()
{
    wxListItem item;
    wxInt32 img=-1;

    if (g_cslSettings->autoSortColumns)
    {
        if (m_sortHelper.m_sortMode==CSL_SORT_ASC)
            img=LIST_IMG_SORT_ASC;
        else
            img=LIST_IMG_SORT_DSC;
    }
    item.SetImage(img);
    SetColumn(m_sortHelper.m_sortType,item);
}

void CslListCtrlServer::ListSort(wxInt32 column)
{
    wxListItem item;
    wxInt32 img;

    if (column==-1)
        column=m_sortHelper.m_sortType;
    else
    {
        item.SetMask(wxLIST_MASK_IMAGE);
        GetColumn(column,item);

        if (item.GetImage()==-1 ||
            item.GetImage()==LIST_IMG_SORT_DSC ||
            item.GetImage()==LIST_IMG_SORT_DSC_LIGHT)
        {
            g_cslSettings->autoSortColumns ? img=LIST_IMG_SORT_ASC : img=LIST_IMG_SORT_ASC_LIGHT;
            m_sortHelper.m_sortMode=CSL_SORT_ASC;
        }
        else
        {
            g_cslSettings->autoSortColumns ? img=LIST_IMG_SORT_DSC : img=LIST_IMG_SORT_DSC_LIGHT;
            m_sortHelper.m_sortMode=CSL_SORT_DSC;
        }

        item.Clear();
        item.SetImage(-1);
        SetColumn(m_sortHelper.m_sortType,item);

        item.SetImage(img);
        SetColumn(column,item);

        m_sortHelper.m_sortType=column;
    }

    if (!GetItemCount())
        return;

    m_dontUpdateInfo=true;
#ifdef CSL_USE_WX_LIST_DESELECT_WORKAROUND
    CslServerInfo **selected=new CslServerInfo*[GetItemCount()];
    wxInt32 i,j;

    /*for (i=0;i<GetItemCount();i++)
        if (GetItemState(i,wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED)
            selected[c++]=(CslServerInfo*)((CslListServerData*)GetItemData(i))->Info;*/
    for (i=0;i<(wxInt32)m_selected.GetCount();i++)
        selected[i]=m_selected.Item(i)->Info;
#endif

    SortItems(ListSortCompareFunc,(long)&m_sortHelper);

#ifdef CSL_USE_WX_LIST_DESELECT_WORKAROUND
    while (i>=0)
    {
        if ((j=ListFindItem(selected[i--],item))==wxNOT_FOUND)
            continue;
        SetItemState(j,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
    }

    delete[] selected;
#endif
    m_dontUpdateInfo=false;
}

int wxCALLBACK CslListCtrlServer::ListSortCompareFunc(long item1,long item2,long data)
{
    CslListServerData *info1=(CslListServerData*)item1;
    CslListServerData *info2=(CslListServerData*)item2;

    bool ping1Ok=CslEngine::PingOk(*info1->Info,g_cslSettings->updateInterval);
    bool ping2Ok=CslEngine::PingOk(*info2->Info,g_cslSettings->updateInterval);
    bool high1=info1->HighLight&CSL_HIGHLIGHT_FOUND_SERVER || info1->HighLight&CSL_HIGHLIGHT_FOUND_PLAYER;
    bool high2=info2->HighLight&CSL_HIGHLIGHT_FOUND_SERVER || info2->HighLight&CSL_HIGHLIGHT_FOUND_PLAYER;
    wxInt32 sortType=((CslListSortHelper*)data)->m_sortType;

    if ((high1 || high2) && !(high1 && high2))
        return high1 ? -1:1;

    if (sortType!=SORT_HOST)
    {
        if (!ping1Ok && !ping2Ok)
            return 0;
        if (!ping1Ok)
            return 1;
        if (!ping2Ok)
            return -1;
    }

    wxInt32 type;
    wxInt32 sortMode=((CslListSortHelper*)data)->m_sortMode;
    wxInt32 vi1=0,vi2=0;
    wxUint32 vui1=0,vui2=0;
    wxString vs1=wxEmptyString,vs2=wxEmptyString;

    switch (sortType)
    {
        case SORT_HOST:
        {
            type=CSL_LIST_SORT_STRING;
            vs1=info1->Host;
            vs2=info2->Host;
            bool isip1=IsIP(vs1);
            bool isip2=IsIP(vs2);
            if (isip1&&!isip2)
                return sortMode==CSL_SORT_ASC ? -1 : 1;
            else if (!isip1&&isip2)
                return sortMode==CSL_SORT_ASC ? 1 : -1;
            else if (isip1&&isip2)
            {
                type=CSL_LIST_SORT_UINT;
                vui1=IP2Int(vs1);
                vui2=IP2Int(vs2);
            }
            break;
        }

        case SORT_DESCRIPTION:
            type=CSL_LIST_SORT_STRING;
            vs1=info1->Description;
            vs2=info2->Description;
            break;

        case SORT_PING:
            type=CSL_LIST_SORT_UINT;
            vui1=info1->Ping;
            vui2=info2->Ping;
            break;

        case SORT_VER:
            type=CSL_LIST_SORT_INT;
            vi1=info1->Protocol;
            vi2=info2->Protocol;
            break;

        case SORT_MODE:
            type=CSL_LIST_SORT_STRING;
            vs1=info1->GameMode;
            vs2=info2->GameMode;
            break;

        case SORT_MAP:
            type=CSL_LIST_SORT_STRING;
            vs1=info1->Map;
            vs2=info2->Map;
            break;

        case SORT_TIME:
            type=CSL_LIST_SORT_UINT;
            vui1=info1->TimeRemain;
            vui2=info2->TimeRemain;
            break;

        case SORT_PLAYER:
            type=CSL_LIST_SORT_INT;
            vi1=info1->Players;
            vi2=info2->Players;
            break;

        case SORT_MM:
            type=CSL_LIST_SORT_INT;
            vi1=info1->MM;
            vi2=info2->MM;
            break;

        default:
            return 0;
    }

    if (type==CSL_LIST_SORT_INT)
    {
        if (vi1==vi2)
            return 0;
        if (vi1<vi2)
            return sortMode==CSL_SORT_ASC ? -1 : 1;
        else
            return sortMode==CSL_SORT_ASC ? 1 : -1;
    }
    else if (type==CSL_LIST_SORT_UINT)
    {
        if (vui1==vui2)
            return 0;
        if (vui1<vui2)
            return sortMode==CSL_SORT_ASC ? -1 : 1;
        else
            return sortMode==CSL_SORT_ASC ? 1 : -1;
    }
    else if (type==CSL_LIST_SORT_STRING)
    {
        if (vs1==vs2)
            return 0;
        if (vs1.CmpNoCase(vs2)<0)
            return sortMode==CSL_SORT_ASC ? -1 : 1;
        else
            return sortMode==CSL_SORT_ASC ? 1 : -1;
    }

    return 0;
}
