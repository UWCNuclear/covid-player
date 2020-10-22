#include "minimizer.h"

#include "TGButton.h"
#include "TGNumberEntry.h"
#include "TGLabel.h"
#include "TSystem.h"
#include "covid_viewer.h"
#include "Fit/Fitter.h"

minimizer::minimizer(const TGWindow *p, const TGWindow *m, UInt_t w, UInt_t h) :
    TGMainFrame(p, w, h)
{
    fMain = const_cast<covid_viewer *>(static_cast < covid_viewer const * >(m));

    SetWindowName("Minimizer options");
    SetLayoutManager(new TGVerticalLayout(this));

    TGGroupFrame *fGroupFrame = new TGGroupFrame(this, "minimizer options", kVerticalFrame);
    AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 3, 3, 0, 0));

    TGCompositeFrame *fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Minimizer"), new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,10,0,0));
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Algorithm"), new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,10,10,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fMinimizer = new TGComboBox(fHorizontalFrame);
    fMinimizer->Resize(200,20);
    fMinimizer->Connect("Selected(Int_t)", "minimizer", this, "UpdateMinimizer()");
    fHorizontalFrame->AddFrame(fMinimizer, new TGLayoutHints( kLHintsCenterY  | kLHintsExpandX ,10,10,0,0));
    fAlgorithm = new TGComboBox(fHorizontalFrame);
    fAlgorithm->Resize(200,20);
    fHorizontalFrame->AddFrame(fAlgorithm, new TGLayoutHints( kLHintsCenterY  | kLHintsExpandX ,10,10,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fMinimizer->AddEntry("Minuit",0);
    fMinimizer->Select(0);
    TString Lib = "libMinuit2";
    if(gSystem->FindDynamicLibrary(Lib)) {
        fMinimizer->AddEntry("Minuit2",1);
        fMinimizer->Select(1);
    }
    fMinimizer->AddEntry("Fumili",2);
    Lib = "libMathMore";
    if(gSystem->FindDynamicLibrary(Lib)) {
        fMinimizer->AddEntry("GSLMultiMin",3);
        fMinimizer->AddEntry("GSLMultiFit",4);
        fMinimizer->AddEntry("GSLSimAn",5);
    }
    Lib = "libGenetic";
    if(gSystem->FindDynamicLibrary(Lib))
        fMinimizer->AddEntry("Genetic",6);

    fMinimizer->Connect("Selected(Int_t)", "minimizer", this, "ApplyMinimizer()");
    fAlgorithm->Connect("Selected(Int_t)", "minimizer", this, "ApplyMinimizer()");

    fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Tolerance"), new TGLayoutHints(kLHintsBottom | kLHintsLeft,10,10,0,0));
    fTolenrance = new TGNumberEntry(fHorizontalFrame, 0.05, 4, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative);
    fTolenrance->Connect("ValueSet(Long_t)", "minimizer", this, "ApplyMinimizer()");
    fHorizontalFrame->AddFrame(fTolenrance,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,10,10,0,0));


    fHorizontalFrame->AddFrame(new TGLabel(fHorizontalFrame, "Print Level"), new TGLayoutHints(kLHintsBottom | kLHintsLeft,10,10,0,0));
    fPrintLevel = new TGNumberEntry(fHorizontalFrame, 0, 4, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);
    fPrintLevel->Connect("ValueSet(Long_t)", "minimizer", this, "ApplyMinimizer()");
    fHorizontalFrame->AddFrame(fPrintLevel,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,10,10,0,0));

    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    ApplyMinimizer();

    Layout();
    MapSubwindows();
    MapWindow();
}

minimizer::~minimizer() {
    Cleanup();
    fMain->SetMinimizerWindow(nullptr);
    // all object added with AddFrame are already removed with Cleanup
}

void minimizer::CloseWindow()
{
    UnmapWindow();
    DeleteWindow();
}

void minimizer::UpdateMinimizer()
{
    fAlgorithm->RemoveAll();

    if(fMinimizer->GetSelected() == 0) {
        fAlgorithm->AddEntry("Migrad",0);
        fAlgorithm->AddEntry("Simplex",1);
        fAlgorithm->AddEntry("Minimize",2);
        fAlgorithm->AddEntry("MigradImproved",3);
        fAlgorithm->AddEntry("Scan",4);
        fAlgorithm->AddEntry("Seek",5);
        fAlgorithm->Select(0);
    }
    if(fMinimizer->GetSelected() == 1) {
        fAlgorithm->AddEntry("Migrad",0);
        fAlgorithm->AddEntry("Simplex",1);
        fAlgorithm->AddEntry("Minimize",2);
        fAlgorithm->AddEntry("Scan",3);
        fAlgorithm->AddEntry("Fumili",4);
        fAlgorithm->Select(0);
    }
    if(fMinimizer->GetSelected() == 2) {
        fAlgorithm->AddEntry("Fumili",0);
        fAlgorithm->Select(0);
    }
    if(fMinimizer->GetSelected() == 3) {
        fAlgorithm->AddEntry("BFGS2",0);
        fAlgorithm->AddEntry("BFGS",1);
        fAlgorithm->AddEntry("ConjugateFR",2);
        fAlgorithm->AddEntry("ConjugatePR",3);
        fAlgorithm->AddEntry("SteepestDescent",4);
        fAlgorithm->Select(0);
    }
    if(fMinimizer->GetSelected() == 4) {
        fAlgorithm->AddEntry("GSLMultiFit",0);
        fAlgorithm->Select(0);
    }
    if(fMinimizer->GetSelected() == 5) {
        fAlgorithm->AddEntry("GSLSimAn",0);
        fAlgorithm->Select(0);
    }
    if(fMinimizer->GetSelected() == 6) {
        fAlgorithm->AddEntry("Genetic",0);
        fAlgorithm->Select(0);
    }
}

void minimizer::ApplyMinimizer()
{
    // Minimizer definition
    ROOT::Math::MinimizerOptions::SetDefaultMinimizer(GetMinimizer(),GetAlgorithm());
    //    ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(2147483647);
    //    ROOT::Math::MinimizerOptions::SetDefaultErrorDef(2);
    ROOT::Math::MinimizerOptions::SetDefaultTolerance(GetTolerance());
    //    ROOT::Math::MinimizerOptions::SetDefaultPrecision(1e-9);
    ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(GetPrintLevel());

}

ClassImp(minimizer)
