//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1992 - 1998  Microsoft Corporation.  All Rights Reserved.
//

#include <streams.h>

// Eliminate two expected level 4 warnings from the Microsoft compiler.
// The class does not have an assignment or copy operator, and so cannot
// be passed by value.  This is normal.  This file compiles clean at the
// highest (most picky) warning level (-W4).
#pragma warning(disable: 4511 4512)

#include <windowsx.h>
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#include "resource.h"    // ids used in the dialog
#include "MMSwitchuids.h"    // public guids
#include "iMMSwitch.h"       // private interface between property sheet and filter
#include "MMSwitchprop.h"    // our own class

//
// CreateInstance
//
// Override CClassFactory method.
// Set lpUnk to point to an IUnknown interface on a new SwitcherIPProperties object
// Part of the COM object instantiation mechanism
//
CUnknown * WINAPI SwitcherIPProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{

    CUnknown *punk = new SwitcherIPProperties(lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;
}


//
// SwitcherIPProperties::Constructor
//
// Constructs and initialises a SwitcherIPProperties object
//
SwitcherIPProperties::SwitcherIPProperties(LPUNKNOWN pUnk, HRESULT *phr)
    : CBasePropertyPage(NAME("SwitcherIP Property Page"),pUnk,
        IDD_DIALOG1, IDS_TITLE)
    , m_pISwitcher(NULL)
{
    ASSERT(phr);

} // (constructor) SwitcherIPProperties


//
// SetDirty
//
// Sets m_hrDirtyFlag and notifies the property page site of the change
//
void SwitcherIPProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
    {
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }

} // SetDirty

