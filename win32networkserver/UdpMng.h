#ifndef _UDP_MNG_H
#define _UDP_MNG_H

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

#include<queue>
#include<vector>
#include<map>

using namespace std;

namespace HelpMng
{

	class CUdpMng;

	class UDP_CONTEXT : protected NET_CONTEXT
	{
		friend class CUdpMng;
	protected:
		IP_ADDR m_RemoteAddr;			//�Զ˵�ַ
		CUdpMng* const m_pMng;			//��UDP_CONTEXT��ص�CUdpMng����ʵ��
		enum
		{
			HEAP_SIZE = 1024 * 1024 * 5,
			MAX_IDL_DATA = 100000,
		};

	public:
		UDP_CONTEXT(CUdpMng* pMng)
			: m_pMng(pMng) {}

		~UDP_CONTEXT() {}

		void* operator new(size_t nSize);
		void operator delete(void* p);

	private:
		static vector<UDP_CONTEXT* > s_IDLQue;
		static CRITICAL_SECTION s_IDLQueLock;
		static HANDLE s_hHeap;	

		//�жϸ����ĳ��ָ������Ƿ���Ч
		static BOOL IsAddressValid(LPCVOID lpMem);
	};

	class DLLENTRY UDP_RCV_DATA
	{
		friend class CUdpMng;
	public:
		CHAR* m_pData;				//���ݻ�����
		INT m_nLen;					//���ݵĳ���
		IP_ADDR m_PeerAddr;			//���ͱ��ĵĵ�ַ

		UDP_RCV_DATA(const CHAR* szBuf, int nLen, const IP_ADDR& PeerAddr);
		~UDP_RCV_DATA();

		void* operator new(size_t nSize);
		void operator delete(void* p);

		//�ж�RCV_DATA��ĳ������ָ���Ƿ���Ч
		static BOOL IsAddressValid(LPCVOID pMem);
		//�ж�m_pData�Ƿ���Ч
		BOOL IsDataVailid();

		enum
		{
			RCV_HEAP_SIZE = 1024 * 1024 *7,		//s_Heap�ѵĴ�С
			DATA_HEAP_SIZE = 50 * 1024* 1024,	//s_DataHeap�ѵĴ�С
			MAX_IDL_DATA = 250000,
		};

	private:
		static vector<UDP_RCV_DATA* > s_IDLQue;
		static CRITICAL_SECTION s_IDLQueLock;
		static HANDLE s_DataHeap;		//���ݻ������Ķ�
		static HANDLE s_Heap;			//RCV_DATA�Ķ�
	};

	class DLLENTRY CUdpMng
	{
	public:	
		CUdpMng();
		~CUdpMng(void);

		/************************************************************************
		* Desc : ��ʼ����̬��Դ��������TCPʵ������֮ǰӦ�ȵ��øú���, ��������޷���������
		************************************************************************/
		static void InitReource();

		/************************************************************************
		* Desc : ���ͷ�TCPʵ���Ժ�, ���øú����ͷ���ؾ�̬��Դ
		************************************************************************/
		static void ReleaseReource();

		//��ָ�����ص�ַ�Ͷ˿ڽ��г�ʼ��
		BOOL Init(const CHAR* szIp = "0.0.0.0", INT nPort = 0);

		//�����ݶ��е�ͷ����ȡһ����������, pCount��Ϊnullʱ���ض��еĳ���
		UDP_RCV_DATA* GetRcvDataFromQue(DWORD* pCount);

		//��Զ˷�������
		BOOL SendData(const IP_ADDR& PeerAddr, const CHAR* szData, INT nLen);
	protected:	
		SOCKET m_hSock;
		vector<UDP_RCV_DATA* > m_RcvDataQue;				//�������ݶ���
		CRITICAL_SECTION m_RcvDataLock;						//����m_RcvDataQue�Ļ�����

		//IO�̳߳ش�����
		static void CALLBACK IOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
		static void ReadCompletion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
		static void SendCompletion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		//������ѹ�뵽���ݰ�������
		void PushInRcvDataQue(const CHAR* szBuf, INT nLen, const IP_ADDR& addr);

		//Ͷ��nNum��Recv����
		void PostRecv(INT nNum);
	};
}

#endif			//#ifndef _UDP_MNG_H