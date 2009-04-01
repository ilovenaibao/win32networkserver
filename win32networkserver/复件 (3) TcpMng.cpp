#include "TcpMng.h"
#include <process.h>
#include <assert.h>

#ifdef WIN32
#include <atltrace.h>

#ifdef _DEBUG
#define _TRACE ATLTRACE
#else
#define _TRACE 
#endif		//#ifdef _DEBUG

#endif		//#ifdef WIN32

#ifndef _XML_NET_
#define _XML_NET_
#endif
/**
* ��Ҫ����XML����򿪺꿪�� _XML_NET_ 
* ��������ģ�齫�����������ݽ��д���, 
* �����������ͷ�μ� HeadFile.h�е� 
* PACKET_HEAD ����
*/

namespace HelpMng
{
	vector<TCPMNG_CONTEXT* > TCPMNG_CONTEXT::s_IDLQue;
	CRITICAL_SECTION TCPMNG_CONTEXT::s_IDLQueLock;
	HANDLE TCPMNG_CONTEXT::s_hHeap = NULL;
	volatile LONG TCPMNG_CONTEXT::s_nCount = 0;

	vector<CONNECT_CONTEXT* > CONNECT_CONTEXT::s_IDLQue;
	CRITICAL_SECTION CONNECT_CONTEXT::s_IDLQueLock;
	HANDLE CONNECT_CONTEXT::s_hHeap = NULL;
	volatile LONG CONNECT_CONTEXT::s_nCount = 0;

	vector<ACCEPT_CONTEXT* > ACCEPT_CONTEXT::s_IDLQue;
	CRITICAL_SECTION ACCEPT_CONTEXT::s_IDLQueLock;
	HANDLE ACCEPT_CONTEXT::s_hHeap = NULL;
	volatile LONG ACCEPT_CONTEXT::s_nCount = 0;	

	HANDLE TCP_RCV_DATA::s_DataHeap = NULL;
	HANDLE TCP_RCV_DATA::s_hHeap = NULL;
	volatile LONG TCP_RCV_DATA::s_nCount = 0;
	vector<TCP_RCV_DATA* > TCP_RCV_DATA::s_IDLQue;
	CRITICAL_SECTION TCP_RCV_DATA::s_IDLQueLock;

	HANDLE TCP_SEND_DATA::s_hHeap = NULL;
	HANDLE TCP_SEND_DATA::s_DataHeap = NULL;
	volatile LONG TCP_SEND_DATA::s_nCount = 0;
	vector<TCP_SEND_DATA* > TCP_SEND_DATA::s_IDLQue;
	CRITICAL_SECTION TCP_SEND_DATA::s_IDLQueLock;

	HANDLE TCP_CONTEXT::s_hHeap = NULL;
	volatile LONG TCP_CONTEXT::s_nCount = 0;
	vector<TCP_CONTEXT* > TCP_CONTEXT::s_IDLQue;
	CRITICAL_SECTION TCP_CONTEXT::s_IDLQueLock;
	
	volatile LONG CTcpMng::s_nCount = 0;							
	LPFN_ACCEPTEX CTcpMng::s_pfAcceptEx = NULL;
	LPFN_CONNECTEX CTcpMng::s_pfConnectEx = NULL;
	//LPFN_GETACCEPTEXSOCKADDRS CTcpMng::s_pfGetAddrs = NULL;

	//class TCPMNG_CONTEXT�����ʵ��

	void* TCPMNG_CONTEXT::operator new(size_t nSize)
	{
		void* pContext = NULL;

		try
		{
			if (NULL == s_hHeap)
			{
				throw ((long)(__LINE__));
			}

			//������ڴ�, �ȴӿ��еĶ������������ж���Ϊ����Ӷ��ڴ�������
			//bool bQueEmpty = true;

			EnterCriticalSection(&s_IDLQueLock);
			vector<TCPMNG_CONTEXT* >::iterator iter = s_IDLQue.begin();
	
			if (iter != s_IDLQue.end())
			{
				pContext = *iter;
				s_IDLQue.erase(iter);	
				//bQueEmpty = false;
			}
			else
			{
				pContext = HeapAlloc(s_hHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, nSize);
			}
			LeaveCriticalSection(&s_IDLQueLock);

			//if (bQueEmpty)
			//{
			//	pContext = HeapAlloc(s_hHeap, HEAP_ZERO_MEMORY, nSize);
			//}

			if (NULL == pContext)
			{
				throw ((long)(__LINE__));
			}
		}
		catch (const long& iErrCode)
		{
			InterlockedExchangeAdd(&s_nCount, -1);
			pContext = NULL;
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}

		return pContext;
	}

	void TCPMNG_CONTEXT::operator delete(void* p)
	{
		if (p)		
		{
			//�����ж��г���С��10000, ���������ж�����; �����ͷ�

			EnterCriticalSection(&s_IDLQueLock);
			const DWORD QUE_SIZE = (DWORD)(s_IDLQue.size());
			TCPMNG_CONTEXT* const pContext = (TCPMNG_CONTEXT*)p;

			if (QUE_SIZE <= MAX_IDL_DATA)
			{
				s_IDLQue.push_back(pContext);
			}
			else
			{
				HeapFree(s_hHeap, HEAP_NO_SERIALIZE, p);
			}
			LeaveCriticalSection(&s_IDLQueLock);

			//if (QUE_SIZE > MAX_IDL_DATA)
			//{
			//	HeapFree(s_hHeap, 0, p);
			//}
			//InterlockedExchangeAdd(&s_nCount, -1);

			//�Ѿ�û�ж����ٱ��ͷ���Ҫ�����ͷ�
			//if (0 >= s_nCount)
			//{
			//	HeapDestroy(s_hHeap);
			//	s_hHeap = NULL;

			//	EnterCriticalSection(&s_IDLQueLock);
			//	s_IDLQue.clear();
			//	LeaveCriticalSection(&s_IDLQueLock);

			//	DeleteCriticalSection(&s_IDLQueLock);
			//}
		}

		return;
	}

	BOOL TCPMNG_CONTEXT::IsAddressValid(LPCVOID lpMem)
	{
		BOOL bResult = HeapValidate(s_hHeap, 0, lpMem);

		if (NULL == lpMem)
		{
			bResult = FALSE;
		}	

		return bResult;
	}

	//class CONNECT_CONTEXT�����ʵ��
	CONNECT_CONTEXT::CONNECT_CONTEXT(LPCONNECT_ROUTINE lpConProc, LPVOID pParam, CTcpMng* pMng)
		: TCPMNG_CONTEXT(pMng)
	{
		m_pConProc = lpConProc;
		m_pParam = pParam;
	}

	void* CONNECT_CONTEXT:: operator new(size_t nSize)
	{
		void* pContext = NULL;

		try
		{
			//��û�ж������������Ҫ�ȴ�����
			//if (0 == InterlockedExchangeAdd(&s_nCount, 1))
			//{			
			//	s_hHeap = HeapCreate(0, BUF_SIZE, HEAP_SIZE);
			//	InitializeCriticalSection(&s_IDLQueLock);
			//}
			if (NULL == s_hHeap)
			{
				throw ((long)(__LINE__));
			}

			//������ڴ�, ���ڿ��ж�����Ѱ��, �����ж�����û��ʱ���ڶ�������
			//bool bQueEmpty = true;

			EnterCriticalSection(&s_IDLQueLock);
			vector<CONNECT_CONTEXT* >::iterator iter = s_IDLQue.begin();

			if (s_IDLQue.end() != iter)
			{
				pContext = *iter;
				s_IDLQue.erase(iter);
				//bQueEmpty = false;
			}
			else
			{
				pContext = HeapAlloc(s_hHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, nSize);
			}
			LeaveCriticalSection(&s_IDLQueLock);

			//if (bQueEmpty)
			//{
			//	pContext = HeapAlloc(s_hHeap, HEAP_ZERO_MEMORY, nSize);
			//}

			if (NULL == pContext)
			{
				throw ((long)(__LINE__));
			}
		}
		catch (const long& iErrCode)
		{
			InterlockedExchangeAdd(&s_nCount, -1);
			pContext = NULL;
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}

		return pContext;
	}

	void CONNECT_CONTEXT:: operator delete(void* p)
	{
		if (p)
		{
			//�����ж��г���С��10000, ���������ж�����; �����ͷ�

			EnterCriticalSection(&s_IDLQueLock);
			const DWORD QUE_SIZE = (DWORD)(s_IDLQue.size());
			CONNECT_CONTEXT* const pContext = (CONNECT_CONTEXT*)p;

			if (QUE_SIZE <= MAX_IDL_DATA)
			{
				s_IDLQue.push_back(pContext);
			}
			else
			{
				HeapFree(s_hHeap, HEAP_NO_SERIALIZE, p);
			}
			LeaveCriticalSection(&s_IDLQueLock);

			//if (QUE_SIZE > MAX_IDL_DATA)
			//{
			//	HeapFree(s_hHeap, 0, p);
			//}
			//InterlockedExchangeAdd(&s_nCount, -1);

			//�Ѿ�û�ж����ٱ��ͷ���Ҫ�����ͷ�
			//if (0 >= s_nCount)
			//{
			//	HeapDestroy(s_hHeap);
			//	s_hHeap = NULL;

			//	EnterCriticalSection(&s_IDLQueLock);
			//	s_IDLQue.clear();
			//	LeaveCriticalSection(&s_IDLQueLock);

			//	DeleteCriticalSection(&s_IDLQueLock);
			//}
		}


		return;
	}

	BOOL CONNECT_CONTEXT::IsAddressValid(LPCVOID lpMem)
	{
		BOOL bResult = HeapValidate(s_hHeap, 0, lpMem);

		if (NULL == lpMem)
		{
			bResult = FALSE;
		}	

		return bResult;
	}

