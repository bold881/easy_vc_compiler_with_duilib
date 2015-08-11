#include "stdafx.h"

#include "CompileModel.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	DuiLib::CPaintManagerUI::SetInstance(hInstance);
#ifdef _DEBUG
	DuiLib::CPaintManagerUI::SetResourcePath(DuiLib::CPaintManagerUI::GetInstancePath() + _T("skin\\CompileGRes"));
#endif

	HRESULT Hr = ::CoInitialize(NULL);
	if( FAILED(Hr) ) return 0;

	CompileModel	 m_model;
	if(m_model._init()){
		DuiLib::CPaintManagerUI::MessageLoop();
	}

	::CoUninitialize();
	return 0;
}