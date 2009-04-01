#ifndef _DATA_STREAM_H
#define _DATA_STREAM_H
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

	//===============================================
	//Name : class CDataStream
	//Desc : ������������ڴ�ķ�ʽ��ʵ����
	//===============================================

	class DLLENTRY CDataStream
	{
	public:
		//===============================================
		//Name : CDataStream()
		//Desc : ����һ����, �����ĳ�ʼ�������Ϊ1���ڴ�ҳ(4K/8K)�Ĵ�С
		//	���ַ�ʽ�������Խ�������, ÿ�ε���������һ���ڴ�ҳ�Ĵ�С
		//  �����ݺ����Ļ���������ַ���ܻᷢ���ı�
		//===============================================
		CDataStream(void);

		//===============================================
		//Name : CDataStream_AWE()
		//Desc : ����һ��ָ���������, ���ַ�ʽ����������ܽ�������
		//===============================================
		CDataStream(LPVOID pBuf, LONG lMaxSize, long lEndPos = 0);

		//===============================================
		//Name : GetState()
		//Desc : ��ȡ����״̬
		//===============================================
		INT GetState()const { return m_iState; }

		//===============================================
		//Name : GetMaxSize()
		//Desc : ��ȡ�����������
		//===============================================
		INT GetMaxSize()const { return m_lMaxSize; }

		//===============================================
		//Name : GetCurSize()
		//Desc : �������ĵ�ǰ����
		//===============================================
		INT GetCurSize()const { return m_lEndPos; }

		//===============================================
		//Name : GetBuf()
		//Desc : ��ȡ���Ļ������ĵ�ַ
		//===============================================
		char* GetBuf()const { return m_pBuf; }

		//===============================================
		//Name : ZeroBuf()
		//Desc : ���������ݽ�����0
		//===============================================
		void ZeroBuf();

		//===============================================
		//Name : SeekBegin()
		//Desc : ��λ��������ʼλ��
		//===============================================
		void SeekBegin() { m_lCurPos = 0; }

		//===============================================
		//Name : SeekEnd()
		//Desc : ��λ�����Ľ���λ��
		//===============================================
		void SeekEnd() { m_lCurPos = m_lEndPos; }

		//===============================================
		//Name : Seek()
		//Desc : ��λ������ĳ��λ��
		//===============================================
		void Seek(LONG lPos)
		{
			if (lPos < 0)
			{
				lPos = 0;
			}
			if (lPos > m_lEndPos)
			{
				lPos = m_lEndPos;
			}
			m_lCurPos = lPos;
		}


		/************************************************************************
		* Desc : �����б�����һ������, ��������λ�ö�λ�����������Ժ�
		* Return : �����ɹ�����true; ���򷵻�false
		************************************************************************/
		bool Reserve(long lSize);

		//===============================================
		//Name : operator <<
		//Desc : ��ص��������
		//===============================================
		CDataStream& operator << (CHAR c);
		CDataStream& operator << (BYTE b);
		CDataStream& operator << (INT i);
		CDataStream& operator << (LONG l);
		CDataStream& operator << (UINT ui);
		CDataStream& operator << (ULONG ul);
		CDataStream& operator << (const CHAR* szStr);
		CDataStream& operator << (double d);
		CDataStream& operator << (WORD w);
		CDataStream& operator << (bool b);

		//===============================================
		//Name : operator >>
		//Desc : ��ص��������
		//===============================================
		CDataStream& operator >> (CHAR& c);
		CDataStream& operator >> (BYTE& b);
		CDataStream& operator >> (INT& i);
		CDataStream& operator >> (LONG& l);
		CDataStream& operator >> (UINT& ui);
		CDataStream& operator >> (ULONG& ul);
		CDataStream& operator >> (string& str);
		CDataStream& operator >> (const char *&szStr);		//����Ϊ������ռ�
		CDataStream& operator >> (double& d);
		CDataStream& operator >> (WORD& w);
		CDataStream& operator >> (bool& b);

		//�����е�����������ص����ݻ�����
		BOOL FillBuf(void *pBuf, int nLen);

		//���������е����ݷ�������
		BOOL SetBuf(const char *pBuf, int nLen);

		~CDataStream(void);

		enum
		{
			STATE_NOERROR,			//����״̬����
			STATE_ALLOC_FIAL,		//�����ڴ�ʧ��
			STATE_BUFF_NULL,		//���Ļ�����Ϊ��
			STATE_MEM_MAX,			//�����ڴ������Ѵﵽ���, �޷���������
			STATE_INCREMENT_FAIL,	//����ʧ��
			STATE_OUT_END,			//�Ѿ���������β��
			STATE_UNKNOW_ERROR,		//δ֪����
		};

	protected:
		//===============================================
		//Name : GetBlockSize()
		//Desc : ��ȡ�������ݿ�Ĵ�С
		//===============================================
		static DWORD GetBlockSize();

		//===============================================
		//Name : Increment()
		//Desc : ������������������������ݲ���ʱ���������
		//===============================================
		BOOL Increment(LONG lSize = 0);

		enum
		{
			INCREMENT_MAX_SIZE = 4096 * 1024,	//���������������
		};

		CHAR* m_pBuf;						//�������Ļ�������ַ
		const BOOL m_bIncrement;			//�Ƿ������������
		static const DWORD s_lBlockSize;	//ÿ����������ݿ�Ĵ�С
		LONG m_lMaxSize;					//�����������
		LONG m_lEndPos;						//������Խ���λ��
		LONG m_lCurPos;						//���ĵ�ǰλ��
		INT m_iState;						//���ĵ�ǰ״̬
	};

}

#endif		//#ifndef _DATA_STREAM_H
