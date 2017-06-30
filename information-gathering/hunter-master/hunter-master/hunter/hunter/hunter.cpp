// hunter.cpp : Defines the entry point for the console application.
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#define WIN32_LEAN_AND_MEAN

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <assert.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <windows.h>
#include <lm.h>
#include <time.h>
#include <sddl.h>
#include <DsGetDC.h>

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "advapi32.lib")

VOID displayHelp()
{
	wprintf(L"(l)user hunter v0.3b - rui@deniable.org\n");
	wprintf(L"Usage: hunter.exe <options> {target specification}\n\n");
	wprintf(L"Options:\n");
	wprintf(L"  -h\t\t\t\tShow this message\n");
	wprintf(L"  -f <filename>\t\t\tRetrieve configuration information for the specified hosts\n");
	wprintf(L"  -d [domain]\t\t\tRetrieve hosts configuration information using current DOMAIN or DOMAIN given\n");
	wprintf(L"  -a [computer]\t\t\tRetrieve all user accounts on a server or DC\n");
	wprintf(L"  -A [computer]\t\t\tRetrieve all user accounts information on a server or DC\n");
	wprintf(L"  -g [computer]\t\t\tRetrieve each global group on a server or DC\n");
	wprintf(L"  -u <username> [computer]\tRetrieve user information on a server or DC\n");
	wprintf(L"  -b <username> [computer]\tRetrieve a list of global groups a user belongs on a server or DC\n");
	wprintf(L"  -m <group> [computer]\t\tRetrieve list of members in a particular global group on a server or DC\n"); 
	wprintf(L"  -e\t\t\t\tEnumerate Domain Controllers in the Local Domain\n\n");
	wprintf(L"  -min <sec> -max <sec>\t\tMin and Max delay between queries in seconds, applies to -f and -d only\n");

	exit(0);
}

VOID printColoured(wchar_t *message, WORD color)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	WORD saved_attributes;

	GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
	saved_attributes = consoleInfo.wAttributes;

	SetConsoleTextAttribute(hConsole, color);
	wprintf(message);

	SetConsoleTextAttribute(hConsole, saved_attributes);
}

int randomDelay(int min, int max)
{
	int stime;

	srand((unsigned)time(NULL));
	stime = (rand() % (max - min)) + min;
	
	printColoured(L"\n Sleeping for ", (FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE));
	wprintf(L"%d", stime);
	printColoured(L" seconds before next request\n", (FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE));

	return stime;
}

VOID getDomainControllers()
{
	DWORD dwRet;
	PDOMAIN_CONTROLLER_INFO pdcInfo;

	dwRet = DsGetDcName(NULL, NULL, NULL, NULL, 0, &pdcInfo);
	
	if (ERROR_SUCCESS == dwRet)
	{
		HANDLE hGetDc;

		dwRet = DsGetDcOpen(pdcInfo->DomainName, DS_NOTIFY_AFTER_SITE_RECORDS, NULL, NULL, NULL, 0, &hGetDc);

		if (ERROR_SUCCESS == dwRet)
		{
			LPTSTR pszDnsHostName;

			while (TRUE)
			{
				ULONG ulSocketCount;
				LPSOCKET_ADDRESS rgSocketAddresses;

				dwRet = DsGetDcNext(hGetDc, &ulSocketCount, &rgSocketAddresses, &pszDnsHostName);

				if (ERROR_SUCCESS == dwRet)
				{
					wprintf(L" -- %s\n", pszDnsHostName);

					NetApiBufferFree(pszDnsHostName);

					LocalFree(rgSocketAddresses);
				}
				else if (ERROR_NO_MORE_ITEMS == dwRet)
					break;
				else if (ERROR_FILEMARK_DETECTED == dwRet)
				{
					wprintf(L" End of site-specific domain controllers\n");
					continue;
				}
				else
					break;
			}

			DsGetDcClose(hGetDc);
		}

		NetApiBufferFree(pdcInfo);
	}
}

