// NewClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NewClient.h"
#include <process.h>

#include "HeadFile.h"
#include "NetWork.h"
#include "UdpSerEx.h"
#include "RunLog.h"
#include "unhandledexception.h"
#include "TcpClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object

CWinApp theApp;

using namespace std;
using namespace HelpMng;


const int TCP_CLIENTS = 2000;
const int UDP_CLIENTS = 300;
UdpSerEx *g_pUdpClient[UDP_CLIENTS] = { NULL };
TcpClient *g_pTcpClients[TCP_CLIENTS] = { NULL };
int g_UdpIndexs[UDP_CLIENTS] = { 0 };
int g_TcpIndexs[TCP_CLIENTS] = { 0 };

BOOL g_bClientRun = FALSE;
BOOL g_bUdp = FALSE;			//�Ƿ�����UDP�ͻ���
BOOL g_bTcp = FALSE;

void CALLBACK OnConnect(void *pParam);
void WINAPI OnTcpClose(void *pParam, SOCKET s, int nOpt);
void WINAPI OnUdpClose(void *pParam, SOCKET s, int nOpt);
UINT WINAPI ThreadProc(LPVOID pParam);


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	SetExceptionHandler();
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{

		// TODO: code your application's behavior here.
#ifdef _DEBUG
		cout << "===================START TEST(CLIENT_D)===================" << endl;
		_TRACE("===================START TEST(CLIENT_D)===================");
#else
		cout << "===================START TEST(CLIENT)===================" << endl;
		_TRACE("===================START TEST(CLIENT)===================");
#endif
		UdpSerEx::InitReource();
		TcpClient::InitReource();

		char szTcpIp[20];
		char szUdpIp[20];
		int nTcpPort, nUdpPort;
		CString szIniFile(RunLog::GetModulePath());
		szIniFile += "client_info.ini";

		nTcpPort = GetPrivateProfileInt("tcp_info", "ser_port", 0, szIniFile);
		nUdpPort = GetPrivateProfileInt("udp_info", "ser_port", 0, szIniFile);
		g_bUdp = GetPrivateProfileInt("udp_info", "udp_start", 0, szIniFile);
		g_bTcp = GetPrivateProfileInt("tcp_info", "tcp_start", 0, szIniFile);

		GetPrivateProfileString("tcp_info", "ser_ip", "127.0.0.1", szTcpIp, sizeof(szTcpIp) -1, szIniFile);
		GetPrivateProfileString("udp_info", "ser_ip", "127.0.0.1", szUdpIp, sizeof(szUdpIp) -1, szIniFile);

		if (g_bTcp)
		{
			for (int nIndex = 0; nIndex < TCP_CLIENTS; nIndex ++)
			{
				g_TcpIndexs[nIndex] = nIndex;
				g_pTcpClients[nIndex] = new TcpClient(OnTcpClose, &g_TcpIndexs[nIndex], OnConnect, &g_TcpIndexs[nIndex]);
				ASSERT(g_pTcpClients[nIndex]);
				if (g_pTcpClients[nIndex]->Init(NULL, 0))
				{
					if (FALSE == g_pTcpClients[nIndex]->Connect(szTcpIp, nTcpPort))
					{
						printf("\r\n [TCP]�ͻ���<%ld>���ӷ����� %s:%ldʧ��", nIndex, szTcpIp, nTcpPort);
						_TRACE("[TCP]�ͻ���<%ld>���ӷ����� %s:%ldʧ��", nIndex, szTcpIp, nTcpPort);
					}
				}
				else
				{
					printf("\r\n[TCP]�ͻ���<%ld>����ʧ��", nIndex);
					_TRACE("[TCP]�ͻ���<%ld>����ʧ��", nIndex);
				}
			}
		}

		if (g_bUdp)
		{
			for (int nIndex = 0; nIndex < UDP_CLIENTS; nIndex ++)
			{
				g_UdpIndexs[nIndex] = nIndex;
				g_pUdpClient[nIndex] = new UdpSerEx(OnUdpClose, &g_UdpIndexs[nIndex], TRUE);
				ASSERT(g_pUdpClient[nIndex]);				
				if (FALSE == g_pUdpClient[nIndex]->Init(NULL, 0))
				{
					printf("\r\n [UDP] �ͻ���<%ld>��ʼ��ʧ��\r\n", nIndex);
					_TRACE("[UDP] �ͻ���<%ld>��ʼ��ʧ��", nIndex);
				}
				else
				{
					//����������͵�һ������
					char szData[1024] = { NULL };

					sprintf(szData, "[UDP] My Test UDP Data <%ld>: ��³������ͳ�Ƹ�³���Ǿ����˳�������", nIndex);
					g_pUdpClient[nIndex]->SendData(IP_ADDR(szUdpIp, nUdpPort), szData, sizeof(szData));					
				}	
			}
		}

		g_bClientRun = TRUE;
		//������̨�̴߳�������
		_beginthreadex(NULL, 0, ThreadProc, NULL, 0, NULL);

		cout << "����q�˳�����" << endl;
		getchar();
		getchar();

		g_bClientRun = FALSE;

		if (g_bTcp)
		{
			for (int nIndex = 0; nIndex < TCP_CLIENTS; nIndex++)
			{
				g_pTcpClients[nIndex]->Close();
				delete g_pTcpClients[nIndex];
			}
		}

		if (g_bUdp)
		{
			for (int nIndex = 0; nIndex < UDP_CLIENTS; nIndex++)
			{
				g_pUdpClient[nIndex]->Close();
				delete g_pUdpClient[nIndex];
			}
		}

		//������Ҫ�ڳ������ʱ�ͷ���Դ		
		TcpClient::ReleaseReource();
		UdpSerEx::ReleaseReource();
		RunLog::_Destroy();
	}

	return nRetCode;
}


