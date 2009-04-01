#ifndef _TCP_SER_EX_H
#define _TCP_SER_EX_H

#include "HeadFile.h"
#include "NetWork.h"

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

#include <vector>

using namespace std;

namespace HelpMng
{
	class TcpSerEx;
	void ReleaseTcpSerEx();

	class TCP_CONTEXT_EX : public NET_CONTEXT
	{
		friend class TcpSerEx;
	protected:
		DWORD m_nDataLen;		//TCPģʽ��Ϊ�ۼƽ��ջ��͵����ݳ���

		TCP_CONTEXT_EX()
			: m_nDataLen(0)
		{

		}
		virtual ~TCP_CONTEXT_EX() {}

		void* operator new(size_t nSize);
		void operator delete(void* p);

		enum
		{
			E_TCP_HEAP_SIZE = 1024 * 1024* 10,
			MAX_IDL_DATA = 20000,
		};
	private:
		static vector<TCP_CONTEXT_EX* > s_IDLQue;		//��Ч�����ݶ���
		static CRITICAL_SECTION s_IDLQueLock;		//����s_IDLQue�Ļ�����
		static HANDLE s_hHeap;	//NET_CONTEXT�Լ��Ķ��ڴ�
	};

	/************************************************************************
	* Name: class ACCEPT_CONTEXT
	* Desc: ������Ҫ����listen socket�����绺����
	************************************************************************/
	class ACCEPT_CONTEXT_EX : public NET_CONTEXT
	{
		friend class TcpSerEx;
	protected:
		SOCKET m_hRemoteSock;			//���ӵ���������SOCKET

		ACCEPT_CONTEXT_EX()
			: m_hRemoteSock(INVALID_SOCKET)
		{
		}

		virtual ~ACCEPT_CONTEXT_EX() {}

		void* operator new(size_t nSize);
		void operator delete(void* p);
	private:
		static vector<ACCEPT_CONTEXT_EX* > s_IDLQue;		//��Ч�����ݶ���
		static CRITICAL_SECTION s_IDLQueLock;			//����s_IDLQue�Ļ�����
		static HANDLE s_hHeap;	//NET_CONTEXT�Լ��Ķ��ڴ�, ������Ϊ800K, ����������20000����NET_CONTEXT����
	};

	class DLLENTRY TCP_RCV_DATA_EX
	{
		friend class TcpSerEx;
	public:
		SOCKET m_hSocket;		//�յ����ݵ�socket
		CHAR* m_pData;			//���ݻ�����
		INT m_nLen;				//���ݻ������ĳ���

		TCP_RCV_DATA_EX(SOCKET hSock, const CHAR* pBuf, INT nLen);
		~TCP_RCV_DATA_EX();

		void* operator new(size_t nSize);
		void operator delete(void* p);

		enum
		{
			HEAP_SIZE = 1024 *1024* 50,		//�Զ���ѵ��������
			DATA_HEAP_SIZE = 1024 *1024 * 100,
			MAX_IDL_DATA = 100000,
		};

	private:
		static vector<TCP_RCV_DATA_EX* > s_IDLQue;		//��Ч�����ݶ���
		static CRITICAL_SECTION s_IDLQueLock;		//����s_IDLQue�Ļ�����
		static HANDLE s_hHeap;		//Rcv_DATA���Զ����
		static HANDLE s_DataHeap;	//���ݻ��������Զ����, �������Ϊ10M
	};

	class DLLENTRY TcpSerEx : public NetWork
	{
		friend void ReleaseTcpSerEx();
	public:
		/****************************************************
		* Name : InitReource()
		* Desc : �����ȫ�ּ���̬��Դ���г�ʼ��
		*	�÷�����DllMain()��������, �����߲��ܵ���
		*	�÷���; ���������ڴ�й¶
		****************************************************/
		static void InitReource();

		TcpSerEx(
			LPCLOSE_ROUTINE pCloseFun		//ĳ��socket�رյĻص�����
			, void *pParam					//�ص�������pParam����
			);
		virtual ~TcpSerEx();

