#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import plotterEngine_EE as plotterEngine

CMS.SetExtraText("Private work")
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

    if yrmin > yrmax:
        return 0.0, 1.5

    span = max(yrmax - yrmin, 0.0)
    pad_bottom = max(0.1, 0.15 * span)
    orig_min = yrmin
    yrmin = yrmin - pad_bottom
    if orig_min >= 0:
        yrmin = max(0.0, yrmin)
    yrmax = yrmax * 1.1

    return yrmin, yrmax

LEGEND_X1 = 0.50
LEGEND_X2 = 0.89
LEGEND_Y2 = 0.70
LEGEND_ENTRY_HEIGHT = 0.05
LEGEND_TEXT_SIZE = 0.04

def MakeLegend(n_entries):
    return CMS.cmsLeg(
        LEGEND_X1,
        LEGEND_Y2 - LEGEND_ENTRY_HEIGHT * n_entries,
        LEGEND_X2,
        LEGEND_Y2,
        textSize=LEGEND_TEXT_SIZE,
    )

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

ee_mcList = [
    "NNLO_EE_10to50",
    "NNLO_EE_inc",
    "NNLO_EE_100to200",
    "NNLO_EE_200to400",
    "NNLO_EE_400to500",
    "NNLO_EE_500to700",
    "NNLO_EE_700to800",
    "NNLO_EE_800to1000",
    "NNLO_EE_1000to1500",
    "NNLO_EE_1500to2000",
    "NNLO_EE_2000toInf",

    "NNLO_tautau",

    "ST_tW_AntiTop",
    "ST_tW_Top",

    "TTTo2L2Nu",
    
    "WW",
    "WZ",
    "ZZ",

    "GGToEE_10to30_ElEl",
    "GGToEE_30to50_ElEl",
    "GGToEE_50to200_ElEl",
    "GGToEE_200to1500_ElEl",
    "GGToEE_1500toInf_ElEl",

    "GGToEE_10to30_InelElElInel",
    "GGToEE_30to50_InelElElInel",
    "GGToEE_50to200_InelElElInel",
    "GGToEE_200to1500_InelElElInel",
    "GGToEE_1500toInf_InelElElInel",

    "GGToEE_10to30_InelInel",
    "GGToEE_30to50_InelInel",
    "GGToEE_50to200_InelInel",
    "GGToEE_200to1500_InelInel",
    "GGToEE_1500toInf_InelInel",
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
    "2017": 42.12,
    "2018": 59.45,
    "merged": 137.88
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


def GetHistoName(name, type, jet):
    
    histoName = ""
    if jet != "":
        histoName = f"{jet}/h_{type}_{name}{jet}"
    if jet == "":
        histoName = f"h_{type}_{name}"

    return histoName


def GetHistoNameEE(name, jet):
    
    histoName = ""
    if jet != "":
        histoName = f"{jet}/h_{name}{jet}"
    if jet == "":
        histoName = f"h_{name}"

    return histoName


def GetEMUComponentHists(plotter, hist_name, total_mc):
    top = plotter.GetMCHist(
        hist_name,
        ["TTTo2L2Nu", "ST_tW_AntiTop", "ST_tW_Top", "WW"]
    )
    dy_tau = plotter.GetMCHist(hist_name, ["NNLO_tautau"])
    diboson = plotter.GetMCHist(hist_name, ["WZ", "ZZ"])

    gamma_gamma = total_mc.Clone(f"GGToMuMu_{uuid.uuid4()}")
    gamma_gamma.Add(top, -1)
    gamma_gamma.Add(dy_tau, -1)
    gamma_gamma.Add(diboson, -1)

    return {
        "#gamma#gamma#rightarrow#mu#mu": gamma_gamma,
        "WZ + ZZ": diboson,
        "DY#rightarrow#tau#tau": dy_tau,
        "t#bar{t} + tW + WW": top,
    }


def DrawDataMCOverlay(data, total_mc, mc_components, era, case, region, latex_items, output_path):
    canvas_name = f"Canv_EMU_{region}_DataMC{case}_{uuid.uuid4()}"
    ymax = max(data.GetMaximum(), total_mc.GetMaximum(), 1.0) * 100.0
    ratio = total_mc.Clone(f"EMU_{region}_PredOverData_{uuid.uuid4()}")
    ratio.Divide(data)
    ratio_yrmin, ratio_yrmax = GetYRange(ratio)

    canvas = CMS.cmsDiCanvas(
        canvas_name,
        200,
        4000,
        2e-2,
        ymax,
        ratio_yrmin,
        ratio_yrmax,
        "M(e#mu) [GeV]",
        "Events",
        "Pred./Data",
        square = CMS.kSquare,
        extraSpace = 0.1,
        iPos = 0
    )

    canvas.cd(1).SetLogy(True)
    canvas.cd(1).SetLogx(True)
    canvas.cd(2).SetLogx(True)

    canvas.cd(1)
    stack = ROOT.THStack(f"EMU_{region}_DataMC_Stack_{uuid.uuid4()}", "Stacked")
    legend = MakeLegend(5)
    CMS.cmsDrawStack(
        stack,
        legend,
        mc_components,
        data = data,
        palette = CMS.petroff_8
    )
    canvas.cd(1).RedrawAxis()

    plot_labels = latex_items.copy()
    plot_labels[0] = f"{plot_labels[0]}, {region.replace('_', ' ')}"
    latex = ROOT.TLatex()
    latex.SetTextAlign(14)
    latex.SetTextSize(0.04)
    latex.SetTextFont(42)
    for idx, addon in enumerate(plot_labels):
        latex.DrawLatexNDC(0.18, 0.86 - idx * 0.065, addon.encode("utf-8"))

    canvas.cd(2)
    CMS.cmsDraw(ratio, "P", mcolor=ROOT.kBlack)
    ref_line = ROOT.TLine(200, 1, 4000, 1)
    CMS.cmsDrawLine(ref_line, lcolor=ROOT.kRed, lstyle=ROOT.kDotted)

    CMS.SaveCanvas(
        canvas,
        os.path.join(
            output_path,
            f"era_{era}/plot_{era}_EMU_{region}_DataMC{case}.pdf"
        )
    )


def main():

    eras = ["2016_preVFP", "2016_postVFP", "2017", "2018"]
    cases = ["", "_0BJ", "_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"]
    
    outputPath = "./plots/BKG_EMU/"
    outputRoot = "./plots/BKG_EMU/EMU_FAKE.root"

    os.makedirs(outputPath, exist_ok=True)
    os.makedirs(os.path.dirname(outputRoot), exist_ok=True)

    outputFile = ROOT.TFile(outputRoot, "RECREATE")
    for era in eras:

        CMS.SetLumi(refLumi[era])
        os.makedirs(f"{outputPath}era_{era}", exist_ok=True)

        latex_ee = [
            f"{era}",
            "p_{T}(e) > 52 (50) GeV, |#eta^{SC}(e)| < 2.5",
            "M_{ee} > 200 GeV",
            ""
        ]

        latex_emu = [
            f"{era}",
            "p_{T}(#mu(e)) > 52 (50) GeV, |#eta(#mu(e))| < 2.4 (2.5)",
            "M_{e#mu} > 200 GeV",
            ""
        ]

        if era == "merged":
            latex_ee[0] = "Run2"
            latex_emu[0] = "Run2"

        outputFile.cd()
        outputFile.mkdir(f"{era}/FAKE_EMU_OS")
        outputFile.mkdir(f"{era}/FAKE_EMU_SS")
        outputFile.mkdir(f"{era}/FAKE_EMU_SStoOS")
        outputFile.mkdir(f"{era}/EMU_OS_TOP")
        outputFile.mkdir(f"{era}/TOP_EMUtoEE")
        outputFile.mkdir(f"{era}/TOP_EE")

        EMU_OS = plotterEngine.Plotter(era, rootPath = "../../plotter/Bck/EMU_nominal.root", 
                                        outputPath = "./plots/temp/plots" + era + "/",
                                        channel = "EMU", 
                                        region = "OS")

        EMU_SS = plotterEngine.Plotter(era, rootPath = "../../plotter/Bck/EMU_nominal.root", 
                                        outputPath = "./plots/temp/plots" + era + "/",
                                        channel = "EMU", 
                                        region = "SS")

        EMU_OS_inverted = plotterEngine.Plotter(era, rootPath = "../../plotter/Bck/EMU_nominal.root", 
                                        outputPath = "./plots/temp/plots" + era + "/",
                                        channel = "EMU", 
                                        region = "OS_inverted")

        EMU_SS_inverted = plotterEngine.Plotter(era, rootPath = "../../plotter/Bck/EMU_nominal.root", 
                                        outputPath = "./plots/temp/plots" + era + "/",
                                        channel = "EMU", 
                                        region = "SS_inverted")

        EE = plotterEngine.Plotter(era, rootPath = "./ROOT/260923_OS_nominal_Shears/output.root", 
                                        outputPath = "./plots/temp/plots" + era + "/",
                                        channel = "EE", 
                                        region = "OS")

        for case in cases:

            latex_copy_ee = latex_ee.copy()
            latex_copy_emu = latex_emu.copy()

            if case != "":
                latex_copy_ee[2] = f"{latex_copy_ee[2]}, {addon_hook_jet[case]}"
                latex_copy_emu[2] = f"{latex_copy_emu[2]}, {addon_hook_jet[case]}"



            EMU_OS_TotalMC = EMU_OS.GetMCHist(GetHistoName("PairMass", "OS", case), emu_mcList)
            EMU_OS_data = EMU_OS.GetDataHist(GetHistoName("PairMass", "OS", case))

            EMU_OS_TT = EMU_OS.GetMCHist(GetHistoName("PairMass", "OS", case), ["TTTo2L2Nu"])
            EMU_OS_TT.SetName("EMU_OS_TT" + case)
            EMU_OS_TW = EMU_OS.GetMCHist(GetHistoName("PairMass", "OS", case), ["ST_tW_AntiTop", "ST_tW_Top"])
            EMU_OS_TW.SetName("EMU_OS_TW" + case)
            EMU_OS_WW = EMU_OS.GetMCHist(GetHistoName("PairMass", "OS", case), ["WW"])
            EMU_OS_WW.SetName("EMU_OS_WW" + case)

            EMU_OS_TOP = EMU_OS_TT.Clone("EMU_OS_TOP" + case)
            EMU_OS_TOP.Add(EMU_OS_TW)
            EMU_OS_TOP.Add(EMU_OS_WW)

            EE_OS_TT = EE.GetMCHist(GetHistoNameEE("dielecMass", case), ["TTTo2L2Nu"])
            EE_OS_TT.SetName("EE_OS_TT" + case)
            EE_OS_TW = EE.GetMCHist(GetHistoNameEE("dielecMass", case), ["ST_tW_AntiTop", "ST_tW_Top"])
            EE_OS_TW.SetName("EE_OS_TW" + case)
            EE_OS_WW = EE.GetMCHist(GetHistoNameEE("dielecMass", case), ["WW"])
            EE_OS_WW.SetName("EE_OS_WW" + case)

            EE_OS_TOP = EE_OS_TT.Clone("EE_OS_TOP" + case)
            EE_OS_TOP.Add(EE_OS_TW)
            EE_OS_TOP.Add(EE_OS_WW)

            TOP_EMUtoEE = EE_OS_TOP.Clone(f"TOP_EMUtoEE_{uuid.uuid4()}")
            TOP_EMUtoEE.Divide(EMU_OS_TOP)
            TOP_EMUtoEE.SetName("TOP_EMUtoEE" + case)

            EMU_SS_TotalMC = EMU_SS.GetMCHist(GetHistoName("PairMass", "SS", case), emu_mcList)
            EMU_SS_data = EMU_SS.GetDataHist(GetHistoName("PairMass", "SS", case))
            
            EMU_SS_FAKE = EMU_SS_data.Clone(f"EMU_SS_FAKE_{uuid.uuid4()}")
            EMU_SS_FAKE.Add(EMU_SS_TotalMC, -1)
            EMU_SS_FAKE.SetName("EMU_SS_FAKE" + case)
            EMU_SS_FAKE = SanityCheck(EMU_SS_FAKE)

            EMU_OS_inverted_TotalMC = EMU_OS_inverted.GetMCHist(GetHistoName("PairMass", "OS_inverted", case), emu_mcList)
            EMU_OS_inverted_data = EMU_OS_inverted.GetDataHist(GetHistoName("PairMass", "OS_inverted", case))
            EMU_OS_inverted_MC_components = GetEMUComponentHists(
                EMU_OS_inverted,
                GetHistoName("PairMass", "OS_inverted", case),
                EMU_OS_inverted_TotalMC
            )
            
            EMU_OS_inverted_FAKE = EMU_OS_inverted_data.Clone(f"EMU_OS_inverted_FAKE_{uuid.uuid4()}")
            EMU_OS_inverted_FAKE.Add(EMU_OS_inverted_TotalMC, -1)
            EMU_OS_inverted_FAKE = SanityCheck(EMU_OS_inverted_FAKE)

            EMU_SS_inverted_TotalMC = EMU_SS_inverted.GetMCHist(GetHistoName("PairMass", "SS_inverted", case), emu_mcList)
            EMU_SS_inverted_data = EMU_SS_inverted.GetDataHist(GetHistoName("PairMass", "SS_inverted", case))
            EMU_SS_inverted_MC_components = GetEMUComponentHists(
                EMU_SS_inverted,
                GetHistoName("PairMass", "SS_inverted", case),
                EMU_SS_inverted_TotalMC
            )

            EMU_SS_inverted_FAKE = EMU_SS_inverted_data.Clone(f"EMU_SS_inverted_FAKE_{uuid.uuid4()}")
            EMU_SS_inverted_FAKE.Add(EMU_SS_inverted_TotalMC, -1)
            EMU_SS_inverted_FAKE = SanityCheck(EMU_SS_inverted_FAKE)

            DrawDataMCOverlay(
                EMU_SS_inverted_data,
                EMU_SS_inverted_TotalMC,
                EMU_SS_inverted_MC_components,
                era,
                case,
                "SS_inverted",
                latex_copy_emu,
                outputPath
            )
            DrawDataMCOverlay(
                EMU_OS_inverted_data,
                EMU_OS_inverted_TotalMC,
                EMU_OS_inverted_MC_components,
                era,
                case,
                "OS_inverted",
                latex_copy_emu,
                outputPath
            )

            EMU_inverted_SStoOS = EMU_OS_inverted_FAKE.Clone(f"EMU_inverted_SStoOS_{uuid.uuid4()}")
            EMU_inverted_SStoOS.Divide(EMU_SS_inverted_FAKE)
            EMU_inverted_SStoOS.SetName("EMU_inverted_SStoOS" + case)

            EMU_OS_FAKE_DataDriven = GetOSFromSS(EMU_SS_FAKE, EMU_inverted_SStoOS)
            EMU_OS_FAKE_DataDriven.SetName("EMU_OS_FAKE_DataDriven" + case)

            EMU_OS_NonTargetMC = EMU_OS.GetMCHist(GetHistoName("PairMass", "OS", case), emu_mcList_woTop)
            EMU_OS_NonTargetMC.SetName("EMU_OS_NonTargetMC" + case)

            EMU_OS_TotalMC_ex_TOP = EMU_OS_NonTargetMC.Clone(f"EMU_OS_TotalMC_ex_TOP_{uuid.uuid4()}")
            EMU_OS_TotalMC_ex_TOP.Add(EMU_OS_FAKE_DataDriven)

            EMU_OS_TOP_DataDriven = EMU_OS_data.Clone(f"EMU_OS_TOP_DataDriven_{uuid.uuid4()}")
            EMU_OS_TOP_DataDriven.Add(EMU_OS_TotalMC_ex_TOP, -1)
            EMU_OS_TOP_DataDriven.SetName("EMU_OS_TOP_DataDriven" + case)

            EE_OS_TOP_DataDriven = GetOSFromSS(EMU_OS_TOP_DataDriven, TOP_EMUtoEE)
            EE_OS_TOP_DataDriven.SetName("EE_OS_TOP_DataDriven" + case)
            EE_OS_TOP_DataDriven_Ratio = EE_OS_TOP_DataDriven.Clone(f"EE_OS_TOP_DataDriven_Ratio_{uuid.uuid4()}")
            EE_OS_TOP_DataDriven_Ratio.Divide(EE_OS_TOP)

            outputFile.cd(f"{era}/FAKE_EMU_SStoOS")
            EMU_inverted_SStoOS.Write()
            outputFile.cd(f"{era}/FAKE_EMU_SS")
            EMU_SS_FAKE.Write()
            outputFile.cd(f"{era}/FAKE_EMU_OS")
            EMU_OS_FAKE_DataDriven.Write()
            outputFile.cd(f"{era}/EMU_OS_TOP")
            EMU_OS_TOP_DataDriven.Write()
            outputFile.cd(f"{era}/TOP_EE")
            EE_OS_TOP_DataDriven.Write()
            outputFile.cd(f"{era}/TOP_EMUtoEE")
            TOP_EMUtoEE.Write()


            #################################################################
            # TOP EMU Data-Driven
            #################################################################
            Canv_EMU_OS_TOP_DataDriven = CMS.cmsCanvas(
                "Canv_EMU_OS_TOP_DataDriven",
                200,
                4000,
                2e-2,
                EMU_OS_TOP_DataDriven.GetBinContent(2) * 1e3,
                "M(e#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EMU_OS_TOP_DataDriven.cd(1).SetLogy(True)
            Canv_EMU_OS_TOP_DataDriven.cd(1).SetLogx(True)
            Canv_EMU_OS_TOP_DataDriven.cd(2).SetLogx(True)

            EMU_OS_TOP_DataDriven_Stack = ROOT.THStack("EMU_OS_TOP_DataDriven_Stack", "Stacked")

            Leg_EMU_OS_TOP_DataDriven_Stack = MakeLegend(1)

            EMU_OS_TOP_DataDriven_StackSheet = {
                "tt + tW + WW": EMU_OS_TOP_DataDriven,
            }

            CMS.cmsDrawStack(
                EMU_OS_TOP_DataDriven_Stack,
                Leg_EMU_OS_TOP_DataDriven_Stack,
                EMU_OS_TOP_DataDriven_StackSheet,
                palette = ["#D8B4E2"]
            )
            Canv_EMU_OS_TOP_DataDriven.cd(1).RedrawAxis()

            latex_EMU_OS_TOP_DataDriven_item = latex_copy_emu.copy()
            latex_EMU_OS_TOP_DataDriven_item[0] = f"{latex_EMU_OS_TOP_DataDriven_item[0]}, tt + tW + WW, data-driven"

            Latex_EMU_OS_TOP_DataDriven = ROOT.TLatex()
            Latex_EMU_OS_TOP_DataDriven.SetTextAlign(14);
            Latex_EMU_OS_TOP_DataDriven.SetTextSize(0.04);
            Latex_EMU_OS_TOP_DataDriven.SetTextFont(42);
            for idx, addon in enumerate(latex_EMU_OS_TOP_DataDriven_item):
                Latex_EMU_OS_TOP_DataDriven.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_EMU_OS_TOP_DataDriven, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_EMU_TOP_DataDriven" + case + ".pdf"))


            #################################################################
            # EMU Target Subtraction Inputs
            #################################################################
            EMU_OS_Subtraction_Ratio = EMU_OS_TotalMC_ex_TOP.Clone(f"EMU_OS_Subtraction_Ratio_{uuid.uuid4()}")
            EMU_OS_Subtraction_Ratio.Divide(EMU_OS_data)
            ratio_yrmin, ratio_yrmax = GetYRange(EMU_OS_Subtraction_Ratio)

            Canv_EMU_Target_Subtraction = CMS.cmsDiCanvas(
                "Canv_EMU_Target_Subtraction",
                200,
                4000,
                2e-2,
                EMU_OS_data.GetBinContent(2) * 1e3,
                ratio_yrmin,
                ratio_yrmax,
                "M(e#mu) [GeV]",
                "Events",
                "Pred./Data",
                square = CMS.kSquare,
                extraSpace = 0.1,
                iPos = 0,
            )

            Canv_EMU_Target_Subtraction.cd(1).SetLogy(True)
            Canv_EMU_Target_Subtraction.cd(1).SetLogx(True)
            Canv_EMU_Target_Subtraction.cd(2).SetLogx(True)

            Canv_EMU_Target_Subtraction.cd(1)

            Leg_EMU_Target_Subtraction = MakeLegend(3)
            Leg_EMU_Target_Subtraction.AddEntry(EMU_OS_data, "Data", "lp")

            EMU_Target_Subtraction_Stack = ROOT.THStack("EMU_Target_Subtraction_Stack", "Stacked")
            EMU_Target_Subtraction_StackSheet = {
                "Fake, data-driven": EMU_OS_FAKE_DataDriven,
                "Non-(t#bar{t} + tW + WW), MC": EMU_OS_NonTargetMC,
            }

            CMS.cmsDrawStack(
                EMU_Target_Subtraction_Stack,
                Leg_EMU_Target_Subtraction,
                EMU_Target_Subtraction_StackSheet,
                palette = CMS.petroff_8
            )

            Canv_EMU_Target_Subtraction.cd(1)
            CMS.cmsDraw(EMU_OS_data, "PE", mcolor=ROOT.kBlack)
            Canv_EMU_Target_Subtraction.cd(1).RedrawAxis()

            latex_EMU_Target_Subtraction_item = latex_copy_emu.copy()
            latex_EMU_Target_Subtraction_item[0] = f"{latex_EMU_Target_Subtraction_item[0]}"

            Latex_EMU_Target_Subtraction = ROOT.TLatex()
            Latex_EMU_Target_Subtraction.SetTextAlign(14);
            Latex_EMU_Target_Subtraction.SetTextSize(0.04);
            Latex_EMU_Target_Subtraction.SetTextFont(42);
            for idx, addon in enumerate(latex_EMU_Target_Subtraction_item):
                Latex_EMU_Target_Subtraction.DrawLatexNDC(0.18, 0.86 - idx * 0.065, addon.encode('utf-8'))

            Canv_EMU_Target_Subtraction.cd(2)
            CMS.cmsDraw(EMU_OS_Subtraction_Ratio, "P", mcolor=ROOT.kBlack)

            ref_line = ROOT.TLine(200, 1, 4000, 1)
            CMS.cmsDrawLine(ref_line, lcolor=ROOT.kRed, lstyle=ROOT.kDotted)

            CMS.SaveCanvas(Canv_EMU_Target_Subtraction, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_EMU_Target_Subtraction" + case + ".pdf"))


            #################################################################
            # Fake SS
            #################################################################
            Canv_EMU_SS_FAKE = CMS.cmsCanvas(
                "Canv_EMU_SS_FAKE",
                200,
                4000,
                2e-2,
                max(EMU_SS_FAKE.GetMaximum() * 1e3, 1.0),
                "M(e#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EMU_SS_FAKE.cd(1).SetLogy(True)
            Canv_EMU_SS_FAKE.cd(1).SetLogx(True)
            Canv_EMU_SS_FAKE.cd(2).SetLogx(True)

            EMU_SS_FAKE_Stack = ROOT.THStack("EMU_SS_FAKE_Stack", "Stacked")

            Leg_EMU_SS_FAKE_Stack = MakeLegend(1)

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
                2e-2,
                max(EMU_SS_inverted_FAKE.GetMaximum() * 1e3, 1.0),
                "M(e#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EMU_SS_FAKE_inverted.cd(1).SetLogy(True)
            Canv_EMU_SS_FAKE_inverted.cd(1).SetLogx(True)
            Canv_EMU_SS_FAKE_inverted.cd(2).SetLogx(True)

            EMU_SS_FAKE_inverted_Stack = ROOT.THStack("EMU_SS_FAKE_inverted_Stack", "Stacked")

            Leg_EMU_SS_FAKE_inverted_Stack = MakeLegend(1)

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
                2e-2,
                max(EMU_OS_FAKE_DataDriven.GetMaximum() * 1e3, 1.0),
                "M(e#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EMU_OS_FAKE.cd(1).SetLogy(True)
            Canv_EMU_OS_FAKE.cd(1).SetLogx(True)
            Canv_EMU_OS_FAKE.cd(2).SetLogx(True)

            EMU_OS_FAKE_Stack = ROOT.THStack("EMU_OS_FAKE_Stack", "Stacked")

            Leg_EMU_OS_FAKE_Stack = MakeLegend(1)

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
                2e-2,
                max(EMU_OS_inverted_FAKE.GetMaximum() * 1e3, 1.0),
                "M(e#mu) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EMU_OS_FAKE_inverted.cd(1).SetLogy(True)
            Canv_EMU_OS_FAKE_inverted.cd(1).SetLogx(True)
            Canv_EMU_OS_FAKE_inverted.cd(2).SetLogx(True)

            EMU_OS_FAKE_inverted_Stack = ROOT.THStack("EMU_OS_FAKE_inverted_Stack", "Stacked")

            Leg_EMU_OS_FAKE_inverted_Stack = MakeLegend(1)

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
                EMU_OS_inverted_FAKE.GetBinContent(2) * 1e3,
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

            Leg_SStoOS = MakeLegend(2)
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
            # TOP EMUtoEE
            #################################################################
            yrmin_TOP_EMUtoEE, yrmax_TOP_EMUtoEE = GetYRange(TOP_EMUtoEE)
            Canv_TOP_EMUtoEE = CMS.cmsDiCanvas(
                "Canv_TOP_EMUtoEE",
                200,
                4000,
                2e-2,
                EE_OS_TOP.GetBinContent(2) * 1e3,
                yrmin_TOP_EMUtoEE,
                yrmax_TOP_EMUtoEE,
                "Mass [GeV]",
                "Events",
                "ee/e#mu",
                square = CMS.kSquare,
                extraSpace = 0.1,
                iPos = 0,
            )

            Canv_TOP_EMUtoEE.cd(1).SetLogy(True)
            Canv_TOP_EMUtoEE.cd(1).SetLogx(True)
            Canv_TOP_EMUtoEE.cd(2).SetLogx(True)

            Canv_TOP_EMUtoEE.cd(1)

            Leg_TOP_EMUtoEE = MakeLegend(2)
            Leg_TOP_EMUtoEE.AddEntry(EE_OS_TOP, "ee, tt + tW + WW, MC", "lp")
            Leg_TOP_EMUtoEE.AddEntry(EMU_OS_TOP, "e#mu, tt + tW + WW, MC", "lp")

            Canv_TOP_EMUtoEE.cd(1)
            CMS.cmsDraw(EE_OS_TOP, "P", mcolor=ROOT.kBlack)
            CMS.cmsDraw(EMU_OS_TOP, "P", mcolor=ROOT.kRed)


            latex_TOP_EMUtoEE_item = latex_copy_emu.copy()
            latex_TOP_EMUtoEE_item[0] = f"{latex_TOP_EMUtoEE_item[0]}, ee/e#mu"

            Latex_TOP_EMUtoEE = ROOT.TLatex()
            Latex_TOP_EMUtoEE.SetTextAlign(14);
            Latex_TOP_EMUtoEE.SetTextSize(0.04);
            Latex_TOP_EMUtoEE.SetTextFont(42);
            for idx, addon in enumerate(latex_TOP_EMUtoEE_item):
                Latex_TOP_EMUtoEE.DrawLatexNDC(0.18, 0.86 - idx * 0.065, addon.encode('utf-8'))

            Canv_TOP_EMUtoEE.cd(2)
            CMS.cmsDraw(TOP_EMUtoEE, "P", mcolor=ROOT.kBlack)

            ref_line = ROOT.TLine(200, 1, 4000, 1)
            CMS.cmsDrawLine(ref_line, lcolor=ROOT.kRed, lstyle=ROOT.kDotted)

            CMS.SaveCanvas(Canv_TOP_EMUtoEE, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_TOP_EMUtoEE" + case + ".pdf"))

            #################################################################
            # TOP Comparison
            #################################################################
            yrmin_TOP_Comparision, yrmax_TOP_Comparision = GetYRange(EE_OS_TOP_DataDriven_Ratio)
            Canv_TOP_Comparision = CMS.cmsDiCanvas(
                "Canv_TOP_Comparision",
                200,
                4000,
                2e-2,
                EE_OS_TOP_DataDriven.GetBinContent(2) * 1e3,
                yrmin_TOP_Comparision,
                yrmax_TOP_Comparision,
                "M(ee) [GeV]",
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

            Leg_TOP_Comparision = MakeLegend(4)
            Leg_TOP_Comparision.AddEntry(EE_OS_TOP_DataDriven, "tt + tW + WW, data-driven", "lp")

            TOP_Comparision_Stack = ROOT.THStack("TOP_Comparision_Stack", "Stacked")
            TOP_Comparision_StackSheet = {
                "tW, MC": EE_OS_TW,
                "WW, MC": EE_OS_WW,
                "tt, MC": EE_OS_TT,
            }

            CMS.cmsDrawStack(TOP_Comparision_Stack, Leg_TOP_Comparision, TOP_Comparision_StackSheet)
            
            Canv_TOP_Comparision.cd(1)
            CMS.cmsDraw(EE_OS_TOP_DataDriven, "PE", mcolor=ROOT.kBlack)
            Canv_TOP_Comparision.cd(1).RedrawAxis()

            latex_TOP_Comparision_item = latex_copy_ee.copy()
            latex_TOP_Comparision_item[0] = f"{latex_TOP_Comparision_item[0]}, tt + tW + WW"
        
            Latex_TOP_Comparision = ROOT.TLatex()
            Latex_TOP_Comparision.SetTextAlign(14);
            Latex_TOP_Comparision.SetTextSize(0.04);
            Latex_TOP_Comparision.SetTextFont(42);
            for idx, addon in enumerate(latex_TOP_Comparision_item):
                Latex_TOP_Comparision.DrawLatexNDC(0.18, 0.86 - idx * 0.065, addon.encode('utf-8'))

            Canv_TOP_Comparision.cd(2)
            CMS.cmsDraw(EE_OS_TOP_DataDriven_Ratio, "P", mcolor=ROOT.kBlack)

            ref_line = ROOT.TLine(200, 1, 4000, 1)
            CMS.cmsDrawLine(ref_line, lcolor=ROOT.kRed, lstyle=ROOT.kDotted)

            CMS.SaveCanvas(Canv_TOP_Comparision, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_TOP_DataDriven" + case + ".pdf"))



    outputFile.Close()


if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    main()
