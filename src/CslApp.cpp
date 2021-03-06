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
#include "Csl.h"
#include "CslEngine.h"
#include "CslFrame.h"
#include "CslIPC.h"
#include "CslApp.h"
#include <wx/stdpaths.h>
#ifdef __WXMAC__
#include <wx/sysopt.h>
#include <Carbon/Carbon.h>
#endif //__WXMAC__
#if wxUSE_DEBUGREPORT && wxUSE_ON_FATAL_EXCEPTION
#include <wx/debugrpt.h>
#endif

IMPLEMENT_APP(CslApp)

BEGIN_EVENT_TABLE(CslApp,wxApp)
    EVT_END_SESSION(CslApp::OnEndSession)
END_EVENT_TABLE()

#ifdef __WXMAC__
static pascal OSErr MacCallbackGetUrl(const AppleEvent *in,AppleEvent *out,long ptr)
{
    Size l=0;
    OSErr err=noErr;
    DescType type=typeChar;

    if ((err=AESizeOfParam(in,keyDirectObject,&type,&l))==noErr && l)
    {
        char buf[l+1];

        if ((err=AEGetParamPtr(in,keyDirectObject,type,0,&buf,l,&l))==noErr && l)
        {
            buf[l]=0;
            const CslApp& app=::wxGetApp();
            app.IpcCall(C2U(buf),app.GetTopWindow());
        }
    }

    return noErr;
}
#endif //__WXMAC__


wxString CslApp::GetHomeDir(wxPathFormat format) const
{
    if (format==wxPATH_NATIVE)
        return m_home;

    wxFileName fn(m_home);

    return fn.GetPathWithSep(format);
}

bool CslApp::OnInit()
{
    SetAppName(wxString(CSL_NAME_SHORT_STR).Lower());

#if defined(__WXMSW__)
    wxCrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

#if CSL_DEBUG < 1
#if wxUSE_DEBUGREPORT && wxUSE_ON_FATAL_EXCEPTION
    ::wxHandleFatalExceptions(true);
#endif
#endif

    wxString ipcCmd;
    const wxString cwd = ::DirName(::wxPathOnly(argv[0]));

    static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        { wxCMD_LINE_SWITCH, NULL,       wxT_2("version"),   wxTRANSLATE("show version"),   wxCMD_LINE_VAL_NONE,   0                         },
        { wxCMD_LINE_SWITCH, wxT_2("h"), wxT_2("help"),      wxTRANSLATE("show this help"), wxCMD_LINE_VAL_NONE,   wxCMD_LINE_OPTION_HELP    },
        { wxCMD_LINE_OPTION, wxT_2("d"), wxT_2("datadir"),   wxTRANSLATE("add data dir"),   wxCMD_LINE_VAL_STRING, 0                         },
        { wxCMD_LINE_OPTION, wxT_2("p"), wxT_2("plugindir"), wxTRANSLATE("add plugin dir"), wxCMD_LINE_VAL_STRING, 0                         },
        { wxCMD_LINE_OPTION, wxT_2("q"), wxT_2("homedir"),   wxTRANSLATE("set home dir"),   wxCMD_LINE_VAL_STRING, 0                         },
        { wxCMD_LINE_PARAM,  NULL,       NULL,               wxTRANSLATE("IPC command"),    wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
        { wxCMD_LINE_NONE                                                                                                                    }
    };

    wxString sOpt;
    wxCmdLineParser parser(cmdLineDesc, argc, argv);

    switch (parser.Parse())
    {
        case -1: // help
            return true;

        case 0:
            if (parser.Found(wxT("version")))
            {
                wxPrintf(wxT("%s\n"), CSL_VERSION_STR);
                return true;
            }
            if (parser.Found(wxT("q"), &sOpt))
                m_home = ::DirName(sOpt, cwd);
            if (parser.Found(wxT("d"), &sOpt))
                AddDataDir(sOpt, cwd);
            if (parser.Found(wxT("p"), &sOpt))
                AddPluginDir(sOpt, cwd);
            break;

        default: // syntax error
            return true;
    }

    if (parser.GetParamCount())
        ipcCmd = parser.GetParam(0);

    if (m_home.IsEmpty())
        m_home = ::DirName(wxStandardPaths::Get().GetUserDataDir());

    if (!wxFileName::DirExists(m_home))
    {
        if (!wxFileName::Mkdir(m_home, 0700, wxPATH_MKDIR_FULL))
        {
            wxMessageBox(wxString::Format(_("Failed to create the home dir '%s'."),
                                          m_home.c_str()),
                         _("Fatal error!"), wxICON_ERROR);
            return false;
        }
    }
    else if (!wxFileName::IsDirWritable(m_home))
    {
        wxMessageBox(wxString::Format(_("Home dir '%s' isn't writable."),
                                      m_home.c_str()),
                     _("Fatal error!"), wxICON_ERROR);
        return false;
    }

    CSL_LOG_DEBUG("using home dir: %s\n", U2C(m_home));

    AddDataDir(m_home, cwd);
#ifdef PROJECTDIR
    if (::wxDirExists(wxT(PROJECTDIR)))
        AddDataDir(wxT(PROJECTDIR), cwd);
#endif
#ifdef __WXMAC__
    AddDataDir(wxT("../Resources"), cwd);
#else
    AddDataDir(cwd);
#endif
#ifdef DATADIR
    if (cwd!=m_home)
        AddDataDir(wxT(DATADIR), cwd);
#endif

    AddPluginDir(m_home, cwd);
#ifdef __WXMAC__
    AddPluginDir(wxT(".."), cwd);
#else
    AddPluginDir(cwd);
#endif
#ifdef BUILDDIR
    if (::wxDirExists(wxT(BUILDDIR)))
        AddPluginDir(wxT(BUILDDIR), cwd);
#endif
#ifdef PKGLIBDIR
    if (cwd!=m_home)
        AddPluginDir(wxT(PKGLIBDIR));
#endif

    m_locale.Init(wxLANGUAGE_DEFAULT,
#if wxCHECK_VERSION(2, 9, 0)
                  wxLOCALE_LOAD_DEFAULT
#else
                  wxLOCALE_CONV_ENCODING
#endif //wxCHECK_VERSION
                 );

#ifndef LOCALEDIR
#ifdef __WXMSW__
    m_locale.AddCatalogLookupPathPrefix(cwd+wxT("\\locale"));
#else
#ifdef __WXMAC__
    m_locale.AddCatalogLookupPathPrefix(cwd+wxT("../Resources"));
#endif //__WXMAC__
#endif //__WXMSW__
#else
    m_locale.AddCatalogLookupPathPrefix(wxT(LOCALEDIR));
#endif //LOCALEDIR

    if (m_locale.AddCatalog(wxString(CSL_NAME_SHORT_STR).Lower()))
        m_lang=m_locale.GetCanonicalName();

#ifdef __WXMAC__
    wxSystemOptions::SetOption(wxT("mac.listctrl.always_use_generic"), 1);

    // enables Command-H, Command-M and Command-Q at least when not in fullscreen
    wxSetEnv(wxT("SDL_SINGLEDISPLAY"),wxT("1"));
    wxSetEnv(wxT("SDL_ENABLEAPPEVENTS"),wxT("1"));

    // TODO wxApp::SetExitOnFrameDelete(false);

    // register event handler for URI schemes
    AEInstallEventHandler(kInternetEventClass, kAEGetURL,
                          NewAEEventHandlerUPP((AEEventHandlerProcPtr)MacCallbackGetUrl),
                          0, false);
#endif //__WXMAC__

    m_single = new wxSingleInstanceChecker;

    wxString lock = wxString::Format(wxT("%s-%s.%s"), CSL_NAME_SHORT_STR, wxGetUserId().c_str(),
#ifdef __WXMSW__
                                     GetHomeDir(wxPATH_UNIX).c_str()
#else
                                     wxT("lock")
#endif
                                    ).Lower();

    if (m_single->Create(lock, m_home) && m_single->IsAnotherRunning())
    {
        IpcCall(ipcCmd.IsEmpty() ? wxT("show") : ipcCmd);
        return true;
    }

    m_engine = new CslEngine;

    wxInitAllImageHandlers();

    CslFrame* frame = new CslFrame;

    if (m_shutdown!=CSL_SHUTDOWN_NONE)
        return true;

    SetTopWindow(frame);
    frame->Show();

    if (!ipcCmd.IsEmpty())
        IpcCall(ipcCmd, frame);

    return true;
}

