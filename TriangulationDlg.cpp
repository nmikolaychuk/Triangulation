#pragma once
#include "pch.h"
#include "framework.h"
#include "Triangulation.h"
#include "TriangulationDlg.h"
#include "afxdialogex.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DOTS(x,y) (xp*((x)-xminSuper)), (yp*((y)-ymaxSuper))
using namespace std;

CTriangulationDlg::CTriangulationDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TRIANGULATION_DIALOG, pParent)
	, stepKoef(4)
	, xmin(10)
	, xmax(90)
	, ymin(10)
	, ymax(90)
	, centerMagnetX(50)
	, centerMagnetY(50)
	, magnet_inner_radius(16)
	, magnet_outer_radius(22)
	, magnet_cut_width(45)
	, magnet_angle_north(90)
	, magnet_angle_south(90)
	, potentialRect(0)
	, potentialMagnet(5)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTriangulationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ED_PT_SIZE2, stepKoef);
	DDX_Text(pDX, IDC_RECT_XMIN, xmin);
	DDX_Text(pDX, IDC_RECT_XMAX, xmax);
	DDX_Text(pDX, IDC_RECT_YMIN, ymin);
	DDX_Text(pDX, IDC_RECT_YMAX, ymax);
	DDX_Text(pDX, IDC_MAGN_CENTER_X, centerMagnetX);
	DDX_Text(pDX, IDC_MAGN_CENTER_Y, centerMagnetY);
	DDX_Text(pDX, IDC_MAGNET_RADIUS_INNER, magnet_inner_radius);
	DDX_Text(pDX, IDC_MAGNET_RADIUS_OUTER, magnet_outer_radius);
	DDX_Text(pDX, IDC_MAGNET_CUT_WIDTH, magnet_cut_width);
	DDX_Text(pDX, IDC_MAGNET_ANGLE_NORTH, magnet_angle_north);
	DDX_Text(pDX, IDC_MAGNET_ANGLE_SOUTH, magnet_angle_south);
	DDX_Text(pDX, IDC_POTENTIAL_RECT, potentialRect);
	DDX_Text(pDX, IDC_POTENTIAL_MAGNET, potentialMagnet);
	DDX_Control(pDX, IDC_CHECK_GRAPH_POTENTIAL, graphPotentialValues);
	DDX_Control(pDX, IDC_CHECK_GRAPH_EQUIPOTENTIAL, graphEquipotential);
	DDX_Control(pDX, IDC_CHECK_GRAPH_POWER, graphPower);
}

BEGIN_MESSAGE_MAP(CTriangulationDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CALC, &CTriangulationDlg::OnBnClickedBtnCalc)
	ON_BN_CLICKED(IDC_BTN_EXIT, &CTriangulationDlg::OnBnClickedBtnExit)
	ON_BN_CLICKED(IDC_CHECK_GRAPH_POTENTIAL, &CTriangulationDlg::OnBnClickedCheckGraphPotential)
	ON_BN_CLICKED(IDC_CHECK_GRAPH_EQUIPOTENTIAL, &CTriangulationDlg::OnBnClickedCheckGraphEquipotential)
	ON_BN_CLICKED(IDC_CHECK_GRAPH_POWER, &CTriangulationDlg::OnBnClickedCheckGraphPower)
END_MESSAGE_MAP()


// Обработчики сообщений CTriangulationDlg

BOOL CTriangulationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	srand(time(0));

	PicWnd = GetDlgItem(IDC_DRAW_AREA);
	PicDc = PicWnd->GetDC();
	PicWnd->GetClientRect(&Pic);

	dots_pen.CreatePen(
		PS_SOLID,
		-1,
		RGB(240, 240, 240)
	);

	lines_pen.CreatePen(
		PS_SOLID,
		-1,
		RGB(200, 200, 200)
	);

	randMin = -xmax * 0.0007;
	randMax = xmax * 0.0007 * 2;

	graphPotentialValues.SetCheck(BST_UNCHECKED);

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CTriangulationDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CTriangulationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTriangulationDlg::DrawTriangulation(vector<Points> pts, vector<Delone> dln,
	vector<vector<Points>> equipot, vector<vector<Points>> power)
{
	// создание контекста устройства
	CBitmap bmp;
	CDC* MemDc;
	MemDc = new CDC;
	MemDc->CreateCompatibleDC(PicDc);

	double window_signal_width = Pic.Width();
	double window_signal_height = Pic.Height();
	xp = (window_signal_width / (xmaxSuper - xminSuper));			//Коэффициенты пересчёта координат по Х
	yp = -(window_signal_height / (ymaxSuper - yminSuper));			//Коэффициенты пересчёта координат по У

	bmp.CreateCompatibleBitmap(PicDc, window_signal_width, window_signal_height);
	CBitmap* pBmp = (CBitmap*)MemDc->SelectObject(&bmp);
	// заливка фона графика белым цветом
	MemDc->FillSolidRect(Pic, RGB(240, 240, 240));

	double differ_x = 0.0035 * xmax;
	double differ_y = 0.0035 * ymax;

	for (int i = 0; i < dln.size(); ++i)
	{
		MemDc->SelectObject(&lines_pen);
		MemDc->MoveTo(DOTS(dln[i].A.x, dln[i].A.y));
		MemDc->LineTo(DOTS(dln[i].B.x, dln[i].B.y));
		MemDc->LineTo(DOTS(dln[i].C.x, dln[i].C.y));
		MemDc->LineTo(DOTS(dln[i].A.x, dln[i].A.y));
	}

	//MemDc->SelectObject(&dots_pen);
	for (int i = 0; i < pts.size(); i++)
	{
		// Граничные точки rect-области
		if (pts[i].is_rect_border)
		{
			CPen circle_pen;
			circle_pen.CreatePen(
				PS_SOLID,
				-1,
				RGB(250, 170, 0));

			CBrush circle_brush(RGB(250, 200, 0));
			MemDc->SelectObject(&circle_pen);
			MemDc->SelectObject(&circle_brush);
			MemDc->Ellipse(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), DOTS(pts[i].x + differ_x, pts[i].y + differ_y));

			if (graphPotentialValues.GetCheck() == BST_CHECKED)
			{
				CString str;
				str.Format(_T("%.1f"), pts[i].potential);
				MemDc->SetBkMode(0);
				MemDc->TextOutW(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), str);
			}
		}

		// Граничные точки magnet-области (south)
		if (pts[i].is_magnet_border_south)
		{
			CPen circle_pen;
			circle_pen.CreatePen(
				PS_SOLID,
				-1,
				RGB(150, 20, 20));

			CBrush circle_brush(RGB(150, 20, 20));
			MemDc->SelectObject(&circle_pen);
			MemDc->SelectObject(&circle_brush);
			MemDc->Ellipse(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), DOTS(pts[i].x + differ_x, pts[i].y + differ_y));

			if (graphPotentialValues.GetCheck() == BST_CHECKED && i % 5 == 0)
			{
				CString str;
				str.Format(_T("%.1f"), pts[i].potential);
				MemDc->SetBkMode(0);
				MemDc->TextOutW(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), str);
			}
		}

		// Граничные точки magnet-области (north)
		if (pts[i].is_magnet_border_north)
		{
			CPen circle_pen;
			circle_pen.CreatePen(
				PS_SOLID,
				-1,
				RGB(20, 20, 150));

			CBrush circle_brush(RGB(20, 20, 150));
			MemDc->SelectObject(&circle_pen);
			MemDc->SelectObject(&circle_brush);
			MemDc->Ellipse(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), DOTS(pts[i].x + differ_x, pts[i].y + differ_y));

			if (graphPotentialValues.GetCheck() == BST_CHECKED && i % 5 == 0)
			{
				CString str;
				str.Format(_T("%.1f"), pts[i].potential);
				MemDc->SetBkMode(0);
				MemDc->TextOutW(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), str);
			}
		}

		// Остальные точки
		if (!pts[i].is_magnet_border_north
			&& !pts[i].is_magnet_border_south
			&& !pts[i].is_rect_border)
		{
			CPen circle_pen;
			circle_pen.CreatePen(
				PS_SOLID,
				-1,
				RGB(100, 134, 95));

			CBrush circle_brush(RGB(100, 134, 95));
			MemDc->SelectObject(&circle_pen);
			MemDc->SelectObject(&circle_brush);
			MemDc->Ellipse(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), DOTS(pts[i].x + differ_x, pts[i].y + differ_y));

			if (graphPotentialValues.GetCheck() == BST_CHECKED)
			{
				CString str;
				str.Format(_T("%.1f"), pts[i].potential);
				MemDc->SetBkMode(0);
				MemDc->TextOutW(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), str);
			}
		}
	}

	if (!equipot.empty())
	{
		for (int i = 0; i < equipot.size(); i++)
		{
			CPen circle_pen;
			circle_pen.CreatePen(
				PS_SOLID,
				2,
				RGB(20, 20, 120));

			CBrush circle_brush(RGB(20, 20, 120));
			MemDc->SelectObject(&circle_pen);
			MemDc->SelectObject(&circle_brush);

			MemDc->MoveTo(DOTS(equipot[i][0].x, equipot[i][0].y));
			for (int j = 0; j < equipot[i].size(); j++)
			{
				MemDc->LineTo(DOTS(equipot[i][j].x, equipot[i][j].y));
			}
		}
	}

	if (!power.empty())
	{
		for (int i = 0; i < power.size(); i++)
		{
			CPen circle_pen;
			circle_pen.CreatePen(
				PS_SOLID,
				2,
				RGB(120, 20, 20));

			CBrush circle_brush(RGB(120, 20, 20));
			MemDc->SelectObject(&circle_pen);
			MemDc->SelectObject(&circle_brush);

			MemDc->MoveTo(DOTS(power[i][0].x, power[i][0].y));
			for (int j = 0; j < power[i].size(); j++)
			{
				MemDc->LineTo(DOTS(power[i][j].x, power[i][j].y));
			}
		}
	}

	// вывод на экран
	PicDc->BitBlt(0, 0, window_signal_width, window_signal_height, MemDc, 0, 0, SRCCOPY);
	delete MemDc;
}

