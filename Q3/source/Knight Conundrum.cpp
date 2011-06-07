// Knight Conundrum.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Knight Conundrum.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND mainDlg;
HBITMAP hbmKnight_wbg;
HBITMAP hbmKnight_bbg;
HWND chessBoard[64];
int knightPos;
int numMoves = 0;
std::set<int> visitedSquares;
HANDLE destroyEvent;
bool tourStarted = false;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
BOOL CALLBACK		DialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI		KnightTour(LPVOID);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_KNIGHTCONUNDRUM, szWindowClass, MAX_LOADSTRING);
	//MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KNIGHTCONUNDRUM));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC) DialogProc);
   mainDlg = hWnd;

   if (!hWnd) {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: DialogProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
BOOL CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId, wmEvent;
	HWND hControl;
	RECT screenRect;
	POINT p;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) {
	case WM_INITDIALOG:
		{
		// Create the static controls that will make up the chessboard and store the handles
		// in an array to keep track of where on the chessboard each control is located
		int x = 40, y = 40;

		HWND hStatic;
		int cnt = 0;
		for (int i=0; i<8; i++) {
			for (int j=0; j<8; j++) {
				if ((cnt + i) % 2 == 0)
					hStatic = CreateWindow(L"Static", L"", WS_CHILD | WS_BORDER | SS_NOTIFY | SS_CENTER | SS_CENTERIMAGE | WS_VISIBLE, x, y, 
						38, 38, hWnd, (HMENU) IDC_WHITE_BOX, hInst, NULL);
				else 
					hStatic = CreateWindow(L"Static", L"", WS_CHILD | SS_NOTIFY | SS_CENTER | SS_CENTERIMAGE | WS_VISIBLE, x, y,
						38, 38, hWnd, (HMENU) IDC_BLACK_BOX, hInst, NULL);
				chessBoard[cnt] = hStatic;
				cnt++;
				x = x + 38;
			}
			y = y + 38;
			x = 40;
		}	

		// Load bitmap files for the knight
		hbmKnight_wbg  = (HBITMAP) LoadImage(hInst, L"blueKnight_wbg.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
		hbmKnight_bbg  = (HBITMAP) LoadImage(hInst, L"blueKnight_bbg.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
		
		// Create the static control that will display the knight and position it 
		// in the correct space on the chessboard
		GetWindowRect(chessBoard[57], (LPRECT) &screenRect);
		p.x = screenRect.left;
		p.y = screenRect.top;
		ScreenToClient(hWnd, (LPPOINT) &p);
		hControl = CreateWindow(L"Static", NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE, p.x + 1, p.y + 1,
			36, 36, hWnd, (HMENU) IDC_KNIGHT, hInst, NULL);
		SendMessage(hControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbmKnight_wbg);

		knightPos = 57;

		// Set the icon of the main window
		HICON my_icon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_KNIGHTCONUNDRUM), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE); 
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM) my_icon);

		destroyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		}
	
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		hControl = (HWND) lParam;

		// Parse the menu selections
		switch (wmId) {
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		}
		
		switch (wmEvent) {
		case STN_CLICKED:
			switch (wmId) {
			case IDC_START:
				tourStarted = true;
				EnableWindow(GetDlgItem(mainDlg, IDC_RESET), FALSE);
				EnableWindow(GetDlgItem(mainDlg, IDC_START), FALSE);
				CreateThread(NULL, 0, KnightTour, (LPVOID) knightPos, 0, NULL);
				break;
			
			case IDC_RESET:
				for (int i=0; i<64; i++) {				
					SetWindowText(chessBoard[i], L"");
					if (GetDlgCtrlID(chessBoard[knightPos]) == IDC_WHITE_BOX)
						SendMessage(GetDlgItem(mainDlg, IDC_KNIGHT), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbmKnight_wbg);
					else
						SendMessage(GetDlgItem(mainDlg, IDC_KNIGHT), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbmKnight_bbg);
				}
				visitedSquares.clear();
				numMoves = 0;
				SetWindowText(GetDlgItem(mainDlg, IDC_NUM_MOVES), L"");
				EnableWindow(GetDlgItem(mainDlg, IDC_START), TRUE);
				break;
			
			case IDC_WHITE_BOX:
				if (!tourStarted) {
					GetWindowRect(hControl, (LPRECT) &screenRect);
					int x = screenRect.left;
					int y = screenRect.top;
					p.x = screenRect.left;
					p.y = screenRect.top;
					ScreenToClient(mainDlg, (LPPOINT) &p);
		
					SetWindowPos(GetDlgItem(mainDlg, IDC_KNIGHT), HWND_TOP, p.x + 1, p.y + 1, 0, 0, SWP_NOSIZE);
					SendMessage(GetDlgItem(mainDlg, IDC_KNIGHT), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbmKnight_wbg);

					// Calculate new position on board
					GetWindowRect(chessBoard[knightPos], (LPRECT) &screenRect);
					int delta_i, delta_j, i, j;
					delta_j = (x - screenRect.left) / 38;
					delta_i = (y - screenRect.top) / 38;
					i = knightPos >> 3;
					j = knightPos - (i << 3); 

					knightPos = ((i + delta_i) << 3) + (j + delta_j);
				}
				break;

			case IDC_BLACK_BOX:
				if (!tourStarted) {
					GetWindowRect(hControl, (LPRECT) &screenRect);
					int x = screenRect.left;
					int y = screenRect.top;
					p.x = screenRect.left;
					p.y = screenRect.top;
					ScreenToClient(mainDlg, (LPPOINT) &p);
		
					SetWindowPos(GetDlgItem(mainDlg, IDC_KNIGHT), HWND_TOP, p.x + 1, p.y + 1, 0, 0, SWP_NOSIZE);
					SendMessage(GetDlgItem(mainDlg, IDC_KNIGHT), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbmKnight_bbg);

					// Calculate new position on board
					GetWindowRect(chessBoard[knightPos], (LPRECT) &screenRect);
					int delta_i, delta_j, i, j;
					delta_j = (x - screenRect.left) / 38;
					delta_i = (y - screenRect.top) / 38;
					i = knightPos >> 3;
					j = knightPos - (i << 3); 

					knightPos = ((i + delta_i) << 3) + (j + delta_j);

				}
				break;
			}
	
			break;
		}
		break;
	
	case WM_CTLCOLORSTATIC:
		hdc = (HDC) wParam;
		wmId = GetDlgCtrlID((HWND) lParam);
		switch (wmId)  {
		case IDC_WHITE_BOX: 
			SetDCBrushColor(hdc, RGB(210, 210, 210));
			SetBkColor(hdc, RGB(210, 210, 210));
			return (LRESULT)GetStockObject(DC_BRUSH);
		case IDC_BLACK_BOX:
			SetBkColor(hdc, RGB(0, 0, 0));
			SetTextColor(hdc, RGB(210, 210, 210));
			return (LRESULT)GetStockObject(BLACK_BRUSH);
		case IDC_STATIC:
			SetDCBrushColor(hdc, RGB(80, 138, 197));
			SetBkColor(hdc, RGB(80, 138, 197));
			return (LRESULT)GetStockObject(DC_BRUSH);
		}
		break;
	
	case WM_CTLCOLORDLG:
		{
		hdc = (HDC) wParam;
		SetDCBrushColor(hdc, RGB(80, 138, 197));
		return (LRESULT)GetStockObject(DC_BRUSH);
		break;
		}
	
	case WM_CLOSE:
		SetEvent(destroyEvent);
		DestroyWindow(hWnd);

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	
	default:
		return FALSE;
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

