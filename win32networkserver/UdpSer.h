#ifndef _UDP_SER_H_
#define _UDP_SER_H_
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

#include<vector>

using namespace std;

namespace HelpMng
{
	class UDP_CONTEXT : protected NET_CONTEXT
	{
		friend class UdpSer;
	protected:
		IP_ADDR m_RemoteAddr;			//�Զ˵�ַ

		enum
		{
			HEAP_SIZE = 1024 * 1024 * 5,
			MAX_IDL_DATA = 10000,
		};

	public:
		UDP_CONTEXT() {}
		virtual ~UDP_CONTEXT() {}

		void* operator new(size_t nSize);
		void operator delete(void* p);

	private:
		static vector<UDP_CONTEXT* > s_IDLQue;
		static CRITICAL_SECTION s_IDLQueLock;
		static HANDLE s_hHeap;	
	};

	class DLLENTRY UDP_RCV_DATA
	{
		friend class UdpSer;
	public:
		CHAR* m_pData;				//���ݻ�����
		INT m_nLen;					//���ݵĳ���
		IP_ADDR m_PeerAddr;			//���ͱ��ĵĵ�ַ

		UDP_RCV_DATA(const CHAR* szBuf, int nLen, const IP_ADDR& PeerAddr);
		~UDP_RCV_DATA();

		void* operator new(size_t nSize);
		void operator delete(void* p);

		enum
		{
			RCV_HEAP_SIZE = 1024 * 1024 *50,		//s_Heap�ѵĴ�С
			DATA_HEAP_SIZE = 100 * 1024* 1024,	//s_DataHeap�ѵĴ�С
			MAX_IDL_DATA = 250000,
		};

	private:
		static vector<UDP_RCV_DATA* > s_IDLQue;
		static CRITICAL_SECTION s_IDLQueLock;
		static HANDLE s_DataHeap;		//���ݻ������Ķ�
		static HANDLE s_Heap;			//RCV_DATA�Ķ�
	};

	class DLLENTRY UdpSer
	{
	public:
		UdpSer();
		~UdpSer();

		/************************************************************************
		* Desc : ��ʼ����̬��Դ��������UDPʵ������֮ǰӦ�ȵ��øú���, ��������޷���������
		************************************************************************/
		static void InitReource();

		/************************************************************************
		* Desc : ���ͷ�UDPʵ���Ժ�, ���øú����ͷ���ؾ�̬��Դ
		************************************************************************/
		static void ReleaseReource();

		//��ָ�����ص�ַ�Ͷ˿ڽ��г�ʼ��
		BOOL StartServer(const CHAR* szIp = "0.0.0.0", INT nPort = 0);

		//�����ݶ��е�ͷ����ȡһ����������, pCount��Ϊnullʱ���ض��еĳ���
		UDP_RCV_DATA* GetRcvData(DWORD* pCount);

		//��Զ˷�������
		BOOL SendData(const IP_ADDR& PeerAddr, const CHAR* szData, INT nLen);

		/****************************************************
		* Name : CloseServer()
		* Desc : �رշ�����
		****************************************************/
		void CloseServer();

	protected:
		SOCKET m_hSock;
		vector<UDP_RCV_DATA* > m_RcvDataQue;				//�������ݶ���
		CRITICAL_SECTION m_RcvDataLock;						//����m_RcvDataQue�Ļ�����
		long volatile m_bThreadRun;								//�Ƿ������̨�̼߳�������
		BOOL m_bSerRun;											//�������Ƿ���������

		HANDLE *m_pThreads;				//�߳�����
		HANDLE m_hCompletion;					//��ɶ˿ھ��

		void ReadCompletion(BOOL bSuccess, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		/****************************************************
		* Name : WorkThread()
		* Desc : I/O ��̨�����߳�
		****************************************************/
		static UINT WINAPI WorkThread(LPVOID lpParam);
	};
}
#endif		//#ifndef _UDP_SER_H_