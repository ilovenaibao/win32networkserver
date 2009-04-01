#ifndef _MY_CLIENT_H
#define _MY_CLIENT_H
#include "ClientNet.h"
using namespace HelpMng;

//�ͻ�����
class MyClient
{
public:
	MyClient() {}
	virtual ~MyClient();

	/************************************************
	* Desc : ��ʼ���ͻ���
	************************************************/
	BOOL Init(
		DWORD nId				//�ÿͻ��˵�ID
		, const char* szSerIp	//��������IP��ַ
		, int nPort				//�������Ķ˿�
		, BOOL bType			//tcp or udp
		);
protected:
	ClientNet* m_pNet;			//����TCP�����紫��ģ��
	DWORD m_nId;				//�ÿͻ��˵�ID
	DWORD m_nCount;				//�ͻ��˵ı��ļ�����


	/************************************************
	* Desc : TCP�Ķ����ݴ�����
	************************************************/
	static int CALLBACK ReadProc(
		LPVOID pParam
		, const char* szData
		, int nLen
		);

	/************************************************
	* Desc : UDP�Ķ����ݴ�����
	************************************************/
	static void CALLBACK ReadFromProc(
		LPVOID pParam
		, const IP_ADDR& peer_addr
		, const char* szData
		, int nLen
		);

	/************************************************
	* Desc : ���ӳɹ���Ĵ�����
	************************************************/
	static void CALLBACK ConnectPrco(LPVOID pParam);

	/************************************************
	* Desc : ����Ĵ�����
	************************************************/
	static void CALLBACK ErrorProc(LPVOID pParam, int nOpt);
private:
};

#endif		//#define _MY_CLIENT_H