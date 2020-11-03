#include "covid_viewer.h"
#include "TSystem.h"
#include "TAxis.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TSystemDirectory.h"
#include "TGButton.h"
#include "TMath.h"
#include "TFrame.h"
#include "TApplication.h"
#include "TGMsgBox.h"
#include "TGFileDialog.h"
#include "TGTab.h"
#include "TF1.h"
#include "TVirtualX.h"

#include "dprime_model.h"
#include "dprime2_model.h"
#include "dprime2full_model.h"
#include "esir_model.h"
#include "esir2_model.h"
#include "esir2full_model.h"

#include "d_model.h"
#include "d2_model.h"
#include "d2full_model.h"


covid_viewer::covid_viewer(const TGWindow *p, UInt_t w, UInt_t h): TGMainFrame(p, w, h)
{
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);

    ULong_t CXred, CXblue, CXblack, CXgreen;
    gClient->GetColorByName("red",  CXred);
    gClient->GetColorByName("blue", CXblue);
    gClient->GetColorByName("black",CXblack);
    gClient->GetColorByName("green",CXgreen);

    const TGPicture* pict_exit = gClient->GetPicture("bld_exit.png");
    const TGPicture* pict_saveas = gClient->GetPicture("bld_save.png");

    fMenuFile = new TGPopupMenu(gClient->GetRoot());
    fMenuFile->Connect("Activated(Int_t)", "covid_viewer", this, "HandleMenu(Int_t)");
    fMenuFile->AddEntry("S&ave As", M_FILE_SAVE, 0, pict_saveas);
    fMenuFile->AddEntry("E&xit", M_FILE_EXIT, 0, pict_exit);

    fMenuData = new TGPopupMenu(gClient->GetRoot());
    fMenuData->Connect("Activated(Int_t)", "covid_viewer", this, "HandleMenu(Int_t)");
    fMenuData->AddEntry("Update", M_UPDATE);

    fMenuOptions = new TGPopupMenu(gClient->GetRoot());
    fMenuOptions->Connect("Activated(Int_t)", "covid_viewer", this, "HandleMenu(Int_t)");
    fMenuOptions->AddEntry("Minimizer", M_MINIMIZER);
    fMenuOptions->AddEntry("Ratio to population", M_POP_RATIO);

    fMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);
    this->AddFrame(fMenuBar, new TGLayoutHints(kLHintsExpandX));
    fMenuBar->AddPopup("File", fMenuFile, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));
    fMenuBar->AddPopup("Data", fMenuData, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));
    fMenuBar->AddPopup("Options", fMenuOptions, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0));

    TGCompositeFrame *Toolbar = new TGCompositeFrame(this,600,100,kHorizontalFrame);
    AddFrame(Toolbar,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,10,10,0,-5));

    TGGroupFrame *gFrame = new TGGroupFrame(Toolbar, "Tools", kVerticalFrame);
    gFrame->SetTextColor(CXblue);
    gFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    Toolbar->AddFrame(gFrame, new TGLayoutHints( kLHintsExpandX  | kLHintsExpandY , 0, 0, 0, -5) );

    TGHorizontalFrame *hframe = new TGHorizontalFrame(gFrame);
    gFrame->AddFrame(hframe, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,-10,-5,0,-2));

    fTypeBox = new TGComboBox(hframe);
    fTypeBox->Resize(80,20);
    fTypeBox->AddEntry("Cases",0);
    fTypeBox->AddEntry("Deaths",1);
    fTypeBox->Select(1);
    fTypeBox->Connect("Selected(Int_t)", "covid_viewer", this, "ReadData()");
    hframe->AddFrame(fTypeBox, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,0,0,0,0));

    fCountryBox = new TGComboBox(hframe);
    fCountryBox->Resize(200,20);
    UpdateCountryList();
    fCountryBox->Select(0);
    fCountryBox->Connect("Selected(Int_t)", "covid_viewer", this, "ReadData()");
    hframe->AddFrame(fCountryBox, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,0,0,0,0));

    TGLabel *label = new TGLabel(hframe,"Type: ");
    label->SetTextColor(CXred);
    hframe->AddFrame(label, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,10,5,0,0));

    fTotal = new TGRadioButton(hframe,"Total",kButtonTotal);
    hframe->AddFrame(fTotal, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,5,0,0,0));
    fTotal->Connect("Clicked()", "covid_viewer", this, "HandleButtons()");

    fDaily = new TGRadioButton(hframe,"Daily",kButtonDaily);
    hframe->AddFrame(fDaily, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,5,0,0,0));
    fDaily->Connect("Clicked()", "covid_viewer", this, "HandleButtons()");
    fDaily->SetState(kButtonDown);

    label = new TGLabel(hframe,"Smooth: ");
    label->SetTextColor(CXred);
    hframe->AddFrame(label, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,10,5,0,0));
    fSmoothEntry = new TGNumberEntry(hframe,7,5,-1,TGNumberFormat::kNESInteger,TGNumberFormat::kNEAPositive,TGNumberFormat::kNELLimitMin,1);
    fSmoothEntry->Connect("ValueSet(Long_t)", "covid_viewer", this, "SetSmoothing()");
    hframe->AddFrame(fSmoothEntry, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,5,0,0,0));
    SetSmoothing();

    label = new TGLabel(hframe,"Date min: ");
    label->SetTextColor(CXred);
    hframe->AddFrame(label, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,10,5,0,0));

    fDateMin = new TGNumberEntry(hframe,1,5,-1,TGNumberFormat::kNESInteger,TGNumberFormat::kNEAPositive,TGNumberFormat::kNELLimitMinMax,1,31);
    hframe->AddFrame(fDateMin, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,5,0,0,0));
    fDateMin->Connect("ValueSet(Long_t)", "covid_viewer", this, "HandleDateRange()");

    fMounthMin = new TGComboBox(hframe);
    fMounthMin->Resize(100,20);
    fMounthMin->AddEntry("January",1);
    fMounthMin->AddEntry("February",2);
    fMounthMin->AddEntry("March",3);
    fMounthMin->AddEntry("April",4);
    fMounthMin->AddEntry("May",5);
    fMounthMin->AddEntry("June",6);
    fMounthMin->AddEntry("July",7);
    fMounthMin->AddEntry("August",8);
    fMounthMin->AddEntry("September",9);
    fMounthMin->AddEntry("October",10);
    fMounthMin->AddEntry("November",11);
    fMounthMin->AddEntry("December",12);
    fMounthMin->Select(3);
    hframe->AddFrame(fMounthMin, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,0,0,0,0));
    fMounthMin->Connect("Selected(Int_t)", "covid_viewer", this, "HandleMounthRange()");
    fMounthMin->Connect("Selected(Int_t)", "covid_viewer", this, "HandleDateRange()");

    fYearMin = new TGComboBox(hframe);
    fYearMin->Resize(60,20);
    fYearMin->AddEntry("2020",1);
    fYearMin->AddEntry("2021",2);
    fYearMin->Select(1);
    hframe->AddFrame(fYearMin, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,0,0,0,0));
    fYearMin->Connect("Selected(Int_t)", "covid_viewer", this, "HandleMounthRange()");
    fYearMin->Connect("Selected(Int_t)", "covid_viewer", this, "HandleDateRange()");

    label = new TGLabel(hframe,"Date max: ");
    label->SetTextColor(CXred);
    hframe->AddFrame(label, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,10,5,0,0));

    fDateMax = new TGNumberEntry(hframe,1,5,-1,TGNumberFormat::kNESInteger,TGNumberFormat::kNEAPositive,TGNumberFormat::kNELLimitMinMax,1,31);
    hframe->AddFrame(fDateMax, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,5,0,0,0));
    fDateMax->Connect("ValueSet(Long_t)", "covid_viewer", this, "HandleDateRange()");

    fMounthMax = new TGComboBox(hframe);
    fMounthMax->Resize(100,20);
    fMounthMax->AddEntry("January",1);
    fMounthMax->AddEntry("February",2);
    fMounthMax->AddEntry("March",3);
    fMounthMax->AddEntry("April",4);
    fMounthMax->AddEntry("May",5);
    fMounthMax->AddEntry("June",6);
    fMounthMax->AddEntry("July",7);
    fMounthMax->AddEntry("August",8);
    fMounthMax->AddEntry("September",9);
    fMounthMax->AddEntry("October",10);
    fMounthMax->AddEntry("November",11);
    fMounthMax->AddEntry("December",12);
    fMounthMax->Select(1);
    hframe->AddFrame(fMounthMax, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,0,0,0,0));
    fMounthMax->Connect("Selected(Int_t)", "covid_viewer", this, "HandleMounthRange()");
    fMounthMax->Connect("Selected(Int_t)", "covid_viewer", this, "HandleDateRange()");

    fYearMax = new TGComboBox(hframe);
    fYearMax->Resize(60,20);
    fYearMax->AddEntry("2020",1);
    fYearMax->AddEntry("2021",2);
    fYearMax->Select(2);
    hframe->AddFrame(fYearMax, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,0,0,0,0));
    fYearMax->Connect("Selected(Int_t)", "covid_viewer", this, "HandleMounthRange()");
    fYearMax->Connect("Selected(Int_t)", "covid_viewer", this, "HandleDateRange()");

    TGTextButton *PlotButton = new TGTextButton(hframe,"  Plot  ");
    PlotButton->SetTextColor(CXred);
    PlotButton->Connect("Released()","covid_viewer",this,"PlotData()");
    hframe->AddFrame(PlotButton, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,10,0,0,0));

    Pixel_t black;
    gClient->GetColorByName("black", black);
    fColorSel = new TGColorSelect(hframe, black, 0);
    hframe->AddFrame(fColorSel, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,2,0,0,0));

    fDrawSameButton = new TGCheckButton(hframe,"same");
    hframe->AddFrame(fDrawSameButton, new TGLayoutHints( kLHintsCenterY  | kLHintsLeft ,5,0,0,0));

    TGCompositeFrame *Global = new TGCompositeFrame(this,600,100,kHorizontalFrame);
    AddFrame(Global,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,10,10,10,2));

    TGVerticalFrame *vframe = new TGVerticalFrame(Global,10,10,kFixedWidth);
    Global->AddFrame(vframe, new TGLayoutHints(kLHintsLeft | kLHintsExpandY));
    vframe->SetWidth(GetWidth()*0.25);

    TGGroupFrame *fGroupFrame = new TGGroupFrame(vframe, "Histogram to fit", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    vframe->AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0));

    fHistotofitBox = new TGComboBox(fGroupFrame);
    fHistotofitBox->SetHeight(20);
    fHistotofitBox->Connect("Selected(Int_t)", "covid_viewer", this, "HandleHistoToFit()");
    fGroupFrame->AddFrame(fHistotofitBox,new TGLayoutHints(kLHintsCenterY | kLHintsCenterX  | kLHintsExpandX ,0,0,5,2));

    fGroupFrame = new TGGroupFrame(vframe, "Fitting model", kVerticalFrame);
    fGroupFrame->SetTextColor(CXblue);
    fGroupFrame->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    vframe->AddFrame(fGroupFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0));

    fFittingModelBox = new TGComboBox(fGroupFrame);
    fFittingModelBox->SetHeight(20);
    fFittingModelBox->Connect("Selected(Int_t)", "covid_viewer", this, "HandleModels()");

    fGroupFrame->AddFrame(fFittingModelBox,new TGLayoutHints(kLHintsCenterY | kLHintsCenterX  | kLHintsExpandX ,0,0,5,2));

    TGCompositeFrame *fHorizontalFrame = new TGCompositeFrame(fGroupFrame, 60, 20, kHorizontalFrame);
    TGTextButton *DoClearButton = new TGTextButton(fHorizontalFrame, "Clear Fits");
    DoClearButton->Connect("Clicked()", "covid_viewer", this, "DoClear()");
    DoClearButton->SetTextColor(CXred);
    fHorizontalFrame->AddFrame(DoClearButton,new TGLayoutHints(kLHintsCenterY | kLHintsExpandX,5,10,0,0));
    fGroupFrame->AddFrame(fHorizontalFrame,new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,-10,-10,5,0));

    fMainTab = new TGTab(vframe);
    vframe->AddFrame(fMainTab, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
    //    fMainTab->Connect("CloseTab(Int_t)", "CXMainWindow", this, "CloseToolsTab(Int_t)");

    fRootCanvas = new TRootEmbeddedCanvas("PB", Global);
    Global->AddFrame(fRootCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 0, 0, 5, 5));
    fCanvas = fRootCanvas->GetCanvas();
    fCanvas->ToggleToolBar();
    fRootCanvas->GetContainer()->Connect("ProcessedEvent(Event_t*)", "covid_viewer", this, "ProcessedKeyEvent(Event_t*)");
    fCanvas->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "covid_viewer", this, "HandleMovement(Int_t,Int_t,Int_t, TObject*)");

    fCanvas->SetLeftMargin(0.107635);
    fCanvas->SetRightMargin(0.00125156);
    fCanvas->SetBottomMargin(0.148893);
    fCanvas->SetTopMargin(0.0211946);

    // status bar
    Int_t parts[] = {40 , 30 , 30};
    fStatusBar = new TGStatusBar(this,50,10,kHorizontalFrame);
    fStatusBar->SetParts(parts,3);
    fStatusBar->Draw3DCorner(kFALSE);
    AddFrame(fStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0));

    // What to clean up in destructor
    SetCleanup(kDeepCleanup);
    // Set a name to the main frame
    SetWindowName("covid player");
    SetIconName("covid player");
    MapSubwindows();
    Layout();
    Resize(GetDefaultSize());
    MapWindow();
    gSystem->ProcessEvents();

    gStyle->SetTitleSize(0.1,"Pad");


    ReadPopulation();
    //    fCountryBox->Select(2);
    //    PlotData();
}

