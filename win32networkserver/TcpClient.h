#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include "HeadFile.h"
#include "NetWork.h"

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

#include<vector>

using namespace std;

namespace HelpMng
{
	void ReleaseTcpClient();

	class TCP_CLIENT_EX : public NET_CONTEXT
	{
		friend class TcpClient;
	protected:
		DWORD m_nDataLen;		//TCPģʽ��Ϊ�ۼƽ��ջ��͵����ݳ���

		TCP_CLIENT_EX()
			: m_nDataLen(0)
		{
		}
		virtual ~TCP_CLIENT_EX() {}

		void* operator new(size_t nSize);
		void operator delete(void* p);

		enum
		{
			E_TCP_HEAP_SIZE = 1024 * 1024* 10,
			MAX_IDL_DATA = 20000,
		};
	private:
		static vector<TCP_CLIENT_EX* > s_IDLQue;		//��Ч�����ݶ���
		static CRITICAL_SECTION s_IDLQueLock;		//����s_IDLQue�Ļ�����
		static HANDLE s_hHeap;	//NET_CONTEXT�Լ��Ķ��ڴ�
	};

	class DLLENTRY TCP_CLIENT_RCV_EX
	{
		friend class TcpClient;
	public:
		CHAR* m_pData;			//���ݻ�����, �Ż�ʱ�ɲ��������ڴ�ķ�ʽʵ��
		INT m_nLen;				//���ݻ������ĳ���

		TCP_CLIENT_RCV_EX(const CHAR* pBuf, INT nLen);
		~TCP_CLIENT_RCV_EX();

		void* operator new(size_t nSize);
		void operator delete(void* p);

		enum
		{
			HEAP_SIZE = 1024 *1024* 50,		//�Զ���ѵ��������
			DATA_HEAP_SIZE = 1024 *1024 * 100,
			MAX_IDL_DATA = 100000,
		};

	private:
		static vector<TCP_CLIENT_RCV_EX* > s_IDLQue;		//��Ч�����ݶ���
		static CRITICAL_SECTION s_IDLQueLock;		//����s_IDLQue�Ļ�����
		static HANDLE s_hHeap;		//Rcv_DATA���Զ����
		static HANDLE s_DataHeap;	//���ݻ��������Զ����, �������Ϊ10M
	};

	class DLLENTRY TcpClient : public NetWork
	{
		friend void ReleaseTcpClient();

	public:
		/****************************************************
		* Name : InitReource()
		* Desc : �����ȫ�ּ���̬��Դ���г�ʼ��
		*	�÷�����DllMain()��������, �����߲��ܵ���
		*	�÷���; ���������ڴ�й¶
		****************************************************/
		static void InitReource();

		//* ע : �벻Ҫ�ص�����������ʱ����, ����ᵼ��I/O�߳�����
		TcpClient(
			LPCLOSE_ROUTINE pCloseFun		//ĳ��socket�رյĻص�����
			, void *pParam					//�ص�������pParam����
			, LPONCONNECT pConnFun		//���ӳɹ��Ļص�����
			, void *pConnParam			//pConnFun �Ĳ���
			);
		virtual ~TcpClient();

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
		* Name : IsConnected()
		* Desc : �ͻ��������Ƿ�����
		***********************************************/
		BOOL IsConnected()const { return m_bConnected; }

		void* operator new(size_t nSize);
		void operator delete(void* p);

		void *operator new(size_t nSize, const char *, int)
		{
			return operator new(nSize);
		}

		void operator delete(void *p, const char *, int)
		{
			operator delete(p);
		}
	protected:
		vector<TCP_CLIENT_RCV_EX* > m_RcvDataQue;        //�������ݻ���������
		CRITICAL_SECTION m_RcvQueLock;					//����s_RcvDataQue�Ļ�����

		LPCLOSE_ROUTINE m_pCloseFun;					//����رյĻص�֪ͨ����
		LPVOID m_pCloseParam;							//���ݸ�m_pCloseFun�Ĳ���

		LPONCONNECT m_pConnFun;               //���ӳɹ��Ļص�����
		void *m_pConnParam;

		BOOL m_bConnected;					//�ͻ��������Ƿ�����

		/****************************************************
		* Name : IOCompletionProc()
		* Desc : IO��ɺ�Ļص�����
		****************************************************/
		virtual void IOCompletionProc(BOOL bSuccess, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) ;

		/****************************************************
		* Name : OnConnect()
		* Desc : ���ӵĻص�����
		****************************************************/
		void OnConnect(BOOL bSuccess, DWORD dwTrans, LPOVERLAPPED lpOverlapped);

		/****************************************************
		* Name : OnRead() 
		* Desc : ��������ɵĻص�����
		****************************************************/
		void OnRead(BOOL bSuccess, DWORD dwTrans, LPOVERLAPPED lpOverlapped);

	private:
		static HANDLE s_hHeap;			//�����������ʵ���Ķ�
		static vector<TcpClient*> s_IDLQue;		//���е�TcpClient����
		static CRITICAL_SECTION s_IDLQueLock;	//����s_IDLQue���ݶ��л�����
		static LPFN_CONNECTEX s_pfConnectEx;		//ConnectEx�����ĵ�ַ
		static BOOL s_bInit;					//�Ƿ��Ѿ���ʼ��

		enum
		{
			E_BUF_SIZE = 4096,
			E_HEAP_SIZE = 800 *1024,		 //����ʵ��������
			E_MAX_IDL_NUM = 2000,			 //����ʵ�����е���󳤶�
		};

		/****************************************************
		* Name : ReleaseReource()
		* Desc : �����ȫ�ֺ;�̬��Դ�����ͷ�
		****************************************************/
		static void ReleaseReource();
	};
}

#endif				//#ifndef _TCP_CLIENT_H_