	//class ACCEPT_CONTEXT�����ʵ��
	void* ACCEPT_CONTEXT:: operator new(size_t nSize)
	{
		void* pContext = NULL;

		try
		{
			//��û�ж������������Ҫ�ȴ�����
			//if (0 == InterlockedExchangeAdd(&s_nCount, 1))
			//{			
			//	s_hHeap = HeapCreate(0, BUF_SIZE, HEAP_SIZE);
			//	InitializeCriticalSection(&s_IDLQueLock);
			//}
			if (NULL == s_hHeap)
			{
				throw ((long)(__LINE__));
			}

			//������ڴ�, �ȴӿ��еĶ������������ж���Ϊ����Ӷ��ڴ�������
			//bool bQueEmpty = true;

			EnterCriticalSection(&s_IDLQueLock);
			vector<ACCEPT_CONTEXT* >::iterator iter = s_IDLQue.begin();

			if (iter != s_IDLQue.end())
			{
				pContext = *iter;
				s_IDLQue.erase(iter);	
				//bQueEmpty = false;
			}
			else
			{
				pContext = HeapAlloc(s_hHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, nSize);
			}
			LeaveCriticalSection(&s_IDLQueLock);

			//if (bQueEmpty)
			//{
			//	pContext = HeapAlloc(s_hHeap, HEAP_ZERO_MEMORY, nSize);
			//}

			if (NULL == pContext)
			{
				throw ((long)(__LINE__));
			}
		}
		catch (const long& iErrCode)
		{
			InterlockedExchangeAdd(&s_nCount, -1);
			pContext = NULL;
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}

		return pContext;
	}

	void ACCEPT_CONTEXT:: operator delete(void* p)
	{
		if (p)
		{
			//�����ж��г���С��10000, ���������ж�����; �����ͷ�

			EnterCriticalSection(&s_IDLQueLock);
			const DWORD QUE_SIZE = (DWORD)(s_IDLQue.size());
			const DWORD MAX_IDL = 500;
			ACCEPT_CONTEXT* const pContext = (ACCEPT_CONTEXT*)p;

			if (QUE_SIZE <= MAX_IDL)
			{
				s_IDLQue.push_back(pContext);
			}
			else
			{
				HeapFree(s_hHeap, HEAP_NO_SERIALIZE, p);
			}
			LeaveCriticalSection(&s_IDLQueLock);

			//if (QUE_SIZE > MAX_IDL)
			//{
			//	HeapFree(s_hHeap, 0, p);
			//}
			//InterlockedExchangeAdd(&s_nCount, -1);

			////�Ѿ�û�ж����ٱ��ͷ���Ҫ�����ͷ�
			//if (0 >= s_nCount)
			//{
			//	HeapDestroy(s_hHeap);
			//	s_hHeap = NULL;

			//	EnterCriticalSection(&s_IDLQueLock);
			//	s_IDLQue.clear();
			//	LeaveCriticalSection(&s_IDLQueLock);

			//	DeleteCriticalSection(&s_IDLQueLock);
			//}
		}


		return;
	}

	BOOL ACCEPT_CONTEXT::IsAddressValid(LPCVOID lpMem)
	{
		BOOL bResult = HeapValidate(s_hHeap, 0, lpMem);

		if (NULL == lpMem)
		{
			bResult = FALSE;
		}	

		return bResult;
	}

	//TCP_RCV_DATA ���ʵ��
	TCP_RCV_DATA::TCP_RCV_DATA(const SOCKET& hSock, const CHAR* pBuf, INT nLen)
		: m_hSock(hSock)
		, m_nLen(nLen)
	{
		try
		{
			assert(pBuf);
			if (NULL == s_DataHeap)
			{
				throw ((long)(__LINE__));
			}

			//_TRACE("\r\n%s:%ld s_DataHeap = 0x%x; m_pData = 0x%x", __FILE__, __LINE__, s_DataHeap, m_pData);
			m_pData = (CHAR*)HeapAlloc(s_DataHeap, HEAP_ZERO_MEMORY, nLen);
			memcpy(m_pData, pBuf, nLen);

			//_TRACE("\r\n%s:%ld s_DataHeap = 0x%x; m_pData = 0x%x", __FILE__, __LINE__, s_DataHeap, m_pData);
		}
		catch (const long& iErrCode)
		{
			m_pData = NULL;
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}
	}

	TCP_RCV_DATA::~TCP_RCV_DATA()
	{
		if ((NULL != m_pData ) && (NULL != s_DataHeap))
		{
			HeapFree(s_DataHeap, 0, m_pData);
			m_pData = NULL;
		}
	}

	void* TCP_RCV_DATA::operator new(size_t nSize)
	{
		void* pRcvData = NULL;

		try
		{
			//��û�ж������������Ҫ�ȴ�����
			//if (0 == InterlockedExchangeAdd(&s_nCount, 1))
			//{			
			//	s_hHeap = HeapCreate(0, 0, HEAP_SIZE);
			//	s_DataHeap = HeapCreate(0, 0, DATA_HEAP_SIZE);
			//	InitializeCriticalSection(&s_IDLQueLock);
			//}
			if (NULL == s_hHeap || NULL == s_DataHeap)
			{
				throw ((long)(__LINE__));
			}

			//bool bQueEmpty = true;
			EnterCriticalSection(&s_IDLQueLock);
			vector<TCP_RCV_DATA* >::iterator iter = s_IDLQue.begin();
			if (s_IDLQue.end() != iter)
			{
				pRcvData = *iter;
				s_IDLQue.erase(iter);
				//bQueEmpty = false;
			}
			else
			{
				pRcvData = HeapAlloc(s_hHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, nSize);
			}
			LeaveCriticalSection(&s_IDLQueLock);

			//if (bQueEmpty)
			//{
			//	//������ڴ�
			//	pRcvData = HeapAlloc(s_hHeap, HEAP_ZERO_MEMORY, nSize);
			//}
			//_TRACE("\r\n%s:%ld pRcvData= 0x%x; s_hHeap= 0x%x", __FILE__, __LINE__, pRcvData, s_hHeap);

			if (NULL == pRcvData)
			{
				throw ((long)(__LINE__));
			}
		}
		catch (const long& iErrCode)
		{
			InterlockedExchangeAdd(&s_nCount, -1);
			pRcvData = NULL;
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}

		return pRcvData;
	}

	void TCP_RCV_DATA::operator delete(void* p)
	{
		if (p)
		{
			EnterCriticalSection(&s_IDLQueLock);
			const DWORD QUE_SIZE = (DWORD)(s_IDLQue.size());
			TCP_RCV_DATA* const pContext = (TCP_RCV_DATA*)p;

			if (QUE_SIZE <= MAX_IDL_DATA)
			{
				s_IDLQue.push_back(pContext);
			}
			else
			{
				HeapFree(s_hHeap, HEAP_NO_SERIALIZE, p);
			}
			LeaveCriticalSection(&s_IDLQueLock);

			//if (QUE_SIZE > MAX_IDL_DATA)
			//{
			//	HeapFree(s_hHeap, 0, p);
			//}
			//InterlockedExchangeAdd(&s_nCount, -1);

			////�Ѿ�û�ж����ٱ��ͷ���Ҫ�����ͷ�
			//if (0 >= s_nCount)
			//{
			//	HeapDestroy(s_hHeap);
			//	s_hHeap = NULL;

			//	HeapDestroy(s_DataHeap);
			//	s_DataHeap = NULL;

			//	EnterCriticalSection(&s_IDLQueLock);
			//	s_IDLQue.clear();
			//	LeaveCriticalSection(&s_IDLQueLock);
   //             
			//	DeleteCriticalSection(&s_IDLQueLock);
			//}
		}

		return;
	}

	BOOL TCP_RCV_DATA::IsAddressValid(LPCVOID pMem)
	{
		BOOL bResult = HeapValidate(s_hHeap, 0, pMem);

		if (NULL == pMem)
		{
			bResult = FALSE;
		}

		return bResult;
	}

	BOOL TCP_RCV_DATA::IsDataValid()
	{
		BOOL bResult = HeapValidate(s_DataHeap, 0, m_pData);

		if (NULL == m_pData)
		{
			bResult = FALSE;
		}

		return bResult;
	}

	//TCP_SEND_DATA ���ʵ��
	TCP_SEND_DATA::TCP_SEND_DATA(const CHAR* szData, INT nLen)
		: m_nLen(nLen)
	{
		assert(szData);
		try
		{
			if (NULL == s_DataHeap)
			{
				throw ((long)(__LINE__));
			}

			m_pData = (CHAR*)HeapAlloc(s_DataHeap, HEAP_ZERO_MEMORY, nLen);
			memcpy(m_pData, szData, nLen);
		}
		catch (const long& iErrCode)
		{
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}
	}

	TCP_SEND_DATA::~TCP_SEND_DATA()
	{
		if ((NULL != m_pData ) && (NULL != s_DataHeap))
		{
			HeapFree(s_DataHeap, 0, m_pData);
			m_pData = NULL;
		}
	}

	void* TCP_SEND_DATA::operator new(size_t nSize)
	{
		void* pSendData = NULL;

		try
		{
			//��û�ж������������Ҫ�ȴ�����
			//if (0 == InterlockedExchangeAdd(&s_nCount, 1))
			//{			
			//	s_hHeap = HeapCreate(0, 0, SEND_DATA_HEAP_SIZE);
			//	s_DataHeap = HeapCreate(0, 0, DATA_HEAP_SIZE);
			//	InitializeCriticalSection(&s_IDLQueLock);
			//}
			if (NULL == s_hHeap || NULL == s_DataHeap)
			{
				throw ((long)(__LINE__));
			}

			//bool bQueEmpty = true;
			EnterCriticalSection(&s_IDLQueLock);
			vector<TCP_SEND_DATA* >::iterator iter = s_IDLQue.begin();
			if (s_IDLQue.end() != iter)
			{
				pSendData = *iter;
				s_IDLQue.erase(iter);
				//bQueEmpty = false;
			}
			else
			{
				pSendData = HeapAlloc(s_hHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, nSize);
			}
			LeaveCriticalSection(&s_IDLQueLock);

			//if (bQueEmpty)
			//{
			//	//������ڴ�
			//	pSendData = HeapAlloc(s_hHeap, HEAP_ZERO_MEMORY, nSize);
			//}

			if (NULL == pSendData)
			{
				throw ((long)(__LINE__));
			}
		}
		catch (const long& iErrCode)
		{
			InterlockedExchangeAdd(&s_nCount, -1);
			pSendData = NULL;
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}

		return pSendData;
	}

