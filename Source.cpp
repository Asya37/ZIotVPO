#include <Windows.h>
#include <iostream>
#include <fstream>
#include <WTSApi32.h>
using namespace std;

#define SERVICE_NAME TEXT("MyServiceUI")
SERVICE_STATUS ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE hServiceStatusHandle = NULL;
HANDLE hServiceEvent = NULL;

void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpArgv);
void WINAPI ServiceControlHandler(DWORD dwControl);
void ServiceReportStatus(
	DWORD dwCurrentState,
	DWORD dwWin32ExitCode,
	DWORD dwWaitHint);
void ServiceInit(DWORD dwArgc, LPTSTR* lpArgv);
void ServiceInstall(void);
void ServiceDelete(void);
void ServiceStart(void);
void ServiceStop(void);

int main(int argc, CHAR* argv[])
{

	BOOL bStServiceCtrlDispatcher = FALSE;

	if (lstrcmpiA(argv[1], "install") == 0)
	{
		ServiceInstall();
	}
	else if (lstrcmpiA(argv[1], "start") == 0)
	{
		ServiceStart();

	}
	else if (lstrcmpiA(argv[1], "stop") == 0)
	{
		ServiceStop();
	}
	else if (lstrcmpiA(argv[1], "delete") == 0)
	{
		ServiceDelete();
	}

	else
	{
		SERVICE_TABLE_ENTRY DispatchTable[] =
		{
		{const_cast<LPWSTR>(SERVICE_NAME), reinterpret_cast<LPSERVICE_MAIN_FUNCTION>(ServiceMain)},
		{NULL,NULL}
		};

		bStServiceCtrlDispatcher = StartServiceCtrlDispatcher(DispatchTable);
		if (FALSE == bStServiceCtrlDispatcher)
		{
			cout << "StartServiceCtrlDispatcher Failed" << GetLastError() << endl;
		}
		else
		{
			cout << "StartServiceCtrlDispatcher Success" << endl;
		}

	}

	system("PAUSE");
	return 0;
}


void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpArgv)
{
	BOOL bServiceStatus = FALSE;

	hServiceStatusHandle = RegisterServiceCtrlHandler(
		SERVICE_NAME,
		ServiceControlHandler);

	if (NULL == hServiceStatusHandle)
	{
		cout << "RegisterServiceCtrlHandler Failed" << GetLastError() << endl;
	}
	else
	{
		cout << "RegisterServiceCtrlHandler Success" << endl;
	}

	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStatus.dwServiceSpecificExitCode = 0;

	ServiceReportStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	bServiceStatus = SetServiceStatus(hServiceStatusHandle, &ServiceStatus);
	if (FALSE == bServiceStatus)
	{
		cout << "Service Status initial Setup FAILED = " << GetLastError() << endl;
	}
	else
	{
		cout << "Service Status initial Setup SUCCESS" << endl;
	}


	PWTS_SESSION_INFO wtsSessions;
	DWORD sessionCount;
	if (!WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &wtsSessions, &sessionCount))
	{
		return;
	}

	for (auto i = 1; i < sessionCount; ++i)
	{
		auto wtsSession = wtsSessions[i].SessionId;
		HANDLE userToken;
		WTSQueryUserToken(wtsSession, &userToken);
		PROCESS_INFORMATION pi{};
		STARTUPINFO si;
		WCHAR path[] = L"C:\\Windows\\System32\\calc.exe";
		CreateProcessAsUser(userToken, NULL, path, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}

	ServiceInit(dwArgc, lpArgv);

}

void WINAPI ServiceControlHandler(DWORD dwControl) {
	DWORD errorCode = NO_ERROR;
	switch (dwControl)
	{
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_STOP:
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		break;
	default:
		errorCode = ERROR_CALL_NOT_IMPLEMENTED;
		break;
	}
	ServiceReportStatus(ServiceStatus.dwCurrentState, errorCode, 0);
}

void ServiceInit(DWORD dwArgc, LPTSTR* lpArgv)
{

	hServiceEvent
		= CreateEvent(
			NULL,
			TRUE,
			FALSE,
			NULL);

	if (NULL == hServiceEvent)
	{
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}
	else
	{
		ServiceReportStatus(SERVICE_RUNNING, NO_ERROR, 0);
	}


	while (1)
	{
		WaitForSingleObject(hServiceEvent, INFINITE);
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}

}

