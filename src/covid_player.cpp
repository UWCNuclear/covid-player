#include "TGClient.h"
#include "TApplication.h"
#include "TRandom3.h"
#include "Riostream.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TString.h"
#include "TSystem.h"
#include "Fit/Fitter.h"

#include "covid_viewer.h"

using namespace  std;

int main(int argc, char **argv) {

    TApplication *theApp = new TApplication("App", &argc, argv,NULL,0);

    ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Migrad");
    ROOT::Math::MinimizerOptions::SetDefaultTolerance(0.05);
    ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(2147483647);
    ROOT::Math::MinimizerOptions::SetDefaultErrorDef(2);

    // make sure that the Gpad and GUI libs are loaded
    TApplication::NeedGraphicsLibs();
    gApplication->InitializeGraphics();

    covid_viewer *fViewer = new covid_viewer(gClient->GetRoot(), 1300, 600 );
    theApp->Run(false);

    if (fViewer) {
        fViewer->CloseWindow();
        fViewer = nullptr;
    }

    if (theApp) {
        delete (theApp);
        theApp = nullptr;
    }

    return 0;
}