covid_viewer::~covid_viewer()
{
    UnmapWindow();
    CloseWindow();
}

void covid_viewer::ReadPopulation()
{
    TString InputFile = Form("%s/database/Population.csv",getenv("COVID_PLAYER_SYS"));
    ifstream file(InputFile.Data());

    TString Buffer;
    string line;

    while(file) {
        getline(file,line);
        Buffer = line;
        TObjArray *arr = nullptr;

        if(Buffer.Contains(";")) {
            Buffer.Append(";");
            Buffer.ReplaceAll(";;","; ;");
            Buffer.ReplaceAll(";;","; ;");

            arr = Buffer.Tokenize(";");
        }
        else if(Buffer.Contains(",")) {
            Buffer.Append(",");
            Buffer.ReplaceAll(",,",", ,");
            Buffer.ReplaceAll(",,",", ,");

            arr = Buffer.Tokenize(",");
        }
        else continue;
        TString Country = ((TString)arr->At(0)->GetName()).ReplaceAll(" ","_");
        Int_t population = ((TString)arr->At(1)->GetName()).Atoi();

        fPopulationMap[Country] = population;
    }
}

void covid_viewer::UpdateCountryList()
{
    if(getenv("COVID_PLAYER_SYS") == nullptr) exit(EXIT_FAILURE);

    fCountryBox->RemoveAll();
    fCountryBox->AddEntry("Country",0);

    TString DatabaseDir = Form("%s/database/",getenv("COVID_PLAYER_SYS"));

    TSystemDirectory *dir = new TSystemDirectory(DatabaseDir,DatabaseDir);

    TList *l = dir->GetListOfFiles();
    l->Sort();

    for(int i=2 ; i<l->GetEntries() ;i++) {
        TString FileName = l->At(i)->GetName();
        if(FileName == "country_list.csv") continue;
        if(FileName == "Population.csv") continue;
        fCountryBox->AddEntry(FileName.ReplaceAll(".csv",""),i);
    }
    fCountryBox->Select(0);
}

