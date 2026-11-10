#include "NT.h"
#include "options.h"
#include "DYLoopMUMU.h"

#include <iostream>
#include <map>
#include <string>
#include <typeinfo>

#include "TFile.h"
#include "TROOT.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TStopwatch.h"
#include "TChain.h"
#include "TLorentzVector.h"
#include "TH1.h"

#include "yaml-cpp/yaml.h"

int main(int argc, char* argv[]) {
  TH1::AddDirectory(false);
  TH1::SetDefaultSumw2();
  TDirectory::AddDirectory(0);
  

  options* fOpt = new options(argc, argv);

  DYLoopMUMU fLoops = DYLoopMUMU(fOpt);
  fLoops.Loop();
  
  return 0;
}
