#ifndef d_model_H
#define d_model_H

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

class d_model : public fitmodel
{
    RQ_OBJECT("d_model");

public:
    TGNumberEntry *fNE_a[3];
    TGNumberEntry *fNE_b[3];
    TGNumberEntry *fNE_c[3];

    TGTextEntry *fT0_TextEntry = nullptr;
    TGCheckButton *fFixT0 = nullptr;

public:
    d_model(const TGCompositeFrame *MotherFrame, UInt_t w, UInt_t h, covid_viewer *main = nullptr);
    virtual ~d_model();

    virtual void InitParameters();
    virtual Double_t FitFunction(Double_t*/*xx*/,Double_t*/*pp*/);


    ClassDef(d_model,0);
};

#endif
