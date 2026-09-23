#ifndef DYLoopMUMU_h
#define DYLoopMUMU_h 1

#ifdef DEBUGMODE
  #define DYLOG(head, msg) if (fDebug) std::cout << "[" << head << "] " << msg << std::endl
#else
  #define DYLOG(head, msg)
#endif

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <variant>
#include <vector>

#include "muon.h"
#include "jet.h"
#include "NT.h"
#include "options.h"
#include "LumiReWeighting.h"
#include "EfficiencyTable.h"
#include "HistoSetMUMU.h"
#include "weight.h"

#include "TFile.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TChain.h"
#include "TLorentzVector.h"
#include "TH1.h"

#include "correction.h"

#include "yaml-cpp/yaml.h"

using FuncDiMuonCorrection = std::function<double(const TLorentzVector&, const TLorentzVector&)>;
using FuncSingleMuonCorrection = std::function<double(const TLorentzVector&)>;
using FuncEmptyInput = std::function<double()>;
using FuncSingleFloat = std::function<double(const float&)>;
using FuncSingleDouble = std::function<double(const double&)>;
using FuncSingleInt = std::function<double(const int&)>;

using FuncCorrections = std::variant<FuncDiMuonCorrection, FuncSingleMuonCorrection, FuncSingleFloat, FuncSingleDouble, FuncSingleInt, FuncEmptyInput>;

class DYLoopMUMU
{
public:

  DYLoopMUMU(options* fOpt) {
    fOpt->GetVariable("config", &fConfigPath);
    fConfig = YAML::LoadFile(std::string(fConfigPath));

    fOpt->GetVariable("id", &fJobID);
    fOpt->GetVariable("era", &fEra);
    fOpt->GetVariable("sample", &fSampleName);
    fOpt->GetVariable("debug", &fDebug);

    fCorrectionFuncs = {}; 

    fWeightEnvelope = WeigthEnvelope();
    fWeightEnvelope.AddType("OS");
    fWeightEnvelope.AddType("SS");
    fWeightEnvelope.AddType("OS_inverted");
    fWeightEnvelope.AddType("SS_inverted");

    if (fDebug) fWeightEnvelope.SetDebug();

    YAML::Node fConfigSample = YAML::LoadFile(std::string("../../input/dataset.yml"));
    fIsMC = false;
    fIsMC = fConfigSample[std::string(fEra)][std::string(fSampleName)]["IsMC"].as<bool>();

    fHistoSet = new HistoSetMUMU();
    if (fSampleName.Contains("NNLO_MUMU"))
      fHistoSet->InitGenInfo();
    
    fHistoSet->SetEra(fEra);

    fNtuples = new NT(fConfig, fIsMC);
    fNtuples->SetSampleName(fSampleName);
    fNtuples->SetEra(fEra);
    fNtuples->AddChain(fSampleName, fJobID);
    std::cout << " " << std::endl;

    fMaxEntries = fNtuples->GetEntries();
    fNtuples->init();
    std::cout << " " << std::endl;

    fDoRoccoR = fConfig["Correction"]["RoccoR"].as<bool>();
    if (fDoRoccoR)
      fRoccoR = std::make_unique<RoccoR>(fConfig["RoccoR"]["Path"].as<std::string>());

    fMuons = new MUON(fConfig);
    fMuons->IsMC(fIsMC);
    fMuons->SetRoccoR(fRoccoR.get(), fDoRoccoR);
    fMuons->init(fNtuples->GetTreeReader());
    
    fJets = new JET(fConfig);
    fJets->IsMC(fIsMC);
    fJets->init(fNtuples->GetTreeReader());
    fJets->SetSampleName(fSampleName.Data());

    fPuReweighting = new LumiReWeighting(
      "../../PileupInfo/" + fConfig["Pileup"]["MC"].as<std::string>(),
      "../../PileupInfo/" + fConfig["Pileup"]["Data"].as<std::string>(),
      "pileup",
      "pileup"
    );

    fDoPU = fConfig["Correction"]["PileUp"].as<bool>();    
    fDoTopPtReweighing = fConfig["Correction"]["TopPtReweighing"].as<bool>();
    fDoL1Pre = fConfig["Correction"]["L1PreFiring"].as<bool>();
    fTriggerPt = (fEra == "2017") ? 29. : 26.;

    fDoReco = false;
    // Reco SF is reserved for the high-pT workflow.
    // fDoReco = fConfig["Correction"]["Reco"].as<bool>();
    // fReco_SF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["Reco"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["Reco"]["Name"].as<std::string>());
    // if (fIsMC && fDoReco) {
    //   fCorrectionFuncs["RecoEff"] = FuncSingleMuonCorrection([this](const TLorentzVector& fMuon) -> double {
    //     float tP = fMuon.P();
    //     if (fMuon.P() < 50.) tP = 50.01;
    //     return fReco_SF->evaluate({std::abs(fMuon.Eta()), tP, "nominal"});
    //   });
    // }

    fDoID = fConfig["Correction"]["ID"].as<bool>();
    fID_SF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["ID"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["ID"]["Name"].as<std::string>());
    if (fIsMC && fDoID) {

      fCorrectionFuncs["IDEff"] = FuncSingleMuonCorrection([this](const TLorentzVector& fMuon) -> double {

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                        ID efficiency debugging                       " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " RESULT: " << fMuon.P() << " " << fMuon.Eta() << " " << fID_SF->evaluate({std::abs(fMuon.Eta()), fMuon.Pt(), "nominal"}) << std::endl;
        // std::cout << "######################################################################" << std::endl;
        // std::cout << " " << std::endl;

        return fID_SF->evaluate({std::abs(fMuon.Eta()), fMuon.Pt(), "nominal"});
      });
    }

    fDoISO = fConfig["Correction"]["ISO"].as<bool>();
    fISO_SF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["ISO"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["ISO"]["Name"].as<std::string>());
    if (fIsMC && fDoISO) {

      fCorrectionFuncs["IsoEff"] = FuncSingleMuonCorrection([this](const TLorentzVector& fMuon) -> double {

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                       ISO efficiency debugging                       " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " RESULT: " << fMuon.P() << " " << fMuon.Eta() << " " << fISO_SF->evaluate({std::abs(fMuon.Eta()), fMuon.Pt(), "nominal"}) << std::endl;
        // std::cout << "######################################################################" << std::endl;
        // std::cout << " " << std::endl;

        return fISO_SF->evaluate({std::abs(fMuon.Eta()), fMuon.Pt(), "nominal"});
      });
    }

    fDoTRIGG = false;
    fDoTRIGG = fConfig["Correction"]["Trigger"].as<bool>();
    fTRIG_Eff_MC = correction::CorrectionSet::from_file(fConfig["Efficiency"]["Trigger"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["Trigger"]["MC"].as<std::string>());
    fTRIG_Eff_Data = correction::CorrectionSet::from_file(fConfig["Efficiency"]["Trigger"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["Trigger"]["Data"].as<std::string>());

    if (fIsMC && fDoTRIGG) {
      fCorrectionFuncs["EventTriggerEff"] = FuncDiMuonCorrection([this](const TLorentzVector& fLMu, const TLorentzVector& fSMu) -> double {
        
        double mu_1_data = 0;
        double mu_2_data = 0;

        double mu_1_mc = 0;
        double mu_2_mc = 0;

        if (fLMu.Pt() >= fTriggerPt) {
          mu_1_data = fTRIG_Eff_Data->evaluate({std::abs(fLMu.Eta()), fLMu.Pt(), "nominal"});
          mu_1_mc = fTRIG_Eff_MC->evaluate({std::abs(fLMu.Eta()), fLMu.Pt(), "nominal"});
        }

        if (fSMu.Pt() >= fTriggerPt) {
          mu_2_data = fTRIG_Eff_Data->evaluate({std::abs(fSMu.Eta()), fSMu.Pt(), "nominal"});
          mu_2_mc = fTRIG_Eff_MC->evaluate({std::abs(fSMu.Eta()), fSMu.Pt(), "nominal"});
        }

        double data_tot = 1. - (1. - mu_1_data) * (1. - mu_2_data);
        double mc_tot = 1. - (1. - mu_1_mc) * (1. - mu_2_mc);

        double eventTriggerEffSF = 1.;
        if ( mc_tot != 0 )
          eventTriggerEffSF = data_tot / mc_tot;

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                       TRIGG efficiency debugging                     " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " LEADING: " << fLMu.Pt() << " " << fLMu.Eta() << " " << mu_1_data << " " << mu_1_mc << std::endl;
        // std::cout << " SUB-LEADING: " << fSMu.Pt() << " " << fSMu.Eta() << " " << mu_2_data << " " << mu_2_mc << std::endl;
        // std::cout << "(1 - (1 - " <<  mu_1_data << ") * (1 - " << mu_2_data << ")) / (1 - (1 - " << mu_1_mc << ") * (1 - " << mu_2_mc << ")) = " << eventTriggerEffSF << std::endl;
        // std::cout << "######################################################################" << std::endl;
        // std::cout << " " << std::endl;

        return eventTriggerEffSF;
      });
    }

    if (fIsMC && fDoTRIGG) {
      fCorrectionFuncs["SingleTriggerEff"] = FuncSingleMuonCorrection([this](const TLorentzVector& fLMu) -> double {
        
        double mu_1_data = 0;
        double mu_1_mc = 0;

        if (fLMu.Pt() >= fTriggerPt) {
          mu_1_data = fTRIG_Eff_Data->evaluate({std::abs(fLMu.Eta()), fLMu.Pt(), "nominal"});
          mu_1_mc = fTRIG_Eff_MC->evaluate({std::abs(fLMu.Eta()), fLMu.Pt(), "nominal"});
        }

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                       TRIGG efficiency debugging                     " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " LEADING: " << fLMu.Pt() << " " << fLMu.Eta() << " " << mu_1_data << " " << mu_1_mc << " " << mu_1_data / mu_1_mc << std::endl;
        // std::cout << "######################################################################" << std::endl;
        // std::cout << " " << std::endl;

        return (mu_1_mc > 0.) ? mu_1_data / mu_1_mc : 1.;
      });
    }

    fDoJetPUID = false;
    fDoJetPUID = fConfig["Correction"]["JetPU"].as<bool>();

    if (fIsMC && fDoJetPUID) {
      fCorrectionFuncs["PUJetIDEff"] = FuncEmptyInput([this]() -> double {

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                     PU Jet ID efficiency debugging                   " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " FACTOR: " << fJets->GetPUIDSF() << std::endl;
        // std::cout << "######################################################################" << std::endl;
        // std::cout << " " << std::endl;

        return fJets->GetPUIDSF();
      });
    }

    fDoBTag = false;
    fDoBTag = fConfig["Correction"]["BTag"].as<bool>();

    if (fIsMC && fDoBTag) {
      fCorrectionFuncs["bTaggingEff"] = FuncEmptyInput([this]() -> double {

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                     b-tagging efficiency debugging                   " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " FACTOR: " << fJets->GetBTagSF() << std::endl;
        // std::cout << "######################################################################" << std::endl;
        // std::cout << " " << std::endl;

        return fJets->GetBTagSF();
      });
    }

    Print();

    std::cout << std::fixed;
  }

  void Print() {

    std::cout << "######################################################################" << std::endl;
    std::cout << "                             Loop setting                             " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " fDoRoccoR  : " << fDoRoccoR << " " << fConfig["RoccoR"]["Path"].as<std::string>() << std::endl;
    std::cout << " fDoReco    : " << fDoReco << " (reserved for high-pT workflow)" << std::endl;
    std::cout << " fDoID      : " << fDoID << " " << fConfig["Efficiency"]["ID"]["Path"].as<std::string>() << std::endl;
    std::cout << "              " << fDoID << " " << fConfig["Efficiency"]["ID"]["Name"].as<std::string>() << std::endl;
    std::cout << " fDoISO     : " << fDoISO << " " << fConfig["Efficiency"]["ISO"]["Path"].as<std::string>() << std::endl;
    std::cout << "              " << fDoISO << " " << fConfig["Efficiency"]["ISO"]["Name"].as<std::string>() << std::endl;
    std::cout << " fDoTRIGG   : " << fDoTRIGG << " " << fConfig["Efficiency"]["Trigger"]["Path"].as<std::string>() << std::endl;
    std::cout << "              " << fDoTRIGG << " " << fConfig["Efficiency"]["Trigger"]["Data"].as<std::string>() << std::endl;
    std::cout << "              " << fDoTRIGG << " " << fConfig["Efficiency"]["Trigger"]["MC"].as<std::string>() << std::endl;
    std::cout << " fDoPU      : " << fDoPU << " " << fConfig["Pileup"]["Data"].as<std::string>() << std::endl;
    std::cout << "              " << fConfig["Pileup"]["MC"].as<std::string>() << std::endl;
    std::cout << " fDoL1Pre   : " << fDoL1Pre << " " << std::endl;
    std::cout << " fDoJetPUID : " << fDoJetPUID << " " << fConfig["Efficiency"]["JetPU"]["Path"].as<std::string>() << std::endl;
    std::cout << " fDoBTag    : " << fDoBTag << " " << fConfig["Efficiency"]["BTag"]["Path"].as<std::string>() << std::endl;
    std::cout << "                " << fConfig["Efficiency"]["BTagEff"]["Path"].as<std::string>() << std::endl;
    std::cout << " fDoTopPtReweighing : " << fDoTopPtReweighing << std::endl;
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
  void InitCorrections();

  TString fConfigPath;
  TString fEra;
  TString fSampleName;
  int fJobID;
  bool fIsMC;

  LumiReWeighting* fPuReweighting;
  std::unique_ptr<RoccoR> fRoccoR;
  std::shared_ptr<const correction::Correction> fReco_SF;
  std::shared_ptr<const correction::Correction> fID_SF;
  std::shared_ptr<const correction::Correction> fISO_SF;
  std::shared_ptr<const correction::Correction> fTRIG_Eff_Data;
  std::shared_ptr<const correction::Correction> fTRIG_Eff_MC;

  bool fDoReco;
  bool fDoRoccoR;
  bool fDoID;
  bool fDoISO;
  bool fDoTRIGG;
  bool fDoPU;
  bool fDoL1Pre;
  bool fDoJetPUID;
  bool fDoBTag;
  bool fDoTopPtReweighing;
  bool fIsInverted;
  
  bool fDebug;

  NT* fNtuples;
  YAML::Node fConfig;

  MUON* fMuons;
  JET* fJets;

  double fMaxEntries;
  double fTriggerPt;

  HistoSetMUMU* fHistoSet;

  WeigthEnvelope fWeightEnvelope;

  std::map<std::string, FuncCorrections> fCorrectionFuncs;
};

#endif
