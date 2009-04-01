#include "DataType.h"
#include ".\runlog.h"
#include <direct.h>
#include <assert.h>
#include <atlstr.h>
#include <atltime.h>

namespace HelpMng
{

	RunLog* RunLog::s_RunLog = NULL;
	BOOL RunLog::s_bThreadRun = FALSE;
	char RunLog::s_szModulePath[512] = { 0 };


	RunLog::RunLog(void)
		: m_hThread(NULL)
	{	
		m_bPrintScreen = FALSE;
		m_bInit = TRUE;
		m_pDataBuff = NULL;
		m_nFileCount = 0;
		m_ulCurrSize = 0;
		m_nDays = 1;
 		GetLocalTime(&m_tCurrTime);
		GetLocalTime(&m_tLastTime);
		CString strPath = GetModulePath();
		strPath += "RunLog";

		_mkdir(strPath);		//��һ��logĿ¼
		m_hMutex = CreateMutex(NULL, FALSE, NULL);
		if (NULL == m_hMutex)
		{
			m_bInit = FALSE;
		}

		//�����ڴ�ӳ���ļ�
		m_bInit = CreateMemoryFile();
	}

	RunLog::~RunLog(void)
	{
		CloseHandle(m_hMutex);
		if (NULL != m_pDataBuff)
		{
			UnmapViewOfFile(m_pDataBuff);
			m_pDataBuff = NULL;
		}

		InterlockedExchange((volatile long *)&s_bThreadRun, 0);
	}

	UINT WINAPI RunLog::ThreadProc(LPVOID lpParam)
	{
		while (InterlockedExchangeAdd((volatile long *)&s_bThreadRun, 0))
		{
			LONG nDays = s_RunLog->m_nDays;
			if (nDays < 1)
			{
				nDays = 1;
			}
			//const DWORD TIME_TICK = nDays *24 *60 *60 *1000;

			CString strFindFile(s_RunLog->GetModulePath());
			strFindFile += "RunLog\\*.LOG";

			CTime tmSave = CTime::GetCurrentTime();
			CTimeSpan tmSpan(nDays, 0, 0, 0);
			tmSave -= tmSpan;

			WIN32_FIND_DATA FindData;
			HANDLE hFind = FindFirstFile(strFindFile, &FindData);
			if (INVALID_HANDLE_VALUE != hFind)
			{
				do 
				{
					CTime tmFile(FindData.ftLastWriteTime);
					if (tmSave > tmFile)
					{
						CString strDelFile(s_RunLog->GetModulePath());
						strDelFile += "RunLog\\";
						strDelFile += FindData.cFileName;

						DeleteFile(strDelFile);
					}
				} while(FindNextFile(hFind, &FindData));	

				FindClose(hFind);
			}

			Sleep(TIMEQUEUE_PERIOD);
		}

		return 0;
	}