VOID userInfo(wchar_t *host, wchar_t *user)
{
	NET_API_STATUS nStatus;
	LPTSTR pszServerName = NULL;
	LPUSER_INFO_0 pBuf = NULL;
	LPUSER_INFO_4 pBuf4 = NULL;
	DWORD dwLevel = 4;
	LPTSTR sStringSid = NULL;
	int i = 0;

	if (host != NULL)
	{
		pszServerName = (LPTSTR)host;
	}
	else
	{
		nStatus = NetGetDCName(NULL, NULL, (LPBYTE *)&pszServerName);

		if (nStatus == NERR_Success)
			wprintf(L" Primary Domain Controller: %ls\n", pszServerName);
		else
		{
			wprintf(L" Failed to retrieve Primary DC\n");
			return;
		}
	}

	nStatus = NetUserGetInfo((LPCWSTR)pszServerName, user, dwLevel, (LPBYTE *)& pBuf);

	if (nStatus == NERR_Success)
	{
		if (pBuf != NULL)
		{
				pBuf4 = (LPUSER_INFO_4)pBuf;
				wprintf(L" User account name: %s\n", pBuf4->usri4_name);
				wprintf(L" Password: %s\n", pBuf4->usri4_password);
				wprintf(L" Password age (seconds): %d\n", pBuf4->usri4_password_age);
				wprintf(L" Privilege level: %d\n", pBuf4->usri4_priv);
				wprintf(L" Home directory: %s\n", pBuf4->usri4_home_dir);
				wprintf(L" Comment: %s\n", pBuf4->usri4_comment);
				wprintf(L" Flags (in hex): %x\n", pBuf4->usri4_flags);
				wprintf(L" Script path: %s\n", pBuf4->usri4_script_path);
				wprintf(L" Auth flags (in hex): %x\n",	pBuf4->usri4_auth_flags);
				wprintf(L" Full name: %s\n", pBuf4->usri4_full_name);
				wprintf(L" User comment: %s\n", pBuf4->usri4_usr_comment);
				wprintf(L" Parameters: %s\n", pBuf4->usri4_parms);
				wprintf(L" Workstations: %s\n", pBuf4->usri4_workstations);
				wprintf(L" Last logon (seconds since January 1, 1970 GMT): %d\n", pBuf4->usri4_last_logon);
				wprintf(L" Last logoff (seconds since January 1, 1970 GMT): %d\n", pBuf4->usri4_last_logoff);
				wprintf(L" Account expires (seconds since January 1, 1970 GMT): %d\n",	pBuf4->usri4_acct_expires);
				wprintf(L" Max storage: %d\n", pBuf4->usri4_max_storage);
				wprintf(L" Units per week: %d\n", pBuf4->usri4_units_per_week);
				wprintf(L" Logon hours:");
				for (i = 0; i < 21; i++)
				{
					printf(" %x", (BYTE)pBuf4->usri4_logon_hours[i]);
				}
				wprintf(L"\n");
				wprintf(L" Bad password count: %d\n", pBuf4->usri4_bad_pw_count);
				wprintf(L" Number of logons: %d\n", pBuf4->usri4_num_logons);
				wprintf(L" Logon server: %s\n", pBuf4->usri4_logon_server);
				wprintf(L" Country code: %d\n", pBuf4->usri4_country_code);
				wprintf(L" Code page: %d\n", pBuf4->usri4_code_page);
				if (ConvertSidToStringSid(pBuf4->usri4_user_sid, &sStringSid))
				{
					wprintf(L" User SID: %s\n", sStringSid);
					LocalFree(sStringSid);
				}
				else
					wprintf(L" ConvertSidToSTringSid failed with error %d\n", GetLastError());
				wprintf(L" Primary group ID: %d\n", pBuf4->usri4_primary_group_id);
				wprintf(L" Profile: %s\n", pBuf4->usri4_profile);
				wprintf(L" Home directory drive letter: %s\n", pBuf4->usri4_home_dir_drive);
				wprintf(L" Password expired information: %d\n", pBuf4->usri4_password_expired);
		}
	}

	else
		fprintf(stderr, " NetUserGetinfo failed with error: %d\n", nStatus);

	if (pBuf != NULL)
		NetApiBufferFree(pBuf);
}

