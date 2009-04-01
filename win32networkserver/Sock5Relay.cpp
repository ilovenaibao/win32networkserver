#include "Sock5Relay.h"
#include <assert.h>

#pragma pack(1)

struct SOCK5REQ1
{
	char Ver;			//sock ����汾
	char nMethods;		//��֤��ʽ
	char Methods[255];	//��������
};

struct SOCK5REQ2
{
	char ver;			//sock����汾
	char cmd;			//socket����
	char rsv;			//����
	char atyp;			//��ַ����
	ULONG ulAddr;		//�ض���ַ
	WORD wPort;			//�ض��˿�
};


//UDP�����ͷ����Ϣ
struct UDP_SEND_HEAD
{
	WORD m_nRsv;			//����
	CHAR m_bSeg;			//�Ƿ��Ƭ
	CHAR m_atpy;			//��ַ����
	ULONG m_Addr;			//Ŀ�ĵ�ַ
	WORD m_nPort;			//Ŀ�Ķ˿�
};

#pragma pack()

namespace HelpMng
{

	Sock5Relay::Sock5Relay(		
		const string& szUser
		, const string& szPwd
		, const CHAR* szDstIp
		, INT nDstPort
		, LPONREAD pReadFun			//����ɹ��Ժ�����ݶ�ȡ�ص�����
		, LPONERROR pErrorFun		//����Ĵ�������
		, LPRELAY_RESULT pResultFun	//�������Ļص�����
		, LPVOID pParam				//�����ص������Ĳ���
		)
		: m_szUser(szUser)
		, m_szPwd(szPwd)
		, m_nDstPort(nDstPort)
		, m_nNetType(SOCK_STREAM)
		, m_pReadProc(pReadFun)
		, m_pErrorProc(pErrorFun)
		, m_pResultProc(pResultFun)
		, m_pParam(pParam)
		, m_pRelayNet(NULL)
		, m_pUdpNet(NULL)
		, m_nState(STATE_INIT)
	{

		assert(szDstIp);
		strcpy(m_szDstIP, szDstIp);

		m_pRelayNet = new ClientNet(TRUE);
		assert(m_pRelayNet);

		//������ػص�����
		m_pRelayNet->SetTCPCallback(this, NULL, ConnectProc, ReadProc, ErrorProc);

		if (szUser.empty())
		{
			m_bAuth = FALSE;
		}
		else
		{
			m_bAuth = TRUE;
		}
	}

	Sock5Relay::Sock5Relay(
		const string& szUser
		, const string& szPwd
		, INT nLocalPort
		, LPONREADFROM pReadFromFun
		, LPONERROR pErrorFun
		, LPRELAY_RESULT pResultFun
		, LPVOID pParam
		)
		: m_szUser(szUser)
		, m_szPwd(szPwd)
		, m_nLocalPort(nLocalPort)
		, m_nNetType(SOCK_DGRAM)
		, m_pRelayNet(NULL)
		, m_pUdpNet(NULL)
		, m_pReadFromProc(pReadFromFun)
		, m_pErrorProc(pErrorFun)
		, m_pResultProc(pResultFun)
		, m_pParam(pParam)
		, m_nState(STATE_INIT)
	{	

		m_pRelayNet = new ClientNet(TRUE);
		m_pUdpNet = new ClientNet(TRUE);

		
		assert(m_pRelayNet);
		assert(m_pUdpNet);

		if (szUser.empty())
		{
			m_bAuth = FALSE;
		}
		else
		{
			m_bAuth = TRUE;
		}
	}

	Sock5Relay::~Sock5Relay(void)
	{
		if (m_pRelayNet)
		{
			delete m_pRelayNet;
			m_pRelayNet = NULL;
		}

		if (m_pUdpNet)
		{
			delete m_pUdpNet;
			m_pUdpNet = NULL;
		}
	}
	