double OuterCircleRadius(Points pt1, Points pt2, Points pt3, Points& center)
{
	center.x = ((pt2.x * pt2.x - pt1.x * pt1.x + pt2.y * pt2.y - pt1.y * pt1.y)
		* (pt3.y - pt1.y) - (pt3.x * pt3.x - pt1.x * pt1.x + pt3.y * pt3.y
			- pt1.y * pt1.y) * (pt2.y - pt1.y))
		/ ((pt2.x - pt1.x) * (pt3.y - pt1.y) - (pt3.x - pt1.x) * (pt2.y - pt1.y));
	center.x /= 2.;

	center.y = ((pt3.x * pt3.x - pt1.x * pt1.x + pt3.y * pt3.y - pt1.y * pt1.y)
		* (pt2.x - pt1.x) - (pt2.x * pt2.x - pt1.x * pt1.x + pt2.y * pt2.y
			- pt1.y * pt1.y) * (pt3.x - pt1.x))
		/ ((pt2.x - pt1.x) * (pt3.y - pt1.y) - (pt3.x - pt1.x) * (pt2.y - pt1.y));
	center.y /= 2.;

	return sqrt((center.x - pt1.x) * (center.x - pt1.x) + (center.y - pt1.y) * (center.y - pt1.y));
}

void CTriangulationDlg::Triangulation(vector<Points> pt, vector<Delone>& dln)
{
	for (int i = 0; i < pt.size() - 2; i++)
	{
		for (int j = i + 1; j < pt.size() - 1; j++)
		{
			for (int k = j + 1; k < pt.size(); k++)
			{
				Points center;
				center.x = 0.;
				center.y = 0.;
				double radius = OuterCircleRadius(pt[i], pt[j], pt[k], center);
				bool isDelone = true;
				int counterDelone = 0;

				for (int m = 0; m < pt.size(); m++)
				{
					if (m != i && m != j && m != k)
					{
						double condition = (pt[m].x - center.x) * (pt[m].x - center.x) + (pt[m].y - center.y) * (pt[m].y - center.y);
						if (condition <= radius * radius)
						{
							isDelone = false;
							break;
						}
					}
				}

				if (isDelone)
				{
					Delone triangle;
					triangle.A = pt[i];
					triangle.B = pt[j];
					triangle.C = pt[k];
					triangle.radius = radius;
					triangle.center = center;
					dln.push_back(triangle);
				}
			}
		}
	}
}

bool pnpoly(int size, vector<Points> pts, double x, double y)
{
	bool c = false;
	for (int i = 0, j = size - 1; i < size; j = i++)
	{
		if (((pts[i].y < pts[j].y) && (pts[i].y <= y) && (y <= pts[j].y)
			&& ((pts[j].y - pts[i].y) * (x - pts[i].x) > (pts[j].x - pts[i].x) * (y - pts[i].y)))
			|| ((pts[i].y > pts[j].y) && (pts[j].y < y) && (y < pts[i].y)
				&& ((pts[j].y - pts[i].y) * (x - pts[i].x) < (pts[j].x - pts[i].x) * (y - pts[i].y))))
			c = !c;
	}
	return c;
}

