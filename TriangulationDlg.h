#pragma once
#include <math.h>
#include <vector>

struct Points
{
	double x;
	double y;
};

struct Delone
{
	Points A;
	Points B;
	Points C;
	
	double radius;
	Points center;
};

// Диалоговое окно CTriangulationDlg
class CTriangulationDlg : public CDialogEx
{
// Создание
public:
	CTriangulationDlg(CWnd* pParent = nullptr);	// стандартный конструктор

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TRIANGULATION_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV

// Реализация
protected:
	HICON m_hIcon;

	// draw-conversion variables
	double xp = 0, yp = 0,
		xmin = 0, xmax = 10,
		ymin = 0, ymax = 10;
	double xminSuper = -5, xmaxSuper = 15,
		yminSuper = -5, ymaxSuper = 15;
	double nXmin = 0, nXmax = 0,
		nYmin = 0, nYmax = 0;

	double randMin = -xmax * 0.001;
	double randMax = xmax * 0.001*2;

	DWORD dwThread;
	HANDLE hThread;

	// draw-window
	CWnd* PicWnd;
	CDC* PicDc;
	CRect Pic;

	// draw-pen
	CPen dots_pen;
	CPen lines_pen;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnCalc();
	afx_msg void OnBnClickedBtnExit();
	
	std::vector<Points> global_points;
	std::vector<Delone> global_triangles;
	
	int ptsSize;
	double stepKoef;
	void Animation();
	void DrawTriangulation(std::vector<Points> pts, std::vector<Delone> dln);
	void Triangulation(std::vector<Points> pt, std::vector<Delone>& dln);
	void RecurrentTriangulation(std::vector<Points> vec, std::vector<Delone>& pts);
	void drawArea(std::vector<Points>& vec, bool mode);
	void drawSuperstructure(std::vector<Points>& vec);
	afx_msg void OnBnClickedBtnTriang();
};
