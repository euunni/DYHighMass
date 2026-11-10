#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import plotterEngine_MUMU as plotterEngine

def Merged(hist1, hist2, hist3, name):

    returnHist = ROOT.TH1D(name, "", 45, 0, 45)

    for i in range(1, 16):
        
        returnHist.SetBinContent(i, hist1.GetBinContent(i))
        returnHist.SetBinError(i, hist1.GetBinError(i))
        
        returnHist.SetBinContent(i + 15, hist2.GetBinContent(i))
        returnHist.SetBinError(i + 15, hist2.GetBinError(i))
        
        returnHist.SetBinContent(i + 30, hist3.GetBinContent(i))
        returnHist.SetBinError(i + 30, hist3.GetBinError(i))

    return returnHist


def main():

    eras = ["2018", "2017", "2016_postVFP", "2016_preVFP", "merged"]
    cases = ["_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"]

    types = ["Data", "DYReco", "DYFake", "TT", "DYtau", "EW", "GG", "Fake", "TotalMC"]

    histoSet = {}
    histoMergedSet = {}
 
    for era in eras:

        input_file = "../Bck/root/260908_MUMU_Update_merged"

        plotter = plotterEngine.Plotter(era, 
                                        rootPath = f"{input_file}.root", 
                                        outputPath = f"./temp",
                                        channel = "MUMU", 
                                        region = "OS")

        plotter.SetFakes(rootPath = "../Bck/MUMU_FAKE.root")

        histoSet[era] = {}
        histoMergedSet[era] = {}

        for type in types:
            histoSet[era][type] = {}
            histoMergedSet[era][type] = {}
        
        for case in cases:
            histName = case + "/h_OS_dimuonMass" + case
            histName_FailGen = case + "/h_OS_dimuonMassFailGen" + case

            data = plotter.GetDataHist(histName)
            data.SetName(f"Data{case}")

            DY = plotter.GetMCHist(histName, plotterEngine.DYMCList, CheckSanity = False)
            DY.SetName(f"DYReco{case}")

            DYFake_Sample = []
            for DYSample in plotterEngine.DYMCList:
                DYFake_Sample.append(plotter.GetSingleHist(histName_FailGen, DYSample))

            DYFake = DYFake_Sample[0].Clone(f"DYFake{case}")
            for i in range(1, len(DYFake_Sample)):
                DYFake.Add(DYFake_Sample[i])

            TT = plotter.GetMCHist(histName, plotterEngine.TWMCList, CheckSanity = True)
            TT.SetName(f"TT{case}")

            DY_tau = plotter.GetMCHist(histName, ["NNLO_tautau"], CheckSanity = False)
            DY_tau.SetName(f"DYtau{case}")

            EW = plotter.GetMCHist(histName, ["WZ", "ZZ"], CheckSanity = True)
            EW.SetName(f"EW{case}")

            GG = plotter.GetMCHist(histName, ["GG"], CheckSanity = True)
            GG.SetName(f"GG{case}")

            Fake = plotter.FakeFile.Get(era + "/FAKE_MUMU_OS/MUMU_OS_FAKE_DataDriven" + case)
            Fake.SetName(f"Fake{case}")

            TotalMC = TT.Clone(f"TotalMC{case}")
            TotalMC.Add(DY_tau)
            TotalMC.Add(EW)
            TotalMC.Add(GG)
            TotalMC.Add(Fake)

            histoSet[era]["Data"][case] = data
            histoSet[era]["DYReco"][case] = DY
            histoSet[era]["DYFake"][case] = DYFake
            histoSet[era]["TT"][case] = TT
            histoSet[era]["DYtau"][case] = DY_tau
            histoSet[era]["EW"][case] = EW
            histoSet[era]["GG"][case] = GG
            histoSet[era]["Fake"][case] = Fake
            histoSet[era]["TotalMC"][case] = TotalMC

        for type in types:
            hist1 = histoSet[era][type][cases[0]]
            hist2 = histoSet[era][type][cases[1]]
            hist3 = histoSet[era][type][cases[2]]
            histoMergedSet[era][type] = Merged(hist1, hist2, hist3, f"{type}")

    fOutput = ROOT.TFile(f"./store_MUMU.root", "RECREATE")
    for era in eras:
        fOutput.cd()
        fOutput.mkdir(era)
        fOutput.cd(f"{era}")

        for type in types:
            histoMergedSet[era][type].Write()
            histoSet[era][type][cases[0]].Write()
            histoSet[era][type][cases[1]].Write()
            histoSet[era][type][cases[2]].Write()

    fOutput.Close()

if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    ROOT.TH1.SetDefaultSumw2()
    
    main()
