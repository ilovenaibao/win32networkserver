#include "stdafx.h"
#include "MyClient.h"
#include "unhandledexception.h"

using namespace std;			

__my_set_exception_handler g_Exception;		//����������й������쳣��Ϣ

MyClient::MyClient(const SOCKET& s, DWORD nId)
	: m_nId(nId)
{
	m_pNet = new ClientNet(s, TRUE, this, ReadProc, ErrorProc);
	ASSERT(m_pNet);
}

MyClient::~MyClient()
{
	if (m_pNet)
	{
		delete m_pNet;
		m_pNet = NULL;
	}
}

int MyClient::ReadProc(LPVOID pParam , const char* szData , int nLen )
{
	cout << "�յ����Կͻ��˵�TCP���ݰ� DATA = " << szData << endl;

	//�����ݰ����ظ��ͻ���
	MyClient* const pClient = (MyClient*)pParam;
	ASSERT(pClient);

	if (FALSE == pClient->m_pNet->Send(szData, nLen))
	{
		printf("[%ld] ��ͻ��˷������ݰ�ʧ��\r\n", pClient->m_nId);
	}

	return 0;
}

void MyClient::ErrorProc(LPVOID pParam, int nOpt)
{
	MyClient* const pClient = (MyClient*)pParam;
	ASSERT(pClient);

	printf("[%ld] ��������� ERR_CODE = %ld\r\n", pClient->m_nId, nOpt);

	//ɾ���ÿͻ���
	delete pClient;
}
