#include "stdafx.h"
#include "CompilerWnd.h"
#include "Commdlg.h"
#include "Markup.h"

#include "CompileModel.h"

#define PROJECTFILEVERSION		_T("_ProjectFileVersion")

CCompilerWnd::CCompilerWnd():
m_pPrjData(NULL)
{

}


CompileModel* pCmpModel = NULL;

void CCompilerWnd::Notify(DuiLib::TNotifyUI& msg)
{
	if( msg.sType == _T("windowinit") ) 
		OnPrepare();
	else if( msg.sType == _T("click") ) 
	{
		if( msg.pSender == m_pCloseBtn )
		{
			PostQuitMessage(0);
			return; 
		}
		else if( msg.pSender == m_pMinBtn )
		{ 
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); 
			return; 
		}
		else if( msg.pSender == m_pMaxBtn )
		{ 
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); 
			return; 
		}
		else if( msg.pSender == m_pRestoreBtn ) 
		{ 
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
			return;
		}
		else if(msg.pSender == m_pGoCompile)
		{
			// 清除编译结果窗口
			if(m_pCtrlBuildString!=NULL)
				m_pCtrlBuildString->SetText(_T(""));
			// 屏蔽重新加载配置文件窗口
			m_pBtnFindCfgFile->SetEnabled(false);

			if(pCmpModel != NULL)
				pCmpModel->CompileNode();

			m_pBtnFindCfgFile->SetEnabled(true);
			return;
		}
		else if(msg.pSender->GetName().Find(_T("_projectname")) != -1)
		{
			CString szName = msg.pSender->GetName();
			szName.Replace(_T("_projectname"), _T(""));
			if(pCmpModel!=NULL)
				pCmpModel->OpenProjectSolution(szName);
			return;
		}
		else if(msg.pSender->GetName().Find(_T("_result")) != -1)
		{
			m_pCtrlBuildString->SetText(msg.pSender->GetUserData());
			return;
		} else if(msg.pSender == m_pBtnFindCfgFile) {
			GetCfgFile();
			SetHistoryCfgInReg();
			pCmpModel->setHisCfgPath(m_szHistoryCfg);
			m_pEditCfgPath->SetText(m_szHistoryCfg);
			pCmpModel->ParseConfigFile();
			pCmpModel->ParseSpecCfgFile();
			AddDataToUI();
		}
	}
	else if(msg.sType==_T("selectchanged"))
	{
		CString name = msg.pSender->GetName();
		if(name.Find(_T("_check")) != -1)
		{
			// 检测鼠标位置和空间位置，判断是否点击选择的。
			RECT ctrRect = msg.pSender->GetPos();
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(m_hWnd, &pt);
			if(pt.x<ctrRect.left||pt.x>ctrRect.right
				||pt.y<ctrRect.top||pt.y>ctrRect.bottom)
				return;

			name.Replace(_T("_check"), _T(""));
			int nIndex = _wtoi(name);
			DuiLib::COptionUI* pCurOp = static_cast<DuiLib::COptionUI*>(msg.pSender);
			bool needCompile = pCurOp->IsSelected();
			int childIndex = 0;
			std::vector<COMPILE_NODE>::iterator itchild = 
				m_pPrjData->at(nIndex).childCompileNode.begin();
			CString szChildName;
			for (; itchild!=m_pPrjData->at(nIndex).childCompileNode.end(); 
				itchild++, childIndex++)
			{
				itchild->bNeedCompile = needCompile;
				szChildName.Format(_T("%d_%d_childcheck"), nIndex, childIndex);
				DuiLib::COptionUI *pChildCheck = 
					static_cast<DuiLib::COptionUI*>(m_pm.FindControl(szChildName));
				if(pChildCheck != NULL)
					pChildCheck->Selected(needCompile);
			}
		}
		else if(name.Find(_T("_childcheck")) != -1)
		{
			name.Replace(_T("_childcheck"), _T(""));
			int nItem = _wtoi(name.Left(name.Find(_T("_"))));
			int nCfg = _wtoi(name.Mid(name.Find(_T("_"))+1));
			DuiLib::COptionUI* pCurOp = static_cast<DuiLib::COptionUI*>(msg.pSender);
			bool needCompile = pCurOp->IsSelected();
			if(needCompile)
			{
				m_pPrjData->at(nItem).childCompileNode.at(nCfg).bNeedCompile = true;
				CString szItem;
				szItem.Format(_T("%d_check"), nItem);
				DuiLib::COptionUI* pCurOp =
					static_cast<DuiLib::COptionUI*>(m_pm.FindControl(szItem));
				if(pCurOp!=NULL)
				{
					pCurOp->Selected(true);
					m_pPrjData->at(nItem).bCompileNeed = true;
				}
			}
			else
			{
				m_pPrjData->at(nItem).childCompileNode.at(nCfg).bNeedCompile = false;
				std::vector<COMPILE_NODE>::iterator itchild = 
					m_pPrjData->at(nItem).childCompileNode.begin();
				bool bExistCompile = false;
				for (; itchild!=m_pPrjData->at(nItem).childCompileNode.end(); itchild++)
				{
					if(itchild->bNeedCompile)
					{
						bExistCompile = true;
						break;
					}
				}
				if(!bExistCompile)
				{
					CString szItem;
					szItem.Format(_T("%d_check"), nItem);
					DuiLib::COptionUI* pCurOp =
						static_cast<DuiLib::COptionUI*>(m_pm.FindControl(szItem));
					if(pCurOp!=NULL)
					{
						pCurOp->Selected(false);
						m_pPrjData->at(nItem).bCompileNeed = false;
					}
				}
			}
		}
		return;

	}
	else if(msg.sType == _T("itemselect"))
	{
		CString szCtrlName = msg.pSender->GetName();
		if(szCtrlName.Find(_T("_combo")) != -1)
		{
			DuiLib::CComboBoxUI* pCbBox = static_cast<DuiLib::CComboBoxUI*>(msg.pSender);
			DuiLib::CListLabelElementUI* pListLabel = static_cast<DuiLib::CListLabelElementUI*>(pCbBox->GetItemAt(pCbBox->GetCurSel()));
			szCtrlName.Replace(_T("_combo"), _T(""));
			CString szCmpverName;
			szCmpverName = szCtrlName+_T("_cmpvername");
			DuiLib::CLabelUI* pLabelPlatform = static_cast<DuiLib::CLabelUI*>(m_pm.FindControl(szCmpverName));
			if(pLabelPlatform != NULL)
			{
				pLabelPlatform->SetText(pListLabel->GetUserData());
			}
			int nItem = _wtoi(szCtrlName.Left(szCtrlName.Find(_T("_"))));
			int nCfg = _wtoi(szCtrlName.Mid(szCtrlName.Find(_T("_"))+1));
			
			CString szCfgName = CString(pCbBox->GetText())+pListLabel->GetUserData();
			setCmpCfgSelected(nItem, nCfg, szCfgName);
			if(pCmpModel!=NULL){
				pCmpModel->RecordUserCompileSetting(nItem, nCfg, szCfgName);
			}
		}
		return;
	}
}