void covid_viewer::SetSmoothing()
{
    fSmooth = fSmoothEntry->GetIntNumber();
    ReadData();
}

void covid_viewer::HandleButtons(Int_t id)
{
    // Handle different buttons
    auto *btn = static_cast<TGButton*>(gTQSender);

    if (id == -1)
        id = btn->WidgetId();

    if(id == kButtonTotal) {
        fDaily->SetState(kButtonUp);
        fCurrentHist = fDataTotalHist;
    }
    if(id == kButtonDaily) {
        fTotal->SetState(kButtonUp);
        fCurrentHist = fDataDailyHist;
    }
}

void covid_viewer::PlotData()
{

    if(fCurrentHist == nullptr) return;

    Int_t NBinsInRange = fDummyHist->GetXaxis()->FindFixBin(DateMaxStr)-fDummyHist->GetXaxis()->FindFixBin(DateMinStr);

    Int_t Step = NBinsInRange/40;

    for(int i=1 ; i<=fCurrentHist->GetNbinsX() ; i+=Step) {
        for(int ii=1 ; ii<Step ; ii++) {
            if((i+ii) <= fCurrentHist->GetNbinsX()) {
                fCurrentHist->GetXaxis()->SetBinLabel(i+ii,"");
            }
        }
    }

    if(fDrawSameButton->GetState() == kButtonUp) {
        fCurrentHist->Draw("p");
        fHistotofitBox->RemoveAll();
        Int_t Id = fHistotofitBox->GetNumberOfEntries();
        fHistotofitBox->AddEntry(fCurrentHist->GetName(),Id);
        fHistotofitBox->Select(Id);
    }
    else if((fCanvas->GetListOfPrimitives()->FindObject(fCurrentHist->GetName()) == nullptr)) {
        fCurrentHist->Draw("p same");
        Int_t Id = fHistotofitBox->GetNumberOfEntries();
        fHistotofitBox->AddEntry(fCurrentHist->GetName(),Id);
        fHistotofitBox->Select(Id);
    }

    Pixel_t pixel = fColorSel->GetColor();
    Int_t color = TColor::GetColor(pixel);
    fCurrentHist->SetMarkerColor(color);
    fCurrentHist->SetLineColor(color);


    Float_t Max=0;
    TH1 *fFirstHist = nullptr;

    for(int i=0 ; i<fCanvas->GetListOfPrimitives()->GetEntries() ; i++) {
        TObject *obj = fCanvas->GetListOfPrimitives()->At(i);
        if(obj->InheritsFrom(TH1::Class())) {
            fFirstHist = (TH1*) obj;
            break;
        }
    }

    fFirstHist->GetXaxis()->SetRange(DateMin,DateMax);

    for(int i=0 ; i<fCanvas->GetListOfPrimitives()->GetEntries() ; i++) {
        TObject *obj = fCanvas->GetListOfPrimitives()->At(i);
        if(obj->InheritsFrom(TH1::Class())) {
            TH1 *hist = (TH1*) obj;
            if(fFirstHist == nullptr) fFirstHist = (TH1*) obj;
            double locmax=0;
            for(int ibin=DateMin ; ibin<=DateMax ; ibin++) if(hist->GetBinContent(ibin)>locmax) locmax=hist->GetBinContent(ibin);
            if(locmax>Max) Max = locmax;
        }
    }

    fFirstHist->GetYaxis()->SetRangeUser(0,Max*1.2);

    gPad->Modified();
    gPad->Update();

    fHistoToFit = fCurrentHist;

    fFitMinGlobal = GetDateMin();
    fFitMaxGlobal = GetDateMax();

    Float_t XVal = gPad->GetFrame()->GetX1()+0.01*(gPad->GetFrame()->GetX2()-gPad->GetFrame()->GetX1());
    Float_t YVal = gPad->GetFrame()->GetY2()-0.04*(gPad->GetFrame()->GetY2()-gPad->GetFrame()->GetY1());

    Int_t NDY=0;
    Float_t DY = (gPad->GetFrame()->GetY2()-gPad->GetFrame()->GetY1())*0.05;

    for(auto i: fHistNames) delete i;
    fHistNames.clear();

    for(int i=0 ; i<fCanvas->GetListOfPrimitives()->GetEntries() ; i++) {
        TObject *obj = fCanvas->GetListOfPrimitives()->At(i);
        if(obj->InheritsFrom(TH1::Class())) {
            TH1 *hist = (TH1*) obj;
            TString Name = hist->GetTitle();
            TLatex *text = new TLatex(XVal,YVal-DY*NDY,Name);
            text->SetTextColor(hist->GetLineColor());
            text->SetTextSize(0.04);
            text->SetTextFont(132);
            text->Draw();
            NDY++;
            fHistNames.push_back(text);
        }
    }

    gPad->Modified();
    gPad->Update();

}

