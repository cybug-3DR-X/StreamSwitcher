#include <streams.h>          // quartz, includes windows

// Eliminate two expected level 4 warnings from the Microsoft compiler.
// The class does not have an assignment or copy operator, and so cannot
// be passed by value.  This is normal.  This file compiles clean at the
// highest (most picky) warning level (-W4).
#pragma warning(disable: 4511 4512)

#include <measure.h>          // performance measurement (MSR_)

#include <initguid.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif
#include <stdio.h>
#define SWITCHER_MAX_PINS 1000

#include <qnetwork.h>
#include "wmp.h"

MIDL_INTERFACE("3A0DAA30-908D-4789-BA87-AED879B5C49B")
IWMPWindowMessageSink : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE OnWindowMessage( 
        UINT uMsg,
        WPARAM wparam,
        LPARAM lparam,
        LRESULT *plRet,
        BOOL *pfHandled) = 0;
    
};

#include "AVIRIFF.h"

#include "MMSwitchUids.h"         // our own public guids

#include "iMMSwitch.h"            // interface between filter and property sheet
#include "MMSwitchprop.h"         // property sheet implementatino class
#include "resource.h"

// ----------------------------------------------------------------------------
// Class definitions of input pin, output pin and filter
// ----------------------------------------------------------------------------


class CSwitcherInPlaceInputPin : public CTransInPlaceInputPin
{
    public:

	    CCritSec m_csReceive;
        
		CSwitcherInPlaceInputPin( TCHAR *pObjectName
                            , CTransInPlaceFilter *pTransInPlaceFilter
                            , HRESULT * phr
                            , LPCWSTR pName
							, LCID lcid
                            )
                            : CTransInPlaceInputPin( pObjectName
                                                     , pTransInPlaceFilter
                                                     , phr
                                                     , pName
                                                     ),
							m_lcid(lcid)
        {
			strcpy(m_szName, pObjectName);
        }

		STDMETHODIMP Receive(IMediaSample * pSample);
		STDMETHODIMP Redirect(IMediaSample * pSample);
        HRESULT CheckMediaType(const CMediaType* pmt);
		STDMETHODIMP ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt);
		HRESULT CompleteConnect(IPin *pReceivePin);

		STDMETHODIMP QueryId(LPWSTR *Id)
			{ return CBasePin::QueryId(Id);};

		char m_szName[MAX_PATH];
		LCID m_lcid;

        HRESULT SetName(WCHAR *pName)
		{
			delete[] m_pName;
			if (pName) {
				DWORD nameLen = lstrlenW(pName)+1;
				m_pName = new WCHAR[nameLen];
				if (m_pName) {
					CopyMemory(m_pName, pName, nameLen*sizeof(WCHAR));
				}
			}
			WideCharToMultiByte(CP_ACP, 0, m_pName, -1, m_szName, MAX_PATH, NULL, NULL);
			return S_OK;
		}

        HRESULT SetName(char *pName)
		{
			delete[] m_pName;
			if (pName) {
				DWORD nameLen = strlen(pName)+1;
				m_pName = new WCHAR[nameLen];
				if (m_pName) {
					MultiByteToWideChar(CP_ACP, 0, pName, -1, m_pName, strlen(pName)+1);
				}
			}
			strcpy(m_szName, pName);
			return S_OK;
		}
};

class CSwitcherInPlaceOutputPin : public CTransInPlaceOutputPin
{
    public:
        CSwitcherInPlaceOutputPin( TCHAR *pObjectName
                             , CTransInPlaceFilter *pTransInPlaceFilter
                             , HRESULT * phr
                             , LPCWSTR pName
                             )
                               : CTransInPlaceOutputPin( pObjectName
                                                       , pTransInPlaceFilter
                                                       , phr
                                                       , pName
                                                       )
        {
        }

        HRESULT CheckMediaType(const CMediaType* pmt);

		STDMETHODIMP QueryId(LPWSTR *Id)
			{ return CBasePin::QueryId(Id);};
};

// CMediaProperty : Describe a media property
//
class CMediaProperty
{
public:

	CMediaProperty(const BSTR t, const BSTR v) :
		tag(SysAllocString(t)),
		val(SysAllocString(v)) {
		var.vt = VT_BSTR;
		var.bstrVal = val;}

	~CMediaProperty() {
		SysFreeString(tag); SysFreeString(val);}

	const BSTR tag;
	const BSTR val;

	VARIANT var;
};
typedef CGenericList <CMediaProperty> CMediaPropertyList; // Define a list of media properties

