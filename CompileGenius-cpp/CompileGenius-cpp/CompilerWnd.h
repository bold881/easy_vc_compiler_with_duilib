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

	// 配置文件没有的时候，让用户手动选择配置文件
	bool GetCfgFile();

	// 在注册表中记录配置文件路径
	bool SetHistoryCfgInReg();

	// 添加数据到界面
	void AddDataToUI(void);

	void AddNodeToUI(PROJECTDATA& pNode,
		int& nIndex);

	void SetModelPointer(void* pClass);

	// 设置是那个编译选项被选中
	void setCmpCfgSelected(int nItem, int nCfg, CString szCfgName);

	// 附加结果到结果窗
	void ShowResult(CString &szOutcome);

	// 显示具体某一编译选项的结果
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
	
	CListUIEx* m_pProjectList;	// 列表

	DuiLib::CRichEditUI* m_pCtrlBuildString; // 编译的结果

	CString m_szHistoryCfg;	// 自定义的项目配置文件

	std::vector<PROJECTDATA>* m_pPrjData;

	DuiLib::CEditUI* m_pEditCfgPath;	// Edit用以显示历史配置文件

	DuiLib::CButtonUI* m_pBtnFindCfgFile;	// 查找历史配置文件
	
};