bool covid_viewer::ReadData()
{
    TString CountryName = fCountryBox->GetSelectedEntry()->GetTitle();
    if(CountryName == "Country") return false;

    //    if(fDrawSameButton->GetState() == kButtonUp) {
    //        delete fDataDailyHist;
    //        delete fDataTotalHist;
    //    }

    delete fDummyHist;

    TString DataGraphNameDaily = CountryName;
    TString DataGraphNameTotal = CountryName;

    TString DataGraphTitleDaily = CountryName.Copy().ReplaceAll("_"," ");
    TString DataGraphTitleTotal = CountryName.Copy().ReplaceAll("_"," ");

    DataGraphNameDaily += "__Daily";
    DataGraphNameTotal += "__Total";

    DataGraphTitleDaily += ", daily";
    DataGraphTitleTotal += ", total";

    if(fTypeBox->GetSelected()==1) {
        DataGraphNameDaily += "_Death";
        DataGraphNameTotal += "_Death";
        DataGraphTitleDaily += " deaths";
        DataGraphTitleTotal += " deaths";
    }
    else {
        DataGraphNameDaily += "_Cases";
        DataGraphNameTotal += "_Cases";
        DataGraphTitleDaily += " cases";
        DataGraphTitleTotal += " cases";
    }

    if(fSmooth>1) {
        DataGraphNameDaily += Form("_Smooth%d",fSmooth);
        DataGraphNameTotal += Form("_Smooth%d",fSmooth);

        DataGraphTitleDaily += Form(", %d days average",fSmooth);
        DataGraphTitleTotal += Form(", %d days average",fSmooth);
    }

    TString InputFile = Form("%s/database/%s.csv",getenv("COVID_PLAYER_SYS"),CountryName.Data());
    ifstream file(InputFile.Data());

    TString Buffer;
    string line;

    vDates.clear();
    vDeaths.clear();
    vDeaths_e.clear();
    vDeaths_Tot.clear();
    vDeaths_Tot_e.clear();

    Int_t Day=1;
    TString Mounth_str[24] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    Int_t NDaysPerMounth[24] = {31,29,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31};
    Int_t NDaysInYear=0;
    for(int i=0 ; i<24 ; i++) NDaysInYear += NDaysPerMounth[i];

    getline(file,line);

    int NToRemove = 0;

    while(file) {
        getline(file,line);
        Buffer = line;
        TObjArray *arr = nullptr;

        if(Buffer.Contains(";")) {
            Buffer.Append(";");
            Buffer.ReplaceAll(";;","; ;");
            Buffer.ReplaceAll(";;","; ;");

            arr = Buffer.Tokenize(";");
        }
        else if(Buffer.Contains(",")) {
            Buffer.Append(",");
            Buffer.ReplaceAll(",,",", ,");
            Buffer.ReplaceAll(",,",", ,");

            arr = Buffer.Tokenize(",");
        }
        else continue;
        TString Date = (TString)arr->At(1)->GetName();
        Date.ReplaceAll(" ","-");
        TObjArray *temp = Date.Tokenize("-");
        if(temp->GetEntries()==0) continue;
        TString Mounth_tmp = (TString)temp->At(0)->GetName();
        Day = ((TString)temp->At(1)->GetName()).Atoi();
        Date = Form("%d-%s-20",Day,Mounth_tmp.Data());
        delete temp;

        //        if(Date == "16-Jun" && CountryName == "India") NToRemove = 1600;
        //        if(Date == "6-May" && CountryName == "Belgium") NToRemove = 240;
        //        if(Date == "17-Apr" && CountryName == "China") NToRemove = 1290;

        Float_t Deaths;

        if(fTypeBox->GetSelected()==0) Deaths = ((TString)arr->At(2)->GetName()).Atoi()-NToRemove;
        else Deaths = ((TString)arr->At(3)->GetName()).Atoi()-NToRemove;

        delete arr;
        if(Deaths) {
            vDates.push_back(Date);
            vDeaths_Tot.push_back(Deaths);
        }
    }

    Int_t DeathsMin = 10;

    while(vDeaths_Tot.size() && vDeaths_Tot.front()<DeathsMin) {
        vDates.erase(vDates.begin());
        vDeaths_Tot.erase(vDeaths_Tot.begin());
    }

    vDeaths.push_back(vDeaths_Tot.front());
    for(size_t i=1 ; i<vDates.size() ; i++) {
        if(vDeaths_Tot.size()>i && vDeaths_Tot.at(i)>0) vDeaths.push_back(vDeaths_Tot.at(i)-vDeaths_Tot.at(i-1));
    }

    SmoothVector(fSmooth,vDeaths,vDeaths_e);
    SmoothVector(fSmooth,vDeaths_Tot,vDeaths_Tot_e);

    Pixel_t pixel = fColorSel->GetColor();
    Int_t color = TColor::GetColor(pixel);


    fDataTotalHist = (TH1D*) gROOT->FindObject(DataGraphNameTotal);
    if(fDataTotalHist) {
        fDataTotalHist->Reset();
    }
    else {
        fDataTotalHist = new TH1D(DataGraphNameTotal,DataGraphTitleTotal,NDaysInYear,0,NDaysInYear);
    }

    fDummyHist = new TH1D("Dummy","Dummy",NDaysInYear,0,NDaysInYear);

    if(fTypeBox->GetSelected()==0) fDataTotalHist->GetYaxis()->SetTitle("TOTAL CASES");
    else fDataTotalHist->GetYaxis()->SetTitle("TOTAL DEATHS");
    if(fApplyPopulationRatio) fDataTotalHist->GetYaxis()->SetTitle(Form("%s (poputlation %%)",fDataTotalHist->GetYaxis()->GetTitle()));
    fDataTotalHist->GetYaxis()->CenterTitle();
    fDataTotalHist->GetXaxis()->SetLabelSize(0.05);
    fDataTotalHist->GetXaxis()->SetTitleOffset(1.);
    fDataTotalHist->GetXaxis()->SetTitleFont(132);
    fDataTotalHist->GetXaxis()->SetLabelFont(132);

    fDataTotalHist->GetYaxis()->SetLabelSize(0.05);
    fDataTotalHist->GetYaxis()->SetTitleSize(0.05);
    fDataTotalHist->GetYaxis()->SetTitleOffset(1.15);
    fDataTotalHist->GetYaxis()->SetTickSize(0.01);
    fDataTotalHist->GetXaxis()->SetTickSize(0.01);
    fDataTotalHist->GetYaxis()->SetTitleFont(132);
    fDataTotalHist->GetYaxis()->SetLabelFont(132);

    fDataTotalHist->SetMarkerStyle(20);
    fDataTotalHist->SetMarkerColor(color);
    fDataTotalHist->SetLineColor(color);

    fDataDailyHist = (TH1D*) gROOT->FindObject(DataGraphNameDaily);
    if(fDataDailyHist) {
        fDataDailyHist->Reset();
    }
    else {
        fDataDailyHist = new TH1D(DataGraphNameDaily,DataGraphTitleDaily,NDaysInYear,0,NDaysInYear);
    }

    if(fTypeBox->GetSelected()==0) fDataDailyHist->GetYaxis()->SetTitle("CASES / DAY");
    else fDataDailyHist->GetYaxis()->SetTitle("DEATH / DAY");
    if(fApplyPopulationRatio) fDataDailyHist->GetYaxis()->SetTitle(Form("%s (population %%)",fDataDailyHist->GetYaxis()->GetTitle()));
    fDataDailyHist->GetYaxis()->CenterTitle();
    fDataDailyHist->GetXaxis()->SetLabelSize(0.05);
    fDataDailyHist->GetXaxis()->SetTitleOffset(1.);
    fDataDailyHist->GetXaxis()->SetTitleFont(132);
    fDataDailyHist->GetXaxis()->SetLabelFont(132);

    fDataDailyHist->GetYaxis()->SetLabelSize(0.05);
    fDataDailyHist->GetYaxis()->SetTitleSize(0.05);
    fDataDailyHist->GetYaxis()->SetTitleOffset(1.15);
    fDataDailyHist->GetYaxis()->SetTickSize(0.01);
    fDataDailyHist->GetXaxis()->SetTickSize(0.01);
    fDataDailyHist->GetYaxis()->SetTitleFont(132);
    fDataDailyHist->GetYaxis()->SetLabelFont(132);

    fDataDailyHist->SetMarkerStyle(20);

    fDataDailyHist->SetMarkerColor(color);
    fDataDailyHist->SetLineColor(color);

    if(fTotal->GetState()) fCurrentHist = fDataTotalHist;
    else fCurrentHist = fDataDailyHist;

    Int_t ibin=1;
    for(int i=0 ; i<24 ; i++) {
        for(int j=0 ; j<NDaysPerMounth[i] ; j++) {
            TString Year = "20";
            if(i>=12) Year = "21";
            TString Label = Form("%d-%s-%s",j+1,Mounth_str[i].Data(),Year.Data());
            fDataDailyHist->GetXaxis()->SetBinLabel(ibin,Label);
            fDataTotalHist->GetXaxis()->SetBinLabel(ibin,Label);
            fDummyHist->GetXaxis()->SetBinLabel(ibin,Label);
            ibin++;
        }
    }

    DateMin = fDummyHist->GetXaxis()->FindFixBin(DateMinStr);
    DateMax = fDummyHist->GetXaxis()->FindFixBin(DateMaxStr);

    TString LastDate = vDates.back();

    //    fDataDailyHist->SetBinContent(1,0.001);
    //    fDataDailyHist->SetBinContent(NDaysInYear,0.001);
    //    fDataDailyHist->SetBinError(NDaysInYear,0.001);

    for(size_t i=0 ; i<vDates.size() ; i++) {
        if(i<vDeaths_Tot.size() && vDeaths_Tot.at(i)) {
            Int_t Bin = fDataTotalHist->GetXaxis()->FindFixBin(vDates.at(i));
            if(Bin>0) {
                if(fApplyPopulationRatio) {
                    if(fPopulationMap.count(CountryName)) {
                        fDataTotalHist->SetBinContent(Bin,vDeaths_Tot.at(i)/fPopulationMap[CountryName]*100.);
                        fDataTotalHist->SetBinError(Bin,sqrt(vDeaths_Tot.at(i))/fPopulationMap[CountryName]*100.);
                    }
                    else {
                        cout << "OUPS, POPULATION NOT FOUND FOR COUNTRY: "<<CountryName<<endl;
                        return false;
                    }
                }
                else {
                    fDataTotalHist->SetBinContent(Bin,vDeaths_Tot.at(i));
                    fDataTotalHist->SetBinError(Bin,sqrt(vDeaths_Tot.at(i)));
                }
            }
        }
        if(i<vDeaths.size() && vDeaths.at(i)) {
            Int_t Bin = fDataDailyHist->GetXaxis()->FindFixBin(vDates.at(i));
            if(Bin>0) {
                if(fApplyPopulationRatio) {
                    if(fPopulationMap.count(CountryName)) {
                        fDataDailyHist->SetBinContent(Bin,vDeaths.at(i)/fPopulationMap[CountryName]*100.);
                        fDataDailyHist->SetBinError(Bin,vDeaths_e.at(i)/fPopulationMap[CountryName]*100.);
                    }
                    else {
                        cout << "OUPS, POPULATION NOT FOUND FOR COUNTRY: "<<CountryName<<endl;
                        return false;
                    }
                }
                else {
                    fDataDailyHist->SetBinContent(Bin,vDeaths.at(i));
                    fDataDailyHist->SetBinError(Bin,vDeaths_e.at(i));
                }
            }

            //            cout<<left<<setw(10)<<vDates.at(i)<<setw(10)<<vDeaths.at(i)<<setw(10)<<vDeaths_Tot.at(i)<<endl;
            LastDate = vDates.at(i);
        }
    }

    //    PlotData();

    return true;
}