LRESULT CCompilerWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_pm.Init(m_hWnd);

#ifndef _DEBUG
	// zip res
	HRSRC hResource = ::FindResource(m_pm.GetResourceDll(), MAKEINTRESOURCE(IDR_ZIPRES1), _T("ZIPRES"));
	if( hResource == NULL )
		return 0L;
	DWORD dwSize = 0;
	HGLOBAL hGlobal = ::LoadResource(m_pm.GetResourceDll(), hResource);
	if( hGlobal == NULL ) 
	{
#if defined(WIN32) && !defined(UNDER_CE)
		::FreeResource(hResource);
#endif
		return 0L;
	}
	dwSize = ::SizeofResource(m_pm.GetResourceDll(), hResource);
	if( dwSize == 0 )
		return 0L;
	LPBYTE m_lpResourceZIPBuffer = new BYTE[ dwSize ];
	if (m_lpResourceZIPBuffer != NULL)
	{
		::CopyMemory(m_lpResourceZIPBuffer, (LPBYTE)::LockResource(hGlobal), dwSize);
	}
#if defined(WIN32) && !defined(UNDER_CE)
	::FreeResource(hResource);
#endif
	m_pm.SetResourceZip(m_lpResourceZIPBuffer, dwSize);

#endif

	DuiLib::CDialogBuilder builder;
	CDialogBuilderCallbackEx cb;
	DuiLib::CControlUI* pRoot = builder.Create(_T("skin.xml"), (UINT)0,  &cb, &m_pm);
	ASSERT(pRoot && "Failed to parse XML");
	m_pm.AttachDialog(pRoot);
	m_pm.AddNotifier(this);

	return 0;
}

