#include <iostream>
#include <vector>

#include "jet.h"
#include "muon.h"
#include "electron.h"

#include "TChain.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TRandom.h"

void JET::init(TTreeReader* fTreeReader) {

  nJet = new TTreeReaderValue<unsigned int>(*fTreeReader, "nJet");
  Jet_pt = new TTreeReaderArray<float>(*fTreeReader, "Jet_pt");
  Jet_eta = new TTreeReaderArray<float>(*fTreeReader, "Jet_eta");
  Jet_phi = new TTreeReaderArray<float>(*fTreeReader, "Jet_phi");
  Jet_mass = new TTreeReaderArray<float>(*fTreeReader, "Jet_mass");
  Jet_jetId = new TTreeReaderArray<int>(*fTreeReader, "Jet_jetId");
  Jet_puId = new TTreeReaderArray<int>(*fTreeReader, "Jet_puId");
  Jet_btagDeepFlavB = new TTreeReaderArray<float>(*fTreeReader, "Jet_btagDeepFlavB");

  if (fIsMC) {

    Jet_genJetIdx = new TTreeReaderArray<int>(*fTreeReader, "Jet_genJetIdx");
    Jet_hadronFlavour = new TTreeReaderArray<int>(*fTreeReader, "Jet_hadronFlavour");
  }
}

bool JET::PrepareJet() {

  fFVecJets.clear();
  fFVecBJets.clear();
  fFVecHSJet.clear();

  for (int i = 0; i < **nJet; i++) {

    if (!(Jet_pt->At(i) > fJetPt))
      continue;

    if (!(std::fabs(Jet_eta->At(i)) < fEta))
      continue;

    if (!(Jet_jetId->At(i) >= fJetID))
      continue;

    TLorentzVector jets;
    jets.SetPtEtaPhiM(Jet_pt->At(i), Jet_eta->At(i), Jet_phi->At(i), Jet_mass->At(i));

    int hadFlav = -1;
    if (fIsMC)
      hadFlav = Jet_hadronFlavour->At(i);

    int genJetIdx = -1;
    if (fIsMC)
      genJetIdx = Jet_genJetIdx->At(i);

    bool isBJet = false;
    if (Jet_btagDeepFlavB->At(i) > fBJetTaggerCut)
      isBJet = true;

    bool tPassingPUJetID = false;
    if (Jet_puId->At(i) >= fJetPUID)
      tPassingPUJetID = true;

    if (Jet_pt->At(i) >= 50. || (Jet_pt->At(i) < 50. && tPassingPUJetID)) {

      fFVecJets.push_back(StdJet(jets, jets, isBJet, Jet_jetId->At(i), hadFlav, genJetIdx, tPassingPUJetID));
      if (isBJet)
        fFVecBJets.push_back(StdJet(jets, jets, isBJet, Jet_jetId->At(i), hadFlav, genJetIdx, tPassingPUJetID));

    }

    if (Jet_pt->At(i) < 50. && genJetIdx != -1) {

      fFVecHSJet.push_back(StdJet(jets, jets, isBJet, Jet_jetId->At(i), hadFlav, genJetIdx, tPassingPUJetID));
    }
  }

  // std::cout << "######################################################################" << std::endl;
  // std::cout << "                            Jet Debugging                             " << std::endl;
  // std::cout << "----------------------------------------------------------------------" << std::endl;
  // std::cout << " number of jets: " << fFVecJets.size() << std::endl;
  // for (int i = 0; i < fFVecJets.size(); i++) {
  //   std::cout << "  " << i << " th jet: " << std::endl;
  //   std::cout << "    " << i << " pT: " << fFVecJets.at(i).fVec.Pt() << std::endl;
  //   std::cout << "    " << i << " eta: " << fFVecJets.at(i).fVec.Eta() << std::endl;
  //   std::cout << "    " << i << " ID: " << fFVecJets.at(i).fID << std::endl;
  //   std::cout << "    " << i << " b-tagging: " << fFVecJets.at(i).fPassingBJetTagger << std::endl;
  //   std::cout << "    " << i << " PUID: " << fFVecJets.at(i).fPassingPUID << std::endl;
  // }

  // std::cout << " number of b-jets: " << fFVecBJets.size() << std::endl;
  // for (int i = 0; i < fFVecBJets.size(); i++) {
  //   std::cout << "  " << i << " th jet: " << std::endl;
  //   std::cout << "    " << i << " pT: " << fFVecBJets.at(i).fVec.Pt() << std::endl;
  //   std::cout << "    " << i << " eta: " << fFVecBJets.at(i).fVec.Eta() << std::endl;
  //   std::cout << "    " << i << " ID: " << fFVecBJets.at(i).fID << std::endl;
  //   std::cout << "    " << i << " b-tagging: " << fFVecBJets.at(i).fPassingBJetTagger << std::endl;
  //   std::cout << "    " << i << " PUID: " << fFVecBJets.at(i).fPassingPUID << std::endl;
  // }

  // std::cout << " number of HS jets: " << fFVecHSJet.size() << std::endl;
  // for (int i = 0; i < fFVecHSJet.size(); i++) {
  //   std::cout << "  " << i << " th jet: " << std::endl;
  //   std::cout << "    " << i << " pT: " << fFVecHSJet.at(i).fVec.Pt() << std::endl;
  //   std::cout << "    " << i << " eta: " << fFVecHSJet.at(i).fVec.Eta() << std::endl;
  //   std::cout << "    " << i << " ID: " << fFVecHSJet.at(i).fID << std::endl;
  //   std::cout << "    " << i << " b-tagging: " << fFVecHSJet.at(i).fPassingBJetTagger << std::endl;
  //   std::cout << "    " << i << " PUID: " << fFVecHSJet.at(i).fPassingPUID << std::endl;
  // }
  // std::cout << "######################################################################" << std::endl;

  return true;
}

