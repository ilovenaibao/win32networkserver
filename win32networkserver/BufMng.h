#ifndef _BUF_MNG_H
#define _BUF_MNG_H

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

//��ģ����Ҫ���������Ĺ���
namespace HelpMng
{

	//���಻��Ҫ��ʵ������
	class DLLENTRY BufMng
	{	
	public:
		/************************************************************************
		* Desc : ����һ���µĻ�����
		* Return : ����ɹ�������Ӧ�ĵ�ַ, ʧ���򷵻�NULL
		************************************************************************/
		static char* _new(
			int nSize = 1		//Ҫ����Ļ���������
			);

		/************************************************************************
		* Desc : ��������������Ҫ�����ͷ�
		* Return : 
		************************************************************************/
		static void _delete(void* p);

	private:
		BufMng() {}
		~BufMng() {}

		enum
		{
			BUF_CAP = 1024 * 1024 *10,		//��������������
		};

		static unsigned long PAGE_SIZE;		//һ���ڴ�ҳ�Ĵ�С
	};

	//���BufMng��ʹ��
	class DLLENTRY BufData
	{
	public:
		BufData();
		~BufData();
		
		/************************************************************************
		* Desc : szBuf ����BufMng����
		************************************************************************/
		BufData(char *szBuf);

		/************************************************************************
		* Desc : ��ֵ����
		************************************************************************/
		BufData &operator = (char *szBuf);

        
		char *pBufData;					//��BufMng���ɵ����ݿռ�
	};
}

#endif			//#ifndef _BUF_MNG_H