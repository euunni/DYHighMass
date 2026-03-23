#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import plotterEngine

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
        if (hist.GetBinCenter(i) > 200 and hist.GetBinCenter(i) < 4000):
            if (hist.GetBinContent(i) != 0):
                yrmin = min(yrmin, hist.GetBinContent(i))
                yrmax = max(yrmax, hist.GetBinContent(i))

    yrmin = yrmin * 0.9
    yrmax = yrmax * 1.1

    return yrmin, yrmax

addon_hook_jet = {
    "": "",
    "_0J": "N(jet) = 0",
    "_1J": "N(jet) = 1",
    "_mt1J": "N(jet) > 1",
    "_0BJ": "b-veto",
    "_1BJ": "N(b-jet) = 1",
    "_mt1BJ": "N(b-jet) > 1",
    "_bVeto_0J": "b-veto, N(jet) = 0",
    "_bVeto_1J": "b-veto, N(jet) = 1",
    "_bVeto_mt1J": "b-veto, N(jet) > 1",
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

refLumi = {
    "2016_preVFP": 19.5,
    "2016_postVFP": 16.8,
    "2017": 41.5,
    "2018": 59.8,
    "merged": 137.6
}

def GetOSFromSS(SS_Fake, SStoOS):

    FAKE_DataDriven = SS_Fake.Clone(f"FAKE_DataDriven_{uuid.uuid4()}")
    FAKE_DataDriven.Reset("ICES");

    for i in range(1, SStoOS.GetNbinsX() + 1):
        if SStoOS.GetBinContent(i) == 0 or SS_Fake.GetBinContent(i) <= 0:
            FAKE_DataDriven.SetBinContent(i, 0)
        else:
            FAKE_DataDriven.SetBinContent(i, SS_Fake.GetBinContent(i) * SStoOS.GetBinContent(i))

    return FAKE_DataDriven


def main():

    eras = ["2016_preVFP", "2016_postVFP", "2017", "2018", "merged"]
    cases = ["", "_0BJ", "_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"]
    
    histoName_MUMU = "h_dimuonMass"
    outputPath = "./Bck/MUMU_FAKE/"
    outputRoot = "./Bck/MUMU_FAKE.root"

    os.makedirs(outputPath, exist_ok=True)

    outputFile = ROOT.TFile(outputRoot, "RECREATE")

    for era in eras:

        CMS.SetLumi(refLumi[era])
        os.makedirs(f"{outputPath}era_{era}", exist_ok=True)

        latex_mumu = [
            f"{era}",
            "p_{T}(#mu) > 52 (15) GeV, |#eta(#mu)| < 2.4",
            "M_{#mu#mu} > 200 GeV",
            ""
        ]

        if era == "merged":
            latex_mumu[0] = "Run2"

        outputFile.cd()
        outputFile.mkdir(f"{era}/FAKE_MUMU_OS")
        outputFile.mkdir(f"{era}/FAKE_MUMU_SS")
        outputFile.mkdir(f"{era}/FAKE_MUMU_SStoOS")

        MUMU_OS = plotterEngine.Plotter(era, rootPath = "./Bck/ROOT/MUMU_OS.root", 
                                            outputPath = "./plots/temp/plots" + era + "/",
                                            channel = "MUMU", 
                                            region = "OS")

        MUMU_SS = plotterEngine.Plotter(era, rootPath = "./Bck/ROOT/MUMU_SS.root", 
                                            outputPath = "./plots/temp/plots" + era + "/",
                                            channel = "MUMU", 
                                            region = "SS")

        MUMU_OS_inverted = plotterEngine.Plotter(era, rootPath = "./Bck/ROOT/MUMU_OS_inverted.root", 
                                            outputPath = "./plots/temp/plots" + era + "/",
                                            channel = "MUMU", 
                                            region = "OS_inverted")

        MUMU_SS_inverted = plotterEngine.Plotter(era, rootPath = "./Bck/ROOT/MUMU_SS_inverted.root", 
                                            outputPath = "./plots/temp/plots" + era + "/",
                                            channel = "MUMU", 
                                            region = "SS_inverted")

        for case in cases:

            latex_copy_mumu = latex_mumu.copy()

            if case != "":
                latex_copy_mumu[2] = f"{latex_copy_mumu[2]}, {addon_hook_jet[case]}"

            histoName = "h_dimuonMass"
            if case != "":
                histoName = case + "/" + histoName + case
            if case == "":
                histoName = histoName

            MUMU_SS_TotalMC = MUMU_SS.GetMCHist(histoName, mcList)
            MUMU_SS_data = MUMU_SS.GetDataHist(histoName)

            MUMU_SS_FAKE = MUMU_SS_data.Clone(f"MUMU_SS_FAKE_{uuid.uuid4()}")
            MUMU_SS_FAKE.Add(MUMU_SS_TotalMC, -1)
            MUMU_SS_FAKE = SanityCheck(MUMU_SS_FAKE)
            MUMU_SS_FAKE.SetName("MUMU_SS_FAKE" + case)

            MUMU_OS_inverted_TotalMC = MUMU_OS_inverted.GetMCHist(histoName, mcList)
            MUMU_OS_inverted_data = MUMU_OS_inverted.GetDataHist(histoName)
            
            MUMU_OS_inverted_FAKE = MUMU_OS_inverted_data.Clone(f"MUMU_OS_inverted_FAKE_{uuid.uuid4()}")
            MUMU_OS_inverted_FAKE.Add(MUMU_OS_inverted_TotalMC, -1)
            MUMU_OS_inverted_FAKE = SanityCheck(MUMU_OS_inverted_FAKE)

            MUMU_SS_inverted_TotalMC = MUMU_SS_inverted.GetMCHist(histoName, mcList)
            MUMU_SS_inverted_data = MUMU_SS_inverted.GetDataHist(histoName)

            MUMU_SS_inverted_FAKE = MUMU_SS_inverted_data.Clone(f"MUMU_SS_inverted_FAKE_{uuid.uuid4()}")
            MUMU_SS_inverted_FAKE.Add(MUMU_SS_inverted_TotalMC, -1)
            MUMU_SS_inverted_FAKE = SanityCheck(MUMU_SS_inverted_FAKE)

            MUMU_inverted_SStoOS = MUMU_OS_inverted_FAKE.Clone(f"MUMU_inverted_SStoOS_{uuid.uuid4()}")
            MUMU_inverted_SStoOS.Divide(MUMU_SS_inverted_FAKE)
            MUMU_inverted_SStoOS.SetName("MUMU_inverted_SStoOS" + case)

            MUMU_OS_FAKE_DataDriven = GetOSFromSS(MUMU_SS_FAKE, MUMU_inverted_SStoOS)
            MUMU_OS_FAKE_DataDriven.SetName("MUMU_OS_FAKE_DataDriven" + case)

            outputFile.cd(f"{era}/FAKE_MUMU_SStoOS")
            MUMU_inverted_SStoOS.Write()
            outputFile.cd(f"{era}/FAKE_MUMU_SS")
            MUMU_SS_FAKE.Write()
            outputFile.cd(f"{era}/FAKE_MUMU_OS")
            MUMU_OS_FAKE_DataDriven.Write()


            #################################################################
            # Fake SS
            #################################################################
            Canv_MUMU_SS_FAKE = CMS.cmsCanvas(
                "Canv_MUMU_SS_FAKE",
                200,
                4000,
                0,
                MUMU_SS_FAKE.GetMaximum() * 1.3,
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
                MUMU_SS_inverted_FAKE.GetMaximum() * 1.3,
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
                MUMU_OS_FAKE_DataDriven.GetMaximum() * 1.3,
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
                MUMU_OS_inverted_FAKE.GetMaximum() * 1.3,
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
                MUMU_OS_inverted_FAKE.GetMaximum() * 1e3,
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
            latex_SStoOS_item[0] = f"{latex_SStoOS_item[0]}, OS/SS, only one muon with inverted ISO"

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
    main()