	void TCP_SEND_DATA::operator delete(void* p)
	{
		if (p)
		{
			EnterCriticalSection(&s_IDLQueLock);
			const DWORD QUE_SIZE = (DWORD)(s_IDLQue.size());
			TCP_SEND_DATA* pContext = (TCP_SEND_DATA*)p;

			if (QUE_SIZE <= MAX_IDL_DATA)
			{
				s_IDLQue.push_back(pContext);
			}
			else
			{
				HeapFree(s_hHeap, HEAP_NO_SERIALIZE, p);
			}
			LeaveCriticalSection(&s_IDLQueLock);

			//if (QUE_SIZE > MAX_IDL_DATA)
			//{
			//	HeapFree(s_hHeap, 0, p);
			//}
			//InterlockedExchangeAdd(&s_nCount, -1);

			////�Ѿ�û�ж����ٱ��ͷ���Ҫ�����ͷ�
			//if (0 >= s_nCount)
			//{
			//	HeapDestroy(s_hHeap);
			//	s_hHeap = NULL;

			//	HeapDestroy(s_DataHeap);
			//	s_DataHeap = NULL;

			//	EnterCriticalSection(&s_IDLQueLock);
			//	s_IDLQue.clear();
			//	LeaveCriticalSection(&s_IDLQueLock);
			//	DeleteCriticalSection(&s_IDLQueLock);
			//}
		}

		return;
	}

	BOOL TCP_SEND_DATA::IsAddressValid(LPCVOID pMem)
	{
		BOOL bResult = HeapValidate(s_hHeap, 0, pMem);

		if (NULL == pMem)
		{
			bResult = FALSE;
		}

		return bResult;
	}

	BOOL TCP_SEND_DATA::IsDataValid()
	{
		BOOL bResult = HeapValidate(s_DataHeap, 0, m_pData);

		if (NULL == m_pData)
		{
			bResult = FALSE;
		}

		return bResult;
	}

	//TCP_CONTEXT��ʵ��
	TCP_CONTEXT::TCP_CONTEXT(CTcpMng* pMng)
	{
		m_pRcvContext = new TCPMNG_CONTEXT(pMng);
		m_pSendContext = new TCPMNG_CONTEXT(pMng);
		m_nInteractiveTime = GetTickCount();

		assert(m_pRcvContext);
		assert(m_pSendContext);
	}

	TCP_CONTEXT::~TCP_CONTEXT()
	{
		if (NULL != m_pRcvContext)
		{
			delete m_pRcvContext;
			m_pRcvContext = NULL;
		}

		if (NULL != m_pSendContext)
		{
			delete m_pSendContext;
			m_pSendContext = NULL;
		}

		TCP_SEND_DATA* pSendData = NULL;
		while (FALSE == m_SendDataQue.empty())
		{
			pSendData = m_SendDataQue.front();
			m_SendDataQue.pop();
			if (NULL != pSendData)
			{
				delete pSendData;
				pSendData = NULL;
			}
		}
	}

	void* TCP_CONTEXT::operator new(size_t nSize)
	{
		void* pContext = NULL;

		try
		{
			//��û�ж������������Ҫ�ȴ�����
			//if (0 == InterlockedExchangeAdd(&s_nCount, 1))
			//{			
			//	s_hHeap = HeapCreate(0, 0, TCP_CONTEXT_HEAP_SIZE);
			//	InitializeCriticalSection(&s_IDLQueLock);
			//}
			if (NULL == s_hHeap)
			{
				throw ((long)(__LINE__));
			}

			//bool bQueEmpty = true;
			EnterCriticalSection(&s_IDLQueLock);
			vector<TCP_CONTEXT*>::iterator iter = s_IDLQue.begin();
			if (s_IDLQue.end() != iter)
			{
				pContext = *iter;
				s_IDLQue.erase(iter);
				//bQueEmpty = false;
			}
			else
			{
				pContext = HeapAlloc(s_hHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, nSize);
			}
			LeaveCriticalSection(&s_IDLQueLock);

			//if (bQueEmpty)
			//{
			//	//������ڴ�
			//	pContext = HeapAlloc(s_hHeap, HEAP_ZERO_MEMORY, nSize);
			//}

			if (NULL == pContext)
			{
				throw ((long)(__LINE__));
			}
		}
		catch (const long& iErrCode)
		{
			InterlockedExchangeAdd(&s_nCount, -1);
			pContext = NULL;
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}

		return pContext;
	}

	void TCP_CONTEXT::operator delete(void* p)
	{
		if (p)
		{
			EnterCriticalSection(&s_IDLQueLock);
			const DWORD QUE_SIZE = (DWORD)(s_IDLQue.size());
			TCP_CONTEXT* const pContext = (TCP_CONTEXT*)p;

			if (QUE_SIZE <= MAX_IDL_DATA)
			{
				s_IDLQue.push_back(pContext);
			}
			else
			{	
				HeapFree(s_hHeap, 0, p);
			}	
			LeaveCriticalSection(&s_IDLQueLock);

			//if (QUE_SIZE > MAX_IDL_DATA)
			//{
			//	HeapFree(s_hHeap, 0, p);
			//}
			//InterlockedExchangeAdd(&s_nCount, -1);

			////�Ѿ�û�ж����ٱ��ͷ���Ҫ�����ͷ�
			//if (0 >= s_nCount)
			//{
			//	HeapDestroy(s_hHeap);
			//	s_hHeap = NULL;

			//	EnterCriticalSection(&s_IDLQueLock);
			//	s_IDLQue.clear();
			//	LeaveCriticalSection(&s_IDLQueLock);
			//	DeleteCriticalSection(&s_IDLQueLock);
			//}
		}

		return;
	}


	BOOL TCP_CONTEXT::IsAddressValid(LPCVOID pMem)
	{
		BOOL bResult = HeapValidate(s_hHeap, 0, pMem);

		if (NULL == pMem)
		{
			bResult = FALSE;
		}

		return bResult;
	}

	//CTcpMng�����ʵ��
	CTcpMng::CTcpMng(void)
		: m_pCloseFun(NULL)
		, m_pCloseParam(NULL)
		, m_hSock(INVALID_SOCKET)
		, m_bThreadRun(TRUE)
	{
		if(0 == InterlockedExchangeAdd(&s_nCount, 1))
		{
			WSADATA wsData;
			WSAStartup(MAKEWORD(2, 2), &wsData);	
		}	

		InitializeCriticalSection(&m_RcvQueLock);
		InitializeCriticalSection(&m_InvalidLock);
		InitializeCriticalSection(&m_ContextLock);

		//Ϊ��ض���Ԥ���ռ�
		m_RcvDataQue.reserve(50000 * sizeof(void*));
		m_AcceptQue.reserve(200 * sizeof(void*));
		m_InvalidQue.reserve(20000 * sizeof(SOCKET));
		//m_hSock = INVALID_SOCKET;
		//m_bThreadRun = TRUE;
		//m_InvalidIter = m_InvalidQue.begin();
		//m_MapIter = m_ContextMap.begin();

		//������̨�߳�, �̷߳���StartServer������
		//_beginthreadex(NULL, 0, ThreadProc, this, 0, NULL);
	}

	CTcpMng::~CTcpMng(void)
	{
		CloseServer();

		DeleteCriticalSection(&m_RcvQueLock);
		DeleteCriticalSection(&m_InvalidLock);
		DeleteCriticalSection(&m_ContextLock);

		if (0 == InterlockedExchangeAdd(&s_nCount, -1))
		{
			WSACleanup();
		}
	}

	void CTcpMng::PushInInvalidQue(const SOCKET& s)
	{
		try
		{
			EnterCriticalSection(&m_InvalidLock);
			const DWORD QUE_SIZE = (DWORD)(m_InvalidQue.size());
			DWORD nNum = 0;

			for (nNum = 0; nNum < QUE_SIZE; nNum++)
			{
				if (s == m_InvalidQue[nNum])
				{
					break;
				}
			}
			//���ڶ�����ʱ�Ų���
			if (nNum >= QUE_SIZE)
			{
				m_InvalidQue.push_back(s);
			}
			//�������֮ǰΪ�գ���Ҫ�������ñ�����
			if (0 == QUE_SIZE)
			{
				m_InvalidIter = m_InvalidQue.begin();
			}
			LeaveCriticalSection(&m_InvalidLock);
		}
		catch(...)
		{
			LeaveCriticalSection(&m_InvalidLock);
		}
	}

	//void CTcpMng::CloseServer()
	//{
	//	if (INVALID_SOCKET == m_hSock)
	//	{
	//		return;
	//	}
	//
	//	try
	//	{
	//		//�Ƚ�����ص�ACCEPT_CONTEXT��m_nPostCount�ı�־��Ϊ-1, ACCEPT_CONTEXT���ͷŲ����ɺ�̨�̴߳���
	//		for(INT index = 0; index < (INT)(m_AcceptQue.size()); index++)
	//		{
	//			ACCEPT_CONTEXT* pAccContext = m_AcceptQue[index];
	//			if (NULL != pAccContext)
	//			{
	//				InterlockedExchange(&(pAccContext->m_nPostCount), -1);
	//			}
	//		}
	//
	//		closesocket(m_ListenSock);
	//		//�ȴ�һ��ʱ����ʹ�����߳����㹻��ʱ�����δ����IO����
	//		Sleep(200);
	//		//����socketѹ����Ч������
	//		PushInInvalidQue(m_ListenSock);
	//	}
	//	catch(...)
	//	{
	//
	//	}
	//	m_ListenSock = INVALID_SOCKET;
	//}