void covid_viewer::SmoothVector(Int_t smooth, vector<double> &data, vector<double> &data_err)
{
    vector<Double_t> vSmooth;

    for(size_t i=0 ; i<data.size() ; i++) {
        Double_t NPoints = 0;
        Double_t Tot = 0.;
        Double_t Err2 = 0.;


        if(i>=(size_t)smooth) {
            for(int ii=0 ; ii<smooth ; ii++) {
                if((data.at(i-ii)>0)) {
                    Tot += data.at(i-ii);
                    Err2 += 2.*TMath::Power(sqrt(data.at(i-ii)),2);
                    NPoints ++;
                }
            }
        }

        Tot = Tot/NPoints;
        //        cout<<i<<" "<<vDates.at(i)<<" "<< vDeaths.at(i)<<" "<<Tot<<" "<<NPoints<<" "<<" "<<Err2<<" "<<sqrt(Err2)/NPoints<<endl;

        if(Tot>0.) {
            vSmooth.push_back(Tot);
            data_err.push_back(sqrt(Err2)/NPoints);
        }
        else {
            vSmooth.push_back(0);
            data_err.push_back(0);
        }
    }
    for(size_t i=0 ; i<data.size() ; i++) data.at(i) = vSmooth.at(i);
}

void covid_viewer::ProcessedKeyEvent(Event_t *event)
{
    char input[10];
    UInt_t keysym;

    gVirtualX->LookupString(event, input, sizeof(input), keysym);

    //        std::cout << "ProcessedKeyEvent : " << event->fCode << " " << event->fState <<" ; "<< event->fType  << "; " << keysym << std::endl;

    fCTRL = false;
    if(event->fState & kKeyControlMask) fCTRL = true;

    //    cout<<"Ctrl status: "<< fCTRL << endl;
    //    cout<<"fSelected Object: "<< fLastSelected << endl;
}

