/***************************************************************************
 *   Copyright (C) 2005 to 2015 by Jonathan Duddington                     *
 *   email: jonsd@users.sourceforge.net                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see:                                 *
 *               <http://www.gnu.org/licenses/>.                           *
 ***************************************************************************/

#include "wx/wx.h"


#include "wx/wfstream.h"
#include "wx/image.h"
#include "wx/filename.h"
#include "wx/numdlg.h"
#include "wx/mdi.h"
#include "wx/laywin.h"
#include "wx/sashwin.h"
#include "wx/utils.h"
#include "wx/html/htmlwin.h"
#include <locale.h>

#include "speak_lib.h"
#include "main.h"
#include "speech.h"
#include "options.h"
#include "phoneme.h"
#include "synthesize.h"
#include "voice.h"
#include "spect.h"
#include "translate.h"
#include "prosodydisplay.h"



static const char *about_string2 = "espeakedit: %s\nAuthor: Jonathan Duddington (c) 2009\n\n"
"Licensed under GNU General Public License version 3\n"
"http://espeak.sourceforge.net/";


static const char *about_string = "<font size=0><b>espeakedit </b> %s<br>Author: Jonathan Duddington (c) 2009<br>"
"<a href=\"http://espeak.sourceforge.net/\">http://espeak.sourceforge.net</a><br>"
"Licensed under <a href=\"http://espeak.sourceforge.net/license.html\">GNU General Public License version 3</a></font>";

const char *path_data;

extern void TestTest(int control);
extern void CompareLexicon(int);
extern void ConvertToUtf8();
extern void DictionaryFormat(const char *dictname);
extern void DictionarySort(const char *dictname);

extern void init_z();
extern void CompilePhonemeData(void);
extern void CompileSampleRate(void);
extern void CompileMbrola();
extern void CompileIntonation();
extern void InitSpectrumDisplay();
extern void InitProsodyDisplay();
extern void InitWaveDisplay();

extern void VowelChart(int control, char *fname);
extern void MakeVowelLists(void);
extern void MakeWordFreqList();

extern wxMenu *speak_menu;
extern wxMenu *data_menu;


MyFrame *myframe = NULL;
SpectDisplay *currentcanvas = NULL;

ProsodyDisplay *prosodycanvas = NULL;
wxNotebook *notebook = NULL;
wxNotebook *screenpages = NULL;

wxProgressDialog *progress;
int progress_max;
int gui_flag = 0;
int frame_x, frame_y, frame_w, frame_h;

int adding_page = 0;   // fix for wxWidgets (2,8,7) bug, adding first page to a wxNotebook gives emptystring for GetPageTex() in Notebook_Page_Changed event.

IMPLEMENT_APP(MyApp)

wxString AppName = _T("espeakedit");



int MyApp::OnExit()
{//================
	ConfigSave(1);
	return(0);
}


static const char *help_text =
"\n\nespeakedit\n"
"\tRun with GUI\n"
"espeakedit --compile\n"
"\tCompile phoneme data in espeak-data/phsource\n"
"\tand dictionary data in espeak-data/dictsource\n";


