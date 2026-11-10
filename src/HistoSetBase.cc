#include "HistoSetBase.h"
#include "jet.h"

#include "TFile.h"
#include "TLorentzVector.h"
#include "TH1.h"



void HistoSetBase::BookHisto(
  TString fRegionName,
  TString fJetBin,
  TString fMassBin,
  TString fObservable,
  TString fHistoName
) {

  const auto fBins = GetBinning(fHistoName.Data());

  if (fBins.empty())
    throw std::invalid_argument("Error: HistoSetBase::BookHisto: Unknown histogram name: " + std::string(fHistoName.Data()));

  auto& fHist = fHistSet[fSampleName].fRegionCollection[fRegionName][fJetBin][fMassBin][fObservable];
  if (fBins[0] == -9999) {
    fHist = std::make_unique<TH1D>(fHistoName, fHistoName, static_cast<int>(fBins[1]), fBins[2], fBins[3]);
  } else {
    fHist = std::make_unique<TH1D>(fHistoName, fHistoName, static_cast<int>(fBins.size()) - 1, fBins.data());
  }
  fHist->SetDirectory(nullptr);
}

void HistoSetBase::BookHisto(
  TString fRegionName,
  TString fJetBin,
  TString fMassBin,
  TString fObservable,
  TString fHistoName,
  std::vector<double> fBins
) {

  if (fBins.empty())
    throw std::invalid_argument("Error: HistoSetBase::BookHisto: Empty binning for " + std::string(fHistoName.Data()));

  auto& fHist = fHistSet[fSampleName].fRegionCollection[fRegionName][fJetBin][fMassBin][fObservable];
  if (fBins[0] == -9999) {
    fHist = std::make_unique<TH1D>(fHistoName, fHistoName, static_cast<int>(fBins[1]), fBins[2], fBins[3]);
  } else {
    fHist = std::make_unique<TH1D>(fHistoName, fHistoName, static_cast<int>(fBins.size()) - 1, fBins.data());
  }
  fHist->SetDirectory(nullptr);
}

void HistoSetBase::BookHisto(
  HistoSetBase::HistGroup fHistGroup,
  TString fObservable,
  TString fHistoName,
  std::vector<double> fBins
) {

  if (fBins.empty())
    throw std::invalid_argument("Error: HistoSetBase::BookHisto: Empty binning for " + std::string(fHistoName.Data()));

  std::unique_ptr<TH1> fHist;
  if (fBins[0] == -9999) {
    fHist = std::make_unique<TH1D>(fHistoName, fHistoName, static_cast<int>(fBins[1]), fBins[2], fBins[3]);
  } else {
    fHist = std::make_unique<TH1D>(fHistoName, fHistoName, static_cast<int>(fBins.size()) - 1, fBins.data());
  }
  fHist->SetDirectory(nullptr);

  if(fHistGroup == HistoSetBase::HistGroup::EventInfo) {
    fHistSet[fSampleName].fEventInfo[fObservable] = std::move(fHist);
  }

  if(fHistGroup == HistoSetBase::HistGroup::GenInfo) {
    fHistSet[fSampleName].fGenInfo[fObservable] = std::move(fHist);
  }
}

void HistoSetBase::BookHisto(
  HistoSetBase::HistGroup fHistGroup,
  TString fObservable,
  TH2D* fHisto
) {

  if (!fHisto)
    throw std::invalid_argument("Error: HistoSetBase::BookHisto: Null 2D histogram");

  fHisto->SetDirectory(nullptr);
  if (fHistGroup == HistoSetBase::HistGroup::GenInfo)
    fHistSet[fSampleName].fGenInfo[fObservable].reset(fHisto);
  if (fHistGroup == HistoSetBase::HistGroup::EventInfo)
    fHistSet[fSampleName].fEventInfo[fObservable].reset(fHisto);
}

void HistoSetBase::FillHisto(
  TString fRegionName,
  TString fJetBin,
  TString fMassBin,
  TString fObservable,
  double fValue,
  double fWeight
){

  auto& fHist = fHistSet[fSampleName].fRegionCollection[fRegionName][fJetBin][fMassBin][fObservable];
  
  if (!fHist)
    throw std::invalid_argument(
      "Error: HistoSetBase::FillHisto: Histogram not found: " +
      std::string((fRegionName + " " + fJetBin + " " + fMassBin + " " + fObservable).Data())
    );

  fHist->Fill(fValue, fWeight);
}

