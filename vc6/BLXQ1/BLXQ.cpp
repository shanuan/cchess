// BLXQ.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "mmsystem.h"
#include "BLXQ.h"

#define MAX_LOADSTRING 100

// 版本号
const LPCSTR cszAbout = "象棋小巫师 0.1\n象棋百科全书 荣誉出品\n\n"
    "(C) 2004-2008 www.xqbase.com\n本产品符合GNU通用公共许可协议\n\n"
    "欢迎登录 www.xqbase.com\n免费下载PC版 象棋巫师";

// 窗口和绘图属性
const int WINDOW_STYLES = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
const int MASK_COLOR = RGB(0, 255, 0);

const int SQUARE_SIZE = 56;
const int BOARD_EDGE = 8;
const int BOARD_WIDTH = BOARD_EDGE + SQUARE_SIZE * 9 + BOARD_EDGE;
const int BOARD_HEIGHT = BOARD_EDGE + SQUARE_SIZE * 10 + BOARD_EDGE;

// 棋子编号
const int PIECE_KING = 0;
const int PIECE_PAWN = 6;

// 棋盘范围 
const int RANK_TOP = 3;
const int RANK_BOTTOM = 12;
const int FILE_LEFT = 3; 
const int FILE_RIGHT = 11;

// 棋盘初始设置
static const BYTE cucpcStartup[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 20, 19, 18, 17, 16, 17, 18, 19, 20,  0,  0,  0,  0,
  0,  0,  0, 12,  0,  0, 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0, 21,  0,  0,  0,  0,  0, 21,  0,  0,  0,  0,  0,
  0,  0,  0, 22,  0, 22,  0, 22,  0, 22,  0, 22,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 14,  0, 14,  0, 14,  0, 14,  0, 14,  0,  0,  0,  0,
  0,  0,  0,  0, 13,  0,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 12, 11, 10,  9,  8,  9, 10, 11, 12,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};


// 获得格子的横坐标
inline int RANK_Y(int sq) {
  return sq >> 4;
}

// 获得格子的纵坐标
inline int FILE_X(int sq) {
  return sq & 15;
}

// 获得红黑标记(红子是8，黑子是16)
inline int SIDE_TAG(int sd) {
  return 8 + (sd << 3);
}
// 获得走法的起点
inline int SRC(int mv) {
  return mv & 255;
}
// 获得走法的终点
inline int DST(int mv) {
  return mv >> 8;
}

// 根据起点和终点获得走法
inline int MOVE(int sqSrc, int sqDst) {
  return sqSrc + sqDst * 256;
}


// 局面结构
struct PositionStruct {
  int sdPlayer;                   // 轮到谁走，0=红方，1=黑方
  BYTE ucpcSquares[256];          // 棋盘上的棋子

  void Startup(void) {            // 初始化棋盘
    sdPlayer = 0;
    memcpy(ucpcSquares, cucpcStartup, 256);
  }
  void ChangeSide(void) {         // 交换走子方
    sdPlayer = 1 - sdPlayer;
  }
  void AddPiece(int sq, int pc) { // 在棋盘上放一枚棋子
    ucpcSquares[sq] = pc;
  }
  void DelPiece(int sq) {         // 从棋盘上拿走一枚棋子
    ucpcSquares[sq] = 0;
  }
  void MovePiece(int mv);         // 搬一步棋的棋子
  void MakeMove(int mv) {         // 走一步棋
    MovePiece(mv);
    ChangeSide();
  }
};

// 搬一步棋的棋子
void PositionStruct::MovePiece(int mv) {
  int sqSrc, sqDst, pc;
  sqSrc = SRC(mv);
  sqDst = DST(mv);
 
  pc = ucpcSquares[sqSrc];
  DelPiece(sqSrc);
  AddPiece(sqDst, pc);
}

static PositionStruct pos; // 局面实例

// 与图形界面有关的全局变量
 
CBlWnd		Xqwl;

// 初始化棋局
static void xdStartup(void) {
  pos.Startup();
  Xqwl.sqSelected = Xqwl.mvLast = 0;
}

 


// 根据纵坐标和横坐标获得格子
inline int COORD_XY(int x, int y) {
  return x + (y << 4);
}

// 纵坐标水平镜像
inline int FILE_FLIP(int x) {
  return 14 - x;
}