void ServiceReportStatus(
	DWORD dwCurrentState,
	DWORD dwWin32ExitCode,
	DWORD dwWaitHunt)
{
	static DWORD dwCheckPoint = 1;
	BOOL bSetServiceStatus = FALSE;

	ServiceStatus.dwCurrentState = dwCurrentState;
	ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	ServiceStatus.dwWaitHint = dwWaitHunt;

	if (dwCurrentState == SERVICE_START_PENDING)
	{
		ServiceStatus.dwControlsAccepted = 0;
	}
	else
	{
		ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}

	if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
	{
		ServiceStatus.dwCheckPoint = 0;
	}
	else
	{
		ServiceStatus.dwCheckPoint = dwCheckPoint++;
	}

	bSetServiceStatus = SetServiceStatus(hServiceStatusHandle, &ServiceStatus);

	if (FALSE == bSetServiceStatus)
	{
		cout << "Service Status Failed = " << GetLastError() << endl;
	}
	else
	{
	}
}



void ServiceInstall(void)
{
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScCreateService = NULL;
	DWORD dwGetModuleFileName = 0;
	TCHAR szPath[MAX_PATH];

	dwGetModuleFileName = GetModuleFileName(NULL, szPath, MAX_PATH);
	if (0 == dwGetModuleFileName)
	{
		cout << "Service Installation Failed" << GetLastError() << endl;
	}
	else
	{
	}

	hScOpenSCManager = OpenSCManager(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);

	if (NULL == hScOpenSCManager)
	{
		cout << "OpenSCManager Failed = " << GetLastError() << endl;
	}
	else
	{
	}

	hScCreateService = CreateService(
		hScOpenSCManager,
		SERVICE_NAME,
		SERVICE_NAME,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		szPath,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	if (NULL == hScCreateService)
	{
		cout << "CreateSeervice Failed = " << GetLastError() << endl;
		CloseServiceHandle(hScOpenSCManager);
	}
	else
	{
	}

	CloseServiceHandle(hScCreateService);
	CloseServiceHandle(hScOpenSCManager);

}


void ServiceDelete(void)
{
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScOpenService = NULL;
	BOOL bDeleteService = FALSE;

	hScOpenSCManager = OpenSCManager(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);

	if (NULL == hScOpenSCManager)
	{
		cout << "OpenSCManager Failed = " << GetLastError() << endl;
	}
	else
	{
	}

	hScOpenService = OpenService(
		hScOpenSCManager,
		SERVICE_NAME,
		SERVICE_ALL_ACCESS);

	if (NULL == hScOpenService)
	{
		cout << "OpenService Failed = " << GetLastError() << endl;
	}
	else
	{
	}

	bDeleteService = DeleteService(hScOpenService);

	if (FALSE == bDeleteService)
	{
		cout << "Delete Service Failed = " << GetLastError() << endl;
	}
	else
	{
	}

	CloseServiceHandle(hScOpenService);
	CloseServiceHandle(hScOpenSCManager);
	cout << "ServiceDelete End" << endl;

}


void ServiceStart(void)
{

	BOOL bStartService
		= FALSE;
	SERVICE_STATUS_PROCESS SvcStatusProcess;
	SC_HANDLE hOpenSCManager = NULL;
	SC_HANDLE hOpenService = NULL;
	BOOL bQueryServiceStatus = FALSE;
	DWORD dwBytesNeeded;

	hOpenSCManager = OpenSCManager(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);

	if (NULL == hOpenSCManager)
	{
		cout << "hOpenSCManager Failed = " << GetLastError() << endl;
	}
	else
	{
	}

	hOpenService = OpenService(
		hOpenSCManager,
		SERVICE_NAME,
		SC_MANAGER_ALL_ACCESS);

	if (NULL == hOpenService)
	{
		cout << "OpenServcie Failed = " << GetLastError() << endl;
		CloseServiceHandle(hOpenSCManager);
	}
	else
	{
	}

	bQueryServiceStatus = QueryServiceStatusEx(
		hOpenService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&SvcStatusProcess,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded);

	if (FALSE == bQueryServiceStatus)
	{
		cout << "QueryService Failed = " << GetLastError() << endl;
		CloseServiceHandle(hOpenService);
		CloseServiceHandle(hOpenSCManager);
	}
	else
	{
	}

	if ((SvcStatusProcess.dwCurrentState != SERVICE_STOPPED) &&
		(SvcStatusProcess.dwCurrentState != SERVICE_STOP_PENDING))
	{
		cout << "service is already running" << endl;
	}
	else
	{
	}

	while (SvcStatusProcess.dwCurrentState == SERVICE_STOP_PENDING)
	{
		bQueryServiceStatus = QueryServiceStatusEx(
			hOpenService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&SvcStatusProcess,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded);

		if (FALSE == bQueryServiceStatus)
		{
			cout << "Query Service Failed = " << GetLastError() << endl;
			CloseServiceHandle(hOpenService);
			CloseServiceHandle(hOpenSCManager);
		}
		else
		{
		}
	}

	bStartService = StartService(
		hOpenService,
		NULL,
		NULL);

	if (FALSE == bStartService)
	{
		cout << "StartService Failed = " << GetLastError() << endl;
		CloseServiceHandle(hOpenService);
		CloseServiceHandle(hOpenSCManager);
	}
	else
	{
	}

	bQueryServiceStatus = QueryServiceStatusEx(
		hOpenService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&SvcStatusProcess,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded);

	if (FALSE == bQueryServiceStatus)
	{
		cout << "QueryService Failed = " << GetLastError() << endl;
		CloseServiceHandle(hOpenService);
		CloseServiceHandle(hOpenSCManager);
	}
	else
	{
	}

	if (SvcStatusProcess.dwCurrentState == SERVICE_RUNNING)
	{
	}
	else
	{
		cout << "Service Running Failed = " << GetLastError() << endl;
		CloseServiceHandle(hOpenService);
		CloseServiceHandle(hOpenSCManager);
	}

	CloseServiceHandle(hOpenService);
	CloseServiceHandle(hOpenSCManager);
}

void ServiceStop(void)
{

	SERVICE_STATUS_PROCESS SvcStatusProcess;
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScOpenService = NULL;
	BOOL bQueryServiceStatus = TRUE;
	BOOL bControlService = TRUE;
	DWORD dwBytesNeeded;

	hScOpenSCManager = OpenSCManager(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);

	if (NULL == hScOpenSCManager)
	{
		cout << "OpenSCManager Failed = " << GetLastError() << endl;
	}
	else
	{
	}

	hScOpenService = OpenService(
		hScOpenSCManager,
		SERVICE_NAME,
		SC_MANAGER_ALL_ACCESS);

	if (NULL == hScOpenService)
	{
		cout << "OpenService Failed = " << GetLastError() << endl;
		CloseServiceHandle(hScOpenSCManager);
	}
	else
	{
	}

	bQueryServiceStatus = QueryServiceStatusEx(
		hScOpenService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&SvcStatusProcess,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded);

	if (FALSE == bQueryServiceStatus)
	{
		cout << "QueryService Failed = " << GetLastError() << endl;
		CloseServiceHandle(hScOpenService);
		CloseServiceHandle(hScOpenSCManager);
	}
	else
	{
	}

	bControlService = ControlService(
		hScOpenService,
		SERVICE_CONTROL_STOP,
		(LPSERVICE_STATUS)&SvcStatusProcess);

	if (TRUE == bControlService)
	{
	}
	else
	{
		cout << "Control Service Failed = " << GetLastError() << endl;
		CloseServiceHandle(hScOpenService);
	}

	while (SvcStatusProcess.dwCurrentState != SERVICE_STOPPED)
	{
		bQueryServiceStatus = QueryServiceStatusEx(
			hScOpenService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&SvcStatusProcess,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded);

		if (TRUE == bQueryServiceStatus)
		{
			cout << "Queryservice FAiled = " << GetLastError() << endl;
			CloseServiceHandle(hScOpenService);
			CloseServiceHandle(hScOpenSCManager);
		}
		else
		{
		}

		if (SvcStatusProcess.dwCurrentState == SERVICE_STOPPED)
		{
			break;
		}
		else
		{
			cout << "Service stopped Faield = " << GetLastError() << endl;
			CloseServiceHandle(hScOpenService);
			CloseServiceHandle(hScOpenSCManager);
		}
	}

	CloseServiceHandle(hScOpenService);
	CloseServiceHandle(hScOpenSCManager);
}