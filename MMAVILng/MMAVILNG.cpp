// MMAVILNG.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "windows.h"
#include "mmsystem.h"
#include "AVIRIFF.h"
#include "stdio.h"
#include "stdlib.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	char *lpFileName = lpCmdLine; 

	if (*lpFileName == '"')
	{
		lpFileName++;
		lpCmdLine++;
		while (*lpCmdLine != '"' && *lpCmdLine != 0)
			lpCmdLine++;
	}
	else
		while (*lpCmdLine != ' ' && *lpCmdLine != 0)
			lpCmdLine++;

	if (*lpCmdLine != 0)
	{
		*lpCmdLine = 0;
		lpCmdLine++;
		while (*lpCmdLine == ' ') lpCmdLine++;
	}

	Sleep(1000);

	char szText[MAX_PATH];
	HANDLE	hFile;

	hFile = CreateFile(lpFileName,
					   GENERIC_READ | GENERIC_WRITE,
					   FILE_SHARE_READ | FILE_SHARE_WRITE,
					   NULL,
					   OPEN_EXISTING,
					   FILE_ATTRIBUTE_NORMAL,
					   NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{

		BYTE buf[4 * 1024];
		DWORD dwBytesRead = 0;
		BOOL bOK = ReadFile(hFile, buf, sizeof(buf), &dwBytesRead, NULL);

		char *lpDefTrackIndex = lpCmdLine;

		while (*lpCmdLine != ' ' && *lpCmdLine != 0)
			lpCmdLine++;

		if (*lpCmdLine != 0)
		{
			*lpCmdLine = 0;
			lpCmdLine++;
			while (*lpCmdLine == ' ') lpCmdLine++;
		}

		if (bOK && dwBytesRead)
		{
			// Retrieve all 'strh' (Stream Header)
			for (DWORD i = 0; i < dwBytesRead; i++)
			{
				if (*(DWORD *)&buf[i] == ckidSTREAMHEADER)
				{
					AVISTREAMHEADER *strh;
					strh = (AVISTREAMHEADER *)&buf[i];
					// Video Language is Default Language
					if (strh->fccType == streamtypeVIDEO)
					{
						strh->wLanguage = atoi(lpDefTrackIndex);
					}
					// Audio Language
					else if (strh->fccType == streamtypeAUDIO)
					{
						// Strore Audio Language
						char *lpLCID = lpCmdLine;

						while (*lpCmdLine != ' ' && *lpCmdLine != 0)
							lpCmdLine++;

						if (*lpCmdLine != 0)
						{
							*lpCmdLine = 0;
							lpCmdLine++;
							while (*lpCmdLine == ' ') lpCmdLine++;
						}

						strh->wLanguage = LANGIDFROMLCID(atoi(lpLCID));
					}
				}
			}
 
			SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

			DWORD dwBytesWritten;
			bOK = WriteFile(hFile, buf, dwBytesRead, &dwBytesWritten, NULL);
			if (!bOK || !dwBytesWritten)
			{
				// Can't WriteFile ...
				sprintf(szText, "Can't write data in\r\n%s\r\nError %d.", lpFileName, GetLastError());
				MessageBox(GetActiveWindow(), szText, "Information", MB_OK | MB_ICONINFORMATION);
			}
		}
		else
		{
			// Can't ReadFile ...
			sprintf(szText, "Can't read data from\r\n%s\r\nError %d.", lpFileName, GetLastError());
			MessageBox(GetActiveWindow(), szText, "Information", MB_OK | MB_ICONINFORMATION);
		}

		CloseHandle(hFile);
	}
	else
	{
		// Can't CreateFile ...
		sprintf(szText, "Can't open\r\n%s\r\nError %d.", lpFileName, GetLastError());
		MessageBox(GetActiveWindow(), szText, "Information", MB_OK | MB_ICONINFORMATION);
	}

	return 0;
}



