#ifndef _TCP_MNG_H
#define _TCP_MNG_H

#include "HeadFile.h"

#pragma warning (disable:4251)

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

#include <vector>
#include <map>

using namespace std;

namespace HelpMng
{
	class CTcpMng;
	class SOCKET_CONTEXT;
    
	typedef map<SOCKET, SOCKET_CONTEXT*> CSocketMap;

	class TCP_CONTEXT : public NET_CONTEXT
	{
		friend class CTcpMng;
	protected:
		DWORD m_nDataLen;		//TCPģʽ��Ϊ�ۼƽ��ջ��͵����ݳ���
		CTcpMng* const m_pMng;

		TCP_CONTEXT(CTcpMng* pMng)
			: m_pMng(pMng)
			, m_nDataLen(0)
		{

		}
		virtual ~TCP_CONTEXT() {}

		void* operator new(size_t nSize);
		void operator delete(void* p);

		enum
		{
			E_TCP_HEAP_SIZE = 1024 * 1024* 2,
			MAX_IDL_DATA = 40000,
		};
	private:
		static vector<TCP_CONTEXT* > s_IDLQue;		//��Ч�����ݶ���
		static CRITICAL_SECTION s_IDLQueLock;		//����s_IDLQue�Ļ�����
		static HANDLE s_hHeap;	//NET_CONTEXT�Լ��Ķ��ڴ�, ������Ϊ800K, ����������20000����NET_CONTEXT����

		//�жϸ����ĳ��ָ������Ƿ���Ч
		static BOOL IsAddressValid(LPCVOID lpMem);
	};

	/************************************************************************
	* Name: class ACCEPT_CONTEXT
	* Desc: ������Ҫ����listen socket�����绺����
	************************************************************************/
	class ACCEPT_CONTEXT : public NET_CONTEXT
	{
		friend class CTcpMng;

	protected:
		SOCKET m_hRemoteSock;			//���ӵ���������SOCKET
		CTcpMng *const m_pMng;

		ACCEPT_CONTEXT(CTcpMng* pMng)
			: m_pMng(pMng) 
			, m_hRemoteSock(INVALID_SOCKET)
		{

		}

		virtual ~ACCEPT_CONTEXT() {}

		void* operator new(size_t nSize);
		void operator delete(void* p);
	private:
		static vector<ACCEPT_CONTEXT* > s_IDLQue;		//��Ч�����ݶ���
		static CRITICAL_SECTION s_IDLQueLock;			//����s_IDLQue�Ļ�����
		static HANDLE s_hHeap;	//NET_CONTEXT�Լ��Ķ��ڴ�, ������Ϊ800K, ����������20000����NET_CONTEXT����

		//�жϸ����ĳ��ָ������Ƿ���Ч
		static BOOL IsAddressValid(LPCVOID lpMem);
	};

	class DLLENTRY TCP_RCV_DATA
	{
		friend class CTcpMng;

	public:
		SOCKET_CONTEXT *m_pSocket;	//�յ����ݵ�socket
		CHAR* m_pData;			//���ݻ�����, �Ż�ʱ�ɲ��������ڴ�ķ�ʽʵ��
		INT m_nLen;				//���ݻ������ĳ���

		TCP_RCV_DATA(SOCKET_CONTEXT *pSocket, const CHAR* pBuf, INT nLen);
		~TCP_RCV_DATA();

		void* operator new(size_t nSize);
		void operator delete(void* p);

		static BOOL IsAddressValid(LPCVOID pMem);
		BOOL IsDataValid();

		enum
		{
			HEAP_SIZE = 1024 *1024* 5,		//�Զ���ѵ��������
			DATA_HEAP_SIZE = 1024 *1024 * 50,
			MAX_IDL_DATA = 50000,
		};

	private:
		static vector<TCP_RCV_DATA* > s_IDLQue;		//��Ч�����ݶ���
		static CRITICAL_SECTION s_IDLQueLock;		//����s_IDLQue�Ļ�����
		static HANDLE s_hHeap;		//Rcv_DATA���Զ����, �����Դ���20���TCP_RCV_DATA����
		static HANDLE s_DataHeap;	//���ݻ��������Զ����, �������Ϊ10M
	};
	
	class DLLENTRY SOCKET_CONTEXT
	{
		friend class CTcpMng;
	public:
		const IP_ADDR &GetPeerAddr()const
		{
			return m_Addr;
		}

	protected:
		IP_ADDR m_Addr;				//��socket����Ӧ�ĶԶ˵�ַ
		DWORD m_nInteractiveTime;	//socket���ϴν���ʱ��
		SOCKET m_hSocket;			//��ص�����ӿ�

		SOCKET_CONTEXT(const IP_ADDR& peer_addr, const SOCKET &sock);
		~SOCKET_CONTEXT() {}

		void* operator new(size_t nSize);
		void operator delete(void* p);

