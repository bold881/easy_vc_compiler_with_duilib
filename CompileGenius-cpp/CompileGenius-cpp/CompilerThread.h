#pragma once
#include "stdafx.h"
#include "gGlobalDefine.h"

typedef struct NodeData{
	std::vector<PROJECTDATA> m_PrjData;

	CString szVs2005Path;

	CString szVs2008Path;

	CString szVs2010Path;

	CString szVs2012Path;

	bool isContinue;
}NODEDATA;


class CompilerThread
{
public:
	CompilerThread(void);
	~CompilerThread(void);
	
	void doit();
	
	void setProjData(std::vector<PROJECTDATA> & prjData);

	void setVsPath(
		CString& szVs2005,
		CString& szVs2008,
		CString& szVs2010,
		CString& szVs2012);

	void stop();

private:
	static void run(void* parg);

	static void CompileNode(NODEDATA* pNodeData);

	static bool DoCompileOneNode(
		CString &szBatchFile,	// 批处理文件
		CString &szParameter,	// 参数
		CString &szResult);		// 编译结果

	static bool CreateCompileBatchFile(
		IN CString &szOriginalBatchFile,	// 原始的批处理文件
		IN CString &szPlatformVer,			// 编译的平台
		IN CString &szProjFile,			// 编译的配置文件
		IN CString &szCompileCfg,			// 编译的选项名字	
		OUT CString &szCreatedBatchFile);	// 新生成的编译配置文件

private:
	NODEDATA m_nodeData;

	HANDLE m_hThread;

};
