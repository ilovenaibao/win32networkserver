#ifndef _FILE_STREAM_H
#define _FILE_STREAM_H

#include <string>

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

	/************************************************
	������Ҫ���ڴ����ļ������������, �������������ļ�ʱ
	����ָ��Ҫӳ����ļ��Ĵ�С
	************************************************/

	class DLLENTRY CFileStream
	{
	public:
		enum
		{
			MAP_READ,				//ֻ��ӳ��
			MAP_WRITE,				//д��ʽӳ��
			MAP_READ_WRITE,			//��д��ʽӳ��
		};

		//===============================================
		//Name : CFileStream()
		//Desc : Ϊָ�����ļ�����һ��ָ����С���ļ���, ������
		//	��ָ����СΪ0ʱ�����Ĵ�С����ָ�����ļ��Ĵ�С��ͬ
		//Param : strFile[IN] Ҫӳ����ļ�
		//	lSize[IN] ��������, Ϊ0 ��ʾ��ӳ����ļ��Ĵ�С��ͬ
		//	bEndFlag[IN] �ļ����Ƿ���������ֶ�; �����������ֶ�ʱ
		//	�ļ�������ʼ4���ֽڽ���Ԥ�����������д���. ���ֶ���
		//	�౾��ά��, �����ֶμ�����ʵ�ʴ�С
		//===============================================
		CFileStream(const char* szFile, ULONG lSize = 0, BOOL bEndFlag = FALSE);
		CFileStream() 			
			: m_lCurrPos(0)
			, m_lEndPos(0)
			, m_nState(0)
			, m_pBuff(NULL)
			, m_hFile(NULL)
			, C_STARTPOS(0)
			, m_lMaxSize(0)
		{
		}

		~CFileStream(void);

		UINT GetState()const { return m_nState; }

		//===============================================
		//Name : GetBuff()
		//Desc : ��ȡ���Ļ�����
		//===============================================
		CHAR* GetBuff()const { return (m_pBuff +C_STARTPOS); }

		//===============================================
		//Name : SeekBegin()
		//Desc : ��λ��������ʼλ�ô�
		//===============================================
		void SeekBegin() { m_lCurrPos = C_STARTPOS; }

		//===============================================
		//Name : SeekEnd()
		//Desc : ��λ�����Ľ���λ��
		//===============================================
		ULONG SeekEnd() { m_lCurrPos = m_lEndPos; return m_lCurrPos; }

		//===============================================
		//Name : Seek()
		//Desc : ��λ����ָ����λ��
		//===============================================
		void Seek(ULONG ulPos);

		/************************************************************************
		* Desc : �����ĵ�ǰ��ŵ����ַ���, ���������ַ���
		* Return : �������������ַ����ĳ���, �ó��Ȱ����ַ����Ľ�����
		************************************************************************/
		DWORD SkipString();

		/************************************************************************
		* Desc : �������ĵ�ǰλ��ָ������һ����λ��
		************************************************************************/
		void Skip(DWORD nLen);

		//===============================================
		//Name : GetCurrBuff()
		//Desc : ��ȡ��ǰλ�ô��Ļ�����
		//===============================================
		CHAR* GetCurrBuff()const { return (m_pBuff + m_lCurrPos); }

		//===============================================
		//Name : GetSize()
		//Desc : ��ȡ���ĵ�ǰʵ������, ֻ����н�����־������Ч
		//===============================================
		ULONG GetSize()const { return (m_lEndPos - C_STARTPOS); }

		/************************************************************************
		* Desc : ��ȡ�ļ�������󳤶�
		************************************************************************/
		DWORD GetFileLen()const { return m_lMaxSize; }

		//===============================================
		//Name : FillBuff()
		//Desc : ��ָ�����ȵĻ������������뵽�ļ���ָ��λ����
		// ���lPos = -1; �����ļ��ĵ�ǰλ�ý�������
		//===============================================
		void FillBuff(LONG lPos, LPVOID pBuf, UINT nSize);

		/************************************************************************
		* Desc : ���ļ���ǰλ����������ָ���Ļ�������
		* Return : �����ɹ�����true, ���򷵻�false
		************************************************************************/
		BOOL SetBuf(void *pBuf, int nSize);

		CFileStream& operator << (const INT& i);
		CFileStream& operator << (const LONG& l);
		CFileStream& operator << (const UINT& ui);
		CFileStream& operator << (const ULONG& ul);
		CFileStream& operator << (const CHAR* szSTR);
		CFileStream& operator << (const double& d);

		CFileStream& operator >> (INT& i);
		CFileStream& operator >> (LONG& l);
		CFileStream& operator >> (UINT& ui);
		CFileStream& operator >> (ULONG& ul);
		CFileStream& operator >> (string& str);
		CFileStream& operator >> (const char *&szStr);
		CFileStream& operator >> (double& d);

		//===============================================
		//Name : ��ָ��λ����������32λ��CRCУ����
		//Desc : ���ɵ�CRCУ����
		//===============================================
		DWORD MakeCrc32();

		//===============================================
		//Name : MapFile()
		//Desc : ���ļ�����ӳ��
		//===============================================
		BOOL MapFile(
			const char* szFile
			, ULONG lSize = 0
			, BOOL bEndFlag = FALSE
			, int nMapType = MAP_READ_WRITE			//ӳ�䷽ʽ
			);

		/************************************************************************
		* Desc : ȡ�����ļ���ӳ��
		************************************************************************/
		void UnMapFile();

		//===============================================
		//Name : MakeCrc32()
		//Desc : ���ض�����������32λ��crcУ����
		//===============================================
		static DWORD MakeCrc32(const char *pszData, int nLen);

		/************************************************************************
		* Desc : ���ļ��Ļ����������������
		************************************************************************/
		void ZeroBuf();

		enum
		{
			STATE_NORMAL,					//���ĵ�ǰ״̬����
			STATE_OPEN_FILE_FAIL,			//���ļ�ʧ��
			STATE_STREAM_SIZE_ERROR,		//������������
			STATE_END_STREAM,				//�ѵ�������β��
			STATE_INCREMENT_FIAL,			//����ʧ��
		};

	protected:
		//===============================================
		//Name : SetEndPos()
		//Desc : �������Ľ���λ��, ��ÿ�ν����ļ���������Ժ������ôα�־
		// ֻ����н�����־�ֶε�����Ч
		//===============================================
		inline void SetEndPos();

		//===============================================
		//Name : IncrementFile()
		//Desc : ���ļ�������ռ䲻��ʱ, ���ļ���������
		//Param : nSize[IN] : Ҫ���ݵ��ֽ���
		//		bType[IN] : �Ƿ񰴹̶��ֽڽ�������; bType = TRUE
		//		��ָ���ֽڽ�������, ����nSize < FILE_INCREMENTʱ
		//		������nSize���ֽ�, bType = FALSE ���̶��ֽڽ�������
		//		����nSize < FILE_INCREMENTʱ������FILE_INCREMENT���ֽ�
		//===============================================
		inline BOOL IncrementFile(UINT nSize = FILE_INCREMENT, BOOL bType = FALSE);

		enum
		{
			FILE_INCREMENT = 1024,			//�ļ�����, �������ļ�����������ﵽ�ļ���ĩβʱ����ļ���������
		};

		CHAR* m_pBuff;			//ӳ���ļҵĻ�����
		ULONG m_lCurrPos;		//�ļ����ĵ�ǰλ��		
		ULONG m_lEndPos;		//������Խ���λ��
		ULONG C_STARTPOS;		//������ʼλ��
		ULONG m_lMaxSize;		//���ļ����������	
		UINT m_nState;			//����״̬
		HANDLE m_hFile;
	};

}

#endif  //#ifndef _FILE_STREAM_H