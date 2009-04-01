#ifndef _TCP_SER_H_
#define _TCP_SER_H_
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

#include <vector>

using namespace std;

namespace HelpMng
{
	class TcpServer;

	class TCP_CONTEXT : public NET_CONTEXT
	{
		friend class TcpServer;
	protected:
		DWORD m_nDataLen;		//TCPģʽ��Ϊ�ۼƽ��ջ��͵����ݳ���

		TCP_CONTEXT()
			: m_nDataLen(0)
		{

		}
		virtual ~TCP_CONTEXT() {}

		void* operator new(size_t nSize);
		void operator delete(void* p);

		enum
		{
			E_TCP_HEAP_SIZE = 1024 * 1024* 10,
			MAX_IDL_DATA = 20000,
		};
	private:
		static vector<TCP_CONTEXT* > s_IDLQue;		//��Ч�����ݶ���
		static CRITICAL_SECTION s_IDLQueLock;		//����s_IDLQue�Ļ�����
		static HANDLE s_hHeap;	//NET_CONTEXT�Լ��Ķ��ڴ�
	};

	/************************************************************************
	* Name: class ACCEPT_CONTEXT
	* Desc: ������Ҫ����listen socket�����绺����
	************************************************************************/
	class ACCEPT_CONTEXT : public NET_CONTEXT
	{
		friend class TcpServer;
	protected:
		SOCKET m_hRemoteSock;			//���ӵ���������SOCKET

		ACCEPT_CONTEXT()
			: m_hRemoteSock(INVALID_SOCKET)
		{

		}

		virtual ~ACCEPT_CONTEXT() {}

		void* operator new(size_t nSize);
		void operator delete(void* p);
	private:
		static vector<ACCEPT_CONTEXT* > s_IDLQue;		//��Ч�����ݶ���
		static CRITICAL_SECTION s_IDLQueLock;			//����s_IDLQue�Ļ�����
		static HANDLE s_hHeap;	//NET_CONTEXT�Լ��Ķ��ڴ�, ������Ϊ800K, ����������20000����NET_CONTEXT����
	};

	class DLLENTRY TCP_RCV_DATA
	{
		friend class TcpServer;
	public:
		SOCKET m_hSocket;		//�յ����ݵ�socket
		CHAR* m_pData;			//���ݻ�����, �Ż�ʱ�ɲ��������ڴ�ķ�ʽʵ��
		INT m_nLen;				//���ݻ������ĳ���

		TCP_RCV_DATA(SOCKET hSock, const CHAR* pBuf, INT nLen);
		~TCP_RCV_DATA();

		void* operator new(size_t nSize);
		void operator delete(void* p);

		enum
		{
			HEAP_SIZE = 1024 *1024* 50,		//�Զ���ѵ��������
			DATA_HEAP_SIZE = 1024 *1024 * 100,
			MAX_IDL_DATA = 100000,
		};

	private:
		static vector<TCP_RCV_DATA* > s_IDLQue;		//��Ч�����ݶ���
		static CRITICAL_SECTION s_IDLQueLock;		//����s_IDLQue�Ļ�����
		static HANDLE s_hHeap;		//Rcv_DATA���Զ����
		static HANDLE s_DataHeap;	//���ݻ��������Զ����, �������Ϊ10M
	};
	
	class DLLENTRY TcpServer
	{
	public:
		TcpServer();
		~TcpServer();

		/************************************************************************
		* Desc : ��ʼ����̬��Դ��������TCPʵ������֮ǰӦ�ȵ��øú���, ��������޷���������
		************************************************************************/
		static void InitReource();

		/************************************************************************
		* Desc : ���ͷ�TCPʵ���Ժ�, ���øú����ͷ���ؾ�̬��Դ
		************************************************************************/
		static void ReleaseReource();

		/****************************************************
		* Name : StartServer()
		* Desc : ��������
		****************************************************/
		BOOL StartServer(
			const char *szIp	//Ҫ��������ı��ص�ַ, NULL �����Ĭ�ϵ�ַ
			, INT nPort	//Ҫ��������Ķ˿�
			, LPCLOSE_ROUTINE pCloseFun		//ĳ��socket�رյĻص�����
			, LPVOID pParam					//�ص�������pParam����
			);

		/****************************************************
		* Name : CloseServer()
		* Desc : �رշ���
		****************************************************/
		void CloseServer();

		/****************************************************
		* Name : SendData()
		* Desc : ��nDataLen���ȵ�����szData��ָ��������ӿ�sock�Ϸ���ȥ���ݵ�˳���������ϲ����
		* ע��ϵͳ�������szData���ͷ�, �ͷŲ������û��Լ����. �����������Ժ��ɽ����ͷŲ���
		 * nDataLen�ĳ��Ȳ�Ҫ����4096
		****************************************************/
		BOOL SendData(SOCKET hSock, const CHAR* szData, INT nDataLen);

		/****************************************************
		* Name : GetRcvData()
		* Desc : �ӽ������ݶ����л�ȡһ���������ݰ�; pQueLen ��Ϊ��ʱ���������ݶ��еĳ���.	
		* ע�����û�ʹ�������ݰ���Ӧ�����ͷŵ�, �Է�ֹ�ڴ��й©
		****************************************************/
		TCP_RCV_DATA* GetRcvData(
			DWORD* const pQueLen
			);
	protected:
		enum
		{
			LISTEN_EVENTS = 2,					//����socket���¼�����
			MAX_ACCEPT = 50,
			_SOCK_NO_RECV = 0xf0000000,		//socket�����ӵ�δ��������
			_SOCK_RECV = 0xf0000001				//socket�����Ӳ���Ҳ����������
		};

		vector<TCP_RCV_DATA* > m_RcvDataQue;			//�������ݻ���������
		CRITICAL_SECTION m_RcvQueLock;					//����s_RcvDataQue�Ļ�����

		vector<SOCKET> m_SocketQue;							//���ӵ��������Ŀͻ���socket����
		CRITICAL_SECTION m_SockQueLock;				//����m_SocketQue�Ļ�����

		LPCLOSE_ROUTINE m_pCloseFun;					//����رյĻص�֪ͨ����
		LPVOID m_pCloseParam;							//���ݸ�m_pCloseFun�Ĳ���

		SOCKET m_hSock;									//���������socket
		long volatile m_bThreadRun;								//�Ƿ������̨�߳��Ƿ��������
		long volatile m_nAcceptCount;				//accept��Ͷ�ݼ�����
		BOOL m_bSerRun;									//�����Ƿ�����

		//����Ͷ��accept���¼�����
		HANDLE m_ListenEvents[LISTEN_EVENTS];
		HANDLE *m_pThreads;				//�߳�����
		HANDLE m_hCompletion;					//��ɶ˿ھ��

		static LPFN_ACCEPTEX s_pfAcceptEx;				//AcceptEx������ַ
		static LPFN_GETACCEPTEXSOCKADDRS s_pfGetAddrs;	//GetAcceptExSockaddrs�����ĵ�ַ


		/****************************************************
		* Name : PushInRecvQue()
		* Desc : �����ܵ������ݷ�����ܶ�����
		****************************************************/
		void PushInRecvQue(TCP_RCV_DATA* const pRcvData);

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
		* Name : WorkThread()
		* Desc : I/O ��̨�����߳�
		****************************************************/
		static UINT WINAPI WorkThread(LPVOID lpParam);

		/****************************************************
		* Name : AideThread()
		* Desc : ��̨�����߳�, ���߳���ʱ����
		****************************************************/
		static UINT WINAPI AideThread(LPVOID lpParam);
	};
}

#endif