// CSwitcherInPlace
//
class CSwitcherInPlace : public CTransInPlaceFilter
					, public IAMStreamSelect
					, public IAMMediaContent
					, public IPersistPropertyBag
					, public IPropertyBag
					, public IWMPWindowMessageSink
					, public ISpecifyPropertyPages
					, public ISwitcher
{

        friend class CSwitcherInPlaceInputPin;
        friend class CSwitcherInPlaceOutputPin;

		CBaseDispatch m_basedisp;

    public:

        static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

		/* IUnknown methods */
        DECLARE_IUNKNOWN;

		/* IDispatch methods */
		STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

		STDMETHODIMP GetTypeInfo(
		  UINT itinfo,
		  LCID lcid,
		  ITypeInfo ** pptinfo);

		STDMETHODIMP GetIDsOfNames(
		  REFIID riid,
		  OLECHAR  ** rgszNames,
		  UINT cNames,
		  LCID lcid,
		  DISPID * rgdispid);

		STDMETHODIMP Invoke(
		  DISPID dispidMember,
		  REFIID riid,
		  LCID lcid,
		  WORD wFlags,
		  DISPPARAMS * pdispparams,
		  VARIANT * pvarResult,
		  EXCEPINFO * pexcepinfo,
		  UINT * puArgErr);

        // CBaseFilter Override
		STDMETHODIMP JoinFilterGraph(
						IFilterGraph * pGraph,
						LPCWSTR pName)
		{
			// Exit Graph && Audio Track Name, LCID, or def Audio Track changed.
			if (!pGraph && m_bDirty)
			{
				// Save changes ?
				char szText[MAX_PATH];
				sprintf(szText, "At least one Audio Track Name, LCID, or Default Audio Track has been changed in\r\n%s.\r\n\r\nDo you want to save changes ?", m_SrcFileName);
				
				BOOL bSave = MessageBox(GetActiveWindow(), szText, "Question", MB_YESNO | MB_ICONQUESTION);
				if (bSave)
				{
					LCID lcid = 0;
					if (m_DefTrackIndex)
						GetTrackLCID(m_DefTrackIndex - 1, &lcid);
					sprintf(szText, "\"%s\" %d", m_SrcFileName, lcid);
					int nTrack = 0;
					while (GetTrackLCID(nTrack++, &lcid) == S_OK)
					{
						strcat(szText, " %d");
						sprintf(szText, szText, lcid);
					}
					//
					// Source File is still opened, create another process to patch it when
					// we and the process we belong to is died, so Source File is closed.
					//
					// Note: MMAVILNG.EXE will Sleep 1 sec. before patching the Source File, 
					//       giving us the time to rest in peace, and Source File to be closed.
					//
					//MessageBox(0, szText, "MMAVILNG.EXE", 0);
					ShellExecute(GetDesktopWindow(), NULL, "MMAVILNG.EXE", szText, NULL, 0);
				}
			}

			return CBaseFilter::JoinFilterGraph(pGraph, pName);
		}

        // CTransInPlaceFilter Overrides 
		virtual int GetPinCount();
        virtual CBasePin *GetPin( int n );

		STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin) 
			{ return CBaseFilter::FindPin(Id, ppPin);};

        HRESULT CheckInputType(const CMediaType* mtIn)
            { UNREFERENCED_PARAMETER(mtIn);  return S_OK; }

		CSwitcherInPlaceInputPin  *InputPin()
		{
			return (CSwitcherInPlaceInputPin *)m_pSelectedPin;
		};

		BOOL m_bStartStreaming;
		HRESULT StartStreaming();

        // Basic COM - used here to reveal our property interface.
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

		int m_DefTrackIndex;
		BOOL m_bDirty;
        //
        // --- ISwitcher ---
        //

        STDMETHODIMP Switch();
        STDMETHODIMP SwitchTo(int n);
        STDMETHODIMP GetTrackName(int n, char *pszName);
        STDMETHODIMP SetTrackName(int n, char *pszName);
		STDMETHODIMP GetTrackLCID(int n, LCID* plcid);
		STDMETHODIMP SetTrackLCID(int n, LCID lcid);
        STDMETHODIMP GetDefTrackIndex(long *lIndex) {if (lIndex) *lIndex = GetDefaultTrack(); return S_OK;}
        STDMETHODIMP SetDefTrackIndex(long lIndex) {m_DefTrackIndex = lIndex; m_bDirty = TRUE; return S_OK;}
        STDMETHODIMP GetActiveTrack();

        //
        // --- ISpecifyPropertyPages ---
        //

        STDMETHODIMP GetPages(CAUUID *pPages);

        //
		// --- IAMStreamSelect ---
        //

		BOOL m_bEnableMute;
		STDMETHODIMP Count(DWORD* pcStreams); 
		STDMETHODIMP Enable(long lIndex, DWORD dwFlags); 
		STDMETHODIMP Info(long lIndex, AM_MEDIA_TYPE** ppmt, DWORD* pdwFlags, LCID* plcid, DWORD* pdwGroup, WCHAR** ppszName, IUnknown** ppObject, IUnknown** ppUnk);  

		// IAMMediaContent methods
		STDMETHODIMP get_AuthorName(BSTR* pbstrAuthorName);
		STDMETHODIMP get_Title(BSTR* pbstrTitle);
		STDMETHODIMP get_Rating(BSTR* pbstrRating);
		STDMETHODIMP get_Description(BSTR* pbstrDescription);
		STDMETHODIMP get_Copyright(BSTR* pbstrCopyright);
		STDMETHODIMP get_BaseURL(BSTR* pbstrBaseURL);
		STDMETHODIMP get_LogoURL(BSTR* pbstrLogoURL);
		STDMETHODIMP get_LogoIconURL(BSTR* pbstrLogoIconURL); 
		STDMETHODIMP get_WatermarkURL(BSTR* pbstrWatermarkURL);
		STDMETHODIMP get_MoreInfoURL(BSTR* pbstrMoreInfoURL);
		STDMETHODIMP get_MoreInfoBannerImage(BSTR* pbstrMoreInfoBannerImage);
		STDMETHODIMP get_MoreInfoBannerURL(BSTR* pbstrMoreInfoBannerURL);
		STDMETHODIMP get_MoreInfoText(BSTR* pbstrMoreInfoText);

		STDMETHODIMP get_MediaProperty(BSTR bstrInfo, BSTR* pbstrValue);
		STDMETHODIMP get_sourceURL(BSTR* pbstrSourceURL);

		IStream* StreamFromResource(int id)
		{
			HMODULE hModule;
			HRSRC hrsrc = NULL;
			int len = 0;
			HGLOBAL hres = NULL;
			LPVOID pres = NULL;
			IStream* pstm = NULL;

			hModule = GetModuleHandle("MMSwitch.ax");			
			hrsrc = FindResource(hModule, MAKEINTRESOURCE(id), "STREAM");
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

		//
		// IWMPWindowMessageSink
		//
		STDMETHODIMP OnWindowMessage(
		  UINT  uMsg,
		  WPARAM  wparam,
		  LPARAM  lparam,
		  LRESULT*  plRet,
		  BOOL*  pfHandled) {return S_OK;}

		// IPersist
        STDMETHODIMP GetClassID(CLSID __RPC_FAR *pClassID)
		{
			return CBaseFilter::GetClassID(pClassID);
		}

		// IPersistPropertyBag methods 
		// WARNING !!! 
		// WARNING !!! Not exposed in NonDelegatingQueryInterface
		// WARNING !!! 
        STDMETHODIMP InitNew() 
		{
			return S_OK;
		}

        STDMETHODIMP Load(IPropertyBag *pPropBag, IErrorLog *pErrorLog)
		{
			return E_NOTIMPL;
		}

        STDMETHODIMP Save(IPropertyBag *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
		{
			if (m_MediaPropertyList.GetCount() == 0)
				return E_NOTIMPL;

			POSITION pos = m_MediaPropertyList.GetHeadPosition();
			while(pos)
			{
				CMediaProperty *pInfo = m_MediaPropertyList.GetNext(pos);

				pPropBag->Write(pInfo->tag, &pInfo->var);
			}
			return S_OK;
		}

		// IPropertyBag methods
		STDMETHODIMP Read(LPCOLESTR pszPropName,VARIANT *pVar, IErrorLog *pErrorLog)
		{
			if (lstrcmpW(pszPropName, L"IconStreams") == 0)
			{
				HMODULE hModule = GetModuleHandle("MMSwitch.ax");			
//#define ANIMATE
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

		// The input pins should support the IPropertyBag interface for setting/getting
		// properties on individual streams. 
		// "name" as a 'strn' chunk.  

        //
        // --- Input Pins ---
        //

		CSwitcherInPlaceInputPin *m_pMasterPin;
		CSwitcherInPlaceInputPin *m_pSelectedPin;
		CSwitcherInPlaceInputPin *m_pPreferredPin;
		CSwitcherInPlaceInputPin *m_pMutePin;
		BOOL m_bStartMuted;

	    typedef CGenericList <CSwitcherInPlaceInputPin> CInputList;

		INT m_NumInputPins;            // Current Input pin count
		CInputList m_InputPinsList;    // List of the Input pins
		INT m_NextInputPinNumber;      // Increases monotonically.

		void InitInputPinsList();
		BOOL m_bEnumerating;
		HRESULT LoadSourceInfo();
		HRESULT LoadPersistMediaPropertyBag();
		HRESULT FindMediaProperty(WCHAR* pszInfo, BSTR *pbstrValue = NULL);
		LCID PreparePinInfo(char* szName, WCHAR* wszName);
		int GetDefaultTrack();
		CSwitcherInPlaceInputPin *CreateNextInputPin(CSwitcherInPlace *pSwitch);
		CSwitcherInPlaceInputPin *GetPinNFromList(int n);
		int GetNumFreePins();
		int GetNumConnectedPins();

		char m_SrcFileName[MAX_PATH];
		BSTR m_bstrSrcURL;
		char m_SrcFileNameIni[MAX_PATH];
		unsigned long m_TotalIndex;

		CMediaPropertyList m_MediaPropertyList;

    private:

        CSwitcherInPlace(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
		~CSwitcherInPlace();

        //
        // Overrides the virtual Transform of CTransInPlaceFilter base class
        //
        HRESULT Transform(IMediaSample *pSample)
            { UNREFERENCED_PARAMETER(pSample); return NOERROR; }

        //
        // If there are multiple instances of this filter active, it's
        // useful for debug messages etc. to know which one this is.
        //

        CMediaType m_mtPreferred;  // Media type chosen from property sheet

        CCritSec m_SwitcherLock; // To serialise access.

        static m_nInstanceCount;   // total instances
        int m_nThisInstance;
#define MAX_INSTANCE 256
	public:
		static ISwitcher* m_InstanceList[MAX_INSTANCE];
};


// ----------------------------------------------------------------------------
// Implementation of pins and filter
// ----------------------------------------------------------------------------


//
// DbgFunc
//
// Put out the name of the function and instance on the debugger.
// Call this at the start of interesting functions to help debug
//
#ifdef DEBUG
#define DbgFunc(a) DbgLog(( LOG_TRACE                        \
                          , 2                                \
                          , TEXT("CSwitcherInPlace(Instance %d)::%s") \
                          , m_nThisInstance                  \
                          , TEXT(a)                          \
                         ));
#else
#define DbgFunc(a)
#endif

//
// setup data
//

const AMOVIESETUP_MEDIATYPE
//sudPinTypes =   { &MEDIATYPE_NULL                // clsMajorType
sudPinTypes =   { &MEDIATYPE_Audio              // clsMajorType
				, &MEDIASUBTYPE_PCM  }  ;       // clsMinorType
                //, &MEDIASUBTYPE_NULL }  ;       // clsMinorType


const AMOVIESETUP_PIN
psudPins[] = {{ L"Input"             // strName
               , FALSE               // bRendered
               , FALSE               // bOutput
               , FALSE               // bZero
               , TRUE		         // bMany
               , &CLSID_NULL         // clsConnectsToFilter
			   //, &CLSID_AviSplitter  // clsConnectsToFilter
               , L"Output"           // strConnectsToPin
               , 1                   // nTypes
               , &sudPinTypes }
			, { L"Output"           // strName
               , FALSE               // bRendered
               , TRUE                // bOutput
               , FALSE               // bZero
               , FALSE               // bMany
               , &CLSID_NULL         // clsConnectsToFilter
               , L"Input"            // strConnectsToPin
               , 1                   // nTypes
               , &sudPinTypes }
			};   // lpTypes


const AMOVIESETUP_FILTER
sudSwitcherIP = { &CLSID_SwitcherInPlace                 // clsID
            , L"Morgan Stream Switcher"                // strName
            //, MERIT_DO_NOT_USE                // dwMerit
			, MERIT_PREFERRED + 1			  // dwMerit
			//, MERIT_PREFERRED			  // dwMerit
            , 2                               // nPins
            , psudPins };                     // lpPin

//
// Needed for the CreateInstance mechanism
//
CFactoryTemplate g_Templates[]=
    {   {L"Switcher-In-Place"
        , &CLSID_SwitcherInPlace
        ,   CSwitcherInPlace::CreateInstance
        , NULL
        , &sudSwitcherIP }
    ,
        { L"Switcher IP Property Page"
        , &CLSID_SwitcherIPPropertyPage
        , SwitcherIPProperties::CreateInstance }

    };
int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);


//
// initialise the static instance count.
//
int CSwitcherInPlace::m_nInstanceCount = 0;
ISwitcher* CSwitcherInPlace::m_InstanceList[MAX_INSTANCE];

HRESULT CSwitcherInPlace::LoadPersistMediaPropertyBag()
{
	if (!m_pGraph)
		return S_FALSE;

    HRESULT hr;

    IEnumFilters* pEnum = NULL;
    IBaseFilter* pFilter = NULL;
	IPersistMediaPropertyBag* pPMPB = NULL;
    
	ULONG cFetched;

	m_bEnumerating = TRUE;
    hr = m_pGraph->EnumFilters(&pEnum);
	m_bEnumerating = FALSE;
    if(FAILED(hr)) 
		return S_FALSE;

    while (pEnum->Next(1, &pFilter, &cFetched) == S_OK)
    {
        hr = pFilter->QueryInterface(IID_IPersistMediaPropertyBag, (LPVOID*)&pPMPB);

        pFilter->Release();

        if (FAILED(hr))
			continue;

		ISwitcher* pMSS = NULL;
		hr = pFilter->QueryInterface(IID_ISwitcher, (LPVOID*)&pMSS);
		if (SUCCEEDED(hr))
		{
			// I'v find myself, Reject
			pMSS->Release();
			pPMPB->Release();
			continue;
		}

		IMediaPropertyBag *pPropBag;

		CoCreateInstance(CLSID_MediaPropertyBag, NULL, CLSCTX_INPROC,
				IID_IMediaPropertyBag, (LPVOID *)&pPropBag);

		if (pPropBag)
		{
		
			hr = pPMPB->Save(pPropBag, 0, 0);
			if (SUCCEEDED(hr))
			{
				VARIANT varPropertyName;
				VARIANT varPropertyValue;
				int i = 0;
				varPropertyName.vt = VT_BSTR;
				varPropertyValue.vt = VT_BSTR;
				while ((hr = pPropBag->EnumProperty(i++, &varPropertyName, &varPropertyValue)) == S_OK)
				{			
					CMediaProperty* pInfo = new CMediaProperty(varPropertyName.bstrVal, varPropertyValue.bstrVal);
					
					m_MediaPropertyList.AddTail(pInfo);

					VariantClear(&varPropertyName);
					VariantClear(&varPropertyValue);
				}

				pPropBag->Release();
			}
		}

		pPMPB->Release();
    }

	pEnum->Release();

    return S_OK;
}

HRESULT CSwitcherInPlace::FindMediaProperty(WCHAR* pszInfo, BSTR *pbstrValue)
{
	if (m_MediaPropertyList.GetCount() == 0)
		return VFW_E_NOT_FOUND;

	POSITION pos = m_MediaPropertyList.GetHeadPosition();
    while(pos)
	{
		CMediaProperty *pInfo = m_MediaPropertyList.GetNext(pos);

		if (lstrcmpW(pInfo->tag, pszInfo) == 0)
		{
			if (pbstrValue == NULL)
				lstrcpyW(pszInfo, pInfo->val);
			else
			{
				*pbstrValue = SysAllocString(pInfo->val);
				if (*pbstrValue == NULL)
					return E_OUTOFMEMORY;
			}
			return S_OK;
		}
	}

	return VFW_E_NOT_FOUND;
}

HRESULT CSwitcherInPlace::LoadSourceInfo()
{
	if (!m_pGraph)
		return S_FALSE;

    HRESULT hr;

    IEnumFilters* pEnum = NULL;
    IBaseFilter* pFilter = NULL;
	IFileSourceFilter* pFSF = NULL;
    
	ULONG cFetched;

	m_bEnumerating = TRUE;
    hr = m_pGraph->EnumFilters(&pEnum);
	m_bEnumerating = FALSE;
    if(FAILED(hr))
		return S_FALSE;

    while(pEnum->Next(1, &pFilter, &cFetched) == S_OK)
    {
        hr = pFilter->QueryInterface(IID_IFileSourceFilter, (LPVOID*)&pFSF);
        if(FAILED(hr))
        {
            pFilter->Release();
            continue;
        }

		LPOLESTR fnw = NULL;
		AM_MEDIA_TYPE mt;

		hr = pFSF->GetCurFile(&fnw, &mt);
        if(FAILED(hr) || fnw == NULL)
        {
			pFSF->Release();
            pFilter->Release();
            continue;
        }

		m_bstrSrcURL = fnw;
		WideCharToMultiByte(CP_ACP, 0, fnw, -1, m_SrcFileName, MAX_PATH, NULL, NULL);

		// Load AVI header
		IPin *pPin = NULL;
		pFilter->FindPin(L"Output", &pPin);
        if(SUCCEEDED(hr) && pPin)
		{
			IAsyncReader* pReader = NULL;
	        hr = pPin->QueryInterface(IID_IAsyncReader, (LPVOID*)&pReader);
			if(SUCCEEDED(hr) && pReader)
			{
				BYTE buf[4 * 1024];
				hr = pReader->SyncRead(0, sizeof(buf), (BYTE *)&buf);
				if(SUCCEEDED(hr))
				{
					// Retrieve all 'strh' (Stream Header)
					int nVideo = 0;
					int nAudio = 0;
					BOOL bAudio = FALSE;
					WORD wDefLanguage = 0;
					for (int i = 0; i < sizeof(buf); i++)
					{
						if (*(DWORD *)&buf[i] == ckidSTREAMHEADER)
						{
							AVISTREAMHEADER *strh;
							strh = (AVISTREAMHEADER *)&buf[i];
							// Video Language is Default Language
							if (strh->fccType == streamtypeVIDEO)
							{
								nVideo++;
								if (strh->wLanguage && nVideo == 1)
									wDefLanguage = strh->wLanguage;
							}
							// Audio Language
							else if (strh->fccType == streamtypeAUDIO)
							{
								nAudio++;
								// Strore Audio Language in Media Property List
								if (strh->wLanguage)
								{
									WCHAR tag[16];
									swprintf(tag, L"INFO/IAS%d", nAudio);
									
									WCHAR val[16];
									swprintf(val, L"0x%x", strh->wLanguage);
									
									CMediaProperty* pInfo = new CMediaProperty(tag, val);
									
									m_MediaPropertyList.AddTail(pInfo);

									bAudio = TRUE;
								}
							}
							/*else if (strh->fccType == streamtypeMIDI)
							{
							}
							else if (strh->fccType == streamtypeTEXT)
							{
							}*/
						}
					}
					if (bAudio && wDefLanguage)
					{
						int nIdx = 0;
						for (int i = 0; i < sizeof(buf); i++)
						{
							if (*(DWORD *)&buf[i] == ckidSTREAMHEADER)
							{
								AVISTREAMHEADER *strh;
								strh = (AVISTREAMHEADER *)&buf[i];
								if (strh->fccType == streamtypeAUDIO)
								{
									nIdx++;
									if (strh->wLanguage == wDefLanguage)
									{
										WCHAR val[16];
										swprintf(val, L"%d", nIdx);
										
										CMediaProperty* pInfo = new CMediaProperty(L"INFO/ICAS", val);
										
										m_MediaPropertyList.AddTail(pInfo);
										break;
									}
								}
							}
						}						
					}
				}
				pReader->Release();
			}
			pPin->Release();
		}

		pFSF->Release();
        pFilter->Release();
		pEnum->Release();

		return S_OK;
    }

    pEnum->Release();

    return S_FALSE;
}

//
// GetPin
//
// Override CBaseFilter method.
// return a non-addrefed CBasePin * for the user to addref if he holds onto it
// for longer than his pointer to us.  This is part of the implementation of
// EnumMediaTypes.  All attempts to refer to our pins from the outside have
// to come through here, so it's a valid place to create them.
//
CBasePin *CSwitcherInPlace::GetPin(int n)
{
    // Create the single input pin and the single output pin
    // If anything fails, fail the whole lot and clean up.

	if (!m_bEnumerating && m_SrcFileName[0] == NULL)
	{
		// Load AVI Stream Headers
		if (LoadSourceInfo() == S_OK)
		{
			strcpy(m_SrcFileNameIni, m_SrcFileName);
			int l = strlen(m_SrcFileNameIni);
			m_SrcFileNameIni[l-3] = 'm';
			m_SrcFileNameIni[l-2] = 's';
			m_SrcFileNameIni[l-1] = 's';

			// Load INFO chunk
			LoadPersistMediaPropertyBag();	
		}
	}

    if (m_pInput == NULL || m_pOutput == NULL) {

        HRESULT hr = S_OK;

		char szName[MAX_PATH];
		WCHAR wszName[MAX_PATH];
		LCID lcid = PreparePinInfo(szName, wszName);

        m_pMasterPin = new CSwitcherInPlaceInputPin(szName,  // Name (char*)
                                          this,              // Owner filter
                                          &hr,               // Result code
                                          wszName,			 // Pin Name (WCHAR*)
										  lcid);			 // Pin LCID

        // a failed return code should delete the object

        if (FAILED(hr) || m_pMasterPin == NULL) {
            delete m_pMasterPin;
            m_pMasterPin = NULL;
            //return NULL;
        }

		m_pInput = m_pMasterPin;
		m_pSelectedPin = m_pMasterPin;

        m_pOutput = new CSwitcherInPlaceOutputPin(NAME("Switcher output pin"),
                                            this,            // Owner filter
                                            &hr,             // Result code
                                            L"Output");      // Pin name

        // failed return codes cause both objects to be deleted

        if (FAILED(hr) || m_pOutput == NULL) {
            delete m_pInput;
            m_pInput = NULL;
            delete m_pOutput;
            m_pOutput = NULL;
            //return NULL;
        }

		// Init other coming Input pin at this time
		InitInputPinsList();
    }

    /* Find which pin is required */
    if (n == 0)
		return m_pInput;

    if (n == 1)
		return m_pOutput;
			
	return GetPinNFromList(n - 2);

	return NULL;
} // GetPin

static IPin* FindFirstPin(IBaseFilter* pBF)
{
    IEnumPins *pEnum = NULL;
    IPin *pPin = NULL;
    ULONG cFetched;

    HRESULT hr = pBF->EnumPins(&pEnum);
    if(FAILED(hr)) return NULL;

    pEnum->Next(1, &pPin, &cFetched);
    pEnum->Release();

    return(pPin);
}

// ----------------------------------------------------------------------------
//            Input pin implementation
// ----------------------------------------------------------------------------

// 
// Overiden to know who's trying to connect to me ...
//
STDMETHODIMP CSwitcherInPlaceInputPin::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{ CSwitcherInPlace *pSwitcher = (CSwitcherInPlace *) m_pTIPFilter;

	HRESULT hr;
	PIN_INFO pi;
	hr = pConnector->QueryPinInfo(&pi);
	if (FAILED(hr))
		return hr;

	if (pi.pFilter)
	{
		ISwitcher* pMSS = NULL;
		hr = pi.pFilter->QueryInterface(IID_ISwitcher, (LPVOID*)&pMSS);
		if (SUCCEEDED(hr))
		{
			// Filter Graph Manager is trying to connet me to myself.
			// Reject connection.
			pi.pFilter->Release();
			pMSS->Release();
			return E_INVALIDARG;
		}

		pi.pFilter->Release();

	}

    hr = CBaseInputPin::ReceiveConnection(pConnector, pmt);
  	return hr;
}

// CheckMediaType
//
// Override CTransInPlaceInputPin method.
// If we have been given a preferred media type from the property sheet
// then only accept a type that is exactly that.
// else if there is nothing downstream, then accept anything
// else if there is a downstream connection then first check to see if
// the subtype (and implicitly the major type) are different from the downstream
// connection and if they are different, fail them
// else ask the downstream input pin if the type (i.e. all details of it)
// are acceptable and take that as our answer.
//
HRESULT CSwitcherInPlaceInputPin::CheckMediaType( const CMediaType *pmt )
{   CSwitcherInPlace *pSwitcher = (CSwitcherInPlace *) m_pTIPFilter;

#ifdef DEBUG
    DisplayType(TEXT("Input type proposed"),pmt);
#endif

	if (pmt->majortype == MEDIATYPE_Audio &&
		pmt->subtype == MEDIASUBTYPE_PCM)
        return S_OK;
	else
        return VFW_E_TYPE_NOT_ACCEPTED;

/*    if (pSwitcher->m_mtPreferred.IsValid() == FALSE)
    {
        if( pSwitcher->m_pOutput->IsConnected() ) {

            return pSwitcher->m_pOutput->GetConnected()->QueryAccept( pmt );
        }
        return S_OK;
    }
    else
        if (*pmt == pSwitcher->m_mtPreferred)
            return S_OK  ;
        else
            return VFW_E_TYPE_NOT_ACCEPTED;*/

}

//
// CompleteConnect
//
HRESULT CSwitcherInPlaceInputPin::CompleteConnect(IPin *pReceivePin)
{   CSwitcherInPlace *pSwitcher = (CSwitcherInPlace *) m_pTIPFilter;

    CAutoLock lock_it(m_pLock);
    ASSERT(m_Connected == pReceivePin);
    HRESULT hr = NOERROR;

    hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
        return hr;

    // Since this pin has been connected up, create another Input pin. We
    // will do this only if there are no unconnected pins on us. However
    // CompleteConnect will get called for the same pin during reconnection

    int n = pSwitcher->GetNumFreePins();
    //ASSERT(n <= 1);
    if (n == 1 || pSwitcher->m_NumInputPins == SWITCHER_MAX_PINS)
        return NOERROR;

    // No unconnected pins left so spawn a new one

    CSwitcherInPlaceInputPin *pInputPin = pSwitcher->CreateNextInputPin(pSwitcher);
    if (pInputPin != NULL )
    {
        pSwitcher->m_NumInputPins++;
        pSwitcher->m_InputPinsList.AddTail(pInputPin);
		pSwitcher->IncrementPinVersion();
    }

	if (pSwitcher->m_SrcFileName[0])
	{
		int nPin = pSwitcher->GetDefaultTrack();
		if (nPin == 1)
			pSwitcher->m_pPreferredPin = pSwitcher->m_pMasterPin;
		else
			pSwitcher->m_pPreferredPin = pSwitcher->GetPinNFromList(nPin - 2);

		pSwitcher->m_pSelectedPin = pSwitcher->m_pPreferredPin;
	}

    return NOERROR;

} // CompleteConnect

HRESULT
CSwitcherInPlaceInputPin::Receive(IMediaSample * pSample)
{CSwitcherInPlace *pSwitcher = (CSwitcherInPlace *) m_pTIPFilter;

	// Is "Mute" default and selected pin, and am I the Master pin ?
	if (pSwitcher->m_bStartMuted &&
		pSwitcher->m_pSelectedPin == pSwitcher->m_pMutePin &&
		this == pSwitcher->m_pMasterPin)
	{
		// Send at least one sample to the audio renderer to make it start
		REFERENCE_TIME tStart = 0, tStop = 1;
		pSample->SetTime(&tStart, &tStop); // min time
		pSample->SetActualDataLength(1); // min data

		// Next time we'll return E_NOINTERFACE ...
		pSwitcher->m_bStartMuted = FALSE;

		return pSwitcher->m_pMasterPin->Redirect(pSample);
	}
	// Is it me selected pin ?
	else if (this == pSwitcher->m_pSelectedPin)
	{
		// Yes, redirect the sample I receive to the Master pin
		return pSwitcher->m_pMasterPin->Redirect(pSample);
	}
	else
		// No, tell the upstream filter we can't support Receive
		return E_NOINTERFACE;
}

// here's the next block of data from the stream.
// AddRef it yourself if you need to hold it beyond the end
// of this call.

HRESULT
CSwitcherInPlaceInputPin::Redirect(IMediaSample * pSample)
{CSwitcherInPlace *pSwitcher = (CSwitcherInPlace *) m_pTIPFilter;

    CAutoLock lck(&pSwitcher->m_csReceive);
    ASSERT(pSample);
    HRESULT hr;

    // check all is well with the base class
    hr = CBaseInputPin::Receive(pSample);
    if (S_OK == hr) {
		// send the sample to our filter
        hr = pSwitcher->Receive(pSample);
    }
    return hr;
}

HRESULT
CSwitcherInPlace::StartStreaming()
{
	if (m_pMutePin == NULL)
		m_pMutePin = m_InputPinsList.Get(m_InputPinsList.GetTailPosition());
	m_bStartMuted = m_pPreferredPin == m_pMutePin;
	m_bStartStreaming = TRUE;

	return CTransInPlaceFilter::StartStreaming();
}

// ----------------------------------------------------------------------------
//            Output pin implementation
// ----------------------------------------------------------------------------


// CheckMediaType
//
// Override CTransInPlaceOutputPin method.
// If we have ben given a media type from the property sheet, then insist on
// exactly that, else pass the request up to the base class implementation.
//
HRESULT CSwitcherInPlaceOutputPin::CheckMediaType( const CMediaType *pmt )
{   CSwitcherInPlace *pSwitcher = (CSwitcherInPlace *) m_pTIPFilter;

    if (pSwitcher->m_mtPreferred.IsValid() == FALSE)
    {
        return CTransInPlaceOutputPin::CheckMediaType (pmt) ;
    }
    else
        if (*pmt == pSwitcher->m_mtPreferred)
            return S_OK;
        else
            return VFW_E_TYPE_NOT_ACCEPTED;

}


// ----------------------------------------------------------------------------
//            Filter implementation
// ----------------------------------------------------------------------------


//
// CSwitcherInPlace::Constructor
//
CSwitcherInPlace::CSwitcherInPlace(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
    : 
    m_InputPinsList(NAME("Switcher Input Pins list")),
	m_MediaPropertyList(NAME("Switcher Info Tag list"), TRUE, 255),
    m_NumInputPins(0),
    m_NextInputPinNumber(0),
	CTransInPlaceFilter (tszName, punk, CLSID_SwitcherInPlace, phr, false /* bModifiesData */),
	m_bStartStreaming(FALSE),
	m_bStartMuted(FALSE),
	m_bEnableMute(FALSE), //(TRUE),
	m_TotalIndex(0),
	m_DefTrackIndex(-1),
	m_bDirty(FALSE),
	m_bEnumerating(FALSE)
{
	if (m_nInstanceCount == 0)
		ZeroMemory(&m_InstanceList[0], MAX_INSTANCE * sizeof(m_InstanceList[0]));
	m_nThisInstance = m_nInstanceCount++;
	m_InstanceList[m_nThisInstance] = (ISwitcher*)this;

    m_mtPreferred.InitMediaType();

	m_pMasterPin = NULL;
	m_pSelectedPin = NULL;
	m_pPreferredPin = NULL;
	m_pMutePin = NULL;

	m_SrcFileName[0] = NULL;

    DbgFunc("CSwitcherInPlace");


} // (CSwitcherInPlace constructor)


CSwitcherInPlace::~CSwitcherInPlace()
{
    // Delete the pins
    InitInputPinsList();

	m_InstanceList[m_nThisInstance] = NULL;
	if (m_nThisInstance == m_nInstanceCount)
		m_nInstanceCount--;
}

//
// CreateInstance
//
// Override CClassFactory method.
// Provide the way for COM to create a CSwitcherInPlace object
//
CUnknown * WINAPI CSwitcherInPlace::CreateInstance(LPUNKNOWN punk, HRESULT *phr) {

	CSwitcherInPlace *pNewObject = new CSwitcherInPlace(NAME("Switcher-In-Place Filter"), punk, phr );
    if (pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;

} // CreateInstance


//
// InitInputPinsList
//
void CSwitcherInPlace::InitInputPinsList()
{
    POSITION pos = m_InputPinsList.GetHeadPosition();
    while(pos)
    {
        CSwitcherInPlaceInputPin *pInputPin = m_InputPinsList.GetNext(pos);
		delete pInputPin;
        //pInputPin->Release();
    }
    m_NumInputPins = 0;
    m_InputPinsList.RemoveAll();

} // InitInputPinsList

LCID LangToLCID(char* pLang)
{
	// pLang is a LCID in a string ("0x09" for instance)
	if (pLang[0] == '0' &&
		pLang[1] == 'x')
	{
		return (LCID)strtoul(pLang, NULL, 16);
	}
	
	// pLang is a string ("English" for instance)
	LCID lcid = 0;
	int max = strlen(pLang);
	if (max > 0)
	{		
		char szLang[MAX_PATH];
		int len;
		for (int lang = 1; lang <= 0x3ff; lang++)
		{		
			lcid = MAKELCID(MAKELANGID(lang, SUBLANG_NEUTRAL), SORT_DEFAULT);
	
			// Try Native lang name
			len = GetLocaleInfo(lcid, LOCALE_SNATIVELANGNAME, (char *)szLang, MAX_PATH);
			if (--len > 0)
			{
				if (len > max) len = max;
				if (_strnicmp(pLang, szLang, len) == 0)
					break;
			}

			// Try English lang name
			len = GetLocaleInfo(lcid, LOCALE_SENGLANGUAGE, (char *)szLang, MAX_PATH);
			if (--len > 0)
			{
				if (len > max) len = max;
				if (_strnicmp(pLang, szLang, len) == 0)
					break;
			}

			// Try Local lang name
			len = GetLocaleInfo(lcid, LOCALE_SLANGUAGE, (char *)szLang, MAX_PATH);
			if (--len > 0)
			{
				if (len > max) len = max;
				if (_strnicmp(pLang, szLang, len) == 0)
					break;
			}
		}
	}

	return lcid;
}

char* LCIDToLang(LCID lcid, char* szLang)
{
	// Local lang name
	GetLocaleInfo(lcid, LOCALE_SLANGUAGE, (char *)szLang, MAX_PATH);

	return szLang;
}

LCID CSwitcherInPlace::PreparePinInfo(char* szName, WCHAR* wszName)
{
	swprintf(wszName, L"INFO/IAS%d", m_NextInputPinNumber + 1);
	if (FindMediaProperty(wszName) != S_OK)
	{
		if (m_SrcFileName[0])
		{
			char szSection[16];
			char szDefault[16];
			wsprintf(szSection, "Audio_%d", m_NextInputPinNumber + 1);
			wsprintf(szDefault, "Audio Track %d", m_NextInputPinNumber + 1);
			GetPrivateProfileString("MSS", szSection, szDefault, szName, MAX_PATH, m_SrcFileNameIni);
			MultiByteToWideChar(CP_ACP, 0, szName, -1, wszName, strlen(szName)+1);
		}
		else
		{
			swprintf(wszName, L"Audio Track %d", m_NextInputPinNumber + 1);
			WideCharToMultiByte(CP_ACP, 0, wszName, -1, szName, MAX_PATH, NULL, NULL);
		}
	}
	else
		WideCharToMultiByte(CP_ACP, 0, wszName, -1, szName, MAX_PATH, NULL, NULL);

	LCID lcid = LangToLCID(szName);

	if (szName[0] == '0' &&
		szName[1] == 'x')
	{
		LCIDToLang(lcid, szName);
		if (szName[0] != '0')
			szName[0] &= 0xDF;
		MultiByteToWideChar(CP_ACP, 0, szName, -1, wszName, strlen(szName)+1);
	}

	return lcid;
}

int CSwitcherInPlace::GetDefaultTrack()
{
	if (m_DefTrackIndex >= 0)
		return m_DefTrackIndex;

	int nTrack = -1;
	WCHAR wszVal[MAX_PATH];
	swprintf(wszVal, L"INFO/ICAS");
	if (FindMediaProperty(wszVal) == S_OK)
	{
		nTrack = _wtoi(wszVal);
	}
	else
		nTrack = GetPrivateProfileInt("MSS", "Audio_def", -1, m_SrcFileNameIni);

	// Try to find a track matching Local LCID
	if (nTrack <= 0)
	{
		LCID user_lcid;
		LCID lcid;
		HRESULT hr;

		// Try LANG + SUBLANG
		user_lcid = ConvertDefaultLocale(LANG_USER_DEFAULT);
		nTrack = 0;
		while ((hr = GetTrackLCID(nTrack++, &lcid)) == S_OK)
		{
			if (lcid == user_lcid)
				break;
		}
		if (hr == S_OK)
			return nTrack;

		// Try LANG only
		user_lcid = PRIMARYLANGID(ConvertDefaultLocale(LANG_USER_DEFAULT));
		nTrack = 0;
		while ((hr = GetTrackLCID(nTrack++, &lcid)) == S_OK)
		{
			if (lcid == user_lcid)
				break;
		}
		if (hr == S_OK)
			return nTrack;

		// No track matchs, use first track ...
		nTrack = 1;
	}

	m_DefTrackIndex = nTrack;

	return nTrack;
}

//
// CreateNextInputPin
//
CSwitcherInPlaceInputPin *CSwitcherInPlace::CreateNextInputPin(CSwitcherInPlace *pSwitch)
{
    m_NextInputPinNumber++;     // Next number to use for pin
    HRESULT hr = NOERROR;

	char szName[MAX_PATH];
	WCHAR wszName[MAX_PATH];
	LCID lcid = PreparePinInfo(szName, wszName);

    CSwitcherInPlaceInputPin *pPin = new CSwitcherInPlaceInputPin(szName,
						pSwitch,
					    &hr, 
						wszName,
						lcid);

    if (FAILED(hr) || pPin == NULL) {
        delete pPin;
        return NULL;
    }

    //pPin->AddRef();
    return pPin;

} // CreateNextInputPin

//
// GetNumFreePins
//
int CSwitcherInPlace::GetNumFreePins()
{
    int n = 0;
    POSITION pos = m_InputPinsList.GetHeadPosition();
    while(pos) {
        CSwitcherInPlaceInputPin *pInputPin = m_InputPinsList.GetNext(pos);
        if (pInputPin && !pInputPin->IsConnected())
            n++;
    }
    return n;

} // GetNumFreePins

//
// GetNumConnectedPins
//
int CSwitcherInPlace::GetNumConnectedPins()
{
    int n = 0;
    POSITION pos = m_InputPinsList.GetHeadPosition();
    while(pos) {
        CSwitcherInPlaceInputPin *pInputPin = m_InputPinsList.GetNext(pos);
        if (pInputPin && pInputPin->IsConnected())
            n++;
    }
    return n;

} // GetNumConnectedPins

int CSwitcherInPlace::GetPinCount()
{
    return (2 + m_NumInputPins);
}

//
// GetPinNFromList
//
CSwitcherInPlaceInputPin *CSwitcherInPlace::GetPinNFromList(int n)
{
    // Validate the position being asked for
    if (n >= m_NumInputPins)
        return m_InputPinsList.Get(m_InputPinsList.GetTailPosition());

    // Get the head of the list
    POSITION pos = m_InputPinsList.GetHeadPosition();

    n++;       // Make the number 1 based

    CSwitcherInPlaceInputPin *pInputPin;
    while(n) {
        pInputPin = m_InputPinsList.GetNext(pos);
        n--;
    }
    return pInputPin;

} // GetPinNFromList

HRESULT CSwitcherInPlace::GetTrackLCID(int n, LCID *plcid)
{
	if (n >= m_NumInputPins)
		return S_FALSE;
	
	if (n == 0)
		*plcid = m_pMasterPin->m_lcid;
	else
		*plcid = GetPinNFromList(n - 1)->m_lcid;

	return S_OK;
}

HRESULT CSwitcherInPlace::SetTrackLCID(int n, LCID lcid)
{
	if (n >= m_NumInputPins)
		return S_FALSE;
	
	if (n == 0)
		m_pMasterPin->m_lcid = lcid;
	else
		GetPinNFromList(n - 1)->m_lcid = lcid;

	m_bDirty = TRUE;

	return S_OK;
}

//
// NonDelegatingQueryInterface
//
// Override CUnknown method.
// Part of the basic COM (Compound Object Model) mechanism.
// This is how we expose our interfaces.
//
STDMETHODIMP CSwitcherInPlace::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_ISwitcher) 
	{
        return GetInterface((ISwitcher *) this, ppv);
    }
    else if(riid == IID_IAMStreamSelect)
	{
#define ALWAYS_EXPOSE
#ifdef ALWAYS_EXPOSE
		return GetInterface((IAMStreamSelect *)this, ppv);
#else
		// Expose only this interface if more than 1 input pin is connected.
		if (GetNumConnectedPins())
			return GetInterface((IAMStreamSelect *)this, ppv);
		else
			return E_NOINTERFACE;
#endif
	}
    else if (riid == IID_ISpecifyPropertyPages) 
	{
#ifdef ALWAYS_EXPOSE
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
#else
		// Expose only this interface if more than 1 input pin is connected.
		if (GetNumConnectedPins())
	        return GetInterface((ISpecifyPropertyPages *) this, ppv);
		else
			return E_NOINTERFACE;
#endif
    }
    else if (riid == IID_IAMMediaContent) 
	{
        return GetInterface((IAMMediaContent *) this, ppv);
    }
    else if (riid == IID_IPropertyBag) 
	{
#ifdef ALWAYS_EXPOSE
        return GetInterface((IPropertyBag *) this, ppv);
#else
		if (GetNumConnectedPins())
	        return GetInterface((IPropertyBag *) this, ppv);
		else
			return E_NOINTERFACE;
#endif
    }
    /*else if (riid == IID_IPersistPropertyBag) 
	{
        return GetInterface((IPersistPropertyBag *) this, ppv);
    }*/
    else if (riid == __uuidof(IWMPWindowMessageSink)) 
	{
        return GetInterface((IWMPWindowMessageSink *) this, ppv);
    }
    else 
	{
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }

} // NonDelegatingQueryInterface

STDMETHODIMP CSwitcherInPlace::Count(DWORD* pcStreams)
{
	*pcStreams = 0;

	// Count DVobSub Streams
	IBaseFilter *pDVobSub = NULL;
	m_pGraph->FindFilterByName(L"DirectVobSub (auto-loading version)", &pDVobSub);
	if (pDVobSub)
	{
		IAMStreamSelect *pStrmSel = NULL;
		pDVobSub->QueryInterface(IID_IAMStreamSelect, (void**) &pStrmSel);
		if (pStrmSel)
		{
			pStrmSel->Count(pcStreams);
			pStrmSel->Release();
		}
		pDVobSub->Release();
	}

	*pcStreams += GetNumConnectedPins() + (m_bEnableMute ? 2 : 1);
	m_TotalIndex = *pcStreams;
	return S_OK;
}

STDMETHODIMP CSwitcherInPlace::Enable(long lIndex, DWORD dwFlags)
{
	if (lIndex < 0 || 
		lIndex > (long)m_TotalIndex) 
		return E_INVALIDARG;

	// Call DVobSub Enable
	if (lIndex > GetNumConnectedPins() + (m_bEnableMute ? 1 : 0)) 
	{
		IBaseFilter *pDVobSub = NULL;
		m_pGraph->FindFilterByName(L"DirectVobSub (auto-loading version)", &pDVobSub);
		if (pDVobSub)
		{
			HRESULT hr;
			IAMStreamSelect *pStrmSel = NULL;
			pDVobSub->QueryInterface(IID_IAMStreamSelect, (void**) &pStrmSel);
			if (pStrmSel)
			{
				hr = pStrmSel->Enable(lIndex - GetNumConnectedPins() - (m_bEnableMute ? 2 : 1), dwFlags);
				pStrmSel->Release();
			}
			pDVobSub->Release();

			return hr;
		}
	}

	// WMP Hacks :
	//
	// WMP6 call us 3 times before playing, enabling each audio track.
	//
	// WMP9 call us 1 time before playing, enabling first audio track
	// even if :
	//	- Previous calls to Info specified another seleted track.
	//	- "Audio and Language Tracks"/"Default ..."/"Audio Language"
	//	  is set on "(Title Default)" or something else.
	//
	// So return without doing anything while we're not receiving samples
	if (!m_bStartStreaming)
		return E_INVALIDARG;

	// Safety check
	if (!m_pSelectedPin)
		return E_INVALIDARG;

#define WMP9_FAST_SWITCH
#ifdef WMP9_FAST_SWITCH
	// WMP9 calls Enable with dwFlags == 0 (DISABLE ALL)
	// and then call Enable with dwFlags == AMSTREAMSELECTENABLE_ENABLE
	// => this stops and starts the graph twice.
	// To avoid this, return when dwFlags == 0 (DISABLE ALL).
	if (dwFlags == 0)
		return E_NOTIMPL;
#endif

#define SMART_SWITCH
#ifdef SMART_SWITCH
	if (dwFlags & AMSTREAMSELECTENABLE_ENABLE)
	{
		CSwitcherInPlaceInputPin *pPin;
		if (lIndex == 0)
			pPin = m_pMasterPin;
		else
			pPin = GetPinNFromList(lIndex - 1);

		// Enabled pin currently selected so return
		if (pPin == m_pSelectedPin)
			return S_OK;
	}
#endif

//#define PREFERRED_SWITCH
#ifndef PREFERRED_SWITCH
	// Can't ENABLE ALL
	if (dwFlags & AMSTREAMSELECTENABLE_ENABLEALL)
		return E_NOTIMPL;
#endif

	// Get current position and suspend streaming
	IMediaSeeking *pSeek;
	HRESULT hr = m_pGraph->QueryInterface(IID_IMediaSeeking, (void **) &pSeek);
	if (FAILED(hr))
		return hr;
	
	LONGLONG Current;
	pSeek->GetCurrentPosition(&Current);

	IMediaControl *pControl = NULL;
	hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **) &pControl);
	OAFilterState state = State_Running;
    if (pControl)
	{
		pControl->GetState(10, &state);

//#define STOP_SWITCH
#ifdef STOP_SWITCH
		if (state != State_Stopped)
			pControl->Stop();
#else
		if (state == State_Running)
			pControl->Pause();
#endif

	}

	// Switch ...
	if (dwFlags & AMSTREAMSELECTENABLE_ENABLE)
	{
		if (lIndex == 0)
			m_pSelectedPin = m_pMasterPin;
		else
			m_pSelectedPin = GetPinNFromList(lIndex - 1);

		m_bStartMuted = m_pSelectedPin == m_pMutePin;
	}