// Initialise this in OnInit, not statically
bool MyApp::OnInit(void)
{//=====================

int j;
const wxChar *p;
char param[120];


if(argc > 1)
{
	p = argv[1];
	j = 0;
	while((param[j] = p[j]) != 0) j++;

	if((strcmp(param,"--help")==0) || (strcmp(param,"-h")==0))
	{
		printf(about_string2,espeak_Info(NULL));
		printf("%s", help_text);
		exit(0);
	}

	ConfigInit();

	if(strcmp(param,"--compile")==0)
	{
        samplerate_native = samplerate = 22050;
	    LoadPhData(NULL);
		if(LoadVoice("", 0) == NULL)
		{
			fprintf(stderr, "Failed to load default voice\n");
			exit(1);
		}
		CompilePhonemeData();
		CompileIntonation();
	}
    exit(0);
}

	ConfigInit();
	gui_flag = 1;
	// It seems that the wctype functions don't work until the locale has been set
	// to something other than the default "C".  Then, not only Latin1 but also the
	// other characters give the correct results with iswalpha() etc.
	if(setlocale(LC_CTYPE,"en_US.UTF-8") == NULL)
	{
		if(setlocale(LC_CTYPE,"UTF-8") == NULL)
			setlocale(LC_CTYPE,"");
	}

	if((frame_w == 0) || (frame_h == 0))
	{
		frame_w = 800;
		frame_h = 768;
	}

  // Create the main frame window
	myframe = new MyFrame(NULL, -1, AppName, wxPoint(frame_x, frame_y), wxSize(frame_w, frame_h),
                      wxDEFAULT_FRAME_STYLE |
                      wxNO_FULL_REPAINT_ON_RESIZE |
                      wxHSCROLL | wxVSCROLL);


	// Make a menubar
	myframe->SetMenuBar(MakeMenu(0, voice_name2));
	myframe->CreateStatusBar();
	myframe->SetVoiceTitle(voice_name2);

//	myframe->Maximize();
	myframe->Show(TRUE);

	SetTopWindow(myframe);
	wxInitAllImageHandlers();
//	wxImage::AddHandler(wxPNGHandler);
	wxLogStatus(_T("Using espeak_data at: ")+wxString(path_home, wxConvLocal));
  return TRUE;
}

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_CHAR(MyFrame::OnKey)
   EVT_MENU(MENU_ABOUT, MyFrame::OnAbout)
   EVT_MENU(MENU_DOCS, MyFrame::OnAbout)
   EVT_MENU(MENU_SPECTRUM, MyFrame::OnNewWindow)
   EVT_MENU(MENU_SPECTRUM2, MyFrame::OnNewWindow)
   EVT_MENU(MENU_PROSODY, MyFrame::OnProsody)
   EVT_MENU(MENU_OPT_SPEED, MyFrame::OnOptions)
   EVT_MENU(MENU_OPT_PUNCT, MyFrame::OnOptions)
   EVT_MENU(MENU_OPT_SPELL, MyFrame::OnOptions)
   EVT_MENU(MENU_OPT_SPELL2, MyFrame::OnOptions)
   EVT_MENU(MENU_PATH_DATA, MyFrame::OnOptions)
   EVT_MENU(MENU_PATH0, MyFrame::OnOptions)
   EVT_MENU(MENU_PATH1, MyFrame::OnOptions)
   EVT_MENU(MENU_PATH2, MyFrame::OnOptions)
   EVT_MENU(MENU_PATH3, MyFrame::OnOptions)
   EVT_MENU(MENU_PATH4, MyFrame::OnOptions)
   EVT_MENU(MENU_COMPILE_PH, MyFrame::OnTools)
   EVT_MENU(MENU_COMPILE_PH2, MyFrame::OnTools)
	EVT_MENU(MENU_COMPILE_DICT, MyFrame::OnTools)
	EVT_MENU(MENU_COMPILE_DICT_DEBUG, MyFrame::OnTools)
	EVT_MENU(MENU_FORMAT_DICTIONARY, MyFrame::OnTools)
	EVT_MENU(MENU_SORT_DICTIONARY, MyFrame::OnTools)
	EVT_MENU(MENU_COMPILE_MBROLA, MyFrame::OnTools)
	EVT_MENU(MENU_COMPILE_INTONATION, MyFrame::OnTools)
	EVT_MENU(MENU_QUIT, MyFrame::OnQuit)
	EVT_MENU(MENU_SPEAK_TRANSLATE, MyFrame::OnSpeak)
	EVT_MENU(MENU_SPEAK_RULES, MyFrame::OnSpeak)
	EVT_MENU(MENU_SPEAK_IPA, MyFrame::OnSpeak)
	EVT_MENU(MENU_SPEAK_TEXT, MyFrame::OnSpeak)
	EVT_MENU(MENU_SPEAK_FILE, MyFrame::OnSpeak)
	EVT_MENU(MENU_SPEAK_STOP, MyFrame::OnSpeak)
	EVT_MENU(MENU_SPEAK_PAUSE, MyFrame::OnSpeak)
	EVT_MENU(MENU_SPEAK_VOICE, MyFrame::OnSpeak)
	EVT_MENU(MENU_SPEAK_VOICE_VARIANT, MyFrame::OnSpeak)
	EVT_MENU(MENU_LOAD_WAV, MyFrame::OnTools)
	EVT_MENU(MENU_VOWELCHART1, MyFrame::OnTools)
	EVT_MENU(MENU_VOWELCHART2, MyFrame::OnTools)
	EVT_MENU(MENU_VOWELCHART3, MyFrame::OnTools)
	EVT_MENU(MENU_LEXICON_RU, MyFrame::OnTools)
	EVT_MENU(MENU_LEXICON_BG, MyFrame::OnTools)
	EVT_MENU(MENU_LEXICON_DE, MyFrame::OnTools)
	EVT_MENU(MENU_LEXICON_IT, MyFrame::OnTools)
	EVT_MENU(MENU_LEXICON_TEST, MyFrame::OnTools)
	EVT_MENU(MENU_TO_UTF8, MyFrame::OnTools)
	EVT_MENU(MENU_COUNT_WORDS, MyFrame::OnTools)
	EVT_MENU(MENU_TEST, MyFrame::OnTools)
	EVT_MENU(MENU_TEST2, MyFrame::OnTools)

    EVT_MENU(SPECTSEQ_SAVE, MyFrame::PageCmd)
    EVT_MENU(SPECTSEQ_SAVEAS, MyFrame::PageCmd)
    EVT_MENU(SPECTSEQ_SAVESELECT, MyFrame::PageCmd)
    EVT_MENU(SPECTSEQ_CLOSE, MyFrame::PageCmd)
    EVT_MENU(SPECTSEQ_SAVEPITCH, MyFrame::PageCmd)
	EVT_MENU(MENU_CLOSE_ALL, MyFrame::PageCmd)


    EVT_NOTEBOOK_PAGE_CHANGED(ID_SCREENPAGES, MyFrame::OnPageChanged)
	EVT_TIMER(1, MyFrame::OnTimer)
