#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import plotterEngine_EE as plotterEngine

CMS.SetExtraText("Private Work")
CMS.SetEnergy("13")
ROOT.gROOT.SetBatch(ROOT.kTRUE)

X_AXIS_MIN = 200
X_AXIS_MAX = 4000

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

def GetHistDrawXRange(hist, xmin, xmax):
    xlow = None
    xhigh = None
    axis = hist.GetXaxis()

    for i in range(1, hist.GetNbinsX() + 1):
        if hist.GetBinContent(i) == 0:
            continue

        bin_low = axis.GetBinLowEdge(i)
        bin_high = axis.GetBinUpEdge(i)
        if bin_high < xmin or bin_low > xmax:
            continue

        xlow = max(bin_low, xmin) if xlow is None else min(xlow, max(bin_low, xmin))
        xhigh = min(bin_high, xmax) if xhigh is None else max(xhigh, min(bin_high, xmax))

    if xlow is None or xhigh is None:
        return xmin, xmax
    return xlow, xhigh

def _tune_logx_axis_labels(axis):
    """
    ROOT log-x axes tend to hide non-decade labels (e.g. 200).
    Request more labels and disable 10^n exponent formatting.
    """
    if not axis:
        return
    try:
        axis.SetMoreLogLabels(True)
        axis.SetNoExponent(True)
    except Exception:
        pass

def _tune_pad_logx(pad):
    """Find the internal frame TH1 of a pad and tune its log-x axis labels."""
    frame = None
    prims = pad.GetListOfPrimitives()
    if prims:
        for obj in prims:
            try:
                if obj.InheritsFrom("TH1"):
                    frame = obj
                    break
            except Exception:
                pass
    if frame:
        _tune_logx_axis_labels(frame.GetXaxis())
    pad.Modified()
    pad.Update()
    pad.RedrawAxis()

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

refLumi = {
    "2016_preVFP": 19.5,
    "2016_postVFP": 16.8,
    "2017": 42.12,
    "2018": 59.45,
    "merged": 137.88
}

Y_AXIS_RANGES = {
    # Set y-axis ranges by the main plot category.
    # FakeOSDataDriven / SStoOS are keyed by era; FakeSS / FakeSSinverted /
    # FakeOSinverted are keyed by jet category and used for all eras.
    # Only _0BJ, _bVeto_0J, _bVeto_1J, and _bVeto_mt1J use manual ranges.

    "FakeSS": {
        "_0BJ": (0.1, 2e4),
        "_bVeto_0J": (0.1, 1e4),
        "_bVeto_1J": (0.1, 1e4),
        "_bVeto_mt1J": (0.1, 1e4),
    },
    "FakeSSinverted": {
        "_0BJ": (0.1, 1e6),
        "_bVeto_0J": (0.1, 4e4),
        "_bVeto_1J": (0.1, 2e5),
        "_bVeto_mt1J": (0.1, 4e5),
    },
    "FakeOSinverted": {
        "_0BJ": (0.1, 1e6),
        "_bVeto_0J": (0.1, 1e5),
        "_bVeto_1J": (0.1, 5e5),
        "_bVeto_mt1J": (0.1, 5e5),
    },
    "FakeOSDataDriven": {
        "2016_preVFP": (0.1, 2e3),
        "2016_postVFP": (0.1, 2e3),
        "2017": (0.1, 2e4),
        "2018": (0.1, 3e4),
    },
    "SStoOS": {
        "top": {
            "2016_preVFP": (0.1, 7e4),
            "2016_postVFP": (0.1, 7e4),
            "2017": (0.1, 2e5),
            "2018": (0.1, 3e5),
        },
        "ratio": (0, 3.0),
    }
}

def GetManualYRange(plotCategory, padName=None, era=None, case=None):
    manualRange = Y_AXIS_RANGES.get(plotCategory)
    if not manualRange:
        return None

    if isinstance(manualRange, dict):
        if padName is not None:
            padRange = manualRange.get(padName)
            if isinstance(padRange, dict):
                if case is not None and case in padRange:
                    return padRange.get(case)
                return padRange.get(era) if era is not None else None
            return padRange
        if case is not None and case in manualRange:
            return manualRange.get(case)
        if era is not None:
            return manualRange.get(era)
        return None
    if padName is None:
        return manualRange
    return None