LRESULT CCompilerWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT CCompilerWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::PostQuitMessage(0L);

	bHandled = FALSE;
	return 0;
}

LRESULT CCompilerWnd::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if( ::IsIconic(*this) ) bHandled = FALSE;
	return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CCompilerWnd::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CCompilerWnd::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CCompilerWnd::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(*this, &pt);

	RECT rcClient;
	::GetClientRect(*this, &rcClient);

	// 		if( !::IsZoomed(*this) ) {
	// 			RECT rcSizeBox = m_pm.GetSizeBox();
	// 			if( pt.y < rcClient.top + rcSizeBox.top ) {
	// 				if( pt.x < rcClient.left + rcSizeBox.left ) return HTTOPLEFT;
	// 				if( pt.x > rcClient.right - rcSizeBox.right ) return HTTOPRIGHT;
	// 				return HTTOP;
	// 			}
	// 			else if( pt.y > rcClient.bottom - rcSizeBox.bottom ) {
	// 				if( pt.x < rcClient.left + rcSizeBox.left ) return HTBOTTOMLEFT;
	// 				if( pt.x > rcClient.right - rcSizeBox.right ) return HTBOTTOMRIGHT;
	// 				return HTBOTTOM;
	// 			}
	// 			if( pt.x < rcClient.left + rcSizeBox.left ) return HTLEFT;
	// 			if( pt.x > rcClient.right - rcSizeBox.right ) return HTRIGHT;
	// 		}

	RECT rcCaption = m_pm.GetCaptionRect();
	if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
		&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
			DuiLib::CControlUI* pControl = static_cast<DuiLib::CControlUI*>(m_pm.FindControl(pt));
			if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 && 
				_tcscmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
				_tcscmp(pControl->GetClass(), _T("TextUI")) != 0 )
				return HTCAPTION;
	}

	return HTCLIENT;
}

LRESULT CCompilerWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SIZE szRoundCorner = m_pm.GetRoundCorner();
	if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
		DuiLib::CDuiRect rcWnd;
		::GetWindowRect(*this, &rcWnd);
		rcWnd.Offset(-rcWnd.left, -rcWnd.top);
		rcWnd.right++; rcWnd.bottom++;
		HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
		::SetWindowRgn(*this, hRgn, TRUE);
		::DeleteObject(hRgn);
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CCompilerWnd::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	DuiLib::CDuiRect rcWork = oMonitor.rcWork;
	rcWork.Offset(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);

	LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
	lpMMI->ptMaxPosition.x	= rcWork.left;
	lpMMI->ptMaxPosition.y	= rcWork.top;
	lpMMI->ptMaxSize.x		= rcWork.right;
	lpMMI->ptMaxSize.y		= rcWork.bottom;

	bHandled = FALSE;
	return 0;
}

