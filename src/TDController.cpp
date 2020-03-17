// TDController.cpp: implementation of the TDController class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#if !defined(TDCONTROLLER_H)
    #include "TDController.h"
#endif

/*
#if !defined(BFFILEHELPER_H)
	#include "BFFileHelper.h"
#endif

char runningChars [] = {'-', '\\', '|', '/'};
CDCDebug* debugObject = new CDCDebug();
*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTDController::CTDController(LPCTSTR rawDataFile, 
                             LPCTSTR attributesFile,
                             LPCTSTR nameFile, 
                             LPCTSTR supFile,
                             LPCTSTR transformedDataFile, 
                             LPCTSTR transformedTestFile, 
                             LPCTSTR transformedSVMDataFile, 
                             LPCTSTR transformedSVMTestFile,
                             bool bIncludeNonVA, 
                             int  defaultK,
                             int  nInputRecs,
                             int  nTraining)
    : m_attribMgr(attributesFile, nameFile, supFile, bIncludeNonVA), 
      m_virtualMgr(attributesFile, defaultK),
      m_dataMgr(rawDataFile, transformedDataFile, transformedTestFile, transformedSVMDataFile, transformedSVMTestFile, bIncludeNonVA, nInputRecs, nTraining)
{
    if (!m_virtualMgr.initialize(&m_attribMgr))
        ASSERT(false);
    if (!m_dataMgr.initialize(&m_attribMgr))
        ASSERT(false);
    if (!m_partitioner.initialize(&m_attribMgr, &m_virtualMgr, &m_dataMgr))
        ASSERT(false);
    if (!m_evalMgr.initialize(&m_attribMgr, &m_partitioner))
        ASSERT(false);
}

CTDController::~CTDController()
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CTDController::runTopDown()
{	
    cout << _T("***************************************") << endl;
    cout << _T("* Privacy Data Transformer - Top Down *") << endl;
    cout << _T("***************************************") << endl;

	printTime();
    if (!m_attribMgr.readAttributes())
        return false;

    printTime();
    if (!m_virtualMgr.readVirtualIDs())
        return false;

    printTime();
    if (!m_dataMgr.readRecords())
        return false;

#ifndef _TD_C45_SVM_DATA_CONVERTER
    printTime();
    if (!m_partitioner.transformData())
        return false;

    printTime();
    if (!m_dataMgr.writeRecords(false))
    //if (!m_dataMgr.writeRecords(false, 40700))
        return false;
#endif

    printTime();
    if (!m_attribMgr.writeNameFile())
        return false;

    //printTime();
    //if (!m_dataMgr.writeRecordsSVM())
    //    return false;

#ifdef _TD_SCORE_FUNTION_TRANSACTION
    if (!m_attribMgr.writeSupFile())
        return false;
#endif

    int catDistortion = 0;
    float contDistortion = 0.0f;
    if (!m_evalMgr.countNumDistortions(catDistortion, contDistortion)) {
        ASSERT(false);
        return false;
    }
    cout << _T("Distortion for categorical: ") << catDistortion << endl;
    cout << _T("Distortion for continuous: ") << contDistortion << endl;


    int catDiscern = 0;
    if (!m_evalMgr.countNumDiscern(catDiscern)) {
        ASSERT(false);
        return false;
    }
    cout << _T("Discernibility for categorical: ") << catDiscern << endl;

    float catPrecision = 0.0f;
    if (!m_evalMgr.calPrecision(catPrecision)) {
        ASSERT(false);
        return false;
    }
    cout << _T("Precision for categorical: ") << catPrecision << endl;

    printTime();
    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CTDController::removeUnknowns()
{	
    cout << _T("****************************************") << endl;
    cout << _T("* Removing Records With Unknown Values *") << endl;
    cout << _T("****************************************") << endl;
    
    if (!m_attribMgr.readAttributes())
        return false;

    if (!m_dataMgr.readRecords())
        return false;

    if (!m_dataMgr.writeRecords(true))
        return false;

    return true;
}