def GetCanvasYRange(plotCategory, hist, ymin=0.1, ymaxScale=1e3, padName=None, era=None, case=None):
    manualRange = GetManualYRange(plotCategory, padName, era, case)
    if manualRange:
        return manualRange
    return ymin, hist.GetMaximum() * ymaxScale

def GetOSFromSS(SS_Fake, SStoOS):

    FAKE_DataDriven = SS_Fake.Clone(f"FAKE_DataDriven_{uuid.uuid4()}")
    FAKE_DataDriven.Reset("ICES");

    for i in range(1, SStoOS.GetNbinsX() + 1):
        if SStoOS.GetBinContent(i) == 0 or SS_Fake.GetBinContent(i) <= 0:
            FAKE_DataDriven.SetBinContent(i, 0)
        else:
            FAKE_DataDriven.SetBinContent(i, SS_Fake.GetBinContent(i) * SStoOS.GetBinContent(i))

    return FAKE_DataDriven


def PrintSStoOSDebug(era, case, histoName, os_data, os_mc, os_fake, ss_data, ss_mc, ss_fake, ratio):
    case_label = case if case else "inclusive"
    print(f"\n[SStoOS debug] era={era}, case={case_label}, hist={histoName}")
    print(
        "bin  mass_low  mass_high  "
        "OS_Data  OS_MC  OS_Data-MC(raw)  OS_Fake  "
        "SS_Data  SS_MC  SS_Data-MC(raw)  SS_Fake  OS/SS"
    )

    xaxis = os_fake.GetXaxis()
    for i in range(1, os_fake.GetNbinsX() + 1):
        bin_low = xaxis.GetBinLowEdge(i)
        bin_high = xaxis.GetBinUpEdge(i)
        if bin_high < X_AXIS_MIN or bin_low > X_AXIS_MAX:
            continue

        os_data_value = os_data.GetBinContent(i)
        os_mc_value = os_mc.GetBinContent(i)
        ss_data_value = ss_data.GetBinContent(i)
        ss_mc_value = ss_mc.GetBinContent(i)
        os_raw = os_data_value - os_mc_value
        ss_raw = ss_data_value - ss_mc_value

        print(
            f"{i:3d}  {bin_low:8.1f}  {bin_high:9.1f}  "
            f"{os_data_value:8.3f}  {os_mc_value:8.3f}  {os_raw:15.3f}  {os_fake.GetBinContent(i):8.3f}  "
            f"{ss_data_value:8.3f}  {ss_mc_value:8.3f}  {ss_raw:15.3f}  {ss_fake.GetBinContent(i):8.3f}  "
            f"{ratio.GetBinContent(i):8.4f}"
        )


def ParseArgs():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--print-sstoos-values",
        action="store_true",
        help="Print inverted OS/SS Data, MC, Data-MC, and OS/SS bin values for debugging.",
    )
    return parser.parse_args()