		enum
		{
			HEAP_SIZE = 500 *1024,
			MAX_IDL_SIZE = 20000,
		};
	private:
		static vector<SOCKET_CONTEXT*> s_IDLQue;
		static CRITICAL_SECTION s_IDLQueLock;
		static HANDLE s_hHeap;			//�Զ������ݶ�
	};

	class DLLENTRY CTcpMng
	{
	public:
		CTcpMng(void);
		~CTcpMng(void);


		/************************************************************************
		* Desc : ��ʼ����̬��Դ��������TCPʵ������֮ǰӦ�ȵ��øú���, ��������޷���������
		************************************************************************/
		static void InitReource();

		/************************************************************************
		* Desc : ���ͷ�TCPʵ���Ժ�, ���øú����ͷ���ؾ�̬��Դ
		************************************************************************/
		static void ReleaseReource();

		//����һ��TCP����, �����������ɹ�����TRUE, ���򷵻�FALSE
		BOOL StartServer(
			INT nPort
			, LPCLOSE_ROUTINE pCloseFun		//ĳ��socket�رյĻص�����
			, LPVOID pParam					//�ص�������pParam����
			);

		//�رշ���
		void CloseServer();
		//��nDataLen���ȵ�����szData��ָ��������ӿ�sock�Ϸ���ȥ���ݵ�˳���������ϲ����
		//ע��ϵͳ�������szData���ͷ�, �ͷŲ������û��Լ����. �����������Ժ��ɽ����ͷŲ���
		// nDataLen�ĳ��Ȳ�Ҫ����4094
		BOOL SendData(SOCKET_CONTEXT *const pSocket, const CHAR* szData, INT nDataLen);

		//�ӽ������ݶ����л�ȡһ���������ݰ�; pQueLen ��Ϊ��ʱ���������ݶ��еĳ���.	
		//ע�����û�ʹ�������ݰ���Ӧ�����ͷŵ�, �Է�ֹ�ڴ��й©
		TCP_RCV_DATA* GetRcvData(
			DWORD* const pQueLen
			, const SOCKET_CONTEXT *pSocket = NULL		//Ҫ���ĸ�socket�ϻ�ȡ���� ֻ���ַ�����ʽ��������Ч
			);

	protected:
		vector<TCP_RCV_DATA* > m_RcvDataQue;			//�������ݻ���������
		CRITICAL_SECTION m_RcvQueLock;					//����s_RcvDataQue�Ļ�����

		vector<ACCEPT_CONTEXT* > m_AcceptQue;			//����SOCKET�������������

		//CSendMap m_SendDataMap;							//�������ݵ�MAP
		//CRITICAL_SECTION m_SendDataMapLock;				//����m_SendDataMapLock�Ļ�����

		CSocketMap m_SocketMap;							//�ͻ��˵�socket���map
		CSocketMap::iterator m_SockIter;				//����m_SocketMap�ı�����
		CRITICAL_SECTION m_SockMapLock;					//����m_SocketMap�Ļ�����

		LPCLOSE_ROUTINE m_pCloseFun;					//����رյĻص�֪ͨ����
		LPVOID m_pCloseParam;							//���ݸ�m_pCloseFun�Ĳ���
		static LPFN_ACCEPTEX s_pfAcceptEx;				//AcceptEx������ַ
		static LPFN_CONNECTEX s_pfConnectEx;			//ConnectEx�����ĵ�ַ
		static LPFN_GETACCEPTEXSOCKADDRS s_pfGetAddrs;	//GetAcceptExSockaddrs�����ĵ�ַ

		SOCKET m_hSock;									//���������socket
		BOOL m_bThreadRun;								//�Ƿ������̨�߳��Ƿ��������

		//������ѹ��������ݶ�����
		void PushInRecvQue(TCP_RCV_DATA* const pRcvData);

		//���յ����ͽ���������Ҫ�޸Ľ���ʱ��
		void ModifyInteractiveTime(const SOCKET& s);

		//�������ӵ���socketѹ�뵽socket map��
		inline void PushInSocketMap(const SOCKET& s, SOCKET_CONTEXT *pSocket);

		//��socket������ɾ����ص�socket��������
		void DelSocketContext(const SOCKET &s);

		//IO�̳߳ش�����
		static void CALLBACK IOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		//��AcceptEx�����������Ҫ���еĴ���
		static void AcceptCompletionProc(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		//��WSARecv�����������Ҫ���еĴ���
		static void RecvCompletionProc(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		//��WSASend�����������Ҫ���еĴ���
		static void SendCompletionProc(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		//��̨�̴߳�����
		static UINT WINAPI ThreadProc(LPVOID lpParam);

		//��ȡһ����Ч��socket��������
		SOCKET_CONTEXT *GetSocketContext(const SOCKET &sock);
	};
}

#endif			//#ifndef _TCP_MNG_H
