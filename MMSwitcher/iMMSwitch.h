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
// iSwitcher.h
//----------------------------------------------------------------------------

// A custom interface to allow the user select media types

#ifndef __INULLIP__
#define __INULLIP__

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// ISwitcherIP's GUID
//
// {D3CD785A-971A-4838-ACEC-40CA5D529DC8}
DEFINE_GUID(IID_ISwitcher,
0xd3cd785A, 0x971a, 0x4838, 0xac, 0xec, 0x40, 0xca, 0x5d, 0x52, 0x9d, 0xc8);

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ISwitcher
//----------------------------------------------------------------------------
DECLARE_INTERFACE_(ISwitcher, IUnknown)
{
    // Basic methods
    STDMETHOD(Switch) (THIS_) PURE;
    STDMETHOD(SwitchTo) (THIS_ int n) PURE;
    STDMETHOD(GetTrackName) (THIS_ int n, char *pszName) PURE;
    STDMETHOD(SetTrackName) (THIS_ int n, char *pszName) PURE;
	STDMETHOD(GetTrackLCID)(THIS_ int n, LCID *plcid) PURE;
	STDMETHOD(SetTrackLCID)(THIS_ int n, LCID lcid) PURE;
	STDMETHOD(GetDefTrackIndex) (THIS_ long* lIndex) PURE;
	STDMETHOD(SetDefTrackIndex) (THIS_ long lIndex) PURE;
    STDMETHOD(GetActiveTrack) (THIS_) PURE;
    // Extended methods
	STDMETHOD(get_MediaProperty) (THIS_ BSTR bstrInfo, BSTR* pbstrValue) PURE;
    STDMETHOD(get_sourceURL) (THIS_ BSTR *pbstrSourceURL) PURE;
    // IAMMediaContent like methods
    STDMETHOD(get_AuthorName)(THIS_ BSTR FAR* pbstrAuthorName) PURE;
    STDMETHOD(get_Title)(THIS_ BSTR FAR* pbstrTitle) PURE;
    STDMETHOD(get_Rating)(THIS_ BSTR FAR* pbstrRating) PURE;
    STDMETHOD(get_Description)(THIS_ BSTR FAR* pbstrDescription) PURE;
    STDMETHOD(get_Copyright)(THIS_ BSTR FAR* pbstrCopyright) PURE;
    STDMETHOD(get_BaseURL)(THIS_ BSTR FAR* pbstrBaseURL) PURE;
    STDMETHOD(get_LogoURL)(THIS_ BSTR FAR* pbstrLogoURL) PURE;
    STDMETHOD(get_LogoIconURL)(THIS_ BSTR FAR* pbstrLogoURL) PURE;
    STDMETHOD(get_WatermarkURL)(THIS_ BSTR FAR* pbstrWatermarkURL) PURE;
    STDMETHOD(get_MoreInfoURL)(THIS_ BSTR FAR* pbstrMoreInfoURL) PURE;
    STDMETHOD(get_MoreInfoBannerImage)(THIS_ BSTR FAR* pbstrMoreInfoBannerImage) PURE;
    STDMETHOD(get_MoreInfoBannerURL)(THIS_ BSTR FAR* pbstrMoreInfoBannerURL) PURE;
    STDMETHOD(get_MoreInfoText)(THIS_ BSTR FAR* pbstrMoreInfoText) PURE;
};
//----------------------------------------------------------------------------

typedef ISwitcher** (WINAPI *PGetInstanceList)();
typedef ISwitcher* (WINAPI *PGetInstance)(BSTR bstrSourceURL);

#ifdef __cplusplus
}
#endif

#endif // __INULLIP__
