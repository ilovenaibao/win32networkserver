#ifndef _MY_CLIENT_H
#define _MY_CLIENT_H
#include "ClientNet.h"
using namespace HelpMng;

//�ͻ�����
class MyClient
{
public:
	MyClient() {}
	MyClient(const SOCKET& s, DWORD nId);
	virtual ~MyClient();

protected:
	ClientNet* m_pNet;			//����TCP�����紫��ģ��
	DWORD m_nId;				//�ÿͻ��˵�ID


	/************************************************
	* Desc : TCP�Ķ����ݴ�����
	************************************************/
	static int CALLBACK ReadProc(
		LPVOID pParam
		, const char* szData
		, int nLen
		);

	/************************************************
	* Desc : ����Ĵ�����
	************************************************/
	static void CALLBACK ErrorProc(LPVOID pParam, int nOpt);
private:
};

#endif		//#define _MY_CLIENT_H