// 横坐标垂直镜像
inline int RANK_FLIP(int y) {
  return 15 - y;
}

// TransparentBlt 的替代函数，用来修正原函数在 Windows 98 下资源泄漏的问题
static void TransparentBlt2(HDC hdcDest, 
							int nXOriginDest, int nYOriginDest, 
							int nWidthDest, int nHeightDest,
							HDC hdcSrc, int nXOriginSrc, 
							int nYOriginSrc, int nWidthSrc,
							int nHeightSrc, 
							UINT crTransparent)
 {
  HDC hImageDC, hMaskDC;
  HBITMAP hOldImageBMP, hImageBMP, hOldMaskBMP, hMaskBMP;

  hImageBMP = CreateCompatibleBitmap(hdcDest, nWidthDest, nHeightDest);
  hMaskBMP = CreateBitmap(nWidthDest, nHeightDest, 1, 1, NULL);
  hImageDC = CreateCompatibleDC(hdcDest);
  hMaskDC = CreateCompatibleDC(hdcDest);
  hOldImageBMP = (HBITMAP) SelectObject(hImageDC, hImageBMP);
  hOldMaskBMP = (HBITMAP) SelectObject(hMaskDC, hMaskBMP);

  if (nWidthDest == nWidthSrc && nHeightDest == nHeightSrc) {
    BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest,
        hdcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY);
  } else {
    StretchBlt(hImageDC, 0, 0, nWidthDest, nHeightDest,
        hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);
  }
  SetBkColor(hImageDC, crTransparent);
  BitBlt(hMaskDC, 0, 0, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCCOPY);
  SetBkColor(hImageDC, RGB(0,0,0));
  SetTextColor(hImageDC, RGB(255,255,255));
  BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);
  SetBkColor(hdcDest, RGB(255,255,255));
  SetTextColor(hdcDest, RGB(0,0,0));
  BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
      hMaskDC, 0, 0, SRCAND);
  BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
      hImageDC, 0, 0, SRCPAINT);

  SelectObject(hImageDC, hOldImageBMP);
  DeleteDC(hImageDC);
  SelectObject(hMaskDC, hOldMaskBMP);
  DeleteDC(hMaskDC);
  DeleteObject(hImageBMP);
  DeleteObject(hMaskBMP);
}


// 绘制透明图片
inline void DrawTransBmp(HDC hdc, HDC hdcTmp, int xx, int yy, HBITMAP bmp) {
  SelectObject(hdcTmp, bmp);
  TransparentBlt2(hdc, xx, yy, SQUARE_SIZE, SQUARE_SIZE,
	  hdcTmp, 0, 0, SQUARE_SIZE, SQUARE_SIZE, MASK_COLOR);
}

// 绘制棋盘
void CBlWnd::DrawBoard(HDC hdc) {
  int x, y, xx, yy, sq, pc;
  HDC hdcTmp;

  // 画棋盘
  hdcTmp = CreateCompatibleDC(hdc);
  SelectObject(hdcTmp, Xqwl.bmpBoard);
  BitBlt(hdc, 0, 0, BOARD_WIDTH, BOARD_HEIGHT, hdcTmp, 0, 0, SRCCOPY);
  // 画棋子
  for (x = FILE_LEFT; x <= FILE_RIGHT; x ++) {
    for (y = RANK_TOP; y <= RANK_BOTTOM; y ++) {
      if (Xqwl.bFlipped) {
        xx = BOARD_EDGE + (FILE_FLIP(x) - FILE_LEFT) * SQUARE_SIZE;
        yy = BOARD_EDGE + (RANK_FLIP(y) - RANK_TOP) * SQUARE_SIZE;
      } else {
        xx = BOARD_EDGE + (x - FILE_LEFT) * SQUARE_SIZE;
        yy = BOARD_EDGE + (y - RANK_TOP) * SQUARE_SIZE;
      }
      sq = COORD_XY(x, y);
      pc = pos.ucpcSquares[sq];
      if (pc != 0) {
        DrawTransBmp(hdc, hdcTmp, xx, yy, Xqwl.bmpPieces[pc]);
      }
      if (sq == Xqwl.sqSelected || sq == SRC(Xqwl.mvLast) || sq == DST(Xqwl.mvLast)) {
		  ::MessageBox(NULL,"xddbg","xdxd",IDOK);
        DrawTransBmp(hdc, hdcTmp, xx, yy, Xqwl.bmpSelected);
      }
    }
  }
  DeleteDC(hdcTmp);
}



