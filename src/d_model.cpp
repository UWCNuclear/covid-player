#include "d_model.h"

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

d_model::d_model(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, covid_viewer *main) : fitmodel(MotherFrame, w, h, main)
{
    fNPars = 4;

    fInitPars = new Double_t[fNPars];
    fInitLimitsMin = new Double_t[fNPars];
    fInitLimitsMax = new Double_t[fNPars];

    fInitPars[0] = 50;
    fInitPars[1] = 4.;
    fInitPars[2] = 1e-3;

    fInitLimitsMin[0] = 0.;
    fInitLimitsMin[1] = 1.;
    fInitLimitsMin[2] = 1e-6;

    fInitLimitsMax[0] = 1e4;
    fInitLimitsMax[1] = 20.;
    fInitLimitsMax[2] = 1.;

    TGLabel *label;

    /// Fit Peak
    TGGroupFrame *fGroupFrame = new TGGroupFrame(MotherFrame, "Fitting model", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    TGCompositeFrame *fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "a = ");label->SetTextColor(CXred);
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
    label = new TGLabel(fHorizontalFrame, "b = ");label->SetTextColor(CXred);
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
    label = new TGLabel(fHorizontalFrame, "c = ");label->SetTextColor(CXred);
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

d_model::~d_model()
{

}

void d_model::InitParameters()
{
    TH1 *h = fMainWindow->GetHistoToFit();

    fFitFunction = new TF1(Form("D'_%s",h->GetName()),this,&d_model::FitFunction,h->GetXaxis()->GetXmin(),h->GetXaxis()->GetXmax(),fNPars,"fitmodel","FitFunction");
    fFitFunction->SetNpx(1000);
    fFitFunction->SetLineColor(kMagenta);

    fFitFunction->SetParName(0,"a");
    fFitFunction->SetParName(1,"b");
    fFitFunction->SetParName(2,"c");
    fFitFunction->SetParName(3,"T0");

    fFitFunction->SetParameter(0,fNE_a[0]->GetNumber());
    fFitFunction->SetParameter(1,fNE_b[0]->GetNumber());
    fFitFunction->SetParameter(2,fNE_c[0]->GetNumber());

    TH1 *hdummy = fMainWindow->GetDummyHist();
    Int_t bin = hdummy->GetXaxis()->FindFixBin(fFitRange[0]->GetText());
    Float_t T0 = hdummy->GetBinCenter(bin);

    fFitFunction->SetParameter(3,T0);

    fFitFunction->SetParLimits(0,fNE_a[1]->GetNumber(),fNE_a[2]->GetNumber());
    fFitFunction->SetParLimits(1,fNE_b[1]->GetNumber(),fNE_b[2]->GetNumber());
    fFitFunction->SetParLimits(2,fNE_c[1]->GetNumber(),fNE_c[2]->GetNumber());

    fFitFunction->FixParameter(3,T0);
}


Double_t d_model::FitFunction(Double_t*xx,Double_t*pp)
{
    Double_t a1  = pp[0];
    Double_t b1  = pp[1];
    Double_t c1  = pp[2];

    Double_t t0 = pp[3];
    Double_t x   = xx[0] - t0;

    Double_t D1 = a1*TMath::Exp(x/b1)/(1+c1*TMath::Exp(x/b1));

    Double_t Daily = D1;

    return Daily;
}

ClassImp(d_model)
