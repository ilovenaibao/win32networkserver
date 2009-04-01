#ifndef _CLIENT_NET_H
#define _CLIENT_NET_H

#include "HeadFile.h"
#include <vector>
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

using namespace std;

namespace HelpMng
{
	class DLLENTRY ClientNet;

	/************************************************************************
	* Desc : ������ģ���յ�����ʱ����øú���������յ�������; TCP��ʽ��Ч
	* Return : ����Ҫ�������Ͻ��ն��ٸ��ֽڵ�����, ����0��ʾ�˴������ѽ������
	************************************************************************/
	typedef int (CALLBACK *LPONREAD)(
		LPVOID pParam			//�ò������û�ͨ��SetCallbackParam��������
		//���²���������ģ���ṩ
		, const char* szData	//�������Ͻ��յ������ݻ�����, �û����ܽ����ͷŲ���								
		, int nLen				//���ݻ������ĳ���
		);

	/************************************************************************
	* Desc : UDP��ʽ���ݽ��մ���
	* Return : 
	************************************************************************/
	typedef void (CALLBACK *LPONREADFROM)(
		LPVOID pParam				//�ò������û��ṩ
		//���²���������ģ���ṩ
		, const IP_ADDR& PeerAddr	//�������ݵĶԷ���ַ
		, const char* szData		//���ݻ�����
		, int nLen					//���ݻ���������
		);

	/************************************************************************
	* Desc : ���������пͻ������ӵ���������ʱ�Ļص�����
	* Return : 
	************************************************************************/
	typedef void (CALLBACK *LPONACCEPT)(
		LPVOID pParam				//�ò������û��ṩ
		//���²���������ģ���ṩ
		, const SOCKET& sockClient	//���ӵ������Ŀͻ���socket
		, const IP_ADDR& AddrClient	//���ӵ������Ŀͻ��˵�ַ
		);

	/************************************************************************
	* Desc : ����⵽�������,��socket�رյȴ���ʱ��ص��ú���
	* Return : 
	************************************************************************/
	typedef void (CALLBACK *LPONERROR)(
		LPVOID pParam				//�ò������û��ṩ
		, int nOperation			//����ģ�鵱ǰ���ڽ��еĲ���
		);

	//�ͻ��˵�����Context��
	class CLIENT_CONTEXT : public NET_CONTEXT
	{
		friend class ClientNet;
		//friend class Sock5Relay;
	public:
		ClientNet* const m_pClient;		//���context��صĴ�����
		SOCKET m_ClientSock;			//���ӵ���������socket, ����Accept����
		IP_ADDR m_PeerIP;				//�Զ�IP��ַ, ֻ��UDP��ʽ��Ч
		volatile LONG m_nPostCount;		//1�������ڽ��ж�д����, 0����socket����, ��Ϊ-1���������socket�ر�
		WORD m_nAllLen;					//Ҫ����������ܳ���, TCP��Ч
		WORD m_nCurrentLen;				//��ǰ�Ѵ�������ݳ���, TCP��Ч

		CLIENT_CONTEXT(ClientNet* pClient) 
			: m_pClient(pClient)
			, m_nAllLen (0)
			, m_nCurrentLen (0){};

		virtual ~CLIENT_CONTEXT() {};

		void* operator new (size_t nSize);
		void operator delete(void* p);

		enum
		{
			HEAP_SIZE = 1024 *1024,
			MAX_IDL_NUM = 5000,			//���ж��е���󳤶�
		};

	private:
		static HANDLE s_hHeap;			//NET_CONTEXT�Լ��Ķ��ڴ�, ������Ϊ800K, ����������20000����NET_CONTEXT����
		static vector<CLIENT_CONTEXT*> s_IDLQue;	//���е�CLIENT_TCP_CONTEXT����
		static CRITICAL_SECTION s_IDLQueLock;			//����s_IDLQue�����ݶ���

		//�жϸ����ĳ��ָ������Ƿ���Ч
		static BOOL IsAddressValid(LPCVOID lpMem);
	};

	//�������ݵĶ��л�����
	class CLIENT_SEND_DATA
	{
		friend class ClientNet;
	public:
		CHAR* m_pData;			//���ݻ�����
		INT m_nLen;				//Ҫ���͵����ݳ���
		IP_ADDR m_PeerIP;		//���ĵ�Ŀ�ĵ�, ֻ��UDP��ʽ��Ч

		CLIENT_SEND_DATA(const CHAR* szData, INT nLen);
		CLIENT_SEND_DATA(const CHAR* szData, INT nLen, const IP_ADDR& peer_ip);
		~CLIENT_SEND_DATA();

		void* operator new(size_t nSize);
		void operator delete(void* p);

		static BOOL IsAddressValid(LPCVOID pMem);
		BOOL IsDataValid();

