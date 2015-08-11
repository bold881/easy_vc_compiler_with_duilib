#include "stdafx.h"
#include "ListEx.h"

/************************** CListUIEx Start **************************/
CListUIEx::CListUIEx()
{
}

CListUIEx::~CListUIEx()
{
}

//ͨ�����봴����  
BOOL CListUIEx::InsertColumn(
		int nCol,
		LPCTSTR lpszColumnHeaderStr,
		UINT uTextStyle/* = DT_LEFT*/,
		int nWidth/* = -1*/
		)
{
	DuiLib::CListHeaderItemUI *pHeaderItem = new DuiLib::CListHeaderItemUI;
	pHeaderItem->SetTextStyle(uTextStyle|DT_VCENTER|DT_SINGLELINE);
	pHeaderItem->SetText(lpszColumnHeaderStr);

	pHeaderItem->SetAttribute(_T("sepimage"), _T("Headerctrl_Sperator.bmp"));
	pHeaderItem->SetAttribute(_T("sepwidth"), _T("1"));
	pHeaderItem->SetAttribute(_T("pushedimage"), _T("headerctrl_down.bmp"));
	pHeaderItem->SetAttribute(_T("hotimage"), _T("headerctrl_hot.bmp"));
	pHeaderItem->SetAttribute(_T("normalimage"), _T("headerctrl_normal.bmp"));
	pHeaderItem->SetFixedWidth(nWidth);

	return InsertColumn(nCol, pHeaderItem);
}


BOOL CListUIEx::InsertColumn(
							 int nCol,
							 DuiLib::CListHeaderItemUI *pHeaderItem)
{
	DuiLib::CListHeaderUI *pHeader = CListUI::GetHeader();
	if (pHeader == NULL)
	{
		return FALSE;
	}
	
	if (pHeader->AddAt(pHeaderItem, nCol))
	{		
		return TRUE;
	}

	delete pHeaderItem;
	pHeaderItem = NULL;
	return FALSE;
}

BOOL CListUIEx::SetHeaderItemData(int nColumn, CControlUI* pControl)
{
	DuiLib::CListHeaderUI *pHeader = CListUI::GetHeader();
	if (pHeader == NULL)
	{
		return FALSE;
	}
	DuiLib::CListHeaderItemUI *pHeaderItem = (DuiLib::CListHeaderItemUI *)pHeader->GetItemAt(nColumn);
	pHeaderItem->Add(pControl);
	return TRUE;
}

int CListUIEx::InsertItem(int nItem, int nHeight)
{
	DuiLib::CListContainerElementUI *pListItem = new DuiLib::CListContainerElementUI;
	pListItem->SetFixedHeight(nHeight);/*�̶�һ���и�*/

	pListItem->m_pHeader = CListUI::GetHeader();
	if (NULL != pListItem->m_pHeader)
	{
		int nHeaderCount = pListItem->m_pHeader->GetCount();
		for (int i = 0; i < nHeaderCount; i++)
		{
			pListItem->Add(new DuiLib::CHorizontalLayoutUI);
		}
	}
	if ( !CListUI::AddAt(pListItem, nItem) )
	{
		delete pListItem;
		pListItem = NULL;
		return -1;
	}
	return nItem;
}


int CListUIEx::InsertItem(int nItem, int nHeight, DuiLib::CListContainerElementUI *pListItem)
{
	pListItem->SetFixedHeight(nHeight);	
	pListItem->m_pHeader =  CListUI::GetHeader();
	if ( !CListUI::AddAt(pListItem, nItem) )
	{
		delete pListItem;
		pListItem = NULL;
		return -1;
	}
	
	return nItem;
}

void CListUIEx::SetItemData(int nItem,
							int nColumn,
							LPCTSTR Text, LPCTSTR Name)
{
	DuiLib::CLabelUI *pLabel = new DuiLib::CLabelUI;
	pLabel->SetText(Text);//�ؼ����Ծ͸����������ð�,�Ҽ�����һ��
	pLabel->SetTextStyle(DT_CENTER);
	pLabel->SetAttribute(_T("endellipsis"), _T("true"));
	pLabel->SetName(Name);
	SetItemData(nItem, nColumn, pLabel);//��ӵ����ؼ�
}

void CListUIEx::SetItemData(int nItem, int nColumn,	CControlUI* pControl)
{
	DuiLib::CHorizontalLayoutUI *pSubHor = GetListSubItem(nItem, nColumn);
	pSubHor->SetAttribute(_T("inset"), _T("3,1,3,1"));
	pSubHor->Add(pControl);//��ӵ����ؼ�
}

DuiLib::CListContainerElementUI* CListUIEx::GetListItem(int iIndex)
{
	return static_cast<DuiLib::CListContainerElementUI*>(CListUI::GetItemAt(iIndex));
}

DuiLib::CHorizontalLayoutUI* CListUIEx::GetListSubItem(int iIndex, int iSubIndex)
{
	//��ȡ�����пؼ�
	DuiLib::CListContainerElementUI *pListItem = static_cast<DuiLib::CListContainerElementUI*>(CListUI::GetItemAt(iIndex));
	if (pListItem == NULL)
		return NULL;
	return static_cast<DuiLib::CHorizontalLayoutUI*>(pListItem->GetItemAt(iSubIndex));
}
