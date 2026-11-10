#ifndef DYLoopEE_h
#define DYLoopEE_h 1

#include <iostream>
#include <map>
#include <string>
#include <typeinfo>

#include "muon.h"
#include "jet.h"
#include "NT.h"
#include "options.h"
#include "LumiReWeighting.h"
#include "EfficiencyTable.h"
#include "HistoSetEE.h"

#include "TFile.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TChain.h"
#include "TLorentzVector.h"
#include "TH1.h"

#include "correction.h"

#include "yaml-cpp/yaml.h"

class DYLoopEE
{
public:

  DYLoopEE(options* fOpt) {
    fOpt->GetVariable("config", &fConfigPath);
    fConfig = YAML::LoadFile(std::string(fConfigPath));

    fOpt->GetVariable("id", &fJobID);
    fOpt->GetVariable("era", &fEra);
    fOpt->GetVariable("sample", &fSampleName);

    YAML::Node fConfigSample = YAML::LoadFile(std::string("../../input/dataset.yml"));
    fIsMC = false;
    fIsMC = fConfigSample[std::string(fEra)][std::string(fSampleName)]["IsMC"].as<bool>();

    fDoL1Pre = false;
    fDoL1Pre = fConfig["Correction"]["L1PreFiring"].as<bool>();

    fDoReco = false;
    fDoReco = fConfig["Correction"]["Reco"].as<bool>(); 
    fReco_SF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["Reco"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["Reco"]["Name"].as<std::string>());

    fDoID = false;
    fDoID = fConfig["Correction"]["ID"].as<bool>();
    fID_SF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["ID"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["ID"]["Name"].as<std::string>());

    fDoTRIGG = false;
    fDoTRIGG = fConfig["Correction"]["Trigger"].as<bool>();
    auto trigSF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["Trigger"]["Path"].as<std::string>());
    fTRIG_SF_Leg1 = trigSF->at(fConfig["Efficiency"]["Trigger"]["NameLeg1"].as<std::string>());
    fTRIG_SF_Leg2 = trigSF->at(fConfig["Efficiency"]["Trigger"]["NameLeg2"].as<std::string>());

    fDoElecMisCharge = false;
    fDoElecMisCharge = fConfig["Correction"]["ElecMisCharge"].as<bool>();
    fElecMisCharge_SF = (TH2D*)TFile::Open((TString)(fConfig["Efficiency"]["ElecMisCharge"]["Path"].as<std::string>()))->Get("cfsf");
    if (!fElecMisCharge_SF)
      throw std::runtime_error("Histogram 'cfsf' not found!");

    fElecMisCharge_SF->SetDirectory(0);

    fDoPU = false;
    fDoPU = fConfig["Correction"]["PileUp"].as<bool>();
    fPuReweighting = new LumiReWeighting(
      "../../PileupInfo/" + fConfig["Pileup"]["MC"].as<std::string>(),
      "../../PileupInfo/" + fConfig["Pileup"]["Data"].as<std::string>(),
      "pileup",
      "pileup"
    );

    fDoJetPUID = false;
    fDoJetPUID = fConfig["Correction"]["JetPU"].as<bool>();

    fDoBTag = false;
    fDoBTag = fConfig["Correction"]["BTag"].as<bool>();

    fDoTopPtReweighing = false;
    fDoTopPtReweighing = fConfig["Correction"]["TopPtReweighing"].as<bool>();

    Print();

    fHistoSet = new HistoSetEE(); 

    fNtuples = new NT(fConfig, fIsMC);
    fNtuples->SetSampleName(fSampleName);
    fNtuples->SetEra(fEra);
    fNtuples->AddChain(fSampleName, fJobID);

    std::cout << " " << std::endl;

    fMaxEntries = fNtuples->GetEntries();
    fNtuples->init();

    std::cout << " " << std::endl;

    fElecs = new ELEC(fConfig);
    fElecs->IsMC(fIsMC);
    fElecs->init(fNtuples->GetTreeReader());

    fJets = new JET(fConfig);
    fJets->IsMC(fIsMC);
    fJets->init(fNtuples->GetTreeReader());
    fJets->SetSampleName(fSampleName.Data());