void CTriangulationDlg::drawMagnet(vector<Points>& vec, Points center, double radius_inner,
	double radius_outer, double cut_width,
	double angle_north, double angle_south,
	vector<Points>& polygon, bool is_hitbox)
{
	UpdateData(TRUE);
	polygon.clear();
	double cut_angle = (180 * cut_width) / (pi * radius_inner);
	double rand_x, rand_y;

	double magnet_contour_step = stepKoef * 2.5;

	Points top_in, top_out, bottom_in, bottom_out;

	rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
	rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;
	top_in.x = radius_inner * cos((pi * (cut_angle / 2.)) / 180) + center.x + rand_x;
	top_in.y = radius_inner * sin((pi * (cut_angle / 2.)) / 180) + center.y + rand_y;

	rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
	rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;
	bottom_in.x = radius_inner * cos((pi * (360 - cut_angle / 2.)) / 180) + center.x + rand_x;
	bottom_in.y = radius_inner * sin((pi * (360 - cut_angle / 2.)) / 180) + center.y + rand_y;

	// Проверка углов среза (верхняя точка)
	double limit_angle_top = 0.;
	double limit_angle_bot = 0.;

	if (cut_angle < 180)
	{
		for (double i = 0; i <= 180; i += magnet_contour_step)
		{
			Points top;
			int counter = 0;
			for (double j = 0.25 * stepKoef; j < radius_inner; j += magnet_contour_step)
			{
				top.x = j * cos((pi * i) / 180) + top_in.x;
				top.y = j * sin((pi * i) / 180) + top_in.y;
				double condition = (top.x - center.x) * (top.x - center.x) + (top.y - center.y) * (top.y - center.y);

				if (condition <= radius_inner * radius_inner)
				{
					counter++;
				}
			}

			if (counter > 0)
			{
				limit_angle_top = i - magnet_contour_step;
				break;
			}
		}

		// Проверка углов среза (нижняя точка)
		for (double i = 360; i >= 180; i -= magnet_contour_step)
		{
			Points bot;
			int counter = 0;
			for (double j = 0.25 * magnet_contour_step; j < radius_inner; j += magnet_contour_step)
			{
				bot.x = j * cos((pi * i) / 180) + bottom_in.x;
				bot.y = j * sin((pi * i) / 180) + bottom_in.y;
				double condition = (bot.x - center.x) * (bot.x - center.x) + (bot.y - center.y) * (bot.y - center.y);

				if (condition <= radius_inner * radius_inner)
				{
					counter++;
				}
			}

			if (counter > 0)
			{
				limit_angle_bot = 360 - i + magnet_contour_step;
				break;
			}
		}

		if (angle_north > limit_angle_top)
		{
			angle_north = limit_angle_top;
			magnet_angle_north = limit_angle_top;
			UpdateData(FALSE);
		}

		if (angle_south > limit_angle_bot)
		{
			angle_south = limit_angle_bot;
			magnet_angle_south = limit_angle_bot;
			UpdateData(FALSE);
		}
	}
	else
	{
		for (double i = 90; i <= 270; i += magnet_contour_step)
		{
			Points top;
			int counter = 0;
			for (double j = 0.25 * magnet_contour_step; j < radius_inner; j += magnet_contour_step)
			{
				top.x = j * cos((pi * i) / 180) + top_in.x;
				top.y = j * sin((pi * i) / 180) + top_in.y;
				double condition = (top.x - center.x) * (top.x - center.x) + (top.y - center.y) * (top.y - center.y);

				if (condition <= radius_inner * radius_inner)
				{
					counter++;
				}
			}

			if (counter > 0)
			{
				limit_angle_top = i - magnet_contour_step;
				break;
			}
		}

		// Проверка углов среза (нижняя точка)
		for (double i = 270; i >= 90; i -= magnet_contour_step)
		{
			Points bot;
			int counter = 0;
			for (double j = 0.25 * magnet_contour_step; j < radius_inner; j += magnet_contour_step)
			{
				bot.x = j * cos((pi * i) / 180) + bottom_in.x;
				bot.y = j * sin((pi * i) / 180) + bottom_in.y;
				double condition = (bot.x - center.x) * (bot.x - center.x) + (bot.y - center.y) * (bot.y - center.y);

				if (condition <= radius_inner * radius_inner)
				{
					counter++;
				}
			}

			if (counter > 0)
			{
				limit_angle_bot = 360 - i + magnet_contour_step;
				break;
			}
		}

		if (angle_north > limit_angle_top)
		{
			angle_north = limit_angle_top;
			magnet_angle_north = limit_angle_top;
			UpdateData(FALSE);
		}

		if (angle_south > limit_angle_bot)
		{
			angle_south = limit_angle_bot;
			magnet_angle_south = limit_angle_bot;
			UpdateData(FALSE);
		}
	}

	// Расчет крайних точек на внешней окружности (верхняя точка)
	double side_top = radius_outer - radius_inner;
	rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
	rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;
	while (1)
	{
		top_out.x = side_top * cos((pi * angle_north) / 180) + top_in.x + rand_x;
		top_out.y = side_top * sin((pi * angle_north) / 180) + top_in.y + rand_y;
		double condition = (top_out.x - center.x) * (top_out.x - center.x) + (top_out.y - center.y) * (top_out.y - center.y);

		if (condition > radius_outer * radius_outer)
		{
			break;
		}
		side_top += 0.01;
	}
	double phi_top = (atan2((top_out.y - center.y), (top_out.x - center.x)) * 180) / pi;

	// Расчет крайних точек на внешней окружности (нижняя точка)
	double side_bot = radius_outer - radius_inner;
	rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
	rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;
	while (1)
	{
		bottom_out.x = side_bot * cos((pi * (360 - angle_south)) / 180) + bottom_in.x + rand_x;
		bottom_out.y = side_bot * sin((pi * (360 - angle_south)) / 180) + bottom_in.y + rand_y;
		double condition = (bottom_out.x - center.x) * (bottom_out.x - center.x) + (bottom_out.y - center.y) * (bottom_out.y - center.y);

		if (condition > radius_outer * radius_outer)
		{
			break;
		}
		side_bot += 0.01;
	}
	double phi_bot = 360 - (atan2((center.y - bottom_out.y), (bottom_out.x - center.x)) * 180) / pi;

	// Линия между радиусами (верхняя точка)
	if (cut_angle > 0 || angle_north > 0)
	{
		for (double j = 0.25 * magnet_contour_step; j < side_top; j += 0.25 * magnet_contour_step)
		{
			rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
			rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;

			Points dot_border_top, dot_border_bottom;
			dot_border_top.x = j * cos((pi * angle_north) / 180) + top_in.x + rand_x;
			dot_border_top.y = j * sin((pi * angle_north) / 180) + top_in.y + rand_y;
			dot_border_top.is_magnet_border_south = true;
			dot_border_top.potential = potentialMagnet;

			polygon.push_back(dot_border_top);
			if (!is_hitbox)
			{
				vec.push_back(dot_border_top);
			}
		}
	}

	for (double i = 0; i < 360; i += magnet_contour_step)
	{
		// Построение внешней окружности
		if ((i > phi_top) && (i < phi_bot))
		{
			rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
			rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;
			Points dot_outer;
			dot_outer.x = radius_outer * cos((pi * i) / 180) + center.x + rand_x;
			dot_outer.y = radius_outer * sin((pi * i) / 180) + center.y + rand_y;

			if (i < 180)
			{
				dot_outer.is_magnet_border_south = true;
				dot_outer.potential = potentialMagnet;
			}
			else if (i >= 180)
			{
				dot_outer.is_magnet_border_north = true;
				dot_outer.potential = -potentialMagnet;
			}

			polygon.push_back(dot_outer);
			if (!is_hitbox)
			{
				vec.push_back(dot_outer);
			}
		}
	}

	// Линия между радиусами (нижняя точка)
	vector<Points> buf;
	if (cut_angle > 0 || angle_south > 0)
	{
		for (double j = 0.25 * magnet_contour_step; j < side_bot; j += 0.25 * magnet_contour_step)
		{
			rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
			rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;

			Points dot_border_bottom;
			dot_border_bottom.x = j * cos((pi * (360 - angle_south)) / 180) + bottom_in.x + rand_x;
			dot_border_bottom.y = j * sin((pi * (360 - angle_south)) / 180) + bottom_in.y + rand_y;
			dot_border_bottom.is_magnet_border_north = true;
			dot_border_bottom.potential = -potentialMagnet;

			buf.push_back(dot_border_bottom);
			if (!is_hitbox)
			{
				vec.push_back(dot_border_bottom);
			}
		}
	}
	reverse(buf.begin(), buf.end());

	for (int i = 0; i < buf.size(); i++)
	{
		polygon.push_back(buf[i]);
	}
	buf.clear();

	for (double i = 0; i < 360; i += magnet_contour_step)
	{
		// Построение внутренней окружности
		if ((i >= cut_angle / 2.) && (i <= (360 - cut_angle / 2.)))
		{
			rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
			rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;

			Points dot_inner;
			dot_inner.x = radius_inner * cos((pi * i) / 180) + center.x + rand_x;
			dot_inner.y = radius_inner * sin((pi * i) / 180) + center.y + rand_y;

			if (i < 180)
			{
				dot_inner.is_magnet_border_south = true;
				dot_inner.potential = potentialMagnet;
			}
			else if (i >= 180)
			{
				dot_inner.is_magnet_border_north = true;
				dot_inner.potential = -potentialMagnet;
			}

			buf.push_back(dot_inner);
			if (!is_hitbox)
			{
				vec.push_back(dot_inner);
			}
		}
	}
	reverse(buf.begin(), buf.end());

	for (int i = 0; i < buf.size(); i++)
	{
		polygon.push_back(buf[i]);
	}

	buf.clear();
}