#ifdef PREFERRED_SWITCH
	else if (dwFlags & AMSTREAMSELECTENABLE_ENABLEALL)
	{
		// Handle ENABLE ALL as ENABLE PREFERRED
		m_pSelectedPin = m_pPreferredPin;
		m_bStartMuted = m_pSelectedPin == m_pMutePin;
	}
#endif
#ifndef WMP9_FAST_SWITCH
	else // DISABLE ALL
	{
		m_pSelectedPin = m_pMutePin;
		m_bStartMuted = TRUE;
	}
#endif

#ifdef STOP_SWITCH
	// Maybe we should call pControl->Pause() before calling 
	// pSeek->SetPositions(...)
#endif

	// Restore position and graph state
	pSeek->SetPositions(&Current, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
	pSeek->Release();

	if (pControl)
	{
        if (state == State_Running) 
            pControl->Run();
        else if (state == State_Paused) 
            pControl->Pause();
		pControl->Release();
	}

	return S_OK;
}

STDMETHODIMP CSwitcherInPlace::Info(long lIndex, AM_MEDIA_TYPE** ppmt, DWORD* pdwFlags, LCID* plcid, DWORD* pdwGroup, WCHAR** ppszName, IUnknown** ppObject, IUnknown** ppUnk)
{
	if (lIndex < 0 || 
		lIndex > (long)m_TotalIndex) 
		return S_FALSE;

	// Call DVobSub Info
	if (lIndex > GetNumConnectedPins() + (m_bEnableMute ? 1 : 0)) 
	{
		IBaseFilter *pDVobSub = NULL;
		m_pGraph->FindFilterByName(L"DirectVobSub (auto-loading version)", &pDVobSub);
		if (pDVobSub)
		{
			HRESULT hr;
			IAMStreamSelect *pStrmSel = NULL;
			pDVobSub->QueryInterface(IID_IAMStreamSelect, (void**) &pStrmSel);
			if (pStrmSel)
			{
				hr = pStrmSel->Info(lIndex - GetNumConnectedPins() - (m_bEnableMute ? 2 : 1), ppmt, pdwFlags, plcid, pdwGroup, ppszName, ppObject, ppUnk);
				pStrmSel->Release();
			}
			pDVobSub->Release();

			return hr;
		}
	}

	CSwitcherInPlaceInputPin *pPin;

	if (lIndex == 0)
		pPin = m_pMasterPin;
	else
		pPin = GetPinNFromList(lIndex - 1);

	if (pPin == NULL)
		return S_FALSE;

	if(ppmt)
	{
		CSwitcherInPlaceInputPin *pFormatPin;
		if (pPin->IsConnected())
			pFormatPin = pPin;
		else
			pFormatPin = m_pMasterPin;

		*ppmt = CreateMediaType(&pFormatPin->CurrentMediaType());

	}

	if (pdwFlags)
	{
		if (pPin == m_pSelectedPin)
			*pdwFlags = AMSTREAMSELECTINFO_ENABLED | AMSTREAMSELECTINFO_EXCLUSIVE;
		else
			*pdwFlags = 0;
	}

	if (plcid)
	{
		LCID lcid = pPin->m_lcid;
		if (pPin != m_pMutePin)
		{
			if (lcid == 0)
			{
				lcid = LangToLCID(pPin->m_szName);
				pPin->m_lcid = lcid;
			}
		}
		*plcid = (DWORD)lcid;
	}

	if (pdwGroup)
	{
		if (pPin->IsConnected())
			*pdwGroup = pPin->CurrentMediaType().majortype.Data1;
		else
			*pdwGroup = 0;
	}
	
	if (ppszName) 
	{
		if (pPin == m_pMutePin)
			*ppszName = SysAllocString(L"Mute");
		else
			*ppszName = SysAllocString(pPin->Name());
	}

	if (ppObject) *ppObject = NULL;

	if (ppUnk) *ppUnk = NULL;

	return S_OK;
}