VOID getGroups(wchar_t *host)
{
	NET_API_STATUS nStatus;
	LPTSTR pszServerName = NULL;
	DWORD dwLevel = 0;
	PGROUP_INFO_0 pBuf = NULL;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD_PTR dwResumeHandle = 0;
	PGROUP_INFO_0 pTmpBuf;
	DWORD i = 0;

	if (host != NULL)
	{
		pszServerName = (LPTSTR)host;
	}
	else
	{
		nStatus = NetGetDCName(NULL, NULL, (LPBYTE *)&pszServerName);

		if (nStatus == NERR_Success)
			wprintf(L" Primary Domain Controller: %ls\n", pszServerName);
		else
		{
			wprintf(L" Failed to retrieve Primary DC");
			return;
		}
	}

	do
	{
		nStatus = NetGroupEnum((LPCWSTR)pszServerName, dwLevel, (LPBYTE*)&pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);

		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				for (i = 0; i < dwEntriesRead; i++)
				{
					assert(pTmpBuf != NULL);

					if (pTmpBuf == NULL)
					{
						fprintf(stderr, "An access violation has occurred\n");
						break;
					}

					wprintf(L" -- %s\n", pTmpBuf->grpi0_name);
					pTmpBuf++;
				}
			}
		}
		else
			fprintf(stderr, "A system error has occurred: %d\n", nStatus);

		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}

	} while (nStatus == ERROR_MORE_DATA);

	if (pBuf != NULL)
	{
		NetApiBufferFree(pBuf);
		pBuf = NULL;
	}
}

VOID getUsers(wchar_t *host, BOOL all)
{
	NET_API_STATUS nStatus;
	LPTSTR pszServerName = NULL;
	DWORD dwLevel = 0;
	LPUSER_INFO_0 pBuf = NULL;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	LPUSER_INFO_0 pTmpBuf;
	DWORD i = 0;

	if(host != NULL)
	{
		pszServerName = (LPTSTR)host;
	}
	else
	{
		nStatus = NetGetDCName(NULL, NULL, (LPBYTE *)&pszServerName);

		if (nStatus == NERR_Success)
			wprintf(L" Primary Domain Controller: %ls\n", pszServerName);
		else
		{
			wprintf(L" Failed to retrieve Primary DC\n");
			return;
		}
	}

	do
	{
		nStatus = NetUserEnum((LPCWSTR)pszServerName, dwLevel, FILTER_NORMAL_ACCOUNT, (LPBYTE*)&pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);

		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				for (i = 0; i < dwEntriesRead; i++)
				{
					assert(pTmpBuf != NULL);

					if (pTmpBuf == NULL)
					{
						fprintf(stderr, "An access violation has occurred\n");
						break;
					}

					if (all == TRUE)
					{
						wprintf(L"\n");
						userInfo(pszServerName, pTmpBuf->usri0_name);
					}
					else
						wprintf(L" -- %s\n", pTmpBuf->usri0_name);

					

					pTmpBuf++;
				}
			}
		}
		else
			fprintf(stderr, "A system error has occurred: %d\n", nStatus);

		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}
	
	} while (nStatus == ERROR_MORE_DATA);

	if (pBuf != NULL)
	{
		NetApiBufferFree(pBuf);
		pBuf = NULL;
	}
}

VOID isLocaladmin(wchar_t *host)
{
	SC_HANDLE schSCManager;
	wchar_t server[255]; 

	_snwprintf(server, 255, L"\\\\%s", host);

	schSCManager = OpenSCManager(server, NULL, SC_MANAGER_ALL_ACCESS);

	if (schSCManager != 0)
	{
		printColoured(L"\t* You are a local admin at:", (FOREGROUND_INTENSITY | FOREGROUND_RED));
		wprintf(L" %s\n", server);
	}
}

VOID groupsOfUser(wchar_t *user, wchar_t *host)
{
	NET_API_STATUS nStatus;
	DWORD dwLevel = 0;
	LPGROUP_USERS_INFO_0 pBuf = NULL;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	LPTSTR pszServerName = NULL;

	printColoured(L" [+] Retrieving groups of which the user: ", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));
	wprintf(L"%ls", user);
	printColoured(L" belongs\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));

	if (host != NULL)
	{
		pszServerName = (LPTSTR)host;
	}
	else
	{
		nStatus = NetGetDCName(NULL, NULL, (LPBYTE *)&pszServerName);

		if (nStatus == NERR_Success)
			wprintf(L" Primary Domain Controller: %ls\n", pszServerName);
		else
		{
			wprintf(L" Failed to retrieve Primary DC");
			return;
		}
	}

	wprintf(L" Server: %ls\n", pszServerName);

	nStatus = NetUserGetGroups((LPCWSTR)pszServerName, user, dwLevel, (LPBYTE *)&pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries);

	if (nStatus == NERR_Success)
	{
		LPGROUP_USERS_INFO_0 pTmpBuf;
		DWORD i;
		DWORD dwTotalCount = 0;

		if ((pTmpBuf = pBuf) != NULL)
		{
			for (i = 0; i < dwEntriesRead; i++)
			{
				assert(pTmpBuf != NULL);

				if (pTmpBuf == NULL)
				{
					fprintf(stderr, "An access violation has occurred\n");
					break;
				}

				wprintf(L" -- %s\n", pTmpBuf->grui0_name);

				pTmpBuf++;
				dwTotalCount++;
			}
		}
	}
	else
		fprintf(stderr, "A system error has occurred: %d\n", nStatus);

	if (pBuf != NULL)
		NetApiBufferFree(pBuf);
}

