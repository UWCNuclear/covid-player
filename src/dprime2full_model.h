#ifndef dprime2full_model_H
#define dprime2full_model_H

#include "RQ_OBJECT.h"
#include "TGFrame.h"
#include "TArrow.h"
#include "TObject.h"
#include "TGComboBox.h"
#include "TGNumberEntry.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "fitmodel.h"

using namespace std;

class dprime2full_model : public fitmodel
{
    RQ_OBJECT("dprime2full_model");

public:
    TGNumberEntry *fNE_a1[3];
    TGNumberEntry *fNE_a2[3];
    TGNumberEntry *fNE_b1[3];
    TGNumberEntry *fNE_b2[3];
    TGNumberEntry *fNE_c1[3];
    TGNumberEntry *fNE_c2[3];

    TGTextEntry *fT0_TextEntry = nullptr;
    TGCheckButton *fFixT0 = nullptr;

public:
    dprime2full_model(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, covid_viewer *main = nullptr);
    virtual ~dprime2full_model();

    virtual void InitParameters();
    virtual Double_t FitFunction(Double_t*/*xx*/,Double_t*/*pp*/);


    ClassDef(dprime2full_model,0);
};

#endif