int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{	
  MSG msg;
  WNDCLASSEX wce;

  // 初始化全局变量
  Xqwl.m_hInst = hInstance;
  Xqwl.bFlipped = FALSE;
  xdStartup();

  Xqwl.Init();
  
  // 设置窗口
  wce.cbSize = sizeof(WNDCLASSEX);
  wce.style = 0;
  wce.lpfnWndProc =   CBlWnd::WndProc;
  wce.cbClsExtra = wce.cbWndExtra = 0;
  wce.hInstance = hInstance;
  wce.hIcon = (HICON) LoadImage(hInstance, 
	  MAKEINTRESOURCE(IDI_BLXQ), IMAGE_ICON, 32, 32, LR_SHARED);
  wce.hCursor = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
  wce.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
  wce.lpszMenuName = MAKEINTRESOURCE(IDM_MAINMENU);
  wce.lpszClassName = "XQWLIGHT";
  wce.hIconSm = (HICON) LoadImage(hInstance, 
	  MAKEINTRESOURCE(IDI_BLXQ), IMAGE_ICON, 16, 16, LR_SHARED);
  RegisterClassEx(&wce);

  // 打开窗口
  Xqwl.hWnd = CreateWindow("XQWLIGHT", "象棋小巫师", WINDOW_STYLES,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
	  NULL, NULL, hInstance, NULL);
  if (Xqwl.hWnd == NULL) {
    return 0;
  }
  ShowWindow(Xqwl.hWnd, nCmdShow);
  UpdateWindow(Xqwl.hWnd);

  // 接收消息
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.wParam;
}

   

// 绘制格子
void CBlWnd::DrawSquare(int sq, BOOL bSelected ) {
  int sqFlipped, xx, yy, pc;

  sqFlipped = Xqwl.bFlipped ? SQUARE_FLIP(sq) : sq;
  xx = BOARD_EDGE + (FILE_X(sqFlipped) - FILE_LEFT) * SQUARE_SIZE;
  yy = BOARD_EDGE + (RANK_Y(sqFlipped) - RANK_TOP) * SQUARE_SIZE;
  SelectObject(Xqwl.m_hdcTmp, Xqwl.bmpBoard);
  BitBlt(Xqwl.m_hdc, xx, yy, SQUARE_SIZE, SQUARE_SIZE, Xqwl.m_hdcTmp, xx, yy, SRCCOPY);
  pc = pos.ucpcSquares[sq];
  if (pc != 0) {
    DrawTransBmp(Xqwl.m_hdc, Xqwl.m_hdcTmp, xx, yy, Xqwl.bmpPieces[pc]);
  }
  if (bSelected) {
    DrawTransBmp(Xqwl.m_hdc, Xqwl.m_hdcTmp, xx, yy, Xqwl.bmpSelected);
//	::MessageBox(NULL,"xddbg","xdxd2",IDOK);
  }
}
// 播放资源声音
inline void PlayResWav(int nResId) {
  PlaySound(MAKEINTRESOURCE(nResId), Xqwl.m_hInst, SND_ASYNC | SND_NOWAIT | SND_RESOURCE);
}

// "DrawSquare"参数
const BOOL DRAW_SELECTED = TRUE;