VOID groupusersEnum(wchar_t *group, wchar_t *host)
{
	NET_API_STATUS nStatus;
	LPTSTR pszServerName = NULL;
	GROUP_USERS_INFO_0 *BufPtr, *p;
	DWORD er = 0;
	DWORD tr = 0;
	DWORD resume = 0;
	DWORD i = 0;

	if (host != NULL)
	{
		pszServerName = (LPTSTR)host;
	}
	else
	{
		nStatus = NetGetDCName(NULL, NULL, (LPBYTE *)&pszServerName);

		if (nStatus == NERR_Success)
			wprintf(L" Primary Domain Controller: %ls\n", pszServerName);
		else
		{
			wprintf(L" Failed to retrieve Primary DC");
			return;
		}
	}

	printColoured(L" [+] Retrieving users from group: ", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));
	wprintf(L"%ls\n", group);

	nStatus = NetGroupGetUsers(pszServerName, group, 0, (LPBYTE *)&BufPtr, MAX_PREFERRED_LENGTH, &er, &tr, NULL);

	if (nStatus == ERROR_SUCCESS || nStatus == ERROR_MORE_DATA)
	{
		p = BufPtr;

		for (i = 1; i <= er; i++)
		{
			wprintf(L" -- %ls\n", p->grui0_name);
			p++;
		}
	}
	else if (nStatus == NERR_GroupNotFound)
	{
		wprintf(L" -- Group not found\n");
	}
	else
		wprintf(L"Error: %d\n", nStatus);

	if (BufPtr != NULL)
		NetApiBufferFree(BufPtr);
}

BOOL hasReadAccess(LPCTSTR pszFolder)
{
	HANDLE hToken;
	DWORD dwAccessDesired;
	PRIVILEGE_SET PrivilegeSet;
	DWORD dwPrivSetSize;
	DWORD dwAccessGranted;
	BOOL fAccessGranted = FALSE;
	GENERIC_MAPPING GenericMapping;
	SECURITY_INFORMATION si = (SECURITY_INFORMATION)(OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION);
	PSECURITY_DESCRIPTOR psdSD = NULL;
	DWORD dwNeeded;

	GetFileSecurity(pszFolder, si, NULL, 0, &dwNeeded);

	psdSD = (PSECURITY_DESCRIPTOR) new BYTE[dwNeeded];

	GetFileSecurity(pszFolder, si, psdSD, dwNeeded, &dwNeeded);

	ImpersonateSelf(SecurityImpersonation);
	OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, TRUE, &hToken);

	dwAccessDesired = ACCESS_READ;

	memset(&GenericMapping, 0xff, sizeof(GENERIC_MAPPING));
	GenericMapping.GenericRead = ACCESS_READ;
	GenericMapping.GenericWrite = ACCESS_WRITE;
	GenericMapping.GenericExecute = 0;
	GenericMapping.GenericAll = ACCESS_READ | ACCESS_WRITE;

	MapGenericMask(&dwAccessDesired, &GenericMapping);

	dwPrivSetSize = sizeof(PRIVILEGE_SET);

	AccessCheck(psdSD, hToken, dwAccessDesired, &GenericMapping, &PrivilegeSet, &dwPrivSetSize, &dwAccessGranted, &fAccessGranted);

	delete[] psdSD;

	RevertToSelf();

	return fAccessGranted;
}