	BOOL Sock5Relay::Start(const char* szRelayIp, int nSerPort )
	{
		assert(pOnResult);
		m_pOnResult = pOnResult;
		m_pParam = pParam;

		//Ϊ����ӿ����ûص�����
		switch(m_nNetType)
		{			
		case SOCK_STREAM:		//tcp����ʽ
			m_pNet->SetTCPCallback(this, NULL, OnConnect, OnRead, OnError);
			break;

		case SOCK_DGRAM:		//udp����ʽ
			break;

		default:
			assert(0);
			break;
		}
	}

	INT Sock5Relay::OnRead(LPVOID pParam, const CHAR* szData, INT nDataLen)
	{
		INT nRet = 0;

		switch (m_nState)
		{
		case STATE_RELAY_STEP_1:		//����������������֤�ĵ�һ��
			if (nDataLen < 2)
			{
				InterlockedExchange(&m_nState, STATE_RELAY_FAILE);
			}
			//S -> C : ������Ӧ���� �汾�� | ������ѡ���ķ���
			//��һ����֤ͨ�����еڶ���
			//C -> S: Э��汾 | Socks���� |�������ֽڡ�| ��ַ���� | �ض���ַ | �ض��˿�
			if (0x05 == szData[0] && 0x00 == szData[1])
			{
				InterlockedExchange(&m_nState, STATE_RELAY_STEP_2);
				CHAR szBuf[256];
				SOCK5REQ2* pReq = (SOCK5REQ2*)szBuf;
				pReq->ver = 0x05;
				pReq->rsv = 0;
				pReq->atyp = 0x01;

				//TCP��ʽ����
				if (SOCK_STREAM == m_nNetType)
				{
					pReq->cmd = 0x01;
					pReq->ulAddr = inet_addr(m_szDstIP);
					pReq->wPort = htons(m_nDstPort);
				}
				//UDP��ʽ����
				else
				{
					pReq->cmd = 0x03;
					pReq->ulAddr = 0;
					pReq->wPort = htons(m_nLocalPort);
				}

				if (FALSE == CRelay::Send(szBuf, sizeof(SOCK5REQ2)))
				{
					InterlockedExchange(&m_nState, STATE_RELAY_FAILE);
				}
			} 
			else if (0x05 == szData[0] && 0x02 == szData[1])
			{
				//S -> C : ������Ӧ���� �汾�� | ������ѡ���ķ���
				//C -> S: 0x01 | �û������ȣ�1�ֽڣ�| �û��������ȸ����û���������ָ���� | ����ȣ�1�ֽڣ� | ��������ɿ������ָ����
				InterlockedExchange(&m_nState, STATE_RELAY_STEP_1_AUTH);
				INT nAllLen = 3;
				INT nTemp;
#define LEN 100
				CHAR szBuf[512] = { 0 };

				szBuf[0] = 0x01;
				nTemp = (INT)(m_szUser.length() > LEN ? LEN : m_szUser.length());
				szBuf[1] = (CHAR)nTemp;
				strcpy(szBuf+2, m_szUser.c_str());
				nAllLen += nTemp;
				szBuf[nAllLen -1] = (CHAR)(m_szPwd.length() > LEN ? LEN : m_szPwd.length());
				nTemp = szBuf[nAllLen -1];
				strcpy(szBuf +nAllLen, m_szPwd.c_str());
				nAllLen += nTemp;

				if (FALSE == CRelay::Send(szBuf, nAllLen))
				{
					InterlockedExchange(&m_nState, STATE_RELAY_FAILE);
				}

			}
			else
			{
				InterlockedExchange(&m_nState, STATE_RELAY_FAILE);
			}
			break;

		case STATE_RELAY_STEP_1_AUTH:		//�û�������֤�ķ�ʽ
			//S -> C: 0x01 | ��֤�����־
			//C -> S: Э��汾 | Socks���� |�������ֽڡ�| ��ַ���� | �ض���ַ | �ض��˿�
			if (nDataLen < 2)
			{
				InterlockedExchange(&m_nState, STATE_RELAY_FAILE);
			}
			if (0x05 == szData[0] && 0x00 == szData[1])
			{
				InterlockedExchange(&m_nState, STATE_RELAY_STEP_2);
				CHAR szBuf[256];
				SOCK5REQ2* pReq = (SOCK5REQ2*)szBuf;
				pReq->ver = 0x05;
				pReq->rsv = 0;
				pReq->atyp = 0x01;

				//TCP��ʽ����
				if (SOCK_STREAM == m_nNetType)
				{
					pReq->cmd = 0x01;
					pReq->ulAddr = inet_addr(m_szDstIP);
					pReq->wPort = htons(m_nDstPort);
				}
				//UDP��ʽ����
				else
				{
					pReq->cmd = 0x03;
					pReq->ulAddr = 0;
					pReq->wPort = htons(m_nLocalPort);
				}

				if (FALSE == CRelay::Send(szBuf, sizeof(SOCK5REQ2)))
				{
					InterlockedExchange(&m_nState, STATE_RELAY_FAILE);
				}
			}
			else
			{
				InterlockedExchange(&m_nState, STATE_RELAY_FAILE);
			}
			break;

		case STATE_RELAY_STEP_2:		//�����������ĵڶ�����֤
			//S -> C:  �汾 | �����Ӧ�� |������1�ֽڡ�| ��ַ���� | �����������ַ | �󶨵Ĵ���˿�
			if (nDataLen < sizeof(SOCK5REQ2))
			{
				InterlockedExchange(&m_nState, STATE_RELAY_FAILE);
			}
			if (0x05 == szData[0] && 0 == szData[1])
			{
				InterlockedExchange(&m_nState, STATE_RELAY_SUCCESS);
				SOCK5REQ2* pReq = (SOCK5REQ2*)szData;

				if (SOCK_DGRAM == m_nNetType)
				{
					m_SerAddr.sin_port = pReq->wPort;
					m_SerAddr.sin_addr.s_addr = pReq->ulAddr;

					//Ϊm_UdpSockͶ�ݽ��ղ���
					PostRecv(m_pUdpRcvContext, m_UdpSock);
				}
			}
			else
			{
				InterlockedExchange(&m_nState, STATE_RELAY_FAILE);
			}
			break;

		default:
			break;

		}
		return nRet;
	}