LRESULT CCompilerWnd::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
	if( wParam == SC_CLOSE )
	{
		::PostQuitMessage(0L);
		bHandled = TRUE;
		return 0;
	}
	BOOL bZoomed = ::IsZoomed(*this);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	if( ::IsZoomed(*this) != bZoomed ) 
	{
		if( !bZoomed )
		{
			DuiLib::CControlUI* pControl = static_cast<DuiLib::CControlUI*>(m_pm.FindControl(_T("maxbtn")));
			if( pControl )
				pControl->SetVisible(false);
			pControl = static_cast<DuiLib::CControlUI*>(m_pm.FindControl(_T("restorebtn")));
			if( pControl )
				pControl->SetVisible(true);
		}
		else 
		{
			DuiLib::CControlUI* pControl = static_cast<DuiLib::CControlUI*>(m_pm.FindControl(_T("maxbtn")));
			if( pControl ) 
				pControl->SetVisible(true);
			pControl = static_cast<DuiLib::CControlUI*>(m_pm.FindControl(_T("restorebtn")));
			if( pControl )
				pControl->SetVisible(false);
		}
	}
	return lRes;
}

// 屏蔽双击窗体放大整个窗口
LRESULT CCompilerWnd::OnNcDBClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return 0L;
}

LRESULT CCompilerWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch( uMsg ) 
	{
	case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
	case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
	case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
	case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
	case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
	case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
	case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
	case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
	case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
	case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
	case WM_NCLBUTTONDBLCLK: lRes = OnNcDBClick(uMsg, wParam, lParam, bHandled); break;

	default:
		bHandled = FALSE;
	}
	if( bHandled ) 
		return lRes;
	if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) 
		return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}


void CCompilerWnd::_init()
{
	this->Create(NULL, _T("打包精灵 v0.1"),
		UI_WNDSTYLE_FRAME,
		0L, 0, 0, 800, 572);
	this->CenterWindow();
	this->SetIcon(IDI_LOGO);
	this->ShowWindow(true,false);
}


void CCompilerWnd::OnPrepare()
{
	m_pCloseBtn = static_cast<DuiLib::CButtonUI*>(m_pm.FindControl(_T("closebtn")));
	m_pProjectList = static_cast<CListUIEx*>(m_pm.FindControl(_T("project_list")));
	m_pGoCompile = static_cast<DuiLib::CButtonUI*>(m_pm.FindControl(_T("go-compile")));
	m_pCancelBtn = static_cast<DuiLib::CButtonUI*>(m_pm.FindControl(_T("cancel")));
	m_pCancelBtn->SetVisible(false);
	m_pCtrlBuildString = static_cast<DuiLib::CRichEditUI*>(m_pm.FindControl(_T("buildresult")));
	m_pEditCfgPath = static_cast<DuiLib::CEditUI*>(m_pm.FindControl(_T("cfg_path")));
	m_pBtnFindCfgFile = static_cast<DuiLib::CButtonUI*>(m_pm.FindControl(_T("find_cfg")));
	// 添加数据到界面
	AddDataToUI();
}

//////////////////////////////////////////////////////////////////////////
// 设置注册表中关于历史配置记录的值
bool CCompilerWnd::SetHistoryCfgInReg()
{
	if(m_szHistoryCfg.IsEmpty()) return false;

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
		m_szHistoryCfg,
		m_szHistoryCfg.GetLength());

	RegCloseKey(hkey_his_cfg);
	
	if(dRet == ERROR_SUCCESS)
		return true;
	else
		return false;
}


bool CCompilerWnd::GetCfgFile()
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

	m_szHistoryCfg = CString(openFileName.lpstrFile);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 向列表中添加数据
