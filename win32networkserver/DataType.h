#ifndef _DATA_TYPE_H
#define _DATA_TYPE_H

typedef unsigned long			ULONG;
typedef int						BOOL;
typedef char					CHAR;
typedef short					SHORT;
typedef long					LONG;
typedef const void*				LPCVOID;
typedef void*					LPVOID;
typedef int						INT;
typedef unsigned int			UINT;
typedef unsigned long			DWORD;
typedef unsigned char			BYTE;
typedef unsigned short			WORD;
typedef float					FLOAT;
typedef void *HANDLE;

#ifndef VOID
#define VOID void
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


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

#ifndef THROW_LINE
#define THROW_LINE (throw ((long)(__LINE__)))
#endif		//#ifndef THROW_LINE

#endif		//#ifndef _DATA_TYPE_H