	void Sock5Relay::OnConnect(INT iErrorCode)
	{
		//����ʧ��
		if (0 != iErrorCode)
		{
			//���Ӵ��������ʧ��
			if (STATE_CONNECT_SER == m_nState)
			{
				InterlockedExchange(&m_nState, STATE_RELAY_FAILE);
			}
		}
		//���ӳɹ�
		else if (STATE_CONNECT_SER == m_nState)
		{		
			//�ɹ����ӵ����������, �������������֤��Ϣ
			//�汾�ţ�1�ֽڣ� | �ɹ�ѡ�����֤����(1�ֽ�) | �������У�1-255���ֽڳ��ȣ�

			//Ͷ�ݶ�����
			if (FALSE == PostRecv(m_pRcvContext, m_hSock))
			{
				InterlockedExchange(&m_nState, STATE_RELAY_FAILE);
				return;
			}

			CHAR szBuf[512];
			SOCK5REQ1* pReq1 = (SOCK5REQ1*)szBuf;
			INT nLen = 0;
			if (FALSE == m_bAuth)
			{
				pReq1->Ver = 0x05;
				pReq1->nMethods = 0x01;
				pReq1->Methods[0] = 0x00;
				nLen = 3;				
			}
			else
			{
				pReq1->Ver = 0x05;
				pReq1->nMethods = 0x01;
				pReq1->Methods[0] = 0x02;
				nLen = 3;
			}			

			InterlockedExchange(&m_nState, STATE_RELAY_STEP_1);

			if (FALSE == CRelay::Send(szBuf, nLen))
			{
				InterlockedExchange(&m_nState, STATE_RELAY_FAILE);
			}		
		}
	}