void CCompilerWnd::AddDataToUI(void)
{
	// 配置文件
	m_pEditCfgPath->SetText(m_szHistoryCfg);

	if(m_pPrjData == NULL)
		return;

	m_pProjectList->RemoveAll();

	std::vector<PROJECTDATA>::iterator itProj = m_pPrjData->begin();
	for (int i = 0; itProj!=m_pPrjData->end(); itProj++, i++)
	{
		AddNodeToUI(*itProj, i);
	}
}

void CCompilerWnd::AddNodeToUI(PROJECTDATA& pNode, int& nIndex)
{
	DuiLib::CListContainerElementUI *pItem = new DuiLib::CListContainerElementUI;
	m_pProjectList->InsertItem(nIndex, pNode.childCompileNode.size()*23, pItem);

	// 工程
	DuiLib::CHorizontalLayoutUI* pHor = new DuiLib::CHorizontalLayoutUI;
	pItem->Add(pHor);
	DuiLib::CVerticalLayoutUI* pVecLeft = new DuiLib::CVerticalLayoutUI;
	pVecLeft->ApplyAttributeList(_T("width=\"50\""));
	pHor->Add(pVecLeft);
	// 项目选取
	DuiLib::COptionUI *pCheck = new DuiLib::COptionUI;
	pCheck->ApplyAttributeList(_T("width=\"25\" height=\"25\" normalimage=\"file='short69_cube_op_img_0.png' dest='7,4,22,19'\" selectedimage=\"file='short69_cube_op_img_1.png' dest='7,4,22,19'\""));
	CString szCheckName;
	szCheckName.Format(_T("%d_check"), nIndex);
	pCheck->SetName(szCheckName);
	pVecLeft->Add(pCheck);

	// 项目名称
	DuiLib::CLabelUI* pPrjName = new DuiLib::CLabelUI;
	pPrjName->SetText(pNode.szPrjName);
	pPrjName->SetToolTip(pNode.szPrjName);
	pVecLeft->Add(pPrjName);

	// 子工程的选取
	DuiLib::CVerticalLayoutUI* pVecRight = new DuiLib::CVerticalLayoutUI;
	pHor->Add(pVecRight);

	// 项目配置文件路径
	DuiLib::CVerticalLayoutUI* pVecChdPrjPath = new DuiLib::CVerticalLayoutUI;
	pItem->Add(pVecChdPrjPath);

	// 编译器版本
	DuiLib::CVerticalLayoutUI* pVecCmpVer = new DuiLib::CVerticalLayoutUI;
	pItem->Add(pVecCmpVer);

	// 编译器选项
	DuiLib::CVerticalLayoutUI* pVecCmpCfg = new DuiLib::CVerticalLayoutUI;
	pItem->Add(pVecCmpCfg);

	
	// 编译平台
	DuiLib::CVerticalLayoutUI* pVecPlatformVer = new DuiLib::CVerticalLayoutUI;
	pItem->Add(pVecPlatformVer);

	// 编译结果
	DuiLib::CVerticalLayoutUI* pVecCmpResult = new DuiLib::CVerticalLayoutUI;
	pItem->Add(pVecCmpResult);


	std::vector<COMPILE_NODE>::iterator childIt = pNode.childCompileNode.begin();
	for (int childIndex=0; childIt!=pNode.childCompileNode.end(); childIt++, childIndex++)
	{
		DuiLib::CHorizontalLayoutUI* pChildHor = new DuiLib::CHorizontalLayoutUI;
		pVecRight->Add(pChildHor);
		// 选项框
		DuiLib::COptionUI *pChildCheck = new DuiLib::COptionUI;
		pChildCheck->ApplyAttributeList(_T("width=\"25\" height=\"25\" normalimage=\"file='short69_cube_op_img_0.png' dest='7,4,22,19'\" selectedimage=\"file='short69_cube_op_img_1.png' dest='7,4,22,19'\""));
		CString szChildCheckName;
		szChildCheckName.Format(_T("%d_%d_childcheck"), nIndex, childIndex);
		pChildCheck->SetName(szChildCheckName);
		pChildHor->Add(pChildCheck);

		// 工程标题
		//DuiLib::CLabelUI* pChildPrjTitle = new DuiLib::CLabelUI;
		//pChildPrjTitle->SetText(childIt->szPrjName);
		//pChildHor->Add(pChildPrjTitle);
		DuiLib::CButtonUI* pBtnChildPrjTitle = new DuiLib::CButtonUI;
		if(childIt->cfgType == vcproject){
			pBtnChildPrjTitle->SetText(childIt->szPrjName);
		} else if(childIt->cfgType == copyfile) {
			pBtnChildPrjTitle->SetText(_T("文件复制"));
		} else if(childIt->cfgType == delfolder) {
			pBtnChildPrjTitle->SetText(_T("删除文件夹"));
		}
		pBtnChildPrjTitle->ApplyAttributeList(_T("hottextcolor=\"0xff8000ff\" pushedtextcolor=\"0xff2112ed\""));
		CString szBtnPrjName;
		szBtnPrjName.Format(_T("%d_%d_projectname"), nIndex, childIndex);
		pBtnChildPrjTitle->SetName(szBtnPrjName);
		pChildHor->Add(pBtnChildPrjTitle);


		// 配置文件路径
		DuiLib::CLabelUI* pChildPrjCfgPath = new DuiLib::CLabelUI;
		if(childIt->cfgType == vcproject) {
			pChildPrjCfgPath->SetText(childIt->sz_PrjFilePath); 
			pChildPrjCfgPath->SetToolTip(childIt->sz_PrjFilePath);
		} else if(childIt->cfgType == copyfile) {
			pChildPrjCfgPath->SetText(childIt->szSrcFile);
			pChildPrjCfgPath->SetToolTip(childIt->szSrcFile);
		} else if(childIt->cfgType == delfolder) {
			pChildPrjCfgPath->SetText(childIt->szDelFolder);
			pChildPrjCfgPath->SetToolTip(childIt->szDelFolder);
		}
		pChildPrjCfgPath->SetFixedHeight(23);
		pVecChdPrjPath->Add(pChildPrjCfgPath);

		// 编译器版本
		DuiLib::CLabelUI* pChildPrjCmpVer = new DuiLib::CLabelUI;
		pChildPrjCmpVer->SetTextStyle(DT_CENTER|DT_VCENTER);
		if(childIt->nCompilerVer == vs80)
			pChildPrjCmpVer->SetText(_T("vs2005"));
		else if(childIt->nCompilerVer == vs90)
			pChildPrjCmpVer->SetText(_T("vs2008"));
		else if(childIt->nCompilerVer == vs100)
			pChildPrjCmpVer->SetText(_T("vs2010"));
		else if(childIt->nCompilerVer == vs110)
			pChildPrjCmpVer->SetText(_T("vs2012"));
		else if(childIt->nCompilerVer == vs140)
			pChildPrjCmpVer->SetText(_T("vs2015"));
		else
			pChildPrjCmpVer->SetText(_T("未知"));
		pChildPrjCmpVer->SetFixedHeight(23);
		pVecCmpVer->Add(pChildPrjCmpVer);

		// 编译选项
		DuiLib::CComboBoxUI *pCombo = new DuiLib::CComboBoxUI;
		pCombo->SetItemTextStyle(DT_CENTER|DT_VCENTER);
		pCombo->SetFixedHeight(23);
		pCombo->SetBkImage(_T("cmb_bkg.png"));
		CString szComboName;
		szComboName.Format(_T("%d_%d_combo"), nIndex, childIndex);
		pCombo->SetName(szComboName);

		std::vector<COMPILE_INSTANCE>::iterator Instance = childIt->vecConfiguration.begin();
		int selectedCfg = 0; 
		bool selectedFound = false;
		while (Instance != childIt->vecConfiguration.end())
		{
			DuiLib::CListLabelElementUI *pLstLBItem = new DuiLib::CListLabelElementUI;
			pLstLBItem->SetText(Instance->szInstanceName);
			pLstLBItem->SetUserData(Instance->szPlatfromVer);
			pCombo->Add(pLstLBItem);

			if(Instance->isSelected)
			{
				selectedFound = true;
				pCombo->SelectItem(selectedCfg);
			}
			Instance++;
			selectedCfg++;
		}
		if(!selectedFound)
			pCombo->SelectItem(0);
		pVecCmpCfg->Add(pCombo);

		// 编译平台
		DuiLib::CLabelUI* pPlatVer = new DuiLib::CLabelUI;
		pPlatVer->SetFixedHeight(23);
		pPlatVer->SetTextStyle(DT_VCENTER|DT_CENTER);
		//if(childIt->vecConfiguration.size() == 0)
		//	continue;
		
		bool noVecCfgSelected = true;
		for (int vecCfgIndex = 0; vecCfgIndex < childIt->vecConfiguration.size(); vecCfgIndex++)
		{
			if(childIt->vecConfiguration.at(vecCfgIndex).isSelected)
			{
				pPlatVer->SetText(childIt->vecConfiguration.at(vecCfgIndex).szPlatfromVer);
				noVecCfgSelected = false;
				break;
			}
		}
	
		CString szCmpverName;
		szCmpverName.Format(_T("%d_%d_cmpvername"), nIndex, childIndex);
		pPlatVer->SetName(szCmpverName);
		pVecPlatformVer->Add(pPlatVer);

		//childIt->vecConfiguration.begin()->isSelected = true;

		// 编译结果
		DuiLib::CButtonUI *pBtnResult = new DuiLib::CButtonUI;
		pBtnResult->SetFixedHeight(23);
		pBtnResult->SetTextStyle(DT_CENTER|DT_VCENTER);
		CString szResultName;
		szResultName.Format(_T("%d_%d_result"), nIndex, childIndex);
		pBtnResult->SetName(szResultName);
		pVecCmpResult->Add(pBtnResult);
	}
}

