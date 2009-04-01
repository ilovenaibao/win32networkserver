#include "stdafx.h"
#include "MyClient.h"
#include "unhandledexception.h"

using namespace std;			

__my_set_exception_handler g_Exception;		//����������й������쳣��Ϣ

MyClient::~MyClient()
{
	if (m_pNet)
	{
		delete m_pNet;
		m_pNet = NULL;
	}
}

BOOL MyClient::Init(DWORD nId , const char* szSerIp , int nPort , BOOL bType)
{
	BOOL bRet = TRUE;
	try
	{
		m_nId = nId;
		m_nCount = 0;
		m_pNet = new ClientNet(TRUE);
		ASSERT(m_pNet);

		if (NULL == m_pNet)
		{
			throw ((long)(__LINE__));
		}

		//������ػص�����, TCP���͵Ŀͻ���
		if (bType)
		{
			m_pNet->SetTCPCallback(this, NULL, ConnectPrco, ReadProc, ErrorProc);

			//����TCP������
			if (FALSE == m_pNet->Connect(szSerIp, nPort, FALSE, NULL, 0))
			{
				cout << "���ӵ�������ʧ��" <<endl;
				throw ((long)__LINE__);
			}
		}
		else
		{
			m_pNet->SetUDPCallback(this, ReadFromProc, ErrorProc);

			//��ʼ��UDP�ͻ���
			if (FALSE == m_pNet->Bind(0, FALSE, NULL))
			{
				cout << "�󶨱���UDP�˿�ʧ��" << endl;
				throw ((long)__LINE__);
			}

			//��UDP���������͵�һ������
			const INT BUF_SIZE = 512;
			char szBuf[BUF_SIZE] = { 0 };
			sprintf(szBuf, "[%ld]<UDP> MyClient Data 0", m_nId);
			if (FALSE == m_pNet->SendTo(IP_ADDR(szSerIp, nPort), szBuf, (int)(strlen(szBuf) +1)))
			{
				cout << '[' << m_nId << ']' << " ����������͵�1��UDP���ݰ�ʧ��" << endl;
				throw ((long)__LINE__);
			}
		}

	}
	catch (const long& iErrLine)
	{
		bRet = FALSE;
		TRACE("\r\nExcept : %s--%ld", __FILE__, iErrLine);
	}
	
	return bRet;
}

void MyClient::ConnectPrco(LPVOID pParam)
{
	//�ɹ����ӵ�����������������͵�һ��TCP���ݰ�
	MyClient* const pClient = (MyClient*)pParam;
	ASSERT(pClient);
	
	const INT BUF_SIZE = 2048;
	char szBuf[BUF_SIZE] = { 0 };
		
	sprintf(szBuf, "[%ld]<TCP> MyClient Data 0", pClient->m_nId);

	if (FALSE == pClient->m_pNet->Send(szBuf, (int)(sizeof(szBuf) -1)))
	{
		printf("[%ld] ���������1��TCP���ݰ�ʧ��\r\n", pClient->m_nId);
	}	
}

int MyClient::ReadProc(LPVOID pParam , const char* szData , int nLen )
{
	cout << "�յ����Է�������TCP���ݰ� DATA = " << szData << endl;

	TRACE("\r\n strlen(szData) = %ld", strlen(szData));
	//�������������һ�����ݰ�
	MyClient* const pClient = (MyClient*)pParam;
	ASSERT(pClient);
	pClient->m_nCount++;

	const INT BUF_SIZE = 2048;
	char szBuf[BUF_SIZE] = { 0 };

	sprintf(szBuf, "[%ld]<TCP> MyClient Data %ld", pClient->m_nId, pClient->m_nCount);
	Sleep(200);
	if (FALSE == pClient->m_pNet->Send(szBuf, (int)(sizeof(szBuf) -1)))
	{
		printf("[%ld] ���������%ld��TCP���ݰ�ʧ��\r\n", pClient->m_nId, pClient->m_nCount);
	}

	return 0;
}

void MyClient::ReadFromProc(LPVOID pParam , const IP_ADDR& peer_addr , const char* szData , int nLen )
{
	cout << "�յ����Է�������UDP���ݰ� DATA = " << szData <<endl;

	//�������������һ�����ݰ�
	MyClient* const pClient = (MyClient*)pParam;
	ASSERT(pClient);
	pClient->m_nCount++;

	const INT BUF_SIZE = 512;
	char szBuf[BUF_SIZE] = { 0 };
	sprintf(szBuf, "[%ld]<UDP> MyClient Data %ld", pClient->m_nId, pClient->m_nCount);
	Sleep(50);
	if (FALSE == pClient->m_pNet->SendTo(peer_addr, szBuf, (int)(strlen(szBuf) +1)))
	{
		printf("[%ld] ���������%ld��UDP���ݰ�ʧ��\r\n", pClient->m_nId, pClient->m_nCount);
	}
	return;
}

void MyClient::ErrorProc(LPVOID pParam, int nOpt)
{
	MyClient* const pClient = (MyClient*)pParam;
	ASSERT(pClient);

	printf("[%ld] ��������� ERR_CODE = %ld\r\n", pClient->m_nId, nOpt);
}
