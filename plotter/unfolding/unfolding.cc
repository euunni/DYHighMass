// unfold_tunfold.C
// ROOT macro for unfolding with TUnfoldDensity
// Run: root -l -b -q unfold_tunfold.C

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TUnfoldDensity.h"
#include "TGraph.h"
#include "TSpline.h"
#include "TCanvas.h"

void SetBin(TH1D* fHist) {
    for (int i = 1; i <= fHist->GetNbinsX(); i++) {
        if (fHist->GetBinContent(i) < 0) fHist->SetBinContent(i, 0);

        if (fHist->GetBinContent(i) <= 0) fHist->SetBinError(i, 1);
        else fHist->SetBinError(i, std::sqrt(fHist->GetBinContent(i)));
    }
}

void SetBin(TH2D* fHist) {
    for (int i = 0; i <= fHist->GetNbinsX(); i++) {
        for (int j = 1; j <= fHist->GetNbinsY(); j++) {
            if (fHist->GetBinContent(i, j) < 0) fHist->SetBinContent(i, j, 0);

            if (fHist->GetBinContent(i, j) <= 0) fHist->SetBinError(i, j, 1);
            else fHist->SetBinError(i, j, std::sqrt(fHist->GetBinContent(i, j)));
        }
    }
}

void SetBin_v2(TH1D* fHist) {
    for (int i = 1; i <= fHist->GetNbinsX(); i++) {
        if (fHist->GetBinContent(i) <= 0) fHist->SetBinError(i, 1);
        else fHist->SetBinError(i, std::sqrt(fHist->GetBinContent(i)));
    }
}

void SetBin_v2(TH2D* fHist) {
    for (int i = 0; i <= fHist->GetNbinsX(); i++) {
        for (int j = 1; j <= fHist->GetNbinsY(); j++) {
            if (fHist->GetBinContent(i, j) <= 0) fHist->SetBinError(i, j, 1);
            else fHist->SetBinError(i, j, std::sqrt(fHist->GetBinContent(i, j)));
        }
    }
}


void unfolding() {

    TH1::AddDirectory(false);


    // std::vector<std::string> fEras = {"Run2", "2018", "2017", "2016_postVFP", "2016_preVFP"};
    // std::vector<std::string> fEras = {"Medged"};

    TFile* fOut = new TFile("Unfolding_v1.root", "RECREATE");

    TFile* fIn_v1 = new TFile("./store_v1.root", "READ");
    TFile* fIn_v2 = new TFile("./store_MUMU.root", "READ");

    TH1D* fHist_Data     = (TH1D*) fIn_v2->Get("merged/Data");
    TH1D* fHist_Bkg      = (TH1D*) fIn_v2->Get("merged/TotalMC");
    TH1D* fHist_Fake     = (TH1D*) fIn_v2->Get("merged/DYFake");
    TH2D* fHist_Response = (TH2D*) fIn_v1->Get("responseMap_Run2");

    fIn_v1->Close();
    fIn_v2->Close();

    if (fHist_Data == nullptr) {
        Printf("ERROR: fHist_Data is not found");
        return;
    }

    if (fHist_Bkg == nullptr) {
        Printf("ERROR: fHist_Bkg is not found");
        return;
    }

    if (fHist_Fake == nullptr) {
        Printf("ERROR: fHist_Fake is not found");
        return;
    }

    if (fHist_Response == nullptr) {
        Printf("ERROR: fHist_Response is not found");
        return;
    }

    // ERegMode
    // kRegModeNone : regularization 없음
    // kRegModeSize : 결과값 크기 자체를 억제 (0차 미분)
    // kRegModeDerivative : 인접 bin 간 1차 미분 억제
    // kRegModeCurvature : 인접 bin 간 2차 미분(곡률) 억제
    // kRegModeMixed : 사용자가 직접 regularization 조건을 추가할 때 사용

    // EConstraint
    // kEConstraintNone : 전체 이벤트 수 보존 안 함
    // kEConstraintArea : 전체 적분이 입력 데이터와 일치하도록 강제

    // EDensityMode
    // kDensityModeNone : bin 폭 보정 없음
    // kDensityModeBinWidth : bin 폭으로 나눈 밀도 기준으로 regularization
    // kDensityModeUser : 사용자 정의 밀도 factor 사용
    // kDensityModeBinWidthAndUser : bin 폭 + 사용자 정의 factor 동시 적용

    TUnfold::ERegMode    regMode      = TUnfold::kRegModeNone;
    TUnfold::EConstraint constraint   = TUnfold::kEConstraintNone;
    TUnfoldDensity::EDensityMode densityMode = TUnfoldDensity::kDensityModeNone;

    TUnfoldDensity unfold(fHist_Response,
                        TUnfold::kHistMapOutputVert, // truth on vertical (Y) axis
                        regMode,
                        constraint,
                        densityMode);

    if (unfold.SetInput(fHist_Data) >= 10000) {
        Printf("WARNING: input data badly constrained by response matrix");
    }

    unfold.SubtractBackground(fHist_Bkg, "Bkg", 1.0, 0.0);
    unfold.SubtractBackground(fHist_Fake, "Fake", 1.0, 0.0);

    unfold.DoUnfold(0.0);

    TH1*  hUnfolded = unfold.GetOutput("hUnfolded");        
    TH2*  hEmatTot  = unfold.GetEmatrixTotal("hEmatTot");   
    TH2*  hRhoIJ    = unfold.GetRhoIJtotal("hRhoIJ");       
    TH1*  hFoldedBack = unfold.GetFoldedOutput("hFoldedBack"); 

    hEmatTot->SetName(Form("EmatTot_%s", "Run2"));
    hRhoIJ->SetName(Form("RhoIJ_%s", "Run2"));
    hUnfolded->SetName(Form("Unfolded_%s", "Run2"));
    hFoldedBack->SetName(Form("FoldedBack_%s", "Run2"));

    Printf("chi2(A)   = %g", unfold.GetChi2A());
    Printf("chi2(L)   = %g", unfold.GetChi2L());
    Printf("Ndf       = %d", unfold.GetNdf());

    fOut->cd();

    hUnfolded->Write();
    hEmatTot->Write();
    hRhoIJ->Write();
    hFoldedBack->Write();
    fHist_Data->Write();
    fHist_Bkg->Write();
    fHist_Fake->Write();

    fOut->Close();
}