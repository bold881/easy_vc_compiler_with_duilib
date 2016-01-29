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

	CString m_szVs2005VCPath;	// vs2005��vc�ļ���·��

	CString m_szVs2008VCPath;	// vs2005��vc�ļ���·��

	CString m_szVs2010VCPath;	// vs2010��vc�ļ���·��

	CString m_szVs2012VCPath;	// vs2012��vc�ļ���·��

	// ϵͳ�Ƿ�32λϵͳ
	int m_nIsSys32Or64;	// 0δ���ã� 1Ϊ32λϵͳ, 2Ϊ64λϵͳ

private:
	int getHisCfgFilePath();



	// ��ȡvisual studio�İ�װ·��
	void GetVisualStudioInstallDir(
		CString &szVcPath,
		CString szVcVersion);

	// �ж�ϵͳ�Ƿ�32λ��64λϵͳ
	void IsWow64();

	// �õ�ע���ֵ�ӽ���ֵ
	void EnumRegSubkeyValue(
		IN HKEY &hKey,	// ��
		IN CString& szKeyName, // �ӽ�����
		OUT CString& szKeyValue); // �ӽ���ֵ

	// ���������ļ�
	void ParseVcProjFile(
		COMPILE_NODE& pNode,	// ��ǰ���ڴ���Ľڵ�
		CString &szFile);	// �����ļ���ȫ·��

	// ѡ�������ļ�
	bool GetCfgFile();

	// ����ע���ֵ
	bool SetHistoryCfgInReg();

	// ɾ���ļ��м������ļ�
	bool DeleteDirectory(
		const CString &strPath);


public:
	// ����
	void CompileNode(void);

	// ��һ������
	void OpenProjectSolution(CString &szItem);

	// ��¼�û�����ѡ��
	void RecordUserCompileSetting(const int nItem, 
		const int nCfg,
		CString &szCfg);

	void setHisCfgPath(CString& szPath)
	{
		m_szHisCfgFile = szPath;
	};

	// ��������ÿ����Ŀ�������ļ�
	void ParseSpecCfgFile(void);

	// �����������ļ� 
	int ParseConfigFile(void);
private:
	bool CreateCompileBatchFile(
		IN CString &szOriginalBatchFile,	// ԭʼ���������ļ�
		IN CString &szPlatformVer,			// �����ƽ̨
		IN CString &szProjFile,			// ����������ļ�
		IN CString &szCompileCfg,			// �����ѡ������	
		OUT CString &szCreatedBatchFile);	// �����ɵı��������ļ�

	// ����һ���ڵ�
	bool DoCompileOneNode(
		IN CString &szBatchFile,	// �������ļ�
		IN CString &szParameter,	// �������
		OUT CString &szResult); // �������ַ���

	//CompilerThread *m_pCmpThread;
};