void covid_viewer::HandleMovement(Int_t EventType, Int_t EventX, Int_t EventY, TObject *selected)
{
    if(selected != nullptr) {
        const char *text1, *text4;
        char text3[100];
        text1 = selected->GetTitle();
        fStatusBar->SetText(text1,0);

        if(selected->InheritsFrom("TFrame")) fStatusBar->SetText("",0);

        if(fDummyHist) {
            sprintf(text3, "%s: N= %.2f", fDummyHist->GetXaxis()->GetBinLabel(fDummyHist->GetXaxis()->FindFixBin(fCanvas->AbsPixeltoX(EventX))), fCanvas->AbsPixeltoY(EventY) );
            fStatusBar->SetText(text3,1);

            text4 = selected->GetObjectInfo(EventX,EventY);
            fStatusBar->SetText(text4,2);
        }
    }

    //    cout<<"HandleMovement: "<<EventType<<" "<<EventX<<" "<<EventY<<" "<<selected<<endl;

    if(EventType == kButton1Up && fLastEventType == kButton1Down) {
        //        Int_t Z = TMath::Nint(fNucChartHist->GetYaxis()->GetBinCenter(fNucChartHist->GetYaxis()->FindBin(fCanvas->AbsPixeltoY(EventY))));
        //        Int_t N = TMath::Nint(fNucChartHist->GetXaxis()->GetBinCenter(fNucChartHist->GetXaxis()->FindBin(fCanvas->AbsPixeltoX(EventX))));
        //        SelectNucleus(Z,N);
    }

    /// Recuperation de la derniere position de la souris
    if(EventType == kMouseMotion) {
        fCanvas->AbsPixeltoXY(EventX,EventY,fLastYPosition,fLastXPosition);
    }

    fLastEventType = EventType;
    fLastEventX = EventX;
    fLastEventY = EventY;
    fLastSelected = selected;
}

void covid_viewer::HandleMounthRange()
{
    Int_t test = fMounthMin->GetSelected();
    if(test == 1 || test == 3 || test == 5 || test == 7 || test == 8 || test == 10 || test == 12) {
        fDateMin->SetLimits(TGNumberFormat::kNELLimitMinMax,1,31);
    }
    else if(test == 4 || test == 6 || test == 9 || test == 11) {
        fDateMin->SetLimits(TGNumberFormat::kNELLimitMinMax,1,30);
    }
    else {
        if(fYearMin->GetSelected()==1) fDateMin->SetLimits(TGNumberFormat::kNELLimitMinMax,1,29);
        else fDateMin->SetLimits(TGNumberFormat::kNELLimitMinMax,1,28);
    }

    test = fMounthMax->GetSelected();
    if(test == 1 || test == 3 || test == 5 || test == 7 || test == 8 || test == 10 || test == 12) {
        fDateMax->SetLimits(TGNumberFormat::kNELLimitMinMax,1,31);
    }
    else if(test == 4 || test == 6 || test == 9 || test == 11) {
        fDateMax->SetLimits(TGNumberFormat::kNELLimitMinMax,1,30);
    }
    else {
        if(fYearMax->GetSelected()==1) fDateMax->SetLimits(TGNumberFormat::kNELLimitMinMax,1,29);
        else fDateMax->SetLimits(TGNumberFormat::kNELLimitMinMax,1,28);
    }
}

