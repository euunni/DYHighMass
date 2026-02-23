#!/usr/bin/env python3

import os, ROOT, sys, argparse, uuid
import cmsstyle as CMS

CMS.SetExtraText("Preliminary")
CMS.SetEnergy("13")
ROOT.gROOT.SetBatch(ROOT.kTRUE)

parser = argparse.ArgumentParser()
parser.add_argument('--era', required=True, help='era to plot (e.g. 2018) or ALL/Run2')

# Inputs/outputs
parser.add_argument('--root', default="./../Batch/EE/ROOT/260211_ele23ele12_mediumID_pT2820/output.root", help='input ROOT file path')
parser.add_argument('--outdir', default="./../Batch/EE/plots/260211_ele23ele12_mediumID_pT2820/AccEff/", help='output directory')

# Which plots to make
parser.add_argument('--plot_misid_prob', action='store_true',
                    help='Plot electron charge misID probability (from stored ep/em numerator+denominator hists)')
parser.add_argument('--misid_mc', default='DY',
                    help='MC group/sample for misID prob plot (DY/ST/EW/GG_ElEl/GG_InelElElInel/GG_InelInel/ALL or concrete sample name)')

parser.add_argument('--plot_acceptance', action='store_true',
                    help='Plot gen-level acceptance vs gen mass (overlay num/den + ratio)')
parser.add_argument('--plot_efficiency', action='store_true',
                    help='Plot gen-level efficiency vs gen mass (overlay num/den + ratio)')
parser.add_argument('--mc', default='DY',
                    help='MC group/sample for acceptance/efficiency plots (DY/ST/EW/GG_ElEl/GG_InelElElInel/GG_InelInel/ALL or concrete sample name)')

args = parser.parse_args()


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
    "NNLO_EE_2000toInf",
]

stlist = ["ST_s", "ST_t_AntiTop", "ST_t_Top", "ST_tW_AntiTop", "ST_tW_Top"]
ewlist = ["WW", "WZ", "ZZ"]

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
    "2017": 41.5,
    "2018": 59.8
}