void CTriangulationDlg::drawRectangle(vector<Points>& vec)
{
	UpdateData(TRUE);
	Points pt_lt, pt_lb, pt_rt, pt_rb, pt_c;

	double rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
	pt_lt.x = xmin + rand_coord;
	rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
	pt_lt.y = ymin + rand_coord;
	pt_lt.is_rect_border = true;
	pt_lt.potential = potentialRect;

	rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
	pt_lb.x = xmin + rand_coord;
	rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
	pt_lb.y = ymax + rand_coord;
	pt_lb.is_rect_border = true;
	pt_lb.potential = potentialRect;

	rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
	pt_rt.x = xmax + rand_coord;
	rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
	pt_rt.y = ymin + rand_coord;
	pt_rt.is_rect_border = true;
	pt_rt.potential = potentialRect;

	rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
	pt_rb.x = xmax + rand_coord;
	rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
	pt_rb.y = ymax + rand_coord;
	pt_rb.is_rect_border = true;
	pt_rb.potential = potentialRect;

	vec.push_back(pt_lt);
	vec.push_back(pt_lb);
	vec.push_back(pt_rt);
	vec.push_back(pt_rb);
}

void CTriangulationDlg::drawGrid(vector<Points>& vec, double rad_out,
	double rad_in, double cut_wdth, Points center)
{
	double cut_angle = (180 * cut_wdth) / (pi * rad_in);
	double rand_x, rand_y;

	Points top_in, bottom_in;

	rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
	rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;
	top_in.x = rad_in * cos((pi * (cut_angle / 2.)) / 180) + center.x + rand_x;
	top_in.y = rad_in * sin((pi * (cut_angle / 2.)) / 180) + center.y + rand_y;

	rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
	rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;
	bottom_in.x = rad_in * cos((pi * (360 - cut_angle / 2.)) / 180) + center.x + rand_x;
	bottom_in.y = rad_in * sin((pi * (360 - cut_angle / 2.)) / 180) + center.y + rand_y;

	for (double i = xmin; i <= xmax; i += stepKoef)
	{
		for (double j = ymin; j <= ymax; j += stepKoef)
		{
			if ((i != xmin && i != xmax) || (j != ymin && j != ymax))
			{
				if (!polygon_hitbox.empty())
				{
					bool is_inside = pnpoly(polygon_hitbox.size(), polygon_hitbox, i, j);
					if (!is_inside)
					{
						if ((i == xmin || i == xmax) || (j == ymin || j == ymax))
						{
							rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
							rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;

							Points pt;
							pt.x = i + rand_x;
							pt.y = j + rand_y;
							pt.is_rect_border = true;
							pt.potential = potentialRect;
							vec.push_back(pt);
						}
						else
						{
							rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
							rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;

							Points pt;
							pt.x = i + rand_x;
							pt.y = j + rand_y;
							vec.push_back(pt);
						}
					}
				}
			}
		}
	}
}

void CTriangulationDlg::drawSuperstructure(vector<Points>& vec)
{
	// точки сверхструктуры
	Points lt, lb, rt, rb;
	double ratio = 0.01;

	lt.x = xminSuper + ratio * xmaxSuper;
	lt.y = yminSuper + ratio * ymaxSuper;
	lt.is_super_dot = true;

	lb.x = xminSuper + ratio * xmaxSuper;
	lb.y = ymaxSuper - ratio * ymaxSuper;
	lb.is_super_dot = true;

	rt.x = xmaxSuper - ratio * xmaxSuper;
	rt.y = yminSuper + ratio * ymaxSuper;
	rt.is_super_dot = true;

	rb.x = xmaxSuper - ratio * xmaxSuper;
	rb.y = ymaxSuper - ratio * ymaxSuper;
	rb.is_super_dot = true;
	vec.push_back(lt);
	vec.push_back(lb);
	vec.push_back(rt);
	vec.push_back(rb);
}

void CTriangulationDlg::deleteSuperDots(vector<Delone>& dln)
{
	for (int i = 0; i < dln.size(); i++)
	{
		if (dln[i].A.is_super_dot || dln[i].B.is_super_dot || dln[i].C.is_super_dot)
		{
			dln.erase(dln.begin() + i);
			i--;
		}
	}
}

void CTriangulationDlg::deleteFromMagnet(vector<Delone>& dln, vector<Points> polygon)
{
	double eps = 1e-5;
	if (!polygon.empty())
	{
		for (int i = 0; i < dln.size(); i++)
		{
			bool isDeleteA = false;
			bool isDeleteB = false;
			bool isDeleteC = false;
			for (int j = 0; j < polygon.size(); j++)
			{
				if (abs(dln[i].A.x - polygon[j].x) < eps && abs(dln[i].A.y - polygon[j].y) < eps)
				{
					isDeleteA = true;
				}

				if (abs(dln[i].B.x - polygon[j].x) < eps && abs(dln[i].B.y - polygon[j].y) < eps)
				{
					isDeleteB = true;
				}

				if (abs(dln[i].C.x - polygon[j].x) < eps && abs(dln[i].C.y - polygon[j].y) < eps)
				{
					isDeleteC = true;
				}
			}

			if (isDeleteA && isDeleteB && isDeleteC)
			{
				dln.erase(dln.begin() + i);
				i--;
			}
		}
	}
}

void CTriangulationDlg::RecurrentTriangulation(vector<Points> vec, vector<Delone>& dln)
{
	vector<Points> deletedPts;
	for (int i = 0; i < dln.size(); i++)
	{
		double condition = (vec.back().x - dln[i].center.x) * (vec.back().x - dln[i].center.x)
			+ (vec.back().y - dln[i].center.y) * (vec.back().y - dln[i].center.y);
		if (condition <= dln[i].radius * dln[i].radius)
		{
			deletedPts.push_back(dln[i].A);
			deletedPts.push_back(dln[i].B);
			deletedPts.push_back(dln[i].C);

			dln.erase(dln.begin() + i);
			i--;
		}
	}

	for (int g = 0; g < deletedPts.size(); g++)
	{
		for (int j = 0; j < deletedPts.size(); j++)
		{
			if (g != j)
			{
				if (deletedPts[g].x == deletedPts[j].x &&
					deletedPts[g].y == deletedPts[j].y)
				{
					deletedPts.erase(deletedPts.begin() + j);
					j--;
				}
			}
		}
	}

	deletedPts.push_back(vec.back());

	vector<Delone> deletedTriag;
	Triangulation(deletedPts, deletedTriag);

	for (int i = 0; i < deletedTriag.size(); i++)
	{
		if ((deletedTriag[i].A.x == vec.back().x && deletedTriag[i].A.y == vec.back().y)
			|| (deletedTriag[i].B.x == vec.back().x && deletedTriag[i].B.y == vec.back().y)
			|| (deletedTriag[i].C.x == vec.back().x && deletedTriag[i].C.y == vec.back().y))
		{
			dln.push_back(deletedTriag[i]);
		}
	}

	deletedTriag.clear();
	deletedPts.clear();
}

vector<Points> CTriangulationDlg::notBorderDots(vector<Points> pts)
{
	vector<Points> notBorder;
	for (int i = 0; i < pts.size(); i++)
	{
		// Условие, что i-ая точка - не граничная
		if (!pts[i].is_magnet_border_north && !pts[i].is_magnet_border_south && !pts[i].is_rect_border)
		{
			notBorder.push_back(pts[i]);
		}
	}
	return notBorder;
}