STDMETHODIMP CSwitcherInPlace::get_AuthorName(BSTR* pbstrAuthorName)
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/IART", pbstrAuthorName);
}

STDMETHODIMP CSwitcherInPlace::get_Title(BSTR* pbstrTitle)
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/INAM", pbstrTitle);
}

STDMETHODIMP CSwitcherInPlace::get_Rating(BSTR* pbstrRating)
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/IRTD", pbstrRating);
}

STDMETHODIMP CSwitcherInPlace::get_Description(BSTR* pbstrDescription)
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/ICMT", pbstrDescription);
}

STDMETHODIMP CSwitcherInPlace::get_Copyright(BSTR* pbstrCopyright)
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/ICOP", pbstrCopyright);
}

STDMETHODIMP CSwitcherInPlace::get_BaseURL(BSTR* pbstrBaseURL) 
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/IBSU", pbstrBaseURL);
}

STDMETHODIMP CSwitcherInPlace::get_LogoURL(BSTR* pbstrLogoURL) 
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/ILGU", pbstrLogoURL);
}

STDMETHODIMP CSwitcherInPlace::get_LogoIconURL(BSTR* pbstrLogoIconURL) 
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/ILIU", pbstrLogoIconURL);
}

STDMETHODIMP CSwitcherInPlace::get_WatermarkURL(BSTR* pbstrWatermarkURL)
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/IWMU", pbstrWatermarkURL);
}

