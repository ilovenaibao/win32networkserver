// checknat_imclient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "checknat_imclient.h"
#include <process.h>

#include <string>
#include "ClientNet.h"
#include "natcheck.h"

// The one and only application object

CWinApp theApp;

using namespace std;
using namespace HelpMng;			//��Ҫ���������ռ�
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientNet* g_pTcpNet = NULL;       //�ͻ�������TCP����ģ��
//This is Listen
ClientNet* g_pTcpListen = NULL;
//This is Listen
ClientNet* g_pTcpSym = NULL;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool g_bNCClose = false;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char g_szClientNatIP[16]; 
char g_szClientNatPort[6];
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int g_NatType = CHECK_NAT_FAILE;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TCP�����ص�
void CALLBACK AcceptProc(LPVOID pParam, const SOCKET& sockClient,const IP_ADDR& AddrClient);

//�������ⲿ�������ӻص��ӷ���������
void CALLBACK AcceptErrorProc(LPVOID pParam, int nOpt);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//TCPģʽ�����ݽ���������������
int CALLBACK ReadProc(LPVOID pParam, const char* szData, int nLen);

//�������������ɹ���Ļص�����
void CALLBACK ConnectProc(LPVOID pParam);

//���������������ص�����
void CALLBACK ErrorProc(LPVOID pParam, int nOpt);
////////////////////////////////////////////////////////////////////////////////////

//TCPģʽ�����ݽ��մӷ���������
int CALLBACK NCSymReadProc(LPVOID pParam, const char* szData, int nLen);
//���ӳɹ���Ļص��ӷ���������
void CALLBACK NCSymConnectProc(LPVOID pParam);
//������ӷ������ص�����
void CALLBACK NCSymErrorProc(LPVOID pParam, int nOpt);
//////////////////////////////////////////////////////////////////////////////////////////////


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		ClientNet::InitReource();

		g_pTcpNet = new ClientNet(TRUE);
		assert(g_pTcpNet);
		
		g_pTcpListen = new ClientNet(TRUE);
		assert(g_pTcpListen);
		
		g_pTcpSym = new ClientNet(TRUE);
		assert(g_pTcpSym);

		//����������ͨ��
		g_pTcpNet->SetTCPCallback(NULL, NULL, ConnectProc, ReadProc, ErrorProc);
		
		//��ӷ�����ͨ��
		g_pTcpSym->SetTCPCallback(NULL, NULL, NCSymConnectProc, NCSymReadProc, NCSymErrorProc);
		
		//�����ⲿ����ͨ��
		g_pTcpListen->SetTCPCallback(NULL, AcceptProc ,NULL, NULL, AcceptErrorProc);
	
		//������������
		if (FALSE == g_pTcpNet->Connect(NC_MAINSER_IP, NC_MAINSER_IP_PORT, true, "192.168.100.69", 10000))
		{
			cout << "���ӵ������� " << NC_MAINSER_IP << ":" << NC_MAINSER_IP_PORT << "ʧ��" << endl;
		
		}		

		// TODO: code your application's behavior here.
	}
	cout << "����q�˳�����" << endl;
	while (!g_bNCClose)
	{

	}
	cout << "this is end" << endl;
/*	while (getchar() != 'q')
	{
	}*/


	//��������������ͨ��
	if (g_pTcpNet != NULL)
	{
		delete g_pTcpNet;
		g_pTcpNet = NULL;
	}
	if (g_pTcpSym != NULL)
	{
		delete g_pTcpSym;
		g_pTcpSym = NULL;

	}
	//ɾ���Դӷ������ļ���
	if (g_pTcpListen != NULL)
	{
		delete g_pTcpListen;
		g_pTcpListen = NULL;

	}

	ClientNet::ReleaseReource();
	return nRetCode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TCPģʽ�����ݽ���������������
int CALLBACK ReadProc(LPVOID pParam, const char* szData, int nLen)
{
	static int nCount = 1;
	char szBuf[BUF_SIZE] = { 0 };
	int iResult;

	string szIp;
	int nPort;

 
	//�������������ͨ�ŵĵ�ַ
	g_pTcpNet->GetLocalAddr(szIp, nPort);

	cout << "�ܵ���������������TCP���ݰ� DATA = " << szData << endl;
	sprintf(szBuf, "Client TCP Data %ld", nCount);

	//ɾ���Դӷ������ļ���
	if (g_pTcpListen != NULL)
	{
		delete g_pTcpListen;
		g_pTcpListen = NULL;

	}

	//��������
	iResult = NCHandleRecvMsg(szData, g_szClientNatIP, g_szClientNatPort);
	if (iResult == REQUEST_CHECKNAT_SYMMETRIC) {
		cout << "��ʼ���Գ���,���Ӵӷ�����" << endl;
		cout << "Bind IP: " << szIp.c_str() << "port" << nPort << endl;
		if (FALSE == g_pTcpSym->Connect(NC_ASSISTANSER_IP, NC_ASSISTANTSER_PORT, TRUE, szIp.c_str(), nPort))
		{
			cout << "���ӵ��ӷ����� " << NC_ASSISTANSER_IP << ":" << NC_ASSISTANTSER_PORT << "ʧ��" << endl;
			if (g_pTcpSym != NULL)
			{
				delete g_pTcpSym;
				g_pTcpSym = NULL;

			}
		}
		cout << "���ӵ��ӷ����� " << NC_ASSISTANSER_IP << ":" << NC_ASSISTANTSER_PORT << "�ɹ�" << endl;
		
		
	}
	else if (iResult == INEXIST_NAT_NOT_PASSIVITY__VISIT) {
		cout << "This is result: INEXIST_NAT_NOT_PASSIVITY__VISIT" << endl;
		g_bNCClose = false;
	}
	else {
		cout << "This is result : ERROR" << endl;
		g_bNCClose = false;
	}
	
	nCount ++;
	return 0;
}

