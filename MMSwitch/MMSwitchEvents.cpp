/////////////////////////////////////////////////////////////////////////////
//
// MMSwitchEvents.cpp : Implementation of CMMSwitch events
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MMSwitch.h"
#include "..\MMSwitcher\iMMSwitch.h"

void CMMSwitch::OpenStateChange( long NewState )
{
    switch (NewState)
    {
    case wmposUndefined:
        break;
	case wmposPlaylistChanging:
        break;
	case wmposPlaylistLocating:
        break;
	case wmposPlaylistConnecting:
        break;
	case wmposPlaylistLoading:
        break;
	case wmposPlaylistOpening:
        break;
	case wmposPlaylistOpenNoMedia:
        break;
	case wmposPlaylistChanged:
        break;
	case wmposMediaChanging:
        break;
	case wmposMediaLocating:
        break;
	case wmposMediaConnecting:
        break;
	case wmposMediaLoading:
        break;
	case wmposMediaOpening:
        break;
	case wmposMediaOpen:
        break;
	case wmposBeginCodecAcquisition:
        break;
	case wmposEndCodecAcquisition:
        break;
	case wmposBeginLicenseAcquisition:
        break;
	case wmposEndLicenseAcquisition:
        break;
	case wmposBeginIndividualization:
        break;
	case wmposEndIndividualization:
        break;
	case wmposMediaWaiting:
        break;
	case wmposOpeningUnknownURL:
        break;
    default:
        break;
    }
}

void CMMSwitch::PlayStateChange( long NewState )
{
    switch (NewState)
    {
    case wmppsUndefined:
        break;
	case wmppsStopped:
        break;
	case wmppsPaused:
        break;
	case wmppsPlaying:
        break;
	case wmppsScanForward:
        break;
	case wmppsScanReverse:
        break;
	case wmppsBuffering:
        break;
	case wmppsWaiting:
        break;
	case wmppsMediaEnded:
        break;
	case wmppsTransitioning:
        break;
	case wmppsReady:
        break;
	case wmppsReconnecting:
        break;
	case wmppsLast:
        break;
    default:
        break;
    }
}

void CMMSwitch::AudioLanguageChange( long LangID )
{
}

void CMMSwitch::StatusChange()
{
}

void CMMSwitch::ScriptCommand( BSTR scType, BSTR Param )
{
}

void CMMSwitch::NewStream()
{
}

void CMMSwitch::Disconnect( long Result )
{
}

void CMMSwitch::Buffering( VARIANT_BOOL Start )
{
}

void CMMSwitch::Error()
{
    CComPtr<IWMPError>      spError;
    CComPtr<IWMPErrorItem>  spErrorItem;
    HRESULT                 dwError = S_OK;
    HRESULT                 hr = S_OK;

    if (m_spCore)
    {
        hr = m_spCore->get_error(&spError);

        if (SUCCEEDED(hr))
        {
            hr = spError->get_item(0, &spErrorItem);
        }

        if (SUCCEEDED(hr))
        {
            hr = spErrorItem->get_errorCode( (long *) &dwError );
        }
    }
}

void CMMSwitch::Warning( long WarningType, long Param, BSTR Description )
{
}

void CMMSwitch::EndOfStream( long Result )
{
}

void CMMSwitch::PositionChange( double oldPosition, double newPosition)
{
}

void CMMSwitch::MarkerHit( long MarkerNum )
{
}

void CMMSwitch::DurationUnitChange( long NewDurationUnit )
{
}

void CMMSwitch::CdromMediaChange( long CdromNum )
{
}

void CMMSwitch::PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change )
{
    switch (change)
    {
    case wmplcUnknown:
        break;
	case wmplcClear:
        break;
	case wmplcInfoChange:
        break;
	case wmplcMove:
        break;
	case wmplcDelete:
        break;
	case wmplcInsert:
        break;
	case wmplcAppend:
        break;
	case wmplcPrivate:
        break;
	case wmplcNameChange:
        break;
	case wmplcMorph:
        break;
	case wmplcSort:
        break;
	case wmplcLast:
        break;
    default:
        break;
    }
}

