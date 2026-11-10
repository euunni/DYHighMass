#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import math

parser = argparse.ArgumentParser()
parser.add_argument('--era', help=' : era to plot')
args = parser.parse_args()


CMS.SetExtraText("Private work")
CMS.SetEnergy("13")
ROOT.gROOT.SetBatch(ROOT.kTRUE)

def _tune_logx_axis_labels(axis):
    """
    ROOT log-x axes tend to hide non-decade labels (e.g. 200).
    These options request more labels and disable 10^n exponent formatting.
    """
    if not axis:
        return
    try:
        axis.SetMoreLogLabels(True)
        axis.SetNoExponent(True)
    except Exception:
        pass

def _hist_for_logx(hist, xmin_floor=1e-3):
    """
    ROOT fills the whole pad if a bar is drawn from x=0 on a log-x axis.
    Rebuild the histogram so the first bin edge is positive, keeping the
    original first-bin content.
    """
    if hist is None:
        return hist

    n = hist.GetNbinsX()
    edges = [hist.GetXaxis().GetBinLowEdge(i) for i in range(1, n + 2)]
    if edges[0] > 0:
        return hist

    first_edge = xmin_floor if xmin_floor > 0 else 1e-3
    if first_edge >= edges[1]:
        first_edge = 0.1 * edges[1]
    edges[0] = first_edge

    new_hist = ROOT.TH1D(
        f"{hist.GetName()}_logx_{uuid.uuid4()}",
        hist.GetTitle(),
        n,
        array.array("d", edges),
    )
    new_hist.SetDirectory(0)
    new_hist.Sumw2()
    for i in range(0, n + 2):
        new_hist.SetBinContent(i, hist.GetBinContent(i))
        new_hist.SetBinError(i, hist.GetBinError(i))
    return new_hist

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

    "ST_s",
    "ST_t_AntiTop",
    "ST_t_Top",
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

dylist = [
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
    "NNLO_EE_2000toInf"
]

stlist = [
    "ST_s",
    "ST_t_AntiTop",
    "ST_t_Top"
]

toplist = [
    "TTTo2L2Nu",
    "ST_tW_AntiTop",
    "ST_tW_Top",
    "WW"
]

ewlist = ["WZ", "ZZ"]

GG_ElEl_list = [
    "GGToEE_10to30_ElEl",
    "GGToEE_30to50_ElEl",
    "GGToEE_50to200_ElEl",
    "GGToEE_200to1500_ElEl",
    "GGToEE_1500toInf_ElEl",
]

GG_InelElElInel_list = [
    "GGToEE_10to30_InelElElInel",
    "GGToEE_30to50_InelElElInel",
    "GGToEE_50to200_InelElElInel",
    "GGToEE_200to1500_InelElElInel",
    "GGToEE_1500toInf_InelElElInel",
]

GG_InelInel_list = [
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
    "2018": 59.45
}

