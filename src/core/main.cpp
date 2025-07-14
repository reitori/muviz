#include "core/Application.h"

#include "TApplication.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TH1.h"


int main(int argc, char** argv){
    ROOT::EnableThreadSafety();
    ROOT::EnableImplicitMT();
    TH1::AddDirectory(false);
    
    viz::Application app(argc, argv);
    app.run();
}