def main(print_sstoos_values=False):

    # eras = ["2016_preVFP", "2016_postVFP", "2017", "2018", "merged"]
    eras = ["2016_preVFP", "2016_postVFP", "2017", "2018"]
    cases = ["", "_0BJ", "_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"]

    histoName_EE = "h_dielecMass"
    outputPath = "./plots/BKG/EE_FAKE/"
    outputRoot = "./plots/BKG/EE_FAKE.root"

    os.makedirs(outputPath, exist_ok=True)

    outputFile = ROOT.TFile(outputRoot, "RECREATE")

    for era in eras:

        CMS.SetLumi(refLumi[era])
        os.makedirs(f"{outputPath}era_{era}", exist_ok=True)

        latex_ee = [
            f"{era}",
            "p_{T}(e) > 52 (50) GeV, |#eta(e)| < 2.5",
            "M_{ee} > 200 GeV",
            ""
        ]

        if era == "merged":
            latex_ee[0] = "Run2"

        outputFile.cd()
        outputFile.mkdir(f"{era}/FAKE_EE_OS")
        outputFile.mkdir(f"{era}/FAKE_EE_SS")
        outputFile.mkdir(f"{era}/FAKE_EE_SStoOS")

        EE_OS = plotterEngine.Plotter(era, rootPath = "./ROOT/260923_OS_nominal_Shears/output.root",
                                            outputPath = outputPath,
                                            channel = "EE",
                                            region = "OS")

        EE_SS = plotterEngine.Plotter(era, rootPath = "./ROOT/261028_SS_nominal_GenPartIdx/output.root",
                                            outputPath = outputPath,
                                            channel = "EE",
                                            region = "SS")

        EE_OS_inverted = plotterEngine.Plotter(era, rootPath = "./ROOT/261028_OS_inverted_GenPartIdx/output.root",
                                            outputPath = outputPath,
                                            channel = "EE",
                                            region = "OS_inverted")

        EE_SS_inverted = plotterEngine.Plotter(era, rootPath = "./ROOT/261028_SS_inverted_GenPartIdx/output.root",
                                            outputPath = outputPath,
                                            channel = "EE",
                                            region = "SS_inverted")

        for case in cases:

            latex_copy_ee = latex_ee.copy()

            if case != "":
                latex_copy_ee[2] = f"{latex_copy_ee[2]}, {addon_hook_jet[case]}"

            histoName = histoName_EE
            if case != "":
                histoName = case + "/" + histoName + case

            EE_SS_TotalMC = EE_SS.GetMCHist(histoName, mcList)
            EE_SS_data = EE_SS.GetDataHist(histoName)

            EE_SS_FAKE = EE_SS_data.Clone(f"EE_SS_FAKE_{uuid.uuid4()}")
            EE_SS_FAKE.Add(EE_SS_TotalMC, -1)
            EE_SS_FAKE = SanityCheck(EE_SS_FAKE)
            EE_SS_FAKE.SetName("EE_SS_FAKE" + case)

            EE_OS_inverted_TotalMC = EE_OS_inverted.GetMCHist(histoName, mcList)
            EE_OS_inverted_data = EE_OS_inverted.GetDataHist(histoName)

            EE_OS_inverted_FAKE = EE_OS_inverted_data.Clone(f"EE_OS_inverted_FAKE_{uuid.uuid4()}")
            EE_OS_inverted_FAKE.Add(EE_OS_inverted_TotalMC, -1)
            EE_OS_inverted_FAKE = SanityCheck(EE_OS_inverted_FAKE)

            EE_SS_inverted_TotalMC = EE_SS_inverted.GetMCHist(histoName, mcList)
            EE_SS_inverted_data = EE_SS_inverted.GetDataHist(histoName)

            EE_SS_inverted_FAKE = EE_SS_inverted_data.Clone(f"EE_SS_inverted_FAKE_{uuid.uuid4()}")
            EE_SS_inverted_FAKE.Add(EE_SS_inverted_TotalMC, -1)
            EE_SS_inverted_FAKE = SanityCheck(EE_SS_inverted_FAKE)

            EE_inverted_SStoOS = EE_OS_inverted_FAKE.Clone(f"EE_inverted_SStoOS_{uuid.uuid4()}")
            EE_inverted_SStoOS.Divide(EE_SS_inverted_FAKE)
            EE_inverted_SStoOS.SetName("EE_inverted_SStoOS" + case)

            if print_sstoos_values:
                PrintSStoOSDebug(
                    era,
                    case,
                    histoName,
                    EE_OS_inverted_data,
                    EE_OS_inverted_TotalMC,
                    EE_OS_inverted_FAKE,
                    EE_SS_inverted_data,
                    EE_SS_inverted_TotalMC,
                    EE_SS_inverted_FAKE,
                    EE_inverted_SStoOS,
                )

            EE_OS_FAKE_DataDriven = GetOSFromSS(EE_SS_FAKE, EE_inverted_SStoOS)
            EE_OS_FAKE_DataDriven.SetName("EE_OS_FAKE_DataDriven" + case)

            outputFile.cd(f"{era}/FAKE_EE_SStoOS")
            EE_inverted_SStoOS.Write()
            outputFile.cd(f"{era}/FAKE_EE_SS")
            EE_SS_FAKE.Write()
            outputFile.cd(f"{era}/FAKE_EE_OS")
            EE_OS_FAKE_DataDriven.Write()


            #################################################################
            # Fake SS
            #################################################################
            ymin_EE_SS_FAKE, ymax_EE_SS_FAKE = GetCanvasYRange(
                "FakeSS", EE_SS_FAKE, case=case
            )

            Canv_EE_SS_FAKE = CMS.cmsCanvas(
                "Canv_EE_SS_FAKE",
                X_AXIS_MIN,
                X_AXIS_MAX,
                ymin_EE_SS_FAKE,
                ymax_EE_SS_FAKE,
                "M(ee) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EE_SS_FAKE.cd(1).SetLogx(True)
            Canv_EE_SS_FAKE.cd(1).SetLogy(True)
            Canv_EE_SS_FAKE.cd(2).SetLogx(True)

            EE_SS_FAKE_Stack = ROOT.THStack("EE_SS_FAKE_Stack", "Stacked")

            Leg_EE_SS_FAKE_Stack = CMS.cmsLeg(0.80, 0.6, 0.89, 0.7, textSize=0.03)

            EE_SS_FAKE_StackSheet = {
                "Fake": EE_SS_FAKE,
            }

            CMS.cmsDrawStack(EE_SS_FAKE_Stack, Leg_EE_SS_FAKE_Stack, EE_SS_FAKE_StackSheet)
            _tune_pad_logx(Canv_EE_SS_FAKE.cd(1))

            latex_EE_SS_FAKE_item = latex_copy_ee.copy()
            latex_EE_SS_FAKE_item[0] = f"{latex_EE_SS_FAKE_item[0]}, Fake SS"

            Latex_EE_SS_FAKE = ROOT.TLatex()
            Latex_EE_SS_FAKE.SetTextAlign(14);
            Latex_EE_SS_FAKE.SetTextSize(0.04);
            Latex_EE_SS_FAKE.SetTextFont(42);
            for idx, addon in enumerate(latex_EE_SS_FAKE_item):
                Latex_EE_SS_FAKE.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_EE_SS_FAKE, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_FakeSS" + case + ".pdf"))

            #################################################################
            # Fake SS inverted
            #################################################################
            ymin_EE_SS_FAKE_inverted, ymax_EE_SS_FAKE_inverted = GetCanvasYRange(
                "FakeSSinverted", EE_SS_inverted_FAKE, case=case
            )

            Canv_EE_SS_FAKE_inverted = CMS.cmsCanvas(
                "Canv_EE_SS_FAKE_inverted",
                X_AXIS_MIN,
                X_AXIS_MAX,
                ymin_EE_SS_FAKE_inverted,
                ymax_EE_SS_FAKE_inverted,
                "M(ee) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EE_SS_FAKE_inverted.cd(1).SetLogx(True)
            Canv_EE_SS_FAKE_inverted.cd(1).SetLogy(True)
            Canv_EE_SS_FAKE_inverted.cd(2).SetLogx(True)

            EE_SS_FAKE_inverted_Stack = ROOT.THStack("EE_SS_FAKE_inverted_Stack", "Stacked")

            Leg_EE_SS_FAKE_inverted_Stack = CMS.cmsLeg(0.80, 0.6, 0.89, 0.7, textSize=0.03)

            EE_SS_FAKE_inverted_StackSheet = {
                "Fake": EE_SS_inverted_FAKE,
            }

            CMS.cmsDrawStack(EE_SS_FAKE_inverted_Stack, Leg_EE_SS_FAKE_inverted_Stack, EE_SS_FAKE_inverted_StackSheet)
            _tune_pad_logx(Canv_EE_SS_FAKE_inverted.cd(1))

            latex_EE_SS_FAKE_inverted_item = latex_copy_ee.copy()
            latex_EE_SS_FAKE_inverted_item[0] = f"{latex_EE_SS_FAKE_inverted_item[0]}, SS, Fake CR"

            Latex_EE_SS_FAKE_inverted = ROOT.TLatex()
            Latex_EE_SS_FAKE_inverted.SetTextAlign(14);
            Latex_EE_SS_FAKE_inverted.SetTextSize(0.04);
            Latex_EE_SS_FAKE_inverted.SetTextFont(42);
            for idx, addon in enumerate(latex_EE_SS_FAKE_inverted_item):
                Latex_EE_SS_FAKE_inverted.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_EE_SS_FAKE_inverted, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_FakeSSinverted" + case + ".pdf"))


            #################################################################
            # Fake OS
            #################################################################
            ymin_EE_OS_FAKE, ymax_EE_OS_FAKE = GetCanvasYRange(
                "FakeOSDataDriven", EE_OS_FAKE_DataDriven, era=era
            )

            Canv_EE_OS_FAKE = CMS.cmsCanvas(
                "Canv_EE_OS_FAKE",
                X_AXIS_MIN,
                X_AXIS_MAX,
                ymin_EE_OS_FAKE,
                ymax_EE_OS_FAKE,
                "M(ee) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EE_OS_FAKE.cd(1).SetLogx(True)
            Canv_EE_OS_FAKE.cd(1).SetLogy(True)
            Canv_EE_OS_FAKE.cd(2).SetLogx(True)

            EE_OS_FAKE_Stack = ROOT.THStack("EE_OS_FAKE_Stack", "Stacked")

            Leg_EE_OS_FAKE_Stack = CMS.cmsLeg(0.80, 0.60, 0.89, 0.70, textSize=0.03)

            EE_OS_FAKE_StackSheet = {
                "Fake": EE_OS_FAKE_DataDriven,
            }

            CMS.cmsDrawStack(EE_OS_FAKE_Stack, Leg_EE_OS_FAKE_Stack, EE_OS_FAKE_StackSheet)
            _tune_pad_logx(Canv_EE_OS_FAKE.cd(1))

            latex_EE_OS_FAKE_item = latex_copy_ee.copy()
            latex_EE_OS_FAKE_item[0] = f"{latex_EE_OS_FAKE_item[0]}, Fake OS data-driven"
            
            Latex_EE_OS_FAKE = ROOT.TLatex()
            Latex_EE_OS_FAKE.SetTextAlign(14);
            Latex_EE_OS_FAKE.SetTextSize(0.04);
            Latex_EE_OS_FAKE.SetTextFont(42);
            for idx, addon in enumerate(latex_EE_OS_FAKE_item):
                Latex_EE_OS_FAKE.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_EE_OS_FAKE, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_FakeOSDataDriven" + case + ".pdf"))

            #################################################################
            # Fake OS inverted
            #################################################################
            ymin_EE_OS_FAKE_inverted, ymax_EE_OS_FAKE_inverted = GetCanvasYRange(
                "FakeOSinverted", EE_OS_inverted_FAKE, case=case
            )

            Canv_EE_OS_FAKE_inverted = CMS.cmsCanvas(
                "Canv_EE_OS_FAKE_inverted",
                X_AXIS_MIN,
                X_AXIS_MAX,
                ymin_EE_OS_FAKE_inverted,
                ymax_EE_OS_FAKE_inverted,
                "M(ee) [GeV]",
                "Events",
                square = True,
                extraSpace = 0.05,
                iPos = 0,
                yTitOffset = 1.25
            )

            Canv_EE_OS_FAKE_inverted.cd(1).SetLogx(True)
            Canv_EE_OS_FAKE_inverted.cd(1).SetLogy(True)
            Canv_EE_OS_FAKE_inverted.cd(2).SetLogx(True)

            EE_OS_FAKE_inverted_Stack = ROOT.THStack("EE_OS_FAKE_inverted_Stack", "Stacked")

            Leg_EE_OS_FAKE_inverted_Stack = CMS.cmsLeg(0.80, 0.6, 0.89, 0.7, textSize=0.03)

            EE_OS_FAKE_inverted_StackSheet = {
                "Fake": EE_OS_inverted_FAKE,
            }

            CMS.cmsDrawStack(EE_OS_FAKE_inverted_Stack, Leg_EE_OS_FAKE_inverted_Stack, EE_OS_FAKE_inverted_StackSheet)
            _tune_pad_logx(Canv_EE_OS_FAKE_inverted.cd(1))

            latex_EE_OS_FAKE_inverted_item = latex_copy_ee.copy()
            latex_EE_OS_FAKE_inverted_item[0] = f"{latex_EE_OS_FAKE_inverted_item[0]}, OS, Fake CR"

            Latex_EE_OS_FAKE_inverted = ROOT.TLatex()
            Latex_EE_OS_FAKE_inverted.SetTextAlign(14);
            Latex_EE_OS_FAKE_inverted.SetTextSize(0.04);
            Latex_EE_OS_FAKE_inverted.SetTextFont(42);
            for idx, addon in enumerate(latex_EE_OS_FAKE_inverted_item):
                Latex_EE_OS_FAKE_inverted.DrawLatexNDC(0.20, 0.89 - idx * 0.065, addon.encode('utf-8'))

            CMS.SaveCanvas(Canv_EE_OS_FAKE_inverted, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_FakeOSinverted" + case + ".pdf"))




            #################################################################
            # Fake SStoOS
            #################################################################
            manual_SStoOS_ratio_range = GetManualYRange("SStoOS", "ratio", era)
            if manual_SStoOS_ratio_range:
                yrmin, yrmax = manual_SStoOS_ratio_range
            elif case == "_bVeto_0J":
                yrmin, yrmax = GetYRange(EE_inverted_SStoOS)
            else:
                yrmin, yrmax = 0.9, 1.3

            xmin_axis = X_AXIS_MIN
            ymin_SStoOS_top, ymax_SStoOS_top = GetCanvasYRange(
                "SStoOS", EE_OS_inverted_FAKE, ymin=2e-2, padName="top", era=era
            )

            Canv_SStoOS = CMS.cmsDiCanvas(
                "Canv_SStoOS",
                xmin_axis,
                X_AXIS_MAX,
                ymin_SStoOS_top,
                ymax_SStoOS_top,
                yrmin,
                yrmax,
                "M(ee) [GeV]",
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

            # Leg_SStoOS = CMS.cmsLeg(0.70, 0.70 - 0.05 * 2, 0.89, 0.70, textSize=0.03)
            Leg_SStoOS = CMS.cmsLeg(0.70, 0.74, 0.89, 0.84, textSize=0.03)
            Leg_SStoOS.AddEntry(EE_OS_inverted_FAKE, "Data - MC, OS", "lp")
            Leg_SStoOS.AddEntry(EE_SS_inverted_FAKE, "Data - MC, SS", "lp")

            Canv_SStoOS.cd(1)
            CMS.cmsDraw(EE_OS_inverted_FAKE, "P", mcolor=ROOT.kBlack)
            CMS.cmsDraw(EE_SS_inverted_FAKE, "P", mcolor=ROOT.kRed)

            _tune_pad_logx(Canv_SStoOS.cd(1))

            latex_SStoOS_item = latex_copy_ee.copy()
            latex_SStoOS_item[0] = f"{latex_SStoOS_item[0]}, OS/SS, only one electron with inverted ID"
            
            Latex_SStoOS = ROOT.TLatex()
            Latex_SStoOS.SetTextAlign(14);
            Latex_SStoOS.SetTextSize(0.04);
            Latex_SStoOS.SetTextFont(42);
            for idx, addon in enumerate(latex_SStoOS_item):
                Latex_SStoOS.DrawLatexNDC(0.18, 0.86 - idx * 0.065, addon.encode('utf-8'))

            Canv_SStoOS.cd(2)
            CMS.cmsDraw(EE_inverted_SStoOS, "P", mcolor=ROOT.kBlack)

            ref_line_xmin, ref_line_xmax = GetHistDrawXRange(EE_inverted_SStoOS, xmin_axis, X_AXIS_MAX)
            ratio_grid_lines = []
            for grid_index in range(1, int(yrmax / 0.5) + 1):
                grid_y = grid_index * 0.5
                if grid_y <= yrmin or grid_y >= yrmax:
                    continue
                grid_line = ROOT.TLine(ref_line_xmin, grid_y, ref_line_xmax, grid_y)
                CMS.cmsDrawLine(
                    grid_line,
                    lcolor=ROOT.kGray + 1,
                    lstyle=ROOT.kDashed,
                )
                ratio_grid_lines.append(grid_line)

            ref_line = ROOT.TLine(ref_line_xmin, 1, ref_line_xmax, 1)
            CMS.cmsDrawLine(ref_line, lcolor=ROOT.kRed, lstyle=ROOT.kDotted)

            _tune_pad_logx(Canv_SStoOS.cd(2))

            CMS.SaveCanvas(Canv_SStoOS, os.path.join(outputPath, "era_" + era + "/plot_" + era + "_SStoOS" + case + ".pdf"))




    outputFile.Close()


if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    args = ParseArgs()
    main(print_sstoos_values=args.print_sstoos_values)
