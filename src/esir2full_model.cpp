#include "esir2full_model.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "TGButton.h"
#include "TGLabel.h"
#include "TSpectrum.h"
#include "TROOT.h"
#include "TLatex.h"
#include "TMath.h"
#include "TF1.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TMatrixDSym.h"
#include "TGListBox.h"
#include "TGClient.h"
#include "TGResourcePool.h"
#include "TSystem.h"
#include "TF1.h"
#include "Fit/Fitter.h"
#include "Fit/BinData.h"
#include "Fit/Chi2FCN.h"
#include "TList.h"
#include "Math/WrappedMultiTF1.h"
#include "HFitInterface.h"
#include "TVirtualFitter.h"

#include "covid_viewer.h"

using namespace std;

esir2full_model::esir2full_model(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, covid_viewer *main) : fitmodel(MotherFrame, w, h, main)
{
    fNPars = 9;

    fInitPars = new Double_t[fNPars];
    fInitLimitsMin = new Double_t[fNPars];
    fInitLimitsMax = new Double_t[fNPars];

    fInitPars[0] = 5e-6;
    fInitPars[1] = 15.;
    fInitPars[2] = 5e-6;
    fInitPars[3] = 5e-6;
    fInitPars[4] = 25.;
    fInitPars[5] = 5e-6;

    fInitPars[6] = 500.;
    fInitPars[7] = 1e-4;

    fInitLimitsMin[0] = 1e-15;
    fInitLimitsMin[1] = 1.;
    fInitLimitsMin[2] = 1e-15;
    fInitLimitsMin[3] = 1e-15;
    fInitLimitsMin[4] = 1.;
    fInitLimitsMin[5] = 1e-15;
    fInitLimitsMin[6] = 1e1;
    fInitLimitsMin[7] = 1e-15;

    fInitLimitsMax[0] = 1e-2;
    fInitLimitsMax[1] = 50.;
    fInitLimitsMax[2] = 1e-2;
    fInitLimitsMax[3] = 1e-2;
    fInitLimitsMax[4] = 50.;
    fInitLimitsMax[5] = 1e-2;
    fInitLimitsMax[6] = 1e7;
    fInitLimitsMax[7] = 1e-1;

    TGLabel *label;

    /// Fit Peak
    TGGroupFrame *fGroupFrame = new TGGroupFrame(MotherFrame, "Fitting model", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    TGCompositeFrame *fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "a  = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_a[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[0], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_a[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_a[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[0], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_a[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_a[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[0], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_a[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "b  = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_b[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[1], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_b[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_b[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[1], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_b[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_b[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[1], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_b[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "c  = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_c[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[2], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_c[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_c[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[2], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_c[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_c[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[2], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_c[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "a' = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_aprime[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[3], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_aprime[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_aprime[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[3], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_aprime[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_aprime[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[3], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_aprime[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "b' = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_bprime[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[4], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_bprime[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_bprime[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[4], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_bprime[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_bprime[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[4], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_bprime[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "c' = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_cprime[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[5], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_cprime[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_cprime[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[5], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_cprime[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_cprime[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[5], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_cprime[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));


    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "a2 = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_a2[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[6], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_a2[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_a2[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[6], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_a2[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_a2[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[6], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_a2[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "b2 = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_b2[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[7], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_b2[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_b2[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[7], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_b2[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_b2[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[7], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_b2[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

//    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
//    label = new TGLabel(fHorizontalFrame, "T0: ");label->SetTextColor(CXred);
//    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
//    fT0_TextEntry = new TGTextEntry(fHorizontalFrame);
//    fHorizontalFrame->AddFrame(fT0_TextEntry,new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
//    label = new TGLabel(fHorizontalFrame, "Fixed: ");label->SetTextColor(CXred);
//    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 10, 3, 0, 0));
//    fFixT0 = new TGCheckButton(fHorizontalFrame);
//    fFixT0->SetState(kButtonDown);
//    fHorizontalFrame->AddFrame(fFixT0,new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));

//    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

//    if(fMainWindow && fMainWindow->GetHistoToFit()) {
//        TH1 *hist = fMainWindow->GetHistoToFit();
//        Int_t bin = hist->FindFirstBinAbove(1);
//        TString name = fMainWindow->GetDummyHist()->GetXaxis()->GetBinLabel(bin);
//        fT0_TextEntry->SetText(name);
//    }

    fGroupFrame = new TGGroupFrame(MotherFrame, "Fit", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *DoFit = new TGTextButton(fHorizontalFrame, "Fit");
    DoFit->Connect("Clicked()", "fitmodel", this, "DoFit()");
    DoFit->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(DoFit,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "From: ");label->SetTextColor(CXblue);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fFitRange[0] = new TGTextEntry(fHorizontalFrame);
    fHorizontalFrame->AddFrame(fFitRange[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    label = new TGLabel(fHorizontalFrame, "To: ");label->SetTextColor(CXblue);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fFitRange[1] = new TGTextEntry(fHorizontalFrame);
    fHorizontalFrame->AddFrame(fFitRange[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fFitRange[0]->Connect("TextChanged(const char *)", "fitmodel", this, "CheckRange()");
    fFitRange[1]->Connect("TextChanged(const char *)", "fitmodel", this, "CheckRange()");

    fGroupFrame = new TGGroupFrame(MotherFrame, "Fit results", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY | kLHintsExpandX, 3, 3, 0, 0));

    fFitResultsBox = new TGListBox(fGroupFrame);
    fGroupFrame->AddFrame(fFitResultsBox, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,-10,-10,0,0));

    HandleMyButton();
}

esir2full_model::~esir2full_model()
{

}

void esir2full_model::InitParameters()
{
    TH1 *h = fMainWindow->GetHistoToFit();

    fFitFunction = new TF1(Form("ESIR2_%s",h->GetName()),this,&esir2full_model::FitFunction,h->GetXaxis()->GetXmin(),h->GetXaxis()->GetXmax(),fNPars,"fitmodel","FitFunction");
    fFitFunction->SetNpx(1000);
    fFitFunction->SetLineColor(kRed);

    fFitFunction->SetParName(0,"a");
    fFitFunction->SetParName(1,"b");
    fFitFunction->SetParName(2,"c");
    fFitFunction->SetParName(3,"a'");
    fFitFunction->SetParName(4,"b'");
    fFitFunction->SetParName(5,"c'");

    fFitFunction->SetParName(6,"a2");
    fFitFunction->SetParName(7,"b2");
    fFitFunction->SetParName(8,"T0");

    fFitFunction->SetParameter(0,fNE_a[0]->GetNumber());
    fFitFunction->SetParameter(1,fNE_b[0]->GetNumber());
    fFitFunction->SetParameter(2,fNE_c[0]->GetNumber());
    fFitFunction->SetParameter(3,fNE_aprime[0]->GetNumber());
    fFitFunction->SetParameter(4,fNE_bprime[0]->GetNumber());
    fFitFunction->SetParameter(5,fNE_cprime[0]->GetNumber());
    fFitFunction->SetParameter(6,fNE_a2[0]->GetNumber());
    fFitFunction->SetParameter(7,fNE_b2[0]->GetNumber());

    TH1 *hdummy = fMainWindow->GetDummyHist();
    Int_t bin = hdummy->GetXaxis()->FindFixBin(fFitRange[0]->GetText());
    Float_t T0 = hdummy->GetBinCenter(bin);

    fFitFunction->SetParameter(5,T0);

    fFitFunction->SetParLimits(0,fNE_a[1]->GetNumber(),fNE_a[2]->GetNumber());
    fFitFunction->SetParLimits(1,fNE_b[1]->GetNumber(),fNE_b[2]->GetNumber());
    fFitFunction->SetParLimits(2,fNE_c[1]->GetNumber(),fNE_c[2]->GetNumber());
    fFitFunction->SetParLimits(3,fNE_aprime[1]->GetNumber(),fNE_aprime[2]->GetNumber());
    fFitFunction->SetParLimits(4,fNE_bprime[1]->GetNumber(),fNE_bprime[2]->GetNumber());
    fFitFunction->SetParLimits(5,fNE_cprime[1]->GetNumber(),fNE_cprime[2]->GetNumber());
    fFitFunction->SetParLimits(6,fNE_a2[1]->GetNumber(),fNE_a2[2]->GetNumber());
    fFitFunction->SetParLimits(7,fNE_b2[1]->GetNumber(),fNE_b2[2]->GetNumber());

    fFitFunction->FixParameter(8,T0);
}


Double_t esir2full_model::FitFunction(Double_t*xx,Double_t*pp)
{
    Double_t a  = pp[0];
    Double_t b  = pp[1];
    Double_t c  = pp[2];

    Double_t ap  = pp[3];
    Double_t bp  = pp[4];
    Double_t cp  = pp[5];

    Double_t a2  = pp[6];
    Double_t b2  = pp[7];

    Double_t t0 = pp[8];
    Double_t x   = xx[0] - t0;

    Double_t r = a/(2*c + TMath::Exp(-x/b)) + ap/(2*cp + TMath::Exp(-x/bp));

    Double_t Daily = a2 * (1 - TMath::Exp(-r/b2) - r);

    return Daily;
}

ClassImp(esir2full_model)