BOOL EditProc(HWND hwnd,
            UINT uMsg,
            WPARAM wParam,
            LPARAM lParam)
{
	SwitcherIPProperties *pProperties = (SwitcherIPProperties *)GetWindowLong(hwnd, GWL_USERDATA);

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
			pProperties = (SwitcherIPProperties *)lParam;
			SetWindowLong(hwnd, GWL_USERDATA, lParam);

			char szLang[MAX_PATH];
			int i = SendDlgItemMessage(pProperties->GetWindow(), IDC_LANGLIST, LB_GETCURSEL , 0, 0);
			LCID lcidTrack = SendDlgItemMessage(pProperties->GetWindow(), IDC_LANGLIST, LB_GETITEMDATA , i, 0);
			int selTrack = 0;
			LCID lcidDef = pProperties->GetDefaultLCID();
			int selDef = 0;

			LCID lcid = 0;
			int len;
			i= SendDlgItemMessage(hwnd, IDC_DEFLANGLIST, CB_INSERTSTRING , -1, (LPARAM)"(Default)");
			SendDlgItemMessage(hwnd, IDC_DEFLANGLIST, CB_SETITEMDATA, i, lcid);
			i= SendDlgItemMessage(hwnd, IDC_LANGLIST, CB_INSERTSTRING , -1, (LPARAM)"(None)");
			SendDlgItemMessage(hwnd, IDC_LANGLIST, CB_SETITEMDATA, i, lcid);

			for (int lang = 1; lang <= 0x3ff; lang++)
			{		
				lcid = MAKELCID(MAKELANGID(lang, SUBLANG_NEUTRAL), SORT_DEFAULT);
		
				// Local lang name
				len = GetLocaleInfo(lcid, LOCALE_SLANGUAGE, (char *)szLang, MAX_PATH);
				if (--len > 0)
				{
					i = SendDlgItemMessage(hwnd, IDC_DEFLANGLIST, CB_INSERTSTRING , -1, (LPARAM)szLang);
					SendDlgItemMessage(hwnd, IDC_DEFLANGLIST, CB_SETITEMDATA, i, lcid);
					i= SendDlgItemMessage(hwnd, IDC_LANGLIST, CB_INSERTSTRING , -1, (LPARAM)szLang);
					SendDlgItemMessage(hwnd, IDC_LANGLIST, CB_SETITEMDATA, i, lcid);

					if (LANGIDFROMLCID(lcid) == LANGIDFROMLCID(lcidTrack))
						selTrack = i;
					if (LANGIDFROMLCID(lcid) == LANGIDFROMLCID(lcidDef))
						selDef = i;
				}
			}

			SendDlgItemMessage(hwnd, IDC_DEFLANGLIST, CB_SETCURSEL , selDef, (LPARAM)0);
			SendDlgItemMessage(hwnd, IDC_LANGLIST, CB_SETCURSEL , selTrack, (LPARAM)0);
			SetFocus(GetDlgItem(hwnd, IDC_LANGLIST));
            return TRUE;
        }

        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDOK)
			{
				char szLang[MAX_PATH];
				int i = SendDlgItemMessage(hwnd, IDC_LANGLIST, CB_GETCURSEL , 0, 0);
				if (i > 0)
				{
					LCID lcidTrack = SendDlgItemMessage(hwnd, IDC_LANGLIST, CB_GETITEMDATA , i, 0);
					SendDlgItemMessage(hwnd, IDC_LANGLIST, CB_GETLBTEXT , i, (LPARAM)szLang);

					i = SendDlgItemMessage(pProperties->GetWindow(), IDC_LANGLIST, LB_GETCURSEL , 0, 0);
					if (lcidTrack != (LCID)SendDlgItemMessage(pProperties->GetWindow(), IDC_LANGLIST, LB_GETITEMDATA , i, 0))
					{
						SendDlgItemMessage(pProperties->GetWindow(), IDC_LANGLIST, LB_DELETESTRING , i, 0);
						SendDlgItemMessage(pProperties->GetWindow(), IDC_LANGLIST, LB_INSERTSTRING , i, (LPARAM)szLang);
						SendDlgItemMessage(pProperties->GetWindow(), IDC_LANGLIST, LB_SETITEMDATA , i, lcidTrack);
						SendDlgItemMessage(pProperties->GetWindow(), IDC_LANGLIST, LB_SETCURSEL , i, 0);
						pProperties->SetDirty();
					}
				}

				i = SendDlgItemMessage(hwnd, IDC_DEFLANGLIST, CB_GETCURSEL , 0, 0);
				LCID lcid = SendDlgItemMessage(hwnd, IDC_DEFLANGLIST, CB_GETITEMDATA , i, 0);
				if (lcid != pProperties->GetDefaultLCID())
				{
					pProperties->SetDefaultLCID(lcid);
					pProperties->SetDirty();
				}

				EndDialog(hwnd, IDOK);
			}
            else if (LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hwnd, IDCANCEL);
			}

            return TRUE;
        }

    }
    return FALSE;

} // OnReceiveMessage

//
// OnReceiveMessage
//
// Override CBasePropertyPage method.
// Handles the messages for our property window
//
BOOL SwitcherIPProperties::OnReceiveMessage(HWND hwnd,
                                        UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            return (LRESULT) 1;
        }

        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDC_BUTTON1)
			{
				m_pISwitcher->Switch();
				SendDlgItemMessage(m_Dlg, IDC_LANGLIST, LB_SETCURSEL , m_pISwitcher->GetActiveTrack(), 0);
			}
            else if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				m_pISwitcher->SwitchTo(SendDlgItemMessage(m_Dlg, IDC_LANGLIST, LB_GETCURSEL , 0, 0));
			}
            else if (LOWORD(wParam) == IDC_EDIT)
			{
				DialogBoxParam((HINSTANCE__ *)GetWindowLong(hwnd, GWL_HINSTANCE), MAKEINTRESOURCE(IDD_DIALOG2), m_Dlg, EditProc, (LPARAM)this);
			}

            return (LRESULT) 1;
        }

    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

} // OnReceiveMessage


