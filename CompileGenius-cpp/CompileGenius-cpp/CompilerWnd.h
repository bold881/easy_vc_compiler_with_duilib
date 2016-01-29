#pragma once
#include "ControlEx.h"
#include "ListEx.h"
#include "gGlobalDefine.h"


class CCompilerWnd : public DuiLib::CWindowWnd, 
	public DuiLib::INotifyUI
{
public:
	CCompilerWnd();
	LPCTSTR GetWindowClassName() const { return _T("UIMainFrame"); };
	UINT GetClassStyle() const { return CS_DBLCLKS; };
	void OnFinalMessage(HWND /*hWnd*/) { delete this; };

	void _init();

	void setProjectData(std::vector<PROJECTDATA>& prjData)
	{
		m_pPrjData = &prjData;
	};

	inline void setCfgHistFile(CString& szFile){
		m_szHistoryCfg = szFile;
	};

	void OnPrepare();

	// �����ļ�û�е�ʱ�����û��ֶ�ѡ�������ļ�
	bool GetCfgFile();

	// ��ע����м�¼�����ļ�·��
	bool SetHistoryCfgInReg();

	// ������ݵ�����
	void AddDataToUI(void);

	void AddNodeToUI(PROJECTDATA& pNode,
		int& nIndex);

	void SetModelPointer(void* pClass);

	// �������Ǹ�����ѡ�ѡ��
	void setCmpCfgSelected(int nItem, int nCfg, CString szCfgName);

	// ���ӽ���������
	void ShowResult(CString &szOutcome);

	// ��ʾ����ĳһ����ѡ��Ľ��
	void ShowItemResult(int nItem, 
		int nCfg,
		CString &szOutcome);

	void Notify(DuiLib::TNotifyUI& msg);

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnNcDBClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);



public:
	DuiLib::CPaintManagerUI m_pm;

private:
	DuiLib::CButtonUI* m_pCloseBtn;
	DuiLib::CButtonUI* m_pMaxBtn;
	DuiLib::CButtonUI* m_pRestoreBtn;
	DuiLib::CButtonUI* m_pMinBtn;

	DuiLib::CButtonUI* m_pGoCompile;
	DuiLib::CButtonUI* m_pCancelBtn;
	
	CListUIEx* m_pProjectList;	// �б�

	DuiLib::CRichEditUI* m_pCtrlBuildString; // ����Ľ��

	CString m_szHistoryCfg;	// �Զ������Ŀ�����ļ�

	std::vector<PROJECTDATA>* m_pPrjData;

	DuiLib::CEditUI* m_pEditCfgPath;	// Edit������ʾ��ʷ�����ļ�

	DuiLib::CButtonUI* m_pBtnFindCfgFile;	// ������ʷ�����ļ�
	
};