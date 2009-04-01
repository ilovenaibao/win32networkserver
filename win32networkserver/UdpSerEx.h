#ifndef _UDP_SER_EX_H
#define _UDP_SER_EX_H
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
	void ReleaseUdpSerEx();

	class UDP_CONTEXT_EX : protected NET_CONTEXT
	{
		friend class UdpSerEx;
	protected:
		IP_ADDR m_RemoteAddr;			//�Զ˵�ַ

		enum
		{
			HEAP_SIZE = 1024 * 1024 * 5,
			MAX_IDL_DATA = 10000,
		};

	public:
		UDP_CONTEXT_EX() {}
		virtual ~UDP_CONTEXT_EX() {}

		void* operator new(size_t nSize);
		void operator delete(void* p);

	private:
		static vector<UDP_CONTEXT_EX* > s_IDLQue;
		static CRITICAL_SECTION s_IDLQueLock;
		static HANDLE s_hHeap;	
	};

	class DLLENTRY UDP_RCV_DATA_EX
	{
		friend class UdpSerEx;
	public:
		CHAR* m_pData;				//���ݻ�����
		INT m_nLen;					//���ݵĳ���
		IP_ADDR m_PeerAddr;			//���ͱ��ĵĵ�ַ

		UDP_RCV_DATA_EX(const CHAR* szBuf, int nLen, const IP_ADDR& PeerAddr);
		~UDP_RCV_DATA_EX();

		void* operator new(size_t nSize);
		void operator delete(void* p);

		enum
		{
			RCV_HEAP_SIZE = 1024 * 1024 *50,		//s_Heap�ѵĴ�С
			DATA_HEAP_SIZE = 100 * 1024* 1024,	//s_DataHeap�ѵĴ�С
			MAX_IDL_DATA = 250000,
		};

	private:
		static vector<UDP_RCV_DATA_EX* > s_IDLQue;
		static CRITICAL_SECTION s_IDLQueLock;
		static HANDLE s_DataHeap;		//���ݻ������Ķ�
		static HANDLE s_Heap;			//RCV_DATA�Ķ�
	};

	//�ͻ��˵�UDP�����Ҳʹ�ø���ʵ��
	class DLLENTRY UdpSerEx : public NetWork
	{
		friend void ReleaseUdpSerEx();
	public:
		/****************************************************
		* Name : InitReource()
		* Desc : �����ȫ�ּ���̬��Դ���г�ʼ��
		*	�÷�����DllMain()��������, �����߲��ܵ���
		*	�÷���; ���������ڴ�й¶
		****************************************************/
		static void InitReource();

		//	* ע: �벻Ҫ��pCloseFun������ʱ�Ĳ���, �����ʹIO�߳�����
		UdpSerEx(
			LPCLOSE_ROUTINE pCloseFun		//ĳ��socket�رյĻص�����
			, void *pParam					//�ص�������pParam����
			, BOOL bClient = FALSE	//��UDP����ģ���Ƿ�Ϊ�ͻ�������ģ��
			);
		virtual ~UdpSerEx();

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
		* Desc : UDPģʽ�����ݷ���
		****************************************************/
		virtual BOOL SendData(const IP_ADDR& PeerAddr, const char * szData, int nLen);

		/****************************************************
		* Name : GetRecvData()
		* Desc : �ӽ��ն�����ȡ��һ����������
		****************************************************/
		virtual void *GetRecvData(DWORD* const pQueLen);

		void * operator new(size_t nSize);
		void operator delete(void* p);		

		void *operator new(size_t nSize, const char *, long)
		{
			return operator new(nSize);
		}
		void operator delete(void *p, const char *, long)
		{
			operator delete(p);
		}

		void *operator new[](size_t nSize)
		{
			return operator new(nSize);
		}
		void operator delete[](void *p)
		{
			operator delete(p);
		}

	protected:
		BOOL m_bSerRun;											//�������Ƿ���������
		const BOOL c_bClient;												//��UDP����ʵ���Ƿ�Ϊ�ͻ���
		long volatile m_lReadCount;								//������������
		vector<UDP_RCV_DATA_EX* > m_RcvDataQue;				//�������ݶ���
		CRITICAL_SECTION m_RcvDataLock;						//����m_RcvDataQue�Ļ�����

		LPCLOSE_ROUTINE m_pCloseProc;                         //socket�ر�֪ͨ����
		void *m_pCloseParam;

		void ReadCompletion(BOOL bSuccess, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		/****************************************************
		* Name : IOCompletionProc()
		* Desc : IO��ɺ�Ļص�����
		****************************************************/
		virtual void IOCompletionProc(BOOL bSuccess, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

	private:
		static HANDLE s_hHeap;			//�����������ʵ���Ķ�
		static vector<UdpSerEx*> s_IDLQue;		//���е�UdpSerEx����
		static CRITICAL_SECTION s_IDLQueLock;	//����s_IDLQue���ݶ��л�����
		static BOOL s_bInit;			//�Ƿ��Ѿ���ʼ��

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

#endif			//#ifndef _UDP_SER_EX_H