xSec = {
    "NNLO_10to50": 7012.53,
    "NNLO_inc": 1925.65,
    "NNLO_100to200": 77.95,
    "NNLO_200to400": 2.78,
    "NNLO_400to500": 0.15,
    "NNLO_500to700": 0.084,
    "NNLO_700to800": 0.013,
    "NNLO_800to1000": 0.011,
    "NNLO_1000to1500": 0.006,
    "NNLO_1500to2000": 0.00081,
    "NNLO_2000toInf": 0.0002,

    "NNLO_MUMU_10to50": 7012.53,
    "NNLO_MUMU_inc": 1925.65,
    "NNLO_MUMU_100to200": 77.95,
    "NNLO_MUMU_200to400": 2.78,
    "NNLO_MUMU_400to500": 0.15,
    "NNLO_MUMU_500to700": 0.084,
    "NNLO_MUMU_700to800": 0.013,
    "NNLO_MUMU_800to1000": 0.011,
    "NNLO_MUMU_1000to1500": 0.006,
    "NNLO_MUMU_1500to2000": 0.00081,
    "NNLO_MUMU_2000toInf": 0.0002,

    "NNLO_EE_10to50": 7012.53,
    "NNLO_EE_inc": 1925.65,
    "NNLO_EE_100to200": 77.95,
    "NNLO_EE_200to400": 2.78,
    "NNLO_EE_400to500": 0.15,
    "NNLO_EE_500to700": 0.084,
    "NNLO_EE_700to800": 0.013,
    "NNLO_EE_800to1000": 0.011,
    "NNLO_EE_1000to1500": 0.006,
    "NNLO_EE_1500to2000": 0.00081,
    "NNLO_EE_2000toInf": 0.0002,
    
    "NNLO_tautau": 1164.31,

    "ST_s": 10.32,
    "ST_t_AntiTop": 80.0,
    "ST_t_Top": 134.2,
    "ST_tW_AntiTop": 39.65,
    "ST_tW_Top": 39.65,
    
    "TTTo2L2Nu": 88.51,
    
    "WJetsToLNu": 61526.7,
    
    "WW": 11.09,
    "WZ": 27.59,
    "ZZ": 12.17,

    "GGToMuMu_10to30_ElEl": 11.39,
    "GGToMuMu_30to50_ElEl": 0.6716,
    "GGToMuMu_50to200_ElEl": 0.2473,
    "GGToMuMu_200to1500_ElEl": 0.005637,
    "GGToMuMu_1500toInf_ElEl": 0.000003813,

    "GGToMuMu_10to30_InelElElInel": 10.52,
    "GGToMuMu_30to50_InelElElInel": 0.8958,
    "GGToMuMu_50to200_InelElElInel": 0.3694,
    "GGToMuMu_200to1500_InelElElInel": 0.0111,
    "GGToMuMu_1500toInf_InelElElInel": 0.000008786,

    "GGToMuMu_10to30_InelInel": 10.40,
    "GGToMuMu_30to50_InelInel": 1.171,
    "GGToMuMu_50to200_InelInel": 0.5615,
    "GGToMuMu_200to1500_InelInel": 0.02173,
    "GGToMuMu_1500toInf_InelInel": 0.00002023,

    "GGToEE_10to30_ElEl": 11.39,
    "GGToEE_30to50_ElEl": 0.6716,
    "GGToEE_50to200_ElEl": 0.2473,
    "GGToEE_200to1500_ElEl": 0.005637,
    "GGToEE_1500toInf_ElEl": 0.000003813,

    "GGToEE_10to30_InelElElInel": 10.52,
    "GGToEE_30to50_InelElElInel": 0.8958,
    "GGToEE_50to200_InelElElInel": 0.3694,
    "GGToEE_200to1500_InelElElInel": 0.0111,
    "GGToEE_1500toInf_InelElElInel": 0.000008786,

    "GGToEE_10to30_InelInel": 10.40,
    "GGToEE_30to50_InelInel": 1.171,
    "GGToEE_50to200_InelInel": 0.5615,
    "GGToEE_200to1500_InelInel": 0.02173,
    "GGToEE_1500toInf_InelInel": 0.00002023,
}

normFactor = {}