STDMETHODIMP CSwitcherInPlace::get_MoreInfoURL(BSTR* pbstrMoreInfoURL)
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/IMIU", pbstrMoreInfoURL);
}

STDMETHODIMP CSwitcherInPlace::get_MoreInfoBannerImage(BSTR* pbstrMoreInfoBannerImage)
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/IMBI", pbstrMoreInfoBannerImage);
}

STDMETHODIMP CSwitcherInPlace::get_MoreInfoBannerURL(BSTR* pbstrMoreInfoBannerURL)
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/IMBU", pbstrMoreInfoBannerURL);
}

STDMETHODIMP CSwitcherInPlace::get_MoreInfoText(BSTR* pbstrMoreInfoText)
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(L"INFO/IMIT", pbstrMoreInfoText);
}

STDMETHODIMP CSwitcherInPlace::get_MediaProperty(BSTR bstrInfo, BSTR* pbstrValue)
{
    CAutoLock l(&m_SwitcherLock);
	return FindMediaProperty(bstrInfo, pbstrValue);
}

STDMETHODIMP CSwitcherInPlace::get_sourceURL(BSTR* pbstrSourceURL)
{
	if (pbstrSourceURL && m_SrcFileName[0] != '\0')
	{
		*pbstrSourceURL = m_bstrSrcURL;
		return S_OK;
	}
	return VFW_E_NOT_FOUND;
}

