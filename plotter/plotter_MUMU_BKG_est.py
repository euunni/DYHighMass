#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import plotterEngine_MUMU as plotterEngine
import numpy as np

CMS.SetExtraText("Private Work")
CMS.SetEnergy("13")
ROOT.gROOT.SetBatch(ROOT.kTRUE)

def SanityCheck(hist):
    for i in range(1, hist.GetNbinsX() + 1):
        if hist.GetBinContent(i) <= 0:
            hist.SetBinContent(i, 0)
    return hist

def GetYRange(hist):
    yrmin = 9999
    yrmax = -9999

    for i in range(1, hist.GetNbinsX() + 1):
        if (hist.GetBinCenter(i) > 40 and hist.GetBinCenter(i) < 3000):
            if (hist.GetBinContent(i) != 0):
                yrmin = min(yrmin, hist.GetBinContent(i))
                yrmax = max(yrmax, hist.GetBinContent(i))

    yrmin = yrmin * 0.9
    yrmax = yrmax * 1.1

    if yrmin > 1: yrmin = 0

    return yrmin, yrmax

def Rebin(hist):
      
    fRebinBins = np.array([
        39, 40, 45, 50, 55, 60, 64, 68, 72, 76, 81, 86, 91, 96, 101, 106,
        110, 115, 120, 126, 133, 141, 150, 160, 171, 185, 200, 220, 243,
        273, 320, 380, 440, 510, 600, 700, 830, 1000, 1500, 2000, 3000,
        3001,
    ], dtype=np.double)
    fReturnHist = hist.Rebin(len(fRebinBins) - 1, f"h_dimuonMass_rebin_{uuid.uuid4()}", fRebinBins)

    return fReturnHist



addon_hook_jet = {
    "": "",
    "_0BJ": "b-veto",
}

mcList = [
    "NNLO_tautau",
    
    "ST_tW_AntiTop",
    "ST_tW_Top",

    "TTTo2L2Nu",
    
    "WW",
    "WZ",
    "ZZ",

    "GGToMuMu_10to30_ElEl",
    "GGToMuMu_30to50_ElEl",
    "GGToMuMu_50to200_ElEl",
    "GGToMuMu_200to1500_ElEl",
    "GGToMuMu_1500toInf_ElEl",

    "GGToMuMu_10to30_InelElElInel",
    "GGToMuMu_30to50_InelElElInel",
    "GGToMuMu_50to200_InelElElInel",
    "GGToMuMu_200to1500_InelElElInel",
    "GGToMuMu_1500toInf_InelElElInel",

    "GGToMuMu_10to30_InelInel",
    "GGToMuMu_30to50_InelInel",
    "GGToMuMu_50to200_InelInel",
    "GGToMuMu_200to1500_InelInel",
    "GGToMuMu_1500toInf_InelInel",
]

emu_mcList_woTop = [
    "NNLO_tautau",
    
    "WZ",
    "ZZ",

    "GGToMuMu_10to30_ElEl",
    "GGToMuMu_30to50_ElEl",
    "GGToMuMu_50to200_ElEl",
    "GGToMuMu_200to1500_ElEl",
    "GGToMuMu_1500toInf_ElEl",

    "GGToMuMu_10to30_InelElElInel",
    "GGToMuMu_30to50_InelElElInel",
    "GGToMuMu_50to200_InelElElInel",
    "GGToMuMu_200to1500_InelElElInel",
    "GGToMuMu_1500toInf_InelElElInel",

    "GGToMuMu_10to30_InelInel",
    "GGToMuMu_30to50_InelInel",
    "GGToMuMu_50to200_InelInel",
    "GGToMuMu_200to1500_InelInel",
    "GGToMuMu_1500toInf_InelInel",
]

def ErrorPropagation(x1, dx1, x2, dx2):

    if x1 == 0 or x2 == 0:
        # print(f"Error Zero Central value x1: {x1}, dx1: {dx1}, x2: {x2}, dx2: {dx2}")
        return 0

    return (x1 * x2) * np.sqrt((dx1 / x1)**2 + (dx2 / x2)**2)


