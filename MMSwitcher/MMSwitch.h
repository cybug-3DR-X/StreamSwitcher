/////////////////////////////////////////////////////////////////////////////
//
// MMSwitch.h : Declaration of the CMMSwitch
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __MMSWITCH_H_
#define __MMSWITCH_H_

#pragma once

#include "resource.h"
#include "wmpplug.h"

// {E822508C-390A-4b0b-A2D4-8FF50AA7DD18}
DEFINE_GUID(CLSID_MMSwitch, 0xE822508C, 0x390A, 0x4B0B, 0xA2, 0xD4, 0x8F, 0xF5, 0x0A, 0xA7, 0xDD, 0x18);

/////////////////////////////////////////////////////////////////////////////
// CMMSwitch
class ATL_NO_VTABLE CMMSwitch : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CMMSwitch, &CLSID_MMSwitch>,
    public IWMPEvents,
    public IWMPPluginUI,
	public IPropertyBag
{
public:
    CMMSwitch();
    ~CMMSwitch();

DECLARE_REGISTRY_RESOURCEID(IDR_MMSWITCH)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMMSwitch)
    COM_INTERFACE_ENTRY(IWMPEvents)
    COM_INTERFACE_ENTRY(IWMPPluginUI)
    COM_INTERFACE_ENTRY(IPropertyBag)
