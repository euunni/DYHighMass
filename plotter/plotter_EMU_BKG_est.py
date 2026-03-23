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

mumu_mcList = [
    "NNLO_MUMU_10to50",
    "NNLO_MUMU_inc",
    "NNLO_MUMU_100to200",
    "NNLO_MUMU_200to400",
    "NNLO_MUMU_400to500",
    "NNLO_MUMU_500to700",
    "NNLO_MUMU_700to800",
    "NNLO_MUMU_800to1000",
    "NNLO_MUMU_1000to1500",
    "NNLO_MUMU_1500to2000",
    "NNLO_MUMU_2000toInf",

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

emu_mcList = [
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

def GetOSFromSS(EMU_SS_FAKE, SStoOS):

    EMU_OS_FAKE_DataDriven = EMU_SS_FAKE.Clone(f"EMU_OS_FAKE_DataDriven_{uuid.uuid4()}")
    EMU_OS_FAKE_DataDriven.Reset("ICES");

    for i in range(1, SStoOS.GetNbinsX() + 1):
        if SStoOS.GetBinContent(i) == 0 or EMU_SS_FAKE.GetBinContent(i) <= 0:
            EMU_OS_FAKE_DataDriven.SetBinContent(i, 0)
        else:
            EMU_OS_FAKE_DataDriven.SetBinContent(i, EMU_SS_FAKE.GetBinContent(i) * SStoOS.GetBinContent(i))

    return EMU_OS_FAKE_DataDriven


def main():

    eras = ["2016_preVFP", "2016_postVFP", "2017", "2018", "merged"]
    cases = ["", "_0BJ", "_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"]
    

    histoName = "h_PairMass"
    histoName_MUMU = "h_dimuonMass"
    outputPath = "./Bck/EMU_FAKE/"
    outputRoot = "./Bck/EMU_FAKE.root"

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

        latex_emu = [
            f"{era}",
            "p_{T}(#mu(e)) > 52 (20) GeV, |#eta(#mu(e))| < 2.4 (2.5)",
            "M_{e#mu} > 200 GeV",
            ""
        ]

        if era == "merged":
            latex_mumu[0] = "Run2"
            latex_emu[0] = "Run2"

        outputFile.cd()
        outputFile.mkdir(f"{era}/FAKE_EMU_OS")
        outputFile.mkdir(f"{era}/FAKE_EMU_SS")
        outputFile.mkdir(f"{era}/FAKE_EMU_SStoOS")
        outputFile.mkdir(f"{era}/EMU_OS_TOP")
        outputFile.mkdir(f"{era}/TOP_EMUtoMUMU")
        outputFile.mkdir(f"{era}/TOP_MUMU")

        EMU_OS = plotterEngine.Plotter(era, rootPath = "./Bck/ROOT/EMU_OS.root", 
                                        outputPath = "./plots/temp/plots" + era + "/",
                                        channel = "EMU", 
                                        region = "OS")

        EMU_SS = plotterEngine.Plotter(era, rootPath = "./Bck/ROOT/EMU_SS.root", 
                                        outputPath = "./plots/temp/plots" + era + "/",
                                        channel = "EMU", 
                                        region = "SS")

        EMU_OS_inverted = plotterEngine.Plotter(era, rootPath = "./Bck/ROOT/EMU_OS_inverted.root", 
                                        outputPath = "./plots/temp/plots" + era + "/",
                                        channel = "EMU", 
                                        region = "OS_inverted")

        EMU_SS_inverted = plotterEngine.Plotter(era, rootPath = "./Bck/ROOT/EMU_SS_inverted.root", 
                                        outputPath = "./plots/temp/plots" + era + "/",
                                        channel = "EMU", 
                                        region = "SS_inverted")

        MUMU = plotterEngine.Plotter(era, rootPath = "./Bck/ROOT/MUMU_OS.root", 
                                        outputPath = "./plots/temp/plots" + era + "/",
                                        channel = "MUMU", 
                                        region = "OS")

        for case in cases:

            latex_copy_mumu = latex_mumu.copy()
            latex_copy_emu = latex_emu.copy()

            if case != "":
                latex_copy_mumu[2] = f"{latex_copy_mumu[2]}, {addon_hook_jet[case]}"
                latex_copy_emu[2] = f"{latex_copy_emu[2]}, {addon_hook_jet[case]}"

            histoName = "h_PairMass"
            histoName_MUMU = "h_dimuonMass"
            if case != "":
                histoName = case + "/" + histoName + case
                histoName_MUMU = case + "/" + histoName_MUMU + case
            if case == "":
                histoName = histoName
                histoName_MUMU = histoName_MUMU

            EMU_OS_TotalMC = EMU_OS.GetMCHist(histoName, emu_mcList)
            EMU_OS_data = EMU_OS.GetDataHist(histoName)

            EMU_OS_TT = EMU_OS.GetMCHist(histoName, ["TTTo2L2Nu"])
            EMU_OS_TT.SetName("EMU_OS_TT" + case)
            EMU_OS_TW = EMU_OS.GetMCHist(histoName, ["ST_tW_AntiTop", "ST_tW_Top"])
            EMU_OS_TW.SetName("EMU_OS_TW" + case)
            EMU_OS_WW = EMU_OS.GetMCHist(histoName, ["WW"])
            EMU_OS_WW.SetName("EMU_OS_WW" + case)

            EMU_OS_TOP = EMU_OS_TT.Clone("EMU_OS_TOP" + case)
            EMU_OS_TOP.Add(EMU_OS_TW)
            EMU_OS_TOP.Add(EMU_OS_WW)

            MUMU_OS_TT = MUMU.GetMCHist(histoName_MUMU, ["TTTo2L2Nu"])
            MUMU_OS_TT.SetName("MUMU_OS_TT" + case)
            MUMU_OS_TW = MUMU.GetMCHist(histoName_MUMU, ["ST_tW_AntiTop", "ST_tW_Top"])
            MUMU_OS_TW.SetName("MUMU_OS_TW" + case)
            MUMU_OS_WW = MUMU.GetMCHist(histoName_MUMU, ["WW"])
            MUMU_OS_WW.SetName("MUMU_OS_WW" + case)

            MUMU_OS_TOP = MUMU_OS_TT.Clone("MUMU_OS_TOP" + case)
            MUMU_OS_TOP.Add(MUMU_OS_TW)
            MUMU_OS_TOP.Add(MUMU_OS_WW)

            TOP_EMUtoMUMU = MUMU_OS_TOP.Clone(f"TOP_EMUtoMUMU_{uuid.uuid4()}")
            TOP_EMUtoMUMU.Divide(EMU_OS_TOP)
            TOP_EMUtoMUMU.SetName("TOP_EMUtoMUMU" + case)

            EMU_SS_TotalMC = EMU_SS.GetMCHist(histoName, emu_mcList)
            EMU_SS_data = EMU_SS.GetDataHist(histoName)
            
            EMU_SS_FAKE = EMU_SS_data.Clone(f"EMU_SS_FAKE_{uuid.uuid4()}")
            EMU_SS_FAKE.Add(EMU_SS_TotalMC, -1)
            EMU_SS_FAKE.SetName("EMU_SS_FAKE" + case)
            EMU_SS_FAKE = SanityCheck(EMU_SS_FAKE)

            EMU_OS_inverted_TotalMC = EMU_OS_inverted.GetMCHist(histoName, emu_mcList)
            EMU_OS_inverted_data = EMU_OS_inverted.GetDataHist(histoName)
            
            EMU_OS_inverted_FAKE = EMU_OS_inverted_data.Clone(f"EMU_OS_inverted_FAKE_{uuid.uuid4()}")
            EMU_OS_inverted_FAKE.Add(EMU_OS_inverted_TotalMC, -1)
            EMU_OS_inverted_FAKE = SanityCheck(EMU_OS_inverted_FAKE)

            EMU_SS_inverted_TotalMC = EMU_SS_inverted.GetMCHist(histoName, emu_mcList)
            EMU_SS_inverted_data = EMU_SS_inverted.GetDataHist(histoName)

            EMU_SS_inverted_FAKE = EMU_SS_inverted_data.Clone(f"EMU_SS_inverted_FAKE_{uuid.uuid4()}")
            EMU_SS_inverted_FAKE.Add(EMU_SS_inverted_TotalMC, -1)
            EMU_SS_inverted_FAKE = SanityCheck(EMU_SS_inverted_FAKE)

            EMU_inverted_SStoOS = EMU_OS_inverted_FAKE.Clone(f"EMU_inverted_SStoOS_{uuid.uuid4()}")
            EMU_inverted_SStoOS.Divide(EMU_SS_inverted_FAKE)
            EMU_inverted_SStoOS.SetName("EMU_inverted_SStoOS" + case)

            EMU_OS_FAKE_DataDriven = GetOSFromSS(EMU_SS_FAKE, EMU_inverted_SStoOS)
            EMU_OS_FAKE_DataDriven.SetName("EMU_OS_FAKE_DataDriven" + case)

            EMU_OS_TotalMC_ex_TOP = EMU_OS.GetMCHist(histoName, emu_mcList_woTop)
            EMU_OS_TotalMC_ex_TOP.Add(EMU_OS_FAKE_DataDriven)

            EMU_OS_TOP_DataDriven = EMU_OS_data.Clone(f"EMU_OS_TOP_DataDriven_{uuid.uuid4()}")
            EMU_OS_TOP_DataDriven.Add(EMU_OS_TotalMC_ex_TOP, -1)
            EMU_OS_TOP_DataDriven.SetName("EMU_OS_TOP_DataDriven" + case)

            MUMU_OS_TOP_DataDriven = GetOSFromSS(EMU_OS_TOP_DataDriven, TOP_EMUtoMUMU)
            MUMU_OS_TOP_DataDriven.SetName("MUMU_OS_TOP_DataDriven" + case)
            MUMU_OS_TOP_DataDriven_Ratio = MUMU_OS_TOP_DataDriven.Clone(f"MUMU_OS_TOP_DataDriven_Ratio_{uuid.uuid4()}")
            MUMU_OS_TOP_DataDriven_Ratio.Divide(MUMU_OS_TOP)

            outputFile.cd(f"{era}/FAKE_EMU_SStoOS")
            EMU_inverted_SStoOS.Write()
            outputFile.cd(f"{era}/FAKE_EMU_SS")
            EMU_SS_FAKE.Write()
            outputFile.cd(f"{era}/FAKE_EMU_OS")
            EMU_OS_FAKE_DataDriven.Write()
            outputFile.cd(f"{era}/TOP_MUMU")
            MUMU_OS_TOP_DataDriven.Write()
            outputFile.cd(f"{era}/TOP_EMUtoMUMU")
            TOP_EMUtoMUMU.Write()


            #################################################################
            # Fake SS
            #################################################################
            Canv_EMU_SS_FAKE = CMS.cmsCanvas(
                "Canv_EMU_SS_FAKE",
                200,
                4000,
                0,
                EMU_SS_FAKE.GetMaximum() * 1.3,
                "M(e#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EMU_SS_FAKE.cd(1).SetLogx(True)
            Canv_EMU_SS_FAKE.cd(2).SetLogx(True)

            EMU_SS_FAKE_Stack = ROOT.THStack("EMU_SS_FAKE_Stack", "Stacked")

            Leg_EMU_SS_FAKE_Stack = CMS.cmsLeg(0.80, 0.6, 0.89, 0.7, textSize=0.03)

            EMU_SS_FAKE_StackSheet = {
                "Fake": EMU_SS_FAKE,
            }

            CMS.cmsDrawStack(EMU_SS_FAKE_Stack, Leg_EMU_SS_FAKE_Stack, EMU_SS_FAKE_StackSheet)
            Canv_EMU_SS_FAKE.cd(1).RedrawAxis()

            latex_EMU_SS_FAKE_item = latex_copy_emu.copy()
            latex_EMU_SS_FAKE_item[0] = f"{latex_EMU_SS_FAKE_item[0]}, Fake SS"

            Latex_EMU_SS_FAKE = ROOT.TLatex()
            Latex_EMU_SS_FAKE.SetTextAlign(14);
            Latex_EMU_SS_FAKE.SetTextSize(0.04);
            Latex_EMU_SS_FAKE.SetTextFont(42);
            for idx, addon in enumerate(latex_EMU_SS_FAKE_item):
                Latex_EMU_SS_FAKE.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_EMU_SS_FAKE, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_FakeSS" + case + ".pdf"))


            #################################################################
            # Fake SS inverted
            #################################################################
            Canv_EMU_SS_FAKE_inverted = CMS.cmsCanvas(
                "Canv_EMU_SS_FAKE_inverted",
                200,
                4000,
                0,
                EMU_SS_inverted_FAKE.GetMaximum() * 1.3,
                "M(e#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EMU_SS_FAKE_inverted.cd(1).SetLogx(True)
            Canv_EMU_SS_FAKE_inverted.cd(2).SetLogx(True)

            EMU_SS_FAKE_inverted_Stack = ROOT.THStack("EMU_SS_FAKE_inverted_Stack", "Stacked")

            Leg_EMU_SS_FAKE_inverted_Stack = CMS.cmsLeg(0.80, 0.6, 0.89, 0.7, textSize=0.03)

            EMU_SS_FAKE_inverted_StackSheet = {
                "Fake": EMU_SS_inverted_FAKE,
            }

            CMS.cmsDrawStack(EMU_SS_FAKE_inverted_Stack, Leg_EMU_SS_FAKE_inverted_Stack, EMU_SS_FAKE_inverted_StackSheet)
            Canv_EMU_SS_FAKE_inverted.cd(1).RedrawAxis()

            latex_EMU_SS_FAKE_inverted_item = latex_copy_emu.copy()
            latex_EMU_SS_FAKE_item[0] = f"{latex_EMU_SS_FAKE_item[0]}, SS, Fake CR"

            Latex_EMU_SS_FAKE_inverted = ROOT.TLatex()
            Latex_EMU_SS_FAKE_inverted.SetTextAlign(14);
            Latex_EMU_SS_FAKE_inverted.SetTextSize(0.04);
            Latex_EMU_SS_FAKE_inverted.SetTextFont(42);
            for idx, addon in enumerate(latex_EMU_SS_FAKE_inverted_item):
                Latex_EMU_SS_FAKE_inverted.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_EMU_SS_FAKE_inverted, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_FakeSSinverted" + case + ".pdf"))


            #################################################################
            # Fake OS
            #################################################################
            Canv_EMU_OS_FAKE = CMS.cmsCanvas(
                "Canv_EMU_OS_FAKE",
                200,
                4000,
                0,
                EMU_OS_FAKE_DataDriven.GetMaximum() * 1.3,
                "M(e#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EMU_OS_FAKE.cd(1).SetLogx(True)
            Canv_EMU_OS_FAKE.cd(2).SetLogx(True)

            EMU_OS_FAKE_Stack = ROOT.THStack("EMU_OS_FAKE_Stack", "Stacked")

            Leg_EMU_OS_FAKE_Stack = CMS.cmsLeg(0.80, 0.60, 0.89, 0.70, textSize=0.03)

            EMU_OS_FAKE_StackSheet = {
                "Fake": EMU_OS_FAKE_DataDriven,
            }

            CMS.cmsDrawStack(EMU_OS_FAKE_Stack, Leg_EMU_OS_FAKE_Stack, EMU_OS_FAKE_StackSheet)
            Canv_EMU_OS_FAKE.cd(1).RedrawAxis()

            latex_EMU_OS_FAKE_item = latex_copy_emu.copy()
            latex_EMU_OS_FAKE_item[0] = f"{latex_EMU_OS_FAKE_item[0]}, Fake OS data-driven"

            Latex_EMU_OS_FAKE = ROOT.TLatex()
            Latex_EMU_OS_FAKE.SetTextAlign(14);
            Latex_EMU_OS_FAKE.SetTextSize(0.04);
            Latex_EMU_OS_FAKE.SetTextFont(42);
            for idx, addon in enumerate(latex_EMU_OS_FAKE_item):
                Latex_EMU_OS_FAKE.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_EMU_OS_FAKE, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_FakeOSDataDriven" + case + ".pdf"))

            #################################################################
            # Fake OS inverted
            #################################################################
            Canv_EMU_OS_FAKE_inverted = CMS.cmsCanvas(
                "Canv_EMU_OS_FAKE_inverted",
                200,
                4000,
                0,
                EMU_OS_inverted_FAKE.GetMaximum() * 1.3,
                "M(e#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EMU_OS_FAKE_inverted.cd(1).SetLogx(True)
            Canv_EMU_OS_FAKE_inverted.cd(2).SetLogx(True)

            EMU_OS_FAKE_inverted_Stack = ROOT.THStack("EMU_OS_FAKE_inverted_Stack", "Stacked")

            Leg_EMU_OS_FAKE_inverted_Stack = CMS.cmsLeg(0.80, 0.60, 0.89, 0.70, textSize=0.03)

            EMU_OS_FAKE_inverted_StackSheet = {
                "Fake": EMU_OS_inverted_FAKE,
            }

            CMS.cmsDrawStack(EMU_OS_FAKE_inverted_Stack, Leg_EMU_OS_FAKE_inverted_Stack, EMU_OS_FAKE_inverted_StackSheet)
            Canv_EMU_OS_FAKE_inverted.cd(1).RedrawAxis()

            latex_EMU_OS_FAKE_inverted_item = latex_copy_emu.copy()
            latex_EMU_OS_FAKE_inverted_item[0] = f"{latex_EMU_OS_FAKE_inverted_item[0]}, OS, Fake CR"

            Latex_EMU_OS_FAKE_inverted = ROOT.TLatex()
            Latex_EMU_OS_FAKE_inverted.SetTextAlign(14);
            Latex_EMU_OS_FAKE_inverted.SetTextSize(0.04);
            Latex_EMU_OS_FAKE_inverted.SetTextFont(42);
            for idx, addon in enumerate(latex_EMU_OS_FAKE_inverted_item):
                Latex_EMU_OS_FAKE_inverted.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_EMU_OS_FAKE_inverted, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_FakeOSinverted" + case + ".pdf"))


            #################################################################
            # Fake SStoOS
            #################################################################
            
            yrmin, yrmax = GetYRange(EMU_inverted_SStoOS)
            Canv_SStoOS = CMS.cmsDiCanvas(
                "Canv_SStoOS",
                200,
                4000,
                2e-2,
                EMU_OS_inverted_FAKE.GetMaximum() * 1e3,
                yrmin,
                yrmax,
                "M(e#mu) [GeV]",
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
            Leg_SStoOS.AddEntry(EMU_OS_inverted_FAKE, "Data - MC, OS", "lp")
            Leg_SStoOS.AddEntry(EMU_SS_inverted_FAKE, "Data - MC, SS", "lp")

            Canv_SStoOS.cd(1)
            CMS.cmsDraw(EMU_OS_inverted_FAKE, "P", mcolor=ROOT.kBlack)
            CMS.cmsDraw(EMU_SS_inverted_FAKE, "P", mcolor=ROOT.kRed)


            latex_SStoOS_item = latex_copy_emu.copy()
            latex_SStoOS_item[0] = f"{latex_SStoOS_item[0]}, OS/SS, inverted ID and ISO"

            Latex_SStoOS = ROOT.TLatex()
            Latex_SStoOS.SetTextAlign(14);
            Latex_SStoOS.SetTextSize(0.04);
            Latex_SStoOS.SetTextFont(42);
            for idx, addon in enumerate(latex_SStoOS_item):
                Latex_SStoOS.DrawLatexNDC(0.18, 0.86 - idx * 0.065, addon.encode('utf-8'))

            Canv_SStoOS.cd(2)
            CMS.cmsDraw(EMU_inverted_SStoOS, "P", mcolor=ROOT.kBlack)

            ref_line = ROOT.TLine(200, 1, 4000, 1)
            CMS.cmsDrawLine(ref_line, lcolor=ROOT.kRed, lstyle=ROOT.kDotted)

            CMS.SaveCanvas(Canv_SStoOS, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_SStoOS" + case + ".pdf"))


            #################################################################
            # TOP EMUtoMUMU
            #################################################################
            Canv_TOP_EMUtoMUMU = CMS.cmsDiCanvas(
                "Canv_TOP_EMUtoMUMU",
                200,
                4000,
                2e-2,
                MUMU_OS_TOP.GetMaximum() * 1e3,
                1 - 0.55,
                1 + 0.55,
                "Mass [GeV]",
                "Events",
                "#mu#mu/e#mu",
                square = CMS.kSquare,
                extraSpace = 0.1,
                iPos = 0,
            )

            Canv_TOP_EMUtoMUMU.cd(1).SetLogy(True)
            Canv_TOP_EMUtoMUMU.cd(1).SetLogx(True)
            Canv_TOP_EMUtoMUMU.cd(2).SetLogx(True)

            Canv_TOP_EMUtoMUMU.cd(1)

            Leg_TOP_EMUtoMUMU = CMS.cmsLeg(0.50, 0.70 - 0.05 * 2, 0.89, 0.70, textSize=0.03)
            Leg_TOP_EMUtoMUMU.AddEntry(MUMU_OS_TOP, "#mu#mu, tt + tW + WW, MC", "lp")
            Leg_TOP_EMUtoMUMU.AddEntry(EMU_OS_TOP, "e#mu, tt + tW + WW, MC", "lp")

            Canv_TOP_EMUtoMUMU.cd(1)
            CMS.cmsDraw(MUMU_OS_TOP, "P", mcolor=ROOT.kBlack)
            CMS.cmsDraw(EMU_OS_TOP, "P", mcolor=ROOT.kRed)


            latex_TOP_EMUtoMUMU_item = latex_copy_emu.copy()
            latex_TOP_EMUtoMUMU_item[0] = f"{latex_TOP_EMUtoMUMU_item[0]}, #mu#mu/e#mu"

            Latex_TOP_EMUtoMUMU = ROOT.TLatex()
            Latex_TOP_EMUtoMUMU.SetTextAlign(14);
            Latex_TOP_EMUtoMUMU.SetTextSize(0.04);
            Latex_TOP_EMUtoMUMU.SetTextFont(42);
            for idx, addon in enumerate(latex_TOP_EMUtoMUMU_item):
                Latex_TOP_EMUtoMUMU.DrawLatexNDC(0.18, 0.86 - idx * 0.065, addon.encode('utf-8'))

            Canv_TOP_EMUtoMUMU.cd(2)
            CMS.cmsDraw(TOP_EMUtoMUMU, "P", mcolor=ROOT.kBlack)

            ref_line = ROOT.TLine(200, 1, 4000, 1)
            CMS.cmsDrawLine(ref_line, lcolor=ROOT.kRed, lstyle=ROOT.kDotted)

            CMS.SaveCanvas(Canv_TOP_EMUtoMUMU, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_TOP_EMUtoMUMU" + case + ".pdf"))

            #################################################################
            # TOP Comparison
            #################################################################
            Canv_TOP_Comparision = CMS.cmsDiCanvas(
                "Canv_TOP_Comparision",
                200,
                4000,
                2e-2,
                MUMU_OS_TOP_DataDriven.GetMaximum() * 1e3,
                1 - 0.55,
                1 + 0.55,
                "M(#mu#mu) [GeV]",
                "Events",
                "Data/Pred.",
                square = CMS.kSquare,
                extraSpace = 0.1,
                iPos = 0,
            )

            Canv_TOP_Comparision.cd(1).SetLogy(True)
            Canv_TOP_Comparision.cd(1).SetLogx(True)
            Canv_TOP_Comparision.cd(2).SetLogx(True)

            Canv_TOP_Comparision.cd(1)

            Leg_TOP_Comparision = CMS.cmsLeg(0.56, 0.89 - 0.05 * 4, 0.89, 0.89, textSize=0.03)
            Leg_TOP_Comparision.AddEntry(MUMU_OS_TOP_DataDriven, "TTbar + Single Top, data-diven", "lp")

            TOP_Comparision_Stack = ROOT.THStack("TOP_Comparision_Stack", "Stacked")
            TOP_Comparision_StackSheet = {
                "tW, MC": MUMU_OS_TW,
                "WW, MC": MUMU_OS_WW,
                "tt, MC": MUMU_OS_TT,
            }

            CMS.cmsDrawStack(TOP_Comparision_Stack, Leg_TOP_Comparision, TOP_Comparision_StackSheet)
            
            Canv_TOP_Comparision.cd(1)
            CMS.cmsDraw(MUMU_OS_TOP_DataDriven, "PE", mcolor=ROOT.kBlack)
            Canv_TOP_Comparision.cd(1).RedrawAxis()

            latex_TOP_Comparision_item = latex_copy_mumu.copy()
            latex_TOP_Comparision_item[0] = f"{latex_TOP_Comparision_item[0]}, tt + tW + WW"
        
            Latex_TOP_Comparision = ROOT.TLatex()
            Latex_TOP_Comparision.SetTextAlign(14);
            Latex_TOP_Comparision.SetTextSize(0.04);
            Latex_TOP_Comparision.SetTextFont(42);
            for idx, addon in enumerate(latex_TOP_Comparision_item):
                Latex_TOP_Comparision.DrawLatexNDC(0.18, 0.86 - idx * 0.065, addon.encode('utf-8'))

            Canv_TOP_Comparision.cd(2)
            CMS.cmsDraw(MUMU_OS_TOP_DataDriven_Ratio, "P", mcolor=ROOT.kBlack)

            ref_line = ROOT.TLine(200, 1, 4000, 1)
            CMS.cmsDrawLine(ref_line, lcolor=ROOT.kRed, lstyle=ROOT.kDotted)

            CMS.SaveCanvas(Canv_TOP_Comparision, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_TOP_DataDriven" + case + ".pdf"))



    outputFile.Close()


if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    main()