# Keep the existing xSec map style; used only for scaling the overlay plots.
# Acceptance/efficiency ratios are invariant under overall scaling.
xSec = {
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
    "WW": 11.09,
    "WZ": 27.59,
    "ZZ": 12.17,
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


class Plotter:
    def __init__(self, eras, eraLabel, rootPath, outputPath):
        self.rootPath = rootPath
        self.eras = eras
        self.eraLabel = eraLabel
        self.outputPath = outputPath
        os.makedirs(self.outputPath, exist_ok=True)

        # Display total lumi (scaling is still done per-era)
        self.lumi = sum(refLumi[e] for e in self.eras)
        CMS.SetLumi(self.lumi)

        self.fileSet = ROOT.TFile(self.rootPath, "READ")
        if (not self.fileSet) or self.fileSet.IsZombie():
            raise RuntimeError(f"Failed to open ROOT file: {self.rootPath}")

        self.normFactor = {}
        self.PrepareNorm()

    def PrepareNorm(self):
        # normFactor[(era, sample)] = (1000*lumi_era*xsec_sample) / sumw(sample, era)
        for era in self.eras:
            for mcSet in mcList:
                if mcSet not in xSec:
                    continue
                hinfo = self.fileSet.Get(era + "/" + mcSet + "/h_EventInfo")
                if not hinfo:
                    continue
                sumw = hinfo.GetBinContent(4)
                if sumw == 0:
                    continue
                self.normFactor[(era, mcSet)] = (1000.0 * refLumi[era] * xSec[mcSet]) / sumw

    def _GetMCHistScaled(self, era, sample, histName):
        h = self.fileSet.Get(era + "/" + sample + "/" + histName)
        if not h:
            return None
        h = h.Clone(f"{era}_{histName}_{sample}_{uuid.uuid4()}")
        h.SetDirectory(0)
        h.SetStats(0)
        key = (era, sample)
        if key in self.normFactor:
            h.Scale(self.normFactor[key])
        return h

    def GetMCGroupHistScaled(self, group, histName):
        g = str(group)
        if g.upper() in ["ALL", "TOTAL", "TOTALMC", "MC", "ALLMC"]:
            samples = mcList
        elif g == "DY":
            samples = dylist
        elif g == "ST":
            samples = stlist
        elif g == "EW":
            samples = ewlist
        elif g == "GG_ElEl":
            samples = GG_ElEl_list
        elif g == "GG_InelElElInel":
            samples = GG_InelElElInel_list
        elif g == "GG_InelInel":
            samples = GG_InelInel_list
        else:
            samples = [g]

        acc = None
        for era in self.eras:
            for s in samples:
                h = self._GetMCHistScaled(era, s, histName)
                if h is None:
                    continue
                if acc is None:
                    acc = h.Clone(f"{self.eraLabel}_{g}_{histName}_{uuid.uuid4()}")
                    acc.SetDirectory(0)
                else:
                    acc.Add(h)
        return acc

    def _PlotNumDenWithRatio(self, name, num, den, numColor, denColor, yTitle, ratioTitle, denLegend, numLegend, extraTopLeftLines=None, legendTextSize=0.04):
        if (not num) or (not den):
            print(f"[{name}] Missing histogram(s). Skipping.")
            return

        # ratio
        ratio = num.Clone(f"ratio_{uuid.uuid4()}")
        ratio.SetDirectory(0)
        ratio.Divide(den)

        # Ranges
        xmin = den.GetBinLowEdge(1)
        xmax = den.GetBinLowEdge(den.GetNbinsX()) + den.GetBinWidth(den.GetNbinsX())
        ymin = 2e-2
        ymax = max(den.GetMaximum(), num.GetMaximum()) * 5e3 if max(den.GetMaximum(), num.GetMaximum()) > 0 else 1.0

        # Ratio y-range (Acceptance/Efficiency/misID are probabilities)
        rt = ratioTitle.lower()
        if rt in ["acceptance", "efficiency"]:
            yrmin, yrmax = 0.0, 1.2
        elif "misid" in rt:
            yrmin, yrmax = 0.0, 0.006
        else:
            yrmin, yrmax = 0.5, 1.5

        canv = CMS.cmsDiCanvas(
            name,
            xmin, xmax,
            ymin, ymax,
            yrmin, yrmax,
            "M(ee) [GeV]", yTitle, ratioTitle,
            square=CMS.kSquare, extraSpace=0.1, iPos=0,
        )

        canv.cd(1)
        canv.cd(1).SetLogx(True)
        canv.cd(1).SetLogy(True)

        CMS.cmsDraw(den, "P", mcolor=denColor, lcolor=denColor)
        CMS.cmsDraw(num, "P", mcolor=numColor, lcolor=numColor)

        # Top-left text (match original style)
        latex = ROOT.TLatex()
        latex.SetTextAlign(14)
        latex.SetTextSize(0.04)
        latex.SetTextFont(42)
        lines = [
            f"{self.eraLabel}",
            "p_{T}(e) > 28 (20) GeV, |#eta(e)| < 2.5",
            # "p_{T}(e) > 50 (38) GeV, |#eta(e)| < 2.5",
        ]
        if extraTopLeftLines:
            lines.extend(extraTopLeftLines)
        for idx, line in enumerate(lines):
            latex.DrawLatexNDC(0.18, 0.86 - idx * 0.065, line.encode("utf-8"))

        # leg = CMS.cmsLeg(0.60, 0.6, 0.89, 0.89, textSize=legendTextSize)
        leg = CMS.cmsLeg(0.60, 0.6, 0.89, 0.79, textSize=legendTextSize)
        leg.AddEntry(den, denLegend, "lp")
        leg.AddEntry(num, numLegend, "lp")

        canv.cd(2)
        pad2 = canv.cd(2)
        pad2.SetLogx(True)
        # Add more space to avoid x-title/label overlap in ratio pad.
        # Note: cmsDiCanvas draws axes using an internal "frame" hist, so we must tune that axis (not ratio's).
        pad2.SetBottomMargin(0.36)
        pad2.SetTopMargin(0.02)
        # Ratio points style (match acceptance-like look)
        ratio.SetMarkerStyle(20)
        ratio.SetMarkerSize(0.8)
        CMS.cmsDraw(ratio, "P", mcolor=ROOT.kBlack, lcolor=ROOT.kBlack)

        # Try to find the frame histogram in pad2 and tune its x-axis label/title spacing
        frame2 = None
        prims = pad2.GetListOfPrimitives()
        if prims:
            for obj in prims:
                try:
                    if obj.InheritsFrom("TH1"):
                        frame2 = obj
                        break
                except Exception:
                    pass
        if frame2:
            frame2.GetXaxis().SetTitleOffset(1.40)
            frame2.GetXaxis().SetLabelOffset(0.015)
            frame2.GetXaxis().SetTitleSize(0.11)
            frame2.GetXaxis().SetLabelSize(0.09)

            # misID ratio: y-axis title/labels can overlap at very small values → push title slightly outward
            if "misid" in rt:
                frame2.GetYaxis().SetTitle(ratioTitle)
                frame2.GetYaxis().SetTitleOffset(1.25)
                frame2.GetYaxis().SetLabelOffset(0.01)
                frame2.GetYaxis().SetTitleSize(0.11)
                frame2.GetYaxis().SetLabelSize(0.09)
        pad2.Modified()
        pad2.Update()

        ref_line = ROOT.TLine(xmin, 1.0, xmax, 1.0)
        CMS.cmsDrawLine(ref_line, lcolor=ROOT.kRed, lstyle=ROOT.kDotted)

        CMS.SaveCanvas(canv, os.path.join(self.outputPath, f"{name}.pdf"))

    def PlotAcceptanceVsMass(self, mcGroup="DY"):
        den = self.GetMCGroupHistScaled(mcGroup, "h_GenAcc_Denom")
        num = self.GetMCGroupHistScaled(mcGroup, "h_GenAcc_Numer")
        self._PlotNumDenWithRatio(
            name=f"{self.eraLabel}_Acceptance_vsGenMass_{mcGroup}",
            num=num,
            den=den,
            numColor=ROOT.kBlue,
            denColor=ROOT.kBlack,
            yTitle="Events",
            ratioTitle="Acceptance",
            denLegend="Denominator (full phase space)",
            numLegend="Numerator (fiducial phase space)",
        )

    def PlotEfficiencyVsMass(self, mcGroup="DY"):
        # Denominator is identical to acceptance numerator (gen fiducial)
        den = self.GetMCGroupHistScaled(mcGroup, "h_GenAcc_Numer")
        num = self.GetMCGroupHistScaled(mcGroup, "h_GenEff_Numer")
        self._PlotNumDenWithRatio(
            name=f"{self.eraLabel}_Efficiency_vsGenMass_{mcGroup}",
            num=num,
            den=den,
            numColor=ROOT.kGreen + 2,
            denColor=ROOT.kBlack,
            yTitle="Events",
            ratioTitle="Efficiency",
            denLegend="Denominator (fiducial phase space)",
            numLegend="Numerator (passing all selections)",
            # extraTopLeftLines=["Ele23Ele12, HEEP ID"],
            extraTopLeftLines=["Double25, HEEP ID"],
        )

    def PlotElecMisIdProbVsMass(self, mcGroup="DY"):
        """Plot misID probability vs dielectron mass (overlay num/den + ratio), separately for e+ and e-."""
        num_ep = self.GetMCGroupHistScaled(mcGroup, "h_ElecMisIdMass_ep")
        den_ep = self.GetMCGroupHistScaled(mcGroup, "h_ElecMatchedMass_ep")
        num_em = self.GetMCGroupHistScaled(mcGroup, "h_ElecMisIdMass_em")
        den_em = self.GetMCGroupHistScaled(mcGroup, "h_ElecMatchedMass_em")

        if (not num_ep) or (not den_ep) or (not num_em) or (not den_em):
            print(f"[PlotElecMisIdProbVsMass] Missing histograms for mcGroup={mcGroup}. Skipping.")
            return

        # e+ : overlay (misID / matched) + ratio
        self._PlotNumDenWithRatio(
            name=f"{self.eraLabel}_MisIdProb_vsMass_ep_{mcGroup}",
            num=num_ep,
            den=den_ep,
            numColor=ROOT.kBlue,
            denColor=ROOT.kBlack,
            yTitle="Events",
            ratioTitle="misID prob.",
            denLegend="Matched e^{+}",
            numLegend="Charge misID e^{+}",
            extraTopLeftLines=["e^{+}"],
            legendTextSize=0.04,
        )

        # e- : overlay (misID / matched) + ratio
        self._PlotNumDenWithRatio(
            name=f"{self.eraLabel}_MisIdProb_vsMass_em_{mcGroup}",
            num=num_em,
            den=den_em,
            numColor=ROOT.kRed,
            denColor=ROOT.kBlack,
            yTitle="Events",
            ratioTitle="misID prob.",
            denLegend="Matched e^{-}",
            numLegend="Charge misID e^{-}",
            extraTopLeftLines=["e^{-}"],
            legendTextSize=0.04,
        )


def main():
    if str(args.era).upper() in ["ALL", "RUN2"]:
        eras = ["2016_preVFP", "2016_postVFP", "2017", "2018"]
        eraLabel = "Run2"
    else:
        eras = [args.era]
        eraLabel = args.era

    plotter = Plotter(eras, eraLabel, args.root, args.outdir)

    if args.plot_misid_prob:
        plotter.PlotElecMisIdProbVsMass(args.misid_mc)

    if args.plot_acceptance:
        plotter.PlotAcceptanceVsMass(args.mc)

    if args.plot_efficiency:
        plotter.PlotEfficiencyVsMass(args.mc)

    if (not args.plot_misid_prob) and (not args.plot_acceptance) and (not args.plot_efficiency):
        print("Nothing to do. Use --plot_misid_prob and/or --plot_acceptance and/or --plot_efficiency.")


if __name__ == "__main__":
    ROOT.TH1.AddDirectory(False)
    main()
