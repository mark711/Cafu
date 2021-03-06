/*
=================================================================================
This file is part of Cafu, the open-source game engine and graphics engine
for multiplayer, cross-platform, real-time 3D action.
Copyright (C) 2002-2012 Carsten Fuchs Software.

Cafu is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

Cafu is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Cafu. If not, see <http://www.gnu.org/licenses/>.

For support and more information about Cafu, visit us at <http://www.cafu.de>.
=================================================================================
*/

#include "wx/wx.h"
#include "wx/confbase.h"
#include "wx/dir.h"
#include "wx/fileconf.h"
#include "wx/filename.h"
#include "wx/image.h"
#include "wx/snglinst.h"
#include "wx/splash.h"
#include "wx/stdpaths.h"

#include "AppCaWE.hpp"
#include "ArtProvider.hpp"
#include "ChildFrameViewWin2D.hpp"
#include "ChildFrameViewWin3D.hpp"
#include "CursorMan.hpp"
#include "GameConfig.hpp"
#include "ParentFrame.hpp"
#include "MapDocument.hpp"
#include "MapElement.hpp"
#include "Options.hpp"
#include "EditorMaterialManager.hpp"
#include "ToolbarMaterials.hpp"
#include "Tool.hpp"
#include "ToolManager.hpp"

#include "ClipSys/CollisionModelMan_impl.hpp"
#include "ConsoleCommands/Console.hpp"
#include "ConsoleCommands/ConsoleStdout.hpp"
#include "ConsoleCommands/ConsoleInterpreter.hpp"
#include "ConsoleCommands/ConsoleInterpreterImpl.hpp"
#include "ConsoleCommands/ConVar.hpp"
#include "ConsoleCommands/ConFunc.hpp"
#include "FileSys/FileManImpl.hpp"
#include "GuiSys/Window.hpp"
#include "MaterialSystem/MaterialManagerImpl.hpp"
#include "TypeSys.hpp"

#include <fstream>


static cf::ConsoleStdoutT ConsoleStdout;
cf::ConsoleI* Console=&ConsoleStdout;

//static cf::ConsoleFileT ConsoleFile("info.log");
//cf::ConsoleI* Console=&ConsoleFile;

static cf::FileSys::FileManImplT FileManImpl;
cf::FileSys::FileManI* cf::FileSys::FileMan=&FileManImpl;

static cf::ClipSys::CollModelManImplT CCM;
cf::ClipSys::CollModelManI* cf::ClipSys::CollModelMan=&CCM;

ConsoleInterpreterI* ConsoleInterpreter=NULL;
MaterialManagerI*    MaterialManager   =NULL;


OptionsT Options;


IMPLEMENT_APP(AppCaWE)


AppCaWE::AppCaWE()
    : wxApp(),
      m_Locale(NULL),
      m_CmdLineParser(),
      m_FileConfig(NULL),
      m_ParentFrame(NULL)
{
    static ConsoleInterpreterImplT ConInterpreterImpl;
    ConsoleInterpreter=&ConInterpreterImpl;

    // All global convars and confuncs have registered themselves in linked lists.
    // Register them with the console interpreter now.
    ConFuncT::RegisterStaticList();
    ConVarT ::RegisterStaticList();

    SetAppName("CaWE");
    SetVendorName("Carsten Fuchs Software");
    wxStandardPaths::Get().UseAppInfo(wxStandardPaths::AppInfo_VendorName | wxStandardPaths::AppInfo_AppName);
}


