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

CTriangulationDlg::CTriangulationDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TRIANGULATION_DIALOG, pParent)
	, ptsSize(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTriangulationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ED_PT_SIZE, ptsSize);
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
		2,
		RGB(0, 0, 220)
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

void CTriangulationDlg::DrawDots(vector<Points> pt, int num_of_pts)
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

	double differ_x = 0.005 * xmaxSuper;
	double differ_y = 0.005 * ymaxSuper;

	CBrush circle(RGB(240, 240, 240));
	MemDc->SelectObject(&dots_pen);
	for (int i = 0; i < num_of_pts; ++i)
	{
		MemDc->SelectObject(&circle);
		MemDc->Ellipse(DOTS(pt[i].x - differ_x, pt[i].y - differ_y), DOTS(pt[i].x + differ_x, pt[i].y + differ_y));
	}

	// вывод на экран
	PicDc->BitBlt(0, 0, window_signal_width, window_signal_height, MemDc, 0, 0, SRCCOPY);
	delete MemDc;
}

void CTriangulationDlg::DrawTriangulation(vector<Delone> dlnVec, int num_of_delone_pts)
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

	double differ_x = 0.005 * xmax;
	double differ_y = 0.005 * ymax;

	CBrush circle(RGB(240, 240, 240));
	MemDc->SelectObject(&dots_pen);
	for (int i = 0; i < num_of_delone_pts; ++i)
	{
		MemDc->SelectObject(&lines_pen);
		MemDc->MoveTo(DOTS(dlnVec[i].A.x, dlnVec[i].A.y));
		MemDc->LineTo(DOTS(dlnVec[i].B.x, dlnVec[i].B.y));
		MemDc->LineTo(DOTS(dlnVec[i].C.x, dlnVec[i].C.y));
		MemDc->LineTo(DOTS(dlnVec[i].A.x, dlnVec[i].A.y));

		MemDc->SelectObject(&dots_pen);
		MemDc->SelectObject(&circle);
		// перерисовываем каждую точку из готовой трингуляции
		MemDc->Ellipse(DOTS(dlnVec[i].A.x - differ_x, dlnVec[i].A.y - differ_y),
			DOTS(dlnVec[i].A.x + differ_x, dlnVec[i].A.y + differ_y));
		MemDc->Ellipse(DOTS(dlnVec[i].B.x - differ_x, dlnVec[i].B.y - differ_y),
			DOTS(dlnVec[i].B.x + differ_x, dlnVec[i].B.y + differ_y));
		MemDc->Ellipse(DOTS(dlnVec[i].C.x - differ_x, dlnVec[i].C.y - differ_y),
			DOTS(dlnVec[i].C.x + differ_x, dlnVec[i].C.y + differ_y));
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

	double radiusExternal = sqrt((center.x - pt1.x) * (center.x - pt1.x) + (center.y - pt1.y) * (center.y - pt1.y));
	return radiusExternal;
}

void CTriangulationDlg::Triangulation(vector<Points> pt)
{
	delonePts.clear();
	for (int i = 0; i < ptsSize; i++)
	{
		for (int j = i + 1; j < ptsSize; j++)
		{
			for (int k = j + 1; k < ptsSize; k++)
			{
				Points center;
				double radius = OuterCircleRadius(globPts[i], globPts[j], globPts[k], center);
				bool isDeloneLess = true;
				int isDeloneMore = 0;
				for (int m = 0; m < ptsSize; m++)
				{
					if (m != i && m != j && m != k)
					{
						double condition = (globPts[m].x - center.x) * (globPts[m].x - center.x)
							+ (globPts[m].y - center.y) * (globPts[m].y - center.y);
						if (condition < radius * radius)
						{
							isDeloneLess = false;
							break;
						}
						else if (condition > radius * radius)
						{
							isDeloneMore++;
						}
					}
				}
				if (isDeloneLess && isDeloneMore == ptsSize-3)
				{
					Delone triangle;
					triangle.A = globPts[i];
					triangle.B = globPts[j];
					triangle.C = globPts[k];

					delonePts.push_back(triangle);
				}
			}
		}
	}
}

void CTriangulationDlg::OnBnClickedBtnCalc()
{
	UpdateData(TRUE);
	globPts.clear();

	for (int i = 0; i < ptsSize; i++)
	{
		Points pt;
		pt.x = (double)rand() / RAND_MAX * xmax;
		pt.y = (double)rand() / RAND_MAX * ymax;
		globPts.push_back(pt);
	}
	// точки сверхструктуры
	Points lt, lb, rt, rb;
	lt.x = (int)xminSuper + 0.05 * xmaxSuper;
	lt.y = (int)yminSuper + 0.05 * ymaxSuper;
	lb.x = (int)xminSuper + 0.05 * xmaxSuper;
	lb.y = (int)ymaxSuper - 0.05 * ymaxSuper;
	rt.x = (int)xmaxSuper - 0.05 * xmaxSuper;
	rt.y = (int)yminSuper + 0.05 * ymaxSuper;
	rb.x = (int)xmaxSuper - 0.05 * xmaxSuper;
	rb.y = (int)ymaxSuper - 0.05 * ymaxSuper;
	globPts.push_back(lt);
	globPts.push_back(lb);
	globPts.push_back(rt);
	globPts.push_back(rb);
	ptsSize += 4;

	DrawDots(globPts, ptsSize);

	Triangulation(globPts);

	DrawTriangulation(delonePts, delonePts.size());
}


void CTriangulationDlg::OnBnClickedBtnExit()
{
	globPts.clear();
	delonePts.clear();
	CTriangulationDlg::OnCancel();
}


void CTriangulationDlg::OnBnClickedBtnTriang()
{
	UpdateData(TRUE);
}