END_EVENT_TABLE()


MyFrame::~MyFrame(void)
{//====================
	myframe->Maximize(false);
	myframe->Show(false);
	myframe->Iconize(false);   // os=Windows, get the non-iconsized size
	myframe->GetPosition(&frame_x, &frame_y);
	myframe->GetSize(&frame_w, &frame_h);
}

MyFrame::MyFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size,
    const long style):
  wxFrame(parent, id, title, pos, size, style)
{//===================================================================================================================
// Main Frame constructor

	int error_flag = 0;
	int result;
	int param;
	int srate;

	notebook = new wxNotebook(this, ID_NOTEBOOK, wxDefaultPosition, wxSize(312,760));
//	notebook->AddPage(voicedlg,_T("Voice"),FALSE);
	formantdlg = new FormantDlg(notebook);
	notebook->AddPage(formantdlg,_T(" Spect"),FALSE);
	voicedlg = new VoiceDlg(notebook);

	transldlg = new TranslDlg(notebook);
	notebook->AddPage(transldlg,_T("Text"),TRUE);


    screenpages = new wxNotebook(this, ID_SCREENPAGES, wxDefaultPosition, wxSize(554,702));

    wxBoxSizer *framesizer = new wxBoxSizer( wxHORIZONTAL );


    framesizer->Add(
        notebook,
        0,            // make horizontally stretchable
        wxEXPAND |    // make vertically stretchable
        wxALL,        //   and make border all around
        4 );         // set border width

    framesizer->Add(
        screenpages,
        1,            // make horizontally stretchable
        wxEXPAND |    // make vertically stretchable
        wxALL,        //   and make border all around
        4 );         // set border width

    SetSizer( framesizer );      // use the sizer for layout
    framesizer->SetSizeHints( this );   // set size hints to honour minimum size
    SetSize(pos.x, pos.y, size.GetWidth(), size.GetHeight());

	LoadConfig();

	if((result = LoadPhData(&srate)) != 1)
	{
		if(result == -1)
			wxLogError(_T("Failed to read espeak-data/phontab,phondata,phonindex\nPath = ")+wxString(path_home,wxConvLocal)+_T("\n\nThe 'eSpeak' package needs to be installed"));
		else
			wxLogError(_T("Wrong version of espeak-data at:\n")+ wxString(path_home,wxConvLocal)+_T("\nVersion 0x%x (expects 0x%x)"),result,version_phdata);

		error_flag = 1;
		srate = 22050;
	}
	WavegenInit(srate,0);
	WavegenInitSound();

	f_trans = stdout;
	option_ssml = 1;
	option_phoneme_input = 1;


//	if(LoadVoice(voice_name,0) == NULL)
	if(SetVoiceByName(voice_name2) != EE_OK)
	{
		if(error_flag==0)
			wxLogError(_T("Failed to load voice data"));
		strcpy(dictionary_name,"en");
	}
	WavegenSetVoice(voice);

	for(param=0; param<N_SPEECH_PARAM; param++)
		param_stack[0].parameter[param] = param_defaults[param];

	SetParameter(espeakRATE,option_speed,0);

	SetSpeed(3);
	SynthesizeInit();

	InitSpectrumDisplay();
	InitProsodyDisplay();
//	InitWaveDisplay();
	espeak_ListVoices(NULL);

   m_timer.SetOwner(this,1);

   m_timer.Start(500);   /* 0.5 timer */

}  // end of MyFrame::MyFrame