vector<Points> CTriangulationDlg::borderDots(vector<Points> pts)
{
	vector<Points> border;
	for (int i = 0; i < pts.size(); i++)
	{
		// Условие, что i-ая точка - граничная
		if (pts[i].is_magnet_border_north || pts[i].is_magnet_border_south || pts[i].is_rect_border)
		{
			border.push_back(pts[i]);
		}
	}
	return border;
}

vector<Delone> CTriangulationDlg::isDotsNeighbours(Points pt, vector<Delone> dln)
{
	double eps = 1e-6;
	vector<Delone> neighbours;

	// Запоминаем все треугольники, в которые входит заданная точка
	for (int i = 0; i < dln.size(); i++)
	{
		if (abs(dln[i].A.x - pt.x) <= eps && abs(dln[i].A.y - pt.y) <= eps
			&& !dln[i].B.is_magnet_border_north && !dln[i].B.is_magnet_border_south
			&& !dln[i].B.is_rect_border)
		{
			neighbours.push_back(dln[i]);
		}

		if (abs(dln[i].B.x - pt.x) <= eps && abs(dln[i].B.y - pt.y) <= eps
			&& !dln[i].B.is_magnet_border_north && !dln[i].B.is_magnet_border_south
			&& !dln[i].B.is_rect_border)
		{
			neighbours.push_back(dln[i]);
		}

		if (abs(dln[i].C.x - pt.x) <= eps && abs(dln[i].C.y - pt.y) <= eps
			&& !dln[i].B.is_magnet_border_north && !dln[i].B.is_magnet_border_south
			&& !dln[i].B.is_rect_border)
		{
			neighbours.push_back(dln[i]);
		}
	}
	return neighbours;
}

vector<Delone> CTriangulationDlg::isIJNeighbours(Points i, Points j, vector<Delone> dln)
{
	vector<Delone> neigh_i = isDotsNeighbours(i, dln);

	if (!neigh_i.empty())
	{
		vector<Delone> neigh_ij_triangles;
		for (int c = 0; c < neigh_i.size(); c++)
		{
			if ((neigh_i[c].A.x == j.x && neigh_i[c].A.y == j.y)
				|| (neigh_i[c].B.x == j.x && neigh_i[c].B.y == j.y)
				|| (neigh_i[c].C.x == j.x && neigh_i[c].C.y == j.y))
			{
				neigh_ij_triangles.push_back(neigh_i[c]);
			}
		}
		return neigh_ij_triangles;
	}
}

void CTriangulationDlg::calcABForTriang(double& A, double& B, Delone triang)
{
	A = triang.A.y * (triang.B.z - triang.C.z) + triang.B.y * (triang.C.z - triang.A.z) + triang.C.y * (triang.A.z - triang.B.z);
	B = triang.A.z * (triang.B.x - triang.C.x) + triang.B.z * (triang.C.x - triang.A.x) + triang.C.z * (triang.A.x - triang.B.x);

	/*A = (triang.B.y - triang.A.y) * (triang.C.z - triang.A.z) - (triang.C.y - triang.A.y) * (triang.B.z - triang.A.z);
	B = (triang.C.x - triang.A.x) * (triang.B.z - triang.A.z) - (triang.B.x - triang.A.x) * (triang.C.z - triang.A.z);*/
}

Delone CTriangulationDlg::replaceZInTriangle(Points pt, Delone triang, double z_value)
{
	Delone triang_out;
	if (triang.A.x == pt.x && triang.A.y == pt.y)
	{
		triang.A.z = z_value;
	}

	if (triang.B.x == pt.x && triang.B.y == pt.y)
	{
		triang.B.z = z_value;
	}

	if (triang.C.x == pt.x && triang.C.y == pt.y)
	{
		triang.C.z = z_value;
	}
	triang_out = triang;
	return triang_out;
}

void CTriangulationDlg::deleteFromPoints(vector<Delone> dln, vector<Points>& pts)
{
	bool isTrue = false;
	for (int i = 0; i < pts.size(); i++)
	{
		isTrue = false;
		for (int j = 0; j < dln.size(); j++)
		{
			if ((dln[j].A.x == pts[i].x && dln[j].A.y == pts[i].y)
				|| (dln[j].B.x == pts[i].x && dln[j].B.y == pts[i].y)
				|| (dln[j].C.x == pts[i].x && dln[j].C.y == pts[i].y))
			{
				isTrue = true;
			}
		}

		if (!isTrue)
		{
			pts.erase(pts.begin() + i);
			i--;
		}
	}
}

vector<double> CTriangulationDlg::calcAij(vector<Points> pts, vector<Delone> dln)
{
	// Объявляем квадратную матрицу Aij (TxT); T - количество не граничных точек
	vector<double> Aij;

	// Проходим по пулу НЕграничных точек
	for (int i = 0; i < pts.size(); i++)
	{
		for (int j = 0; j < pts.size(); j++)
		{
			// Ситуация #1: i = j ----> Aij = -Sum((A^2 + B^2)*S_tr)
			double a_ij = 0.0;
			if (i == j)
			{
				// Ищем все треугольники, в которые входит данная точка
				vector<Delone> neighboursTriang = isDotsNeighbours(pts[j], dln);

				// Считаем Aij
				if (!neighboursTriang.empty())
				{
					for (int k = 0; k < neighboursTriang.size(); k++)
					{
						// Расчет площади основания треугольника
						double square = abs((neighboursTriang[k].B.x - neighboursTriang[k].A.x) * (neighboursTriang[k].C.y - neighboursTriang[k].A.y)
							- ((neighboursTriang[k].C.x - neighboursTriang[k].A.x) * (neighboursTriang[k].B.y - neighboursTriang[k].A.y)));
						square /= 2.0;

						// Поднимем координату Z у заданной точки в треугольнике
						Delone triang = replaceZInTriangle(pts[j], neighboursTriang[k], 1.0);

						// Расчет коэффициентов A,B из уравнения плоскости
						double A = 0.0;
						double B = 0.0;
						calcABForTriang(A, B, triang);

						a_ij += (A * A + B * B) * square;
					}
				}

				Aij.push_back(-a_ij);
			}

			if (i != j)
			{
				// Проверяем условие, что i - сосед j
				vector<Delone> ij_neigh = isIJNeighbours(pts[i], pts[j], dln);

				if (!ij_neigh.empty())
				{
					// Считаем Aij
					for (int k = 0; k < ij_neigh.size(); k++)
					{
						// Расчет площади основания треугольника
						double square = abs((ij_neigh[k].B.x - ij_neigh[k].A.x) * (ij_neigh[k].C.y - ij_neigh[k].A.y)
							- ((ij_neigh[k].C.x - ij_neigh[k].A.x) * (ij_neigh[k].B.y - ij_neigh[k].A.y)));
						square /= 2.0;

						// Зануляем ось Z для j-ой точки, чтобы посчитать A1,B1
						Delone triang1 = replaceZInTriangle(pts[i], ij_neigh[k], 1.0);

						// Считаем A1,B1
						double A1 = 0.0;
						double B1 = 0.0;
						calcABForTriang(A1, B1, triang1);

						// Зануляем ось Z для i-ой точки, а значение j.z = 1.0;
						Delone triang2 = replaceZInTriangle(pts[j], ij_neigh[k], 1.0);

						// Считаем A1,B1
						double A2 = 0.0;
						double B2 = 0.0;
						calcABForTriang(A2, B2, triang2);

						a_ij += (A1 * A2 + B1 * B2) * square;
					}
				}

				Aij.push_back(-a_ij);
			}
		}
	}

	return Aij;
}

