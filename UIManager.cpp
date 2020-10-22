#include "stdafx.h"
#include "UIManager.h"
#include <cstdarg>

using namespace std;

std::unique_ptr<UIManager> UIManager::m_pUIManager;
std::once_flag UIManager::mOnceFlag;

UIManager::UIManager()
	: m_isInitDLU(false)
	, m_targetWidth(1280)
	, m_targetHeight(1024)
{
}

UIManager::~UIManager()
{
}

UIManager& UIManager::getInstance()
{
	call_once(mOnceFlag, []() {
		m_pUIManager.reset(new UIManager);
	});

	return *(m_pUIManager.get());
}

// 윈도우 작업표시줄의 크기를 구하는 함수
bool UIManager::GetTrayWndRect(CRect& trWndRect){
	HWND hTrayWnd = ::FindWindow(_T("Shell_TrayWnd"), _T(""));

	if (hTrayWnd)
	{
		::GetWindowRect(hTrayWnd, &trWndRect);
		return true;
	}

	return false;
}

// 윈도우 작업표시줄의 높이를 구하는 함수
LONG UIManager::GetTrayWndHeight(){
	CRect trWndRect;

	GetTrayWndRect(trWndRect);

	return trWndRect.bottom - trWndRect.top;
}

// 윈도우 작업표시줄 숨기기
void UIManager::HideTrayWnd()
{
	APPBARDATA stAppBarData;
	ZeroMemory(&stAppBarData, sizeof(stAppBarData));
	stAppBarData.cbSize = sizeof(stAppBarData);

	stAppBarData.hWnd = (HWND)FindWindow(_T("Shell_TrayWnd"), NULL);
	stAppBarData.lParam |= ABS_AUTOHIDE;

	SHAppBarMessage(ABM_SETSTATE, &stAppBarData);
}

// 윈도우 작업표시줄 복구
void UIManager::RestoreTrayWnd()
{
	APPBARDATA stAppBarData;
	ZeroMemory(&stAppBarData, sizeof(stAppBarData));
	stAppBarData.cbSize = sizeof(stAppBarData);

	stAppBarData.hWnd = (HWND)FindWindow(_T("Shell_TrayWnd"), NULL);
	stAppBarData.lParam |= ABS_ALWAYSONTOP;

	SHAppBarMessage(ABM_SETSTATE, &stAppBarData);
}

// 메인프레임의 크기를 지정하는 함수
void UIManager::Resize(CREATESTRUCT& cs, bool fullscreen, bool coverTrayWnd)
{
	cs.style = WS_POPUP;

	// 기본값일 땐, 최대 해상도로 생성 = fullscreen
	if (fullscreen)
	{
		cs.x = 0;
		cs.y = 0;
		cs.cx = GetSystemMetrics(SM_CXSCREEN);
		cs.cy = GetSystemMetrics(SM_CYSCREEN) - (coverTrayWnd ? 0 : GetTrayWndHeight());
	}
	else
	{
		cs.x = (GetSystemMetrics(SM_CXSCREEN) - m_targetWidth) / 2;
		cs.y = (GetSystemMetrics(SM_CYSCREEN) - m_targetHeight) / 2;
		cs.cx = m_targetWidth;
		cs.cy = m_targetHeight - (coverTrayWnd ? 0 : GetTrayWndHeight());
	}
}

// 메뉴바 삭제
bool UIManager::DelMenu(CREATESTRUCT& cs)
{
	if (cs.hMenu != NULL) {
		::DestroyMenu(cs.hMenu);
		cs.hMenu = NULL;
		return true;
	}

	return false;
}

// 뷰의 2px 테두리 삭제
bool UIManager::DelEdge(CWnd* cWnd)
{
	if (!cWnd)
		return false;

	cWnd->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);
	
	return true;
}

// DLUS 초기화
bool UIManager::InitDLU(CWnd* cWnd)
{

	CFont* pFont = cWnd->GetFont();
	CDC *pDC = cWnd->GetDC();

	if (!pFont)
		throw "GetFont()의 반환값이 null 입니다.";

	LOGFONT lf;
	pFont->GetLogFont(&lf);
	CFont* oldFont = pDC->SelectObject(pFont);

	CSize size;
	size = pDC->GetTextExtent(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"), 52);

	m_DLU_X = (size.cx / 26 + 1) / 2 / 4.f;

	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	m_DLU_Y = tm.tmHeight / 8.f;

	pDC->SelectObject(oldFont);

	m_isInitDLU = true;

	return true;
}


bool UIManager::CvtDLU(CWnd* cWnd)
{
	if (!m_isInitDLU)
		InitDLU(cWnd);

	CRect rect = NULL;
	cWnd->GetWindowRect(&rect);

	CWnd* parentWnd = cWnd->GetParent();
	parentWnd->ScreenToClient(&rect);

	rect.left   = LONG(rect.left   / m_DLU_X * DLU_RATIO);
	rect.top    = LONG(rect.top    / m_DLU_Y * DLU_RATIO);
	rect.right  = LONG(rect.right  / m_DLU_X * DLU_RATIO);
	rect.bottom = LONG(rect.bottom / m_DLU_Y * DLU_RATIO);

	MoveWindow(cWnd->GetSafeHwnd(), rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);

	return true;
}

bool UIManager::CvtDLU(CWnd* cWnd, int args, ...)
{
	va_list ap;
	
	va_start(ap, args);
	for (int i = 0; i < args; i++)
		CvtDLU((CView*)cWnd->GetDlgItem(va_arg(ap, int)));
	va_end(ap);

	return true;
}

// 배경 지정
void UIManager::SetBack(CWnd* cWnd, CDC* pDC, COLORREF clr)
{
	CRect viewRect;
	cWnd->GetClientRect(viewRect);

	// 배경색 변경
	pDC->FillSolidRect(viewRect, clr);
}

// 스킨 지정
void UIManager::SetSkin(LPDRAWITEMSTRUCT& lpDrawItemStruct, COLORREF clr, int fontSize)
{
	CDC dc;
	RECT rect;
	dc.Attach(lpDrawItemStruct->hDC);                       // 버튼의 dc구하기
	rect = lpDrawItemStruct->rcItem;                        // 버튼영역 구하기
	dc.Draw3dRect(&rect, RGB(255, 255, 255), RGB(0, 0, 0)); // 버튼의 외곽선 그리기
	dc.FillSolidRect(&rect, clr);               // 버튼색상
	dc.SetTextColor(RGB(255, 255, 255));                    // texttort

	// 폰트 결정
	CFont font, *pOldFont;
	font.CreatePointFont(fontSize * 10, _T("MS Shell Dlg"));
	pOldFont = (CFont*)dc.SelectObject(&font);

	TCHAR buffer[MAX_PATH];                                 // 버튼의 text를 얻기위한 임시버퍼
	ZeroMemory(buffer, MAX_PATH);                           // 버퍼초기화
	::GetWindowText(lpDrawItemStruct->hwndItem, buffer, MAX_PATH);      // 버튼의 text얻기
	dc.DrawText(buffer, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE); // 버튼의 text넣기
	dc.Detach();                                                        // 버튼의 dc 풀어주기
}

// 스크롤바 비활성화
void UIManager::DisableScrollBar(CScrollView* cWnd)
{
	CSize size;
	size = cWnd->GetTotalSize();
	size.cx = 0;
	size.cy = 0;
	cWnd->SetScrollSizes(MM_TEXT, size);
}
