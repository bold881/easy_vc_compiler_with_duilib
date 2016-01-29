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
	CString szInstanceName;			// 编译选项的名称  如：Release_Chutian, Debug
	CString szPlatfromVer;		// 编译的平台选项 如：Win32， x64
	bool isSelected;			// 是否次编译选项被选中
}COMPILE_INSTANCE;

//////////////////////////////////////////////////////////////////////////
typedef struct compile_node
{
	CString sz_PrjFilePath; // 配置文件的路径

	COMPILER_VER nCompilerVer;	// 要使用的编译器的版本

	std::vector<COMPILE_INSTANCE> vecConfiguration;	// 具体编译选项组

	bool bResult;	// 编译的结果

	CString szPrjName;	// 工程的名字

	bool bNeedCompile;	// 是否需要编译

	CString szLastCfg;	// 上次选中的编译选项

	//////////////////////////////////////////////////////////////////////////
	CFG_TYPE cfgType;	// 当前任务配置的类型

	CString szSrcFile;	// 当前任务配置的类型

	CString szDestFile;	// 要复制文件的目标位置

	//////////////////////////////////////////////////////////////////////////
	CString szDelFolder; // 要删除的文件夹的路径

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