void MyFrame::SetVoiceTitle(char *voice_name)
{//==========================================
	char buf[100];

	if(samplerate_native == 22050)
        sprintf(buf, " - %s  voice", voice_name);
    else
        sprintf(buf, " - %s  voice  %dHz", voice_name, samplerate_native);
	SetTitle(AppName + wxString(buf,wxConvLocal));

	if((data_menu != NULL) && (translator != NULL))
	{
		sprintf(buf,"Compile &dictionary '%s'",translator->dictionary_name);
		data_menu->SetLabel(MENU_COMPILE_DICT, wxString(buf,wxConvLocal));
		sprintf(buf,"&Layout '%s_rules' file",translator->dictionary_name);
		data_menu->SetLabel(MENU_FORMAT_DICTIONARY, wxString(buf,wxConvLocal));
		sprintf(buf,"&Sort '%s_rules' file",translator->dictionary_name);
		data_menu->SetLabel(MENU_SORT_DICTIONARY, wxString(buf,wxConvLocal));
	}
}



void MyFrame::PageCmd(wxCommandEvent& event)
{//=========================================
    int pagenum;
    int ix;
    int n_pages;
    SpectDisplay *page;

//    if(currentcanvas != NULL)
    {
        pagenum = screenpages->GetSelection();

        switch(event.GetId())
        {
        case SPECTSEQ_SAVE:
            currentcanvas->Save(currentcanvas->savepath);
            break;
        case SPECTSEQ_SAVEAS:
            currentcanvas->Save();
            screenpages->SetPageText(screenpages->GetSelection(), currentcanvas->spectseq->name+_T(" ²"));
            break;
        case SPECTSEQ_SAVESELECT:
            currentcanvas->Save(_T(""), 1);
            break;
        case SPECTSEQ_CLOSE:
            if(screenpages->GetPageText(pagenum) != _T("Prosody"))
            {
                currentcanvas->OnActivate(0);
            }
            screenpages->DeletePage(pagenum);

            if((n_pages = screenpages->GetPageCount()) > 0)
            {
                if(pagenum >= n_pages)
                    pagenum--;
                page = (SpectDisplay *)screenpages->GetPage(pagenum);

                if(screenpages->GetPageText(pagenum) == _T("Prosody"))
                {
                    MakeMenu(3, NULL);
                }
                else
                {
                    page->OnActivate(1);
                    MakeMenu(2, NULL);
                }
            }
            else
            {
                MakeMenu(1, NULL);
            }
            break;

        case MENU_CLOSE_ALL:
            n_pages = screenpages->GetPageCount();
            for(ix=n_pages-1; ix>=0; ix--)
            {
                screenpages->DeletePage(ix);
            }
            currentcanvas = NULL;
            MakeMenu(1, NULL);
            break;
        case SPECTSEQ_SAVEPITCH:
            currentcanvas->SavePitchenv(currentcanvas->spectseq->pitchenv);
            break;
        }
    }
}