// 点击格子事件处理
void CBlWnd::ClickSquare(int sq) {
   //*
  Xqwl.m_hdc = GetDC(Xqwl.hWnd);
  Xqwl.m_hdcTmp = CreateCompatibleDC(Xqwl.m_hdc);
  sq = Xqwl.bFlipped ? SQUARE_FLIP(sq) : sq;
  int pc = pos.ucpcSquares[sq];

  if ((pc & SIDE_TAG(pos.sdPlayer)) != 0) {
    // 如果点击自己的子，那么直接选中该子
    if (Xqwl.sqSelected != 0) {
      DrawSquare(Xqwl.sqSelected);
    }
    Xqwl.sqSelected = sq;
    DrawSquare(sq, DRAW_SELECTED);
    if (Xqwl.mvLast != 0) {
      DrawSquare(SRC(Xqwl.mvLast));
      DrawSquare(DST(Xqwl.mvLast));
    }
    PlayResWav(IDR_CLICK); // 播放点击的声音

  } else if (Xqwl.sqSelected != 0) {
	  /*
    // 如果点击的不是自己的子，但有子选中了(一定是自己的子)，那么走这个子
    Xqwl.mvLast = MOVE(Xqwl.sqSelected, sq);
    pos.MakeMove(Xqwl.mvLast);
    DrawSquare(Xqwl.sqSelected, DRAW_SELECTED);
    DrawSquare(sq, DRAW_SELECTED);
    Xqwl.sqSelected = 0;
    PlayResWav(pc == 0 ? IDR_MOVE : IDR_CAPTURE); // 播放走子或吃子的声音
	*/
  }
  DeleteDC(Xqwl.m_hdcTmp);
  ReleaseDC(Xqwl.hWnd, Xqwl.m_hdc);
  //*/
}

 
LRESULT CALLBACK CBlWnd::WndProc(HWND hWnd, UINT uMsg, 
						 WPARAM wParam, 
						 LPARAM lParam)
{
  int x, y;
  HDC hdc;
  RECT rect;
  PAINTSTRUCT ps;
  MSGBOXPARAMS mbp;

  switch (uMsg) {
  // 新建窗口
  case WM_CREATE:
    // 调整窗口位置和尺寸
    GetWindowRect(hWnd, &rect);
    x = rect.left;
    y = rect.top;
    rect.right = rect.left + BOARD_WIDTH;
    rect.bottom = rect.top + BOARD_HEIGHT;
    AdjustWindowRect(&rect, WINDOW_STYLES, TRUE);
    MoveWindow(hWnd, x, y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
    break;
  // 退出
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  // 菜单命令
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDM_FILE_RED:
    case IDM_FILE_BLACK:
      Xqwl.bFlipped = (LOWORD(wParam) == IDM_FILE_BLACK);
      xdStartup();
      hdc = GetDC(Xqwl.hWnd);
      DrawBoard(hdc);
      ReleaseDC(Xqwl.hWnd, hdc);
      break;
    case IDM_FILE_EXIT:
      DestroyWindow(Xqwl.hWnd);
      break;
    case IDM_HELP_HOME:
//      ShellExecute(NULL, NULL, "http://www.xqbase.com/", NULL, NULL, SW_SHOWNORMAL);
      break;
    case IDM_HELP_ABOUT:
      // 弹出带象棋小巫师图标的对话框
      MessageBeep(MB_ICONINFORMATION);
      mbp.cbSize = sizeof(MSGBOXPARAMS);
      mbp.hwndOwner = hWnd;
      mbp.hInstance = Xqwl.m_hInst;
      mbp.lpszText = cszAbout;
      mbp.lpszCaption = "关于象棋小巫师";
      mbp.dwStyle = MB_USERICON;
      mbp.lpszIcon = MAKEINTRESOURCE(IDI_BLXQ);
      mbp.dwContextHelpId = 0;
      mbp.lpfnMsgBoxCallback = NULL;
      mbp.dwLanguageId = 0;
      MessageBoxIndirect(&mbp);
      break;
    }
    break;
  // 绘图
  case WM_PAINT:
    hdc = BeginPaint(Xqwl.hWnd, &ps);
    DrawBoard(hdc);
    EndPaint(Xqwl.hWnd, &ps);
    break;
  // 鼠标点击
  case WM_LBUTTONDOWN:
    x = FILE_LEFT + (LOWORD(lParam) - BOARD_EDGE) / SQUARE_SIZE;
    y = RANK_TOP + (HIWORD(lParam) - BOARD_EDGE) / SQUARE_SIZE;
    if (x >= FILE_LEFT && x <= FILE_RIGHT && y >= RANK_TOP && y <= RANK_BOTTOM) {
      ClickSquare(COORD_XY(x, y));
    }
    break;
  // 其他事件
  default:
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
  return FALSE;
}
 
void CBlWnd::prvTest()
{

}

void CBlWnd::Init()
{
	// 装入图片
  bmpBoard = LoadResBmp(IDB_BOARD);
  bmpSelected = LoadResBmp(IDB_SELECTED);
  for (int i = PIECE_KING; i <= PIECE_PAWN; i ++) {
    bmpPieces[SIDE_TAG(0) + i] = LoadResBmp(IDB_RK + i);
    bmpPieces[SIDE_TAG(1) + i] = LoadResBmp(IDB_BK + i);
  }


}
