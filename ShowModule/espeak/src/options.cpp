/***************************************************************************
 *   Copyright (C) 2005 to 2013 by Jonathan Duddington                     *
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
#include "wx/fileconf.h"
#include "wx/filename.h"
#include <sys/stat.h>
#include "speech.h"

#ifdef PLATFORM_WINDOWS
#include "wx/msw/registry.h"
#endif

#include "main.h"

#include "options.h"

extern void WavegenInit(int samplerate, int wavemult_fact);
extern void strncpy0(char *to,const char *from, int size);
extern int GetNumeric(wxTextCtrl *t);
extern void SetNumeric(wxTextCtrl *t, int value);
extern int samplerate;

wxString path_espeakdata;
wxString path_spectload;
wxString path_spectload2;
wxString path_pitches;
wxString path_wave;
wxString path_speech;
wxString path_phfile;
wxString path_phsource;
wxString path_dictsource;
wxString path_speaktext;
wxString path_modifiervoice;
wxString path_dir1;
int option_speed=160;

char path_dsource[sizeof(path_home)+20];
char voice_name2[40];

BEGIN_EVENT_TABLE(Options, wxDialog)
		EVT_BUTTON(wxID_SAVE,Options::OnCommand)
		EVT_BUTTON(wxID_CLOSE,Options::OnCommand)
END_EVENT_TABLE()


Options::Options(wxWindow *parent) : wxDialog(parent,-1,_T("Options"),wxDefaultPosition,wxDefaultSize)
{//===================================================================================================

	m_lab[0] = new wxStaticText(this,-1,_T("Sample rate"),wxPoint(72,84));
	m_samplerate = new wxTextCtrl(this,-1,_T(""),wxPoint(8,80),wxSize(60,24));
	SetNumeric(m_samplerate,samplerate);

	m_save = new wxButton(this,wxID_SAVE,_T("Save"),wxPoint(8,120));
	m_close = new wxButton(this,wxID_CLOSE,_T("Cancel"),wxPoint(98,120));
	Show();
}


Options::~Options()
{//================
	int ix;

	for(ix=0; ix < 1; ix++)
		delete m_lab[ix];

	delete m_samplerate;
	delete m_save;
	delete m_close;
}

void Options::OnCommand(wxCommandEvent& event)
{//===========================================
	int  id;
	int  value;

	switch(id = event.GetId())
	{
	case wxID_SAVE:
		value = GetNumeric(m_samplerate);
		if(value > 0) WavegenInit(value,0);

		Destroy();
		break;

	case wxID_CLOSE:
		Destroy();
		break;
	}
}



void ConfigSetPaths()
{//==================
	// set c_string paths from wxStrings
	strncpy0(path_source,path_phsource.mb_str(wxConvLocal),sizeof(path_source)-1);
	strcat(path_source,"/");

	strncpy0(path_dsource,path_dictsource.mb_str(wxConvLocal),sizeof(path_source)-1);
	strcat(path_dsource,"/");
}


void ConfigInit()
{//==============
	wxString string;
	wxString basedir;
	const char *path_base;

#ifdef PLATFORM_WINDOWS
	int found = 0;
	char buf[200];
	wxRegKey *pRegKey = new wxRegKey(_T("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Speech\\Voices\\Tokens\\eSpeak"));

	if((path_base = getenv("ESPEAK_DATA_PATH")) != NULL)
	{
		sprintf(path_home,"%s\\espeak-data",path_base);
		if(GetFileLength(path_home) == -2)
			found = 1;   // an espeak-data directory exists
	}

	if(found == 0)
	{
		if(pRegKey->Exists() )
		{
			wxString RegVal;
			pRegKey->QueryValue(_T("Path"),RegVal);
			strncpy0(buf,RegVal.mb_str(wxConvLocal),sizeof(buf));
			path_base = buf;
		}
		else
		{
			path_base = "C:\\Program Files\\eSpeak";
		}
		sprintf(path_home,"%s\\espeak-data",path_base);
	}
#else
	snprintf(path_home,sizeof(path_home),"%s/espeak-data",getenv("HOME"));
	path_base = path_home;
#endif
	mkdir(path_home,S_IRWXU);    // create if it doesn't already exist

	wxFileConfig *pConfig = new wxFileConfig(_T("espeakedit"));
	wxFileConfig::Set(pConfig);

	basedir = wxString(path_base,wxConvLocal);  // this is only used to set defaults for other paths if they are not in the config file
	pConfig->Read(_T("/espeakdata"),&path_espeakdata,wxEmptyString);
	if(path_espeakdata != wxEmptyString)
	{
		strcpy(path_home, path_espeakdata.mb_str(wxConvLocal));
	}

	pConfig->Read(_T("/spectload"),&path_spectload,basedir+_T("/phsource"));
	pConfig->Read(_T("/spectload2"),&path_spectload2,basedir+_T("/phsource"));
	pConfig->Read(_T("/pitchpath"),&path_pitches,basedir+_T("/pitch"));
	pConfig->Read(_T("/wavepath"),&path_wave,wxEmptyString);
	pConfig->Read(_T("/speechpath"),&path_speech,wxEmptyString);
	pConfig->Read(_T("/voicename"),&string,wxEmptyString);
	strcpy(voice_name2,string.mb_str(wxConvLocal));
	pConfig->Read(_T("/phsource"),&path_phsource,basedir+_T("/phsource"));
	pConfig->Read(_T("/phfile"),&path_phfile,path_phsource+_T("/phonemes"));
	pConfig->Read(_T("/dictsource"),&path_dictsource,basedir+_T("/dictsource"));
	pConfig->Read(_T("/speaktext"),&path_speaktext,wxEmptyString);
	pConfig->Read(_T("/modifiervoice"),&path_modifiervoice,basedir);
	pConfig->Read(_T("/dir1"),&path_dir1,basedir);
	option_speed = pConfig->Read(_T("/speed"),160);
	frame_x = pConfig->Read(_T("/windowx"), 0l);
	frame_y = pConfig->Read(_T("/windowy"), 0l);
	frame_h = pConfig->Read(_T("/windowh"), 0l);
	frame_w = pConfig->Read(_T("/windoww"), 0l);
	ConfigSetPaths();
}  // end of ConfigInit



void ConfigSave(int exit)
{//======================
	wxFileConfig *pConfig = (wxFileConfig *)(wxConfigBase::Get());


#ifndef PLATFORM_WINDOWS
//	pConfig->Write(_T("/samplerate"),samplerate);
#endif
	pConfig->Write(_T("/espeakdata"),path_espeakdata);
	pConfig->Write(_T("/spectload"),path_spectload);
	pConfig->Write(_T("/spectload2"),path_spectload2);
	pConfig->Write(_T("/pitchpath"),path_pitches);
	pConfig->Write(_T("/wavepath"),path_wave);
	pConfig->Write(_T("/speechpath"),path_speech);
	pConfig->Write(_T("/voicename"),wxString(voice_name2,wxConvLocal));
	pConfig->Write(_T("/phsource"),path_phsource);
	pConfig->Write(_T("/phfile"),path_phfile);
	pConfig->Write(_T("/dictsource"),path_dictsource);
	pConfig->Write(_T("/speaktext"),path_speaktext);
	pConfig->Write(_T("/speed"),option_speed);
	pConfig->Write(_T("/modifiervoice"),path_modifiervoice);
	pConfig->Write(_T("/dir1"),path_dir1);
	pConfig->Write(_T("/windowx"),frame_x);
	pConfig->Write(_T("/windowy"),frame_y);
	pConfig->Write(_T("/windoww"),frame_w);
	pConfig->Write(_T("/windowh"),frame_h);
	if(exit)
		delete wxFileConfig::Set((wxFileConfig *)NULL);
}
