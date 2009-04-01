#ifndef _RUN_LOG_
#define _RUN_LOG_

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

namespace HelpMng
{
	//��ģ����Ҫ��¼��ص�������־��Ϣ,�Ա����Ա�ķ������̰߳�ȫ��
	//������Զ���Ӧ�ó���Ŀ¼�´���һ��logĿ¼; ������һ���Ե�������
	//Ϊ����log�ļ�, ÿ���ļ��Ŀ�д������Ϊ2M����, ������2Mʱ���½�һ��
	//�ļ�; ʹ�ø�ģ��ʱ����stdafx.h�ļ�����ӻ��޸����º�:
	//#define WINVER 0x0400   #define _WIN32_WINNT 0x0500
	//#define _WIN32_WINDOWS 0x0410
	class DLLENTRY RunLog
	{
	public:
		static RunLog* _GetObject();
		static void _Destroy();
		static const char *GetModulePath();

		void WriteLog(const char* szFormat, ...);
		void SetSaveFiles(LONG nDays);

		/************************************************************************
		* Desc : �Ƿ���־��ӡ����Ļ��
		************************************************************************/
		void SetPrintScreen(BOOL bPrint = TRUE);

	protected:
		RunLog(void);
		~RunLog(void);
		BOOL CreateMemoryFile();
		inline void SetFileRealSize();
		//��̨�̴߳�����
		static UINT WINAPI ThreadProc(void *lpParam);
		
		enum { FILE_BUFF_SIZE = 1024 };			//�ļ�Ԥ���Ļ����С
		enum { FILE_MAX_SIZE = 1024* 1024 +FILE_BUFF_SIZE };	//��־�ļ�����󳤶�
		enum { TIMEQUEUE_PERIOD = 1000 *60 *60 *24 };	//�߳������ִ��ʱ����Ϊ24��Сʱ

		static RunLog* s_RunLog;		//ȫ��Ψһ��־����	
		static BOOL s_bThreadRun;		//�Ƿ������̨�̼߳�������
		static char s_szModulePath[512];

		CHAR* m_pDataBuff;				//ָ���ļ��Ļ�����ָ��		
		HANDLE m_hMutex;				//���߳�����Դ�ķ�����
		HANDLE m_hThread;				//�̵߳ľ��
		UINT m_nFileCount;				//��ǰ�Դ������ļ�����,���ڴ������ļ���
		LONG m_nDays;					//���Ա����������ļ�
		BOOL m_bInit;					//��ʼ�������Ƿ�ɹ�
		BOOL m_bPrintScreen;			//�Ƿ���־��ӡ����Ļ��
		ULONG m_ulCurrSize;				//��ǰ��д����ֽ�����
		SYSTEMTIME m_tCurrTime;			//��ǰҪд���ļ���ʱ��
		SYSTEMTIME m_tLastTime;			//ǰһ���ʱ��
	};

#ifndef _TRACE
#define _TRACE (RunLog::_GetObject()->WriteLog)
#endif

	//���ڼ�¼һЩ������Ϣ
#define TRACELOG(x) ((RunLog::_GetObject())->WriteLog("%s L%ld %s", __FILE__, __LINE__, (x)))

	//���ڵ���һЩ������Ϣ,ʹ�÷���: TRACEFUN("funName", y++)
#define TRACEFUN(x, y) ((RunLog::_GetObject())->WriteLog("%s L%ld %s__%ld", __FILE__, __LINE__, (x), (y)))

#define TRACE1NUM(x, y, Param) ((RunLog::_GetObject())->WriteLog("%s L%ld %s__%ld PARAM = 0x%x", __FILE__, __LINE__, (x), (y), (Param)))

#define TRACE1STR(x, y, Param) ((RunLog::_GetObject())->WriteLog("%s L%ld %s__%ld STR_PARAM = %s", __FILE__, __LINE__, (x), (y), (Param)))

	//���ڽ��е���ʱ��¼�����Ϣ
#define TRACEDEBUG1STR(x) (TRACE(_T("\r\n%s %s L%ld STR = %s"), CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S"), __FILE__, __LINE__, x)) 
#define TRACEDEBUG1NUM(x) (TRACE(_T("\r\n%s %s L%ld PARAM = %ld"), CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S"), __FILE__, __LINE__, x))

}

#endif