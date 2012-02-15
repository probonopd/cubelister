/***************************************************************************
 *   Copyright (C) 2007-2011 by Glen Masgai                                *
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

#include "Csl.h"
#include "CslSettings.h"
#include "CslApp.h"
#include "CslFrame.h"

CslListCtrlSettings* CslSettings::GetListSettings(const wxString& name)
{
    CslSettings& settings=GetInstance();

    loopv(settings.CslListSettings)
    {
        if (settings.CslListSettings[i]->Name==name)
            return settings.CslListSettings[i];
    }

    return settings.CslListSettings.add(new CslListCtrlSettings(name));
}

void CslSettings::LoadSettings()
{
    CslSettings& settings=GetInstance();
    CslEngine *engine=::wxGetApp().GetCslEngine();

    if (!::wxFileExists(CSL_SETTINGS_FILE))
        return;

    long int val;
#if 0 // enable later if there is any custom column size support
    double dval;
#endif
    wxString s;

    wxFileConfig config(wxT(""), wxT(""), CSL_SETTINGS_FILE, wxT(""),
                        wxCONFIG_USE_LOCAL_FILE, wxConvLocal);

    config.SetPath(wxT("/Version"));
    config.Read(wxT("Version"), &val, 0);

    config.SetPath(wxT("/Gui"));
    if (config.Read(wxT("SizeX"), &val)) settings.FrameSize.SetWidth(val);
    if (config.Read(wxT("SizeY"), &val)) settings.FrameSize.SetHeight(val);
    if (config.Read(wxT("SizeMaxX"), &val)) settings.FrameSizeMax.SetWidth(val);
    if (config.Read(wxT("SizeMaxY"), &val)) settings.FrameSizeMax.SetHeight(val);
    if (config.Read(wxT("Layout"), &s)) settings.Layout=s;
    if (config.Read(wxT("Systray"), &val)) settings.Systray=val;
    if (config.Read(wxT("TTS"), &val)) settings.TTS=val!=0;
    if (config.Read(wxT("TTSVolume"), &val))
    {
        if (val>0 && val<=100)
            settings.TTSVolume=val;
    }
    if (config.Read(wxT("UpdateInterval"), &val))
    {
        if (val>=CSL_UPDATE_INTERVAL_MIN && val<=CSL_UPDATE_INTERVAL_MAX)
            settings.UpdateInterval=val;
    }
    if (config.Read(wxT("NoUpdatePlaying"), &val)) settings.DontUpdatePlaying=val!=0;
    if (config.Read(wxT("ShowSearch"), &val)) settings.ShowSearch=val!=0;
    if (config.Read(wxT("SearchLAN"), &val)) settings.SearchLAN=val;
    if (config.Read(wxT("FilterMaster"), &val)) settings.FilterMaster=val;
    if (config.Read(wxT("FilterFavourites"), &val)) settings.FilterFavourites=val;
    if (config.Read(wxT("WaitOnFullServer"), &val))
    {
        if (val>=CSL_WAIT_SERVER_FULL_MIN && val<=CSL_WAIT_SERVER_FULL_MAX)
            settings.WaitServerFull=val;
    }
    if (config.Read(wxT("CleanupServers"), &val))
    {
        if ((!val || (val && val>=86400)) && val<=CSL_CLEANUP_SERVERS_MAX*86400)
            settings.CleanupServers=val;
    }
    if (config.Read(wxT("CleanupServersKeepFavourites"), &val))
        settings.CleanupServersKeepFav=val!=0;
    if (config.Read(wxT("CleanupServersKeepStats"), &val))
        settings.CleanupServersKeepStats=val!=0;
    if (config.Read(wxT("TooltipDelay"), &val))
    {
        if (val>=CSL_TOOLTIP_DELAY_MIN && val<=CSL_TOOLTIP_DELAY_MAX)
            settings.TooltipDelay=val/CSL_TOOLTIP_DELAY_STEP*CSL_TOOLTIP_DELAY_STEP;
    }
    if (config.Read(wxT("PingGood"), &val))
    {
        if (val<=9999)
            settings.PingGood=val;
    }
    if (config.Read(wxT("PingBad"), &val))
    {
        if (val<settings.PingGood)
            settings.PingBad=settings.PingGood;
        else
            settings.PingBad=val;
    }
    if (config.Read(wxT("GameOutputPath"), &s)) settings.GameOutputPath=s;
    if (settings.GameOutputPath.IsEmpty())
        settings.GameOutputPath=wxStandardPaths().GetUserDataDir();
    if (config.Read(wxT("AutoSaveOutput"), &val)) settings.AutoSaveOutput=val!=0;
    if (config.Read(wxT("LastSelectedGame"), &s)) settings.LastGame=s;
    if (config.Read(wxT("MinPlaytime"), &val))
    {
        if (val>=CSL_MIN_PLAYTIME_MIN && val<=CSL_MIN_PLAYTIME_MAX)
            settings.MinPlaytime=val;
    }

    /* ListCtrl */
    config.SetPath(wxT("/List"));
    if (config.Read(wxT("AutoSort"), &val)) settings.AutoSortColumns=val!=0;
    if (config.Read(wxT("ColourEmpty"), &val)) settings.ColServerEmpty=INT2COLOUR(val);
    if (config.Read(wxT("ColourOffline"), &val)) settings.ColServerOff=INT2COLOUR(val);
    if (config.Read(wxT("ColourFull"), &val)) settings.ColServerFull=INT2COLOUR(val);
    if (config.Read(wxT("ColourMM1"), &val)) settings.ColServerMM1=INT2COLOUR(val);
    if (config.Read(wxT("ColourMM2"), &val)) settings.ColServerMM2=INT2COLOUR(val);
    if (config.Read(wxT("ColourMM3"), &val)) settings.ColServerMM3=INT2COLOUR(val);
    if (config.Read(wxT("ColourSearch"), &val)) settings.ColServerHigh=INT2COLOUR(val);
    if (config.Read(wxT("ColourPlaying"), &val)) settings.ColServerPlay=INT2COLOUR(val);
    if (config.Read(wxT("ColourStripes"), &val)) settings.ColInfoStripe=INT2COLOUR(val);

    wxInt32 i, pos;
    wxString name, value;

    for (i=0; config.Read(wxString::Format(wxT("ColumnsMask%d"), i), &name); i++)
    {
        unsigned long int columnmask=0;

        if ((pos=name.Find(wxT(':')))==wxNOT_FOUND)
            continue;
        if (!name.Mid(0, pos).ToULong(&columnmask))
            continue;
        if ((name=name.Mid(pos+1)).IsEmpty())
            continue;

        CslListCtrlSettings *ls=GetListSettings(name);
        ls->ColumnMask=columnmask;
    }

    /* Client */
    CslGames& games=engine->GetGames();
    loopv(games)
    {
#ifdef __WXMAC__
        val=true;
#else
        val=false;
#endif //__WXMAC__
        CslGameClientSettings& settings=games[i]->GetClientSettings();

        config.SetPath(wxT("/")+games[i]->GetName());
        if (config.Read(wxT("Binary"), &s)) settings.Binary=s;
        if (config.Read(wxT("GamePath"), &s)) settings.GamePath=s;
        if (config.Read(wxT("ConfigPath"), &s))
        {
            if (s.IsEmpty() && !settings.GamePath.IsEmpty())
                settings.ConfigPath=settings.GamePath;
            else if (!s.IsEmpty() && ::wxDirExists(s))
                settings.ConfigPath=s;
        }
        if (config.Read(wxT("Options"), &s)) settings.Options=s;
        if (config.Read(wxT("PreScript"), &s)) settings.PreScript=s;
        if (config.Read(wxT("PostScript"), &s)) settings.PostScript=s;
        if (!settings.Binary.IsEmpty() && !::wxFileExists(settings.Binary))
            settings.Binary=wxEmptyString;
        if (!settings.GamePath.IsEmpty() && !::wxDirExists(settings.GamePath))
            settings.GamePath=wxEmptyString;
    }
}

