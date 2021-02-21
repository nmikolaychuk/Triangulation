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
		xmin = 0, xmax = 1000,
		ymin = 0, ymax = 1000;
	double nXmin = 0, nXmax = 0,
		nYmin = 0, nYmax = 0;

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
	Points* globPts;
	std::vector<Delone> delonePts;
	int ptsSize;
	void DrawDots(Points pt[], int num_of_pts);
	void DrawTriangulation(std::vector<Delone> dlnVec, int num_of_delone_pts);
	void Triangulation(Points pt[]);
	afx_msg void OnBnClickedBtnTriang();
};
