#ifndef _NET_WORK_H
#define _NET_WORK_H
#include "HeadFile.h"

#ifdef WIN32
#ifdef _DLL_EXPORTS_
#define DLLENTRY __declspec(dllexport)
#include <windows.h>
#else
#define DLLENTRY __declspec(dllimport)
#endif		//#ifdef _DLL_EXPORTS_
#else
#define DLLENTRY  
#endif		//#ifdef WIN32

#pragma warning (disable:4251)

namespace HelpMng
{

	/****************************************************
	* Name : ReleaseNetWork()
	* Desc : �ͷ�class NetWork�������Դ
	****************************************************/
	void ReleaseNetWork();

	class DLLENTRY NetWork
	{
		friend void ReleaseNetWork(); 
	public:
		/****************************************************
		* Name : InitReource()
		* Desc : �����ȫ�ּ���̬��Դ���г�ʼ��
		*	�÷�����DllMain()��������, �����߲��ܵ���
		*	�÷���; ���������ڴ�й¶
		****************************************************/
		static void InitReource();

		NetWork()
			: m_hSock(INVALID_SOCKET)
		{}
		virtual ~NetWork() {}

		/****************************************************
		* Name : Init()
		* Desc : ������ģ����г�ʼ��
		****************************************************/
		virtual BOOL Init(
			const char *szIp	//Ҫ��������ı��ص�ַ, NULL �����Ĭ�ϵ�ַ
			, int nPort	//Ҫ��������Ķ˿�
			) = 0;

		/****************************************************
		* Name : Close()
		* Desc : �ر�����ģ��
		****************************************************/
		virtual void Close() = 0;

		/****************************************************
		* Name : SendData()
		* Desc : TCPģʽ�ķ�������
		****************************************************/
		virtual BOOL SendData(const char * szData, int nDataLen, SOCKET hSock = INVALID_SOCKET);

		/****************************************************
		* Name : SendData()
		* Desc : UDPģʽ�����ݷ���
		****************************************************/
		virtual BOOL SendData(const IP_ADDR& PeerAddr, const char * szData, int nLen);

		/****************************************************
		* Name : GetRecvData()
		* Desc : �ӽ��ն�����ȡ��һ����������
		****************************************************/
		virtual void *GetRecvData(DWORD* const pQueLen);

		/****************************************************
		* Name : Connect()
		* Desc : ���ӵ�ָ���ķ�����, �ͻ�������ģ����ʵ�ָú���
		****************************************************/
		virtual BOOL Connect(
			const char *szIp
			, int nPort
			, const char *szData = NULL     //���ӳɹ����͵ĵ�һ������
			, int nLen = 0                            //���ݵĳ���
			);

		/*************************************************
		* Name : GetSocket()
		* Desc : 
		***********************************************/
		SOCKET GetSocket()const { return m_hSock; }
		
	protected:
		SOCKET m_hSock;

		static HANDLE s_hCompletion;                    //��ɶ˿ھ��
		static HANDLE *s_pThreads;                       //����ɶ˿��Ϲ������߳�

		static UINT WINAPI WorkThread(LPVOID lpParam);

		/****************************************************
		* Name : IOCompletionProc()
		* Desc : IO��ɺ�Ļص�����
		****************************************************/
		virtual void IOCompletionProc(BOOL bSuccess, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) = 0;

	private:
		static BOOL s_bInit;                                   //�Ƿ��ѽ��г�ʼ������

		/****************************************************
		* Name : ReleaseReource()
		* Desc : �����ȫ�ֺ;�̬��Դ�����ͷ�
		****************************************************/
		static void ReleaseReource();
	};
}

#endif		//#ifndef _NET_WORK_H