double JET::GetPUIDSF() {

  double pMC = 1.;
  double pData = 1.;

  // std::cout << "######################################################################" << std::endl;
  // std::cout << "                          PUJetID Debugging                           " << std::endl;
  // std::cout << "----------------------------------------------------------------------" << std::endl;


  for (int i = 0; i < fFVecHSJet.size(); i++) {

    double tSF = fJetPUIDSF->evaluate({fFVecHSJet.at(i).fVec.Eta(), fFVecHSJet.at(i).fVec.Pt(), "nom", "L"});
    double tEff = fJetPUIDSF->evaluate({fFVecHSJet.at(i).fVec.Eta(), fFVecHSJet.at(i).fVec.Pt(), "MCEff", "L"});

    // std::cout << i << " th jet: " << std::endl;

    if (fFVecHSJet.at(i).fPassingPUID) {


      // std::cout << "    " << i << " " << fFVecHSJet.at(i).fPassingPUID << " " << tEff << "  " << tEff * tSF << std::endl;

      pMC *= tEff;
      pData *= tEff * tSF;
    
    } else {

      // std::cout << "    " << i << " " << fFVecHSJet.at(i).fPassingPUID << " " << "1 - " << tEff << "  1 - " << tEff * tSF << std::endl;

      pMC *= (1 - tEff);
      pData *= (1 - tEff * tSF);

    }

  }

  double weight = 1.;
  if (pMC > 0.) weight = pData / pMC;

  // std::cout << "pMC: " << pMC << " pData: " << pData << " weight: " << weight << std::endl;
  // std::cout << "######################################################################" << std::endl;

  return weight;
}

double JET::GetBTagSF() {

  double pMC = 1.;
  double pData = 1.;

  for (int i = 0; i < fFVecJets.size(); i++) {

    double tJetEta = std::abs(fFVecJets.at(i).fVec.Eta());
    double tJetPt = fFVecJets.at(i).fVec.Pt();
    int tHadFlav = fFVecJets.at(i).fHadFlav;

    if (tJetPt >= 1000.)
      tJetPt = 800.;

    double tSFcentral = 1.;
    if (tHadFlav == 5 || tHadFlav == 4) tSFcentral = fBTagMuJets->evaluate({"central", fBTagWP, tHadFlav, std::abs(tJetEta), tJetPt});
    else tSFcentral = fBTagIncl->evaluate({"central", fBTagWP, 0, std::abs(tJetEta), tJetPt});

    double tJetEff = 1.;
    if (tHadFlav == 5)        tJetEff = fJetBTagEffB.getEfficiency(tJetPt, tJetEta);
    else if (tHadFlav == 4)   tJetEff = fJetBTagEffC.getEfficiency(tJetPt, tJetEta);
    else                      tJetEff = fJetBTagEffL.getEfficiency(tJetPt, tJetEta);

    if (fFVecJets.at(i).fPassingBJetTagger) {
      pMC *= tJetEff;
      pData *= tJetEff * tSFcentral;
    } else {
      pMC *= (1 - tJetEff);
      pData *= (1 - tJetEff * tSFcentral);
    }

    // std::cout << tJetEff << " " << pMC << " " << tSFcentral << " " << tSFcentral * tJetEff << " " << pData << std::endl;
  }

  // std::cout << pData / pMC << std::endl;
  return pData / pMC;
}