void CMMSwitch::CurrentPlaylistChange( WMPPlaylistChangeEventType change )
{
    switch (change)
    {
    case wmplcUnknown:
        break;
	case wmplcClear:
        break;
	case wmplcInfoChange:
        break;
	case wmplcMove:
        break;
	case wmplcDelete:
        break;
	case wmplcInsert:
        break;
	case wmplcAppend:
        break;
	case wmplcPrivate:
        break;
	case wmplcNameChange:
        break;
	case wmplcMorph:
        break;
	case wmplcSort:
        break;
	case wmplcLast:
        break;
    default:
        break;
    }
}

void CMMSwitch::CurrentPlaylistItemAvailable( BSTR bstrItemName )
{
}

void CMMSwitch::MediaChange( IDispatch * Item )
{
	{
		HRESULT hr;
		CComPtr<IWMPMedia> spMedia;

		// Get a pointer to the current media item.
		/*hr = m_spCore->get_currentMedia(&spMedia);
		if (FAILED(hr) || !spMedia)
		    return;*/

		if (!Item)
			return;

		CComPtr<IWMPSettings> spSettings;
		hr = m_spCore->get_settings(&spSettings);
		if (SUCCEEDED(hr) && spSettings)
		{
			// Get a pointer to IWMPSettings2 interface.
			CComPtr<IWMPSettings2> spSettings2;
			hr = spSettings->QueryInterface(__uuidof(IWMPSettings2), (void **)&spSettings2);
			if (SUCCEEDED(hr) && spSettings2)
			{
				VARIANT_BOOL ok = 0;
				hr = spSettings2->requestMediaAccessRights(L"full", &ok);
			}
		}

		CComBSTR bstrSourceURL;
		spMedia = (IWMPMedia*)Item;
		spMedia->get_sourceURL(&bstrSourceURL);

		HMODULE hModule = GetModuleHandle("MMSwitch.ax");
		PGetInstance pGetInstance = (PGetInstance)GetProcAddress(hModule, "GetInstance");

		if (!pGetInstance)
			return;

		ISwitcher* pISwitcher = pGetInstance(bstrSourceURL);

		if (!pISwitcher)
			return;

		pISwitcher->AddRef();

		CComBSTR bstrType;
		CComBSTR bstrTitle;
		CComBSTR bstrArtist;
		CComBSTR bstrCopyright;
		CComBSTR bstrRating;
		CComBSTR bstrMoreinfo;
		CComBSTR bstrAbstract;

		// Get the number of attributes for the current media. 
		/*long count;
		spMedia->get_attributeCount(&count);

		// Loop through the attribute list.
		for(i=0; i < count; i++)
		{
			spMedia->getAttributeName(i, &bstrType);
			//MessageBoxW(0, bstrType, L"", 0);
   		}*/

		bstrType = _T("Title");
		//bstrType = _T("Album");
		HRESULT hrTitle = pISwitcher->get_Title(&bstrTitle);
		//spMedia->setItemInfo(bstrType, bstrTitle);
		hr = spMedia->put_name(bstrTitle);
		
		bstrType = _T("WM/AlbumTitle");
		//hr = spMedia->setItemInfo(bstrType, bstrTitle);

		bstrType = _T("Artist");
		pISwitcher->get_AuthorName(&bstrArtist);
		//hr = spMedia->setItemInfo(bstrType, bstrArtist);

		bstrType = _T("Copyright");
		pISwitcher->get_Copyright(&bstrCopyright);
		//hr = spMedia->setItemInfo(bstrType, bstrCopyright);

		bstrType = _T("Rating");
		//bstrType = _T("WM/"WM/ProviderRating"");
		pISwitcher->get_Rating(&bstrRating);
		//hr = spMedia->setItemInfo(bstrType, bstrRating);

		if (hrTitle == S_OK)
		{
			bstrType = _T("Moreinfo");
			bstrMoreinfo = _T("http://www.imdb.com/Find?for=\"");
			bstrMoreinfo += (BSTR)bstrTitle;
			bstrMoreinfo += _T("\"");
			//hr = spMedia->setItemInfo(bstrType, bstrMoreinfo);
		}

		bstrType = _T("Abstract");
		pISwitcher->get_Description(&bstrAbstract);
		//pISwitcher->get_sourceURL(&bstrAbstract);
		//hr = spMedia->setItemInfo(bstrType, bstrAbstract);

		// Get a pointer to the current playlist item.
		CComPtr<IWMPPlaylist> spPlaylist;
		hr = m_spCore->get_currentPlaylist(&spPlaylist);
		if (SUCCEEDED(hr) && spPlaylist)
		{
			// Get the number of attributes for the current media. 
			/*long count;
			spPlaylist->get_attributeCount(&count);

			// Loop through the attribute list.
			for(i=0; i < count; i++)
			{
				spPlaylist->get_attributeName(i, &bstrType);
				//MessageBoxW(0, bstrType, L"", 0);
   			}*/

			bstrType = _T("Title");
			hr = spPlaylist->setItemInfo(bstrType, bstrTitle);

			bstrType = _T("Author");
			hr = spPlaylist->setItemInfo(bstrType, bstrArtist);

			bstrType = _T("Copyright");
			hr = spPlaylist->setItemInfo(bstrType, bstrCopyright);

			bstrType = _T("Rating");
			hr = spPlaylist->setItemInfo(bstrType, bstrRating);

			bstrType = _T("Moreinfo");
			hr = spPlaylist->setItemInfo(bstrType, bstrMoreinfo);

			bstrType = _T("Abstract");
			hr = spPlaylist->setItemInfo(bstrType, bstrAbstract);
							
		}
		
		// Get a pointer to the current control item.
		CComPtr<IWMPControls> spControls;
		hr = m_spCore->get_controls(&spControls);
		if (SUCCEEDED(hr) && spControls)
		{
			// Get a pointer to IWMPControls3 interface.
			CComPtr<IWMPControls3> spControls3;
			hr = spControls->QueryInterface(__uuidof(IWMPControls3), (void **)&spControls3);
			if (SUCCEEDED(hr) && spControls3)
			{
				long lIndex = 0;
				hr = spControls3->get_currentAudioLanguageIndex(&lIndex);
				if (SUCCEEDED(hr) && lIndex)
				{
					long lNewIndex = pISwitcher->GetActiveTrack() + 1;
					if (lIndex != lNewIndex)
						spControls3->put_currentAudioLanguageIndex(lNewIndex);
				}
			}
		}

		pISwitcher->Release();
	}
}