vector<double> CTriangulationDlg::calcBj(vector<Points> pts_border, vector<Points> pts_not_border, vector<Delone> dln)
{
	vector<double> Bj;

	for (int j = 0; j < pts_not_border.size(); j++)
	{
		double bj = 0.0;

		for (int m = 0; m < pts_border.size(); m++)
		{
			double bjm = 0.0;
			// Ситуация i - сосед j
			vector<Delone> mj_neigh = isIJNeighbours(pts_border[m], pts_not_border[j], dln);
			if (!mj_neigh.empty())
			{
				for (int k = 0; k < mj_neigh.size(); k++)
				{
					// Расчет площади основания треугольника
					double square = abs((mj_neigh[k].B.x - mj_neigh[k].A.x) * (mj_neigh[k].C.y - mj_neigh[k].A.y)
						- ((mj_neigh[k].C.x - mj_neigh[k].A.x) * (mj_neigh[k].B.y - mj_neigh[k].A.y)));
					square /= 2.0;

					// Зануляем ось Z для j-ой точки, чтобы посчитать A1,B1
					Delone triang1 = replaceZInTriangle(pts_not_border[j], mj_neigh[k], 1.0);

					// Считаем A1,B1
					double A1 = 0.0;
					double B1 = 0.0;
					calcABForTriang(A1, B1, triang1);

					// Зануляем ось Z для m-ой точки, а значение j.z = 1.0;
					Delone triang2 = replaceZInTriangle(pts_border[m], mj_neigh[k], 1.0);

					// Считаем A1,B1
					double A2 = 0.0;
					double B2 = 0.0;
					calcABForTriang(A2, B2, triang2);

					bjm += (A1 * A2 + B1 * B2) * square;
				}

				bj = bjm * pts_border[m].potential;
			}
		}

		Bj.push_back(bj);
	}
	return Bj;
}

void kazf(vector<double> a, vector<double> b, vector<double>& x, int nn, int ny)
{
	// nn - количество неизвестных;  ny - количество уравнений
	double eps = 1.e-6;
	int i, j, k;
	double s1, s2, fa1, t;
	vector<double> x1(nn);

	x[0] = 0.5;
	for (i = 1; i < nn; i++)
	{
		x[i] = 0.0;
	}

	s1 = s2 = 1.0;
	while (s1 > eps * s2)
	{
		for (i = 0; i < nn; i++)
		{
			x1[i] = x[i];
		}

		for (i = 0; i < ny; i++)
		{
			s1 = 0.0;
			s2 = 0.0;
			for (j = 0; j < nn; j++)
			{
				fa1 = a[i * nn + j];
				s1 += fa1 * x[j];
				s2 += fa1 * fa1;
			}

			t = (b[i] - s1) / s2;
			for (k = 0; k < nn; k++)
			{
				x[k] += a[i * nn + k] * t;
			}
		}

		s1 = 0.0;
		s2 = 0.0;

		for (i = 0; i < nn; i++)
		{
			s1 += (x[i] - x1[i]) * (x[i] - x1[i]);
			s2 += x[i] * x[i];
		}
		s1 = (double)sqrt(s1);
		s2 = (double)sqrt(s2);
	}
}

vector<vector<Points>> CTriangulationDlg::equipotentialLines(vector<Delone> dln)
{
	// Построение эквипотенциальных линий (по аналогии с изотермами)
	double pc_min, pc_max;

	pc_min = dln[0].A.potential;
	pc_max = dln[0].A.potential;

	for (int i = 0; i < dln.size(); i++)
	{
		// max
		if (pc_max < dln[i].A.potential
			&& !dln[i].A.is_magnet_border_north
			&& !dln[i].A.is_magnet_border_south
			&& !dln[i].A.is_rect_border)
		{
			pc_max = dln[i].A.potential;
		}

		if (pc_max < dln[i].B.potential
			&& !dln[i].B.is_magnet_border_north
			&& !dln[i].B.is_magnet_border_south
			&& !dln[i].B.is_rect_border)
		{
			pc_max = dln[i].B.potential;
		}

		if (pc_max < dln[i].C.potential
			&& !dln[i].C.is_magnet_border_north
			&& !dln[i].C.is_magnet_border_south
			&& !dln[i].C.is_rect_border)
		{
			pc_max = dln[i].C.potential;
		}

		// min
		if (pc_min > dln[i].A.potential
			&& !dln[i].A.is_magnet_border_north
			&& !dln[i].A.is_magnet_border_south
			&& !dln[i].A.is_rect_border)
		{
			pc_min = dln[i].A.potential;
		}

		if (pc_min > dln[i].B.potential
			&& !dln[i].B.is_magnet_border_north
			&& !dln[i].B.is_magnet_border_south
			&& !dln[i].B.is_rect_border)
		{
			pc_min = dln[i].B.potential;
		}

		if (pc_min > dln[i].C.potential
			&& !dln[i].C.is_magnet_border_north
			&& !dln[i].C.is_magnet_border_south
			&& !dln[i].C.is_rect_border)
		{
			pc_min = dln[i].C.potential;
		}
	}

	vector<vector<Points>> pts;
	for (int i = 0; i < dln.size(); i++)
	{
		Points p1 = dln[i].A;	// min
		Points p2 = dln[i].B;
		Points p3 = dln[i].C;	// max

		// Шаг 2.
		if (p1.potential > p2.potential)
		{
			swap(p1, p2);
		}

		if (p1.potential > p3.potential)
		{
			swap(p1, p3);
		}

		if (p2.potential > p3.potential)
		{
			swap(p2, p3);
		}

		double step = (abs(pc_max) + abs(pc_min)) / 10.;
		for (double j = pc_min + step; j < pc_max; j += step)
		{
			vector<Points> pair;
			if (j > p1.potential && j < p3.potential)
			{
				Points dot1;
				dot1.x = p3.x - ((p3.potential - j) * (p3.x - p1.x)) / (p3.potential - p1.potential);
				dot1.y = p3.y - ((p3.potential - j) * (p3.y - p1.y)) / (p3.potential - p1.potential);

				pair.push_back(dot1);

				if (j > p2.potential)
				{
					Points dot2;
					dot2.x = p3.x - ((p3.potential - j) * (p3.x - p2.x)) / (p3.potential - p2.potential);
					dot2.y = p3.y - ((p3.potential - j) * (p3.y - p2.y)) / (p3.potential - p2.potential);

					pair.push_back(dot2);
				}

				else if (j < p2.potential)
				{
					Points dot2;
					dot2.x = p2.x - ((p2.potential - j) * (p2.x - p1.x)) / (p2.potential - p1.potential);
					dot2.y = p2.y - ((p2.potential - j) * (p2.y - p1.y)) / (p2.potential - p1.potential);

					pair.push_back(dot2);
				}
			}

			if (!pair.empty())
			{
				pts.push_back(pair);
			}
		}
	}

	return pts;
}