void CslSettings::SaveSettings()
{
    wxString dir=::wxPathOnly(CSL_SETTINGS_FILE);

    CslSettings& settings=GetInstance();
    CslEngine *engine=::wxGetApp().GetCslEngine();
    wxAuiManager& AuiMgr=((CslFrame*)wxGetApp().GetTopWindow())->GetAuiManager();

    if (!::wxDirExists(dir))
        ::wxMkdir(dir, 0700);

    wxFileConfig config(wxT(""), wxT(""), CSL_SETTINGS_FILE, wxT(""),
                        wxCONFIG_USE_LOCAL_FILE, wxConvLocal);
    config.SetUmask(0077);
    config.DeleteAll();

    config.SetPath(wxT("/Version"));
    config.Write(wxT("Version"), CSL_CONFIG_VERSION);

    /* GUI */
    wxSize size=settings.FrameSize;
    config.SetPath(wxT("/Gui"));
    config.Write(wxT("SizeX"), (long int)size.GetWidth());
    config.Write(wxT("SizeY"), (long int)size.GetHeight());
    size=settings.FrameSizeMax;
    config.Write(wxT("SizeMaxX"), (long int)size.GetWidth());
    config.Write(wxT("SizeMaxY"), (long int)size.GetHeight());
    config.Write(wxT("Layout"), AuiMgr.SavePerspective());
    config.Write(wxT("Systray"), settings.Systray);
    config.Write(wxT("TTS"), settings.TTS);
    config.Write(wxT("TTSVolume"), settings.TTSVolume);
    config.Write(wxT("UpdateInterval"), (long int)settings.UpdateInterval);
    config.Write(wxT("NoUpdatePlaying"), settings.DontUpdatePlaying);
    config.Write(wxT("ShowSearch"), settings.ShowSearch);
    config.Write(wxT("SearchLAN"), settings.SearchLAN);
    config.Write(wxT("FilterMaster"), (long int)settings.FilterMaster);
    config.Write(wxT("FilterFavourites"), (long int)settings.FilterFavourites);
    config.Write(wxT("WaitOnFullServer"), (long int)settings.WaitServerFull);
    config.Write(wxT("CleanupServers"), (long int)settings.CleanupServers);
    config.Write(wxT("CleanupServersKeepFavourites"), (long int)settings.CleanupServersKeepFav);
    config.Write(wxT("CleanupServersKeepStats"), (long int)settings.CleanupServersKeepStats);
    config.Write(wxT("TooltipDelay"), (long int)settings.TooltipDelay);
    config.Write(wxT("PingGood"), (long int)settings.PingGood);
    config.Write(wxT("PingBad"), (long int)settings.PingBad);
    config.Write(wxT("GameOutputPath"), settings.GameOutputPath);
    config.Write(wxT("AutoSaveOutput"), settings.AutoSaveOutput);
    config.Write(wxT("LastSelectedGame"), settings.LastGame);
    config.Write(wxT("MinPlaytime"), (long int)settings.MinPlaytime);

    /* ListCtrl */
    config.SetPath(wxT("/List"));
    config.Write(wxT("ColourEmpty"), COLOUR2INT(settings.ColServerEmpty));
    config.Write(wxT("ColourOffline"), COLOUR2INT(settings.ColServerOff));
    config.Write(wxT("ColourFull"), COLOUR2INT(settings.ColServerFull));
    config.Write(wxT("ColourMM1"), COLOUR2INT(settings.ColServerMM1));
    config.Write(wxT("ColourMM2"), COLOUR2INT(settings.ColServerMM2));
    config.Write(wxT("ColourMM3"), COLOUR2INT(settings.ColServerMM3));
    config.Write(wxT("ColourSearch"), COLOUR2INT(settings.ColServerHigh));
    config.Write(wxT("ColourPlaying"), COLOUR2INT(settings.ColServerPlay));
    config.Write(wxT("ColourStripes"), COLOUR2INT(settings.ColInfoStripe));

    wxInt32 id=0;
    wxString n, v;
    vector<wxWindow*> lists;
    GetChildWindowsByClassInfo(NULL, CLASSINFO(CslListCtrl), lists);

    loopv(lists)
    {
        CslListCtrl *list=(CslListCtrl*)lists[i];
        const wxString& name=list->GetName();

        if (!name.empty())
        {
            wxUint32 m=list->ListGetColumnMask();

            if (m)
            {
                n=wxString::Format(wxT("ColumnsMask%d"), id++);
                v=wxString::Format(wxT("%u:"), m)+name;
                config.Write(n, v);
            }
        }
    }

    /* Client */
    CslGames& games=engine->GetGames();
    loopv(games)
    {
        CslGame& game=*games[i];
        const CslGameClientSettings& settings=game.GetClientSettings();

        config.SetPath(wxT("/")+game.GetName());
        config.Write(wxT("Binary"), settings.Binary);
        config.Write(wxT("GamePath"), settings.GamePath);
        config.Write(wxT("ConfigPath"), settings.ConfigPath);
        config.Write(wxT("Options"), settings.Options);
        config.Write(wxT("PreScript"), settings.PreScript);
        config.Write(wxT("PostScript"), settings.PostScript);
        config.Write(wxT("Options"), settings.Options);
    }
}


