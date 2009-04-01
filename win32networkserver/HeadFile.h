#ifndef _HEAD_FILE_H
#define _HEAD_FILE_H

#ifndef WINVER				// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define WINVER 0x0501		// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif

#ifndef _WIN32_WINNT		// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define _WIN32_WINNT 0x0501	// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif						

#ifndef _WIN32_WINDOWS		// ����ʹ���ض��� Windows 98 ����߰汾�Ĺ��ܡ�
#define _WIN32_WINDOWS 0x0410 // ����ֵ����Ϊ�ʵ���ֵ����ָ���� Windows Me ����߰汾��ΪĿ�ꡣ
#endif

#ifndef _WIN32_IE			// ����ʹ���ض��� IE 6.0 ����߰汾�Ĺ��ܡ�
#define _WIN32_IE 0x0600	// ����ֵ����Ϊ��Ӧ��ֵ���������� IE �������汾��
#endif

#ifdef WIN32

#include <WinSock2.h>
#include <MSWSock.h>
#include <vector>

#ifdef _DLL_EXPORTS_
#define DLLENTRY __declspec(dllexport)
//#pragma comment(lib, "ws2_32.lib")

#else
#define DLLENTRY __declspec(dllimport)
#endif		//#ifdef _DLL_EXPORTS_
#else
#define DLLENTRY  
#endif		//#ifdef WIN32

#define THROW_LINE (throw ((long)(__LINE__)))

using namespace std;
namespace HelpMng
{
	enum 
	{
		OP_ACCEPT = 1,			//Ҫ�������Ͻ��л������accept����
		OP_CONNECT,				//Ҫ�������Ͻ��л������connect����
		OP_READ,				//�������Ͻ��л������READ����
		OP_WRITE,				//�������Ͻ��л������write����
	};

	class NET_CONTEXT;
	class IP_ADDR;

	//�û����Ӳ����Ļص�����, �û�ֻ�贫��lpThreadParameter��������, 
	//sock��������ϵͳ����, nState ��������״̬
	//ע���ú����в�Ҫ�й��ڸ��ӵ���������, ��Ϊ�ú�������IO�߳������õ�
	//	������ڸ��ӽ�Ӱ��IO�̵߳�ִ��
	typedef void (WINAPI *PCONNECT_ROUTINE)(
		LPVOID lpThreadParameter
		, SOCKET hSock	
		);
	typedef PCONNECT_ROUTINE LPCONNECT_ROUTINE;

	//���������ر�ʱ֪ͨ�������û�ֻ��Ҫ����pParam��������
	typedef void (WINAPI *PCLOSE_ROUTINE)(
		LPVOID pParam					//����رյĲ���
		, SOCKET hSock
		, int nOpt					//socket�ر�ʱ�����еĲ���
		);
	typedef PCLOSE_ROUTINE LPCLOSE_ROUTINE;

	/************************************************************************
	* Desc : ���û����ӷ������ɹ���Ļص�����, TCP��ʽ��Ч
	* Return : 
	************************************************************************/
	typedef void (CALLBACK *LPONCONNECT)(LPVOID pParam);

	/************************************************************************
	* Name: class NET_CONTEXT
	* Desc: ������Ҫ���ڴ�������Ķ�д�����Ļ�����, ���಻��ֱ�ӽ���ʵ����
	************************************************************************/
	class NET_CONTEXT 
	{
	public:
		WSAOVERLAPPED m_ol;
		SOCKET m_hSock;			//�������ӿ�
		CHAR* m_pBuf;			//���ܻ������ݵĻ�����, ���������ڴ淽ʽʵ��; ���ݵ�ʵ�ʳ��ȴ����BUF��ͷ�����ֽ���
		INT m_nOperation;		//����ӿ���Ҫ���еĲ���, ������OP_ACCEPT, OP_CONNECT, OP_READ, OP_WRITE		
		//volatile LONG m_nPostCount;		//1�������ڽ��ж�д����, 0����socket����, ��Ϊ-1���������socket�ر�

		static DWORD S_PAGE_SIZE;		//��ǰϵͳ���ڴ�ҳ��С

		/****************************************************
		* Name : GetSysMemPageSize()
		* Desc : ��ȡ��ǰϵͳ�ڴ�ҳ�Ĵ�С
		****************************************************/
		static DWORD GetSysMemPageSize();
		
		NET_CONTEXT();
		virtual ~NET_CONTEXT();


		/****************************************************
		* Name : InitReource()
		* Desc : �����ȫ�ּ���̬��Դ���г�ʼ��
		*	�÷�����DllMain()��������, �����߲��ܵ���
		*	�÷���; ���������ڴ�й¶
		****************************************************/
		static void InitReource();

		/****************************************************
		* Name : ReleaseReource()
		* Desc : �����ȫ�ֺ;�̬��Դ�����ͷ�
		*	�÷�����DllMain()��������, �����߲��ܵ���
		*	����.
		****************************************************/
		static void ReleaseReource();

	private:
		void* operator new (size_t nSize);
		void operator delete(void* p);
		static HANDLE s_hDataHeap;	
		static vector<char * > s_IDLQue;		//��Ч�����ݶ���
		static CRITICAL_SECTION s_IDLQueLock;		//����s_IDLQue�Ļ�����
	};


	/************************************************************************
	* Name: class IP_ADDR
	* Desc: ������Ҫ����map����
	************************************************************************/
	class DLLENTRY IP_ADDR : public sockaddr_in
	{
	public:	
		IP_ADDR() { sin_family = AF_INET; }
		IP_ADDR(const char* szIP, INT nPort);
		IP_ADDR(const IP_ADDR& other);
		~IP_ADDR() {};

		IP_ADDR& operator =(const IP_ADDR& other);
		IP_ADDR& operator =(const sockaddr_in& other);
		bool operator ==(const IP_ADDR& other)const;
		bool operator ==(const sockaddr_in& other)const;
		bool operator <(const IP_ADDR& other)const;
		bool operator <(const sockaddr_in& other)const;
	};

	/************************************************
	* Desc : ���ݰ�ͷ�Ķ���
	* ��ʽ���£�
	* |<------16------>|<------16------>|		
	* +----------------+-----------------+
	* |				���ݰ��ܳ���		  |
	* +-----------------------------------+
	* |				���ݰ������к�		 |
	* +----------------+------------------+
	* |��ǰ���ݰ��ĳ���|  ���ݰ�������  |
	* +----------------+------------------+
	* |				  ����				     |
	* .									.
	* .								    .
	* .								    .
	* +----------------+----------------+
	* ע: ���кŴ�0��ʼ; ��ǰ�յ������ݰ������к� + ��ǰ���ݰ��ĳ��� = Ҫ���յ���һ�����ݰ������к�
	* �����ݰ������к� + ��ǰ���ݰ��ĳ��� = ���ݰ����ܳ���, ˵�������͵����ݰ��ѽ������
	* ���ݰ��ĳ��Ȳ�������ͷ�ĳ���
	************************************************/
	struct PACKET_HEAD
	{
		ULONG nTotalLen;			//���ݰ����ܳ���
		ULONG nSerialNum;			//���ݰ������к�
		WORD nCurrentLen;			//��ǰ���ݰ��ĳ���
		WORD nType;					//���ݰ�������
	};


#ifdef __cplusplus
	extern "C" 
	{
#endif

	/************************************************************************
	* Desc : �Ӹ�����addr�л�ȡip��ַ�Ͷ˿�
	************************************************************************/
	void DLLENTRY GetAddress(
		const IP_ADDR& addr
		, void *pSzIp										// string ����
		, int& nPort
		);

	/************************************************************************
	* Desc : ��ȡ����IP��ַ�б�
	************************************************************************/
	BOOL DLLENTRY GetLocalIps(
		void *pIpList										//vector<string>���� 
		);

#ifdef __cplusplus
	}
#endif		//#ifdef __cplusplus

}

#endif //#ifndef _HEAD_FILE_H