STDMETHODIMP CSwitcherInPlace::Switch()
{
    CAutoLock l(&m_SwitcherLock);

	DWORD dwCount = 0;
	Count(&dwCount);

	long lIndex;
	DWORD dwFlags = 0;
	DWORD dwGroup = 0;
	for (lIndex = 0; lIndex < (long)dwCount; lIndex++)
	{
		Info(lIndex, NULL, &dwFlags, NULL, &dwGroup, NULL, NULL, NULL);
		if (dwGroup != MEDIATYPE_Audio.Data1 ||
			dwFlags & AMSTREAMSELECTINFO_ENABLED)
			break;
	}

	Info(++lIndex, NULL, NULL, NULL, &dwGroup, NULL, NULL, NULL);
	if (dwGroup != MEDIATYPE_Audio.Data1)
		lIndex = 0;

	return Enable(lIndex, AMSTREAMSELECTENABLE_ENABLE);
}

STDMETHODIMP CSwitcherInPlace::SwitchTo(int n)
{
    CAutoLock l(&m_SwitcherLock);
	return Enable(n, AMSTREAMSELECTENABLE_ENABLE);
}

STDMETHODIMP CSwitcherInPlace::GetTrackName(int n, char *pszName)
{
    CAutoLock l(&m_SwitcherLock);
	if (n >= m_NumInputPins)
		return S_FALSE;
	
	if (n == 0)
		strcpy(pszName, m_pMasterPin->m_szName);
	else
		strcpy(pszName, GetPinNFromList(n - 1)->m_szName);

	return S_OK;
}

