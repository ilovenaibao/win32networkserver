// UdpServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "UdpServer.h"
#include <vector>
#include <assert.h>
#include <WinSock2.h>
#include <process.h>
#include <string>
#include "HeadFile.h"
#include "UdpSer.h"
#include "RunLog.h"
#include "unhandledexception.h"
#include "HelpMng.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ψһ��Ӧ�ó������

CWinApp theApp;

using namespace std;
using namespace HelpMng;

//__my_set_exception_handler g_Exception;		//����������й������쳣��Ϣ
UdpSer* g_UdpSer = NULL;					//tcp�������ģ��
BOOL g_bRun = FALSE;

UINT WINAPI ThreadProc(LPVOID pParam);

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// ��ʼ�� MFC ����ʧ��ʱ��ʾ����
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: ���Ĵ�������Է���������Ҫ
		_tprintf(_T("��������: MFC ��ʼ��ʧ��\n"));
		nRetCode = 1;
	}
	else
	{
		// TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
		//��־������
		RunLog::_GetObject()->SetSaveFiles(5);			//ֻ�������5�����־��Ϣ
		(RunLog::_GetObject())->SetPrintScreen(FALSE);

		string strDecode;
		char cTemp[5];int asc[4];
		strDecode += (char)(int)(asc[0] << 2 &#124; asc[1] << 2 >> 6)
		//strDecode+=(char)(int)(asc[0] << 2 &#124; asc[1] << 2 >> 6);

#ifdef _DEBUG
		cout << "===================START TEST(SERVER_D)===================" << endl;
		_TRACE("===================START_D TEST===================");
#else
		cout << "===================START TEST(SERVER)===================" << endl;
		_TRACE("===================START TEST===================");
#endif

		UdpSer::InitReource();
		g_UdpSer = new UdpSer();
		g_bRun = TRUE;

		int nTcpPort;
		char szIp[50] = { 0 };
		CString szIniFile(RunLog::GetModulePath());
		szIniFile += "ser_info.ini";

		nTcpPort = GetPrivateProfileInt("ser_info", "port", 0, szIniFile);
		GetPrivateProfileString("ser_info", "ip", "127.0.0.1", szIp, sizeof(szIp) -1, szIniFile);

		try
		{
			//vector<string> ip_list;
			//GetLocalIps(&ip_list);
			printf("\r\n���� %s:%ld ������������\r\n", szIp, nTcpPort);

			//����TCP��UDP����
			if (FALSE == g_UdpSer->StartServer(szIp, nTcpPort))
			{
				cout << "TCP��������ʧ��" << endl;
				throw ((long)__LINE__);
			}

			//������̨�̴߳�������
			_beginthreadex(NULL, 0, ThreadProc, NULL, 0, NULL);

			cout << "����q�˳�����" << endl;
			getchar();
			getchar();

			g_bRun = FALSE;

			_TRACE("===================END TEST===================");
		}
		catch (long iErrLine)
		{
			cout << iErrLine << "�г����쳣�����˳�����" <<endl;
		}

		g_UdpSer->CloseServer();
		delete g_UdpSer;

		//������Ҫ�ڳ������ʱ�ͷ���Դ
		RunLog::_Destroy();
		UdpSer::ReleaseReource();
	}

	return nRetCode;
}

UINT WINAPI ThreadProc(LPVOID pParam)
{
	DWORD nQueLen = 0;

	//TRACE("\r\n%s : %ld g_TcpMng = 0x%x", __FILE__, __LINE__, g_TcpMng);

	while (g_bRun)
	{
		UDP_RCV_DATA* pRcvData = g_UdpSer->GetRcvData(&nQueLen);
		//TRACE("\r\n%s : %ld pRcvData = 0x%x", __FILE__, __LINE__, pRcvData);

		if (pRcvData)
		{
			cout << "�յ����Կͻ��˵�UDP���� : " << endl;
			cout << "LEN = " << pRcvData->m_nLen << " QUE_LEN = " << nQueLen << " DATA = " << pRcvData->m_pData << endl;

			//������ԭ�����ظ��ͻ���
			g_UdpSer->SendData(pRcvData->m_PeerAddr, pRcvData->m_pData, pRcvData->m_nLen);

			//ʹ�ý�����������ڴ��ͷ�
			delete pRcvData;
			pRcvData = NULL;
		}
		else
		{
			Sleep(10);
		}
	}
	return 0;
}
