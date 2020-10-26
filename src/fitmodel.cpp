#include "fitmodel.h"

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
#include "TGraphErrors.h"

#include "covid_viewer.h"

using namespace std;

fitmodel::fitmodel(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, covid_viewer *main) : TGVerticalFrame(MotherFrame, w, h, kFixedWidth)
{
    fMainWindow = main;

    gClient->GetColorByName("red",  CXred);
    gClient->GetColorByName("blue", CXblue);
    gClient->GetColorByName("black",CXblack);
    gClient->GetColorByName("green",CXgreen);
}

fitmodel::~fitmodel()
{
}

void fitmodel::SetMainWindow(covid_viewer *w)
{
    fMainWindow = w;
}

void fitmodel::HandleMyButton()
{

}

void fitmodel::PrintInListBox(TString mess, Int_t Type)
{
#if (OS_TYPE == OS_LINUX)
    const TGFont *ufont;         // will reflect user font changes
    ufont = gClient->GetFont("-*-courier-medium-r-*-*-12-*-*-*-*-*-iso8859-1");
    // ufont = gClient->GetFont("-adobe-times-medium-r-*-*-12-*-*-*-*-*-iso8859-1");
    if (!ufont)
        ufont = fClient->GetResourcePool()->GetDefaultFont();

    TGGC   *uGC;           // will reflect user GC changes
    // graphics context changes
    GCValues_t val;
    val.fMask = kGCFont;
    val.fFont = ufont->GetFontHandle();
    uGC = gClient->GetGC(&val, kTRUE);

    TGTextLBEntry *entry = new TGTextLBEntry(fFitResultsBox->GetContainer(), new TGString(mess), fFitResultsBox->GetNumberOfEntries()+1, uGC->GetGC(), ufont->GetFontStruct());
#else
    TGTextLBEntry *entry = new TGTextLBEntry(fFitResultsBox->GetContainer(), new TGString(mess), fFitResultsBox->GetNumberOfEntries()+1);
#endif

    if(Type == kError)
        entry->SetBackgroundColor((Pixel_t)0xff0000);
    else if(Type == kInfo)
        entry->SetBackgroundColor((Pixel_t)0x87a7d2);
    else if(Type == kWarning)
        entry->SetBackgroundColor((Pixel_t)0xdfdf44);
    else if(Type == kPrint)
        entry->SetBackgroundColor((Pixel_t)0x90f269);

    fFitResultsBox->AddEntry((TGLBEntry *)entry, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX));
    fFitResultsBox->Layout();
}

void fitmodel::DoFit()
{
    if(fMainWindow == nullptr) return;
    TH1 *h = fMainWindow->GetHistoToFit();
    if(h == nullptr) return;

    delete fFitFunction;
    delete fErrorBand;

    InitParameters();

    TH1 *hdummy = fMainWindow->GetDummyHist();

    fFitMin = h->GetXaxis()->GetBinLowEdge(hdummy->GetXaxis()->FindFixBin(fFitRange[0]->GetText()));
    fFitMax = h->GetXaxis()->GetBinUpEdge(hdummy->GetXaxis()->FindFixBin(fFitRange[1]->GetText()));

    // Store data to fit in a TGraph

    TGraphErrors *g = new TGraphErrors;
    Float_t xMax=0.;
    Float_t yMax=0.;

    for(int i=1 ; i<=hdummy->GetNbinsX() ; i++) {
        if(h->GetBinContent(i)==0) continue;
//        if( (i==1) || (i==hdummy->GetNbinsX()) ) {
//            g->SetPoint(g->GetN(),hdummy->GetBinCenter(i),h->GetBinContent(i));
//            g->SetPointError(g->GetN()-1,0.,h->GetBinError(i));
//        }
        else if((i >= hdummy->GetXaxis()->FindFixBin(fFitRange[0]->GetText())) && (i <= hdummy->GetXaxis()->FindFixBin(fFitRange[1]->GetText()))) {
            g->SetPoint(g->GetN(),hdummy->GetBinCenter(i),h->GetBinContent(i));
            g->SetPointError(g->GetN()-1,0.,h->GetBinError(i));

            if(h->GetBinContent(i)>yMax) {
                yMax = h->GetBinContent(i);
                xMax = hdummy->GetBinCenter(i);
            }
        }
    }

    // Add a dummy point at 5* the current max (to force to be at 0 for t infinity)

    g->SetPoint(g->GetN(),5*xMax,0.);
    g->SetPointError(g->GetN()-1,0.,0.1);

    fFitFunction->FixParameter(fFitFunction->GetNpar()-1,g->GetX()[0]);

    fFitResult = g->Fit(fFitFunction,"S0");

//    g->SetMarkerStyle(20);
//    g->SetMarkerColor(kBlue);
//    g->Draw("ape");

    fFitFunction->Draw("same");

    /*Create a histogram to hold the confidence intervals*/
    fErrorBand = (TH1*)h->Clone();
    fErrorBand->Reset();
    fErrorBand->SetName(((TString)h->GetName()).Append("_error"));
    (TVirtualFitter::GetFitter())->GetConfidenceIntervals(fErrorBand);

//    delete g;

    //Now the "hint" histogram has the fitted function values as the
    //bin contents and the confidence intervals as bin errors
    fErrorBand->SetStats(kFALSE);
    fErrorBand->SetFillColor(fFitFunction->GetLineColor());
    fErrorBand->SetFillStyle(3002);
    fErrorBand->SetFillColorAlpha(fFitFunction->GetLineColor(),0.5);
    fErrorBand->SetMarkerSize(0);
    fErrorBand->Draw("e3 same");

    fMainWindow->GetCanvas()->Modified();
    fMainWindow->GetCanvas()->Update();

    PrintResults();
}