bool AppCaWE::OnInit()
{
    cf::GuiSys::GetWindowTIM().Init();  // The one-time init of the Window type info manager.
    GetMapElemTIM().Init();             // The one-time init of the map elements type info manager.
    GetToolTIM().Init();                // The one-time init of the tools type info manager.

    // Parse the command line.
    // Note that this replaces (and in fact conflicts with) wxApp::OnInit().
    m_CmdLineParser.SetCmdLine(argc, argv);
    OnInitCmdLine(m_CmdLineParser);
    m_CmdLineParser.AddParam("filename", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);

    if (m_CmdLineParser.Parse() != 0)
    {
        OnExit();
        return false;
    }

    OnCmdLineParsed(m_CmdLineParser);   // Just for setting wxLog to verbose when "--verbose" is given.

    #ifndef NDEBUG
    {
        std::ofstream OutFile("Doxygen/scripting/GuiWindows.tmpl");

        if (OutFile.is_open())
            cf::GuiSys::GetWindowTIM().CreateLuaDoxygenHeader(OutFile);
    }
    #endif

    const std::string AppDir="./CaWE";

    // This is for registering the CaWE.cmat file farther below.
    cf::FileSys::FileMan->MountFileSystem(cf::FileSys::FS_TYPE_LOCAL_PATH, AppDir+"/res/", AppDir+"/res/");
    cf::FileSys::FileMan->MountFileSystem(cf::FileSys::FS_TYPE_LOCAL_PATH, "./", "");


    for (unsigned int DirNr=0; DirNr<2; DirNr++)
    {
        wxString TestDirs[]={ "CaWE", "Games" };

        if (!wxDirExists(TestDirs[DirNr]))
        {
            wxMessageDialog Msg(NULL, "Could not find directory \"" + TestDirs[DirNr] + "\" in " + wxGetCwd(),
                "Subdirectory not found", wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxICON_ERROR);

            Msg.SetExtendedMessage("Did you run the program from the right working directory?\n\nTo get help, please post this error at the Cafu forums or mailing-list.");
            Msg.SetOKCancelLabels("Open www.cafu.de", "Close");

            while (Msg.ShowModal()==wxID_OK)
                wxLaunchDefaultBrowser("http://www.cafu.de");

            OnExit();
            return false;
        }
    }


    const wxString UserDataDir=wxStandardPaths::Get().GetUserDataDir();

    if (!wxFileName::Mkdir(UserDataDir, 0777, wxPATH_MKDIR_FULL))
        wxMessageBox(wxString("Config file storage path \n")+UserDataDir+"\n doesn't exist, and I was not able to create it, either.", "Warning!");


    static wxSingleInstanceChecker SIC(GetAppName()+"-"+wxGetUserId(), UserDataDir);

    if (SIC.IsAnotherRunning())
    {
        wxMessageBox("An instance of CaWE is already running!");
        OnExit();
        return false;
    }


    // Undo the wx locale initialization, as we want to be sure to use the same (default) locale "C" always and everywhere.
    // Using other locales introduces a lot of subtle errors. E.g. reading floating point numbers from anywhere
    // (like map files!) fails because e.g. "1.4" is no proper floating point string in the German locale (but "1,4" is).
    // setlocale(LC_ALL, "C");      // This alone is not enough, see http://trac.wxwidgets.org/ticket/12970 for details.
    for (int LangNr=wxLANGUAGE_ENGLISH; LangNr<=wxLANGUAGE_ENGLISH_ZIMBABWE; LangNr++)
    {
        if (wxLocale::IsAvailable(LangNr))
        {
            m_Locale=new wxLocale(LangNr, wxLOCALE_DONT_LOAD_DEFAULT);

            wxLogDebug("Program locale set to %s (%s, %s).", m_Locale->GetName(), m_Locale->GetCanonicalName(), m_Locale->GetLocale());
            break;
        }
    }

    if (!m_Locale)
    {
        // If the above for some reason didn't work, set at least the CRT to the "C" locale.
        setlocale(LC_ALL, "C");
        wxLogDebug("Program locale set to \"C\".");
    }


    // Activate a log console as the wx debug target.
    // wxLog::SetActiveTarget(new wxLogWindow(NULL, "wx Debug Console", true, false));

    srand(time(NULL));          // Re-seed the random number generator.
    wxInitAllImageHandlers();   // Needed e.g. for cursor initialization under GTK.

    // Initialize the global cursor manager instance.
    wxASSERT(CursorMan==NULL);
    CursorMan=new CursorManT;

    // Initialize the global art provider.
    if (wxArtProvider::HasNativeProvider())
        wxArtProvider::PushBack(new ArtProviderT("default"));
    else
        wxArtProvider::Push(new ArtProviderT("default"));


    // Set the globally used configuration storage object for easy access via wxConfigBase::Get().
    m_FileConfig=new wxFileConfig(GetAppName(), GetVendorName(), UserDataDir+"/CaWE.config");
    wxConfigBase::Set(m_FileConfig);

    // Setup the global Material Manager pointer.
    static MaterialManagerImplT MatManImpl;

    MaterialManager=&MatManImpl;

    // Register the material script with the CaWE materials definitions.
    if (MaterialManager->RegisterMaterialScript(AppDir+"/res/CaWE.cmat", AppDir+"/res/").Size()==0)
        wxMessageBox("CaWE.cmat not found in \""+AppDir+"\".", "WARNING");

    // Initialize the global options and game configs from the CaWE configuration files.
    // This also registers additional material scripts with the MaterialManager that must be known before the ParentFrameT ctor is called,
    // because under wxMSW, the ParentFrameT ctor calls the ParentFrameT::OnShow() method, where the global Gui Manager initialization
    // expects the "Gui/Default" material to be known.
    Options.Init();


    // Create the MDI parent frame.
    m_ParentFrame=new ParentFrameT(m_CmdLineParser);

    SetTopWindow(m_ParentFrame);


    // Check for autosave files.
    wxDir AutoSaveDir(UserDataDir);

    if (AutoSaveDir.IsOpened())
    {
        ArrayT<wxString> AutoSavedFileNames;
        wxString         AutoSavedFileName;

        for (bool more=AutoSaveDir.GetFirst(&AutoSavedFileName, "autosave*.cmap", wxDIR_FILES); more; more=AutoSaveDir.GetNext(&AutoSavedFileName))
            AutoSavedFileNames.PushBack(AutoSavedFileName);

        if (AutoSavedFileNames.Size()>0)
        {
            const unsigned long ID=wxGetLocalTime();
            wxString RecoveredList;

            for (unsigned long FileNr=0; FileNr<AutoSavedFileNames.Size(); FileNr++)
            {
                const wxString NewName =wxString::Format("recovered_%lu_%lu.cmap", FileNr, ID);
                const bool     RenameOK=wxRenameFile(UserDataDir+"/"+AutoSavedFileNames[FileNr], UserDataDir+"/"+NewName);

                RecoveredList+=RenameOK ? NewName : AutoSavedFileNames[FileNr] + "(WARNING: renaming to "+NewName+" failed! Back it up immediately yourself!)";
                RecoveredList+="\n";
            }

            wxMessageBox(wxString("A previously crashed or otherwise prematurely aborted instance of CaWE left auto-saved files.\n")+
                         "They are kept in "+UserDataDir+" under the name(s)\n\n"+RecoveredList+"\n"+
                         "You may open them as usual, and save, move, delete or rename them at your convenience.",
                         "Crash recovery: auto-save files found and recovered!", wxOK | wxICON_EXCLAMATION);
        }
    }
    else
    {
        wxMessageBox(wxString("Could not open auto-save directory ")+UserDataDir, "WARNING", wxOK | wxICON_ERROR);
    }

    return true;
}


int AppCaWE::OnExit()
{
    // TODO: delete Options; Options=NULL;

    wxConfigBase::Set(NULL);

    delete m_FileConfig;
    m_FileConfig=NULL;

    ConsoleInterpreter=NULL;     // This is very important, to make sure that no ConFuncT or ConVarT dtor accesses the ConsoleInterpreter that might already have been destroyed then.

    // Shutdown the global cursor manager instance.
    delete CursorMan;
    CursorMan=NULL;

    delete m_Locale;
    m_Locale=NULL;

    return wxApp::OnExit();
}
