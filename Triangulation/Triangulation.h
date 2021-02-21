
// Triangulation.h: главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CTriangulationApp:
// Сведения о реализации этого класса: Triangulation.cpp
//

class CTriangulationApp : public CWinApp
{
public:
	CTriangulationApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CTriangulationApp theApp;