class Plotter:
    def __init__(self, era):
        # self.rootPath = "/u/user/haeun/SE_UserHome/CMSAnalysis/DYHighMass/260105/ROOT/260219/output.root"
        self.rootPath = "./ROOT/261028_OS_nominal_GenPartIdx/output.root"
        self.era = era
        self.outputPath = "./plots/261028_OS_nominal_GenPartIdx/plot_" + era + "/"

        os.makedirs(self.outputPath, exist_ok=True)

        self.lumi = refLumi[self.era]
        CMS.SetLumi(self.lumi)

        self.PrepareFiles()
        self.PrepareNorm()


    def PrepareFiles(self):
        self.fileSet = ROOT.TFile(self.rootPath, "READ");

    def PrepareNorm(self):
        for mcSet in mcList:
            nEvent = self.fileSet.Get(self.era + "/" + mcSet + "/h_EventInfo").GetBinContent(4)
            normFactor[mcSet] = (1000. * self.lumi * xSec[mcSet]) / nEvent;

    def Plot(self, histName, 
        case,
        massbin,
        hook,
        xmin = -1, 
        xmax = -1, 
        ymin = -1, 
        ymax = -1, 
        yrmin = -1, 
        yrmax = -1,
        xTitle = "", 
        yTitle = "Events", 
        ratioTitle = "Data/Pred.",
        logy = True, 
        logx = False
        ):

        doAutoXrange = False
        doAutoYrange = False
        doAutoYRatiorange = False

        if (xmin == -1 and xmax == -1): doAutoXrange = True
        if (ymin == -1 and ymax == -1): doAutoYrange = True
        if (yrmin == -1 and yrmax == -1): doAutoYRatiorange = True

        self.histName = ""

        if case != "" and massbin != "":
            self.histName += case + massbin + "/" + histName + case + massbin
        if case != "" and massbin == "":
            self.histName += case + "/" + histName + case
        if case == "" and massbin != "":
            self.histName += massBin + "/" + histName + massBin
        if case == "" and massbin == "":
            self.histName = histName

        self.histOutputName = histName + case + massbin
        canvasName = self.era + "_" + histName


        data = self.fileSet.Get(self.era + "/Data/" + self.histName)
        DY = self.GetMCHist("DY")
        TOP = self.GetTopMCHist()
        ST = self.GetMCHist("ST")
        DY_tau = self.GetMCHist("NNLO_tautau")
        EW = self.GetMCHist("EW")
        GG_ElEl = self.GetMCHist("GG_ElEl")
        GG_InelElElInel = self.GetMCHist("GG_InelElElInel")
        GG_InelInel = self.GetMCHist("GG_InelInel")

        if logx:
            xmin_floor = xmin if xmin > 0 else 1e-3
            data = _hist_for_logx(data, xmin_floor)
            DY = _hist_for_logx(DY, xmin_floor)
            TOP = _hist_for_logx(TOP, xmin_floor)
            ST = _hist_for_logx(ST, xmin_floor)
            DY_tau = _hist_for_logx(DY_tau, xmin_floor)
            EW = _hist_for_logx(EW, xmin_floor)
            GG_ElEl = _hist_for_logx(GG_ElEl, xmin_floor)
            GG_InelElElInel = _hist_for_logx(GG_InelElElInel, xmin_floor)
            GG_InelInel = _hist_for_logx(GG_InelInel, xmin_floor)

        GG = GG_ElEl.Clone(f"GG_{uuid.uuid4()}")
        GG.Add(GG_InelElElInel)
        GG.Add(GG_InelInel)
        # WJets = self.GetMCHist("WJetsToLNu")
        
        MC = DY.Clone(f"MC_{uuid.uuid4()}")
        MC.Add(TOP)
        MC.Add(ST)
        MC.Add(DY_tau)
        MC.Add(EW)
        MC.Add(GG_ElEl)
        MC.Add(GG_InelElElInel)
        MC.Add(GG_InelInel)

        dataOmc = data.Clone(f"ratio_{uuid.uuid4()}")
        dataOmc.Divide(MC)

        if doAutoXrange:
            xmin = MC.GetBinLowEdge(1)
            xmax = MC.GetBinLowEdge(MC.GetNbinsX()) +  MC.GetBinWidth(MC.GetNbinsX())

        if doAutoYrange:
            ymin = 2e-2
            ymax_ref = data.GetBinContent(data.GetMaximumBin())
            if ymax_ref <= 0:
                ymax_ref = MC.GetBinContent(MC.GetMaximumBin())
            ymax = ymax_ref * 1e5

            if histName in ("h_dielecMass", "h_dielecPt", "h_leadingElecPt", "h_subleadingElecPt"):
                ymax = ymax_ref * 1e3
            if histName == "h_ElecRecoGenDeltaR":
                ymax = ymax_ref * 50.

        if doAutoYRatiorange:
            nBinsX = dataOmc.GetNbinsX()
            residual = -999
            for i in range(1, nBinsX + 1):
                if dataOmc.GetBinCenter(i) > xmin and dataOmc.GetBinCenter(i) < xmax:
                    if dataOmc.GetBinContent(i) > 0.2 and dataOmc.GetBinContent(i) < 1.8:
                        if (residual < abs(1 - dataOmc.GetBinContent(i))):
                            residual = abs(1 - dataOmc.GetBinContent(i))

            if residual < 0.05:
                yrmin = 1 - 0.06
                yrmax = 1 + 0.06

            elif residual < 0.1:
                yrmin = 1 - 0.12
                yrmax = 1 + 0.12
            
            elif residual < 0.15:
                yrmin = 1 - 0.18
                yrmax = 1 + 0.18

            elif residual < 0.2:
                yrmin = 1 - 0.24
                yrmax = 1 + 0.24
            
            elif residual < 0.3:
                yrmin = 1 - 0.36
                yrmax = 1 + 0.36

            elif residual < 0.4:
                yrmin = 1 - 0.48
                yrmax = 1 + 0.48

            else:
                yrmin = 0.5
                yrmax = 1.5

        # ROOT cannot draw a log-x axis if the range includes 0.
        is_mass_plot = (histName == "h_dielecMass")
        xmin_axis = xmin
        if logx:
            xmin_axis = max(MC.GetBinLowEdge(1), 1e-3)
            if is_mass_plot and xmin > 0:
                xmin_axis = max(1e-6, float(xmin) * 0.98)

        dicanv = CMS.cmsDiCanvas(
            canvasName,
            xmin_axis,
            xmax,
            ymin,
            ymax,
            yrmin,
            yrmax,
            xTitle,
            yTitle,
            ratioTitle,
            square = CMS.kSquare,
            extraSpace = 0.1,
            iPos = 0,
        )

        dicanv.cd(1)
        pad1 = dicanv.cd(1)
        if logy:
            pad1.SetLogy(True)
        if logx:
            pad1.SetLogx(True)

        stack = ROOT.THStack("stack", "Stacked")

        nLegItems = 7
        leg = CMS.cmsLeg(0.60, 0.89 - 0.05 * nLegItems, 0.88, 0.89, textSize=0.03)
        leg.AddEntry(data, "Data", "lp")

        stackSeet = {
            # "WJets": WJets,
            "Z/#gamma*#rightarrow#tau#tau": DY_tau,
            "#gamma#gamma#rightarrowee": GG,
            "Single top": ST,
            "WZ + ZZ": EW,
            "t#bar{t} + tW + WW": TOP,
            "Z/#gamma*#rightarrowee": DY
        }

        CMS.cmsDrawStack(stack, leg, stackSeet, palette=CMS.petroff_8)
        CMS.cmsDraw(data, "P", mcolor=ROOT.kBlack)

        if logx and is_mass_plot:
            # cmsDiCanvas draws axes with an internal frame hist; tune that x-axis.
            frame1 = None
            prims1 = pad1.GetListOfPrimitives()
            if prims1:
                for obj in prims1:
                    try:
                        if obj.InheritsFrom("TH1"):
                            frame1 = obj
                            break
                    except Exception:
                        pass
            if frame1:
                _tune_logx_axis_labels(frame1.GetXaxis())
            pad1.Modified()
            pad1.Update()
            pad1.RedrawAxis()

        latex = ROOT.TLatex()
        latex.SetTextAlign(14);
        latex.SetTextSize(0.04);
        latex.SetTextFont(42);
        for idx, addon in enumerate(hook):
            latex.DrawLatexNDC(0.18, 0.86 - idx * 0.065, addon.encode('utf-8'))

        dicanv.cd(2)
        pad2 = dicanv.cd(2)
        if logx:
            pad2.SetLogx(True)

        CMS.cmsDraw(dataOmc, "P", mcolor=ROOT.kBlack)

        if logx and is_mass_plot:
            frame2 = None
            prims2 = pad2.GetListOfPrimitives()
            if prims2:
                for obj in prims2:
                    try:
                        if obj.InheritsFrom("TH1"):
                            frame2 = obj
                            break
                    except Exception:
                        pass
            if frame2:
                _tune_logx_axis_labels(frame2.GetXaxis())
            pad2.Modified()
            pad2.Update()
            pad2.RedrawAxis()


        ref_lines = []
        ratio_step = 0.2
        first_line = math.ceil(yrmin / ratio_step)
        last_line = math.floor(yrmax / ratio_step)
        for line_index in range(first_line, last_line + 1):
            yval = line_index * ratio_step
            line = ROOT.TLine(xmin_axis, yval, xmax, yval)
            CMS.cmsDrawLine(line, lcolor=ROOT.kGray+1, lstyle=ROOT.kDashed)
            ref_lines.append(line)

        ref_line = ROOT.TLine(xmin_axis, 1, xmax, 1)
        CMS.cmsDrawLine(ref_line, lcolor=ROOT.kRed, lstyle=ROOT.kDotted)

        CMS.SaveCanvas(dicanv, os.path.join(self.outputPath, self.era + "_" + self.histOutputName + ".pdf"))
        # CMS.SaveCanvas(dicanv, os.path.join(self.outputPath, self.era + "_" + histName + ".png"))

    def GetMCHist(self, mcName):
        if mcName == "DY":
            return self.GetDYHist()
        elif mcName == "ST":
            return self.GetSingleTopHist()
        elif mcName == "EW":
            return self.GetEWHist()
        elif mcName == "GG_ElEl":
            return self.GetGG_ElEl()
        elif mcName == "GG_InelElElInel":
            return self.GetGG_InelElElInel()
        elif mcName == "GG_InelInel":
            return self.GetGG_InelInel()
        else:
            hist = self.fileSet.Get(self.era + "/" + mcName + "/" + self.histName).Clone(f"{self.histName}_{uuid.uuid4()}")
            hist.SetDirectory(0)
            hist.SetStats(0);
            hist.Scale(normFactor[mcName]);

            return hist

    def GetDYHist(self):
        histoSet = {}
        for mc in dylist:
            histoSet[mc] = self.fileSet.Get(self.era + "/" + mc + "/" + self.histName).Clone(f"{self.histName}_{uuid.uuid4()}")
            histoSet[mc].SetDirectory(0)
            histoSet[mc].SetStats(0);
            histoSet[mc].Scale(normFactor[mc]);

        returnHist = histoSet["NNLO_EE_inc"].Clone(f"DY_{uuid.uuid4()}")
        for mc in dylist:
            if (mc != "NNLO_EE_inc"):
                returnHist.Add(histoSet[mc])

        return returnHist

    def GetGG_ElEl(self):
        histoSet = {}
        for mc in GG_ElEl_list:
            histoSet[mc] = self.fileSet.Get(self.era + "/" + mc + "/" + self.histName).Clone(f"{self.histName}_{uuid.uuid4()}")
            histoSet[mc].SetDirectory(0)
            histoSet[mc].SetStats(0);
            histoSet[mc].Scale(normFactor[mc]);

        returnHist = histoSet["GGToEE_10to30_ElEl"].Clone(f"GG_ElEl_{uuid.uuid4()}")
        for mc in GG_ElEl_list:
            if (mc != "GGToEE_10to30_ElEl"):
                returnHist.Add(histoSet[mc])

        return returnHist

    def GetGG_InelElElInel(self):
        histoSet = {}
        for mc in GG_InelElElInel_list:
            histoSet[mc] = self.fileSet.Get(self.era + "/" + mc + "/" + self.histName).Clone(f"{self.histName}_{uuid.uuid4()}")
            histoSet[mc].SetDirectory(0)
            histoSet[mc].SetStats(0);
            histoSet[mc].Scale(normFactor[mc]);

        returnHist = histoSet["GGToEE_10to30_InelElElInel"].Clone(f"GG_InelElElInel_{uuid.uuid4()}")
        for mc in GG_InelElElInel_list:
            if (mc != "GGToEE_10to30_InelElElInel"):
                returnHist.Add(histoSet[mc])

        return returnHist

    def GetGG_InelInel(self):
        histoSet = {}
        for mc in GG_InelInel_list:
            histoSet[mc] = self.fileSet.Get(self.era + "/" + mc + "/" + self.histName).Clone(f"{self.histName}_{uuid.uuid4()}")
            histoSet[mc].SetDirectory(0)
            histoSet[mc].SetStats(0);
            histoSet[mc].Scale(normFactor[mc]);

        returnHist = histoSet["GGToEE_10to30_InelInel"].Clone(f"GG_InelInel_{uuid.uuid4()}")
        for mc in GG_InelInel_list:
            if (mc != "GGToEE_10to30_InelInel"):
                returnHist.Add(histoSet[mc])

        return returnHist

    def GetSingleTopHist(self):
        histoSet = {}
        for mc in stlist:
            histoSet[mc] = self.fileSet.Get(self.era + "/" + mc + "/" + self.histName).Clone(f"{self.histName}_{uuid.uuid4()}")
            histoSet[mc].SetDirectory(0)
            histoSet[mc].SetStats(0);
            histoSet[mc].Scale(normFactor[mc]);

        returnHist = histoSet["ST_s"].Clone(f"SingleTop_{uuid.uuid4()}")
        for mc in stlist:
            if (mc != "ST_s"):
                returnHist.Add(histoSet[mc])

        return returnHist

    def GetEWHist(self):
        histoSet = {}
        for mc in ewlist:
            histoSet[mc] = self.fileSet.Get(self.era + "/" + mc + "/" + self.histName).Clone(f"{self.histName}_{uuid.uuid4()}")
            histoSet[mc].SetDirectory(0)
            histoSet[mc].SetStats(0);
            histoSet[mc].Scale(normFactor[mc]);

        returnHist = histoSet["WZ"].Clone(f"EW_{uuid.uuid4()}")
        for mc in ewlist:
            if (mc != "WZ"):
                returnHist.Add(histoSet[mc])

        return returnHist

    def GetTopMCHist(self):
        histoSet = {}
        for mc in toplist:
            histoSet[mc] = self.fileSet.Get(self.era + "/" + mc + "/" + self.histName).Clone(f"{self.histName}_{uuid.uuid4()}")
            histoSet[mc].SetDirectory(0)
            histoSet[mc].SetStats(0);
            histoSet[mc].Scale(normFactor[mc]);

        returnHist = histoSet["TTTo2L2Nu"].Clone(f"TopMC_{uuid.uuid4()}")
        for mc in toplist:
            if (mc != "TTTo2L2Nu"):
                returnHist.Add(histoSet[mc])

        return returnHist

    def GetCombinedMCHist(self, histName):
        combined = None
        n_added = 0
        for mc in mcList:
            obj = self.fileSet.Get(self.era + "/" + mc + "/" + histName)
            if not obj:
                print(f"[SaveCombinedDeltaR] skip missing {self.era}/{mc}/{histName}")
                continue
            if mc not in normFactor:
                print(f"[SaveCombinedDeltaR] skip {mc}: no normFactor")
                continue
            hist = obj.Clone(f"{histName}_{mc}_{uuid.uuid4()}")
            hist.SetDirectory(0)
            hist.SetStats(0)
            hist.Scale(normFactor[mc])
            if combined is None:
                combined = hist.Clone(histName)
                combined.SetDirectory(0)
                combined.Reset()
            combined.Add(hist)
            n_added += 1

        if combined is None:
            return None, 0

        combined.SetName(histName)
        combined.SetTitle("#DeltaR(e_{reco}, e_{gen})")
        combined.GetXaxis().SetTitle("#DeltaR(e_{reco}, e_{gen})")
        combined.GetYaxis().SetTitle("Weighted electrons")
        return combined, n_added

    def SaveCombinedDeltaRAllEras(self, histName="h_ElecRecoGenDeltaR"):
        out_dir = os.path.dirname(self.outputPath.rstrip("/"))
        os.makedirs(out_dir, exist_ok=True)
        out_path = os.path.join(out_dir, f"{histName}_combined.txt")

        original_era = self.era
        original_lumi = self.lumi

        summary_lines = []
        for era in refLumi:
            self.era = era
            self.lumi = refLumi[era]
            try:
                self.PrepareNorm()
            except Exception as e:
                print(f"[SaveCombinedDeltaR] skip {era}: PrepareNorm failed ({e})")
                continue

            hist, n_added = self.GetCombinedMCHist(histName)
            if hist is None:
                print(f"[SaveCombinedDeltaR] skip {era}: no histograms found")
                continue

            total_yield = hist.Integral(0, hist.GetNbinsX() + 1)
            above_threshold_bin = hist.GetXaxis().FindFixBin(0.3 + 1e-12)
            above_threshold_yield = hist.Integral(
                above_threshold_bin, hist.GetNbinsX() + 1
            )
            above_threshold_fraction = (
                above_threshold_yield / total_yield if total_yield != 0. else 0.
            )
            summary_line = (
                f"{era}: {n_added} samples, "
                f"total integral = {total_yield:.6g}, "
                f"DeltaR > 0.3 integral = {above_threshold_yield:.6g}, "
                f"fraction = {above_threshold_fraction:.6%}"
            )
            summary_lines.append(summary_line)

        with open(out_path, "w") as outFile:
            outFile.write(f"# {histName}: combined MC (normalized), per era\n")
            for line in summary_lines:
                outFile.write(line + "\n")

        self.era = original_era
        self.lumi = original_lumi
        self.PrepareNorm()
        print(f"[SaveCombinedDeltaR] wrote {out_path}")


