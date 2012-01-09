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

#include "PreviewPage.hpp"

#include "FontWizard.hpp"

#include "Bitmap/Bitmap.hpp"

#include "wx/rawbmp.h"


BEGIN_EVENT_TABLE(PreviewPageT, wxWizardPageSimple)
    EVT_WIZARD_PAGE_CHANGED(wxID_ANY, PreviewPageT::OnWizardPageChanged)
END_EVENT_TABLE()


static wxBitmap ConvertBitmap(BitmapT* Source)
{
    wxBitmap          Result(Source->SizeX, Source->SizeY, 24);
    wxNativePixelData Data(Result);

    wxASSERT(Data);

    wxNativePixelData::Iterator Pixel(Data);

    Pixel.Offset(Data, 0, 0);

    for (unsigned int y=0; y<Source->SizeY; ++y)
    {
        wxNativePixelData::Iterator RowStart=Pixel;

        for (unsigned int x=0; x<Source->SizeX; ++x, ++Pixel)
        {
            const uint32_t RGBA =Source->Data[y*Source->SizeX + x];
            const float    Alpha=float((RGBA >> 24) & 0xFF)/255.0f;

            Pixel.Red()  =((RGBA >>  0) & 0xFF)*Alpha;
            Pixel.Green()=((RGBA >>  8) & 0xFF)*Alpha;
            Pixel.Blue() =((RGBA >> 16) & 0xFF)*Alpha;
        }

        Pixel=RowStart;
        Pixel.OffsetY(Data, 1);
    }

    return Result;
}


PreviewPageT::PreviewPageT(FontWizardT* Parent)
    : wxWizardPageSimple(Parent),
      m_Parent(Parent)
{
    // Note: The following code was generated by wxFormBuilder and copied here manually because wxFormbuilder
    // doesn't support wxWizardPage as base class.

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );

	wxStaticText* m_staticText5;
	m_staticText5 = new wxStaticText( this, wxID_ANY, wxT("Font preview:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	bSizer5->Add( m_staticText5, 0, wxALL, 5 );

	m_PreviewBitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_PreviewBitmap, 1, wxALL|wxEXPAND, 5 );

	m_PreviewButton = new wxButton( this, wxID_ANY, wxT("Click to see complete preview"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_PreviewButton, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	this->SetSizer( bSizer5 );
	this->Layout();

	// Connect Events
	m_PreviewButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PreviewPageT::OnPreviewButton ), NULL, this );
}


PreviewPageT::~PreviewPageT()
{
	// Disconnect Events
	m_PreviewButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PreviewPageT::OnPreviewButton ), NULL, this );
}


void PreviewPageT::OnPreviewButton(wxCommandEvent& CE)
{
    ArrayT<wxBitmap> PreviewBitmapsSmall;
    ArrayT<wxBitmap> PreviewBitmapsMedium;
    ArrayT<wxBitmap> PreviewBitmapsLarge;

    for (unsigned long SizeNr=0; SizeNr<m_Parent->GetNrOfSizes(); SizeNr++)
    {
        ArrayT<BitmapT*> Bitmaps=m_Parent->GetBitmaps(SizeNr);

        for (unsigned long i=0; i<Bitmaps.Size(); i++)
        {
            switch (SizeNr)
            {
                case 0:
                    PreviewBitmapsSmall.PushBack(ConvertBitmap(Bitmaps[i]));
                    break;

                case 1:
                    PreviewBitmapsMedium.PushBack(ConvertBitmap(Bitmaps[i]));
                    break;

                case 2:
                    PreviewBitmapsLarge.PushBack(ConvertBitmap(Bitmaps[i]));
                    break;

                default:
                    break;
            }
        }
    }

    // Create dialog that holds previews of all bitmaps.
    wxDialog PreviewPopup(this, wxID_ANY, "Font preview", wxDefaultPosition, wxSize(800, 875), wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU);

    wxBoxSizer* MainSizer=new wxBoxSizer(wxVERTICAL);


	MainSizer->Add(new wxStaticText(&PreviewPopup, wxID_ANY, wxT("Large"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

	wxBoxSizer* SizerLarge=new wxBoxSizer(wxHORIZONTAL);

    // Add all large bitmaps.
    for (unsigned long i=0; i<PreviewBitmapsLarge.Size(); i++)
    {
    	wxStaticBitmap* Tmp=new wxStaticBitmap(&PreviewPopup, wxID_ANY, PreviewBitmapsLarge[i], wxDefaultPosition, wxDefaultSize, 0);
	    SizerLarge->Add(Tmp, 0, wxLEFT, 5);
	}

	MainSizer->Add(SizerLarge, 0, 0, 5);


	MainSizer->Add(new wxStaticText(&PreviewPopup, wxID_ANY, "Medium", wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

	wxBoxSizer* SizerMedium=new wxBoxSizer(wxHORIZONTAL);

    // Add all medium bitmaps.
    for (unsigned long i=0; i<PreviewBitmapsMedium.Size(); i++)
    {
    	wxStaticBitmap* Tmp=new wxStaticBitmap(&PreviewPopup, wxID_ANY, PreviewBitmapsMedium[i], wxDefaultPosition, wxDefaultSize, 0);
	    SizerMedium->Add(Tmp, 0, wxLEFT, 5);
	}

	MainSizer->Add(SizerMedium, 0, 0, 5);


	MainSizer->Add(new wxStaticText(&PreviewPopup, wxID_ANY, "Small", wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

	wxBoxSizer* SizerSmall=new wxBoxSizer(wxHORIZONTAL);

    // Add all small bitmaps.
    for (unsigned long i=0; i<PreviewBitmapsSmall.Size(); i++)
    {
    	wxStaticBitmap* Tmp=new wxStaticBitmap(&PreviewPopup, wxID_ANY, PreviewBitmapsSmall[i], wxDefaultPosition, wxDefaultSize, 0);
	    SizerSmall->Add(Tmp, 0, wxLEFT, 5);
	}

    MainSizer->Add(SizerSmall, 0, 0, 5);


	PreviewPopup.SetSizer( MainSizer );
	PreviewPopup.Layout();

    PreviewPopup.ShowModal();
}


void PreviewPageT::OnWizardPageChanged(wxWizardEvent& WE)
{
    if (m_Parent->GetNrOfSizes()>0)
    {
        // Show image of largest font scale as preview.
        m_PreviewBitmap->SetBitmap(ConvertBitmap(m_Parent->GetBitmaps(m_Parent->GetNrOfSizes()-1)[0]));
    }
    else
    {
        m_PreviewBitmap->SetBitmap(wxNullBitmap);
    }

    this->Layout();
}