void HistoSetBase::FillHisto(
  HistoSetBase::HistGroup fHistGroup,
  TString fObservable,
  double fValue,
  double fWeight
) {

  if (fHistGroup == HistoSetBase::HistGroup::EventInfo) {
    auto& fHist = fHistSet[fSampleName].fEventInfo[fObservable];
    if (!fHist)
      throw std::invalid_argument("Error: HistoSetBase::FillHisto: Histogram not found: " + fObservable);

    fHist->Fill(fValue, fWeight);
  }

  if (fHistGroup == HistoSetBase::HistGroup::GenInfo) {
    auto& fHist = fHistSet[fSampleName].fGenInfo[fObservable];
    if (!fHist)
      throw std::invalid_argument("Error: HistoSetBase::FillHisto: Histogram not found: " + fObservable);

    fHist->Fill(fValue, fWeight);
  }
  
}

void HistoSetBase::FillHisto(
  HistoSetBase::HistGroup fHistGroup,
  TString fObservable,
  double fXValue,
  double fYValue,
  double fWeight
) {

  ObservableCollection* fHistCollection = nullptr;
  if (fHistGroup == HistoSetBase::HistGroup::EventInfo)
    fHistCollection = &fHistSet[fSampleName].fEventInfo;
  if (fHistGroup == HistoSetBase::HistGroup::GenInfo)
    fHistCollection = &fHistSet[fSampleName].fGenInfo;

  auto& fHist = (*fHistCollection)[fObservable];
  auto* fHist2D = dynamic_cast<TH2*>(fHist.get());
  if (!fHist2D)
    throw std::invalid_argument("Error: HistoSetBase::FillHisto: 2D histogram not found: " + std::string(fObservable.Data()));

  fHist2D->Fill(fXValue, fYValue, fWeight);
}


void HistoSetBase::WriteHisto(TFile* fOutputFile, bool fIsData) {

  TDirectory::TContext fContext(fOutputFile);

  auto GetDir = [](TDirectory* fParent, const TString& fName) {
    
    TDirectory* fDir = fParent->GetDirectory(fName.Data());
    if (!fDir) fDir = fParent->mkdir(fName.Data());
    if (!fDir) throw std::runtime_error("Cannot create output directory");
    
    return fDir;
  };

  auto WriteObservables = [](TDirectory* fDir, const ObservableCollection& fObservables) {
    fDir->cd();

    for (const auto& [tName, tHist] : fObservables) {
      if (tHist)
        tHist->Write(tHist->GetName());
    }
  };

  auto* dEra = GetDir(fOutputFile, fEra);

  for (const auto& [tSampleName, cSample] : fHistSet) {
    auto* dSample = GetDir(dEra, tSampleName);

    WriteObservables(
        GetDir(dSample, "EventInfo"),
        cSample.fEventInfo
    );

    if (tSampleName.Contains("NNLO_MUMU")) {
      WriteObservables(
        GetDir(dSample, "GenInfo"),
        cSample.fGenInfo
      );
    }

    for (const auto& [tRegionName, cJet] : cSample.fRegionCollection) {
      auto* dRegion = GetDir(dSample, tRegionName);

      for (const auto& [tJetName, cMass] : cJet) {
        auto* dJet = GetDir(dRegion, tJetName);

        for (const auto& [tMassName, cObs] : cMass) {
          WriteObservables(
            GetDir(dJet, tMassName),
            cObs
          );
        }
      }
    }
  }

  if (fIsData) {
    for (const auto& [tSampleName, cSample] : fHistSet) {
      auto* dSample = GetDir(dEra, "Data");

      WriteObservables(
        GetDir(dSample, "EventInfo"),
        cSample.fEventInfo
      );
  
      for (const auto& [tRegionName, cJet] : cSample.fRegionCollection) {
        auto* dRegion = GetDir(dSample, tRegionName);
  
        for (const auto& [tJetName, cMass] : cJet) {
          auto* dJet = GetDir(dRegion, tJetName);
  
          for (const auto& [tMassName, cObs] : cMass) {
            WriteObservables(
              GetDir(dJet, tMassName),
              cObs
            );
          }
        }
      }
    }
  }
}

