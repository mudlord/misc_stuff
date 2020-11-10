#include "stdafx.h"
#include "rombrowser.h"

CRomBrowser* CRomBrowser::m_Instance = 0 ;
CRomBrowser* CRomBrowser::CreateInstance()
{
	if (0 == m_Instance)
	{
		m_Instance = new CRomBrowser( ) ;
	}
	return m_Instance ;
}

CRomBrowser* CRomBrowser::GetSingleton( )
{
	return m_Instance ;
}