double IsPointInsidePolygon(vector<Points> p, double x, double y)
{
	int i1, i2, n, N, flag;
	double S, S1, S2, S3;
	N = p.size();
	for (n = 0; n < N; n++)
	{
		flag = 0;
		i1 = n < N - 1 ? n + 1 : 0;
		while (flag == 0)
		{
			i2 = i1 + 1;
			if (i2 >= N)
				i2 = 0;
			if (i2 == (n < N - 1 ? n + 1 : 0))
				break;
			S = abs(p[i1].x * (p[i2].y - p[n].y) +
				p[i2].x * (p[n].y - p[i1].y) +
				p[n].x * (p[i1].y - p[i2].y));
			S1 = abs(p[i1].x * (p[i2].y - y) +
				p[i2].x * (y - p[i1].y) +
				x * (p[i1].y - p[i2].y));
			S2 = abs(p[n].x * (p[i2].y - y) +
				p[i2].x * (y - p[n].y) +
				x * (p[n].y - p[i2].y));
			S3 = abs(p[i1].x * (p[n].y - y) +
				p[n].x * (y - p[i1].y) +
				x * (p[i1].y - p[n].y));
			if (S == S1 + S2 + S3)
			{
				flag = 1;
				break;
			}
			i1 = i1 + 1;
			if (i1 >= N)
				i1 = 0;
			break;
		}
		if (flag == 0)
			break;
	}
	return flag;
}

double area(Points d1, Points d2, Points d3)
{
	return abs((d1.x * (d2.y - d3.y) + d2.x * (d3.y - d1.y) + d3.x * (d1.y - d2.y)) / 2.0);
}

void lineKoef(Points pt1, Points pt2, double& A, double& B, double& C)
{
	A = pt1.y - pt2.y;
	B = pt2.x - pt1.x;
	C = -pt2.x * pt1.y + pt2.y * pt1.x;
}

Points lineIntersection(Points start1, Points stop1, Points start2, Points stop2)
{
	double eps = 1.e-6;

	double A1 = 0.0;
	double B1 = 0.0;
	double C1 = 0.0;

	double A2 = 0.0;
	double B2 = 0.0;
	double C2 = 0.0;

	lineKoef(start1, stop1, A1, B1, C1);
	lineKoef(start2, stop2, A2, B2, C2);

	double d = A1 * B2 - A2 * B1;
	Points intersect_dot;

	intersect_dot.x = (B1 * C2 - B2 * C1) / d;
	intersect_dot.y = (C1 * A2 - C2 * A1) / d;

	return intersect_dot;
}

vector<vector<Points>> CTriangulationDlg::powerLine(vector<Points> pts, vector<Delone> dln)
{
	vector<vector<Points>> output;

	double eps = 1.e-9;
	for (int i = 0; i < polygon.size(); i++)
	{
		if (i % 1 == 0)
		{
			vector<Points> power;
			// Добавляем пробные положительные заряды на границе магнита (не в узлах)
			Points middle;
			middle.x = polygon[i].x;
			middle.y = polygon[i].y;

			// Заносим координаты частицы в массив
			power.push_back(middle);

			int counter = 0;
			do
			{
				// Находим ближайший узел к данной точке
				int index_min_elem = 0;
				double distance_min = (pts[0].x - middle.x) * (pts[0].x - middle.x) + (pts[0].y - middle.y) * (pts[0].y - middle.y);
				for (int j = 0; j < pts.size(); j++)
				{
					double distance = (pts[j].x - middle.x) * (pts[j].x - middle.x) + (pts[j].y - middle.y) * (pts[j].y - middle.y);
					if (distance < distance_min)
					{
						distance_min = distance;
						index_min_elem = j;
					}
				}

				// Находим треугольники, в которых содержится найденный узел
				vector<Delone> triang;
				for (int j = 0; j < dln.size(); j++)
				{
					if ((abs(dln[j].A.x - pts[index_min_elem].x) < eps && abs(dln[j].A.y - pts[index_min_elem].y) < eps)
						|| (abs(dln[j].B.x - pts[index_min_elem].x) < eps && abs(dln[j].B.y - pts[index_min_elem].y) < eps)
						|| (abs(dln[j].C.x - pts[index_min_elem].x) < eps && abs(dln[j].C.y - pts[index_min_elem].y) < eps))
					{
						triang.push_back(dln[j]);
					}
				}

				// Определяем в какому треугольнику принадлежит точка
				Delone inside_triang;
				for (int j = 0; j < triang.size(); j++)
				{
					double Ar = area(triang[j].A, triang[j].B, triang[j].C);
					double Ar1 = area(middle, triang[j].B, triang[j].C);
					double Ar2 = area(triang[j].A, middle, triang[j].C);
					double Ar3 = area(triang[j].A, triang[j].B, middle);

					if (abs(Ar - (Ar1 + Ar2 + Ar3)) < eps)
					{
						inside_triang = triang[j];
					}
				}

				inside_triang.A.z = inside_triang.A.potential;
				inside_triang.B.z = inside_triang.B.potential;
				inside_triang.C.z = inside_triang.C.potential;

				double A = 0.0;
				double B = 0.0;
				calcABForTriang(A, B, inside_triang);

				Points pred = middle;

				Points npts;
				npts.x = middle.x - A;
				npts.y = middle.y - B;

				Points intersectAB = lineIntersection(middle, npts, inside_triang.A, inside_triang.B);
				Points intersectAC = lineIntersection(middle, npts, inside_triang.A, inside_triang.C);
				Points intersectBC = lineIntersection(middle, npts, inside_triang.B, inside_triang.C);

				double Ar = area(inside_triang.A, inside_triang.B, inside_triang.C);

				double Ar1_AB = area(intersectAB, inside_triang.B, inside_triang.C);
				double Ar2_AB = area(inside_triang.A, intersectAB, inside_triang.C);
				double Ar3_AB = area(inside_triang.A, inside_triang.B, intersectAB);

				double Ar1_AC = area(intersectAC, inside_triang.B, inside_triang.C);
				double Ar2_AC = area(inside_triang.A, intersectAC, inside_triang.C);
				double Ar3_AC = area(inside_triang.A, inside_triang.B, intersectAC);

				double Ar1_BC = area(intersectBC, inside_triang.B, inside_triang.C);
				double Ar2_BC = area(inside_triang.A, intersectBC, inside_triang.C);
				double Ar3_BC = area(inside_triang.A, inside_triang.B, intersectBC);

				if (abs(Ar - (Ar1_AB + Ar2_AB + Ar3_AB)) < eps && abs(Ar - (Ar1_AC + Ar2_AC + Ar3_AC)) < eps && abs(Ar - (Ar1_BC + Ar2_BC + Ar3_BC)) < eps)
				{
					double conditionAB = (intersectAB.x - middle.x) * (intersectAB.x - middle.x) + (intersectAB.y - middle.y) * (intersectAB.y - middle.y);
					double conditionAC = (intersectAC.x - middle.x) * (intersectAC.x - middle.x) + (intersectAC.y - middle.y) * (intersectAC.y - middle.y);
					double conditionBC = (intersectBC.x - middle.x) * (intersectBC.x - middle.x) + (intersectBC.y - middle.y) * (intersectBC.y - middle.y);

					if (conditionAB >= conditionAC && conditionAB >= conditionBC)
					{
						middle = intersectAB;
					}

					if (conditionAC >= conditionAB && conditionAC >= conditionBC)
					{
						middle = intersectAC;
					}

					if (conditionBC >= conditionAB && conditionBC >= conditionAC)
					{
						middle = intersectBC;
					}
				}

				else if (abs(Ar - (Ar1_AB + Ar2_AB + Ar3_AB)) < eps && abs(Ar - (Ar1_AC + Ar2_AC + Ar3_AC)) < eps && abs(Ar - (Ar1_BC + Ar2_BC + Ar3_BC)) > eps)
				{
					double conditionAB = (intersectAB.x - middle.x) * (intersectAB.x - middle.x) + (intersectAB.y - middle.y) * (intersectAB.y - middle.y);
					double conditionAC = (intersectAC.x - middle.x) * (intersectAC.x - middle.x) + (intersectAC.y - middle.y) * (intersectAC.y - middle.y);

					if (conditionAB >= conditionAC)
					{
						middle = intersectAB;
					}
					else
					{
						middle = intersectAC;
					}
				}

				else if (abs(Ar - (Ar1_AB + Ar2_AB + Ar3_AB)) < eps && abs(Ar - (Ar1_AC + Ar2_AC + Ar3_AC)) > eps && abs(Ar - (Ar1_BC + Ar2_BC + Ar3_BC)) < eps)
				{
					double conditionAB = (intersectAB.x - middle.x) * (intersectAB.x - middle.x) + (intersectAB.y - middle.y) * (intersectAB.y - middle.y);
					double conditionBC = (intersectBC.x - middle.x) * (intersectBC.x - middle.x) + (intersectBC.y - middle.y) * (intersectBC.y - middle.y);

					if (conditionAB >= conditionBC)
					{
						middle = intersectAB;
					}
					else
					{
						middle = intersectBC;
					}
				}

				else if (abs(Ar - (Ar1_AB + Ar2_AB + Ar3_AB)) > eps && abs(Ar - (Ar1_AC + Ar2_AC + Ar3_AC)) < eps && abs(Ar - (Ar1_BC + Ar2_BC + Ar3_BC)) < eps)
				{
					double conditionAC = (intersectAC.x - middle.x) * (intersectAC.x - middle.x) + (intersectAC.y - middle.y) * (intersectAC.y - middle.y);
					double conditionBC = (intersectBC.x - middle.x) * (intersectBC.x - middle.x) + (intersectBC.y - middle.y) * (intersectBC.y - middle.y);

					if (conditionAC >= conditionBC)
					{
						middle = intersectAC;
					}
					else
					{
						middle = intersectBC;
					}
				}

				// Находим угол наклона прямой относительно X
				double side_y = middle.y - pred.y;
				double side_x = middle.x - pred.x;

				double angle_rad = atan2(side_y, side_x);

				// Величина удлинения
				double r = 0.0001;

				middle.x += r * cos(angle_rad);
				middle.y += r * sin(angle_rad);

				counter++;
				power.push_back(middle);

			} while (counter < 1000 && middle.x > xmin && middle.x < xmax && middle.y > ymin && middle.y < ymax
						&& !pnpoly(polygon.size(), polygon, middle.x, middle.y));

			output.push_back(power);
		}
	}

	return output;
}