STDMETHODIMP CSwitcherInPlace::SetTrackName(int n, char *pszName)
{
    CAutoLock l(&m_SwitcherLock);
	if (n >= m_NumInputPins)
		return S_FALSE;
	
	if (n == 0)
		m_pMasterPin->SetName(pszName);
	else
		GetPinNFromList(n - 1)->SetName(pszName);

	m_bDirty = TRUE;

	return S_OK;
}

STDMETHODIMP CSwitcherInPlace::GetActiveTrack()
{
    CAutoLock l(&m_SwitcherLock);
	DWORD dwCount = 0;
	Count(&dwCount);

	long lIndex;
	DWORD dwFlags = 0;
	DWORD dwGroup = 0;
	for (lIndex = 0; lIndex < (long)dwCount; lIndex++)
	{
		Info(lIndex, NULL, &dwFlags, NULL, &dwGroup, NULL, NULL, NULL);
		if (dwGroup != MEDIATYPE_Audio.Data1 ||
			dwFlags & AMSTREAMSELECTINFO_ENABLED)
			break;
	}

	return lIndex;
}

//-----------------------------------------------------------------------------
//                  ISpecifyPropertyPages implementation
//-----------------------------------------------------------------------------


//
// GetPages
//
// Returns the clsid's of the property pages we support
//
STDMETHODIMP CSwitcherInPlace::GetPages(CAUUID *pPages) {

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_SwitcherIPPropertyPage;

    return NOERROR;

} // GetPages

