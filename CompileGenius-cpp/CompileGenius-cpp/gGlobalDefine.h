#pragma once

#include "StdAfx.h"


#if !defined _GLOBAL_DEFINE_H
#define _GLOBAL_DEFINE_H

//////////////////////////////////////////////////////////////////////////
enum COMPILER_VER
{
	vsunknown = 0,
	vs60,
	vs80,
	vs90,
	vs100,
	vs110,
};


//////////////////////////////////////////////////////////////////////////

enum CFG_TYPE {
	vcproject = 0,
	copyfile,
	delfolder,
};
//////////////////////////////////////////////////////////////////////////
typedef struct compile_instance
{
	CString szInstanceName;			// ����ѡ�������  �磺Release_Chutian, Debug
	CString szPlatfromVer;		// �����ƽ̨ѡ�� �磺Win32�� x64
	bool isSelected;			// �Ƿ�α���ѡ�ѡ��
}COMPILE_INSTANCE;

//////////////////////////////////////////////////////////////////////////
typedef struct compile_node
{
	CString sz_PrjFilePath; // �����ļ���·��

	COMPILER_VER nCompilerVer;	// Ҫʹ�õı������İ汾

	std::vector<COMPILE_INSTANCE> vecConfiguration;	// �������ѡ����

	bool bResult;	// ����Ľ��

	CString szPrjName;	// ���̵�����

	bool bNeedCompile;	// �Ƿ���Ҫ����

	CString szLastCfg;	// �ϴ�ѡ�еı���ѡ��

	//////////////////////////////////////////////////////////////////////////
	CFG_TYPE cfgType;	// ��ǰ�������õ�����

	CString szSrcFile;	// ��ǰ�������õ�����

	CString szDestFile;	// Ҫ�����ļ���Ŀ��λ��

	//////////////////////////////////////////////////////////////////////////
	CString szDelFolder; // Ҫɾ�����ļ��е�·��

}COMPILE_NODE;

//////////////////////////////////////////////////////////////////////////
typedef struct ProjectData{
	CString szPrjName;
	std::vector<COMPILE_NODE> childCompileNode;
	bool bCompileNeed;
} PROJECTDATA;

//////////////////////////////////////////////////////////////////////////
#define KEY_SUBPATH_HISTORY_CFG _T("Software\\CompilerGenius")

//////////////////////////////////////////////////////////////////////////
enum COMPILER_ERROR
{
	COMPILE_NO_ERROR = 0,
	CONFIGFILE_HISTORY_NOTFOUND = 1,
	CONFIGFILE_LOAD_FAIL = 2,


};

#endif