void MyFrame::OnPageChanged(wxNotebookEvent& event)
{//=================================================
    int pagenum;
    wxString title;
    SpectDisplay *page;

    pagenum = event.GetSelection();

    if(event.GetId() == ID_SCREENPAGES)
    {
        title = screenpages->GetPageText(pagenum);

        if((title != _T("Prosody")) && (adding_page != 2))
        {
            page = (SpectDisplay *)screenpages->GetPage(pagenum);

            if(page != currentcanvas)
            {
                if(currentcanvas != NULL)
                {
                    currentcanvas->OnActivate(0);
                }
                page->OnActivate(1);
            }
            MakeMenu(2, NULL);
        }
        else
        {
            MakeMenu(3, NULL);
        }
    }
	adding_page = 0;   // work around for wxNotebook bug (version 2.8.7)
}


void MyFrame::OnKey(wxKeyEvent& event)
{
	int key;

	key = event.GetKeyCode();

	if((currentcanvas != NULL) && (currentcanvas != FindFocus()))
	{
		if((key == WXK_F1) || (key == WXK_F2))
		{
			currentcanvas->OnKey(event);
			currentcanvas->SetFocus();
			return;
		}
	}

	event.Skip();
}

void MyFrame::OnTimer(wxTimerEvent &event)
//****************************************
{
   SynthOnTimer();
}


void MyFrame::OnQuit(wxCommandEvent& event)
{
	switch(event.GetId())
	{
	case MENU_QUIT:
		Close(TRUE);
		break;
	case MENU_CLOSE_ALL:
		break;
	}
}



class HtmlWindow: public wxHtmlWindow
{
	public:

	HtmlWindow(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style);
	void OnLinkClicked(const wxHtmlLinkInfo& link);
};

HtmlWindow::HtmlWindow(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style):
	wxHtmlWindow(parent, id, pos, size, style)
{
}

void HtmlWindow::OnLinkClicked(const wxHtmlLinkInfo& link)
{
	if(wxLaunchDefaultBrowser(link.GetHref()) == FALSE)
		wxLogStatus(_T("Failed to launch default browser: "+link.GetHref()));
}


void MyFrame::OnAbout(wxCommandEvent& event)
{//=========================================
	int result;
	char buf[300];
	wxString url_docs;

	wxBoxSizer *topsizer;
	HtmlWindow *html;
	wxDialog dlg(this, wxID_ANY, wxString(_("About")));

	topsizer = new wxBoxSizer(wxVERTICAL);

	switch(event.GetId())
	{
	case MENU_ABOUT:
		{
		sprintf(buf,about_string,espeak_Info(NULL));
		html = new HtmlWindow(&dlg, wxID_ANY, wxDefaultPosition, wxSize(380, 160), wxHW_SCROLLBAR_NEVER);
		html -> SetBorders(0);
		html -> SetPage(wxString(buf,wxConvLocal));
		html -> SetSize(html -> GetInternalRepresentation() -> GetWidth(),
								html -> GetInternalRepresentation() -> GetHeight());

		topsizer -> Add(html, 1, wxALL, 10);

//#if wxUSE_STATLINE
//		topsizer -> Add(new wxStaticLine(&dlg, wxID_ANY), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
//#endif // wxUSE_STATLINE

		wxButton *bu1 = new wxButton(&dlg, wxID_OK, _("OK"));
		bu1 -> SetDefault();

		topsizer -> Add(bu1, 0, wxALL | wxALIGN_RIGHT, 15);

		dlg.SetSizer(topsizer);
		topsizer -> Fit(&dlg);

		dlg.ShowModal();
		}
		break;

	case MENU_DOCS:
		strcpy(buf,"/docs/docindex.html");
		url_docs = wxGetCwd() +  wxString(buf,wxConvLocal);  // look for "docs" in the current directory
		if(!wxFileExists(url_docs))
		{
			strcpy(buf,"http://espeak.sourceforge.net/docindex.html");
			url_docs = wxString(buf,wxConvLocal);
		}
		else
		{
			url_docs = _T("file://") + url_docs;
		}

		result = wxLaunchDefaultBrowser(url_docs);
		if(result == 0)
			wxLogStatus(_T("Failed to launch default browser: "+url_docs));
		break;
	}
}



