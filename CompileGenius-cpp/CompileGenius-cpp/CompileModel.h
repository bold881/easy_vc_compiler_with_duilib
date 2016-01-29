#pragma once
#include "gGlobalDefine.h"
#include "CompilerWnd.h"


class CompileModel
{
public:
	CompileModel(void);
	~CompileModel(void);


	CCompilerWnd m_compilerwnd;
public:

	std::vector<PROJECTDATA> m_PrjData;

	CString m_szHisCfgFile;
	
public:
	bool _init(void);

private:
	int m_compileError;

	CString m_szVs2005VCPath;	// vs2005的vc文件夹路径

	CString m_szVs2008VCPath;	// vs2005的vc文件夹路径

	CString m_szVs2010VCPath;	// vs2010的vc文件夹路径

	CString m_szVs2012VCPath;	// vs2012的vc文件夹路径

	// 系统是否32位系统
	int m_nIsSys32Or64;	// 0未设置， 1为32位系统, 2为64位系统

private:
	int getHisCfgFilePath();



	// 获取visual studio的安装路径
	void GetVisualStudioInstallDir(
		CString &szVcPath,
		CString szVcVersion);

	// 判断系统是否32位或64位系统
	void IsWow64();

	// 得到注册表值子健的值
	void EnumRegSubkeyValue(
		IN HKEY &hKey,	// 健
		IN CString& szKeyName, // 子健名称
		OUT CString& szKeyValue); // 子健的值

	// 解析工程文件
	void ParseVcProjFile(
		COMPILE_NODE& pNode,	// 当前正在处理的节点
		CString &szFile);	// 工程文件的全路径

	// 选择配置文件
	bool GetCfgFile();

	// 设置注册表值
	bool SetHistoryCfgInReg();

	// 删除文件夹及其中文件
	bool DeleteDirectory(
		const CString &strPath);


public:
	// 编译
	void CompileNode(void);

	// 打开一个工程
	void OpenProjectSolution(CString &szItem);

	// 记录用户编译选项
	void RecordUserCompileSetting(const int nItem, 
		const int nCfg,
		CString &szCfg);

	void setHisCfgPath(CString& szPath)
	{
		m_szHisCfgFile = szPath;
	};

	// 解析具体每个项目的配置文件
	void ParseSpecCfgFile(void);

	// 解析总配置文件 
	int ParseConfigFile(void);
private:
	bool CreateCompileBatchFile(
		IN CString &szOriginalBatchFile,	// 原始的批处理文件
		IN CString &szPlatformVer,			// 编译的平台
		IN CString &szProjFile,			// 编译的配置文件
		IN CString &szCompileCfg,			// 编译的选项名字	
		OUT CString &szCreatedBatchFile);	// 新生成的编译配置文件

	// 编译一个节点
	bool DoCompileOneNode(
		IN CString &szBatchFile,	// 批处理文件
		IN CString &szParameter,	// 编译参数
		OUT CString &szResult); // 编译结果字符串

	//CompilerThread *m_pCmpThread;
};