class CslIDMapping
{
    public:
        CslIDMapping(wxInt32 oldId, wxInt32 newId) :
                m_oldId(oldId), m_newId(newId) { }

        wxInt32 m_oldId, m_newId;
};

bool CslSettings::LoadServers(wxUint32 *numm, wxUint32 *nums)
{
    if (!::wxFileExists(CSL_SERVERS_FILE))
        return false;

    CslEngine *engine=::wxGetApp().GetCslEngine();
    CslFrame *frame=(CslFrame*)::wxGetApp().GetTopWindow();

    long int val;
    bool read_server;
    wxUint32 mc=0, sc=0, tmc=0, tsc=0;
    CslMaster *master;
    vector<CslIDMapping*> mappings;
    vector<wxInt32> ids;

    wxString addr, path, pass1, pass2, description;
    wxUint16 port=0, iport=0;
    wxUint32 view=0;
    wxUint32 events=0;
    wxUint32 lastSeen=0;
    wxUint32 playLast=0;
    wxUint32 playTimeLastGame=0;
    wxUint32 playTimeTotal=0;
    wxUint32 connectedTimes=0;
    wxInt32 gt;

    wxFileConfig config(wxT(""), wxT(""), CSL_SERVERS_FILE, wxT(""),
                        wxCONFIG_USE_LOCAL_FILE, wxConvLocal);

    CslGames& games=engine->GetGames();

    for (gt=0; gt<games.length(); gt++)
    {
        wxString s=wxT("/")+games[gt]->GetName();
        config.SetPath(s);

        mc=0;
        while (1)
        {
            long int id;
            CslMasterConnection connection;
            master=NULL;

            config.SetPath(s+wxString::Format(wxT("/Master/%d"), mc++));

            if (config.Read(wxT("Address"), &addr))
            {
                if (config.Read(wxT("ID"), &id))
                {
                    val=0;
                    config.Read(wxT("Port"), &val, 0); port=val;
                    config.Read(wxT("Type"), &val, CslMasterConnection::CONNECTION_HTTP);
                    if (val==CslMasterConnection::CONNECTION_HTTP)
                    {
                        port=port ? port:CSL_DEFAULT_MASTER_WEB_PORT;
                        if (config.Read(wxT("Path"), &path))
                            master=new CslMaster(CslMasterConnection(addr, path, port));
                    }
                    else if (port)
                        master=new CslMaster(CslMasterConnection(addr, port));
                }
            }
            if (!master)
                break;

            if (games[gt]->AddMaster(master)<0)
            {
                delete master;
                continue;
            }
            mappings.add(new CslIDMapping(id, master->GetId()));
            tmc++;
        }

        sc=0;
        read_server=true;
        while (read_server)
        {
            wxInt32 id=0;
            ids.setsize(0);

            config.SetPath(s+wxString::Format(wxT("/Server/%d"), sc++));

            read_server=false;
            if (config.Read(wxT("Address"), &addr))
            {
                config.Read(wxT("Port"), &val, games[gt]->GetDefaultGamePort()); port=val;
                // use 0 as default infoport to import older servers.ini and
                // set the appropriate infoport when creating the CslServerInfo
                config.Read(wxT("InfoPort"), &val, 0); iport=val;
                config.Read(wxT("Password"), &pass1, wxEmptyString);
                config.Read(wxT("AdminPassword"), &pass2, wxEmptyString);
                config.Read(wxT("Description"), &description, wxEmptyString);

                while (config.Read(wxString::Format(wxT("Master%d"), id++), &val))
                    ids.add(val);
                if (ids.length()==0)
                    ids.add(-1);

                if (config.Read(wxT("View"), &val))
                {
                    view=val;
                    config.Read(wxT("Events"), &val, 0);
                    events=(wxUint32)val;
                    config.Read(wxT("LastSeen"), &val, 0);
                    lastSeen=(wxUint32)val;
                    config.Read(wxT("PlayLast"), &val, 0);
                    playLast=(wxUint32)val;
                    config.Read(wxT("PlayTimeLastGame"), &val, 0);
                    playTimeLastGame=(wxUint32)val;
                    config.Read(wxT("PlayTimeTotal"), &val, 0);
                    playTimeTotal=(wxUint32)val;
                    config.Read(wxT("ConnectedTimes"), &val, 0);
                    connectedTimes=(wxUint32)val;
                    read_server=true;
                }
            }

            if (!read_server)
                break;

            loopv(ids)
            {
                CslServerInfo *info=new CslServerInfo(games[gt], addr, port, iport ? iport:port+1,
                                                      view, lastSeen, playLast, playTimeLastGame,
                                                      playTimeTotal, connectedTimes,
                                                      description, pass1, pass2);

                if (ids[i]==-1)
                    id=-1;
                else
                {
                    loopvj(mappings)
                    {
                        if (mappings[j]->m_oldId==ids[i])
                        {
                            id=mappings[j]->m_newId;
                            break;
                        }
                    }
                }

                if (games[gt]->AddServer(info, id))
                {
                    long type;
                    wxInt32 pos, j=0;
                    wxString guiview;

                    engine->ResolveHost(info);
                    info->RegisterEvents(events);

                    while (config.Read(wxString::Format(wxT("GuiView%d"), j++), &guiview))
                    {
                        if ((pos=guiview.Find(wxT(':')))==wxNOT_FOUND)
                            continue;
                        if (!guiview.Mid(0, pos).ToLong(&type))
                            continue;
                        if ((guiview=guiview.Mid(pos+1)).IsEmpty())
                            continue;
                        frame->PlayerListCreateView(info, type, guiview);
                    }

                    tsc++;
                }
                else
                    delete info;
            }
        }
        //engine->ResetPingSends(games[gt], NULL);
    }

    loopv(mappings)
    delete mappings[i];

    if (numm)
        *nums=tmc;
    if (nums)
        *nums=tsc;

    return tmc>0||tsc>0;
}