def GetOSFromSS(SS_Fake, SStoOS):

    FAKE_DataDriven = SS_Fake.Clone(f"FAKE_DataDriven_{uuid.uuid4()}")
    FAKE_DataDriven.Reset("ICES");

    for i in range(1, SStoOS.GetNbinsX() + 1):
        if SStoOS.GetBinContent(i) == 0 or SS_Fake.GetBinContent(i) <= 0:
            FAKE_DataDriven.SetBinContent(i, 0)
        else:
            FAKE_DataDriven.SetBinContent(i, SS_Fake.GetBinContent(i) * SStoOS.GetBinContent(i))

    return FAKE_DataDriven

def GetOSFromSS_rebin(SS_Fake, SStoOS):

    FAKE_DataDriven = SS_Fake.Clone(f"FAKE_DataDriven_rebin_{uuid.uuid4()}")
    FAKE_DataDriven.Reset("ICES");

    for i in range(1, SS_Fake.GetNbinsX() + 1):
        if SStoOS.GetBinContent(SStoOS.FindBin(SS_Fake.GetBinCenter(i))) == 0 or SS_Fake.GetBinContent(i) <= 0:
            FAKE_DataDriven.SetBinContent(i, 0)
            FAKE_DataDriven.SetBinError(i, ErrorPropagation(SStoOS.GetBinContent(SStoOS.FindBin(SS_Fake.GetBinCenter(i))), SStoOS.GetBinError(SStoOS.FindBin(SS_Fake.GetBinCenter(i))), SS_Fake.GetBinContent(i), SS_Fake.GetBinError(i)))
        else:
            FAKE_DataDriven.SetBinContent(i, SS_Fake.GetBinContent(i) * SStoOS.GetBinContent(SStoOS.FindBin(SS_Fake.GetBinCenter(i))))
            FAKE_DataDriven.SetBinError(i, ErrorPropagation(SStoOS.GetBinContent(SStoOS.FindBin(SS_Fake.GetBinCenter(i))), SStoOS.GetBinError(SStoOS.FindBin(SS_Fake.GetBinCenter(i))), SS_Fake.GetBinContent(i), SS_Fake.GetBinError(i)))
            
    return FAKE_DataDriven

def GetHistoName(name, type, jet):

    histoName = ""
    if jet != "":
        histoName = f"{jet}/h_{type}_{name}{jet}"
    if jet == "":
        histoName = f"h_{type}_{name}"

    return histoName