DWORD ipaddrEnum(wchar_t *host)
{
	WSADATA wsaData;
	DWORD dwRetval;
	DWORD iResult;
	DWORD iRetval;

	ADDRINFOW hints;
	ADDRINFOW *result;
	ADDRINFOW *ptr;

	LPSOCKADDR sockaddr_ip;
	wchar_t ipstringbuffer[46];
	DWORD ipbufferlength = 46;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		wprintf(L"WSATartup failed: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	dwRetval = GetAddrInfoW(host, 0, &hints, &result);
	if (dwRetval != 0)
	{
		wprintf(L"GetAddrInfoW failed with error: %d\n", dwRetval);
		WSACleanup();
		return 1;
	}

	printColoured(L"\t* IP Addresses\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		switch (ptr->ai_family)
		{
		case AF_UNSPEC:
			break;
		case AF_INET:
			sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
			ipbufferlength = 46;
			iRetval = WSAAddressToString(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL, ipstringbuffer, &ipbufferlength);
			if (iRetval)
				wprintf(L"WSAAddressToString failed with %u\n", WSAGetLastError());
			else
				wprintf(L"\tIPv4 address %ws\n", ipstringbuffer);
			break;
		case AF_INET6:
			break;
		default:
			break;
		}
	}

	FreeAddrInfoW(result);
	WSACleanup();

	return 0;
}

VOID sharesEnum(wchar_t *host)
{
	NET_API_STATUS nStatus;
	LPTSTR lpszServer = NULL;
	PSHARE_INFO_502 BufPtr, p;
	DWORD er = 0;
	DWORD tr = 0;
	DWORD resume = 0;
	DWORD i = 0;
	wchar_t buff[MAX_PATH];

	printColoured(L"\t* Shared resources\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));

	do
	{
		nStatus = NetShareEnum(lpszServer, 502, (LPBYTE *)&BufPtr, MAX_PREFERRED_LENGTH, &er, &tr, &resume);

		if (nStatus == ERROR_SUCCESS || nStatus == ERROR_MORE_DATA)
		{
			p = BufPtr;

			for (i = 1; i <= er; i++)
			{
				if (_wcsicmp(p->shi502_netname, L"IPC$"))
				{
					_snwprintf(buff, MAX_PATH, L"\\\\%s\\%s", host, p->shi502_netname);

					if (hasReadAccess(buff))
					{
						wprintf(L"\t%s", buff);
						printColoured(L" - Read Access!\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN));
					}
					else
						wprintf(L"\t%s - No Read access\n", buff);
				}

				p++;
			}

			NetApiBufferFree(BufPtr);
		}
		else
			fprintf(stderr, "A system error has occurred: %d\n", nStatus);

	} while (nStatus == ERROR_MORE_DATA);
}

VOID sessionsEnum(wchar_t *host)
{
	NET_API_STATUS nStatus;
	LPTSTR pszClientName = NULL;
	LPTSTR pszUserName = NULL;
	DWORD dwLevel = 10;
	LPSESSION_INFO_10 pBuf = NULL;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	DWORD i = 0;
	LPSESSION_INFO_10 pTmpBuf;

	do
	{
		nStatus = NetSessionEnum(host,
			pszClientName,
			pszUserName,
			dwLevel,
			(LPBYTE*)&pBuf,
			dwPrefMaxLen,
			&dwEntriesRead,
			&dwTotalEntries,
			&dwResumeHandle);

		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				printColoured(L"\t* Sessions established\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));

				for (i = 0; i < dwEntriesRead; i++)
				{
					assert(pTmpBuf != NULL);

					if (pTmpBuf == NULL)
					{
						fprintf(stderr, "An access violation has occurred\n");
						break;
					}

					wprintf(L"\tUser \"%s\" from client \"%s\" - Time: \"%d\" - Idle: \"%d\"\n", pTmpBuf->sesi10_username, pTmpBuf->sesi10_cname, pTmpBuf->sesi10_time, pTmpBuf->sesi10_idle_time);

					pTmpBuf++;
				}
			}
		}
		else
			fprintf(stderr, "A system error has occurred: %d\n", nStatus);

		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}

	} while (nStatus == ERROR_MORE_DATA);

	if (pBuf != NULL) 
	{
		NetApiBufferFree(pBuf);
		pBuf = NULL;
	}
}

VOID loggedonUsers(wchar_t *host)
{
	NET_API_STATUS nStatus;
	LPWSTR pszServerName = NULL;
	DWORD dwLevel = 1;
	LPWKSTA_USER_INFO_1 pBuf = NULL;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	DWORD i = 0;
	LPWKSTA_USER_INFO_1 pTmpBuf = NULL;

	do
	{
		nStatus = NetWkstaUserEnum(pszServerName,
			dwLevel,
			(LPBYTE*)&pBuf,
			dwPrefMaxLen,
			&dwEntriesRead,
			&dwTotalEntries,
			&dwResumeHandle);

		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				printColoured(L"\t* Loggedon Users\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));

				for (i = 0; i < dwEntriesRead; i++)
				{
					assert(pTmpBuf != NULL);

					if (pTmpBuf == NULL)
					{
						fprintf(stderr, "An access violation has occurred\n");
						break;
					}

					if (!wcschr((wchar_t *)pTmpBuf->wkui1_username, L'$'))
					{
						wprintf(L"\tUser \"\%s\\%s\" - is logged on.\n", pTmpBuf->wkui1_logon_domain, pTmpBuf->wkui1_username);
					}

					pTmpBuf++;
				}
			}
			else
				fprintf(stderr, "A system error has occurred: %d\n", nStatus);
		}

		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}

	} while (nStatus == ERROR_MORE_DATA);

	if (pBuf != NULL)
		NetApiBufferFree(pBuf);
}