void CslSettings::SaveServers()
{
    wxString s=::wxPathOnly(CSL_SERVERS_FILE);

    if (!::wxDirExists(s))
        if (!::wxMkdir(s, 0700))
            return;

    CslEngine *engine=::wxGetApp().GetCslEngine();

    wxFileConfig config(wxT(""), wxT(""), CSL_SERVERS_FILE, wxT(""),
                        wxCONFIG_USE_LOCAL_FILE, wxConvLocal);
    config.SetUmask(0077);
    config.DeleteAll();

    wxInt32 mc, sc;
    CslGame *game;
    CslMaster *master;
    CslServerInfo *info;

    CslGames& games=engine->GetGames();

    if (!games.length())
        return;

    config.SetPath(wxT("/Version"));
    config.Write(wxT("Version"), CSL_SERVERCONFIG_VERSION);

    loopv(games)
    {
        game=games[i];
        s=wxT("/")+game->GetName();
        config.SetPath(s);

        CslMasters& masters=game->GetMasters();
        mc=0;

        loopvj(masters)
        {
            master=masters[j];
            CslMasterConnection& connection=master->GetConnection();

            config.SetPath(s+s.Format(wxT("/Master/%d"), mc++));
            config.Write(wxT("Type"), connection.GetType());
            config.Write(wxT("Address"), connection.GetAddress());
            config.Write(wxT("Port"), connection.GetPort());
            config.Write(wxT("Path"), connection.GetPath());
            config.Write(wxT("ID"), master->GetId());
        }

        CslServerInfos& servers=game->GetServers();
        if (servers.length()==0)
            continue;

        sc=0;
        loopvj(servers)
        {
            info=servers[j];
            config.SetPath(s+s.Format(wxT("/Server/%d"), sc++));
            config.Write(wxT("Address"), info->Host);
            config.Write(wxT("Port"), info->GamePort);
            config.Write(wxT("InfoPort"), info->InfoPort);
            config.Write(wxT("Password"), info->Password);
            config.Write(wxT("AdminPassword"), info->PasswordAdmin);
            config.Write(wxT("Description"), info->DescriptionOld);
            const vector<wxInt32> masters=info->GetMasterIDs();
            loopvk(masters) config.Write(wxString::Format(wxT("Master%d"), k), masters[k]);
            config.Write(wxT("View"), (int)info->View);
            config.Write(wxT("Events"), (int)info->GetRegisteredEvents());
            config.Write(wxT("LastSeen"), (int)info->LastSeen);
            config.Write(wxT("PlayLast"), (int)info->PlayedLast);
            config.Write(wxT("PlayTimeLastGame"), (int)info->PlayTimeLast);
            config.Write(wxT("PlayTimeTotal"), (int)info->PlayTimeTotal);
            config.Write(wxT("ConnectedTimes"), (int)info->ConnectedTimes);

            wxUint32 l=0;
            wxAuiManager& AuiMgr=((CslFrame*)wxGetApp().GetTopWindow())->GetAuiManager();
            vector<CslPanelPlayer*>& playerInfos=((CslFrame*)wxGetApp().GetTopWindow())->GetPlayerViews();

            loopvk(playerInfos)
            {
                if (k==0)
                    continue;
                CslPanelPlayer *list=playerInfos[k];
                if (list->ServerInfo()==info)
                {
                    wxAuiPaneInfo& pane=AuiMgr.GetPane(list);
                    if (!pane.IsOk())
                        continue;

                    wxString view=wxString::Format(wxT("%d:"), list->ListCtrl()->View())+pane.name;
                    config.Write(wxString::Format(wxT("GuiView%d"), l++), view);
                }
            }
        }
    }
}