UINT WINAPI ThreadProc(LPVOID pParam)
{
	DWORD nQueLen = 0;
	DWORD nUdpLen = 0;

	while (g_bClientRun)
	{
		if (g_bTcp)
		{
			for (int nIndex = 0; nIndex < TCP_CLIENTS; nIndex++)
			{
				if (g_pTcpClients[nIndex]->IsConnected())
				{
					TCP_CLIENT_RCV_EX* pRcvData = (TCP_CLIENT_RCV_EX *)(g_pTcpClients[nIndex]->GetRecvData(&nQueLen));
					if (pRcvData)
					{
						cout << "�յ�����������TCP���� : " << endl;
						cout << "LEN = " << pRcvData->m_nLen << " QUE_LEN = " << nQueLen << " DATA = " << pRcvData->m_pData + sizeof(PACKET_HEAD) << endl;

						//������ԭ�����ظ��ͻ���
						g_pTcpClients[nIndex]->SendData( pRcvData->m_pData, pRcvData->m_nLen);

						delete pRcvData;

						Sleep(100);
					}					
				}				
			}	
		}

		if (g_bUdp)
		{
			for (int nIndex = 0; nIndex < UDP_CLIENTS; nIndex++)
			{
				UDP_RCV_DATA_EX *pUdpRcvData = (UDP_RCV_DATA_EX *)(g_pUdpClient[nIndex]->GetRecvData(&nUdpLen));
				//TRACE("\r\n%s : %ld pRcvData = 0x%x", __FILE__, __LINE__, pRcvData);

				if (pUdpRcvData)
				{
					printf("�յ����Է�������UDP���� : \r\n");
					printf("LEN = %ld QUE_LEN = %ld DATA = %s\r\n", pUdpRcvData->m_nLen, nUdpLen, pUdpRcvData->m_pData);

					g_pUdpClient[nIndex]->SendData(pUdpRcvData->m_PeerAddr, pUdpRcvData->m_pData, pUdpRcvData->m_nLen);
					delete pUdpRcvData;
					Sleep(50);
				}
			}
		}						
	}
	return 0;
}

void CALLBACK OnConnect(void *pParam)
{
	//���ӳɹ�, ����������͵�һ������
	int nIndex = *((int *)pParam);
	char szBuf[1987] = { 0 };
	PACKET_HEAD *pHead = (PACKET_HEAD *)szBuf;
	pHead->nCurrentLen = (WORD)(1987 - sizeof(PACKET_HEAD));
	char *pData = szBuf + sizeof(PACKET_HEAD);
	sprintf(pData, "[TCP]<%ld> My TCP Data : ���������������������ξ�����Ϲֲ�", nIndex);
	g_pTcpClients[nIndex]->SendData(szBuf, sizeof(szBuf));
}

void WINAPI OnTcpClose(void *pParam, SOCKET s, int nOpt)
{
	int nIndex = *((int *)pParam);
	printf("[TCP] �ͻ��� %ld �Ͽ����� OPT = %ld\r\n", nIndex, nOpt);
}

void WINAPI OnUdpClose(void *pParam, SOCKET s, int nOpt)
{
	int nIndex = *(int *)pParam;
	printf("[UDP] �ͻ���<%ld>�ر�\r\n", nIndex);
}