def main(args):
    plotter = Plotter(args.era)
    plotter.SaveCombinedDeltaRAllEras()

    # cases = ["", "_0J", "_1J", "_mtJ", "_0BJ", "_1BJ", "_mt1BJ", "_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"]
    cases = ["", "_0BJ", "_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"]
    # cases = ["_0BJ"]
    # cases = [""]
    # massBins = ["", "_m200_220", "_m220_243", "_m243_273", "_m273_320", "_m320_380", "_m380_440", "_m440_510", "_m510_600", "_m600_700", "_m700_830", "_m830_1000", "_m1000_1500", "_m1500_4000"]
    massBins = [""] 

    addon_hook = {
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

    yrmax_vec = {
        "": -1,
        "_0J": -1,
        "_1J": -1,
        "_mtJ": -1,
        "_0BJ": 1.5,
        "_1BJ": -1,
        "_mt1BJ": -1,
        # "_bVeto_0J": 1.3,
        # "_bVeto_1J": 1.5,
        # "_bVeto_mt1J": 1.5,    
        "_bVeto_0J": 1.5,
        "_bVeto_1J": 1.5,
        "_bVeto_mt1J": 1.5  
    }

    yrmin_vec = {
        "": -1,
        "_0J": -1,
        "_1J": -1,
        "_mt1J": -1,
        "_0BJ": 0.5,
        "_1BJ": -1,
        "_mt1BJ": -1,
        "_bVeto_0J": 0.5,
        "_bVeto_1J": 0.5,
        "_bVeto_mt1J": 0.5 
    }

    addon_hook_mass = {
        "": "M_{ee} > 200 GeV",
        "_m200_220": "200 < M_{ee} < 220 GeV",
        "_m220_243": "220 < M_{ee} < 243 GeV",
        "_m243_273": "243 < M_{ee} < 273 GeV",
        "_m273_320": "273 < M_{ee} < 320 GeV",
        "_m320_380": "320 < M_{ee} < 380 GeV",
        "_m380_440": "380 < M_{ee} < 440 GeV",
        "_m440_510": "440 < M_{ee} < 510 GeV",
        "_m510_600": "510 < M_{ee} < 600 GeV",
        "_m600_700": "600 < M_{ee} < 700 GeV",
        "_m700_830": "700 < M_{ee} < 830 GeV",
        "_m830_1000": "830 < M_{ee} < 1000 GeV",
        "_m1000_1500": "1000 < M_{ee} < 1500 GeV",
        "_m1500_4000": "1500 < M_{ee} < 4000 GeV"
    }

    # Temporary manual y-axis ranges for the dielectron mass plots.
    mass_y_range = {
        # SS nominal
        # "2016_preVFP": (1., 6e4),
        # "2016_postVFP": (2., 7.5e4),
        # "2017": (2., 1e5),
        # "2018": (5., 2e5),
        # SS nominal
        # "2016_preVFP": (5., 4.5e6),
        # "2016_postVFP": (1.5, 4e6),
        # "2017": (4.5, 9e6),
        # "2018": (7., 1.5e7),
        # SS inverted
        # "2016_preVFP": (1., 2e4),
        # "2016_postVFP": (0.7, 1.5e4),
        # "2017": (1., 3e4),
        # "2018": (5., 4.5e4),
        # OS inverted
        # "2016_preVFP": (2., 4e4), 
        # "2016_postVFP": (0.9, 3.5e4),
        # "2017": (1.5, 9e4),
        # "2018": (4., 2e5),
    }

    latex = [
        args.era + ", dielectron channel",
        "p_{T}(e) > 52 (50) GeV, |#eta^{SC}(e)| < 2.5",
        "",
        "",
    ]

    plotter.Plot("h_nJet",  "", "", latex, xTitle = "N_{jet}", xmin = 0, xmax = 14)
    plotter.Plot("h_nBJet", "", "", latex, xTitle = "N_{b-jet}", xmin = 0, xmax = 14)
    plotter.Plot("h_ElecRecoGenDeltaR", "", "", latex, xTitle = "#DeltaR(e_{reco}, e_{gen})", yTitle = "Electrons", logx = True, logy = True)
 
    for case in cases:

        latex_temp = latex.copy()
        latex_temp[2] = addon_hook[case]

        plotter.Plot("h_dielecMass", case, ""                     , latex_temp, xTitle = "M(ee) [GeV]"  ,xmin = 200, xmax = 4000, yrmin = 0.5, yrmax = 1.5, logy = True, logx = True)

        for massbin in massBins:
            latex_temp[3] = addon_hook_mass[massbin]

            plotter.Plot("h_JetPt", case, massbin                 , latex_temp, xTitle = "pT(jet) [GeV]"          ,xmin = 0, xmax = 500, yrmin = 0.6, yrmax = 1.4, logy = True)
            plotter.Plot("h_JetEta", case, massbin                , latex_temp, xTitle = "#eta(jet)"              ,xmin = -2.5, xmax = 2.5, yrmin = 0.6, yrmax = 1.4, logy = True)
            plotter.Plot("h_JetPhi", case, massbin                , latex_temp, xTitle = "#phi(jet)"              ,xmin = -3.141593, xmax = 3.141593, yrmin = 0.6, yrmax = 1.4, logy = True)

            plotter.Plot("h_BJetPt", case, massbin                , latex_temp, xTitle = "pT(b-jet) [GeV]"          ,xmin = 0, xmax = 500, logy = True)
            plotter.Plot("h_BJetEta", case, massbin               , latex_temp, xTitle = "#eta(b-jet)"              ,xmin = -2.5, xmax = 2.5, logy = True)
            plotter.Plot("h_BJetPhi", case, massbin               , latex_temp, xTitle = "#phi(b-jet)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

            plotter.Plot("h_LeadingElecPt", case, massbin         , latex_temp, xTitle = "pT(e) [GeV]"          ,xmin = 0, xmax = 500, yrmin = 0.6, yrmax = 1.4, logy = True)
            plotter.Plot("h_LeadingElecEta", case, massbin        , latex_temp, xTitle = "#eta(e)"              ,xmin = -2.5, xmax = 2.5, yrmin = 0.6, yrmax = 1.4, logy = True)
            plotter.Plot("h_LeadingElecPhi", case, massbin        , latex_temp, xTitle = "#phi(e)"              ,xmin = -3.141593, xmax = 3.141593, yrmin = 0.8, yrmax = 1.2, logy = True)

            plotter.Plot("h_SubleadingElecPt", case, massbin      , latex_temp, xTitle = "pT(e) [GeV]"          ,xmin = 0, xmax = 500, yrmin = 0.6, yrmax = 1.4, logy = True)
            plotter.Plot("h_SubleadingElecEta", case, massbin     , latex_temp, xTitle = "#eta(e)"              ,xmin = -2.5, xmax = 2.5, yrmin = 0.6, yrmax = 1.4, logy = True)
            plotter.Plot("h_SubleadingElecPhi", case, massbin     , latex_temp, xTitle = "#phi(e)"              ,xmin = -3.141593, xmax = 3.141593, yrmin = 0.8, yrmax = 1.2, logy = True)

            plotter.Plot("h_ElecPt", case, massbin                , latex_temp, xTitle = "pT(e) [GeV]"          ,xmin = 15, xmax = 500, logy = True, logx = True)
            plotter.Plot("h_ElecEta", case, massbin               , latex_temp, xTitle = "#eta(e)"              ,xmin = -2.5, xmax = 2.5, logy = True)
            plotter.Plot("h_ElecPhi", case, massbin               , latex_temp, xTitle = "#phi(e)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)
            plotter.Plot("h_ElecDeltaR", case, massbin            , latex_temp, xTitle = "#DeltaR(e_{1}, e_{2})" ,xmin = 0, xmax = 6.4, logy = True)

            plotter.Plot("h_dielecPt", case, massbin              , latex_temp, xTitle = "pT(ee) [GeV]" ,xmin = 0, xmax = 500, yrmin = 0.6, yrmax = 1.4, logy = True)
            plotter.Plot("h_dielecRap", case, massbin             , latex_temp, xTitle = "rapidity(ee)"      ,xmin = -2.8, xmax = 2.8, yrmin = 0.6, yrmax = 1.4, logy = True)


if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    main(args)
