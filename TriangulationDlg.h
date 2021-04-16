#pragma once
#include <math.h>
#include <vector>

struct Points
{
	double x;
	double y;
	bool is_super_dot = false;
	bool is_rect_border = false;
	bool is_magnet_border_north = false;
	bool is_magnet_border_south = false;

	double potential = 0.0;
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
	double xp = 0, yp = 0;
	double xminSuper = 0, xmaxSuper = 100,
		yminSuper = 0, ymaxSuper = 100;
	double nXmin = 0, nXmax = 0,
		nYmin = 0, nYmax = 0;
	double pi = 3.1415926535;

	double randMin;
	double randMax;

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
	afx_msg void OnBnClickedBtnCalc();
	afx_msg void OnBnClickedBtnExit();
	DECLARE_MESSAGE_MAP()
public:
	
	std::vector<Points> polygon_hitbox;
	std::vector<Points> polygon;
	
	double stepKoef;
	void DrawTriangulation(std::vector<Points> pts, std::vector<Delone> dln);
	void Triangulation(std::vector<Points> pt, std::vector<Delone>& dln);
	void RecurrentTriangulation(std::vector<Points> vec, std::vector<Delone>& pts);
	void drawGrid(std::vector<Points>& vec, double rad_out, double rad_in, double cut_wdth, Points center);
	void drawSuperstructure(std::vector<Points>& vec);
	void deleteSuperDots(std::vector<Delone>& dln);
	void deleteFromMagnet(std::vector<Delone>& dln, std::vector<Points> polygon);
	void drawMagnet(std::vector<Points>& vec, Points center, double radius_inner,
		double radius_outer, double cut_width,
		double angle_north, double angle_south,
		std::vector<Points>& polygon, bool is_hitbox=false);
	void drawRectangle(std::vector<Points>& vec);
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	double centerMagnetX;
	double centerMagnetY;
	double magnet_inner_radius;
	double magnet_outer_radius;
	double magnet_cut_width;
	double magnet_angle_north;
	double magnet_angle_south;
	double potentialRect;
	double potentialMagnet;
	CButton graphPotentialValues;
};