void OnOptions2(int event_id)
{//==========================
	wxString string;
	int value;

	switch(event_id)
	{
	case MENU_OPT_SPEED:
		value = wxGetNumberFromUser(_T(""),_T(""),_T("Speed"),option_speed,80,500);
		if(value > 0)
		{
			option_speed = value;
			SetParameter(espeakRATE,option_speed,0);
			SetSpeed(3);
		}
		break;

	case MENU_OPT_PUNCT:
		transldlg->t_source->SetValue(_T("<tts:style field=\"punctuation\" mode=\"all\">\n"));
		transldlg->t_source->SetInsertionPointEnd();
		notebook->SetSelection(1);
		break;

	case MENU_OPT_SPELL:
		transldlg->t_source->SetValue(_T("<say-as interpret-as=\"characters\">\n"));
		transldlg->t_source->SetInsertionPointEnd();
		notebook->SetSelection(1);
		break;

	case MENU_OPT_SPELL2:
		transldlg->t_source->SetValue(_T("<say-as interpret-as=\"tts:char\">\n"));
		transldlg->t_source->SetInsertionPointEnd();
		notebook->SetSelection(1);
		break;

	case MENU_PATH_DATA:
		string = wxDirSelector(_T("espeak_data directory"), path_espeakdata);
		if(!string.IsEmpty())
		{
			if(!wxDirExists(string+_T("/voices")))
			{
				wxLogError(_T("No 'voices' directory in ") + string);
				break;
			}
			path_espeakdata = string;
			wxLogMessage(_T("Quit and restart espeakedit to use the new espeak_data location"));
		}
		break;

	case MENU_PATH0:
		string = wxFileSelector(_T("Master phonemes file"),wxFileName(path_phfile).GetPath(),
			_T("phonemes"),_T(""),_T("*"),wxOPEN);
		if(!string.IsEmpty())
		{
			path_phfile = string;
		}
		break;

	case MENU_PATH1:
		string = wxDirSelector(_T("Phoneme source directory"),path_phsource);
		if(!string.IsEmpty())
		{
			path_phsource = string;
		}
		break;

	case MENU_PATH2:
		string = wxDirSelector(_T("Dictionary source directory"),path_dictsource);
		if(!string.IsEmpty())
		{
			path_dictsource = string;
		}
		break;

	case MENU_PATH3:
		string = wxFileSelector(_T("Sound output file"),wxFileName(path_speech).GetPath(),
			_T(""),_T("WAV"),_T("*"),wxSAVE);
		if(!string.IsEmpty())
		{
			path_speech = string;
		}
		break;

	case MENU_PATH4:
		string = wxFileSelector(_T("Voice file to modify formant peaks"),wxFileName(path_speech).GetPath(),
			_T(""),_T(""),_T("*"),wxOPEN);
		if(!string.IsEmpty())
		{
			path_modifiervoice = string;
		}
		break;
	}

	ConfigSetPaths();
}


void MyFrame::OnOptions(wxCommandEvent& event)
{//===========================================
	OnOptions2(event.GetId());
}


void DisplayErrorFile(const char *fname)
{//=====================================
	int len;
	FILE *f;
	char *msg;
	wxString msg_string;

	len = GetFileLength(fname);
	if(len > 0)
	{
		if(len > 1500)
			len = 1500;   // restrict length to prevent crash in wxLogMessage()
		msg = (char *)malloc(len+1);
		if(msg != NULL)
		{
			f = fopen(fname,"r");
			len = fread(msg,1, len, f);
			fclose(f);
			msg[len] = 0;
			msg_string = wxString(msg,wxConvUTF8);
			wxLogMessage(msg_string);
			free(msg);
		}
	}
}  // end of DisplayErrorFile