		enum
		{	
			RELAY_HEAP_SIZE = 400 *1024,
			DATA_HEAP_SIZE = 10 *1024* 1024,
			MAX_IDL_NUM = 10000,
		};

	private:
		static HANDLE s_DataHeap;
		static HANDLE s_hHeap;
		static vector<CLIENT_SEND_DATA*> s_IDLQue;		//���е�CLIENT_SEND_DATA����
		static CRITICAL_SECTION s_IDLQueLock;				//����s_IDLQue�����ݶ���
	};
	
	/************************************************************************
	*Name : class ClientNet
	*Desc : ������Ҫ���ڿͻ��˵�������򿪷�
	************************************************************************/
	class DLLENTRY ClientNet
	{

	public:

		/************************************************************************
		* Desc : ��ʼ����̬��Դ��������TCPʵ������֮ǰӦ�ȵ��øú���, ��������޷���������
		************************************************************************/
		static void InitReource();

		/************************************************************************
		* Desc : ���ͷ�TCPʵ���Ժ�, ���øú����ͷ���ؾ�̬��Դ
		************************************************************************/
		static void ReleaseReource();

		ClientNet(
			BOOL bUseCallback		//�Ƿ���ûص������ķ�ʽ, ��������麯���̳еķ�ʽ����
			);
		//ͨ��һ����Ч��socket����һ��ClientNet����, ���������˼�����һ���ͻ���������ʱ���ô˺������д���
		//����OnAccept()����������һ�����ӵ���ʱӦ��Ϊ�µ�����socket����һ���µ�����ģ�����
		//����ʼ��ʧ����GetState()����STATE_SOCK_CLOSE
		ClientNet(
			const SOCKET& SockClient
			, BOOL bUseCallback		//�Ƿ���ûص������ķ�ʽ, ��������麯���̳еķ�ʽ����
			, LPVOID pParam			//bUseCallback = trueʱ��Ч
			, LPONREAD pOnRead		//bUseCallback = trueʱ��Ч
			, LPONERROR pOnError	//bUseCallback = trueʱ��Ч
			);
		virtual ~ClientNet(void);

		//��ʼ������, ���ڽ�����ʹ��
		//nNetType ȡֵSOCK_STREAM �� SOCK_DGRAM
		//virtual BOOL Init(INT nNetType, const CHAR* szSerIP, INT nSerPort);

		//��ָ���Ķ˿��Ͻ��м���, ����TCP
		BOOL Listen(
			INT nPort				//Ҫ�����ı��ض˿�
			, BOOL bReuse			//�Ƿ���е�ַ����
			, const char* szLocalIp	//Ҫ�������õı��ص�ַ, bReuse=TRUEʱ��Ч
			);

		//�󶨵�ָ���Ķ˿���, ����UDP
		BOOL Bind(			
			INT nPort		//���ض˿�
			, BOOL bReuse	//�Ƿ���õ�ַ����
			, const char* szLocalIp //Ҫ�������õı��ص�ַ, bReuse=TRUEʱ��Ч
			);

		//���ӵ�ָ���ķ�������, �˴������첽����
		BOOL Connect(
			const CHAR* szHost	//Զ�̷�������ַ
			, INT nPort			//Ҫ���ӵķ������˿�
			, BOOL bReuse		//�Ƿ���õ�ַ����
			, const char* szLocalIp //Ҫ�������õı��ص�ַ, bReuse=TRUEʱ��Ч			
			, int nLocalPort	//Ҫ���õı��ض˿�, bReuse=TRUEʱ��Ч			
			);

		//�ر�����ӿ�
		void Close();

		//TCP��ʽ�ķ�������
		BOOL Send(const CHAR* szData, INT nLen);

		//UDP��ʽ�����ݷ���
		BOOL SendTo(const IP_ADDR& PeerAddr, const CHAR* szData, INT nLen);

		/************************************************************************
		* Desc : ����TCPģʽ�Ļص���������,�������˻ص������ķ�ʽʱ��Ӧ�ý��д�����
		*	����������Ч; �ò���Ӧ����Listen()��Connect()����֮ǰ����
		* Return : 
		************************************************************************/
		void SetTCPCallback(
			LPVOID pParam
			, LPONACCEPT pOnAccept		//Listen���͵�socket��Ч, connect���͵�socketΪNULL
			, LPONCONNECT pOnConnect	//connect���͵�socket��Ч, ����ΪNULL
			, LPONREAD pOnRead			//connect���͵�socket��Ч, ����ΪNULL
			, LPONERROR pOnError
			);

