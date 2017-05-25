#include "stdafx.h"
#include "CompileModel.h"
#include "Markup.h"
#include <fstream>
#include "Shellapi.h"
#include "Commdlg.h"
#include "Shlwapi.h"

int nTotalItem = 0;

CompileModel::CompileModel(void):
m_nIsSys32Or64(0)
//m_pCmpThread(0)
{
}

CompileModel::~CompileModel(void)
{
}

#define CONFIGURATIONS			_T("Configurations")
#define CONFIGURATION			_T("Configuration")
#define PROPERTYGROUP			_T("PropertyGroup")
#define	CONFIGURATION_TOOL		_T("Tool")

#define VCCOMPILERTOOL		_T("VCCLCompilerTool")
#define VCLINKERTOOL		_T("VCLinkerTool")
#define PREPROCESSOR_DEFINITIONS		_T("PreprocessorDefinitions")
#define OUTPUTFILE		_T("OutputFile")

#define VS_05_08_HEADER _T("VisualStudioProject")
#define VS_10_12_15HEADER _T("Project")

#define LASTCONFIG _T("lastconfig")
#define DEST	_T("dest")
#define SRC		_T("src")

//////////////////////////////////////////////////////////////////////////
// 初始化
bool CompileModel::_init()
{
	// 获取配置文件
	m_compileError = getHisCfgFilePath();

	// 解析配置文件
	if(CONFIGFILE_LOAD_FAIL == ParseConfigFile())
	{
		GetCfgFile();
		SetHistoryCfgInReg();
		ParseConfigFile();
	}

	GetVisualStudioInstallDir(m_szVs2005VCPath, _T("8.0"));

	GetVisualStudioInstallDir(m_szVs2008VCPath, _T("9.0"));

	GetVisualStudioInstallDir(m_szVs2010VCPath, _T("10.0"));

	GetVisualStudioInstallDir(m_szVs2012VCPath, _T("11.0"));

	GetVisualStudioInstallDir(m_szVs2015VcPath, _T("14.0"));

	// 初始化界面
	m_compilerwnd.setProjectData(m_PrjData);
	m_compilerwnd.setCfgHistFile(m_szHisCfgFile);
	m_compilerwnd.SetModelPointer((void*)this);
	m_compilerwnd._init();

	// 解析配置文件中具体的配置文件
	ParseSpecCfgFile();
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 在注册表中获取历史配置文件
int CompileModel::getHisCfgFilePath()
{
	if(!m_szHisCfgFile.IsEmpty())
		return COMPILE_NO_ERROR;

	HKEY hkey_his_cfg;
	DWORD dRet;

	dRet = RegOpenKey(HKEY_CURRENT_USER,
		KEY_SUBPATH_HISTORY_CFG,
		&hkey_his_cfg);

	if(dRet == ERROR_SUCCESS)
	{
		TCHAR bufPath[MAX_PATH] = {0};
		LONG nLength = MAX_PATH;
		dRet = RegQueryValue(hkey_his_cfg,
			NULL,
			bufPath,
			&nLength);

		if(dRet == ERROR_SUCCESS)
		{
			m_szHisCfgFile = CString(bufPath);
		}
	}
	else
	{
		GetCfgFile();
		SetHistoryCfgInReg();
	}

	RegCloseKey(hkey_his_cfg);

	return COMPILE_NO_ERROR;
}

//////////////////////////////////////////////////////////////////////////
// 解析配置文件
int CompileModel::ParseConfigFile()
{
	if(m_szHisCfgFile.IsEmpty())
		return CONFIGFILE_HISTORY_NOTFOUND;
	


	::CMarkup xml;
	if(!xml.Load(m_szHisCfgFile.GetBuffer(m_szHisCfgFile.GetLength())))
	{
		m_szHisCfgFile.ReleaseBuffer();
		m_szHisCfgFile.Empty();
		return CONFIGFILE_LOAD_FAIL;
	}
	m_szHisCfgFile.ReleaseBuffer();

	m_PrjData.clear();

	if(xml.FindElem())
	{
		xml.IntoElem();
		while (xml.FindElem(_T("g")))
		{
			PROJECTDATA gNode;
			gNode.szPrjName = xml.GetAttrib(_T("ProjectName")).c_str();
			
			// daifenga add default config 
			gNode.szDefaultCompileName = xml.GetAttrib(_T("DefaultCfg")).c_str();

			xml.IntoElem();
			while (xml.FindElem())
			{	
				if(xml.GetTagName() == _T("p")){
					COMPILE_NODE cNode;
					cNode.sz_PrjFilePath = xml.GetData().c_str();
					cNode.szLastCfg = xml.GetAttrib(LASTCONFIG).c_str();
					cNode.bResult = false;
					cNode.bNeedCompile = false;
					cNode.cfgType = vcproject;
					cNode.szDefaultCompileName = gNode.szDefaultCompileName;
					gNode.childCompileNode.push_back(cNode); 
				} else if( xml.GetTagName() == _T("f")) {
					COMPILE_NODE cNode;
					cNode.szSrcFile = xml.GetAttrib(SRC).c_str();
					cNode.szDestFile = xml.GetAttrib(DEST).c_str();
					cNode.cfgType = copyfile;
					cNode.bNeedCompile = false;
					gNode.childCompileNode.push_back(cNode);
				} else if(xml.GetTagName() == _T("d")) {
					COMPILE_NODE cNode;
					cNode.szDelFolder = xml.GetData().c_str();
					cNode.bNeedCompile = false;
					cNode.cfgType = delfolder;
					gNode.childCompileNode.push_back(cNode);
				}
			}
			xml.OutOfElem();
			m_PrjData.push_back(gNode);
		}
	}
	return COMPILE_NO_ERROR;
}


//////////////////////////////////////////////////////////////////////////
// 获取visual studio的安装路径
void CompileModel::GetVisualStudioInstallDir(CString &szVcPath,
											 CString szVcVersion)
{
	if(!szVcPath.IsEmpty())
		return;
	if(m_nIsSys32Or64 == 0)
	{
		IsWow64();
	}

	HKEY hKey;
	DWORD dRet;
	CString szSubRegPath;

	if(m_nIsSys32Or64 == 1)
		szSubRegPath = _T("SOFTWARE\\Microsoft\\VisualStudio\\") + szVcVersion; // 32位系统
	else if(m_nIsSys32Or64 == 2)
		szSubRegPath = _T("SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\") + szVcVersion; // 64位系统

	dRet = RegOpenKey(HKEY_LOCAL_MACHINE,
		szSubRegPath,
		&hKey);
	TCHAR buffer[MAX_PATH] = {0};
	long nLength = MAX_PATH;
	if(dRet == ERROR_SUCCESS)
	{
		CString szPath;
		CString szInstallDir = _T("InstallDir");
		EnumRegSubkeyValue(hKey, szInstallDir, szPath);
		szVcPath = szPath;
	}

	RegCloseKey(hKey);
}


typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

void CompileModel::IsWow64()
{
	if(m_nIsSys32Or64 != 0)
		return;

	BOOL bIsWow64 = FALSE;

	//IsWow64Process is not available on all supported versions of Windows.
	//Use GetModuleHandle to get a handle to the DLL that contains the function
	//and GetProcAddress to get a pointer to the function if available.

	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
		GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

	if(NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
		{
			//handle error
		}
	}
	if(bIsWow64)
		m_nIsSys32Or64 = 2;
	else 
		m_nIsSys32Or64 = 1; 
}


#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

//////////////////////////////////////////////////////////////////////////
// 得到注册表值子健的值
void CompileModel::EnumRegSubkeyValue(IN HKEY &hKey,	// 健
									  IN CString& szKeyName, // 子健名称
									  OUT CString& szKeyValue) // 子健的值
{
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys=0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i, retCode; 

	TCHAR  achValue[MAX_KEY_LENGTH]; 
	DWORD cchValue = MAX_KEY_LENGTH;

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

	// Enumerate the key values. 
	if (cValues) 
	{
		TCHAR databuf[MAX_PATH];
		for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) 
		{ 
			DWORD dwLength = MAX_PATH;
			cchValue = MAX_VALUE_NAME; 
			achValue[0] = '\0';
			databuf[0] = '\0';
			retCode = RegEnumValue(hKey, i,
				achValue, 
				&cchValue, 
				NULL,
				NULL,
				(BYTE*)databuf,
				(LPDWORD)&dwLength);

			if (retCode == ERROR_SUCCESS ) 
			{
				if(wcscmp(achValue, _T("InstallDir")) == 0)
				{
					szKeyValue = CString(databuf);
					szKeyValue += _T("..\\..\\VC\\vcvarsall.bat");
					return;
				}
			} 
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// 解析具体的配置文件
void CompileModel::ParseSpecCfgFile()
{
	if(m_PrjData.size() <= 0) return;

	std::vector<PROJECTDATA>::iterator itProj = m_PrjData.begin();
	for (; itProj != m_PrjData.end(); itProj++)
	{
		itProj->bCompileNeed = false;
		std::vector<COMPILE_NODE>::iterator iNode = itProj->childCompileNode.begin();
		for (; iNode != itProj->childCompileNode.end(); iNode++)
		{
			if(iNode->cfgType == vcproject)
			{
				if(PathFileExists(iNode->sz_PrjFilePath)) {
					ParseVcProjFile(*iNode, iNode->sz_PrjFilePath);

					SetDefaultConfigProfile(*iNode);
				}
			}
		}
	}
}

void CompileModel::ParseVcProjFile(COMPILE_NODE& pNode, CString &szFile)
{
	if(szFile.IsEmpty()) return;

	::CMarkup xmlProj;
	if(!xmlProj.Load(szFile.GetBuffer(szFile.GetLength())))
	{	
		szFile.ReleaseBuffer();
		return;
	}
	szFile.ReleaseBuffer();

	if(xmlProj.FindElem(VS_05_08_HEADER))// vs2005 vs2008工程
	{
		// 编译器版本
		CString szVersion = xmlProj.GetAttrib(_T("Version")).c_str();
		if(szVersion == _T("8.00"))
			pNode.nCompilerVer = vs80;
		else if(szVersion == _T("9.00"))
			pNode.nCompilerVer = vs90;
		else
			pNode.nCompilerVer = vsunknown;

		// 编译工程名称
		pNode.szPrjName = xmlProj.GetAttrib(_T("Name")).c_str();

		xmlProj.IntoElem();

		if(xmlProj.FindElem(CONFIGURATIONS))
		{
			xmlProj.IntoElem();
			while (xmlProj.FindElem(CONFIGURATION))
			{
				CString szTemp = xmlProj.GetAttrib(_T("Name")).c_str();
				if(!szTemp.IsEmpty())
				{
					COMPILE_INSTANCE cpInstance;
					int nPos = szTemp.Find(_T("|"));
					if(nPos != -1)
					{
						cpInstance.szInstanceName = szTemp.Left(nPos);
						cpInstance.szPlatfromVer = szTemp.Mid(nPos+1);
					}
					else
					{
						cpInstance.szInstanceName = szTemp;
						cpInstance.szPlatfromVer = _T("Win32");
					}
					if((cpInstance.szInstanceName+cpInstance.szPlatfromVer)
						== pNode.szLastCfg)
						cpInstance.isSelected = true;
					else
						cpInstance.isSelected = false;
					

					xmlProj.IntoElem();
					while(xmlProj.FindElem(CONFIGURATION_TOOL))
					{
						// Get PreprocessorDefinitions from VCCLCompilerTool
						CString szToolTemp = xmlProj.GetAttrib(_T("Name")).c_str();
						if(szToolTemp.IsEmpty())
						{
							continue;
						}

						if(szToolTemp == VCCOMPILERTOOL) {
							cpInstance.szPreProcessDefine = xmlProj.GetAttrib(PREPROCESSOR_DEFINITIONS).c_str();
						}
						// Get OutputFile from VCLinkerTool
						if(szToolTemp == VCLINKERTOOL) {
							cpInstance.szOutputPath = xmlProj.GetAttrib(OUTPUTFILE).c_str();
						}
					}
					xmlProj.OutOfElem();

					pNode.vecConfiguration.push_back(cpInstance);
				}
			}
		}
	}
	else if(xmlProj.FindElem(VS_10_12_15HEADER))
	{
		// vs2010 vs2012工程
		pNode.nCompilerVer = vs100;
		xmlProj.IntoElem();
		while (xmlProj.FindElem(PROPERTYGROUP))
		{
			if(xmlProj.FindChildElem(_T("ProjectName")))
			{
				pNode.szPrjName = xmlProj.GetChildData().c_str();
			}
			else if(xmlProj.FindChildElem(_T("PlatformToolset")))
			{
				CString szCompilerVer = xmlProj.GetChildData().c_str();
				if(szCompilerVer == _T("v110")) {
					pNode.nCompilerVer = vs110;
				} else if(szCompilerVer == _T("v140")) {
					pNode.nCompilerVer = vs140;
				}
			}
		}
		xmlProj.ResetMainPos();
		while (xmlProj.FindElem(_T("ItemDefinitionGroup")))
		{
			CString szCondition = xmlProj.GetAttrib(_T("Condition")).c_str();
			int nPos = szCondition.Find(_T("'$(Configuration)|$(Platform)'=='"));
			szCondition = szCondition.Mid(CString(_T("'$(Configuration)|$(Platform)'=='")).GetLength() );


			COMPILE_INSTANCE cpInstance;
			nPos = szCondition.Find(_T("|"));
			if(nPos != -1)
			{
				cpInstance.szInstanceName = szCondition.Left(nPos);
				szCondition = szCondition.Mid(nPos + 1);
				cpInstance.szPlatfromVer = szCondition.Left(szCondition.GetLength()-1); 
			}
			else
			{
				cpInstance.szInstanceName = szCondition;
				cpInstance.szPlatfromVer = _T("Win32");
			}

			// PreProcessorDefinitions
			if(xmlProj.FindChildElem(_T("ClCompile")))
			{
				xmlProj.IntoElem();
				if(xmlProj.FindChildElem(_T("PreprocessorDefinitions"))) {
					cpInstance.szPreProcessDefine = xmlProj.GetChildData().c_str();
				}
				xmlProj.OutOfElem();
			}
			
			// OutputFile
			if(xmlProj.FindChildElem(_T("Link")))
			{
				xmlProj.IntoElem();
				if(xmlProj.FindChildElem(_T("OutputFile"))) {
					cpInstance.szOutputPath = xmlProj.GetChildData().c_str();
				}
				xmlProj.OutOfElem();
			}

			if((cpInstance.szInstanceName+cpInstance.szPlatfromVer)
				== pNode.szLastCfg)
				cpInstance.isSelected = true;
			else
				cpInstance.isSelected = false;
			pNode.vecConfiguration.push_back(cpInstance);
		}
	}
	else
	{
		// 其他位置工程
	}
}


//////////////////////////////////////////////////////////////////////////
// 编译节点
void CompileModel::CompileNode(void)
{
	nTotalItem = 0;
	//_beginthread(testThread, 0, this);

	if(m_szVs2005VCPath.IsEmpty() ||
		m_szVs2008VCPath.IsEmpty() ||
		m_szVs2010VCPath.IsEmpty() ||
		m_szVs2012VCPath.IsEmpty() ||
		m_szVs2015VcPath.IsEmpty()){
			MessageBox(NULL, 
				_T("VS2005、2008、2010、2012、2015的路径没有设置，或者其中一个路径没有设置"),
				_T("打包精灵"), MB_OK);
			//return;
	}

	
	std::vector<PROJECTDATA>::iterator itItem = m_PrjData.begin();
	int nItem = 0;
	for (;itItem!=m_PrjData.end(); itItem++, nItem++)
	{
		if(!itItem->bCompileNeed)
			continue;
		
		std::vector<COMPILE_NODE>::iterator itChild = itItem->childCompileNode.begin();
		int nCfg = 0;
		for (; itChild!=itItem->childCompileNode.end(); itChild++, nCfg++)
		{
			if(!itChild->bNeedCompile)
				continue;
			if(itChild->cfgType == vcproject) {
				
				// 原始批处理文件地址
				CString szBatFilePath;

				if(itChild->nCompilerVer == vs80)
					szBatFilePath = m_szVs2005VCPath;
				else if(itChild->nCompilerVer == vs90)
					szBatFilePath = m_szVs2008VCPath;
				else if(itChild->nCompilerVer == vs100)
					szBatFilePath = m_szVs2010VCPath;
				else if(itChild->nCompilerVer == vs110)
					szBatFilePath = m_szVs2012VCPath;
				else if(itChild->nCompilerVer == vs140)
					szBatFilePath = m_szVs2015VcPath;

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
#if defined(_IA64_MODE)
					szParameter = _T("ia64");
#elif defined(_AMD64_MODE)
					szParameter = _T("amd64");
#else
					szParameter = _T("x86_amd64");
#endif
				}

				// 编译器输出的结果
				CString szRetOutput;
				DoCompileOneNode(szCreatedBatchFile, 
					szParameter,
					szRetOutput);

				//m_compilerwnd.ShowResult(szRetOutput);
				m_compilerwnd.ShowItemResult(nItem, nCfg, szRetOutput);
				++nTotalItem;
			} else if( itChild->cfgType == copyfile) {
				// 复制文档
				CString szResult;
				if(CopyFile(itChild->szSrcFile, itChild->szDestFile, FALSE)){
					szResult = _T("生成成功。从") + itChild->szSrcFile + _T(" 到 ") + itChild->szDestFile;
				}
				m_compilerwnd.ShowItemResult(nItem, nCfg, szResult);
				++nTotalItem;
			} else if(itChild->cfgType == delfolder) {
				// 删除文件夹及其子类
				CString szResult;
				if(this->DeleteDirectory(itChild->szDelFolder)){
					szResult = _T("生成成功。") + itChild->szDelFolder;
				}

				m_compilerwnd.ShowItemResult(nItem, nCfg, szResult);
				++nTotalItem;
			}
		}
	}

}


bool CompileModel::DoCompileOneNode(CString &szBatchFile,	// 批处理文件
									CString &szParameter,	// 参数
									CString &szResult)		// 编译结果
{
	szBatchFile = _T("\"") + szBatchFile + _T("\"");
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
	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;


	//运行ping
	if(!CreateProcess(NULL, szBatchFile.GetBuffer(szBatchFile.GetLength()),
		NULL,NULL,TRUE,NULL,NULL,NULL,&si, &pi))
	{
		szBatchFile.ReleaseBuffer();
		return false;
	}
	szBatchFile.ReleaseBuffer();
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


bool CompileModel::CreateCompileBatchFile(CString &szOriginalBatchFile,	// 原始的批处理文件
										  CString &szPlatformVer,			// 编译的平台
										  CString &szProjFile,			// 编译的配置文件
										  CString &szCompileCfg,			// 编译选项的名称					
										  CString &szCreatedBatchFile)	// 新生成的编译配置文件
{
	CString szTempCompileCfg = _T("\"") + szCompileCfg + _T("\"");
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
#if defined(_IA64_MODE)
				// ia64
				if(strcmp(line.c_str(), ":ia64") == 0)
				{
					nAddLineIndex++;
				}
#elif defined(_AMD64_MODE)
				if(strcmp(line.c_str(), ":amd64") == 0)
				{
					nAddLineIndex++;
				}
#else
				// x86_amd64
				if(strcmp(line.c_str(), ":x86_amd64") == 0)
				{
					nAddLineIndex++;
				}
#endif

			}

			if(nAddLineIndex == 3)
			{
				// vc2012及以上要特殊处理
				string szAddedLine;
				if(szOriginalBatchFile.Find(_T("Microsoft Visual Studio 11.0"))!=-1)
				{
					szAddedLine.append("set VisualStudioVersion=11.0\n");
				}

				nAddLineIndex = 0;
				
				szAddedLine.append("msbuild ");
				szAddedLine.append(CStringA(szProjFile).GetBuffer(szProjFile.GetLength()));
				szAddedLine.append(" /p:configuration=");
				szAddedLine.append(CStringA(szTempCompileCfg).GetBuffer(szTempCompileCfg.GetLength()));
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

void CompileModel::OpenProjectSolution(CString &szItem)
{
	int nItem = _wtoi(szItem.Left(szItem.Find(_T("_"))));
	int nCfg = _wtoi(szItem.Mid(szItem.Find(_T("_"))+1));

	ShellExecute(NULL, 
				_T("open"),
				m_PrjData.at(nItem).childCompileNode.at(nCfg).sz_PrjFilePath, 
				NULL,NULL, SW_SHOW);
}

void CompileModel::RecordUserCompileSetting(int nItem, 
											int nCfg,
											CString &szCfg)
{
	if(m_szHisCfgFile.IsEmpty())
		return;
	::CMarkup xml;
	if(!xml.Load(m_szHisCfgFile.GetBuffer(m_szHisCfgFile.GetLength())))
	{
		m_szHisCfgFile.ReleaseBuffer();
		m_szHisCfgFile.Empty();
		return;
	}
	m_szHisCfgFile.ReleaseBuffer();

	if(xml.FindElem())
	{
		xml.IntoElem();
		int itemIndex = 0;
		while (xml.FindElem(_T("g")))
		{
			if(itemIndex == nItem)
			{
				xml.IntoElem();
				int childIndex = 0;
				while (xml.FindElem())
				{	
					if(xml.GetTagName() == _T("p") || 
						xml.GetTagName() == _T("f") || 
						xml.GetTagName() == _T("d"))
					{						
						if(childIndex == nCfg)
						{
							xml.SetAttrib(LASTCONFIG, szCfg.GetBuffer(szCfg.GetLength()));
							szCfg.ReleaseBuffer();
							break;	
						}
						childIndex++;
					}
				}
				xml.OutOfElem();
				break;
			}
			itemIndex++;
		}
		xml.Save(m_szHisCfgFile.GetBuffer(m_szHisCfgFile.GetLength()));
		m_szHisCfgFile.ReleaseBuffer();
	}
}

bool CompileModel::GetCfgFile()
{
	TCHAR szFileName[MAX_PATH]={0};  
	OPENFILENAME openFileName = {0};  
	openFileName.lStructSize = sizeof(OPENFILENAME);  
	openFileName.nMaxFile = MAX_PATH;  //这个必须设置，不设置的话不会出现打开文件对话框  
	openFileName.lpstrFilter = _T("配置文件(*.xml)\0*.xml\0所有文件(*.*)\0*.*\0\0");
	openFileName.lpstrFile = (LPWSTR)szFileName;
	openFileName.nFilterIndex = 1;  
	openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;  

	if (!::GetOpenFileName(&openFileName))  return false;

	m_szHisCfgFile = CString(openFileName.lpstrFile);

	return true;
}

// 设置注册表中关于历史配置记录的值
bool CompileModel::SetHistoryCfgInReg()
{
	if(m_szHisCfgFile.IsEmpty()) return false;

	HKEY hkey_his_cfg;
	DWORD dRet;

	dRet = RegCreateKey(HKEY_CURRENT_USER,
		KEY_SUBPATH_HISTORY_CFG,
		&hkey_his_cfg);
	if(dRet != ERROR_SUCCESS)
		return false;

	dRet = RegSetValue(hkey_his_cfg,
		NULL,
		REG_SZ,
		m_szHisCfgFile,
		m_szHisCfgFile.GetLength());

	RegCloseKey(hkey_his_cfg);

	if(dRet == ERROR_SUCCESS)
		return true;
	else
		return false;
}

bool CompileModel::DeleteDirectory(const CString &strPath)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;

	if (strPath.GetLength() > (MAX_PATH - 3)) {
		return false;
	}
	CString tempFolder = strPath + _T("\\*");

	hFind = FindFirstFile(tempFolder, &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		return false;
	} 

	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			CString szTmpFile = strPath + _T("\\") + ffd.cFileName;
			DeleteFile(szTmpFile);
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
	return true;
}


void CompileModel::SetDefaultConfigProfile(COMPILE_NODE& pNode)
{
	if(pNode.szDefaultCompileName.IsEmpty()) {
		return;
	}

	std::vector<COMPILE_INSTANCE>::iterator it = pNode.vecConfiguration.begin();
	int selectedIndex = -1;
	for (int i=0; it!=pNode.vecConfiguration.end(); it++, i++)
	{
		// 记录lastconfig的选择
		if(it->isSelected) {
			// lastconfig如果满足defaultconfig则直接返回
			if(it->szInstanceName.Find(pNode.szDefaultCompileName) !=-1)
				return;
			selectedIndex = i;
			it->isSelected = false;
			break;
		}
		it->isSelected = false;
	}

	// 比对defaultconfig,若满足defaultconfig，且不为debug，则选中并返回，
	// 必须要避免一种情况就是Win32和x64同时存在的情况
	for (it = pNode.vecConfiguration.begin(); it!=pNode.vecConfiguration.end(); it++)
	{
		if(it->szInstanceName.Find(pNode.szDefaultCompileName) != -1 && 
			it->szInstanceName.Find(_T("Debug")) == -1 && 
			it->szInstanceName.Find(_T("debug")) == -1)
		{
			it->isSelected = true;
			return;
		}
	}

	// 恢复lastconfig
	if(selectedIndex == -1)
		return;
	pNode.vecConfiguration.at(selectedIndex).isSelected = true;
}


void CompileModel::OutputReplace(CString &orginValue, 
								 CString &newValue) 
{
	std::vector<PROJECTDATA>::iterator itProj = m_PrjData.begin();
	for (; itProj!=m_PrjData.end(); itProj++)
	{
		std::vector<COMPILE_NODE>::iterator childIt = itProj->childCompileNode.begin();
		for (int childIndex=0; childIt!=itProj->childCompileNode.end(); childIt++, childIndex++)
		{
			VcProjFileOutputReplace(
				orginValue,
				newValue,
				childIt->sz_PrjFilePath);
		}
	}
}


void CompileModel::VcProjFileOutputReplace(CString &originValue,
										   CString &newValue,
										   CString szFile)
{
	if(szFile.IsEmpty()) return;

	bool outputSet = false;
	::CMarkup xmlProj;
	if(!xmlProj.Load(szFile.GetBuffer(szFile.GetLength())))
	{	
		szFile.ReleaseBuffer();
		return;
	}
	szFile.ReleaseBuffer();

	if(xmlProj.FindElem(VS_05_08_HEADER))// vs2005 vs2008工程
	{
		xmlProj.IntoElem();

		if(xmlProj.FindElem(CONFIGURATIONS))
		{
			xmlProj.IntoElem();
			while (xmlProj.FindElem(CONFIGURATION))
			{
				CString szTemp = xmlProj.GetAttrib(_T("Name")).c_str();
				if(!szTemp.IsEmpty())
				{
					xmlProj.IntoElem();
					while(xmlProj.FindElem(CONFIGURATION_TOOL))
					{
						// Get PreprocessorDefinitions from VCCLCompilerTool
						CString szToolTemp = xmlProj.GetAttrib(_T("Name")).c_str();
						if(szToolTemp.IsEmpty())
						{
							continue;
						}

						//if(szToolTemp == VCCOMPILERTOOL) {
						//	cpInstance.szPreProcessDefine = xmlProj.GetAttrib(PREPROCESSOR_DEFINITIONS).c_str();
						//}

						// Get OutputFile from VCLinkerTool
						if(szToolTemp == VCLINKERTOOL) {
							CString szOutputPath = xmlProj.GetAttrib(OUTPUTFILE).c_str();
							if(szOutputPath.Replace(originValue, newValue) > 0)
							{
								xmlProj.SetAttrib(OUTPUTFILE, szOutputPath.GetBuffer(szOutputPath.GetLength()));
								szOutputPath.ReleaseBuffer();
								outputSet = true;
							}
						}
					}
					xmlProj.OutOfElem();
				}
			}
		}
	}
	else if(xmlProj.FindElem(VS_10_12_15HEADER))
	{
		// vs2010 vs2012工程
		xmlProj.IntoElem();
		xmlProj.ResetMainPos();
		while (xmlProj.FindElem(_T("ItemDefinitionGroup")))
		{
			// PreProcessorDefinitions
			//if(xmlProj.FindChildElem(_T("ClCompile")))
			//{
			//	xmlProj.IntoElem();
			//	if(xmlProj.FindChildElem(_T("PreprocessorDefinitions"))) {
			//		cpInstance.szPreProcessDefine = xmlProj.GetChildData().c_str();
			//	}
			//	xmlProj.OutOfElem();
			//}

			// OutputFile
			if(xmlProj.FindChildElem(_T("Link")))
			{
				xmlProj.IntoElem();
				if(xmlProj.FindChildElem(_T("OutputFile"))) {
					CString szOutputPath = xmlProj.GetChildData().c_str();
					if(szOutputPath.Replace(originValue, newValue) > 0)
					{
						xmlProj.SetChildData(szOutputPath.GetBuffer(szOutputPath.GetLength()));
						szOutputPath.ReleaseBuffer();
						outputSet = true;
					}
				}
				xmlProj.OutOfElem();
			}
		}
	}
	else
	{
		// 其他位置工程
	}

	// save vc proj file
	if(outputSet) {
		xmlProj.Save(szFile.GetBuffer(szFile.GetLength()));
		szFile.ReleaseBuffer();
	}
}