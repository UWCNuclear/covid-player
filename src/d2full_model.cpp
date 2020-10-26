#include "d2full_model.h"

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

d2full_model::d2full_model(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, covid_viewer *main) : fitmodel(MotherFrame, w, h, main)
{
    fNPars = 7;

    fInitPars = new Double_t[fNPars];
    fInitLimitsMin = new Double_t[fNPars];
    fInitLimitsMax = new Double_t[fNPars];

    fInitPars[0] = 50;
    fInitPars[1] = 4.;
    fInitPars[2] = 1e-3;
    fInitPars[3] = 50.;
    fInitPars[4] = 7;
    fInitPars[5] = 1e-3;

    fInitLimitsMin[0] = 0.;
    fInitLimitsMin[1] = 1.;
    fInitLimitsMin[2] = 1e-6;
    fInitLimitsMin[3] = 0.;
    fInitLimitsMin[4] = 1.;
    fInitLimitsMin[5] = 1e-6;

    fInitLimitsMax[0] = 1e4;
    fInitLimitsMax[1] = 50.;
    fInitLimitsMax[2] = 1.;
    fInitLimitsMax[3] = 1e4;
    fInitLimitsMax[4] = 50.;
    fInitLimitsMax[5] = 1.;

    TGLabel *label;

    /// Fit Peak
    TGGroupFrame *fGroupFrame = new TGGroupFrame(MotherFrame, "Fitting model", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 3, 0, 0));

    TGCompositeFrame *fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "a1 = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_a1[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[0], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_a1[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_a1[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[0], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_a1[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_a1[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[0], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_a1[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "b1 = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_b1[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[1], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_b1[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_b1[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[1], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_b1[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_b1[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[1], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_b1[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "c1 = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_c1[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[2], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_c1[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_c1[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[2], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_c1[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_c1[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[2], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_c1[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "a2 = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_a2[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[3], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_a2[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_a2[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[3], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_a2[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_a2[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[3], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_a2[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "b2 = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_b2[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[4], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_b2[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_b2[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[4], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_b2[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_b2[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[4], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_b2[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    label = new TGLabel(fHorizontalFrame, "c = ");label->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(label,new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_c2[0] = new TGNumberEntry(fHorizontalFrame, fInitPars[5], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_c2[0],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Min"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_c2[1] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMin[5], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_c2[1],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Max"),new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 3, 0, 0));
    fNE_c2[2] = new TGNumberEntry(fHorizontalFrame, fInitLimitsMax[5], 7,0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive ,TGNumberFormat::kNELNoLimits);
    fHorizontalFrame->AddFrame(fNE_c2[2],new TGLayoutHints(kLHintsCenterY | kLHintsLeft| kLHintsExpandX,0,0,0,0));
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

d2full_model::~d2full_model()
{

}

void d2full_model::InitParameters()
{
    TH1 *h = fMainWindow->GetHistoToFit();

    fFitFunction = new TF1(Form("D'2Full_%s",h->GetName()),this,&d2full_model::FitFunction,h->GetXaxis()->GetXmin(),h->GetXaxis()->GetXmax(),fNPars,"fitmodel","FitFunction");
    fFitFunction->SetNpx(1000);

    fFitFunction->SetParName(0,"a1");
    fFitFunction->SetParName(1,"b1");
    fFitFunction->SetParName(2,"c1");
    fFitFunction->SetParName(3,"a2");
    fFitFunction->SetParName(4,"b2");
    fFitFunction->SetParName(5,"c2");
    fFitFunction->SetParName(6,"T0");

    fFitFunction->SetParameter(0,fNE_a1[0]->GetNumber());
    fFitFunction->SetParameter(1,fNE_b1[0]->GetNumber());
    fFitFunction->SetParameter(2,fNE_c1[0]->GetNumber());
    fFitFunction->SetParameter(3,fNE_a2[0]->GetNumber());
    fFitFunction->SetParameter(4,fNE_b2[0]->GetNumber());
    fFitFunction->SetParameter(5,fNE_c2[0]->GetNumber());

    TH1 *hdummy = fMainWindow->GetDummyHist();
    Int_t bin = hdummy->GetXaxis()->FindFixBin(fFitRange[0]->GetText());
    Float_t T0 = hdummy->GetBinCenter(bin);

    fFitFunction->SetParameter(6,T0);

    fFitFunction->SetParLimits(0,fNE_a1[1]->GetNumber(),fNE_a1[2]->GetNumber());
    fFitFunction->SetParLimits(1,fNE_b1[1]->GetNumber(),fNE_b1[2]->GetNumber());
    fFitFunction->SetParLimits(2,fNE_c1[1]->GetNumber(),fNE_c1[2]->GetNumber());
    fFitFunction->SetParLimits(3,fNE_a2[1]->GetNumber(),fNE_a2[2]->GetNumber());
    fFitFunction->SetParLimits(4,fNE_b2[1]->GetNumber(),fNE_b2[2]->GetNumber());
    fFitFunction->SetParLimits(5,fNE_c2[1]->GetNumber(),fNE_c2[2]->GetNumber());

    fFitFunction->FixParameter(6,T0);
}


Double_t d2full_model::FitFunction(Double_t*xx,Double_t*pp)
{
    Double_t a1  = pp[0];
    Double_t b1  = pp[1];
    Double_t c1  = pp[2];

    Double_t a2  = pp[3];
    Double_t b2  = pp[4];
    Double_t c2  = pp[5];

    Double_t t0 = pp[6];
    Double_t x   = xx[0] - t0;

    Double_t D1 = a1*TMath::Exp(x/b1)/(1+c1*TMath::Exp(x/b1));
    Double_t D2 = a2*TMath::Exp(x/b2)/(1+c2*TMath::Exp(x/b2));

    Double_t Daily = D1+D2;

    return Daily;
}

ClassImp(d2full_model)
