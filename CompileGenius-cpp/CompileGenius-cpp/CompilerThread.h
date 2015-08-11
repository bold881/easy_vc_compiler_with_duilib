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
		CString &szBatchFile,	// �������ļ�
		CString &szParameter,	// ����
		CString &szResult);		// ������

	static bool CreateCompileBatchFile(
		IN CString &szOriginalBatchFile,	// ԭʼ���������ļ�
		IN CString &szPlatformVer,			// �����ƽ̨
		IN CString &szProjFile,			// ����������ļ�
		IN CString &szCompileCfg,			// �����ѡ������	
		OUT CString &szCreatedBatchFile);	// �����ɵı��������ļ�

private:
	NODEDATA m_nodeData;

	HANDLE m_hThread;

};
