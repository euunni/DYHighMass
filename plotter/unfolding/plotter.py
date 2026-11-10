#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import plotterEngine_MUMU as plotterEngine
import numpy as np

def GetSpliitedHist(fHist, name = "plotname"):

    fMassBin = np.array([200,  220,  243, 273, 320, 380, 440, 510, 600, 700, 830, 1000, 1500, 4000], dtype=float)
    
    fHist_0J = ROOT.TH1D(f"{name}_0J", "", 13, fMassBin)
    fHist_1J = ROOT.TH1D(f"{name}_1J", "", 13, fMassBin)
    fHist_mt1J = ROOT.TH1D(f"{name}_mt1J", "", 13, fMassBin)

    for i in range(1, 14):

        fHist_0J.SetBinContent(i, fHist.GetBinContent(i + 1))
        fHist_0J.SetBinError(i, fHist.GetBinError(i + 1))

        fHist_1J.SetBinContent(i, fHist.GetBinContent(i + 16))
        fHist_1J.SetBinError(i, fHist.GetBinError(i + 16))

        fHist_mt1J.SetBinContent(i, fHist.GetBinContent(i + 31))
        fHist_mt1J.SetBinError(i, fHist.GetBinError(i + 31))

    return fHist_0J, fHist_1J, fHist_mt1J


def GetMaximum(hHist1, hHist2):

    return max(hHist1.GetMaximum(), hHist2.GetMaximum())


def SetError(hRef, hError = None):

    for i in range(1, hRef.GetNbinsX() + 1):
        if hError is None:
            hRef.SetBinError(i, 1e-8)
        else:
            hRef.SetBinError(i, hError.GetBinError(i))

    return hRef

def GetYRange(hHist):

    max_idx = -9999;
    for i in range(1, hHist.GetNbinsX() + 1):
        residual = max(abs(1 - (hHist.GetBinContent(i) + hHist.GetBinError(i))), abs(1 - (hHist.GetBinContent(i) - hHist.GetBinError(i))))
        if max_idx < residual:
            max_idx = residual

    return max_idx

def plotter(hData, hPred, name, var = {}):


    hRatio = hPred.Clone(f"{name}_Ratio")
    hRatio.Divide(hData)
    hRatio = SetError(hRatio)

    ratio_residual = GetYRange(hRatio)

    fCanvas = CMS.cmsDiCanvas(
        "Canv_{name}",
        200,
        4000,
        2e-2 if "ymin" not in var else var["ymin"],
        GetMaximum(hPred, hData) * 1e3 if "ymax" not in var else var["ymax"],
        1 - ratio_residual * 1.1  if "yrmin" not in var else var["yrmin"],
        1 + ratio_residual * 1.1 if "yrmax" not in var else var["yrmax"],
        "M(#mu#mu) [GeV]" if "xlabel" not in var else var["xlabel"],
        "Events" if "ylabel" not in var else var["ylabel"],
        "Pred./Data" if "rtitle" not in var else var["rtitle"],
        square = CMS.kSquare,
        extraSpace = 0.1,
        iPos = 0,
    )


    fCanvas.cd(1).SetLogx(True)
    fCanvas.cd(1).SetLogy(True)
    fCanvas.cd(2).SetLogx(True)

    fCanvas.cd(1)
    flegend = CMS.cmsLeg(0.70, 0.70 - 0.05 * 2, 0.89, 0.70, textSize=0.03)
    flegend.AddEntry(hData, "data", "lp")
    flegend.AddEntry(hPred, "DY MC", "lp")

    fCanvas.cd(1)
    CMS.cmsDraw(hPred, "P", mcolor=ROOT.kRed)
    CMS.cmsDraw(hData, "P", mcolor=ROOT.kBlack)

    fCanvas.cd(2)
    CMS.cmsDraw(hRatio, "P", mcolor=ROOT.kBlack)


    # latex_MUMU_SS_FAKE_inverted_item = latex_copy_mumu.copy()
    # latex_MUMU_SS_FAKE_inverted_item[0] = f"{latex_MUMU_SS_FAKE_inverted_item[0]}, SS, Fake CR"

    # Latex_MUMU_SS_FAKE_inverted = ROOT.TLatex()
    # Latex_MUMU_SS_FAKE_inverted.SetTextAlign(14);
    # Latex_MUMU_SS_FAKE_inverted.SetTextSize(0.04);
    # Latex_MUMU_SS_FAKE_inverted.SetTextFont(42);
    # for idx, addon in enumerate(latex_MUMU_SS_FAKE_inverted_item):
    #     Latex_MUMU_SS_FAKE_inverted.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

    CMS.SaveCanvas(fCanvas, f"{name}.pdf" if "outputPath" not in var else var["outputPath"] + ".pdf")



def main():

    fUnfolded = ROOT.TFile.Open("Unfolding_v1.root")
    fInput = ROOT.TFile.Open("store_v1.root")
    fInput_v2 = ROOT.TFile.Open("store_MUMU.root")

    hUnfolded = fUnfolded.Get("Unfolded_Run2")
    hGen = fInput.Get("Gen_Run2")
    hDYReco = fInput_v2.Get("merged/DYReco")
    hData = fInput_v2.Get("merged/Data")
    hMC = fInput_v2.Get("merged/TotalMC")

    hDataOnly = hData.Clone("DataOnly")
    hDataOnly.Add(hMC, -1)

    fUnfolded.Close()
    fInput.Close()

    hUnfolded_0J, hUnfolded_1J, hUnfolded_mt1J = GetSpliitedHist(hUnfolded, "Unfolded")
    hGen_0J, hGen_1J, hGen_mt1J = GetSpliitedHist(hGen, "Gen")

    hReco_0J, hReco_1J, hReco_mt1J = GetSpliitedHist(hDYReco, "Reco")
    hData_0J, hData_1J, hData_mt1J = GetSpliitedHist(hDataOnly, "Data")

    plotter(hUnfolded_0J, hGen_0J, "h0J", {"outputPath": "h_0J", "yrmin": 0.75, "yrmax": 1.25})
    plotter(hUnfolded_1J, hGen_1J, "h0J", {"outputPath": "h_1J", "yrmin": 0.75, "yrmax": 1.25})
    plotter(hUnfolded_mt1J, hGen_mt1J, "h0J", {"outputPath": "h_mt1J", "ymin": 2e-3, "yrmin": 0.6, "yrmax": 1.4})
    
    plotter(hData_0J, hReco_0J, "h_Reco_0J", {"outputPath": "h_Reco_0J", "yrmin": 0.75, "yrmax": 1.25})
    plotter(hData_1J, hReco_1J, "h_Reco_1J", {"outputPath": "h_Reco_1J", "yrmin": 0.75, "yrmax": 1.25})
    plotter(hData_mt1J, hReco_mt1J, "h_Reco_mt1J", {"outputPath": "h_Reco_mt1J", "ymin": 2e-3, "yrmin": 0.6, "yrmax": 1.4})









if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    ROOT.TH1.SetDefaultSumw2()
    
    main()
