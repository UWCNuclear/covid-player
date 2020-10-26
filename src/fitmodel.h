#ifndef fitmodel_H
#define fitmodel_H

#include "Riostream.h"

#include "RQ_OBJECT.h"
#include "TGFrame.h"
#include "TArrow.h"
#include "TObject.h"
#include "TGComboBox.h"
#include "TGNumberEntry.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"

using namespace std;

class covid_viewer;
class TF1;
class TH1;
class TGCheckButton;
class TGListBox;

class fitmodel : public  TGVerticalFrame
{
    RQ_OBJECT("fitmodel");

public:

    TGTextEntry *fFitRange[2];

    TFitResultPtr fFitResult;

    Double_t fFitMin=0;
    Double_t fFitMax=0;

    Int_t fNPars=0;
    Double_t *fInitPars = nullptr;
    Double_t *fInitLimitsMin = nullptr;
    Double_t *fInitLimitsMax = nullptr;

protected:

    ULong_t CXred, CXblue, CXblack, CXgreen;

    covid_viewer *fMainWindow = nullptr;

    TH1 *fCurrentHist = nullptr;

    TF1 *fFitFunction = nullptr;
    TH1 *fErrorBand = nullptr;

    TGListBox *fFitResultsBox = nullptr;

public:
    fitmodel(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, covid_viewer *main = nullptr);
    virtual ~fitmodel();

    virtual void InitParameters() {;}

//    virtual void HandleMouse(Int_t EventType,Int_t EventX,Int_t EventY, TObject* selected);
    virtual void HandleMyButton();

    virtual TGListBox *GetFitResultsBox(){return fFitResultsBox;}
    virtual void PrintInListBox(TString mess, Int_t Type);
    virtual void PrintResults();

    virtual covid_viewer *GetMainWindow(){return fMainWindow;}
    virtual void SetMainWindow(covid_viewer *w);

    virtual void DoFit();
    void CheckRange();
    void UpdateRange();

    ClassDef(fitmodel,0);
};

#endif
