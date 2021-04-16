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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DOTS(x,y) (xp*((x)-xminSuper)), (yp*((y)-ymaxSuper))
using namespace std;

CTriangulationDlg::CTriangulationDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TRIANGULATION_DIALOG, pParent)
	, stepKoef(3)
	, xmin(20)
	, xmax(80)
	, ymin(20)
	, ymax(80)
	, centerMagnetX(50)
	, centerMagnetY(50)
	, magnet_inner_radius(10)
	, magnet_outer_radius(20)
	, magnet_cut_width(15)
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
}

BEGIN_MESSAGE_MAP(CTriangulationDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CALC, &CTriangulationDlg::OnBnClickedBtnCalc)
	ON_BN_CLICKED(IDC_BTN_EXIT, &CTriangulationDlg::OnBnClickedBtnExit)
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

void CTriangulationDlg::DrawTriangulation(vector<Points> pts, vector<Delone> dln)
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
				str.Format(_T("%.0f"), pts[i].potential);
				MemDc->TextOutW(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), str);
			}
		}

		// Граничные точки magnet-области (south)
		else if (pts[i].is_magnet_border_south)
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
				str.Format(_T("%.0f"), pts[i].potential);
				MemDc->TextOutW(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), str);
			}
		}

		// Граничные точки magnet-области (north)
		else if (pts[i].is_magnet_border_north)
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
				str.Format(_T("%.0f"), pts[i].potential);
				MemDc->TextOutW(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), str);
			}
		}

		// Остальные точки
		else
		{
			CPen circle_pen;
			circle_pen.CreatePen(
				PS_SOLID,
				-1,
				RGB(0, 0, 0));

			CBrush circle_brush(RGB(0, 0, 0));
			MemDc->SelectObject(&circle_pen);
			MemDc->SelectObject(&circle_brush);
			MemDc->Ellipse(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), DOTS(pts[i].x + differ_x, pts[i].y + differ_y));

			if (graphPotentialValues.GetCheck() == BST_CHECKED)
			{
				CString str;
				str.Format(_T("%.0f"), pts[i].potential);
				MemDc->TextOutW(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), str);
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

	double magnet_contour_step = stepKoef;

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
		if (deletedTriag[i].A.x == vec.back().x && deletedTriag[i].A.y == vec.back().y
			|| deletedTriag[i].B.x == vec.back().x && deletedTriag[i].B.y == vec.back().y
			|| deletedTriag[i].C.x == vec.back().x && deletedTriag[i].C.y == vec.back().y)
		{
			dln.push_back(deletedTriag[i]);
		}
	}

	deletedTriag.clear();
	deletedPts.clear();
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
		magnet_inner_radius * 0.95, magnet_outer_radius * 1.05,
		magnet_cut_width * 0.8, magnet_angle_north, magnet_angle_south, polygon_hitbox, true);

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

	DrawTriangulation(points, triangles);
	points.clear();
	triangles.clear();
}

void CTriangulationDlg::OnBnClickedBtnExit()
{
	CTriangulationDlg::OnCancel();
}