int CslApp::OnRun()
{
    if (GetTopWindow())
        wxApp::OnRun();

    return m_shutdown>CSL_SHUTDOWN_NORMAL ? 1 : 0;
}

int CslApp::OnExit()
{
    wxDELETE(m_engine);
    wxDELETE(m_single);

    return 0;
}

#if wxUSE_DEBUGREPORT && wxUSE_ON_FATAL_EXCEPTION
void CslApp::OnFatalException()
{
    wxDebugReport report;
    wxDebugReportPreviewStd preview;

    report.AddAll();

    if (preview.Show(report))
        report.Process();
}
#endif

void CslApp::OnEndSession(wxCloseEvent& event)
{
    CSL_LOG_DEBUG("\n");

    m_shutdown=CSL_SHUTDOWN_FORCE;

    event.Skip();
}

int CslApp::FilterEvent(wxEvent& event)
{
    if (event.GetEventType()==wxEVT_KEY_DOWN)
        CslToolTip::Reset();

    return -1;
}

void CslApp::IpcCall(const wxString& value, wxEvtHandler *evtHandler) const
{
    if (evtHandler)
    {
        CslIpcEvent evt(CslIpcEvent::IPC_COMMAND, value);

        ::wxPostEvent(evtHandler, evt);
    }
    else
    {
        CslIpcClient client;

        if (!client.Connect(CSL_IPC_HOST, CSL_IPC_SERV, CSL_IPC_TOPIC))
            return;

        // always convert to char* to gain compatibility
        // with wx 2.8 / 2.9 and ansi / unicode

        client.GetConnection()->Poke(CSL_NAME_SHORT_STR, (wxChar*)U2C(value),
                                     value.Len(), wxIPC_PRIVATE);
    }
}