void CTriangulationDlg::OnBnClickedBtnCalc()
{
	UpdateData(TRUE);

	vector<Points> points;
	vector<Delone> triangles;

	// Рисуем точки сверхструктуры и крайние точки области
	drawSuperstructure(points);
	drawRectangle(points);

	Points center;
	center.x = centerMagnetX;
	center.y = centerMagnetY;

	// Рисуем магнит
	drawMagnet(points, center,
		magnet_inner_radius, magnet_outer_radius,
		magnet_cut_width, magnet_angle_north, magnet_angle_south, polygon, false);
	drawMagnet(points, center,
		magnet_inner_radius * 0.97, magnet_outer_radius * 1.03,
		magnet_cut_width * 0.93, magnet_angle_north, magnet_angle_south, polygon_hitbox, true);

	// Делаем первоначальную триангуляцию
	Triangulation(points, triangles);

	// Рисуем сетку
	vector<Points> area;
	drawGrid(area, magnet_outer_radius, magnet_inner_radius, magnet_cut_width, center);
	for (int i = 0; i < area.size(); i++)
	{
		points.push_back(area[i]);
		RecurrentTriangulation(points, triangles);
	}

	for (int i = 0; i < points.size(); i++)
	{
		if (points[i].is_super_dot)
		{
			points.erase(points.begin() + i);
			i--;
		}
	}

	deleteSuperDots(triangles);
	deleteFromMagnet(triangles, polygon);

	// Убираем лишние точки из вектора с точками
	deleteFromPoints(triangles, points);

	//// Расчет матрицы Aij
	vector<Points> notBorder = notBorderDots(points);
	vector<double> Aij = calcAij(notBorder, triangles);

	// Расчет вектора свободных членов Bj
	vector<Points> border = borderDots(points);
	vector<double> Bj = calcBj(border, notBorder, triangles);

	vector<double> Ci(notBorder.size());

	// Решаем матричное уравнение с помощью Качмаржа
	kazf(Aij, Bj, Ci, Ci.size(), Bj.size());

	// Обновили массив точек
	double eps = 1.e-4;
	for (int i = 0; i < notBorder.size(); i++)
	{
		for (int j = 0; j < points.size(); j++)
		{
			if (abs(notBorder[i].x - points[j].x) < eps && abs(notBorder[i].y - points[j].y) < eps)
			{
				points[j].potential = Ci[i];
			}
		}
	}

	// Обновили массив с триангуляцией
	for (int i = 0; i < notBorder.size(); i++)
	{
		for (int j = 0; j < triangles.size(); j++)
		{
			if (abs(notBorder[i].x - triangles[j].A.x) < eps && abs(notBorder[i].y - triangles[j].A.y) < eps)
			{
				triangles[j].A.potential = Ci[i];
			}

			if (abs(notBorder[i].x - triangles[j].B.x) < eps && abs(notBorder[i].y - triangles[j].B.y) < eps)
			{
				triangles[j].B.potential = Ci[i];
			}

			if (abs(notBorder[i].x - triangles[j].C.x) < eps && abs(notBorder[i].y - triangles[j].C.y) < eps)
			{
				triangles[j].C.potential = Ci[i];
			}
		}
	}

	/*ofstream out1("Aij.txt");
	for (int i = 0; i < notBorder.size(); i++)
	{
		for (int j = 0; j < notBorder.size(); j++)
		{
			out1 << Aij[i * notBorder.size() + j] << "\t";
		}
		out1 << "\n";
	}
	out1.close();*/

	gl_points = points;
	gl_triangles = triangles;

	vector<vector<Points>> empty;
	DrawTriangulation(points, triangles, empty, empty);

	// notBorder.clear();
	points.clear();
	triangles.clear();
}

void CTriangulationDlg::OnBnClickedBtnExit()
{
	CTriangulationDlg::OnCancel();
}

void CTriangulationDlg::OnBnClickedCheckGraphPotential()
{
	if (!gl_points.empty())
	{
		vector<vector<Points>> empty;
		DrawTriangulation(gl_points, gl_triangles, empty, empty);
	}
}


void CTriangulationDlg::OnBnClickedCheckGraphEquipotential()
{
	if (!gl_triangles.empty())
	{
		vector<vector<Points>> equipotential = equipotentialLines(gl_triangles);

		if (graphEquipotential.GetCheck() == BST_CHECKED)
		{
			vector<vector<Points>> empty;
			DrawTriangulation(gl_points, gl_triangles, equipotential, empty);
		}

		if (graphEquipotential.GetCheck() == BST_UNCHECKED)
		{
			vector<vector<Points>> empty;
			DrawTriangulation(gl_points, gl_triangles, empty, empty);
		}
	}
}


void CTriangulationDlg::OnBnClickedCheckGraphPower()
{
	if (!gl_triangles.empty())
	{
		if (graphPower.GetCheck() == BST_CHECKED)
		{
			vector<vector<Points>> power = powerLine(gl_points, gl_triangles);
			vector<vector<Points>> empty;
			DrawTriangulation(gl_points, gl_triangles, empty, power);
		}

		if (graphPower.GetCheck() == BST_UNCHECKED)
		{
			vector<vector<Points>> empty;
			DrawTriangulation(gl_points, gl_triangles, empty, empty);
		}
	}
}