DWORD WINAPI KnightTour(LPVOID lParam) {
	int currentPos = (int) lParam;
	int i, j, validMoves[8], futureMoves[8];
	int min_cnt, cnt, i_min;
	RECT screenRect;
	POINT p;

	while (visitedSquares.size() != 63) {
		// Find possible moves
		i = currentPos >> 3;
		j = currentPos - (i << 3);

		validMoves[0] = (i+1>=0 && i+1<8 && j+2>=0 && j+2<8) ? ((i+1) << 3) + (j+2) : -1;
		validMoves[1] = (i+1>=0 && i+1<8 && j-2>=0 && j-2<8) ? ((i+1) << 3) + (j-2) : -1;
		validMoves[2] = (i-1>=0 && i-1<8 && j+2>=0 && j+2<8) ? ((i-1) << 3) + (j+2) : -1;
		validMoves[3] = (i-1>=0 && i-1<8 && j-2>=0 && j-2<8) ? ((i-1) << 3) + (j-2) : -1;
		validMoves[4] = (i+2>=0 && i+2<8 && j+1>=0 && j+1<8) ? ((i+2) << 3) + (j+1) : -1;
		validMoves[5] = (i+2>=0 && i+2<8 && j-1>=0 && j-1<8) ? ((i+2) << 3) + (j-1) : -1;
		validMoves[6] = (i-2>=0 && i-2<8 && j+1>=0 && j+1<8) ? ((i-2) << 3) + (j+1) : -1;
		validMoves[7] = (i-2>=0 && i-2<8 && j-1>=0 && j-1<8) ? ((i-2) << 3) + (j-1) : -1;

		// Go through each valid move and discover the number of moves possible from
		// that square on the chessboard
		min_cnt = 8;
		for (int x=0; x<8; x++) {
			// Invalidates a move if destination square has already been visited.
			if (visitedSquares.find(validMoves[x]) != visitedSquares.end())
				validMoves[x] = -1;
			
			if (validMoves[x] != -1) {
				i = validMoves[x] >> 3;
				j = validMoves[x] - (i << 3);
				futureMoves[0] = (i+1>=0 && i+1<8 && j+2>=0 && j+2<8) ? ((i+1) << 3) + (j+2) : -1;
				futureMoves[1] = (i+1>=0 && i+1<8 && j-2>=0 && j-2<8) ? ((i+1) << 3) + (j-2) : -1;
				futureMoves[2] = (i-1>=0 && i-1<8 && j+2>=0 && j+2<8) ? ((i-1) << 3) + (j+2) : -1;
				futureMoves[3] = (i-1>=0 && i-1<8 && j-2>=0 && j-2<8) ? ((i-1) << 3) + (j-2) : -1;
				futureMoves[4] = (i+2>=0 && i+2<8 && j+1>=0 && j+1<8) ? ((i+2) << 3) + (j+1) : -1;
				futureMoves[5] = (i+2>=0 && i+2<8 && j-1>=0 && j-1<8) ? ((i+2) << 3) + (j-1) : -1;
				futureMoves[6] = (i-2>=0 && i-2<8 && j+1>=0 && j+1<8) ? ((i-2) << 3) + (j+1) : -1;
				futureMoves[7] = (i-2>=0 && i-2<8 && j-1>=0 && j-1<8) ? ((i-2) << 3) + (j-1) : -1;

				cnt = 0;
				for (int y=0; y<8; y++) {
					if (visitedSquares.find(futureMoves[y]) != visitedSquares.end())
						futureMoves[y] = -1;
					if (futureMoves[y] != -1)
						cnt++;
				}
				if (cnt <= min_cnt) {
					min_cnt = cnt;
					i_min = x;
				}
			}

		}

		// Move knight to new position
		if (validMoves[i_min] == -1) {
			MessageBox(mainDlg, L"No 'minimum move' solution could be found for this starting square. This is most likely due my algorithm, though.", L"Oops", MB_OK);
			knightPos = currentPos;
			tourStarted = false;
			EnableWindow(GetDlgItem(mainDlg, IDC_RESET), TRUE);
			return 0;
		}

		GetWindowRect(chessBoard[validMoves[i_min]], (LPRECT) &screenRect);
		p.x = screenRect.left;
		p.y = screenRect.top;
		ScreenToClient(mainDlg, (LPPOINT) &p);
		
		SetWindowPos(GetDlgItem(mainDlg, IDC_KNIGHT), HWND_TOP, p.x + 1, p.y + 1, 0, 0, SWP_NOSIZE);
		switch (GetDlgCtrlID(chessBoard[validMoves[i_min]])) {
		case IDC_BLACK_BOX:
			SendMessage(GetDlgItem(mainDlg, IDC_KNIGHT), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbmKnight_bbg);
			break;
		case IDC_WHITE_BOX:
			SendMessage(GetDlgItem(mainDlg, IDC_KNIGHT), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbmKnight_wbg);
			break;
		}
		numMoves++;

		TCHAR pszDest[3];
		StringCchPrintf(pszDest, 3, L"%d", numMoves);
		SetWindowText(GetDlgItem(mainDlg, IDC_NUM_MOVES), pszDest);
		SetWindowText(chessBoard[currentPos], L"x");

		visitedSquares.insert(currentPos);
		//moveQueue.push(validMoves[i_min]);
		currentPos = validMoves[i_min];

		if (WaitForSingleObject(destroyEvent, 400) == WAIT_OBJECT_0) {
			CloseHandle(destroyEvent);
			return 0;
		}
	}
	
	knightPos = currentPos;
	tourStarted = false;
	EnableWindow(GetDlgItem(mainDlg, IDC_RESET), TRUE);
	
	return 0;
}
