#include "stdafx.h"
#include "CompileModel.h"
#include "Markup.h"
#include <fstream>
#include "Shellapi.h"
#include "Commdlg.h"
#include "Shlwapi.h"
#include "CompilerThread.h"


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

#define VS_05_08_HEADER _T("VisualStudioProject")
#define VS_10_12_HEADER _T("Project")

#define LASTCONFIG _T("lastconfig")

//////////////////////////////////////////////////////////////////////////
// ��ʼ��
bool CompileModel::_init()
{
	// ��ȡ�����ļ�
	m_compileError = getHisCfgFilePath();

	// ���������ļ�
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

	// ��ʼ������
	m_compilerwnd.setProjectData(m_PrjData);
	m_compilerwnd.SetModelPointer((void*)this);
	m_compilerwnd._init();

	// ���������ļ��о���������ļ�
	ParseSpecCfgFile();
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// ��ע����л�ȡ��ʷ�����ļ�
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
// ���������ļ�
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
			xml.IntoElem();
			while (xml.FindElem(_T("p")))
			{	
				COMPILE_NODE cNode;
				cNode.sz_PrjFilePath = xml.GetData().c_str();
				cNode.szLastCfg = xml.GetAttrib(LASTCONFIG).c_str();
				cNode.bResult = false;
				cNode.bNeedCompile = false;
				gNode.childCompileNode.push_back(cNode);
			}
			xml.OutOfElem();
			m_PrjData.push_back(gNode);
		}
	}
	return COMPILE_NO_ERROR;
}


//////////////////////////////////////////////////////////////////////////
// ��ȡvisual studio�İ�װ·��
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
		szSubRegPath = _T("SOFTWARE\\Microsoft\\VisualStudio\\") + szVcVersion; // 32λϵͳ
	else if(m_nIsSys32Or64 == 2)
		szSubRegPath = _T("SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\") + szVcVersion; // 64λϵͳ

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
// �õ�ע���ֵ�ӽ���ֵ
void CompileModel::EnumRegSubkeyValue(IN HKEY &hKey,	// ��
									  IN CString& szKeyName, // �ӽ�����
									  OUT CString& szKeyValue) // �ӽ���ֵ
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
// ��������������ļ�
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
			if(PathFileExists(iNode->sz_PrjFilePath))
				ParseVcProjFile(*iNode, iNode->sz_PrjFilePath);
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

	if(xmlProj.FindElem(VS_05_08_HEADER))// vs2005 vs2008����
	{
		// �������汾
		CString szVersion = xmlProj.GetAttrib(_T("Version")).c_str();
		if(szVersion == _T("8.00"))
			pNode.nCompilerVer = vs80;
		else if(szVersion == _T("9.00"))
			pNode.nCompilerVer = vs90;
		else
			pNode.nCompilerVer = vsunknown;

		// ���빤������
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
					pNode.vecConfiguration.push_back(cpInstance);
				}
			}
		}
	}
	else if(xmlProj.FindElem(VS_10_12_HEADER))
	{
		// vs2010 vs2012����
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
				if(szCompilerVer == _T("v110"))
					pNode.nCompilerVer = vs110;
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
		// ����λ�ù���
	}
}


// ����
void testThread(void* pArg)
{
	MessageBox(NULL, _T("hello"), NULL, MB_OK);
}




