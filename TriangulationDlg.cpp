#pragma once
#include "pch.h"
#include "framework.h"
#include "Triangulation.h"
#include "TriangulationDlg.h"
#include "afxdialogex.h"
#include <cstdlib>
#include <ctime>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DOTS(x,y) (xp*((x)-xminSuper)), (yp*((y)-ymaxSuper))
using namespace std;

DWORD WINAPI MyProc(PVOID pv)
{
	CTriangulationDlg* p = (CTriangulationDlg*)pv;
	p->Animation();
	return 0;
}

CTriangulationDlg::CTriangulationDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TRIANGULATION_DIALOG, pParent)
	, ptsSize(0)
	, stepKoef(5)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTriangulationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ED_PT_SIZE, ptsSize);
	DDX_Text(pDX, IDC_ED_PT_SIZE2, stepKoef);
}

BEGIN_MESSAGE_MAP(CTriangulationDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CALC, &CTriangulationDlg::OnBnClickedBtnCalc)
	ON_BN_CLICKED(IDC_BTN_EXIT, &CTriangulationDlg::OnBnClickedBtnExit)
	ON_BN_CLICKED(IDC_BTN_TRIANG, &CTriangulationDlg::OnBnClickedBtnTriang)
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
		RGB(220, 0, 0)
	);

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
	MemDc->FillSolidRect(Pic, RGB(0, 0, 0));

	double differ_x = 0.005 * xmax;
	double differ_y = 0.005 * ymax;

	for (int i = 0; i < dln.size(); ++i)
	{
		MemDc->SelectObject(&lines_pen);
		MemDc->MoveTo(DOTS(dln[i].A.x, dln[i].A.y));
		MemDc->LineTo(DOTS(dln[i].B.x, dln[i].B.y));
		MemDc->LineTo(DOTS(dln[i].C.x, dln[i].C.y));
		MemDc->LineTo(DOTS(dln[i].A.x, dln[i].A.y));
	}

	CBrush circle(RGB(240, 240, 240));
	MemDc->SelectObject(&dots_pen);
	for (int i = 0; i < pts.size(); i++)
	{
		MemDc->SelectObject(&circle);
		MemDc->Ellipse(DOTS(pts[i].x - differ_x, pts[i].y - differ_y), DOTS(pts[i].x + differ_x, pts[i].y + differ_y));
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
						if (condition < radius * radius || radius > 40)
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

void CTriangulationDlg::drawArea(vector<Points>& vec, bool mode)
{
	// область
	if (mode)
	{
		Points pt_lt, pt_lb, pt_rt, pt_rb, pt_c;

		double rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
		pt_lt.x = xmin + rand_coord;
		rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
		pt_lt.y = ymin + rand_coord;
		rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
		pt_lb.x = xmin + rand_coord;
		rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
		pt_lb.y = ymax + rand_coord;
		rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
		pt_rt.x = xmax + rand_coord;
		rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
		pt_rt.y = ymin + rand_coord;
		rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
		pt_rb.x = xmax + rand_coord;
		rand_coord = randMin + ((double)rand() / RAND_MAX) * randMax;
		pt_rb.y = ymax + rand_coord;
		vec.push_back(pt_lt);
		vec.push_back(pt_lb);
		vec.push_back(pt_rt);
		vec.push_back(pt_rb);
	}
	else
	{
		double step_x = (xmax - xmin) / stepKoef;
		double step_y = (ymax - ymin) / stepKoef;
		for (double i = xmin; i <= xmax; i += step_x)
		{
			for (double j = ymin; j <= ymax; j += step_y)
			{
				if (i != xmin && i != xmax || j != ymin && j != ymax)
				{
					double rand_x = randMin + ((double)rand() / RAND_MAX) * randMax;
					double rand_y = randMin + ((double)rand() / RAND_MAX) * randMax;

					Points pt_t, pt_b;
					pt_t.x = i + rand_x;
					pt_t.y = j + rand_y;

					vec.push_back(pt_t);
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
	lb.x = xminSuper + ratio * xmaxSuper;
	lb.y = ymaxSuper - ratio * ymaxSuper;
	rt.x = xmaxSuper - ratio * xmaxSuper;
	rt.y = yminSuper + ratio * ymaxSuper;
	rb.x = xmaxSuper - ratio * xmaxSuper;
	rb.y = ymaxSuper - ratio * ymaxSuper;
	vec.push_back(lt);
	vec.push_back(lb);
	vec.push_back(rt);
	vec.push_back(rb);
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

void CTriangulationDlg::Animation()
{
	while (1)
	{

	}
}

void CTriangulationDlg::OnBnClickedBtnCalc()
{
	UpdateData(TRUE);
	vector<Points> points;
	vector<Delone> triangles;

	drawSuperstructure(points);
	drawArea(points, true);

	if (ptsSize != 0)
	{
		for (int i = 0; i < ptsSize; i++)
		{
			Points pt;
			pt.x = ((double)rand() / RAND_MAX) * xmax;
			pt.y = ((double)rand() / RAND_MAX) * ymax;
			points.push_back(pt);
		}
	}

	if (points.size() > 2)
	{
		Triangulation(points, triangles);
		DrawTriangulation(points, triangles);
	}

	global_points = points;
	global_triangles = triangles;
}

void CTriangulationDlg::OnBnClickedBtnTriang()
{
	UpdateData(TRUE);
	vector<Points> area;
	drawArea(area, false);
	
	vector<Points> pts;
	vector<Delone> dln;

	pts = global_points;
	dln = global_triangles;

	for (int i = 0; i < area.size(); i++)
	{
		pts.push_back(area[i]);
		RecurrentTriangulation(pts, dln);
	}
	DrawTriangulation(pts, dln);
}


void CTriangulationDlg::OnBnClickedBtnExit()
{
	CTriangulationDlg::OnCancel();
}