		/************************************************************************
		* Desc : ����UDPģʽ�Ļص���������, �����ûص������Ĳ���ʱ��Ӧ�ý��д�����
		*	����������Ч; �ò���Ӧ����bind()����֮ǰ����
		* Return : 
		************************************************************************/
		void SetUDPCallback(
			LPVOID pParam
			, LPONREADFROM pOnReadFrom
			, LPONERROR pOnError
			);

		/************************************************************************
		* Desc : ��ȡ���˵�ַ�Ͷ˿���Ϣ, ֻ��listen��connect�͵�socket��Ч; �Զ����ӵ�
		*	���˵ĵ�ַ��Ϣ��OnAccept��������
		************************************************************************/
		void GetLocalAddr(string& szIp, int& nPort);

		void* operator new(size_t nSize);
		void operator delete(void* p);

		//��ش����붨��
		enum
		{
			ERR_SOCKET_TYPE = 0x10000000,		//socket���ʹ���
			ERR_SOCKET_INVALID,					//��Ч��socket
			ERR_INIT,							//��ʼ��ʧ��
		};

	protected:
		typedef vector<CLIENT_SEND_DATA* > SEND_QUEUE;

		SOCKET m_hSock;
		const BOOL c_bUseCallback;			//�Ƿ���ûص������ķ�ʽ����
		void* m_pParam;						//�ص���������, c_bUseCallback = trueʱ��Ч
		LPONREAD m_pOnRead;
		LPONREADFROM m_pOnReadFrom;
		LPONCONNECT m_pOnConnect;
		LPONACCEPT m_pOnAccept;
		LPONERROR m_pOnError;

		//volatile LONG m_nState;			//״̬����
		CLIENT_CONTEXT* m_pRcvContext;		//�������ݻ�Accept��Context
		CLIENT_CONTEXT* m_pSendContext;		//�������ݻ����ӷ�������Context
		SEND_QUEUE m_SendDataQue;	//�������ݵĻ���������
		CRITICAL_SECTION m_SendDataLock;			// ����m_SendDataQue�Ļ�����
		static LPFN_CONNECTEX s_pfConnectEx;		//ConnectEx�����ĵ�ַ
		static LPFN_ACCEPTEX s_pfAcceptEx;			//AcceptEx�����ĵ�ַ
		static LPFN_GETACCEPTEXSOCKADDRS s_pfGetAddrs;	//GetAcceptExSockaddrs�����ĵ�ַ		

		//IO������
		static void CALLBACK IOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
		static void ReadCompletion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
		static void SendCompletion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
		static void AcceptCompletion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		//���������յ�����ʱ����ø÷���, �û������ͷ�szData, ��Ϊϵͳ����
		//���ػ�����ܶ೤������. ������0��ʾһ�����������ݰ��ѽ�����; ���Խ�����һ�����ݰ��Ľ���
		//�÷�������TCP�Ľ���, �����������Ӧ��ʵ��
		virtual INT OnRead(const CHAR* szData, INT nDataLen);
		
		//UDP��ʽ�����ݽ���, ˵��ͬOnRead()
		virtual void OnReadFrom(const IP_ADDR& PeerAddr, const CHAR* szData, INT nDataLen);
		
		//�����ӳɹ�����ø÷���
		virtual void OnConnect();

		//����⵽�����ӵĿͻ�, ����ø÷���
		virtual void OnAccept(const SOCKET& SockClient, const IP_ADDR& AddrClient);

		//����⵽���ӶϿ��������ѹر�ʱ���ø÷���
		virtual void OnError(
			int nOperation			//��ǰ���ڽ��еĲ���, ����/����/����/���� ������������, �μ���ö������
			);

		//Ͷ�ݶ�����
		BOOL PostRecv(CLIENT_CONTEXT* const pContext, const SOCKET& sock);
		inline BOOL PostRecv();

		//�ӷ������ݻ�����������ȡ��һ����Ч�ķ�������, ������Ϊ�շ���NULL
		CLIENT_SEND_DATA* GetSendData();

		//����������ݶ���
		void ClearSendQue();

		/************************************************************************
		* Desc : ��ȡsocket������, tcp/udp
		* Return : SOCK_STREAM, SOCK_DGRAM
		************************************************************************/
		inline int GetSockType();
        
	private:
		static HANDLE s_hHeap;			//�����������ʵ���Ķ�
		static vector<ClientNet*> s_IDLQue;		//���е�ClientNet����
		static CRITICAL_SECTION s_IDLQueLock;	//����s_IDLQue���ݶ��л�����

		static BOOL IsAddressValid(LPCVOID pMem);

		enum
		{
			E_BUF_SIZE = 4096,
			E_HEAP_SIZE = 300 *1024,		 //����ʵ��������Ϊ500K
			E_MAX_IDL_NUM = 2000,			 //����ʵ�����е���󳤶�
		};
	};
}

#endif			//#ifndef _CLIENT_NET_H