int UpdateLogo(HWND hdlg)
{
	HANDLE hBitmap; //, hOldBitmap;

	hBitmap = LoadImage((HINSTANCE__ *)GetWindowLong(hdlg, GWL_HINSTANCE), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP,
						0, 0, LR_LOADMAP3DCOLORS | LR_DEFAULTSIZE);
	if (hBitmap)
	{
		//hOldBitmap = (HANDLE)SendDlgItemMessage(hdlg, IDB_BITMAPMORGAN, STM_GETIMAGE, IMAGE_BITMAP, 0);
		SendDlgItemMessage(hdlg, IDB_BITMAPMORGAN, STM_SETIMAGE, IMAGE_BITMAP, (long)hBitmap);
		//DeleteObject(hOldBitmap);
	}

	return 1;
}

//
// OnConnect
//
// Override CBasePropertyPage method.
// Notification of which object this property page should display.
// We query the object for the ISwitcher interface.
//
// If cObjects == 0 then we must release the interface.
// Set the member variable m_pPin to the upstream output pin connected
// to our input pin (or NULL if not connected).
//
HRESULT SwitcherIPProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pISwitcher == NULL);

    HRESULT hr = pUnknown->QueryInterface(IID_ISwitcher, (void **) &m_pISwitcher);
    if (FAILED(hr))
    {
        return E_NOINTERFACE;
    }

    ASSERT(m_pISwitcher);

    return NOERROR;

} // OnConnect


//
// OnDisconnect
//
// Override CBasePropertyPage method.
// Release the private interface, release the upstream pin.
//
HRESULT SwitcherIPProperties::OnDisconnect()
{
    // Release of Interface

    if (m_pISwitcher == NULL)
        return E_UNEXPECTED;
    m_pISwitcher->Release();
    m_pISwitcher = NULL;

    return NOERROR;

} // OnDisconnect


//
// Activate
//
// We are being activated
//
extern LCID LangToLCID(char* pLang);

HRESULT SwitcherIPProperties::OnActivate()
{
	UpdateLogo(m_Dlg);

    ASSERT(m_pISwitcher);

	long defIndex = 0;
	m_pISwitcher->GetDefTrackIndex(&defIndex);
	m_defLCID = 0;

	char szName[MAX_PATH];
	int n = 0;
	while (m_pISwitcher->GetTrackName(n++, szName) == S_OK)
	{
		LCID lcid = LangToLCID(szName);
		int i = SendDlgItemMessage(m_Dlg, IDC_LANGLIST, LB_INSERTSTRING , -1, (LPARAM)szName);
		SendDlgItemMessage(m_Dlg, IDC_LANGLIST, LB_SETITEMDATA , i, lcid);

		if (n == defIndex)
			m_defLCID = lcid;
	}

	SendDlgItemMessage(m_Dlg, IDC_LANGLIST, LB_SETCURSEL , m_pISwitcher->GetActiveTrack(), 0);

    return NOERROR;

} // Activate


//
// OnApplyChanges
//
// Changes made should be kept.
//
HRESULT SwitcherIPProperties::OnApplyChanges()
{

    ASSERT(m_pISwitcher);

	if (IsPageDirty() == S_OK)
	{
		LCID lcid = 0;
		char szName[MAX_PATH];
		int n = SendDlgItemMessage(m_Dlg, IDC_LANGLIST, LB_GETCOUNT , 0, 0);
		for (int i = 0; i < n; i++)
		{
			SendDlgItemMessage(m_Dlg, IDC_LANGLIST, LB_GETTEXT , i, (LPARAM)szName);
			lcid = SendDlgItemMessage(m_Dlg, IDC_LANGLIST, LB_GETITEMDATA , i, 0);
			m_pISwitcher->SetTrackName(i, szName);
			m_pISwitcher->SetTrackLCID(i, lcid);

			if (lcid == m_defLCID)
				m_pISwitcher->SetDefTrackIndex(i + 1);
		}	
	}

    return NOERROR;

} // OnApplyChanges