void covid_viewer::HandleDateRange()
{
    //                          1     2     3     4     5     6     7     8     9     10    11    12
    TString Mounth_str[12] =   {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    //  Int_t NDaysPerMounth[12] = {31   ,29   ,31   ,30   ,31   ,30   ,31   ,31   ,30   ,31   ,30   ,31};

    Int_t datemin = fDateMin->GetIntNumber();
    Int_t Mounthmin = fMounthMin->GetSelected();
    Int_t YearMin = ((TString)fYearMin->GetSelectedEntry()->GetTitle()).Remove(0,2).Atoi();

    Int_t datemax = fDateMax->GetIntNumber();
    Int_t Mounthmax = fMounthMax->GetSelected();
    Int_t YearMax = ((TString)fYearMax->GetSelectedEntry()->GetTitle()).Remove(0,2).Atoi();

    if( (YearMax<YearMin) || ((YearMax==YearMin)  && (Mounthmax<Mounthmin)) || ((YearMax==YearMin)  && (Mounthmax==Mounthmin) && (datemax<=datemin))) {
        datemax = 31;
        Mounthmax = 12;

        fYearMax->Select(2,false);
        fMounthMax->Select(12,false);
        fDateMax->SetIntNumber(31);
    }

    DateMinStr = Form("%d-%s-%d",datemin,Mounth_str[Mounthmin-1].Data(),YearMin);
    DateMaxStr = Form("%d-%s-%d",datemax,Mounth_str[Mounthmax-1].Data(),YearMax);

    DateMin = fDummyHist->GetXaxis()->FindFixBin(DateMinStr);
    DateMax = fDummyHist->GetXaxis()->FindFixBin(DateMaxStr);

    TH1 *fFirstHist = nullptr;

    for(int i=0 ; i<fCanvas->GetListOfPrimitives()->GetEntries() ; i++) {
        TObject *obj = fCanvas->GetListOfPrimitives()->At(i);
        if(obj->InheritsFrom(TH1::Class())) {
            fFirstHist = (TH1*) obj;
            break;
        }
    }

    if(fFirstHist == nullptr) return;

    for(int i=1 ; i<=fFirstHist->GetNbinsX() ; i++) {
        fFirstHist->GetXaxis()->SetBinLabel(i,fDummyHist->GetXaxis()->GetBinLabel(i));
    }

    Int_t NBinsInRange = fDummyHist->GetXaxis()->FindFixBin(DateMaxStr)-fDummyHist->GetXaxis()->FindFixBin(DateMinStr);
    Int_t Step = NBinsInRange/40;

    for(int i=1 ; i<=fCurrentHist->GetNbinsX() ; i+=Step) {
        for(int ii=1 ; ii<Step ; ii++) {
            if((i+ii) <= fFirstHist->GetNbinsX()) {
                fFirstHist->GetXaxis()->SetBinLabel(i+ii,"");
            }
        }
    }

    fFirstHist->GetXaxis()->SetRange(DateMin,DateMax);

    gPad->Modified();
    gPad->Update();
}

void covid_viewer::HandleMenu(Int_t id)
{
    // Handle menu items.
    switch (id) {
    case M_FILE_SAVE:
        SaveCanvasAs();
        break;
    case M_FILE_EXIT:
        CloseWindow();
        UnmapWindow();
        DeleteWindow();

        gApplication->SetReturnFromRun(false);
        gApplication->Terminate(false);
        break;
    case M_MINIMIZER:
        if(GetMinimizerWindow() != nullptr) {
            GetMinimizerWindow()->MapRaised();
        }
        else {
            minimizer *min = new minimizer(gClient->GetRoot(), this, 400, 100);
            SetMinimizerWindow(min);
        }
        break;
    case M_POP_RATIO:
        if(fApplyPopulationRatio) {
            fApplyPopulationRatio = false;
            fMenuOptions->UnCheckEntry(M_POP_RATIO);
        }
        else {
            fApplyPopulationRatio = true;
            fMenuOptions->CheckEntry(M_POP_RATIO);
        }
        break;
    case M_UPDATE:
        TString command = Form("python3 %s/scripts/script.py",getenv("COVID_PLAYER_SYS"));
        system(command.Data());
        UpdateCountryList();
        break;
    }

}

void covid_viewer::SaveCanvasAs()
{
    TCanvas *c = fCanvas;

    if (strcmp("", fSavedAs) != 0) {
        Int_t ret_val;
        TString file = fSavedAs.Data();
        file.ReplaceAll(gSystem->DirName(fSavedAs.Data()), "");
        file.ReplaceAll("/", "");
        new TGMsgBox(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), "File name exist",
                     Form("File name '%s' already exists, OK to owerwrite it?", file.Data()),
                     kMBIconExclamation, kMBOk | kMBCancel, &ret_val);

        if (ret_val & kMBOk) {
            c->SaveAs(fSavedAs);
            return;
        }
    }

    const char* SaveAsTypes[] = {
        "PDF",          "*.pdf",
        //        "PostScript",   "*.ps",
        "Encapsulated PostScript", "*.eps",
        //        "SVG",          "*.svg",
        //        "TeX",          "*.tex",
        //        "GIF",          "*.gif",
        "ROOT macros",  "*.C",
        "ROOT files",   "*.root",
        //        "XML",          "*.xml",
        "PNG",          "*.png",
        "XPM",          "*.xpm",
        "JPEG",         "*.jpg",
        //        "TIFF",         "*.tiff",
        //        "XCF",          "*.xcf",
        //        "All files",    "*",
        nullptr,              nullptr
    };

    TString workdir = gSystem->WorkingDirectory();
    static TString dir(".");
    static Int_t typeidx = 0;
    static Bool_t overwr = kFALSE;
    TGFileInfo fi;
    fi.fFileTypes   = SaveAsTypes;
    fi.fIniDir      = StrDup(dir);
    fi.fFileTypeIdx = typeidx;
    fi.fOverwrite   = overwr;
    new TGFileDialog(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), kFDSave, &fi);
    gSystem->ChangeDirectory(workdir.Data());
    if (!fi.fFilename) return;
    TString fn = fi.fFilename;
    dir     = fi.fIniDir;
    typeidx = fi.fFileTypeIdx;
    overwr  = fi.fOverwrite;

    c->SaveAs(fn);

    fSavedAs = fn;
}

void covid_viewer::HandleHistoToFit()
{
    TString name = fHistotofitBox->GetSelectedEntry()->GetTitle();
    fHistoToFit = (TH1*)fCanvas->FindObject(name);

    if(fHistoToFit == nullptr) {
        cout<<name<<" not found in the canvas"<<endl;
        return;
    }

    fFittingModelBox->RemoveAll();

    if(name.Contains("Daily")) {
        fFittingModelBox->AddEntry("D' Model",3);
        fFittingModelBox->AddEntry("D'2 Model",4);
        fFittingModelBox->AddEntry("D'2 Full Model",5);
        fFittingModelBox->AddEntry("ESIR Model",6);
        fFittingModelBox->AddEntry("ESIR2 Model",7);
        fFittingModelBox->AddEntry("ESIR2 Full Model",7);
    }
    else {
        fFittingModelBox->AddEntry("D Model",0);
        fFittingModelBox->AddEntry("D2 Model",1);
        fFittingModelBox->AddEntry("D2 Full Model",2);
    }
}