    std::cout << std::fixed;
  }

  void Print() {

    std::cout << "######################################################################" << std::endl;
    std::cout << "                             Loop setting                             " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " fDoReco: " << fDoReco << " " << fConfig["Efficiency"]["Reco"]["Path"].as<std::string>() << std::endl;
    std::cout << "          " << fDoReco << " " << fConfig["Efficiency"]["Reco"]["Name"].as<std::string>() << std::endl;
    std::cout << " fDoID: " << fDoID << " " << fConfig["Efficiency"]["ID"]["Path"].as<std::string>() << std::endl;
    std::cout << "        " << fDoID << " " << fConfig["Efficiency"]["ID"]["Name"].as<std::string>() << std::endl;
    std::cout << " fDoTRIGG: " << fDoTRIGG << " " << fConfig["Efficiency"]["Trigger"]["Path"].as<std::string>() << std::endl;
    std::cout << "           Leg1: " << fConfig["Efficiency"]["Trigger"]["NameLeg1"].as<std::string>() << std::endl;
    std::cout << "           Leg2: " << fConfig["Efficiency"]["Trigger"]["NameLeg2"].as<std::string>() << std::endl;
    std::cout << " fDoElecMisCharge: " << fDoElecMisCharge << std::endl;
    std::cout << "                   " << fConfig["Efficiency"]["ElecMisCharge"]["Path"].as<std::string>() << std::endl;
    std::cout << " fDoPU: " << fDoPU << " " << fConfig["Pileup"]["Data"].as<std::string>() << std::endl;
    std::cout << "          " << fConfig["Pileup"]["MC"].as<std::string>() << std::endl;
    std::cout << " fDoL1Pre: " << fDoL1Pre << " " << std::endl;
    std::cout << " fDoJetPUID: " << fDoJetPUID << " " << fConfig["Efficiency"]["JetPU"]["Path"].as<std::string>() << std::endl;
    std::cout << " fDoBTag: " << fDoBTag << " " << fConfig["Efficiency"]["BTag"]["Path"].as<std::string>() << std::endl;
    std::cout << "            " << fConfig["Efficiency"]["BTagEff"]["Path"].as<std::string>() << std::endl;
    std::cout << " fDoTopPtReweighing: " << fDoTopPtReweighing << std::endl;
    std::cout << "######################################################################" << std::endl;
    std::cout << " " << std::endl;
  }

  TH1D* GetHist(TString name, TString title, int nBin, float xlow, float xhigh) {

    float binWidth = (xhigh - xlow) / (float)nBin;

    return new TH1D(name, title, nBin + 2, xlow - binWidth, xhigh + binWidth);
  }

  void FillHisto(TH1D* hist, double value, double weight) {

    float xMin = hist->GetBinLowEdge(1);
    float xMax = hist->GetBinLowEdge(hist->GetNbinsX()) + hist->GetBinWidth(hist->GetNbinsX());

    if (value < xMin) hist->Fill(hist->GetBinCenter(1), weight);
    else if (value > xMax) hist->Fill(hist->GetBinCenter(hist->GetNbinsX()), weight);
    else hist->Fill(value, weight);
  }

  std::string getCurrentTimeString();
  std::string formatDuration(const std::chrono::high_resolution_clock::duration& duration); 

  void SetEra(TString fEra_) { fEra = fEra_; }
  void SetSample(TString fSampleName_) { fSampleName = fSampleName_; }
  void SetJobID(int fJobID_) { fJobID = fJobID_; }
  void SetMC(bool fIsMC_) { fIsMC = fIsMC_; }
  void SetNT();

  void PrepareHist();
  void EndOfJob();

  void Loop();


private:
  TString fConfigPath;
  TString fEra;
  TString fSampleName;
  int fJobID;
  bool fIsMC;

  LumiReWeighting* fPuReweighting;
  std::shared_ptr<const correction::Correction> fReco_SF;
  std::shared_ptr<const correction::Correction> fID_SF;
  std::shared_ptr<const correction::Correction> fTRIG_SF_Leg1;
  std::shared_ptr<const correction::Correction> fTRIG_SF_Leg2;
  TH2D* fElecMisCharge_SF;

  bool fDoReco;
  bool fDoID;
  bool fDoTRIGG;
  bool fDoElecMisCharge;
  bool fDoPU;
  bool fDoL1Pre;
  bool fDoJetPUID;
  bool fDoBTag;
  bool fDoTopPtReweighing;

  NT* fNtuples;
  YAML::Node fConfig;
  
  ELEC* fElecs;
  JET* fJets;

  double fMaxEntries;

  HistoSetEE* fHistoSet;
};

#endif
