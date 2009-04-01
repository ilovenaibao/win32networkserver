#pragma once
#include <string>
#include "ClientNet.h"

using namespace std;

namespace HelpMng
{	
	//��������ɺ�Ļص�����
	typedef void (CALLBACK *LPRELAY_RESULT)(
		LPVOID pParam			//
		, int nErrCode			//0 = ����ɹ�; ��0Ϊ������
		);

	class Sock5Relay
	{
	public:	

		/************************************************************************
		*Name : Sock5Relay()
		*Desc : ����һ��TCP��ʽ��sock5����
		*Param : szUser[IN] ��¼������������û���, ��Ϊ��ʱ������֤��ʽ, ������÷���֤��ʽ
		*	szPwd[IN] ��¼�����������������
		*	szDstIP[IN] ����Ҫ���ӵ�Զ��IP��ַ
		*	nDstPort[IN] ����Ҫ���ӵ�Զ�̶˿�
		************************************************************************/
		Sock5Relay(
			const string& szUser
			, const string& szPwd
			, const CHAR* szDstIp
			, INT nDstPort
			, LPONREAD pReadFun			//����ɹ��Ժ�����ݶ�ȡ�ص�����
			, LPONERROR pErrorFun		//����Ĵ�������
			, LPRELAY_RESULT pResultFun	//�������Ļص�����
			, LPVOID pParam				//�����ص������Ĳ���
			);

		/************************************************************************
		*Name : Sock5Relay()
		*Desc : ����һ��UDP��ʽ��sock5����
		*Param : nLocalPort[IN] Ҫ���б��ذﶨ�Ķ˿�
		*	pNetWork[IN] Ҫ����Э�������ӿ�, ��Ϊ�����Զ������µ�����ӿ�
		************************************************************************/
		Sock5Relay(
			const string& szUser
			, const string& szPwd
			, INT nLocalPort
			, LPONREADFROM pReadFromFun		//���ݶ�ȡ������
			, LPONERROR pErrorFun			//���ݴ�������
			, LPRELAY_RESULT pResultFun		//������������
			, LPVOID pParam					//�����ص������Ĳ���
			);

		//Sock5Relay(const string& szUser, const string& szPwd);
		virtual ~Sock5Relay(void);

		/************************************************
		* Desc : ��ʼ����sock5����
		************************************************/
		BOOL Start(			
			const char* szRelayIp			//����������ĵ�ַ
			, int nSerPort = 1080			//����������˿�
			);

		BOOL Send(const char* szData, int nLen);

		BOOL SendTo(const IP_ADDR& peer_addr, const char* szData, int nLen);

		enum 
		{
			STATE_SUCCESS = 0,				//����ɹ�
			STATE_INIT,
			STATE_CONNECTING,				//�������Ӵ��������
			STATE_STEP_1,					//�����һ��
			STATE_STEP_1_AUTH,				//�����һ������֤��ʽ, ������֤��ʽʱ��Ч
			STATE_STEP_2,
			STATE_FAILE,					//����ʧ��
		};

	protected:        

		/************************************************************************
		* Desc : �Ӵ����������ȡ���ݵĴ�����, �����ڽ��д�������Ҫ�ں����н���
		*	����Э��Ľ���, �������ѳɹ���ֻ��Ҫ������Ӧ�Ļص���������
		* Return : ����Ҫ��ȡ�����ֽڵ�����, ���˴������Զ�ȡ���, ����0
		************************************************************************/
		int CALLBACK ReadProc(LPVOID pParam, const char* szData, int nLen);

		/************************************************************************
		* Desc : UDP����Ķ����ݺ���, ֻҪ����ɹ���ú�������Ч
		************************************************************************/
		void CALLBACK ReadFromProc(LPVOID pParam, const IP_ADDR& peer_addr, const char* szData, int nLen);

		/************************************************************************
		* Desc : ���Ӵ���������ɹ���Ļص�����
		************************************************************************/
		void CALLBACK ConnectProc(LPVOID pParam);

		/************************************************************************
		* Desc : ��������
		************************************************************************/
		void CALLBACK ErrorProc(LPVOID pParam, int nOpt);

		/************************************************************************
		*Name : ParseUdpData()
		*Desc : ���յ������������������UDP���ݰ�ʱ, ��Ҫ�ȵ��øú������н���
		*	�ú�����Ҫ����UDP���ݰ�ͷ��ȡ�Զ˵���ʵ��ַ�ͶԶ�ʵ�ʷ��͵�����
		*Param : szBuf[IN] ���������������������
		*	nBufLen[IN] ���ݵĳ���
		*	PeerAddr[OUT] ʵ�ʷ������ݰ��ĶԶ˵�ַ
		*	nDataLen[OUT] �Զ˷������ݵĳ���
		*Return : �Զ˵����ݵ�ַ
		************************************************************************/
		const CHAR* ParseUdpData(const CHAR* szBuf, INT nBufLen, IP_ADDR& PeerAddr, INT& nDataLen);

		string m_szUser;			//��¼�����������û���
		string m_szPwd;				//��¼��������������
		BOOL m_bAuth;				//�Ƿ���Ҫ��֤
		int m_nState;				//����ĵ�ǰ����
		CHAR m_szSerIP[16];			//������IP��ַ	
		INT m_nSerPort;				//�������˿�
		INT m_nNetType;				//����ģ��, TCP/UDP
		CHAR m_szDstIP[16];			//����Ҫ���ӵķ�����IP��ַ
		INT m_nDstPort;				//
		INT m_nLocalPort;			//Ҫ�ﶨ�ı��ض˿�
		IP_ADDR m_SerAddr;			//UDP��ʽ����ɹ��Ժ���UDP socketͨ�ŵķ�������ַ�Ͷ˿���Ϣ
		ClientNet* m_pRelayNet;		//����������ͨ�ŵ�����ӿ�, ������TCP����ʽ�ýӿڼ�TCP������ӿ�
		ClientNet* m_pUdpNet;		//������UDP��ʽ����ʱ, �ýӿ�����UDPͨ��
		LPONREAD m_pReadProc;		//��Ŀ�ķ������Ķ�ȡ���ݴ�����
		LPONREADFROM m_pReadFromProc;
		LPONERROR m_pErrorProc;
		LPRELAY_RESULT m_pResultProc;
		LPVOID const m_pParam;
	};

}