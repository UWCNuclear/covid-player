#ifndef  minimizer_h
#define  minimizer_h

#include <TGFrame.h>
#include "TGComboBox.h"
#include "TGNumberEntry.h"

class covid_viewer;

class minimizer:public TGMainFrame
{
public:

    covid_viewer  *fMain;

    TGComboBox *fMinimizer = nullptr;
    TGComboBox *fAlgorithm = nullptr;
    TGNumberEntry *fTolenrance = nullptr;
    TGNumberEntry *fPrintLevel = nullptr;

    minimizer(const TGWindow *p, const TGWindow *m, UInt_t w, UInt_t h);
    virtual       ~minimizer();

    void CloseWindow();

    void UpdateMinimizer();
    void ApplyMinimizer();

    const char *GetMinimizer() {return fMinimizer->GetSelectedEntry()->GetTitle();}
    const char *GetAlgorithm() {return fAlgorithm->GetSelectedEntry()->GetTitle();}
    Double_t GetTolerance() {return fTolenrance->GetNumber();}
    Int_t GetPrintLevel() {return fPrintLevel->GetNumber();}

    ClassDef(minimizer,0)// Set Peak Find Dialog
};

#endif