		/****************************************************
		* Name : Init()
		* Desc : ������ģ����г�ʼ��
		****************************************************/
		virtual BOOL Init(
			const char *szIp	//Ҫ��������ı��ص�ַ, NULL �����Ĭ�ϵ�ַ
			, int nPort	//Ҫ��������Ķ˿�
			);

		/****************************************************
		* Name : Close()
		* Desc : �ر�����ģ��
		****************************************************/
		virtual void Close();

		/****************************************************
		* Name : SendData()
		* Desc : TCPģʽ�ķ�������
		****************************************************/
		virtual BOOL SendData(const char * szData, int nDataLen, SOCKET hSock = INVALID_SOCKET);

		/****************************************************
		* Name : GetRecvData()
		* Desc : �ӽ��ն�����ȡ��һ����������
		****************************************************/
		virtual void *GetRecvData(DWORD* const pQueLen);

	protected:
		enum
		{
			LISTEN_EVENTS = 2,	                          //����socket���¼�����
			THREAD_NUM = 2,							//�����̵߳���Ŀ
			MAX_ACCEPT = 50,
			_SOCK_NO_RECV = 0xf0000000,       //socket�����ӵ�δ��������
			_SOCK_RECV = 0xf0000001               //socket�����Ӳ���Ҳ����������
		};

		vector<TCP_RCV_DATA_EX * > m_RcvDataQue;        //�������ݻ���������
		CRITICAL_SECTION m_RcvQueLock;					//����s_RcvDataQue�Ļ�����

		vector<SOCKET> m_SocketQue;							//���ӵ��������Ŀͻ���socket����
		CRITICAL_SECTION m_SockQueLock;				//����m_SocketQue�Ļ�����

		LPCLOSE_ROUTINE m_pCloseFun;					//����رյĻص�֪ͨ����
		LPVOID m_pCloseParam;							//���ݸ�m_pCloseFun�Ĳ���

		long volatile m_bThreadRun;								//�Ƿ������̨�߳��Ƿ��������
		long volatile m_nReadCount;				//������������
		long volatile m_nAcceptCount;				//accept��Ͷ�ݼ�����
		BOOL m_bSerRun;									//�����Ƿ�����

		//����Ͷ��accept���¼�����
		HANDLE m_ListenEvents[LISTEN_EVENTS];
		HANDLE m_Threads[THREAD_NUM];	               //�����̺߳ͼ����߳�

		static LPFN_ACCEPTEX s_pfAcceptEx;				//AcceptEx������ַ
		static LPFN_GETACCEPTEXSOCKADDRS s_pfGetAddrs;	//GetAcceptExSockaddrs�����ĵ�ַ

		/****************************************************
		* Name : IOCompletionProc()
		* Desc : IO��ɺ�Ļص�����
		****************************************************/
		virtual void IOCompletionProc(BOOL bSuccess, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		/****************************************************
		* Name : AcceptCompletionProc()
		* Desc : ���пͻ������ӵ�������ʱ���ô˷���
		****************************************************/
		void AcceptCompletionProc(BOOL bSuccess, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		/****************************************************
		* Name : RecvCompletionProc()
		* Desc : ����ӿ��϶�������ʱ���ô˷���
		****************************************************/
		void RecvCompletionProc(BOOL bSuccess, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		/****************************************************
		* Name : SendCompletionProc()
		* Desc : ���Ͳ������ʱ���ô˷���
		****************************************************/
		void SendCompletionProc(BOOL bSuccess, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		/****************************************************
		* Name : ListenThread()
		* Desc : ��������߳�
		****************************************************/
		static UINT WINAPI ListenThread(LPVOID lpParam);

		/****************************************************
		* Name : AideThread()
		* Desc : ��̨�����߳�, ���߳���ʱ����
		****************************************************/
		static UINT WINAPI AideThread(LPVOID lpParam);
	private:
		static BOOL s_bInit;				//�Ƿ��ʼ�����

		/****************************************************
		* Name : ReleaseReource()
		* Desc : �����ȫ�ֺ;�̬��Դ�����ͷ�
		****************************************************/
		static void ReleaseReource();
	};
}

#endif		//#ifndef _TCP_SER_EX_H