//�������������ɹ���Ļص�����
void CALLBACK ConnectProc(LPVOID pParam)
{
	cout << "�ɹ����ӵ�������" << endl;
	//����NATCHECK��һ��TCP���ݰ� 
	char sendbuf[BUF_SIZE];
	char *pszPort = NULL;
	string szIp;
	int nPort;
    int iMsgLen = 0;

	//�������������Դ��ַ
	g_pTcpNet->GetLocalAddr(szIp, nPort);
	CovertUIntToString(nPort, &pszPort);

	//���������ⲿ����ͨ��
	if (NCHandleSendMsg(sendbuf, REQUSET_NC_ASSISTANTSEV_ACTIVELINK, szIp.c_str(), pszPort, iMsgLen)) {
		
		g_pTcpNet->Send(sendbuf, iMsgLen);
		
		cout << "Client Send msg:= "<< sendbuf << " to NCMainSev is ok" << endl;
	
	}
    DelString(&pszPort);

	//�����ⲿͨѶ
	if (g_pTcpListen->Listen(10000, true, szIp.c_str())) {
		cout << "start Listen is ok" << endl;
	}
	else {
		cout << "start Listen is failed" << endl;
		
		delete g_pTcpListen;
		g_pTcpListen = NULL;
	}
}

//���������������ص�����
void CALLBACK ErrorProc(LPVOID pParam, int nOpt)
{
	cout << "��⵽�������; ������ = " << nOpt << endl;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TCP�����ⲿ�������ӻص�
void CALLBACK AcceptProc(LPVOID pParam, const SOCKET& sockClient,const IP_ADDR& AddrClient)
{
	
	char *pszIP = NULL;
	unsigned int uiPort = 0;
    
	//��öԷ���ַ
	if (P2PGetLocalIPAndPort(AddrClient, &pszIP, uiPort)) {
		cout << "Accept " << pszIP << "Port " << uiPort << endl;
		if (strcmp(pszIP, NC_ASSISTANSER_IP) == 0) 
		{
			cout << "�ӷ������������ӳɹ�" << endl;
			
			g_NatType = PASSIVITY_VISIT;
			cout << "the client Nat is PASSIVITY_VISIT" << endl;
			g_bNCClose = true;
		}
		else 
		{
			cout << "�Ƿ��ⲿ��������" << endl;
			closesocket(sockClient);

		}
	}
    DelString(&pszIP);
	

}
void CALLBACK AcceptErrorProc(LPVOID pParam, int nOpt)
{
	cout << "����socket���ִ���"  << nOpt << endl;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�ӷ������ԳƼ��
int CALLBACK NCSymReadProc(LPVOID pParam, const char* szData, int nLen)
{
	static int nCount = 1;
	char szBuf[BUF_SIZE] = { 0 };
	int iResult = CHECK_NAT_FAILE;

	cout << "�ܵ����Դӷ�������TCP���ݰ� DATA = " << szData << endl;
	sprintf(szBuf, "Client TCP Data %ld", nCount);

	iResult = NCHandleRecvMsg(szData, NULL, NULL);
	if (iResult == CONES_NAT)
	{
		cout << "This is result CONES_NAT" << endl;

	}
	else if (iResult == SYMMETRIC_NAT) 
	{
		cout << "This is result SYMMETRIC_NAT" << endl;
	}
	else
	{
		cout << "NCSym is error" << endl;
	}

	//ɾ����ӷ�������ͨ��
	if (g_pTcpSym != NULL)
	{
		delete g_pTcpSym;
		g_pTcpSym = NULL;

	}
	g_bNCClose = false;
	
	g_NatType = iResult;

	nCount ++;
	return 0;
}

//���Ӵӷ������ԳƼ��ɹ���Ļص�����
void CALLBACK NCSymConnectProc(LPVOID pParam)
{
	cout << "�ɹ����ӵ��ӷ�����" << endl;
	//����NATCHECK��һ��TCP���ݰ� 
	char sendbuf[BUF_SIZE];
	int iMsgLen = 0;


	//��ʼ���sym
	if (NCHandleSendMsg(sendbuf, REQUEST_CHECKNAT_SYMMETRIC, g_szClientNatIP, g_szClientNatPort, iMsgLen)) {
		
		g_pTcpSym->Send(sendbuf, iMsgLen+1);
		cout << iMsgLen << endl;
		cout << strlen(sendbuf) << endl;
		cout << "The client Send Msg = " << sendbuf << endl;
	}

	
}
void CALLBACK NCSymErrorProc(LPVOID pParam, int nOpt)
{
	cout << "NCSymsocket���ִ���" << nOpt << endl;
}