//
// IDispatch Implementation
//

// return 1 if we support GetTypeInfo

STDMETHODIMP
CSwitcherInPlace::GetTypeInfoCount(UINT * pctinfo) {
    return m_basedisp.GetTypeInfoCount(pctinfo);
}


// attempt to find our type library

STDMETHODIMP
CSwitcherInPlace::GetTypeInfo(
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo) {
    return m_basedisp.GetTypeInfo(IID_IMediaControl,
        itinfo,
        lcid,
        pptinfo);
}


STDMETHODIMP
CSwitcherInPlace::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid) {
    return m_basedisp.GetIDsOfNames(IID_IMediaControl,
        rgszNames,
        cNames,
        lcid,
        rgdispid);
}


STDMETHODIMP
CSwitcherInPlace::Invoke(
  DISPID dispidMember,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS * pdispparams,
  VARIANT * pvarResult,
  EXCEPINFO * pexcepinfo,
  UINT * puArgErr) {
    // this parameter is a dead leftover from an earlier interface
    if(IID_NULL != riid) {
        return DISP_E_UNKNOWNINTERFACE;
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if(FAILED(hr)) {
        return hr;
    }

    hr = pti->Invoke((IMediaControl *)this,
        dispidMember,
        wFlags,
        pdispparams,
        pvarResult,
        pexcepinfo,
        puArgErr);

    pti->Release();
    return hr;
}

/******************************Public*Routine******************************\
* exported entry points for registration and
* unregistration (in this case they only call
* through to default implmentations).
*
*
*
* History:
*
\**************************************************************************/
STDAPI
DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI
DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}

STDAPI_(ISwitcher **)
GetInstanceList()
{
  return (ISwitcher **)CSwitcherInPlace::m_InstanceList;
}

STDAPI_(ISwitcher *)
GetInstance(BSTR bstrSourceURL)
{
	int i;
	ISwitcher *pSwitcher = NULL;
	for (i = 0; i < MAX_INSTANCE; i++)
	{
		pSwitcher = CSwitcherInPlace::m_InstanceList[i];
		if (pSwitcher)
		{
			CSwitcherInPlace *pSwitch = (CSwitcherInPlace *)pSwitcher;
			if (pSwitch->m_SrcFileName[0] && (lstrcmpW(pSwitch->m_bstrSrcURL, bstrSourceURL) == 0))
			{
#ifndef ALWAYS_EXPOSE
				if (!pSwitch->GetNumConnectedPins())
					pSwitcher = NULL;
#endif

				break;
			}
			else
				pSwitcher = NULL;
		}
	}

	return pSwitcher;
}


// Microsoft C Compiler will give hundreds of warnings about
// unused inline functions in header files.  Try to disable them.
#pragma warning( disable:4514)