	BOOL Sock5Relay::SendTo(const IP_ADDR& PeerAddr, const CHAR* szData, INT nLen)
	{
		//���ݷǷ�
		if (NULL == szData || (nLen + sizeof(UDP_SEND_HEAD)) > RELAY_CONTEXT::BUF_SIZE 
			|| SOCK_DGRAM != m_nNetType || STATE_RELAY_SUCCESS != m_nState)
		{
			return FALSE;
		}

		BOOL bResult = TRUE;
		INT iErrCode = 0;

		//������û�з��Ͳ���ֱ�ӽ���Ͷ�ݲ���
		if ( 0 == m_pUdpSendContext->m_nPostCount)
		{
			WSABUF SendBuf;
			DWORD dwBytes;
			InterlockedExchange(&(m_pUdpSendContext->m_nPostCount), 1);
			m_pUdpSendContext->m_hSock = m_UdpSock;
			m_pUdpSendContext->m_lAllData = nLen + sizeof(UDP_SEND_HEAD);
			m_pUdpSendContext->m_nDataLen = 0;
			m_pUdpSendContext->m_nOperation = OP_WRITE;
			m_pUdpSendContext->m_PeerIP = m_SerAddr;
			memcpy(m_pUdpSendContext->m_pBuf +sizeof(UDP_SEND_HEAD), szData, nLen);	
			UDP_SEND_HEAD* pHead = (UDP_SEND_HEAD*)m_pUdpSendContext->m_pBuf;
			pHead->m_Addr = PeerAddr.sin_addr.s_addr;
			pHead->m_atpy = 1;
			pHead->m_bSeg = 0;
			pHead->m_nPort = PeerAddr.sin_port;
			pHead->m_nRsv = 0;
			SendBuf.buf = m_pUdpSendContext->m_pBuf;
			SendBuf.len = m_pUdpSendContext->m_lAllData;

			iErrCode = WSASendTo(m_UdpSock, &SendBuf, 1, &dwBytes, 0
				,(sockaddr*)&(m_pUdpSendContext->m_PeerIP), sizeof(m_pUdpSendContext->m_PeerIP), &(m_pUdpSendContext->m_ol), NULL);
			if (SOCKET_ERROR == iErrCode && ERROR_IO_PENDING != WSAGetLastError())
			{
				closesocket(m_UdpSock);
				InterlockedExchange(&(m_pUdpSendContext->m_nPostCount), 0);
				bResult = FALSE;
			}
		}
		//����IO���ڽ��з��Ͳ���, �����ݷ��������
		else
		{
			EnterCriticalSection(&m_SendDataLock);

			RELAY_SEND_DATA* pSendData = new RELAY_SEND_DATA(nLen + sizeof(UDP_SEND_HEAD));
			pSendData->m_PeerIP = m_SerAddr;
			memcpy(pSendData->m_pData +sizeof(UDP_SEND_HEAD), szData, nLen);
			UDP_SEND_HEAD* pHead = (UDP_SEND_HEAD*)(pSendData->m_pData);
			pHead->m_Addr = PeerAddr.sin_addr.s_addr;
			pHead->m_atpy = 1;
			pHead->m_bSeg = 0;
			pHead->m_nPort = PeerAddr.sin_port;
			pHead->m_nRsv = 0;

			if (RELAY_SEND_DATA::IsAddressValid(pSendData) && pSendData->IsDataValid())
			{
				m_SendDataQue.push_back(pSendData);
			}
			else
			{
				delete pSendData;
				bResult = FALSE;
			}

			LeaveCriticalSection(&m_SendDataLock);
		}

		return bResult;
	}

	const CHAR* Sock5Relay::ParseUdpData(const CHAR* szBuf, INT nBufLen, IP_ADDR& PeerAddr, INT& nDataLen)
	{
		//���ݷǷ�
		if (nBufLen < sizeof(UDP_SEND_HEAD))
		{
			return NULL;
		}

		UDP_SEND_HEAD* pHead = (UDP_SEND_HEAD*)szBuf;
		PeerAddr.sin_addr.s_addr = pHead->m_Addr;
		PeerAddr.sin_port = pHead->m_nPort;

		nDataLen = nBufLen - sizeof(UDP_SEND_HEAD);
		return (szBuf +sizeof(UDP_SEND_HEAD));
	}

}