/////////////////////////////////////////////////////////////////////////////
//
// MMSwitch.cpp : Implementation of CMMSwitch
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MMSwitch.h"
#include "CPropertyDialog.h"
#include "..\MMSwitcher\iMMSwitch.h"

/////////////////////////////////////////////////////////////////////////////
// CMMSwitch::CMMSwitch
// Constructor

CMMSwitch::CMMSwitch()
{
    lstrcpyn(m_szPluginText, _T("MMSwitch Plugin"), sizeof(m_szPluginText) / sizeof(m_szPluginText[0]));
    m_dwAdviseCookie = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMMSwitch::~CMMSwitch
// Destructor

CMMSwitch::~CMMSwitch()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMMSwitch:::FinalConstruct
// Called when an plugin is first loaded. Use this function to do one-time
// intializations that could fail instead of doing this in the constructor,
// which cannot return an error.

HRESULT CMMSwitch::FinalConstruct()
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CMMSwitch:::FinalRelease
// Called when a plugin is unloaded. Use this function to free any
// resources allocated in FinalConstruct.

void CMMSwitch::FinalRelease()
{
    ReleaseCore();
}

/////////////////////////////////////////////////////////////////////////////
// CMMSwitch::SetCore
// Set WMP core interface

HRESULT CMMSwitch::SetCore(IWMPCore *pCore)
{
    HRESULT hr = S_OK;

    // release any existing WMP core interfaces
    ReleaseCore();

    // If we get passed a NULL core, this  means
    // that the plugin is being shutdown.

    if (pCore == NULL)
    {
        return S_OK;
    }

    m_spCore = pCore;

    // connect up the event interface
    CComPtr<IConnectionPointContainer>  spConnectionContainer;

    hr = m_spCore->QueryInterface( &spConnectionContainer );

    if (SUCCEEDED(hr))
    {
        hr = spConnectionContainer->FindConnectionPoint( __uuidof(IWMPEvents), &m_spConnectionPoint );
    }

    if (SUCCEEDED(hr))
    {
        hr = m_spConnectionPoint->Advise( GetUnknown(), &m_dwAdviseCookie );

        if ((FAILED(hr)) || (0 == m_dwAdviseCookie))
        {
            m_spConnectionPoint = NULL;
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// CMMSwitch::ReleaseCore
// Release WMP core interfaces

void CMMSwitch::ReleaseCore()
{
    if (m_spConnectionPoint)
    {
        if (0 != m_dwAdviseCookie)
        {
            m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
            m_dwAdviseCookie = 0;
        }
        m_spConnectionPoint = NULL;
    }

    if (m_spCore)
    {
        m_spCore = NULL;
    }
}




/////////////////////////////////////////////////////////////////////////////
// CMMSwitch::DisplayPropertyPage
// Display property page for plugin

HRESULT CMMSwitch::DisplayPropertyPage(HWND hwndParent)
{
    /*CPropertyDialog dialog(this);

    dialog.DoModal(hwndParent);*/

	// Get a pointer to the current media item.
	CComPtr<IWMPMedia> spMedia;
	HRESULT hr = m_spCore->get_currentMedia(&spMedia);
	if (FAILED(hr) || !spMedia)
		return S_FALSE;

	CComBSTR bstrSourceURL;
	spMedia->get_sourceURL(&bstrSourceURL);

	HMODULE hModule = GetModuleHandle("MMSwitch.ax");
	PGetInstance pGetInstance = (PGetInstance)GetProcAddress(hModule, "GetInstance");

	if (!pGetInstance)
		return S_FALSE;

	ISwitcher* pISwitcher = pGetInstance(bstrSourceURL);

	if (!pISwitcher)
		return S_FALSE;

	pISwitcher->AddRef();

	OleCreatePropertyFrame(hwndParent,
		0,
		0,
		L"Morgan Stream Switcher",
		1,
		(IUnknown **)&pISwitcher,
		0,
		NULL,
		0,
		0,
		NULL);

	pISwitcher->Release();

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CMMSwitch::GetProperty
// Get plugin property

HRESULT CMMSwitch::GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty)
{
    if (NULL == pvarProperty)
    {
        return E_POINTER;
    }

    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// CMMSwitch::SetProperty
// Set plugin property

HRESULT CMMSwitch::SetProperty(const WCHAR *pwszName, const VARIANT *pvarProperty)
{
    return E_NOTIMPL;
}