void fitmodel::PrintResults()
{
    fFitResultsBox->RemoveAll();
    ostringstream text;

    text << "Fit results :";
    cout<<text.str()<<endl;PrintInListBox(text.str(),kPrint);text.str("");
    text << "Status: "  <<  fFitResult->Status() << " -- ";
    if(fFitResult->Status()<3)
        text << " Successeful" << endl;
    else
        text << " Failed" << endl;
    cout<<text.str();
    if(fFitResult->Status()<3)
        PrintInListBox(text.str(),kPrint);
    else
        PrintInListBox(text.str(),kError);
    text.str("");

    text << "Chi2/Ndf  = "<< fFitResult->Chi2()/fFitResult->Ndf();
    cout<<text.str()<<endl;PrintInListBox(text.str(),kInfo);text.str("");

    for(int i=0 ; i<fFitFunction->GetNpar() ; i++) {
        double val = fFitFunction->GetParameter(i);
        double err = fFitFunction->GetParError(i);
        TString name = fFitFunction->GetParName(i);
        if(name!="T0") text<<left<<setw(11)<<name<<": "<<setprecision(4)<<setw(10)<<val<<" ("<<setprecision(7)<<setw(10)<<err<<")";
        else {
            int bin = fMainWindow->GetDummyHist()->GetXaxis()->FindBin(val);
            TString date = fMainWindow->GetDummyHist()->GetXaxis()->GetBinLabel(bin);
            text<<left<<setw(11)<<name<<": "<<setprecision(2)<<setw(6)<<val<<" [" << date << "] ("<<setprecision(7)<<setw(10)<<err<<")";
        }
        cout<<text.str()<<endl;
        PrintInListBox(text.str(),kInfo);
        text.str("");
    }
}

void fitmodel::UpdateRange()
{
    if(fMainWindow == nullptr) return;

    fFitRange[0]->SetText(fMainWindow->fFitMinGlobal);
    fFitRange[1]->SetText(fMainWindow->fFitMaxGlobal);
}

void fitmodel::CheckRange()
{
    if(fMainWindow == nullptr) return;

    TH1 *hdummy = fMainWindow->GetDummyHist();

    if(hdummy->GetXaxis()->FindFixBin(fFitRange[0]->GetText())>0) {
        fFitRange[0]->SetTextColor(CXblue);
        fMainWindow->fFitMinGlobal = fFitRange[0]->GetText();
    }
    else fFitRange[0]->SetTextColor(CXred);

    if(hdummy->GetXaxis()->FindFixBin(fFitRange[1]->GetText())>0) {
        fFitRange[1]->SetTextColor(CXblue);
        fMainWindow->fFitMaxGlobal = fFitRange[1]->GetText();
    }
    else fFitRange[1]->SetTextColor(CXred);
}

ClassImp(fitmodel)
