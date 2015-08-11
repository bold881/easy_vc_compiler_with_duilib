#include "StdAfx.h"
#include "CompilerThread.h"
#include <string>
#include <fstream>

CompilerThread::CompilerThread(void)
{
	m_nodeData.isContinue = true;
}

CompilerThread::~CompilerThread(void)
{
	WaitForSingleObject(m_hThread, INFINITE);
	CloseHandle(m_hThread);
}

void CompilerThread::doit()
{
	m_hThread = (HANDLE)_beginthread(run, 
									0,
									&m_nodeData);
}

void CompilerThread::run(void* parg)
{
	NODEDATA* pNodeData = (NODEDATA*)parg;
	if(pNodeData==NULL) return;
	CompileNode(pNodeData);
}

void CompilerThread::setProjData(std::vector<PROJECTDATA> & prjData)
{
	m_nodeData.m_PrjData.clear();
	//std::vector<PROJECTDATA>::iterator itPrj = prjData.begin();
	//for (; itPrj!=prjData.end(); itPrj++)
	//{
	//	m_PrjData.push_back(*itPrj);
	//}
	m_nodeData.m_PrjData = prjData;
}

void CompilerThread::CompileNode(NODEDATA* pNodeData)
{
	if(pNodeData->szVs2005Path.IsEmpty() ||
		pNodeData->szVs2008Path.IsEmpty() ||
		pNodeData->szVs2010Path.IsEmpty() ||
		pNodeData->szVs2012Path.IsEmpty()){
			MessageBox(NULL, 
				_T("VC2005、2008、2010、2012的路径没有设置，或者其中一个路径没有设置"),
				_T("打包精灵"), MB_OK);
			return;
	}


	std::vector<PROJECTDATA>::iterator itItem = pNodeData->m_PrjData.begin();
	int nItem = 0;
	for (;itItem!=pNodeData->m_PrjData.end(); itItem++, nItem++)
	{
		if(!itItem->bCompileNeed)
			continue;

		std::vector<COMPILE_NODE>::iterator itChild = itItem->childCompileNode.begin();
		int nCfg = 0;
		for (; itChild!=itItem->childCompileNode.end() &&
			pNodeData->isContinue; 
			itChild++, nCfg++)
		{
			if(!itChild->bNeedCompile)
				continue;

			// 原始批处理文件地址
			CString szBatFilePath;

			if(itChild->nCompilerVer == vs80)
				szBatFilePath = pNodeData->szVs2005Path;
			else if(itChild->nCompilerVer == vs90)
				szBatFilePath = pNodeData->szVs2008Path;
			else if(itChild->nCompilerVer == vs100)
				szBatFilePath = pNodeData->szVs2010Path;
			else if(itChild->nCompilerVer == vs110)
				szBatFilePath = pNodeData->szVs2012Path;

			// 工程配置文件地址
			CString szPrjFilePath = itChild->sz_PrjFilePath;

			//编译选项名称
			CString szSpecCompileName;
			//平台名称
			CString szPlatformVer;

			std::vector<COMPILE_INSTANCE>::iterator itCfg = itChild->vecConfiguration.begin();
			for (;itCfg!=itChild->vecConfiguration.end(); itCfg++)
			{
				if(!itCfg->isSelected)
					continue;
				szSpecCompileName = itCfg->szInstanceName;
				szPlatformVer = itCfg->szPlatfromVer;
				break;
			}
			if(szSpecCompileName.IsEmpty() || 
				szPlatformVer.IsEmpty())
				continue;

			// 创建新的编译文件
			CString szCreatedBatchFile;
			CreateCompileBatchFile(szBatFilePath,
				szPlatformVer,
				szPrjFilePath,
				szSpecCompileName,
				szCreatedBatchFile);

			// 批处理的参数
			CString szParameter;
			if(szPlatformVer.CompareNoCase(_T("win32")) == 0){
				szParameter = _T("x86");
			} else if(szPlatformVer.CompareNoCase(_T("x64")) == 0){
				szParameter = _T("x86_amd64");
			}

			// 编译器输出的结果
			CString szRetOutput;
			DoCompileOneNode(szCreatedBatchFile, 
				szParameter,
				szRetOutput);

			MessageBox(NULL, szRetOutput, NULL, MB_OK);
		}
	}
}


void CompilerThread::setVsPath(CString& szVs2005,
								CString& szVs2008,
								CString& szVs2010,
								CString& szVs2012)
{
	m_nodeData.szVs2005Path = szVs2005;
	m_nodeData.szVs2008Path = szVs2008;
	m_nodeData.szVs2010Path = szVs2010;
	m_nodeData.szVs2012Path = szVs2012;
}

bool CompilerThread::DoCompileOneNode(CString &szBatchFile,	// 批处理文件
									CString &szParameter,	// 参数
									CString &szResult)		// 编译结果
{
	szBatchFile = szBatchFile + _T(" ") + szParameter;
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//创建匿名管道
	if(!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		return false;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	//运行ping
	if(!CreateProcess(NULL, szBatchFile.GetBuffer(szBatchFile.GetLength()),
		NULL,NULL,TRUE,NULL,NULL,NULL,&si, &pi))
	{
		return false;
	}
	CloseHandle(hWrite);


	DWORD bytesRead;
	while(1)
	{
		char buffer[1023] = {0};
		if(ReadFile(hRead, buffer, 1023, &bytesRead, NULL) == NULL)
			break;
		szResult.Append(CString(buffer));
	}
	return true;
}

bool CompilerThread::CreateCompileBatchFile(CString &szOriginalBatchFile,	// 原始的批处理文件
										  CString &szPlatformVer,			// 编译的平台
										  CString &szProjFile,			// 编译的配置文件
										  CString &szCompileCfg,			// 编译选项的名称					
										  CString &szCreatedBatchFile)	// 新生成的编译配置文件
{
	string line;
	ifstream origfile(szOriginalBatchFile);
	szCreatedBatchFile = szOriginalBatchFile;
	szCreatedBatchFile = szCreatedBatchFile.Left(szCreatedBatchFile.ReverseFind(_T('\\'))) + _T("\\vcvarsall-new.bat");
	ofstream outfile(szCreatedBatchFile);

	int nAddLineIndex = 0;
	if (origfile.is_open() && outfile.is_open())
	{
		while ( getline (origfile,line) )
		{
			outfile<<line;
			outfile<<"\n";
			if(szPlatformVer.CompareNoCase(_T("Win32")) == 0)
			{
				// x86
				if(strcmp(line.c_str(), ":x86") == 0)
				{
					// 置标志位
					nAddLineIndex++;
				}
			}
			else if(szPlatformVer.CompareNoCase(_T("x64")) == 0)
			{
				// x86_amd64
				if(strcmp(line.c_str(), ":x86_amd64") == 0)
				{
					nAddLineIndex++;
				}
			}

			if(nAddLineIndex == 3)
			{
				nAddLineIndex = 0;
				string szAddedLine;
				szAddedLine.append("msbuild ");
				szAddedLine.append(CStringA(szProjFile).GetBuffer(szProjFile.GetLength()));
				szAddedLine.append(" /p:configuration=");
				szAddedLine.append(CStringA(szCompileCfg).GetBuffer(szCompileCfg.GetLength()));
				outfile << szAddedLine;
				outfile<<"\n";
			}

			if(nAddLineIndex>0)
				nAddLineIndex++;
		}
		origfile.close();
		outfile.close();
	}
	return true;
}

void CompilerThread::stop()
{
	m_nodeData.isContinue = false;
}