void CMMSwitch::CurrentMediaItemAvailable( BSTR bstrItemName )
{
}

void CMMSwitch::CurrentItemChange( IDispatch *pdispMedia)
{
}

void CMMSwitch::MediaCollectionChange()
{
}

void CMMSwitch::MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal )
{
}

void CMMSwitch::MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal )
{
}

void CMMSwitch::MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal)
{
}

void CMMSwitch::PlaylistCollectionChange()
{
}

void CMMSwitch::PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName)
{
}

void CMMSwitch::PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName)
{
}

void CMMSwitch::PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted)
{
}

void CMMSwitch::ModeChange( BSTR ModeName, VARIANT_BOOL NewValue)
{
}

void CMMSwitch::MediaError( IDispatch * pMediaObject)
{
}

void CMMSwitch::OpenPlaylistSwitch( IDispatch *pItem )
{
}

void CMMSwitch::DomainChange( BSTR strDomain)
{
}

void CMMSwitch::SwitchedToPlayerApplication()
{
}

void CMMSwitch::SwitchedToControl()
{
}

void CMMSwitch::PlayerDockedStateChange()
{
}

void CMMSwitch::PlayerReconnect()
{
}

void CMMSwitch::Click( short nButton, short nShiftState, long fX, long fY )
{
}

void CMMSwitch::DoubleClick( short nButton, short nShiftState, long fX, long fY )
{
}

void CMMSwitch::KeyDown( short nKeyCode, short nShiftState )
{
}

void CMMSwitch::KeyPress( short nKeyAscii )
{
}

void CMMSwitch::KeyUp( short nKeyCode, short nShiftState )
{
}

void CMMSwitch::MouseDown( short nButton, short nShiftState, long fX, long fY )
{
}

void CMMSwitch::MouseMove( short nButton, short nShiftState, long fX, long fY )
{
}

void CMMSwitch::MouseUp( short nButton, short nShiftState, long fX, long fY )
{
}