	//===============================================
	//Name : CreateMemoryFile()
	//Desc : ����һ���ڴ�ӳ���ļ�,ÿ����ཨ200���ļ�
	//Return : TRUE=�����ɹ�; FALSE=����ʧ��
	//===============================================
	BOOL RunLog::CreateMemoryFile()
	{	
		TCHAR szFile[MAX_PATH] = { 0 };
		HANDLE hFile;
		HANDLE hMapFile; 
		BOOL bOpenSuccess = FALSE;
		ULONG nBytesRead = 0;
		CString strPath;

		if (m_tLastTime.wDay != m_tCurrTime.wDay)
		{
			m_nFileCount = 0;
			GetLocalTime(&m_tLastTime);
		}	
		//ɾ���Ѿ�ӳ���MAP
		if (NULL != m_pDataBuff)
		{
			UnmapViewOfFile(m_pDataBuff);
			m_pDataBuff = NULL;
		}
		strPath = GetModulePath();
		strPath += "RunLog";

		do{	
			m_nFileCount++;
			bOpenSuccess = FALSE;
			m_ulCurrSize = 0;

			sprintf(szFile, "%s\\%u%02u%02u%03d.LOG", strPath, m_tCurrTime.wYear, 
				m_tCurrTime.wMonth, m_tCurrTime.wDay, m_nFileCount);
			//�Ȳ鿴�ļ���ʵ�ʴ�С�Ƿ񳬹�3M, ��С��3M����ļҼ���ʹ��, �񴴽�һ���µ��ļ�
			hFile = CreateFile(szFile, GENERIC_WRITE| GENERIC_READ, FILE_SHARE_READ |FILE_SHARE_WRITE
				, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (INVALID_HANDLE_VALUE == hFile)
			{	//���ļ���ʧ�����½�һ���ļ�
				hFile = CreateFile(szFile, GENERIC_WRITE| GENERIC_READ, FILE_SHARE_READ |FILE_SHARE_WRITE
					, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			}
			else
			{	//���ļ��ɹ�,�鿴�ļ��Ƿ񳬹�3M
				ReadFile(hFile, &m_ulCurrSize, sizeof(m_ulCurrSize), &nBytesRead, NULL);
				if (m_ulCurrSize < (FILE_MAX_SIZE - FILE_BUFF_SIZE))
				{
					bOpenSuccess = TRUE;
				}
				else
				{	//��Ҫ����һ���µ��ļ�
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
				}
			}
			if (TRUE == bOpenSuccess)
			{
				break;
			}
			if (m_nFileCount >= 200)
			{	//��200�δ���ʧ��,��˵���޷�����
				return FALSE;
			}
		} while (INVALID_HANDLE_VALUE == hFile);	

		hMapFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, FILE_MAX_SIZE, NULL);
		if (NULL == hMapFile)
		{
			CloseHandle(hFile);
			return FALSE;
		}

		//��MAP��λ���ļ��Ľ�β��		
		if (0 == m_ulCurrSize)
		{
			m_ulCurrSize = sizeof(m_ulCurrSize);
		}	
		m_pDataBuff = (char* )MapViewOfFile(hMapFile, FILE_MAP_WRITE, 0, 0, 0);
		SetFileRealSize();
		CloseHandle(hFile);
		CloseHandle(hMapFile);
		if (NULL == m_pDataBuff)
		{
			return FALSE;
		}

		return TRUE;
	}

	//===============================================
	//Name : _GetObject()
	//Desc : ��ȡ��־���Ψһ����ָ��
	//===============================================
	RunLog* RunLog::_GetObject()
	{
		if (NULL == s_RunLog)
		{
			s_RunLog = new RunLog();
		}
		if (TRUE != s_RunLog->m_bInit && NULL != s_RunLog)
		{
			delete s_RunLog;
			s_RunLog = NULL;
		}
		return s_RunLog;
	}

	//===============================================
	//Name : Destroy()
	//Desc : ������־���ȫ��Ψһ����
	//===============================================
	void RunLog::_Destroy()
	{
		if (NULL != s_RunLog)
		{
			delete s_RunLog;
			s_RunLog = NULL;
		}
	}

	//===============================================
	//Name : WriteLog()
	//Desc : �������Ϣд����־�ļ���,�����ж��ļ��Ĵ�С
	//	�Ƿ��ѳ���2M, ������2M�򴴽�һ���µ��ļ�
	//===============================================
	void RunLog::WriteLog(const char* szFormat, ...)
	{
		DWORD dwFlag = WaitForSingleObject(m_hMutex, 500);
		if (WAIT_OBJECT_0 != dwFlag)
		{	//��û�еȵ��ź����˳�����д��־����
			return;
		}
		GetLocalTime(&m_tCurrTime);	
		//�鿴�ļ��Ƿ��ѳ���3M, �����򴴽�һ���µ��ļ�
		if (m_ulCurrSize > (FILE_MAX_SIZE -FILE_BUFF_SIZE))
		{
			if(FALSE == CreateMemoryFile())
			{
				ReleaseMutex(m_hMutex);
				return;
			}
		}
		if (NULL == m_pDataBuff)
		{
			ReleaseMutex(m_hMutex);
			return;
		}
		//����Ӧ������д�뵽�ļ���������
		ULONG ulCount = 0;
		ulCount = sprintf((m_pDataBuff +m_ulCurrSize), "\r\n%u-%02u-%02u %02u:%02u:%02u:%03u", 
			m_tCurrTime.wYear, m_tCurrTime.wMonth, m_tCurrTime.wDay, 
			m_tCurrTime.wHour, m_tCurrTime.wMinute, m_tCurrTime.wSecond, m_tCurrTime.wMilliseconds);
		m_ulCurrSize += (ulCount +1);

		ulCount = vsprintf((m_pDataBuff +m_ulCurrSize), szFormat, (char* )(&szFormat +1));
		//����Ҫ��ӡ����Ļ�����ӡ����Ļ��
		if (m_bPrintScreen)
		{
			printf("\r\n%s", m_pDataBuff +m_ulCurrSize);
		}

		m_ulCurrSize += (ulCount +1);

		SetFileRealSize();
		ReleaseMutex(m_hMutex);
	}

	//===============================================
	//Name : GetModulePath()
	//Desc : ��õ�����־��Ŀ�ִ���ļ�·��
	//===============================================
	const char *RunLog::GetModulePath()
	{	
		if (0 == s_szModulePath[0])
		{
			DWORD uPathLen = GetModuleFileName(NULL, s_szModulePath, sizeof(s_szModulePath) -1);
			while (uPathLen > 0)
			{
				if ('\\' == s_szModulePath[uPathLen])
				{
					s_szModulePath[uPathLen +1] = NULL;
					break;
				}
				uPathLen --;
			}	
		}
		return s_szModulePath;
	}


	//===============================================
	//Name : SetFileRealSize()
	//Desc : ���ļ���ʵ�ʳ���д���ļ�
	//===============================================
	void RunLog::SetFileRealSize()
	{
		assert(NULL != m_pDataBuff);
		if (NULL == m_pDataBuff)
		{
			return;
		}
		memcpy(m_pDataBuff, &m_ulCurrSize, sizeof(m_ulCurrSize));
	}

	//===============================================
	//Name : SetSaveFiles()
	//Desc : ���ÿ��Ա����������ļ�, ��nDays=1; ��ֻ���浱��
	//		����־�ļ�, nDays=10,�򱣴��10�����־�ļ�.
	//		ע: ����ɾ��������������־�ļ�, �ú������Զ�����һ��
	//		��ʱ���������������̳߳��н����Զ�����.
	//===============================================
	void RunLog::SetSaveFiles(LONG nDays)
	{
		//������̨�߳�û�д����򴴽�֮
		if (NULL == m_hThread)
		{
			//������̨�߳�
			s_bThreadRun = TRUE;
			m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, this, 0, NULL);
		}
		//���ÿɱ��������
		if (nDays <= 1 || nDays >= 365)
		{
			InterlockedExchange(&m_nDays, 1);			
		}
		else
		{
			InterlockedExchange(&m_nDays, nDays);
		}
	}

	void RunLog::SetPrintScreen(BOOL bPrint /* = TRUE */)
	{
		m_bPrintScreen = bPrint;
	}

}
//===============================================
//Name :
//Desc :
//===============================================