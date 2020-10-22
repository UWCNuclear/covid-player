#include "TGFrame.h"
#include "TGNumberEntry.h"
#include "TRootEmbeddedCanvas.h"
#include "TGStatusBar.h"
#include "TGLabel.h"
#include "KeySymbols.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "Riostream.h"
#include "TMarker.h"
#include "TGComboBox.h"
#include "TGNumberEntry.h"
#include "TH1D.h"
#include "TGColorSelect.h"
#include "TGMenu.h"
#include "minimizer.h"

#include <vector>

using namespace std;

class dprime_model;
class dprime2_model;
class dprime2full_model;
class esir_model;
class esir2_model;
class esir2full_model;
class d_model;
class d2_model;
class d2full_model;

class TGTab;

class covid_viewer: public TGMainFrame
{
private:
    enum ButtonsTypes {kButtonTotal, kButtonDaily};
    enum ETestCommandIdentifiers { M_FILE_EXIT, M_FILE_SAVE, M_UPDATE, M_MINIMIZER};

private:

    TGMenuBar *fMenuBar = nullptr;
    TGPopupMenu *fMenuFile = nullptr;
    TGPopupMenu *fMenuData = nullptr;
    TGPopupMenu *fMenuOptions = nullptr;

    TRootEmbeddedCanvas *fRootCanvas;
    TCanvas *fCanvas;

    TGComboBox *fTypeBox = nullptr;
    TGComboBox *fCountryBox = nullptr;
    TGRadioButton *fTotal = nullptr;
    TGRadioButton *fDaily = nullptr;

    TGNumberEntry *fSmoothEntry;

    TGNumberEntry *fDateMin = nullptr;
    TGComboBox *fMounthMin = nullptr;
    TGComboBox *fYearMin = nullptr;
    TGNumberEntry *fDateMax = nullptr;
    TGComboBox *fMounthMax = nullptr;
    TGComboBox *fYearMax = nullptr;

    TGColorSelect *fColorSel = nullptr;

    TGCheckButton *fDrawSameButton = nullptr;

    TGComboBox *fHistotofitBox = nullptr;
    TGComboBox *fFittingModelBox = nullptr;

    UInt_t fSmooth = 1;

    TString fFitUpTo="";

    vector<TString> vDates;
    vector<Double_t> vDeaths;
    vector<Double_t> vDeaths_e;
    vector<Double_t> vDeaths_Tot;
    vector<Double_t> vDeaths_Tot_e;

    TString DateMinStr="1-Mar-20";
    TString DateMaxStr="1-Jan-21";

    Int_t DateMin=0;
    Int_t DateMax=0;


    TH1D *fDataDailyHist = nullptr;
    TH1D *fDataTotalHist = nullptr;
    TH1D *fDummyHist = nullptr;
    TH1D *fCurrentHist = nullptr;

    Int_t fLastEventType;
    Int_t fLastEventX;
    Int_t fLastEventY;
    TObject *fLastSelected;
    TGStatusBar *fStatusBar;

    Double_t fLastXPosition;
    Double_t fLastYPosition;

    Bool_t fCTRL = false;

    TString  fSavedAs="";

    TH1 * fHistoToFit = nullptr;

    TGTab *fMainTab = nullptr;

    TGCompositeFrame *fDModelTab;
    d_model *fD_Model = nullptr;
    TGCompositeFrame *fD2ModelTab;
    d2_model *fD2_Model = nullptr;
    TGCompositeFrame *fD2FullModelTab;
    d2full_model *fD2Full_Model = nullptr;

    TGCompositeFrame *fDprimeModelTab;
    dprime_model *fDPrime_Model = nullptr;
    TGCompositeFrame *fDprime2ModelTab;
    dprime2_model *fDPrime2_Model = nullptr;
    TGCompositeFrame *fDprime2FullModelTab;
    dprime2full_model *fDPrime2Full_Model = nullptr;
    TGCompositeFrame *fESIRModelTab;
    esir_model *fESIR_Model = nullptr;
    TGCompositeFrame *fESIR2ModelTab;
    esir2_model *fESIR2_Model = nullptr;
    TGCompositeFrame *fESIR2FullModelTab;
    esir2full_model *fESIR2Full_Model = nullptr;

    minimizer *fminimizer = nullptr;

public:

    covid_viewer(const TGWindow *p, UInt_t w, UInt_t h);
    virtual ~covid_viewer();

    void ProcessedKeyEvent(Event_t *event);
    void HandleMovement(Int_t EventType, Int_t EventX, Int_t EventY, TObject *selected);
    void HandleButtons(Int_t id=-1);
    void SetSmoothing();
    void PlotData();
    bool ReadData();
    void HandleDateRange();
    void HandleMounthRange();
    void HandleHistoToFit();
    void HandleMenu(Int_t id);
    void SaveCanvasAs();
    TH1 *GetHistoToFit(){return fHistoToFit;}
    TH1 *GetDummyHist(){return fDummyHist;}
    TCanvas *GetCanvas(){return fCanvas;}

    void SetMinimizerWindow(minimizer *_minimizer ) {fminimizer = _minimizer;}
    minimizer *GetMinimizerWindow(){return fminimizer;}

    void HandleModels();
    void ToggleTab(TGCompositeFrame *tab, const char * name);

    void DoClear();

    TString GetDateMin(){return DateMinStr;}
    TString GetDateMax(){return DateMaxStr;}

private:

    void UpdateCountryList();
    void SmoothVector(Int_t smooth, vector<double> &data, vector<double> &data_err);

    ClassDef(covid_viewer,0)
};