def main():

    eras = ["2016_preVFP", "2016_postVFP", "2017", "2018", "merged"]
    cases = ["", "_0BJ"]
    
    histoName_MUMU = "h_dimuonMass"
    outputPath = "./plots_260917/MUMU_FAKE/"
    outputRoot = "./Bck/MUMU_FAKE.root"

    os.makedirs(outputPath, exist_ok=True)

    outputFile = ROOT.TFile(outputRoot, "RECREATE")

    for era in eras:

        CMS.SetLumi(plotterEngine.refLumi[era])
        os.makedirs(f"{outputPath}era_{era}", exist_ok=True)

        latex_mumu = [
            f"{era}",
            "p_{T}(#mu) > 52 (50) GeV, |#eta(#mu)| < 2.4",
            "M_{#mu#mu} > 40 GeV",
            ""
        ]

        if era == "merged":
            latex_mumu[0] = "Run2"

        outputFile.cd()
        outputFile.mkdir(f"{era}/FAKE_MUMU_OS")
        outputFile.mkdir(f"{era}/FAKE_MUMU_SS")
        outputFile.mkdir(f"{era}/FAKE_MUMU_SStoOS")
        outputFile.mkdir(f"{era}/FAKE_MUMU_OS_Inv_rebin")
        outputFile.mkdir(f"{era}/FAKE_MUMU_SS_Inv_rebin")

        MUMU_OS = plotterEngine.Plotter(era, rootPath = "./Bck/root/260908_MUMU_Update_merged.root",
                                            outputPath = "./plots/temp/plots" + era + "/",
                                            channel = "MUMU", 
                                            region = "OS")

        MUMU_SS = plotterEngine.Plotter(era, rootPath = "./Bck/root/260908_MUMU_Update_merged.root",
                                            outputPath = "./plots/temp/plots" + era + "/",
                                            channel = "MUMU", 
                                            region = "SS")

        MUMU_OS_inverted = plotterEngine.Plotter(era, rootPath = "./Bck/root/260908_MUMU_Update_merged.root",
                                            outputPath = "./plots/temp/plots" + era + "/",
                                            channel = "MUMU", 
                                            region = "OS_inverted")

        MUMU_SS_inverted = plotterEngine.Plotter(era, rootPath = "./Bck/root/260908_MUMU_Update_merged.root",
                                            outputPath = "./plots/temp/plots" + era + "/",
                                            channel = "MUMU", 
                                            region = "SS_inverted")

        for case in cases:

            latex_copy_mumu = latex_mumu.copy()

            if case != "":
                latex_copy_mumu[2] = f"{latex_copy_mumu[2]}, {addon_hook_jet[case]}"

            MUMU_SS_TotalMC = MUMU_SS.GetMCHist(GetHistoName("dimuonMass", "SS", case), mcList)
            MUMU_SS_data = MUMU_SS.GetDataHist(GetHistoName("dimuonMass", "SS", case))

            MUMU_SS_FAKE = MUMU_SS_data.Clone(f"MUMU_SS_FAKE_{uuid.uuid4()}")
            MUMU_SS_FAKE.Add(MUMU_SS_TotalMC, -1)
            # MUMU_SS_FAKE = SanityCheck(MUMU_SS_FAKE)
            MUMU_SS_FAKE.SetName("MUMU_SS_FAKE" + case)

            MUMU_OS_inverted_TotalMC = MUMU_OS_inverted.GetMCHist(GetHistoName("dimuonMass", "OS_inverted", case), mcList)
            MUMU_OS_inverted_data = MUMU_OS_inverted.GetDataHist(GetHistoName("dimuonMass", "OS_inverted", case))
            
            MUMU_OS_inverted_FAKE = MUMU_OS_inverted_data.Clone(f"MUMU_OS_inverted_FAKE_{uuid.uuid4()}")
            MUMU_OS_inverted_FAKE.Add(MUMU_OS_inverted_TotalMC, -1)
            # MUMU_OS_inverted_FAKE = SanityCheck(MUMU_OS_inverted_FAKE)

            MUMU_SS_inverted_TotalMC = MUMU_SS_inverted.GetMCHist(GetHistoName("dimuonMass", "SS_inverted", case), mcList)
            MUMU_SS_inverted_data = MUMU_SS_inverted.GetDataHist(GetHistoName("dimuonMass", "SS_inverted", case))

            MUMU_SS_inverted_FAKE = MUMU_SS_inverted_data.Clone(f"MUMU_SS_inverted_FAKE_{uuid.uuid4()}")
            MUMU_SS_inverted_FAKE.Add(MUMU_SS_inverted_TotalMC, -1)
            # MUMU_SS_inverted_FAKE = SanityCheck(MUMU_SS_inverted_FAKE)

            MUMU_OS_inverted_FAKE_rebin = Rebin(MUMU_OS_inverted_FAKE)
            MUMU_SS_inverted_FAKE_rebin = Rebin(MUMU_SS_inverted_FAKE)

            # MUMU_inverted_SStoOS = MUMU_OS_inverted_FAKE_rebin.Clone(f"MUMU_inverted_SStoOS_{uuid.uuid4()}")
            # MUMU_inverted_SStoOS.Divide(MUMU_SS_inverted_FAKE_rebin)
            # MUMU_inverted_SStoOS = SanityCheck(MUMU_inverted_SStoOS)
            # MUMU_inverted_SStoOS.SetName("MUMU_inverted_SStoOS" + case)

            MUMU_inverted_SStoOS = MUMU_OS_inverted_FAKE.Clone(f"MUMU_inverted_SStoOS_{uuid.uuid4()}")
            MUMU_inverted_SStoOS.Divide(MUMU_SS_inverted_FAKE)
            MUMU_inverted_SStoOS = SanityCheck(MUMU_inverted_SStoOS)
            MUMU_inverted_SStoOS.SetName("MUMU_inverted_SStoOS" + case)

            MUMU_OS_FAKE_DataDriven = GetOSFromSS_rebin(MUMU_SS_FAKE, MUMU_inverted_SStoOS)
            MUMU_OS_FAKE_DataDriven = SanityCheck(MUMU_OS_FAKE_DataDriven)
            MUMU_OS_FAKE_DataDriven.SetName("MUMU_OS_FAKE_DataDriven" + case)

            outputFile.cd(f"{era}/FAKE_MUMU_SStoOS")
            MUMU_inverted_SStoOS.Write()

            outputFile.cd(f"{era}/FAKE_MUMU_SS")
            MUMU_SS_FAKE.Write()

            outputFile.cd(f"{era}/FAKE_MUMU_OS")
            MUMU_OS_FAKE_DataDriven.Write()

            outputFile.cd(f"{era}/FAKE_MUMU_OS_Inv_rebin")
            MUMU_OS_inverted_FAKE_rebin.Write()
            
            outputFile.cd(f"{era}/FAKE_MUMU_SS_Inv_rebin")
            MUMU_SS_inverted_FAKE_rebin.Write()

            #################################################################
            # Fake SS
            #################################################################
            Canv_MUMU_SS_FAKE = CMS.cmsCanvas(
                "Canv_MUMU_SS_FAKE",
                200,
                4000,
                0,
                MUMU_SS_FAKE.GetBinContent(2) * 1.3,
                "M(#mu#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_MUMU_SS_FAKE.cd(1).SetLogx(True)
            Canv_MUMU_SS_FAKE.cd(2).SetLogx(True)

            MUMU_SS_FAKE_Stack = ROOT.THStack("MUMU_SS_FAKE_Stack", "Stacked")

            Leg_MUMU_SS_FAKE_Stack = CMS.cmsLeg(0.80, 0.6, 0.89, 0.7, textSize=0.03)

            MUMU_SS_FAKE_StackSheet = {
                "Fake": MUMU_SS_FAKE,
            }

            CMS.cmsDrawStack(MUMU_SS_FAKE_Stack, Leg_MUMU_SS_FAKE_Stack, MUMU_SS_FAKE_StackSheet)
            Canv_MUMU_SS_FAKE.cd(1).RedrawAxis()

            latex_MUMU_SS_FAKE_item = latex_copy_mumu.copy()
            latex_MUMU_SS_FAKE_item[0] = f"{latex_MUMU_SS_FAKE_item[0]}, Fake SS"

            Latex_MUMU_SS_FAKE = ROOT.TLatex()
            Latex_MUMU_SS_FAKE.SetTextAlign(14);
            Latex_MUMU_SS_FAKE.SetTextSize(0.04);
            Latex_MUMU_SS_FAKE.SetTextFont(42);
            for idx, addon in enumerate(latex_MUMU_SS_FAKE_item):
                Latex_MUMU_SS_FAKE.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_MUMU_SS_FAKE, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_FakeSS" + case + ".pdf"))

            #################################################################
            # Fake SS inverted
            #################################################################
            Canv_MUMU_SS_FAKE_inverted = CMS.cmsCanvas(
                "Canv_MUMU_SS_FAKE_inverted",
                200,
                4000,
                0,
                MUMU_SS_inverted_FAKE.GetBinContent(2) * 1.3,
                "M(#mu#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_MUMU_SS_FAKE_inverted.cd(1).SetLogx(True)
            Canv_MUMU_SS_FAKE_inverted.cd(2).SetLogx(True)

            MUMU_SS_FAKE_inverted_Stack = ROOT.THStack("MUMU_SS_FAKE_inverted_Stack", "Stacked")

            Leg_MUMU_SS_FAKE_inverted_Stack = CMS.cmsLeg(0.80, 0.6, 0.89, 0.7, textSize=0.03)

            MUMU_SS_FAKE_inverted_StackSheet = {
                "Fake": MUMU_SS_inverted_FAKE,
            }

            CMS.cmsDrawStack(MUMU_SS_FAKE_inverted_Stack, Leg_MUMU_SS_FAKE_inverted_Stack, MUMU_SS_FAKE_inverted_StackSheet)
            Canv_MUMU_SS_FAKE_inverted.cd(1).RedrawAxis()

            latex_MUMU_SS_FAKE_inverted_item = latex_copy_mumu.copy()
            latex_MUMU_SS_FAKE_inverted_item[0] = f"{latex_MUMU_SS_FAKE_inverted_item[0]}, SS, Fake CR"

            Latex_MUMU_SS_FAKE_inverted = ROOT.TLatex()
            Latex_MUMU_SS_FAKE_inverted.SetTextAlign(14);
            Latex_MUMU_SS_FAKE_inverted.SetTextSize(0.04);
            Latex_MUMU_SS_FAKE_inverted.SetTextFont(42);
            for idx, addon in enumerate(latex_MUMU_SS_FAKE_inverted_item):
                Latex_MUMU_SS_FAKE_inverted.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_MUMU_SS_FAKE_inverted, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_FakeSSinverted" + case + ".pdf"))


            #################################################################
            # Fake OS
            #################################################################
            Canv_MUMU_OS_FAKE = CMS.cmsCanvas(
                "Canv_MUMU_OS_FAKE",
                200,
                4000,
                0,
                MUMU_OS_FAKE_DataDriven.GetBinContent(2) * 1.3,
                "M(#mu#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_MUMU_OS_FAKE.cd(1).SetLogx(True)
            Canv_MUMU_OS_FAKE.cd(2).SetLogx(True)

            MUMU_OS_FAKE_Stack = ROOT.THStack("MUMU_OS_FAKE_Stack", "Stacked")

            Leg_MUMU_OS_FAKE_Stack = CMS.cmsLeg(0.80, 0.60, 0.89, 0.70, textSize=0.03)

            MUMU_OS_FAKE_StackSheet = {
                "Fake": MUMU_OS_FAKE_DataDriven,
            }

            CMS.cmsDrawStack(MUMU_OS_FAKE_Stack, Leg_MUMU_OS_FAKE_Stack, MUMU_OS_FAKE_StackSheet)
            Canv_MUMU_OS_FAKE.cd(1).RedrawAxis()

            latex_MUMU_OS_FAKE_item = latex_copy_mumu.copy()
            latex_MUMU_OS_FAKE_item[0] = f"{latex_MUMU_OS_FAKE_item[0]}, Fake OS data-driven"

            Latex_MUMU_OS_FAKE = ROOT.TLatex()
            Latex_MUMU_OS_FAKE.SetTextAlign(14);
            Latex_MUMU_OS_FAKE.SetTextSize(0.04);
            Latex_MUMU_OS_FAKE.SetTextFont(42);
            for idx, addon in enumerate(latex_MUMU_OS_FAKE_item):
                Latex_MUMU_OS_FAKE.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_MUMU_OS_FAKE, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_FakeOSDataDriven" + case + ".pdf"))

            #################################################################
            # Fake OS inverted
            #################################################################
            Canv_MUMU_OS_FAKE_inverted = CMS.cmsCanvas(
                "Canv_MUMU_OS_FAKE_inverted",
                200,
                4000,
                0,
                MUMU_OS_inverted_FAKE.GetBinContent(2) * 1.3,
                "M(#mu#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_MUMU_OS_FAKE_inverted.cd(1).SetLogx(True)
            Canv_MUMU_OS_FAKE_inverted.cd(2).SetLogx(True)

            MUMU_OS_FAKE_inverted_Stack = ROOT.THStack("MUMU_OS_FAKE_inverted_Stack", "Stacked")

            Leg_MUMU_OS_FAKE_inverted_Stack = CMS.cmsLeg(0.80, 0.6, 0.89, 0.7, textSize=0.03)

            MUMU_OS_FAKE_inverted_StackSheet = {
                "Fake": MUMU_OS_inverted_FAKE,
            }

            CMS.cmsDrawStack(MUMU_OS_FAKE_inverted_Stack, Leg_MUMU_OS_FAKE_inverted_Stack, MUMU_OS_FAKE_inverted_StackSheet)
            Canv_MUMU_OS_FAKE_inverted.cd(1).RedrawAxis()

            latex_MUMU_OS_FAKE_inverted_item = latex_copy_mumu.copy()
            latex_MUMU_OS_FAKE_inverted_item[0] = f"{latex_MUMU_OS_FAKE_inverted_item[0]}, OS, Fake CR"

            Latex_MUMU_OS_FAKE_inverted = ROOT.TLatex()
            Latex_MUMU_OS_FAKE_inverted.SetTextAlign(14);
            Latex_MUMU_OS_FAKE_inverted.SetTextSize(0.04);
            Latex_MUMU_OS_FAKE_inverted.SetTextFont(42);
            for idx, addon in enumerate(latex_MUMU_OS_FAKE_inverted_item):
                Latex_MUMU_OS_FAKE_inverted.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_MUMU_OS_FAKE_inverted, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_FakeOSinverted" + case + ".pdf"))




            #################################################################
            # Fake SStoOS
            #################################################################
            yrmin, yrmax = GetYRange(MUMU_inverted_SStoOS)
            
            Canv_SStoOS = CMS.cmsDiCanvas(
                "Canv_SStoOS",
                200,
                4000,
                2e-2,
                MUMU_OS_inverted_FAKE.GetBinContent(2) * 1e3,
                yrmin,
                yrmax,
                "M(#mu#mu) [GeV]",
                "Events",
                "OS/SS",
                square = CMS.kSquare,
                extraSpace = 0.1,
                iPos = 0,
            )


            Canv_SStoOS.cd(1).SetLogy(True)
            Canv_SStoOS.cd(1).SetLogx(True)
            Canv_SStoOS.cd(2).SetLogx(True)

            Canv_SStoOS.cd(1)

            Leg_SStoOS = CMS.cmsLeg(0.70, 0.70 - 0.05 * 2, 0.89, 0.70, textSize=0.03)
            Leg_SStoOS.AddEntry(MUMU_OS_inverted_FAKE, "Data - MC, OS", "lp")
            Leg_SStoOS.AddEntry(MUMU_SS_inverted_FAKE, "Data - MC, SS", "lp")

            Canv_SStoOS.cd(1)
            CMS.cmsDraw(MUMU_OS_inverted_FAKE, "P", mcolor=ROOT.kBlack)
            CMS.cmsDraw(MUMU_SS_inverted_FAKE, "P", mcolor=ROOT.kRed)


            latex_SStoOS_item = latex_copy_mumu.copy()
            latex_SStoOS_item[0] = f"{latex_SStoOS_item[0]}, OS/SS, One or Both muons with inverted ISO"

            Latex_SStoOS = ROOT.TLatex()
            Latex_SStoOS.SetTextAlign(14);
            Latex_SStoOS.SetTextSize(0.04);
            Latex_SStoOS.SetTextFont(42);
            for idx, addon in enumerate(latex_SStoOS_item):
                Latex_SStoOS.DrawLatexNDC(0.18, 0.86 - idx * 0.065, addon.encode('utf-8'))

            Canv_SStoOS.cd(2)
            CMS.cmsDraw(MUMU_inverted_SStoOS, "P", mcolor=ROOT.kBlack)

            ref_line = ROOT.TLine(200, 1, 4000, 1)
            CMS.cmsDrawLine(ref_line, lcolor=ROOT.kRed, lstyle=ROOT.kDotted)

            CMS.SaveCanvas(Canv_SStoOS, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_SStoOS" + case + ".pdf"))




    outputFile.Close()


if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    ROOT.TH1.SetDefaultSumw2()
    
    main()
