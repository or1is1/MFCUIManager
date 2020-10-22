#pragma once

#include <memory>
#include <mutex>

#define DLU_RATIO 2

using namespace std;

class UIManager
{
private:
	UIManager();

	// 싱글톤 구현 관련
	static unique_ptr<UIManager> m_pUIManager;
	static once_flag mOnceFlag;

	// 복사 방지 코드
	//UIManager() = default;
	UIManager(const UIManager &) = delete;
	UIManager &operator=(const UIManager &) = delete;

	// 해상도 관련
	int m_targetWidth;
	int m_targetHeight;

	// DLU 관련 변수 정의
	float m_DLU_X;
	float m_DLU_Y;
	bool m_isInitDLU;

public:
	~UIManager();
	// 싱글톤 구현 관련
	static UIManager& getInstance();

	// 윈도우 작업표시줄 관련
	bool GetTrayWndRect(CRect& trWndRect);
	LONG GetTrayWndHeight();
	void HideTrayWnd();
	void RestoreTrayWnd();

	// 크기 조절 (Frame::PreCreateWindow 에서 호출)
	void Resize(CREATESTRUCT& cs, bool fullscreen = false, bool coverTrayWnd = true);

	// 메뉴 지우기 (Frame::PreCreateWindow 에서 호출)
	bool DelMenu(CREATESTRUCT& cs);

	// 스크롤바 비활성화 (View::OnInitialUpdate 에서 호출)
	void DisableScrollBar(CScrollView* cWnd);

	// 2px 테두리 지우기 (View::OnInitialUpdate 에서 호출)
	bool DelEdge(CWnd* CWnd);

	// DLU 관련 (View::OnInitialUpdate 에서 호출)
	bool InitDLU(CWnd* CWnd);
	bool CvtDLU(CRect rect);
	bool CvtDLU(CWnd* CWnd);
	bool CvtDLU(CWnd* CWnd, int args, ...);

	// 배경 지정 (View::OnDraw 에서 호출)
	void SetBack(CWnd* cWnd, CDC* pDC, COLORREF clr = RGB(255, 255, 255));

	// 스킨 지정 (View::OnDrawItem 에서 호출)
	void SetSkin(LPDRAWITEMSTRUCT& lpDrawItemStruct, COLORREF clr = RGB(80, 80, 80), int fontSize = 8);
};
