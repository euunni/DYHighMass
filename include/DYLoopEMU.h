#ifndef DYLoopEMU_h
#define DYLoopEMU_h 1

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <typeinfo>

#include "emu.h"
#include "jet.h"
#include "NT.h"
#include "options.h"
#include "LumiReWeighting.h"
#include "EfficiencyTable.h"
#include "HistoSetEMU.h"
#include "weight.h"

#include "TFile.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TChain.h"
#include "TLorentzVector.h"
#include "TH1.h"
#include "TH2.h"

#include "correction.h"

#include "yaml-cpp/yaml.h"

using FuncDiMuonCorrection = std::function<double(const TLorentzVector&, const TLorentzVector&)>;
using FuncSingleMuonCorrection = std::function<double(const TLorentzVector&)>;
using FuncSingleElecCorrection = std::function<double(const TLorentzVector&, const float&)>;

using FuncCorrections = std::variant<FuncDiMuonCorrection, FuncSingleMuonCorrection, FuncSingleElecCorrection>;

class DYLoopEMU
{
public:

  DYLoopEMU(options* fOpt) {
    fOpt->GetVariable("config", &fConfigPath);
    fConfig = YAML::LoadFile(std::string(fConfigPath));

    fOpt->GetVariable("id", &fJobID);
    fOpt->GetVariable("era", &fEra);
    fOpt->GetVariable("sample", &fSampleName);
    fOpt->GetVariable("debug", &fDebug);
    fOpt->GetVariable("OutputDir", &fOutputDir);

    fCorrectionFuncs = {}; 

    fWeightEnvelope = WeigthEnvelope();
    fWeightEnvelope.AddType("OS");
    fWeightEnvelope.AddType("SS");
    fWeightEnvelope.AddType("OS_inverted");
    fWeightEnvelope.AddType("SS_inverted");

    if (fDebug) fWeightEnvelope.SetDebug();

    YAML::Node fConfigSample = YAML::LoadFile(std::string("../../input/dataset.yml"));

    fIsMC = fConfigSample[std::string(fEra)][std::string(fSampleName)]["IsMC"].as<bool>();
    fDoL1Pre = fConfig["Correction"]["L1PreFiring"].as<bool>();
    fDoTopPtReweighing = fConfig["Correction"]["TopPtReweighing"].as<bool>();
    fDoPU = fConfig["Correction"]["PileUp"].as<bool>();
    fPuReweighting = new LumiReWeighting(
      "../../PileupInfo/" + fConfig["Pileup"]["MC"].as<std::string>(),
      "../../PileupInfo/" + fConfig["Pileup"]["Data"].as<std::string>(),
      "pileup",
      "pileup"
    );

    fDoReco = false;
    fDoReco = fConfig["Correction"]["Reco"].as<bool>(); 
    fReco_SF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["Reco"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["Reco"]["Name"].as<std::string>());
    if (fIsMC && fDoReco) {

      fCorrectionFuncs["RecoEff"] = FuncSingleMuonCorrection([this](const TLorentzVector& fMuon) -> double {

        float tP = fMuon.P();
        if (fMuon.P() < 50.) tP = 50.01;

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                       Reco efficiency debugging                      " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " RESULT: " << fMuon.P() << " " << fMuon.Eta() << " " << fReco_SF->evaluate({std::abs(fMuon.Eta()), tP, "nominal"}) << std::endl;
        // std::cout << "######################################################################" << std::endl;
        // std::cout << " " << std::endl;

        return fReco_SF->evaluate({std::abs(fMuon.Eta()), tP, "nominal"});
      });
    }

    fDoID = false;
    fDoID = fConfig["Correction"]["ID"].as<bool>();
    fID_SF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["ID"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["ID"]["Name"].as<std::string>());
    if (fIsMC && fDoID) {

      fCorrectionFuncs["IDEff"] = FuncSingleMuonCorrection([this](const TLorentzVector& fMuon) -> double {

        float tPt = fMuon.Pt();
        if (fMuon.Pt() < 50.) tPt = 50.01;

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                        ID efficiency debugging                       " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " RESULT: " << fMuon.P() << " " << fMuon.Eta() << " " << fID_SF->evaluate({std::abs(fMuon.Eta()), tPt, "nominal"}) << std::endl;
        // std::cout << "######################################################################" << std::endl;
        // std::cout << " " << std::endl;

        return fID_SF->evaluate({std::abs(fMuon.Eta()), tPt, "nominal"});
      });
    }

    fDoISO = false;
    fDoISO = fConfig["Correction"]["ISO"].as<bool>();
    fISO_SF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["ISO"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["ISO"]["Name"].as<std::string>());
    if (fIsMC && fDoISO) {

      fCorrectionFuncs["IsoEff"] = FuncSingleMuonCorrection([this](const TLorentzVector& fMuon) -> double {

        float tPt = fMuon.Pt();
        if (fMuon.Pt() < 50.) tPt = 50.01;

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                       ISO efficiency debugging                       " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " RESULT: " << fMuon.P() << " " << fMuon.Eta() << " " << fISO_SF->evaluate({std::abs(fMuon.Eta()), tPt, "nominal"}) << std::endl;
        // std::cout << "######################################################################" << std::endl;
        // std::cout << " " << std::endl;

        return fISO_SF->evaluate({std::abs(fMuon.Eta()), tPt, "nominal"});
      });
    }

    fDoTRIGG = false;
    fDoTRIGG = fConfig["Correction"]["Trigger"].as<bool>();
    fTRIG_Eff_MC = correction::CorrectionSet::from_file(fConfig["Efficiency"]["Trigger"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["Trigger"]["MC"].as<std::string>());
    fTRIG_Eff_Data = correction::CorrectionSet::from_file(fConfig["Efficiency"]["Trigger"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["Trigger"]["Data"].as<std::string>());
    if (fIsMC && fDoTRIGG) {
      fCorrectionFuncs["SingleTriggerEff"] = FuncSingleMuonCorrection([this](const TLorentzVector& fLMu) -> double {
        
        double mu_1_data = 0;
        double mu_1_mc = 0;
        double mu_1_pt = fLMu.Pt();
        if (mu_1_pt < 52.) mu_1_pt = 52.01;

        mu_1_data = fTRIG_Eff_Data->evaluate({std::abs(fLMu.Eta()), mu_1_pt, "nominal"});
        mu_1_mc = fTRIG_Eff_MC->evaluate({std::abs(fLMu.Eta()), mu_1_pt, "nominal"});

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                       TRIGG efficiency debugging                     " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " LEADING: " << fLMu.Pt() << " " << fLMu.Eta() << " " << mu_1_data << " " << mu_1_mc << " " << mu_1_data / mu_1_mc << std::endl;
        // std::cout << "######################################################################" << std::endl;
        // std::cout << " " << std::endl;

        return mu_1_data / mu_1_mc;
      });
    }

    fDoElecReco = false;
    fDoElecReco = fConfig["Correction"]["ElecReco"].as<bool>();
    fElecReco_SF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["ElecReco"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["ElecReco"]["Name"].as<std::string>());
    if (fIsMC && fDoElecReco) {
      fCorrectionFuncs["ElecRecoEff"] = FuncSingleElecCorrection([this](const TLorentzVector& fLElec, const float& fSCEtaElec) -> double {

        if (fLElec.Pt() < 50.) return 0;
        else                   return fElecReco_SF->evaluate({(std::string)(fEra), "sf", "RecoAbove20", fSCEtaElec, fLElec.Pt()});
      });
    }

    fDoElecID = false;
    fDoElecID = fConfig["Correction"]["ElecID"].as<bool>();
    if (fIsMC && fDoElecID) {
      fCorrectionFuncs["ElecIDEff"] = FuncSingleElecCorrection([this](const TLorentzVector& fLElec, const float& fSCEtaElec) -> double {

        if (fLElec.Pt() < 50.) return 0;
        else                   return fElecReco_SF->evaluate({(std::string)(fEra), "sf", "Medium", fSCEtaElec, fLElec.Pt()});
      });
    }

    // fDoElecMisCharge = false;
    // fDoElecMisCharge = fConfig["Correction"]["ElecMisCharge"].as<bool>();
    // fElecMisCharge_SF = (TH2D*)TFile::Open((TString)(fConfig["Efficiency"]["ElecMisCharge"]["Path"].as<std::string>()))->Get("cfsf");
    // if (!fElecMisCharge_SF)
    //   throw std::runtime_error("Histogram 'cfsf' not found!");
    // fElecMisCharge_SF->SetDirectory(0);

    fDoJetPUID = false;
    fDoJetPUID = fConfig["Correction"]["JetPU"].as<bool>();

    fDoBTag = false;
    fDoBTag = fConfig["Correction"]["BTag"].as<bool>();

    Print();

    fHistoSet = new HistoSetEMU(fEra, fSampleName);

    fNtuples = new NT(fConfig, fIsMC);
    fNtuples->SetSampleName(fSampleName);
    fNtuples->SetEra(fEra);
    fNtuples->AddChain(fSampleName, fJobID);

    std::cout << " " << std::endl;

    fMaxEntries = fNtuples->GetEntries();
    fNtuples->init();

    std::cout << " " << std::endl;

    fEMU = new EMU(fConfig);
    fEMU->IsMC(fIsMC);
    if (fIsMC) {
      const UInt_t tSmearingSeed = MakeMuonSmearingSeed(fEra, fSampleName, fJobID);
      fEMU->SetMuonSmearingSeed(tSmearingSeed);
    }
    fEMU->init(fNtuples->GetTreeReader());

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
    std::cout << " fDoISO: " << fDoISO << " " << fConfig["Efficiency"]["ISO"]["Path"].as<std::string>() << std::endl;
    std::cout << "         " << fDoISO << " " << fConfig["Efficiency"]["ISO"]["Name"].as<std::string>() << std::endl;
    std::cout << " fDoTRIGG   : " << fDoTRIGG << " " << fConfig["Efficiency"]["Trigger"]["Path"].as<std::string>() << std::endl;
    std::cout << "              " << fDoTRIGG << " " << fConfig["Efficiency"]["Trigger"]["Data"].as<std::string>() << std::endl;
    std::cout << "              " << fDoTRIGG << " " << fConfig["Efficiency"]["Trigger"]["MC"].as<std::string>() << std::endl;
    std::cout << " fDoElecReco: " << fDoElecReco << " " << fConfig["Efficiency"]["ElecReco"]["Path"].as<std::string>() << std::endl;
    std::cout << "             " << fDoElecReco << " " << fConfig["Efficiency"]["ElecReco"]["Name"].as<std::string>() << std::endl;
    std::cout << " fDoElecID: " << fDoElecID << " " << fConfig["Efficiency"]["ElecID"]["Path"].as<std::string>() << std::endl;
    std::cout << "            " << fDoElecID << " " << fConfig["Efficiency"]["ElecID"]["Name"].as<std::string>() << std::endl;
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
  static UInt_t MakeMuonSmearingSeed(const TString& fEra, const TString& fSampleName, int fJobID) {
    const std::string tKey =
      std::string(fEra.Data()) + ":" +
      std::string(fSampleName.Data()) + ":" +
      std::to_string(fJobID);

    std::uint32_t tHash = 2166136261u;
    for (const unsigned char tCharacter : tKey) {
      tHash ^= tCharacter;
      tHash *= 16777619u;
    }

    return tHash == 0 ? 1u : tHash;
  }

  TString fConfigPath;
  TString fEra;
  TString fSampleName;
  int fJobID;
  bool fIsMC;
  TString fOutputDir;

  LumiReWeighting* fPuReweighting;
  std::shared_ptr<const correction::Correction> fReco_SF;
  std::shared_ptr<const correction::Correction> fID_SF;
  std::shared_ptr<const correction::Correction> fISO_SF;
  std::shared_ptr<const correction::Correction> fTRIG_Eff_Data;
  std::shared_ptr<const correction::Correction> fTRIG_Eff_MC;
  std::shared_ptr<const correction::Correction> fElecReco_SF;
  std::shared_ptr<const correction::Correction> fElecID_SF;
  
  // TH2D* fElecMisCharge_SF;

  bool fDebug;

  bool fDoReco;
  bool fDoID;
  bool fDoISO;
  bool fDoTRIGG;
  bool fDoElecReco;
  bool fDoElecID;
  bool fDoElecMisCharge;
  bool fDoPU;
  bool fDoL1Pre;
  bool fDoJetPUID;
  bool fDoBTag;
  bool fDoTopPtReweighing;

  NT* fNtuples;
  YAML::Node fConfig;

  EMU* fEMU;
  JET* fJets;

  double fMaxEntries;

  HistoSetEMU* fHistoSet;

  WeigthEnvelope fWeightEnvelope;

  std::map<std::string, FuncCorrections> fCorrectionFuncs;
};

#endif