void covid_viewer::ToggleTab(TGCompositeFrame *tab, const char * name)
{
    fMainTab->RemoveTab(fMainTab->GetCurrent());

    fMainTab->AddTab(name,tab);
    fMainTab->SetTab(name);
    tab->MapSubwindows();
    Layout();
}


void covid_viewer::HandleModels()
{
    fMainTab->RemoveTab(fMainTab->GetCurrent());

    TString ModelName = fFittingModelBox->GetSelectedEntry()->GetTitle();
    if(ModelName=="D' Model") {
        if(fDPrime_Model==nullptr) {
            fDprimeModelTab = fMainTab->AddTab(ModelName);
            fDPrime_Model = new dprime_model(fDprimeModelTab,10,10,this);
            fDPrime_Model->SetName(ModelName);
            fDprimeModelTab->AddFrame(fDPrime_Model, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
            fDprimeModelTab->MapSubwindows();
            Layout();
        }
        ToggleTab(fDprimeModelTab,ModelName);
        fDPrime_Model->UpdateRange();
    }
    if(ModelName=="D'2 Model") {
        if(fDPrime2_Model==nullptr) {
            fDprime2ModelTab = fMainTab->AddTab(ModelName);
            fDPrime2_Model = new dprime2_model(fDprime2ModelTab,10,10,this);
            fDPrime2_Model->SetName(ModelName);
            fDprime2ModelTab->AddFrame(fDPrime2_Model, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
            fDprime2ModelTab->MapSubwindows();
            Layout();
        }
        ToggleTab(fDprime2ModelTab,ModelName);
        fDPrime2_Model->UpdateRange();
    }
    if(ModelName=="D'2 Full Model") {
        if(fDPrime2Full_Model==nullptr) {
            fDprime2FullModelTab = fMainTab->AddTab(ModelName);
            fDPrime2Full_Model = new dprime2full_model(fDprime2FullModelTab,10,10,this);
            fDPrime2Full_Model->SetName(ModelName);
            fDprime2FullModelTab->AddFrame(fDPrime2Full_Model, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
            fDprime2FullModelTab->MapSubwindows();
            Layout();
        }
        ToggleTab(fDprime2FullModelTab,ModelName);
        fDPrime2Full_Model->UpdateRange();
    }
    if(ModelName=="ESIR Model") {
        if(fESIR_Model==nullptr) {
            fESIRModelTab = fMainTab->AddTab(ModelName);
            fESIR_Model = new esir_model(fESIRModelTab,10,10,this);
            fESIR_Model->SetName(ModelName);
            fESIRModelTab->AddFrame(fESIR_Model, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
            fESIRModelTab->MapSubwindows();
            Layout();
        }
        ToggleTab(fESIRModelTab,ModelName);
        fESIR_Model->UpdateRange();
    }
    if(ModelName=="ESIR2 Model") {
        if(fESIR2_Model==nullptr) {
            fESIR2ModelTab = fMainTab->AddTab(ModelName);
            fESIR2_Model = new esir2_model(fESIR2ModelTab,10,10,this);
            fESIR2_Model->SetName(ModelName);
            fESIR2ModelTab->AddFrame(fESIR2_Model, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
            fESIR2ModelTab->MapSubwindows();
            Layout();
        }
        ToggleTab(fESIR2ModelTab,ModelName);
        fESIR2_Model->UpdateRange();
    }
    if(ModelName=="ESIR2 Full Model") {
        if(fESIR2Full_Model==nullptr) {
            fESIR2FullModelTab = fMainTab->AddTab(ModelName);
            fESIR2Full_Model = new esir2full_model(fESIR2FullModelTab,10,10,this);
            fESIR2Full_Model->SetName(ModelName);
            fESIR2FullModelTab->AddFrame(fESIR2Full_Model, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
            fESIR2FullModelTab->MapSubwindows();
            Layout();
        }
        ToggleTab(fESIR2FullModelTab,ModelName);
        fESIR2Full_Model->UpdateRange();
    }

    if(ModelName=="D Model") {
        if(fD_Model==nullptr) {
            fDModelTab = fMainTab->AddTab(ModelName);
            fD_Model = new d_model(fDModelTab,10,10,this);
            fD_Model->SetName(ModelName);
            fDModelTab->AddFrame(fD_Model, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
            fDModelTab->MapSubwindows();
            Layout();
        }
        ToggleTab(fDModelTab,ModelName);
        fD_Model->UpdateRange();
    }
    if(ModelName=="D2 Model") {
        if(fD2_Model==nullptr) {
            fD2ModelTab = fMainTab->AddTab(ModelName);
            fD2_Model = new d2_model(fD2ModelTab,10,10,this);
            fD2_Model->SetName(ModelName);
            fD2ModelTab->AddFrame(fD2_Model, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
            fD2ModelTab->MapSubwindows();
            Layout();
        }
        ToggleTab(fD2ModelTab,ModelName);
        fD2_Model->UpdateRange();
    }
    if(ModelName=="D2 Full Model") {
        if(fD2Full_Model==nullptr) {
            fD2FullModelTab = fMainTab->AddTab(ModelName);
            fD2Full_Model = new d2full_model(fD2FullModelTab,10,10,this);
            fD2Full_Model->SetName(ModelName);
            fD2FullModelTab->AddFrame(fD2Full_Model, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 4, 0, 0));
            fD2FullModelTab->MapSubwindows();
            Layout();
        }
        ToggleTab(fD2FullModelTab,ModelName);
        fD2Full_Model->UpdateRange();
    }

}

void covid_viewer::DoClear()
{
    for(int i=0 ; i<fCanvas->GetListOfPrimitives()->GetEntries() ; i++) {
        TObject *obj = fCanvas->GetListOfPrimitives()->At(i);
        if(obj->InheritsFrom(TF1::Class())) {
            fCanvas->GetListOfPrimitives()->Remove(obj);
            i--;
        }
        if(((TString)obj->GetName()).EndsWith("_error")) {
            fCanvas->GetListOfPrimitives()->Remove(obj);
            i--;
        }
    }
    gPad->Modified();
    gPad->Update();
}

ClassImp(covid_viewer)