END_COM_MAP()

    // CComCoClass methods
    HRESULT FinalConstruct();
    void    FinalRelease();

    // IWMPPluginUI methods
    STDMETHODIMP SetCore(IWMPCore *pCore);
    STDMETHODIMP Create(HWND hwndParent, HWND *phwndWindow) { return E_NOTIMPL; }
    STDMETHODIMP Destroy() { return E_NOTIMPL; }
    STDMETHODIMP TranslateAccelerator(LPMSG lpmsg) { return E_NOTIMPL; }
    STDMETHODIMP DisplayPropertyPage(HWND hwndParent);
    STDMETHODIMP GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty);
    STDMETHODIMP SetProperty(const WCHAR *pwszName, const VARIANT *pvarProperty);

    // IWMPEvents methods
    void STDMETHODCALLTYPE OpenStateChange( long NewState );
    void STDMETHODCALLTYPE PlayStateChange( long NewState );
    void STDMETHODCALLTYPE AudioLanguageChange( long LangID );
    void STDMETHODCALLTYPE StatusChange();
    void STDMETHODCALLTYPE ScriptCommand( BSTR scType, BSTR Param );
    void STDMETHODCALLTYPE NewStream();
    void STDMETHODCALLTYPE Disconnect( long Result );
    void STDMETHODCALLTYPE Buffering( VARIANT_BOOL Start );
    void STDMETHODCALLTYPE Error();
    void STDMETHODCALLTYPE Warning( long WarningType, long Param, BSTR Description );
    void STDMETHODCALLTYPE EndOfStream( long Result );
    void STDMETHODCALLTYPE PositionChange( double oldPosition, double newPosition);
    void STDMETHODCALLTYPE MarkerHit( long MarkerNum );
    void STDMETHODCALLTYPE DurationUnitChange( long NewDurationUnit );
    void STDMETHODCALLTYPE CdromMediaChange( long CdromNum );
    void STDMETHODCALLTYPE PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change );
    void STDMETHODCALLTYPE CurrentPlaylistChange( WMPPlaylistChangeEventType change );
    void STDMETHODCALLTYPE CurrentPlaylistItemAvailable( BSTR bstrItemName );
    void STDMETHODCALLTYPE MediaChange( IDispatch * Item );
    void STDMETHODCALLTYPE CurrentMediaItemAvailable( BSTR bstrItemName );
    void STDMETHODCALLTYPE CurrentItemChange( IDispatch *pdispMedia);
    void STDMETHODCALLTYPE MediaCollectionChange();
    void STDMETHODCALLTYPE MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal );
    void STDMETHODCALLTYPE MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal );
    void STDMETHODCALLTYPE MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal);
    void STDMETHODCALLTYPE PlaylistCollectionChange();
    void STDMETHODCALLTYPE PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName);
    void STDMETHODCALLTYPE PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName);
    void STDMETHODCALLTYPE PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted);
    void STDMETHODCALLTYPE ModeChange( BSTR ModeName, VARIANT_BOOL NewValue);
    void STDMETHODCALLTYPE MediaError( IDispatch * pMediaObject);
    void STDMETHODCALLTYPE OpenPlaylistSwitch( IDispatch *pItem );
    void STDMETHODCALLTYPE DomainChange( BSTR strDomain);
    void STDMETHODCALLTYPE SwitchedToPlayerApplication();
    void STDMETHODCALLTYPE SwitchedToControl();
    void STDMETHODCALLTYPE PlayerDockedStateChange();
    void STDMETHODCALLTYPE PlayerReconnect();
    void STDMETHODCALLTYPE Click( short nButton, short nShiftState, long fX, long fY );
    void STDMETHODCALLTYPE DoubleClick( short nButton, short nShiftState, long fX, long fY );
    void STDMETHODCALLTYPE KeyDown( short nKeyCode, short nShiftState );
    void STDMETHODCALLTYPE KeyPress( short nKeyAscii );
    void STDMETHODCALLTYPE KeyUp( short nKeyCode, short nShiftState );
    void STDMETHODCALLTYPE MouseDown( short nButton, short nShiftState, long fX, long fY );
    void STDMETHODCALLTYPE MouseMove( short nButton, short nShiftState, long fX, long fY );
    void STDMETHODCALLTYPE MouseUp( short nButton, short nShiftState, long fX, long fY );

	// Helper method for IPropertyBag
	IStream* StreamFromResource(int id)
	{
		HMODULE hModule;
		HRSRC hrsrc = NULL;
		int len = 0;
		HGLOBAL hres = NULL;
		LPVOID pres = NULL;
		IStream* pstm = NULL;

		hModule = GetModuleHandle("MMSwitch.ax");			
		hrsrc = FindResource(hModule, MAKEINTRESOURCE(id), "RAW");
		len = SizeofResource(hModule, hrsrc);
		hres = LoadResource(hModule, hrsrc);
		pres = LockResource(hres);
		if (pres)
		{
			HGLOBAL hmem = GlobalAlloc(GMEM_FIXED, len);
			BYTE* pmem = (BYTE*)GlobalLock(hmem);

			if (pmem)
			{
				memcpy(pmem, pres, len);
				CreateStreamOnHGlobal(hmem, FALSE, &pstm);
			}
		
			DeleteObject(hres);
			DeleteObject(hrsrc);
		}

		if (hres) DeleteObject(hres);
		if (hrsrc) DeleteObject(hrsrc);

		return pstm;
	}

	// IPropertyBag methods
	STDMETHODIMP Read(LPCOLESTR pszPropName,VARIANT *pVar, IErrorLog *pErrorLog)
	{
		if (lstrcmpW(pszPropName, L"IconStreams") == 0)
		{
			HMODULE hModule = GetModuleHandle("MMSwitch.ax");			
#define ANIMATE
#ifdef ANIMATE
			SAFEARRAYBOUND sab;
			sab.cElements = 4;
			sab.lLbound = 0;
			SAFEARRAY *psa = SafeArrayCreate(VT_UNKNOWN, 1, &sab);

			if (psa)
			{
				long rgIndices;
				for (rgIndices = 0; rgIndices < 4; rgIndices++)
					SafeArrayPutElement(psa, &rgIndices, StreamFromResource(IDB_ICONSTREAM0 + rgIndices));
			}

			pVar->punkVal = (IUnknown *)psa;
 			pVar->vt = VT_ARRAY;

#else
			pVar->punkVal = StreamFromResource(IDB_ICONSTREAM3);
 			pVar->vt = VT_UNKNOWN;
#endif
			if (pVar->punkVal != NULL)
				return S_OK;
			else
				return E_FAIL;
		}
		return E_NOTIMPL;
	}

	STDMETHODIMP Write(LPCOLESTR pszPropName, VARIANT *pVar) 
		{return E_NOTIMPL;}

    TCHAR        m_szPluginText[MAX_PATH];

private:
    void         ReleaseCore();

    CComPtr<IWMPCore>           m_spCore;
    CComPtr<IConnectionPoint>   m_spConnectionPoint;
    DWORD                       m_dwAdviseCookie;
};

#endif //__MMSWITCH_H_