//////////////////////////////////////////////////////////////////////////
// ����ڵ�
void CompileModel::CompileNode(void)
{
	_beginthread(testThread, 0, NULL);

	if(m_szVs2005VCPath.IsEmpty() ||
		m_szVs2008VCPath.IsEmpty() ||
		m_szVs2010VCPath.IsEmpty() ||
		m_szVs2012VCPath.IsEmpty()){
			MessageBox(NULL, 
				_T("VC2005��2008��2010��2012��·��û�����ã���������һ��·��û������"),
				_T("�������"), MB_OK);
			return;
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
			
			// ԭʼ�������ļ���ַ
			CString szBatFilePath;

			if(itChild->nCompilerVer == vs80)
				szBatFilePath = m_szVs2005VCPath;
			else if(itChild->nCompilerVer == vs90)
				szBatFilePath = m_szVs2008VCPath;
			else if(itChild->nCompilerVer == vs100)
				szBatFilePath = m_szVs2010VCPath;
			else if(itChild->nCompilerVer == vs110)
				szBatFilePath = m_szVs2012VCPath;

			// ���������ļ���ַ
			CString szPrjFilePath = itChild->sz_PrjFilePath;

			//����ѡ������
			CString szSpecCompileName;
			//ƽ̨����
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

			// �����µı����ļ�
			CString szCreatedBatchFile;
			CreateCompileBatchFile(szBatFilePath,
				szPlatformVer,
				szPrjFilePath,
				szSpecCompileName,
				szCreatedBatchFile);

			// �������Ĳ���
			CString szParameter;
			if(szPlatformVer.CompareNoCase(_T("win32")) == 0){
				szParameter = _T("x86");
			} else if(szPlatformVer.CompareNoCase(_T("x64")) == 0){
				szParameter = _T("x86_amd64");
			}

			// ����������Ľ��
			CString szRetOutput;
			DoCompileOneNode(szCreatedBatchFile, 
				szParameter,
				szRetOutput);

			m_compilerwnd.ShowResult(szRetOutput);
			m_compilerwnd.ShowItemResult(nItem, nCfg, szRetOutput);
		}
	}

	//if(m_pCmpThread!=NULL)
	//{
	//	m_pCmpThread->stop();
	//	delete m_pCmpThread;
	//	m_pCmpThread = NULL;
	//}

	//m_pCmpThread = new CompilerThread;
	//m_pCmpThread->setProjData(m_PrjData);
	//m_pCmpThread->setVsPath(m_szVs2005VCPath,
	//	m_szVs2008VCPath,
	//	m_szVs2010VCPath,
	//	m_szVs2012VCPath);
	//m_pCmpThread->doit();
}


bool CompileModel::DoCompileOneNode(CString &szBatchFile,	// �������ļ�
									CString &szParameter,	// ����
									CString &szResult)		// ������
{
	szBatchFile = szBatchFile + _T(" ") + szParameter;
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//���������ܵ�
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


	//����ping
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


bool CompileModel::CreateCompileBatchFile(CString &szOriginalBatchFile,	// ԭʼ���������ļ�
										  CString &szPlatformVer,			// �����ƽ̨
										  CString &szProjFile,			// ����������ļ�
										  CString &szCompileCfg,			// ����ѡ�������					
										  CString &szCreatedBatchFile)	// �����ɵı��������ļ�
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
					// �ñ�־λ
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
				// vc2012������Ҫ���⴦��
				string szAddedLine;
				if(szOriginalBatchFile.Find(_T("Microsoft Visual Studio 11.0"))!=-1)
				{
					szAddedLine.append("set VisualStudioVersion=11.0\n");
				}

				nAddLineIndex = 0;
				
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
				while (xml.FindElem(_T("p")))
				{	
					if(childIndex == nCfg)
					{
						xml.SetAttrib(LASTCONFIG, szCfg.GetBuffer(szCfg.GetLength()));
						szCfg.ReleaseBuffer();
						break;	
					}
					childIndex++;
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
	openFileName.nMaxFile = MAX_PATH;  //����������ã������õĻ�������ִ��ļ��Ի���  
	openFileName.lpstrFilter = _T("�����ļ�(*.xml)\0*.xml\0�����ļ�(*.*)\0*.*\0\0");
	openFileName.lpstrFile = (LPWSTR)szFileName;
	openFileName.nFilterIndex = 1;  
	openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;  

	if (!::GetOpenFileName(&openFileName))  return false;

	m_szHisCfgFile = CString(openFileName.lpstrFile);

	return true;
}

// ����ע����й�����ʷ���ü�¼��ֵ
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