VOID serverInfo(wchar_t *host)
{
	NET_API_STATUS nStatus;
	LPWSTR pszServerName = host;
	DWORD dwLevel = 101;
	LPSERVER_INFO_101 pBuf = NULL;
	LPSERVER_INFO_101 pTmpBuf;

	printColoured(L"\n [*] Configuration information\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));

	nStatus = NetServerGetInfo(pszServerName, dwLevel, (LPBYTE *)& pBuf);

	if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
	{
		if ((pTmpBuf = pBuf) != NULL)
		{
			assert(pTmpBuf != NULL);
			
			if (pTmpBuf == NULL)
			{
				fprintf(stderr, "[-] Access violation!\n");
				return;
			}
			else
			{
				wprintf(L" HOST: %s\n", (wchar_t*)host);
				wprintf(L" OS Version - %d.%d\n", (int)pTmpBuf->sv101_version_major, (int)pTmpBuf->sv101_version_minor);
				if (pTmpBuf->sv101_type & SV_TYPE_DOMAIN_CTRL)
				{
					wprintf(L" Domain Controller\n");
				}
				if (pTmpBuf->sv101_type & SV_TYPE_DOMAIN_BAKCTRL)
				{
					wprintf(L" Backup Domain Controller\n");
				}
				if (pTmpBuf->sv101_type & SV_TYPE_NT)
				{
					wprintf(L" Workstation or Server\n");
				}
				if (pTmpBuf->sv101_type & SV_TYPE_TERMINALSERVER)
				{
					wprintf(L" Terminal Server\n");
				}
				if (pTmpBuf->sv101_type & SV_TYPE_SQLSERVER)
				{
					wprintf(L" MSSQL Server\n");
				}
			}
		}
	}
	if (pBuf != NULL)
	{
		NetApiBufferFree(pBuf);
	}
}

VOID hostsEnum(wchar_t *domain, DWORD min, DWORD max)
{
	NET_API_STATUS nStatus;
	LPWSTR pszServerName = NULL;
	DWORD dwLevel = 101;
	LPSERVER_INFO_101 pBuf = NULL;
	LPSERVER_INFO_101 pTmpBuf;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwServerType = SV_TYPE_SERVER;
	LPWSTR pszDomainName = domain;
	DWORD dwResumeHandle = 0;

	nStatus = NetServerEnum(pszServerName,
		dwLevel,
		(LPBYTE *)& pBuf,
		dwPrefMaxLen,
		&dwEntriesRead,
		&dwTotalEntries,
		dwServerType,
		pszDomainName,
		&dwResumeHandle);

	if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
	{
		if ((pTmpBuf = pBuf) != NULL)
		{
			for (unsigned int i = 0; i < dwEntriesRead; i++)
			{
				assert(pTmpBuf != NULL);
				if (pTmpBuf == NULL)
				{
					fprintf(stderr, "An access violation has occurred\n");
					break;
				}
				else
				{
					serverInfo(pTmpBuf->sv101_name);
					loggedonUsers(pTmpBuf->sv101_name);
					sessionsEnum(pTmpBuf->sv101_name);
					sharesEnum(pTmpBuf->sv101_name);
					ipaddrEnum(pTmpBuf->sv101_name);
					isLocaladmin(pTmpBuf->sv101_name);

					if (min >= 0 && max > 0)
						Sleep(randomDelay(min, max) * 1000);

					pTmpBuf++;
				}
			}
		}
	}

	if (pBuf != NULL)
	{
		NetApiBufferFree(pBuf);
	}
}

int wmain(int argc, wchar_t * argv[])
{
	wchar_t *filename;
	wchar_t *domain = NULL;
	FILE *file_of_hosts;
	wchar_t line[255];
	wchar_t pTmpBuf[255];
	wchar_t *group = NULL;
	wchar_t *user = NULL;
	wchar_t *host = NULL;

	if (argc == 1)
		displayHelp();

	if (_wcsicmp(argv[1], L"-b") == 0) 
	{
		if (argc == 3)
		{
			user = (wchar_t *)malloc((wcslen(argv[2]) + 1) * sizeof(wchar_t));
			wcscpy(user, argv[2]);
			groupsOfUser(user, NULL);
		}
		else if (argc == 4)
		{
			user = (wchar_t *)malloc((wcslen(argv[2]) + 1) * sizeof(wchar_t));
			wcscpy(user, argv[2]);

			host = (wchar_t *)malloc((wcslen(argv[3]) + 1) * sizeof(wchar_t));
			wcscpy(host, argv[3]);

			groupsOfUser(user, host);
		}
		else
			wprintf(L" [-] Error: Invalid options for -b. Exiting\n");

		return 0;
	}
	else if (_wcsicmp(argv[1], L"-g") == 0)
	{
		if (argc == 2)
		{
			printColoured(L" [+] Retrieving groups from Primary DC\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));
			getGroups(NULL);
		}
		else if (argc == 3)
		{
			host = (wchar_t *)malloc((wcslen(argv[2]) + 1) * sizeof(wchar_t));
			wcscpy(host, argv[2]);

			printColoured(L" [+] Retrieving groups from ", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));
			wprintf(L"%s\n", host);

			getGroups(host);
		}
		else
			wprintf(L" [-] Error: Invalid options for -g. Exiting\n");

		return 0;
	}
	else if (wcscmp(argv[1], L"-a") == 0)
	{
		if (argc == 2)
		{
			printColoured(L" [+] Retrieving users from Primary DC\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));
			getUsers(NULL, FALSE);
		}
		else if (argc == 3)
		{
			host = (wchar_t *)malloc((wcslen(argv[2]) + 1) * sizeof(wchar_t));
			wcscpy(host, argv[2]);

			printColoured(L" [+] Retrieving users from ", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));
			wprintf(L"%s\n", host);

			getUsers(host, FALSE);
		}
		else
			wprintf(L" [-] Error: Invalid options for -a. Exiting\n");

		return 0;
	}
	else if (wcscmp(argv[1], L"-A") == 0)
	{
		if (argc == 2)
		{
			printColoured(L" [+] Retrieving users information from Primary DC\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));
			getUsers(NULL, TRUE);
		}
		else if (argc == 3)
		{
			host = (wchar_t *)malloc((wcslen(argv[2]) + 1) * sizeof(wchar_t));
			wcscpy(host, argv[2]);

			printColoured(L" [+] Retrieving users information from ", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));
			wprintf(L"%s\n", host);

			getUsers(host, TRUE);
		}
		else
			wprintf(L" [-] Error: Invalid options for -A. Exiting\n");

		return 0;
	}
	else if (_wcsicmp(argv[1], L"-u") == 0)
	{
		if (argc == 3 || argc == 4)
		{
			user = (wchar_t *)malloc((wcslen(argv[2]) + 1) * sizeof(wchar_t));
			wcscpy(user, argv[2]);

			printColoured(L" [+] Retrieving ", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));
			wprintf(L"%s", user);
		}
		if (argc == 3)
		{
			printColoured(L" information from Primary DC\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));

			userInfo(NULL, user);
		}
		else if (argc == 4)
		{
			host = (wchar_t *)malloc((wcslen(argv[3]) + 1) * sizeof(wchar_t));
			wcscpy(host, argv[3]);

			printColoured(L" information from ", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));
			wprintf(L"%s\n", host);

			userInfo(host, user);
		}
		else
			wprintf(L" [-] Error: Invalid options for -u. Exiting\n");

		return 0;
	}
	else if (_wcsicmp(argv[1], L"-m") == 0)
	{
		if (argc == 3)
		{
			printColoured(L" [+] Retrieving users from Primary DC\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));
		}
		else if (argc == 4)
		{
			host = (wchar_t *)malloc((wcslen(argv[3]) + 1) * sizeof(wchar_t));
			wcscpy(host, argv[3]);

			printColoured(L" [+] Retrieving users from ", (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE));
			wprintf(L"%s\n", host);
		}
		else
		{
			wprintf(L" [-] Error: Invalid options for -l. Exiting\n");
			return 0;
		}

		group = (wchar_t *)malloc((wcslen(argv[2]) + 1) * sizeof(wchar_t));
		wcscpy(group, argv[2]);

		groupusersEnum(group, host);

		return 0;
	}
	else if (_wcsicmp(argv[1], L"-f") == 0)
	{
		if ((argc == 3) || (argc == 7))
		{
			printColoured(L" [+] Reading hosts from file", (FOREGROUND_INTENSITY | FOREGROUND_GREEN));
			filename = (wchar_t *)malloc((wcslen(argv[2]) + 1) * sizeof(wchar_t));
			wcscpy(filename, argv[2]);
			file_of_hosts = _wfopen(filename, L"r");

			if (file_of_hosts == NULL)
			{
				printf(" [-] Error: File not found. Exiting.\n");
			}
			else
			{
				while (fgetws(line, sizeof(line) - 1, file_of_hosts))
				{
					swscanf(line, L"%s\n", pTmpBuf);
					serverInfo(pTmpBuf);
					loggedonUsers(pTmpBuf);
					sessionsEnum(pTmpBuf);
					sharesEnum(pTmpBuf);
					ipaddrEnum(pTmpBuf);
					isLocaladmin(pTmpBuf);
					if (argc == 7)
						if ((_wcsicmp(argv[3], L"-min") == 0) && (_wcsicmp(argv[5], L"-max") == 0))
							Sleep(randomDelay(wcstol(argv[4], NULL, 10), wcstol(argv[6], NULL, 10)) * 1000);
				}
				fclose(file_of_hosts);
			}
		}
		else
			wprintf(L" [-] Error: Invalid options for -f. Exiting\n");

		return 0;
	}
	else if (!_wcsicmp(argv[1], L"-d"))
	{
		if (argc == 2)
		{
			printColoured(L" [+] No domain specified, using current domain", (FOREGROUND_INTENSITY | FOREGROUND_GREEN));
			hostsEnum(domain, 0, 0);
		}
		else if (argc == 3)
		{
			domain = argv[2];
			printf("[+] Using Domain: %ls\n", domain);
			hostsEnum(domain, 0, 0);
		}
		else if (argc == 6)
		{
			if ((_wcsicmp(argv[2], L"-min") == 0) && (_wcsicmp(argv[4], L"-max") == 0))
			{
				hostsEnum(domain, wcstol(argv[3], NULL, 10), wcstol(argv[5], NULL, 10));
			}
			else
				wprintf(L" [-] Error: Invalid options for -d. Exiting.\n");
		}
		else if (argc == 7)
		{
			if ((_wcsicmp(argv[3], L"-min") == 0) && (_wcsicmp(argv[5], L"-max") == 0))
			{
				hostsEnum(domain, wcstol(argv[4], NULL, 10), wcstol(argv[6], NULL, 10));
			}
			else
				wprintf(L" [-] Error: Invalid options for -d. Exiting.\n");
		}
		else
			wprintf(L" [-] Error: Invalid options for -d. Exiting.\n");

		return 0;
	}
	else if (!_wcsicmp(argv[1], L"-e"))
	{
		if (argc == 2)
		{
			printColoured(L" [+] Domain Controllers in the Local Domain\n", (FOREGROUND_INTENSITY | FOREGROUND_GREEN));
			getDomainControllers();
		}
		else
			wprintf(L" [-] Error: Invalid options for -e. Exiting.\n");
	}
	else 
		displayHelp();
	
	return 0;
}