void MyFrame::OnTools(wxCommandEvent& event)
{//=========================================
	int err;
	FILE *log;
	int debug_flag=0;
	char fname_log[sizeof(path_dsource)+12];
	char err_fname[sizeof(path_home)+15];
	static unsigned const char utf8_bom[] = {0xef,0xbb,0xbf,0};

	switch(event.GetId())
	{
	case MENU_TEST:
		TestTest(0);
		break;

	case MENU_TEST2:
		TestTest(2);
		break;

	case MENU_TO_UTF8:
		ConvertToUtf8();
		break;

	case MENU_COUNT_WORDS:
		MakeWordFreqList();
		break;

	case MENU_LEXICON_RU:
	case MENU_LEXICON_BG:
	case MENU_LEXICON_DE:
	case MENU_LEXICON_IT:
	case MENU_LEXICON_TEST:
		CompareLexicon(event.GetId());  // Compare a lexicon with _rules translation
		break;

	case MENU_COMPILE_PH:
		CompilePhonemeData();
		SetVoiceTitle(voice_name2);
		break;

	case MENU_COMPILE_PH2:
		CompileSampleRate();
		SetVoiceTitle(voice_name2);
		break;

	case MENU_COMPILE_MBROLA:
		CompileMbrola();
		break;

	case MENU_COMPILE_INTONATION:
		CompileIntonation();
		break;

	case MENU_COMPILE_DICT_DEBUG:
		debug_flag =1;  // and drop through to next case
	case MENU_COMPILE_DICT:
		sprintf(fname_log,"%s%s",path_dsource,"dict_log");
		log = fopen(fname_log,"w");
		if(log != NULL)
		{
			fprintf(log, "%s", utf8_bom);
		}

		LoadDictionary(translator, translator->dictionary_name, 0);
		if((err = CompileDictionary(path_dsource,translator->dictionary_name,log,err_fname,debug_flag)) < 0)
		{
			wxLogError(_T("Can't access file:\n")+wxString(err_fname,wxConvLocal));

			wxString dir = wxDirSelector(_T("Directory containing dictionary files"),path_dictsource);
			if(!dir.IsEmpty())
			{
				path_dictsource = dir;
				strncpy0(path_dsource,path_dictsource.mb_str(wxConvLocal),sizeof(path_dsource)-1);
				strcat(path_dsource,"/");
			}
			break;
		}
		wxLogStatus(_T("Compiled '")+wxString(dictionary_name,wxConvLocal)+_T("', %d errors"),err);

		if(log != NULL)
		{
			fclose(log);

			if(err > 0)
			{
				// display the error messages
				DisplayErrorFile(fname_log);
			}
		}
		break;

	case MENU_FORMAT_DICTIONARY:
		DictionaryFormat(dictionary_name);
		break;

	case MENU_SORT_DICTIONARY:
		DictionarySort(dictionary_name);
		break;

	case MENU_VOWELCHART1:
		MakeVowelLists();
		break;

	case MENU_VOWELCHART2:
		VowelChart(2,NULL);
		break;

	case MENU_VOWELCHART3:
		VowelChart(3,NULL);
		break;

	case MENU_LOAD_WAV:
//		LoadWavFile();
		break;
	}
}

void MyFrame::OnSpeak(wxCommandEvent& event)
{//=========================================
	switch(event.GetId())
	{
	case MENU_SPEAK_TRANSLATE:
	case MENU_SPEAK_RULES:
	case MENU_SPEAK_IPA:
	case MENU_SPEAK_TEXT:
		transldlg->OnCommand(event);
		break;

	case MENU_SPEAK_FILE:
		out_ptr = NULL;
		transldlg->SpeakFile();
		break;

	case MENU_SPEAK_STOP:
		SpeakNextClause(NULL,NULL,2);
		break;

	case MENU_SPEAK_PAUSE:
		out_ptr = NULL;
		SpeakNextClause(NULL,NULL,3);
		if(SynthStatus() & 2)
			speak_menu->SetLabel(MENU_SPEAK_PAUSE,_T("&Resume"));
		else
		{
			speak_menu->SetLabel(MENU_SPEAK_PAUSE,_T("&Pause"));
		}
		break;

	case MENU_SPEAK_VOICE:
		transldlg->ReadVoice(0);
		SetVoiceTitle(voice_name2);
		break;

	case MENU_SPEAK_VOICE_VARIANT:
		transldlg->ReadVoice(1);
		SetVoiceTitle(voice_name2);
		break;
	}
}