void CCompilerWnd::setCmpCfgSelected(int nItem, int nCfg, CString szCfgName)
{
	std::vector<COMPILE_INSTANCE>::iterator itChild = 
		m_pPrjData->at(nItem).childCompileNode.at(nCfg).vecConfiguration.begin();
	for (; itChild!= m_pPrjData->at(nItem).childCompileNode.at(nCfg).vecConfiguration.end();
		itChild++)
	{
		if((itChild->szInstanceName+itChild->szPlatfromVer) == szCfgName)
			itChild->isSelected = true;
		else
			itChild->isSelected = false;
	}
}

void CCompilerWnd::SetModelPointer(void* pClass)
{
	if(pClass!=NULL)
	{
		pCmpModel = static_cast<CompileModel*>(pClass);
	}
}

void CCompilerWnd::ShowResult(CString &szOutcome)
{
	if(m_pCtrlBuildString!=NULL)
	{
		m_pCtrlBuildString->AppendText(szOutcome);
	}
}

void CCompilerWnd::ShowItemResult(int nItem, 
								  int nCfg,
								  CString &szOutcome)
{
	CString szResult;
	if (szOutcome.Find(_T("生成成功")) != -1 ||
		szOutcome.Find(_T("已成功生成")) != -1 ||
		szOutcome.Find(_T("Build succeeded")) != -1){
		szResult = _T("成功");
	}
	else{
		szResult = _T("失败");
	}

	CString szBtnName;
	szBtnName.Format(_T("%d_%d_result"), nItem, nCfg);
	DuiLib::CLabelUI* pBtnResult =
		static_cast<DuiLib::CButtonUI*>(m_pm.FindControl(szBtnName));
	if(pBtnResult != NULL)
	{
		pBtnResult->SetText(szResult);
		pBtnResult->SetUserData(szOutcome);
	}
}