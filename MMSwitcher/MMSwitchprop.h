//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1992 - 1998  Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;
//----------------------------------------------------------------------------
// MMSwitchprop.h
//----------------------------------------------------------------------------

class SwitcherIPProperties : public CBasePropertyPage
{

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    DECLARE_IUNKNOWN;

	HWND GetWindow() {return m_Dlg;}
	ISwitcher* GetISwitcher() {return m_pISwitcher;}
	void SetDefaultLCID(LCID lcid) {m_defLCID = lcid;}
	LCID GetDefaultLCID() {return m_defLCID;}
    void SetDirty();

private:

    BOOL OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnApplyChanges();

    SwitcherIPProperties(LPUNKNOWN lpunk, HRESULT *phr);

    ISwitcher    *m_pISwitcher;    // Switcher In Place property interface
	LCID m_defLCID;

};  // class SwitcherIPProperties