	BOOL CTcpMng::StartServer(
		INT nPort
		, LPCLOSE_ROUTINE pCloseFun		//ĳ��socket�رյĻص�����
		, LPVOID pParam					//�ص�������pParam����
		)
	{
		BOOL bResult = TRUE;
		INT nRet = 0;

		m_pCloseFun = pCloseFun;
		m_pCloseParam = pParam;
        
		//�ȹر��ϴ������ķ���
		CloseServer();

		m_MapIter = m_ContextMap.begin();

		try
		{
			//���������׽���
			m_hSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (INVALID_SOCKET == m_hSock)
			{
				throw ((long)(__LINE__));
			}
			//����AcceptEx����
			DWORD dwBytes;
			GUID guidProc = WSAID_ACCEPTEX;
			if (NULL == s_pfAcceptEx)
			{
				nRet = WSAIoctl(m_hSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidProc, sizeof(guidProc)
					, &s_pfAcceptEx, sizeof(s_pfAcceptEx), &dwBytes, NULL, NULL);
			}
			if (NULL == s_pfAcceptEx || SOCKET_ERROR == nRet)
			{
				throw ((long)(__LINE__));
			}

			//����GetAcceptExSockaddrs����
			//GUID guidGetAddr = WSAID_GETACCEPTEXSOCKADDRS;
			//if (NULL == s_pfGetAddrs)
			//{
			//	nRet = WSAIoctl(m_hSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidGetAddr, sizeof(guidGetAddr)
			//		, &s_pfGetAddrs, sizeof(s_pfGetAddrs), &dwBytes, NULL, NULL);
			//}
			//if (NULL == s_pfGetAddrs)
			//{
			//	throw ((long)(__LINE__));
			//}

			//�����������񣬲�Ͷ��ACCEPT_NUM��accept����
			ULONG ul = 1;
			ioctlsocket(m_hSock, FIONBIO, &ul);

			//����Ϊ��ַ���ã��ŵ����ڷ������رպ������������
			int nOpt = 1;
			setsockopt(m_hSock, SOL_SOCKET, SO_REUSEADDR, (char*)&nOpt, sizeof(nOpt));

			sockaddr_in LocalAddr;
			LocalAddr.sin_family = AF_INET;
			LocalAddr.sin_port = htons(nPort);
			LocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			nRet = bind(m_hSock, (sockaddr*)&LocalAddr, sizeof(LocalAddr));
			if (SOCKET_ERROR == nRet)
			{
				throw ((long)(__LINE__));
			}

			nRet = listen(m_hSock, 200);
			if (SOCKET_ERROR == nRet)
			{
				throw ((long)(__LINE__));
			}
			//��m_ListenSock���ɶ˿�
			if (FALSE == BindIoCompletionCallback((HANDLE)m_hSock, IOCompletionRoutine, 0))
			{
				throw ((long)(__LINE__));
			}

			//Ͷ��ACCEPT_NUM��Accept����
#define ACCEPT_NUM 10
			for (INT index = 0; index < ACCEPT_NUM; )
			{
				//�����ͻ���socket
				SOCKET clientSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
				if (INVALID_SOCKET == clientSock)
				{
					continue;
				}

				ULONG ul = 1;
				ioctlsocket(clientSock, FIONBIO, &ul);

				//����һ��ACCEPT_CONTEXT
				ACCEPT_CONTEXT* pAccContext = new ACCEPT_CONTEXT(this);
				if (NULL == pAccContext)
				{
					closesocket(clientSock);
					throw ((long)(__LINE__));
				}
				pAccContext->m_hSock = m_hSock;
				pAccContext->m_hRemoteSock = clientSock;
				pAccContext->m_nOperation = OP_ACCEPT;

				nRet = s_pfAcceptEx(m_hSock, clientSock, pAccContext->m_pBuf, 0
					, sizeof(sockaddr_in) +16, sizeof(sockaddr_in) +16, &dwBytes, &(pAccContext->m_ol));
				//accept��������, �رշ�����(�ɺ�̨�̴߳���)
				if (FALSE == nRet && ERROR_IO_PENDING != WSAGetLastError())
				{
					closesocket(clientSock);
					delete pAccContext;
					pAccContext = NULL;
					throw ((long)(__LINE__));
				}
				//��ACCEPT_CONTEXTѹ�뵽ACCEPT_CONTEXT������
				m_AcceptQue.push_back(pAccContext);
				index++;
			}

			//������̨�߳�
			m_bThreadRun = TRUE;
			_beginthreadex(NULL, 0, ThreadProc, this, 0, NULL);
		}
		catch (const long& iErrCode)
		{
			closesocket(m_hSock);
			m_hSock = INVALID_SOCKET;
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);

			bResult = FALSE;
		}
		return bResult;
	}

	void CTcpMng::CloseServer()
	{
		closesocket(m_hSock);
		m_hSock = INVALID_SOCKET;

		m_bThreadRun = FALSE;
		//�ȴ�5����, ʹIO�߳����㹻��ʱ��ִ��δ���Ĳ���
		Sleep(5000);

		//����������ݵĻ���������
		TCP_RCV_DATA* pRcvData = NULL;

		EnterCriticalSection(&m_RcvQueLock);
		const DWORD RCV_QUE_NUM = (DWORD)(m_RcvDataQue.size());
		for (DWORD i = 0; i < RCV_QUE_NUM; i++)
		{
			pRcvData = m_RcvDataQue[i];

			delete pRcvData;
			pRcvData = NULL;
		}

		m_RcvDataQue.clear();
		LeaveCriticalSection(&m_RcvQueLock);

		for (vector<ACCEPT_CONTEXT*>::iterator iterAcc = m_AcceptQue.begin(); m_AcceptQue.end() != iterAcc; iterAcc++)
		{
			ACCEPT_CONTEXT* pContext = *iterAcc;
			delete pContext;
			pContext = NULL;
		}
		m_AcceptQue.clear();

		EnterCriticalSection(&m_ContextLock);

		for (map<SOCKET, TCP_CONTEXT* >::iterator iterMap = m_ContextMap.begin(); m_ContextMap.end() != iterMap; iterMap++)
		{
			TCP_CONTEXT* pContext = iterMap->second;
			delete pContext;
			pContext = NULL;
		}
		m_ContextMap.clear();

		LeaveCriticalSection(&m_ContextLock);

		m_InvalidQue.clear();

	}

	void CALLBACK CTcpMng::IOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
	{

		TCPMNG_CONTEXT* pContext = CONTAINING_RECORD(lpOverlapped, TCPMNG_CONTEXT, m_ol);
		switch (pContext->m_nOperation)
		{
		case OP_ACCEPT:		//�пͻ������ӵ�������
			AcceptCompletionProc(dwErrorCode, dwNumberOfBytesTransfered, lpOverlapped);
			break;

		case OP_READ:		//�����ݲ��������
			RecvCompletionProc(dwErrorCode, dwNumberOfBytesTransfered, lpOverlapped);
			break;

		case OP_WRITE:		//д���ݲ��������
			SendCompletionProc(dwErrorCode, dwNumberOfBytesTransfered, lpOverlapped);
			break;

		case OP_CONNECT:	//�ѳɹ����ӵ�������
			ConnectCompletionProc(dwErrorCode, dwNumberOfBytesTransfered, lpOverlapped);
			break;

		default:
			break;
		}
	}

	//��AcceptEx�����������Ҫ���еĴ���
	void CTcpMng::AcceptCompletionProc(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
	{
		try
		{
			INT iErrCode = 0;
			DWORD nBytes = 0;
			ACCEPT_CONTEXT* pContext = CONTAINING_RECORD(lpOverlapped, ACCEPT_CONTEXT, m_ol);
			//��Ͷ�ݴ���Ϊ-1����˵��listen socket�ѹر�, ������Ͷ��accept����
			if (-1 == pContext->m_nPostCount)
			{
				closesocket(pContext->m_hRemoteSock);
				throw ((long)(__LINE__));
			}
			//�д�����, �رտͻ���socket
			if (0 != dwErrorCode)
			{
				closesocket(pContext->m_hRemoteSock);
			}
			else
			{
				//Ϊ��������������TCP_CONTEXT��Ͷ��WSARecv����, ��������뵽m_ContextMap��
				//�ر�ϵͳ���棬ʹ���Լ��Ļ����Է�ֹ���ݵĸ��Ʋ���
				INT nZero = 0;
				//IP_ADDR* pClientAddr;
				//IP_ADDR* pLocalAddr;
				//INT nClientLen;
				//INT nLocalLen;

				setsockopt(pContext->m_hRemoteSock, SOL_SOCKET, SO_SNDBUF, (char*)&nZero, sizeof(nZero));
				setsockopt(pContext->m_hRemoteSock, SOL_SOCKET, SO_RCVBUF, (CHAR*)&nZero, sizeof(nZero));
				setsockopt(pContext->m_hRemoteSock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&(pContext->m_hSock), sizeof(pContext->m_hSock));
				
				//s_pfGetAddrs(pContext->m_pBuf, 0, 0, sizeof(sockaddr_in) +16
				//	, NULL, NULL, (LPSOCKADDR*)&pClientAddr, &nClientLen);

				TCP_CONTEXT* pTcpContext = new TCP_CONTEXT(pContext->m_pMng);
				//ϵͳ�����Ѿ�û���㹻���ڴ����ʹ��
				if (NULL == pTcpContext)
				{
					closesocket(pContext->m_hRemoteSock);
				}
				//���ͺͽ��ջ����಻�ܿ�
				else if (NULL == pTcpContext->m_pRcvContext || NULL == pTcpContext->m_pSendContext)
				{
					closesocket(pContext->m_hRemoteSock);
					delete pTcpContext;
					pTcpContext = NULL;
				}
				else
				{
					//Ͷ�ݶ�����
					InterlockedExchange(&(pTcpContext->m_pRcvContext->m_nPostCount), 1);
					pTcpContext->m_pRcvContext->m_hSock = pContext->m_hRemoteSock;
					pTcpContext->m_pRcvContext->m_nOperation = OP_READ;

					//pTcpContext->m_pSendContext->m_hSock = pContext->m_hRemoteSock;

					//�󶨵���ɶ˿�ʧ��
					if (FALSE == BindIoCompletionCallback((HANDLE)(pTcpContext->m_pRcvContext->m_hSock), IOCompletionRoutine, 0))
					{
						closesocket(pContext->m_hRemoteSock);
						delete pTcpContext;
						pTcpContext = NULL;
					}
					//�󶨳ɹ�
					else
					{					
						DWORD nFlag = 0;					
						WSABUF RcvBuf;
						RcvBuf.buf = pTcpContext->m_pRcvContext->m_pBuf;
						RcvBuf.len = TCPMNG_CONTEXT::BUF_SIZE;
						iErrCode = WSARecv(pTcpContext->m_pRcvContext->m_hSock, &RcvBuf, 1
							, &nBytes, &nFlag, &(pTcpContext->m_pRcvContext->m_ol), NULL);

						//Ͷ�ݳɹ�
						if (0 == iErrCode || WSA_IO_PENDING == WSAGetLastError())
						{
							pContext->m_pMng->PushInContextMap(pTcpContext->m_pRcvContext->m_hSock, pTcpContext);
						}
						//Ͷ��ʧ��
						else
						{
							closesocket(pContext->m_hRemoteSock);
							InterlockedExchange(&(pTcpContext->m_pRcvContext->m_nPostCount), 0);
							delete pTcpContext;
							pTcpContext = NULL;
						}
					}
				}
			}
			//Ͷ���µ�Accept����
			SOCKET clientSock = INVALID_SOCKET; 
			for (INT index = 0; index < 10; index++)
			{
				clientSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
				//ֻҪ��һ�γɹ����˳�
				if (INVALID_SOCKET != clientSock)
				{
					break;
				}
			}

			ULONG ul = 1;
			ioctlsocket(clientSock, FIONBIO, &ul);

			//ϵͳ��ǰû�п��õ�socket����accept����, ����PostCount��Ϊ0xfafafafa
			if (INVALID_SOCKET == clientSock)
			{
				InterlockedExchange(&(pContext->m_nPostCount), 0xfafafafa);
				throw ((long)(__LINE__));
			}
			pContext->m_hRemoteSock = clientSock;
			iErrCode = s_pfAcceptEx(pContext->m_hSock, clientSock, pContext->m_pBuf, 0
				, sizeof(sockaddr_in) +16, sizeof(sockaddr_in) +16, &nBytes, &(pContext->m_ol));
			//accept��������
			if (FALSE == iErrCode && ERROR_IO_PENDING != WSAGetLastError())
			{
				closesocket(clientSock);
				closesocket(pContext->m_hSock);
				pContext->m_pMng->m_hSock = INVALID_SOCKET;
				InterlockedExchange(&(pContext->m_nPostCount), -1);
				throw ((long)(__LINE__));
			}
		}
		catch (const long& iErrCode)
		{
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}
	}

	//��ConnectEx�����������Ҫ���еĴ���
	void CTcpMng::ConnectCompletionProc(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
	{
#if 0
		CONNECT_CONTEXT* pConnContext = CONTAINING_RECORD(lpOverlapped, CONNECT_CONTEXT, m_ol);
		TCP_CONTEXT* pTcpContext = new TCP_CONTEXT(pConnContext->m_pMng);
		LPCONNECT_ROUTINE lpfun = pConnContext->m_pConProc;
		LPVOID pParam = pConnContext->m_pParam;
		assert(lpfun);
		INT nRet = 0;

		try
		{
			//�д�����, �ر�socket, ���Ӷ��е����������ʱ����
			if (0 != dwErrorCode)
			{
				throw ((long)(__LINE__));
			}

			if (NULL == pTcpContext || NULL == pTcpContext->m_pRcvContext || NULL == pTcpContext->m_pSendContext)
			{
				throw ((long)(__LINE__));
			}

			//�ڸ�socket��Ͷ��һ������������������뵽m_ContextMap��
			WSABUF RcvBuf;
			DWORD dwBytes;
			DWORD dwFlag = 0;

			InterlockedExchange(&(pTcpContext->m_pRcvContext->m_nPostCount), 1);
			pTcpContext->m_pRcvContext->m_hSock = pConnContext->m_hSock;
			pTcpContext->m_pSendContext->m_hSock = pConnContext->m_hSock;

			pTcpContext->m_pRcvContext->m_nOperation = OP_READ;

			RcvBuf.buf = pTcpContext->m_pRcvContext->m_pBuf;
			RcvBuf.len = TCPMNG_CONTEXT::BUF_SIZE;
			nRet = WSARecv(pConnContext->m_hSock, &RcvBuf, 1, &dwBytes, &dwFlag, &(pTcpContext->m_pRcvContext->m_ol), NULL);
			//���ղ���ʧ��
			if (SOCKET_ERROR == nRet && ERROR_IO_PENDING != WSAGetLastError())
			{
				InterlockedExchange(&(pTcpContext->m_pRcvContext->m_nPostCount), 0);
				throw ((long)(__LINE__));
			}

			pConnContext->m_pMng->PushInContextMap(pConnContext->m_hSock, pTcpContext);
			//������ص�����
			if (lpfun)
			{
				lpfun(pParam, pConnContext->m_hSock);
			}
		}
		catch (const long& iErrCode)
		{
			closesocket(pConnContext->m_hSock);
			if (pTcpContext)
			{
				delete pTcpContext;
				pTcpContext = NULL;
			}

			if (lpfun)
			{
				lpfun(pParam, INVALID_SOCKET);
			}
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}

		//�ͷ�CONNECT_CONTEXT
		delete pConnContext;
		pConnContext = NULL;
#endif
	}

	//��WSARecv�����������Ҫ���еĴ���
	void CTcpMng::RecvCompletionProc(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
	{
		TCPMNG_CONTEXT* pRcvContext = CONTAINING_RECORD(lpOverlapped, TCPMNG_CONTEXT, m_ol);
		DWORD dwFlag = 0;

		try
		{
			//�д������, �ر�socket����������뵽��Ч��scket������
			if (0 != dwErrorCode || 0 == dwNumberOfBytesTransfered)
			{			
				throw ((long)(__LINE__));
			}
			
#ifndef _XML_NET_		//���������������
			//�Ƿ��ͻ��˷��������ݰ�, �رոÿͻ���; ����һ���յ����ݰ��������ݰ��ĳ�����Ϣ��ͷ������˵�����ݰ�����
			if (0 == pRcvContext->m_nDataLen && dwNumberOfBytesTransfered < sizeof(PACKET_HEAD))
			{
				throw ((long)(__LINE__));
			}
#endif	//#ifndef _XML_NET_

			//�޸Ľ���ʱ��
			pRcvContext->m_pMng->ModifyInteractiveTime(pRcvContext->m_hSock);

			WSABUF RcvBuf;

#ifdef _XML_NET_	//����XML��
			TCP_RCV_DATA* pRcvData = new TCP_RCV_DATA(pRcvContext->m_hSock, pRcvContext->m_pBuf, dwNumberOfBytesTransfered);
			//����û���㹻���ڴ��ʹ��
			if (NULL == pRcvData)
			{					
				throw ((long)(__LINE__));
			}
			pRcvContext->m_pMng->PushInRecvQue(pRcvData);

			pRcvContext->m_nDataLen = 0;
			RcvBuf.buf = pRcvContext->m_pBuf;
			RcvBuf.len = TCPMNG_CONTEXT::BUF_SIZE;

#else				//���������������

			//������ͷ��Ϣ�е�Ӧ���ܵ����ݰ��ĳ���
			pRcvContext->m_nDataLen += dwNumberOfBytesTransfered;
			//WORD nAllLen = *((WORD*)(pRcvContext->m_pBuf));		//�˳���Ϊ���ݵĳ���+sizeof(WORD)
			PACKET_HEAD* pHeadInfo = (PACKET_HEAD*)(pRcvContext->m_pBuf);
			//WSABUF RcvBuf;
			//���ݳ��ȺϷ��Ž��д���
			if ((pHeadInfo->nCurrentLen <= TCPMNG_CONTEXT::BUF_SIZE)  
				&& ((WORD)(pRcvContext->m_nDataLen) <= pHeadInfo->nCurrentLen + sizeof(PACKET_HEAD)))
			{
				//���������ѽ�����,���������ն�����
				if ((WORD)(pRcvContext->m_nDataLen) == pHeadInfo->nCurrentLen + sizeof(PACKET_HEAD))
				{
					TCP_RCV_DATA* pRcvData = new TCP_RCV_DATA(pRcvContext->m_pBuf, pRcvContext->m_nDataLen);
					//����û���㹻���ڴ��ʹ��
					if (NULL == pRcvData)
					{					
						throw ((long)(__LINE__));
					}
					pRcvContext->m_pMng->PushInRecvQue(pRcvData);

					pRcvContext->m_nDataLen = 0;
					RcvBuf.buf = pRcvContext->m_pBuf;
					RcvBuf.len = TCPMNG_CONTEXT::BUF_SIZE;
				}
				//����û�н�����, ��Ҫ����ʣ�������
				else
				{
					RcvBuf.buf = pRcvContext->m_pBuf +pRcvContext->m_nDataLen;
					RcvBuf.len = pHeadInfo->nCurrentLen - pRcvContext->m_nDataLen +sizeof(PACKET_HEAD);
				}
			}
			//���ݷǷ�, �򲻴�������ֱ�ӽ�����һ�ζ�Ͷ��
			else
			{
				pRcvContext->m_nDataLen = 0;
				RcvBuf.buf = pRcvContext->m_pBuf;
				RcvBuf.len = TCPMNG_CONTEXT::BUF_SIZE;
			}
#endif	//#ifdef _XML_NET_

			//����Ͷ�ݶ�����
			//DWORD dwBytes = 0;
			INT iErrCode = WSARecv(pRcvContext->m_hSock, &RcvBuf, 1, NULL, &dwFlag, &(pRcvContext->m_ol), NULL);
			//Recv��������
			if (SOCKET_ERROR == iErrCode && WSA_IO_PENDING != WSAGetLastError())
			{
				throw ((long)(__LINE__));
			}
		}
		catch (const long& iErrCode)
		{
			InterlockedExchange(&(pRcvContext->m_nPostCount), 0);
			closesocket(pRcvContext->m_hSock);
			pRcvContext->m_pMng->PushInInvalidQue(pRcvContext->m_hSock);
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}
	}

	//��WSASend�����������Ҫ���еĴ���
	void CTcpMng::SendCompletionProc(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
	{
		TCPMNG_CONTEXT* pSendContext = CONTAINING_RECORD(lpOverlapped, TCPMNG_CONTEXT, m_ol);

		try
		{

			//���д���������������Ч��socket������
			if (0 != dwErrorCode || 0 == dwNumberOfBytesTransfered)
			{
				throw ((long)(__LINE__));
			}

#ifndef _XML_NET_
			pSendContext->m_nDataLen += dwNumberOfBytesTransfered;
			PACKET_HEAD* pHeadInfo = (PACKET_HEAD*)(pSendContext->m_pBuf);
#endif	//#ifndef _XML_NET_

			BOOL bSend = FALSE;		//�Ƿ���ҪͶ�ݷ��Ͳ���
			WSABUF SendBuf;
			//DWORD dwBytes = 0;

			//�޸Ľ���ʱ��
			pSendContext->m_pMng->ModifyInteractiveTime(pSendContext->m_hSock);

#ifndef _XML_NET_
			//����û�з����껹�����Ͷ�ݷ��Ͳ���
			if ((pHeadInfo->nCurrentLen +sizeof(PACKET_HEAD) <= TCPMNG_CONTEXT::BUF_SIZE) 
				&& (pSendContext->m_nDataLen < pHeadInfo->nCurrentLen +sizeof(PACKET_HEAD)))
			{
				bSend = TRUE;
				SendBuf.buf = pSendContext->m_pBuf +pSendContext->m_nDataLen;
				SendBuf.len = pHeadInfo->nCurrentLen - pSendContext->m_nDataLen +sizeof(PACKET_HEAD);
			}
			//�����ѷ����꣬���Լ���Ͷ���µķ��Ͳ���
			else
			{
#endif	//#ifndef _XML_NET_

				//�Ӷ�����ɾ���ϴ���Ͷ�ݵķ�������
				TCP_SEND_DATA* pSendData = pSendContext->m_pMng->GetSendData(pSendContext->m_hSock);			

				if (NULL != pSendData)
				{
					bSend = TRUE;
					
					pSendContext->m_nDataLen = 0;	//nLen; ��û���͵����紦��, ���Դ�������ݳ���Ϊ0
					memcpy(pSendContext->m_pBuf, pSendData->m_pData, pSendData->m_nLen);

					SendBuf.buf = pSendContext->m_pBuf;
					SendBuf.len = pSendData->m_nLen;

					delete pSendData;
					pSendData = NULL;
				}
				//û����Ҫ���͵�����
				else
				{
					bSend = FALSE;
					InterlockedExchange(&(pSendContext->m_nPostCount), 0);
				}
#ifndef _XML_NET_
			}
#endif	//#ifndef _XML_NET_

			if (TRUE == bSend)
			{
				INT iErrCode = WSASend(pSendContext->m_hSock, &SendBuf, 1, NULL, 0, &(pSendContext->m_ol), NULL);

				//Ͷ��ʧ��
				if (SOCKET_ERROR == iErrCode && ERROR_IO_PENDING != WSAGetLastError())
				{				
					throw ((long)(__LINE__));
				}
			}
		}
		catch (const long& iErrCode)
		{
			InterlockedExchange(&(pSendContext->m_nPostCount), 0);
			closesocket(pSendContext->m_hSock);
			pSendContext->m_pMng->PushInInvalidQue(pSendContext->m_hSock);
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}
	}

	TCP_SEND_DATA* CTcpMng::GetSendData(const SOCKET &sock)
	{
		TCP_SEND_DATA* pSendData = NULL;

		EnterCriticalSection(&m_ContextLock);

		map<SOCKET, TCP_CONTEXT* >::iterator itermap = m_ContextMap.find(sock);

		if (m_ContextMap.end() != itermap)
		{
			TCP_CONTEXT* pContext = itermap->second;

			if (pContext)
			{
				if (FALSE == pContext->m_SendDataQue.empty())
				{
					pSendData = pContext->m_SendDataQue.front();
					pContext->m_SendDataQue.pop();
				}
			}
			else
			{
				delete pContext;
				pContext = NULL;
				m_ContextMap.erase(itermap);
			}
		}

		LeaveCriticalSection(&m_ContextLock);

		return pSendData;
	}

	//��accept socket������-1 �� 0xfafafafa������ʽ
	UINT WINAPI CTcpMng::ThreadProc(LPVOID lpParam)
	{	
		CTcpMng* pThis = (CTcpMng*)lpParam;
		const DWORD MAX_ERGOD = 200;		//ÿ��������200����Ԫ��

		while (pThis->m_bThreadRun)
		{
			//����Ч������ȡ����Ч��socket�Ľ��д���
			EnterCriticalSection(&(pThis->m_InvalidLock));
		
			const DWORD QUE_SIZE = (DWORD)(pThis->m_InvalidQue.size());
			DWORD nCount = 0;
			//_TRACE("\r\n%s:%ld QUE_SIZE = %ld", __FILE__, __LINE__, QUE_SIZE);
			
			//����ѱ��������е�β��, ���ͷ��ʼ���±���
			if (pThis->m_InvalidQue.end() == pThis->m_InvalidIter)
			{
				pThis->m_InvalidIter = pThis->m_InvalidQue.begin();
			}

			//ÿ����ദ��200��
			for (nCount = 0; ((nCount < MAX_ERGOD) && (pThis->m_InvalidQue.end() != pThis->m_InvalidIter)); nCount++)
			//for (vector<SOCKET>::iterator sock_iter = pThis->m_InvalidQue.begin(); 
			//	pThis->m_InvalidQue.end() != sock_iter; )
			{	

				SOCKET sInvalid = *(pThis->m_InvalidIter);

				//����TCP_CONTEXT ��map������Ҫ��鵱ǰ�Ƿ���δ����IO����, ��û��δ����IO����ʱ����Խ����ͷŲ���
				EnterCriticalSection(&(pThis->m_ContextLock));

				//map <SOCKET, TCP_CONTEXT* >::iterator iterTcp = pThis->m_ContextMap.find(sInvalid);
				pThis->m_MapIter = pThis->m_ContextMap.find(sInvalid);

				if (pThis->m_MapIter != pThis->m_ContextMap.end())
				{
					//�����һ��Ҫ���б���Ԫ����Ҫ����
					//if (iterTcp == pThis->m_MapIter)
					//{
					//	pThis->m_MapIter++;
					//}

					TCP_CONTEXT* pTcpContext = pThis->m_MapIter->second;

					if ((NULL != pTcpContext)
						&& (pTcpContext->m_pRcvContext->m_nPostCount <= 0)
						&& (pTcpContext->m_pSendContext->m_nPostCount <= 0))
					{
						pThis->m_ContextMap.erase(pThis->m_MapIter);					
						pThis->m_InvalidQue.erase(pThis->m_InvalidIter);
						delete pTcpContext;
						pTcpContext = NULL;

						//֪ͨ�ϲ��socket�ѹر�
						if (pThis->m_pCloseFun)
						{
							pThis->m_pCloseFun(pThis->m_pCloseParam, sInvalid);
						}
					}
					else if (NULL == pTcpContext)
					{
						pThis->m_ContextMap.erase(pThis->m_MapIter);					
						pThis->m_InvalidQue.erase(pThis->m_InvalidIter);
					}
					//����������һ��
					else
					{
						pThis->m_InvalidIter++;
					}
				}
				//�Ƿ���socket
				else
				{
					pThis->m_InvalidQue.erase(pThis->m_InvalidIter);
				}

				LeaveCriticalSection(&(pThis->m_ContextLock));

				//����Ƿ���accept ������

				//ACCEPT_CONTEXT* pAccContext = NULL;
				//for (vector<ACCEPT_CONTEXT* >::iterator iter = pThis->m_AcceptQue.begin(); iter != pThis->m_AcceptQue.end(); iter++)
				//{
				//	pAccContext = *iter;
				//	if (NULL != pAccContext && pAccContext->m_hSock == sInvalid)
				//	{
				//		pThis->m_AcceptQue.erase(iter);
				//		pThis->m_InvalidQue.erase(sInvalid);
				//		delete pAccContext;
				//		pAccContext = NULL;
				//	}
				//}
			}

			LeaveCriticalSection(&(pThis->m_InvalidLock));

			//���ÿ����Ч��socket�ϴεĽ���ʱ��,�������ʮ����û�н�����Ͽ�������
			EnterCriticalSection(&(pThis->m_ContextLock));

			DWORD dwNow = GetTickCount();
			const DWORD TIME_OUT = 10000;
			//const DWORD CONTEXT_NUM = (DWORD)(pThis->m_ContextMap.size());

			if (pThis->m_ContextMap.end() == pThis->m_MapIter)
			{
				pThis->m_MapIter = pThis->m_ContextMap.begin();
			}

			for (nCount = 0; (pThis->m_ContextMap.end() != pThis->m_MapIter && nCount < MAX_ERGOD); nCount++)
			//for (map<SOCKET, TCP_CONTEXT*>::iterator map_iter = pThis->m_ContextMap.begin();
			//	pThis->m_ContextMap.end() != map_iter; map_iter++)
			{
				//if (pThis->m_ContextMap.end() == pThis->m_MapIter)
				//{
				//	break;
				//}

				SOCKET sock = pThis->m_MapIter->first;
				TCP_CONTEXT* pTcpContext = pThis->m_MapIter->second;

				if (pTcpContext)
				{
					if (dwNow - pTcpContext->m_nInteractiveTime >= TIME_OUT)
					{
						closesocket(sock);
						pThis->PushInInvalidQue(sock);						
					}
					pThis->m_MapIter++;
				}
				else
				{
					pThis->m_ContextMap.erase(pThis->m_MapIter);
				}	
			}

			LeaveCriticalSection(&(pThis->m_ContextLock));

#if 0
			//���accept������postcountΪ0xfafafafa��socketΪ��Ͷ���µ�Accept����
			//const DWORD ACCEPT_COUNT = (DWORD)(pThis->m_AcceptQue.size());

			//for (nCount = 0; nCount < ACCEPT_COUNT; nCount++)
			//{
			//	ACCEPT_CONTEXT* pAccContext = pThis->m_AcceptQue[nCount];
			//	if (0xfafafafa == pAccContext->m_nPostCount)
			//	{
			//		SOCKET sockClient = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			//		//û�п���socket, ���ACCEPT_CONTEXT
			//		//if (INVALID_SOCKET == sockClient)
			//		//{
			//		//	pThis->m_AcceptQue.erase(iter);
			//		//	delete pAccContext;
			//		//	pAccContext = NULL;
			//		//}
			//		//Ͷ��accept����
			//		//else
			//		if (INVALID_SOCKET != sockClient)
			//		{
			//			ULONG ul = 1;
			//			ioctlsocket(sockClient, FIONBIO, &ul);

			//			DWORD dwNum;
			//			InterlockedExchange(&(pAccContext->m_nPostCount), 1);
			//			pAccContext->m_hRemoteSock = sockClient;
			//			int iErrCode = s_pfAcceptEx(pAccContext->m_hSock, sockClient, pAccContext->m_pBuf, 0
			//				, sizeof(sockaddr_in) +16, sizeof(sockaddr_in) +16, &dwNum, &(pAccContext->m_ol));
			//			//accept��������
			//			if (FALSE == iErrCode && ERROR_IO_PENDING != WSAGetLastError())
			//			{
			//				closesocket(sockClient);
			//				closesocket(pAccContext->m_hSock);
			//				pThis->m_hSock = INVALID_SOCKET;
			//				InterlockedExchange(&(pAccContext->m_nPostCount), 0);
			//			}
			//		}
			//	}
			//}
#endif

			//_TRACE("\r\n%s : %ld 1", __FILE__, __LINE__);
			Sleep(100);
		}

		return 0;
	}

	void CTcpMng::DelTcpContext(const SOCKET& s)
	{
		try
		{
			EnterCriticalSection(&m_ContextLock);
			m_ContextMap.erase(s);
			LeaveCriticalSection(&m_ContextLock);
		}
		catch (...)
		{
			LeaveCriticalSection(&m_ContextLock);
		}
	}

	void CTcpMng::PushInContextMap(const SOCKET&s, TCP_CONTEXT* const pTcpContext)
	{
		try
		{
			EnterCriticalSection(&m_ContextLock);
			bool bEmpty = m_ContextMap.empty();
			m_ContextMap[s] = pTcpContext;

			//������п�ʼΪ����Ҫ���ñ�����
			if (bEmpty)
			{
				m_MapIter = m_ContextMap.begin();
			}

			LeaveCriticalSection(&m_ContextLock);
		}
		catch (...)
		{
			LeaveCriticalSection(&m_ContextLock);
		}
	}

	void CTcpMng::PushInRecvQue(TCP_RCV_DATA* const pRcvData)
	{
		try
		{		
			assert(pRcvData);
			EnterCriticalSection(&m_RcvQueLock);
			m_RcvDataQue.push_back(pRcvData);
			LeaveCriticalSection(&m_RcvQueLock);
		}
		catch (...)
		{
			LeaveCriticalSection(&m_RcvQueLock);
		}
	}

	TCP_RCV_DATA* CTcpMng::GetRcvData(DWORD* const pQueLen)
	{
		TCP_RCV_DATA* pRcvData = NULL;

		try
		{
			EnterCriticalSection(&m_RcvQueLock);
			vector<TCP_RCV_DATA*>::iterator iter = m_RcvDataQue.begin();
			if (m_RcvDataQue.end() != iter)
			{
				pRcvData = *iter;
				m_RcvDataQue.erase(iter);
			}

			if (NULL != pQueLen)
			{
				*pQueLen = (DWORD)(m_RcvDataQue.size());
			}
			LeaveCriticalSection(&m_RcvQueLock);
		}
		catch (...)
		{
			LeaveCriticalSection(&m_RcvQueLock);
		}
		return pRcvData;
	}

	BOOL CTcpMng::Connect(const CHAR* szIP, INT nPort, LPCONNECT_ROUTINE lpConProc, LPVOID pParam)
	{
		BOOL bResult = TRUE;
#if 0
		assert(lpConProc);
		closesocket(m_hSock);

		m_hSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		CONNECT_CONTEXT* pConnContext = new CONNECT_CONTEXT(lpConProc, pParam, this);

		try 
		{
			if (INVALID_SOCKET == m_hSock)
			{
				throw ((long)(__LINE__));
			}
			if (NULL == pConnContext)
			{
				throw ((long)(__LINE__));
			}

			//����ConnectEx����
			GUID guidProc =  WSAID_CONNECTEX;
			DWORD dwBytes;
			INT nRet;		

			if (NULL == s_pfConnectEx)
			{
				nRet = WSAIoctl(m_hSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidProc, sizeof(guidProc)
					, &s_pfConnectEx, sizeof(s_pfConnectEx), &dwBytes, NULL, NULL);
			}
			if (NULL == s_pfConnectEx || SOCKET_ERROR == nRet)
			{
				throw ((long)(__LINE__));
			}

			ULONG ul = 1;
			ioctlsocket(m_hSock, FIONBIO, &ul);

			//Ϊ��������������TCP_CONTEXT��Ͷ��WSARecv����, ��������뵽m_ContextMap��
			//�ر�ϵͳ���棬ʹ���Լ��Ļ����Է�ֹ���ݵĸ��Ʋ���
			INT nZero = 0;
			setsockopt(m_hSock, SOL_SOCKET, SO_SNDBUF, (char*)&nZero, sizeof(nZero));
			setsockopt(m_hSock, SOL_SOCKET, SO_RCVBUF, (CHAR*)&nZero, sizeof(nZero));

			//����socket�󶨵���ɶ˿���
			if (FALSE == BindIoCompletionCallback((HANDLE)m_hSock, IOCompletionRoutine, 0))
			{
				throw ((long)(__LINE__));
			}

			//�Ƚ��а�
			IP_ADDR locaAddr("0.0.0.0", 0);
			if (SOCKET_ERROR == bind(m_hSock, (sockaddr*)&locaAddr, sizeof(locaAddr)))
			{
				throw ((long)(__LINE__));
			}
			//ִ�����Ӳ���
			sockaddr_in SerAddr;
			SerAddr.sin_family = AF_INET;
			SerAddr.sin_port = htons(nPort);
			SerAddr.sin_addr.s_addr = inet_addr(szIP);

			pConnContext->m_hSock = m_hSock;
			pConnContext->m_nOperation = OP_CONNECT;

			nRet = s_pfConnectEx(m_hSock, (sockaddr*)&SerAddr, sizeof(SerAddr), NULL, 0, NULL, &(pConnContext->m_ol));
			//���ӷ�������
			if (SOCKET_ERROR == nRet && ERROR_IO_PENDING != WSAGetLastError())
			{
				throw ((long)(__LINE__));
			}		
		}
		catch (const long& iErrCode)
		{
			bResult = FALSE;
			if (INVALID_SOCKET != m_hSock)
			{
				closesocket(m_hSock);
				m_hSock = INVALID_SOCKET;
			}

			delete pConnContext;
			pConnContext = NULL;
			_TRACE("\r\nExcept : %s--%ld", __FILE__, iErrCode);
		}
#endif

		return bResult;
	}

	BOOL CTcpMng::SendData(const SOCKET& sock, const CHAR* szData, INT nDataLen)
	{
#ifdef _XML_NET_
		//���ݳ��ȷǷ�
		if ((nDataLen > TCPMNG_CONTEXT::BUF_SIZE) || (NULL == szData))
		{
			return FALSE;
		}
#else
		//���ݳ��ȷǷ�
		if ((nDataLen > TCPMNG_CONTEXT::BUF_SIZE) || (NULL == szData) || (nDataLen < sizeof(PACKET_HEAD)))
		{
			return FALSE;
		}
#endif	//#ifdef _XML_NET_

		BOOL bResult = TRUE;

		assert(szData);

		try 
		{
			EnterCriticalSection(&m_ContextLock);

			map<SOCKET, TCP_CONTEXT* >::iterator iterMap = m_ContextMap.find(sock);
			BOOL bSend = FALSE;			//�Ƿ���ҪͶ�ݷ��Ͳ���
			WSABUF SendBuf;
			DWORD dwBytes;
			//WORD nAllLen = 0;

			if (m_ContextMap.end() != iterMap)
			{
				TCP_CONTEXT* pContext = iterMap->second;
				if (pContext)
				{
					//���Ͷ���Ϊ��, ����postcountΪ0, ��Ͷ�ݷ��Ͳ���
					if (pContext->m_SendDataQue.empty() && 0 == pContext->m_pSendContext->m_nPostCount)
					{
						bSend = TRUE;
						InterlockedExchange(&(pContext->m_pSendContext->m_nPostCount), 1);
						//nAllLen = (WORD)(nDataLen + sizeof(WORD));
						pContext->m_pSendContext->m_hSock = sock;
						pContext->m_pSendContext->m_nOperation = OP_WRITE;					
						pContext->m_pSendContext->m_nDataLen = 0;	//nLen; ��û���͵����紦��, ���Դ�������ݳ���Ϊ0
						//memcpy(pContext->m_pSendContext->m_pBuf, &nAllLen, sizeof(nAllLen));
						memcpy(pContext->m_pSendContext->m_pBuf, szData, nDataLen);

						SendBuf.buf = pContext->m_pSendContext->m_pBuf;
						SendBuf.len = nDataLen;
					}
					//������ѹ�������, ���Ӷ���ͷ��ȡ��һ�����ݽ���Ͷ��
					else if (0 == pContext->m_pSendContext->m_nPostCount)
					{
						bSend = TRUE;
						TCP_SEND_DATA* pSendData = new TCP_SEND_DATA(szData, nDataLen);

						if (pSendData)
						{
							pContext->m_SendDataQue.push(pSendData);
						}

						pSendData = pContext->m_SendDataQue.front();
						pContext->m_SendDataQue.pop();

						InterlockedExchange(&(pContext->m_pSendContext->m_nPostCount), 1);
						pContext->m_pSendContext->m_nDataLen = 0;
						pContext->m_pSendContext->m_hSock = sock;
						pContext->m_pSendContext->m_nOperation = OP_WRITE;
						//memcpy(pContext->m_pSendContext->m_pBuf, &nAllLen, sizeof(nAllLen));
						memcpy(pContext->m_pSendContext->m_pBuf, pSendData->m_pData, pSendData->m_nLen);

						SendBuf.buf = pContext->m_pSendContext->m_pBuf;
						SendBuf.len = pSendData->m_nLen;

						delete pSendData;		//pSendData = pContext->m_SendDataQue.front();
						pSendData = NULL;
					}
					//������ѹ�뵽�����м���
					else
					{
						bSend = FALSE;

						TCP_SEND_DATA* pSendData = new TCP_SEND_DATA(szData, nDataLen);
						if (pSendData)
						{
							pContext->m_SendDataQue.push(pSendData);
						}
					}

					if (bSend)
					{
						INT iErr = WSASend(pContext->m_pSendContext->m_hSock, &SendBuf, 1, &dwBytes, 0, &(pContext->m_pSendContext->m_ol), NULL);

						if (SOCKET_ERROR == iErr && ERROR_IO_PENDING != WSAGetLastError())
						{
							InterlockedExchange(&(pContext->m_pSendContext->m_nPostCount), 0);
							closesocket(sock);
							PushInInvalidQue(sock);
						}
					}
				}
				else
				{
					closesocket(sock);
					PushInInvalidQue(sock);
				}
			}
			LeaveCriticalSection(&m_ContextLock);
		}
		catch (...)
		{
			LeaveCriticalSection(&m_ContextLock);
		}

		return bResult;
	}

	TCP_CONTEXT* CTcpMng::GetTcpContext(const SOCKET& s)
	{
		TCP_CONTEXT* pContext = NULL;
		map<SOCKET, TCP_CONTEXT*>:: iterator iter;
		try
		{
			EnterCriticalSection(&m_ContextLock);
			iter = m_ContextMap.find(s);

			if (iter != m_ContextMap.end())
			{
				pContext = iter->second;
			}
			LeaveCriticalSection(&m_ContextLock);
		}
		catch (...)
		{
			LeaveCriticalSection(&m_ContextLock);
			pContext = NULL;
		}

		return pContext;
	}

	void CTcpMng::ModifyInteractiveTime(const SOCKET& s)
	{
		TCP_CONTEXT* pContext = NULL;
		map<SOCKET, TCP_CONTEXT* >::iterator iter;

		EnterCriticalSection(&m_ContextLock);
		iter = m_ContextMap.find(s);

		if (m_ContextMap.end() != iter)
		{
			pContext = iter->second;
			pContext->m_nInteractiveTime = GetTickCount();
		}
		LeaveCriticalSection(&m_ContextLock);
	}

	void CTcpMng::InitReource()
	{
		//TCPMNG_CONTEXT
		TCPMNG_CONTEXT::s_hHeap = HeapCreate(0, TCPMNG_CONTEXT::BUF_SIZE, TCPMNG_CONTEXT::HEAP_SIZE);
		InitializeCriticalSection(&(TCPMNG_CONTEXT::s_IDLQueLock));
		TCPMNG_CONTEXT::s_IDLQue.reserve(TCPMNG_CONTEXT::MAX_IDL_DATA * sizeof(TCPMNG_CONTEXT*));
		_TRACE("\r\n%s:%ld TCPMNG_CONTEXT::s_hHeap = 0x%x sizeof(TCPMNG_CONTEXT*) = %ld"
			, __FILE__, __LINE__, TCPMNG_CONTEXT::s_hHeap, sizeof(TCPMNG_CONTEXT*));

		//CONNECT_CONTEXT
		CONNECT_CONTEXT::s_hHeap = HeapCreate(0, CONNECT_CONTEXT::BUF_SIZE, CONNECT_CONTEXT::HEAP_SIZE);
		InitializeCriticalSection(&(CONNECT_CONTEXT::s_IDLQueLock));

		//ACCEPT_CONTEXT
		ACCEPT_CONTEXT::s_hHeap = HeapCreate(0, ACCEPT_CONTEXT::BUF_SIZE, ACCEPT_CONTEXT::HEAP_SIZE);
		InitializeCriticalSection(&(ACCEPT_CONTEXT::s_IDLQueLock));
		ACCEPT_CONTEXT::s_IDLQue.reserve(500 * sizeof(ACCEPT_CONTEXT*));
		_TRACE("\r\n%s:%ld nACCEPT_CONTEXT::s_hHeap = 0x%x", __FILE__, __LINE__, ACCEPT_CONTEXT::s_hHeap);

		//TCP_RCV_DATA
		TCP_RCV_DATA::s_hHeap = HeapCreate(0, 0, TCP_RCV_DATA::HEAP_SIZE);
		TCP_RCV_DATA::s_DataHeap = HeapCreate(0, 0, TCP_RCV_DATA::DATA_HEAP_SIZE);
		InitializeCriticalSection(&(TCP_RCV_DATA::s_IDLQueLock));
		TCP_RCV_DATA::s_IDLQue.reserve(TCP_RCV_DATA::MAX_IDL_DATA * sizeof(TCP_RCV_DATA*));

		_TRACE("\r\n%s:%ld TCP_RCV_DATA::s_hHeap = 0x%x", __FILE__, __LINE__, TCP_RCV_DATA::s_hHeap);
		_TRACE("\r\n%s:%ld TCP_RCV_DATA::s_DataHeap = 0x%x", __FILE__, __LINE__, TCP_RCV_DATA::s_DataHeap);

		//TCP_SEND_DATA
		TCP_SEND_DATA::s_hHeap = HeapCreate(0, 0, TCP_SEND_DATA::SEND_DATA_HEAP_SIZE);
		TCP_SEND_DATA::s_DataHeap = HeapCreate(0, 0, TCP_SEND_DATA::DATA_HEAP_SIZE);
		InitializeCriticalSection(&(TCP_SEND_DATA::s_IDLQueLock));
		TCP_SEND_DATA::s_IDLQue.reserve(TCP_SEND_DATA::MAX_IDL_DATA * sizeof(TCP_SEND_DATA*));

		_TRACE("\r\n%s:%ld TCP_SEND_DATA::s_hHeap = 0x%x", __FILE__, __LINE__, TCP_SEND_DATA::s_hHeap);
		_TRACE("\r\n%s:%ld TCP_SEND_DATA::s_DataHeap = 0x%x", __FILE__, __LINE__, TCP_SEND_DATA::s_DataHeap);

		//TCP_CONNECT
		TCP_CONTEXT::s_hHeap = HeapCreate(0, 0, TCP_CONTEXT::TCP_CONTEXT_HEAP_SIZE);
		InitializeCriticalSection(&(TCP_CONTEXT::s_IDLQueLock));
		TCP_CONTEXT::s_IDLQue.reserve(TCP_CONTEXT::MAX_IDL_DATA * sizeof(TCP_CONTEXT*));
	}

	void CTcpMng::ReleaseReource()
	{
		//TCPMNG_CONTEXT
		HeapDestroy(TCPMNG_CONTEXT::s_hHeap);
		TCPMNG_CONTEXT::s_hHeap = NULL;

		EnterCriticalSection(&(TCPMNG_CONTEXT::s_IDLQueLock));
		TCPMNG_CONTEXT::s_IDLQue.clear();
		LeaveCriticalSection(&(TCPMNG_CONTEXT::s_IDLQueLock));
		DeleteCriticalSection(&(TCPMNG_CONTEXT::s_IDLQueLock));

		//CONNECT_CONTEXT
		HeapDestroy(CONNECT_CONTEXT::s_hHeap);
		CONNECT_CONTEXT::s_hHeap = NULL;

		EnterCriticalSection(&(CONNECT_CONTEXT::s_IDLQueLock));
		CONNECT_CONTEXT::s_IDLQue.clear();
		LeaveCriticalSection(&(CONNECT_CONTEXT::s_IDLQueLock));

		DeleteCriticalSection(&(CONNECT_CONTEXT::s_IDLQueLock));

		//ACCEPT_CONTEXT
		HeapDestroy(ACCEPT_CONTEXT::s_hHeap);
		ACCEPT_CONTEXT::s_hHeap = NULL;

		EnterCriticalSection(&(ACCEPT_CONTEXT::s_IDLQueLock));
		ACCEPT_CONTEXT::s_IDLQue.clear();
		LeaveCriticalSection(&(ACCEPT_CONTEXT::s_IDLQueLock));

		DeleteCriticalSection(&(ACCEPT_CONTEXT::s_IDLQueLock));

		//TCP_RCV_DATA
		HeapDestroy(TCP_RCV_DATA::s_hHeap);
		TCP_RCV_DATA::s_hHeap = NULL;

		HeapDestroy(TCP_RCV_DATA::s_DataHeap);
		TCP_RCV_DATA::s_DataHeap = NULL;

		EnterCriticalSection(&(TCP_RCV_DATA::s_IDLQueLock));
		TCP_RCV_DATA::s_IDLQue.clear();
		LeaveCriticalSection(&(TCP_RCV_DATA::s_IDLQueLock));

		DeleteCriticalSection(&(TCP_RCV_DATA::s_IDLQueLock));

		//TCP_SEND_DATA
		HeapDestroy(TCP_SEND_DATA::s_hHeap);
		TCP_SEND_DATA::s_hHeap = NULL;

		HeapDestroy(TCP_SEND_DATA::s_DataHeap);
		TCP_SEND_DATA::s_DataHeap = NULL;

		EnterCriticalSection(&(TCP_SEND_DATA::s_IDLQueLock));
		TCP_SEND_DATA::s_IDLQue.clear();
		LeaveCriticalSection(&(TCP_SEND_DATA::s_IDLQueLock));
		DeleteCriticalSection(&(TCP_SEND_DATA::s_IDLQueLock));

		//TCP_CONTEXT
		HeapDestroy(TCP_CONTEXT::s_hHeap);
		TCP_CONTEXT::s_hHeap = NULL;

		EnterCriticalSection(&(TCP_CONTEXT::s_IDLQueLock));
		TCP_CONTEXT::s_IDLQue.clear();
		LeaveCriticalSection(&(TCP_CONTEXT::s_IDLQueLock));
		DeleteCriticalSection(&(TCP_CONTEXT::s_IDLQueLock));
	}
}