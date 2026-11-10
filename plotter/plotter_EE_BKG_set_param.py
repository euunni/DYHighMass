#!/usr/bin/env python3

import argparse
import math
import os
import uuid

import ROOT
import cmsstyle as CMS
import plotterEngine_EE as plotterEngine


ROOT.gROOT.SetBatch(ROOT.kTRUE)
CMS.SetExtraText("Private work")
CMS.SetEnergy("13")

DEFAULT_ERAS = ["2016_preVFP", "2016_postVFP", "2017", "2018"]
DEFAULT_CASES = ["", "_0BJ", "_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"]
DEFAULT_FIT_OUTPUT_ROOT = "./plots/BKG/EE_FAKE_METFit.root"
DEFAULT_FAKE_OUTPUT_ROOT = "./plots/BKG/EE_FAKE.root"

# Keep this identical to plotter_EE_BKG_set.py::mcList.
MC_LIST = [
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

CASE_LABELS = {
    "": "inclusive",
    "_0BJ": "b-veto",
    "_bVeto_0J": "b-veto, N(jet) = 0",
    "_bVeto_1J": "b-veto, N(jet) = 1",
    "_bVeto_mt1J": "b-veto, N(jet) > 1",
}

# Per-region, per-era y-axis ranges for the pre-subtraction MET Data/MC overlay (draw_met_overlay).
MET_OVERLAY_Y_RANGES = {
    "OS_inverted": {
        # Keep 1.5 inside the pad so its guide line is visible.
        "ratio": (0.0, 1.5),
        "top": {
            "2016_preVFP": (7.0, 6e4),
            "2016_postVFP": (7.0, 5e4),
            "2017": (10.0, 1e5),
            "2018": (10.0, 2e5),
        },
    },
    "SS_inverted": {
        "ratio": (0.0, 1.0),
        "top": {
            "2016_preVFP": (5.0, 2.5e4),
            "2016_postVFP": (4.0, 2.5e4),
            "2017": (10.0, 5e4),
            "2018": (10.0, 8e4),
        },
    },
}

MET_XMAX = 300.0
MET_XMIN = 0.0
LOG_YMIN_FLOOR = 1.0
MASS_LOG_YMIN_FLOOR = 0.01
X_AXIS_MIN = 40
X_AXIS_MAX = 3000

Y_AXIS_RANGES = {
    # Set y-axis ranges by the main plot category.
    # FakeOSDataDriven is keyed by era for --GetFake plots.
    #
    # "FakeOSDataDriven": {
    #     "2016_preVFP": (0.01, 1.5e3),
    #     "2016_postVFP": (0.01, 5.5e3),
    #     "2017": (0.01, 2e4),
    #     "2018": (0.01, 1.5e4),
    # },
    "FakeOSDataDriven": {
        "2016_preVFP": (0.01, 3e4),
        "2016_postVFP": (0.01, 3e4),
        "2017": (0.01, 3e4),
        "2018": (0.01, 3e4),
    },
    "SStoOS": {
        "top": {
            "2016_preVFP": (1, 2e4),
            "2016_postVFP": (1, 2e4),
            "2017": (1.5, 4e4),
            "2018": (8., 6e4),
        },
        "ratio": (0.5, 2.5),
    },
    }

# All mass bin suffixes present in the ROOT file (ordered by mass).
ALL_MASS_SUFFIXES = [
    "_m39_40", "_m40_45", "_m45_50", "_m50_55", "_m55_60",
    "_m60_64", "_m64_68", "_m68_72", "_m72_76", "_m76_81",
    "_m81_86", "_m86_91", "_m91_96", "_m96_101", "_m101_106",
    "_m106_110", "_m110_115", "_m115_120", "_m120_126",
    "_m126_133", "_m133_141", "_m141_150", "_m150_160",
    "_m160_171", "_m171_185", "_m185_200", "_m200_220",
    "_m220_243", "_m243_273", "_m273_320", "_m320_380",
    "_m380_440", "_m440_510", "_m510_600", "_m600_700",
    "_m700_830", "_m830_1000", "_m1000_1500", "_m1500_4000",
    "_m4000_4001",
]

# Z peak veto: exclude (same as shears).
# Bins whose range overlaps with [61, 121] GeV are excluded.
Z_PEAK_MASS_SUFFIXES = {
    "_m60_64", "_m64_68", "_m68_72", "_m72_76", "_m76_81",
    "_m81_86", "_m86_91", "_m91_96", "_m96_101", "_m101_106",
    "_m106_110", "_m110_115", "_m115_120"
}

NON_Z_MASS_SUFFIXES = [s for s in ALL_MASS_SUFFIXES if s not in Z_PEAK_MASS_SUFFIXES]


def GetManualYRange(plotCategory, padName=None, era=None):
    manualRange = Y_AXIS_RANGES.get(plotCategory)
    if not manualRange:
        return None

    if isinstance(manualRange, dict):
        if padName is not None:
            padRange = manualRange.get(padName)
            if isinstance(padRange, dict):
                return padRange.get(era) if era is not None else None
            return padRange
        if era is not None:
            return manualRange.get(era)
        return None
    if padName is None:
        return manualRange
    return None


def GetCanvasYRange(plotCategory, hist, ymin=0.1, ymaxScale=1e3, padName=None):
    manualRange = GetManualYRange(plotCategory, padName)
    if manualRange:
        return manualRange
    return ymin, hist.GetMaximum() * ymaxScale


def GetAutoLogYRange(hist, xmin, xmax, ymin_floor=MASS_LOG_YMIN_FLOOR, ymax_scale=20.0):
    ymin = None
    ymax = 0.0
    axis = hist.GetXaxis()

    for i_bin in range(1, hist.GetNbinsX() + 1):
        bin_low = axis.GetBinLowEdge(i_bin)
        bin_high = axis.GetBinUpEdge(i_bin)
        if bin_high <= xmin or bin_low >= xmax:
            continue

        content = hist.GetBinContent(i_bin)
        if content <= 0:
            continue

        ymin = content if ymin is None else min(ymin, content)
        ymax = max(ymax, content)

    if ymin is None or ymax <= 0:
        return ymin_floor, ymin_floor * 10.0

    return max(ymin * 0.5, ymin_floor), ymax * ymax_scale


def _tune_logx_axis_labels(axis):
    if not axis:
        return
    try:
        axis.SetMoreLogLabels(True)
        axis.SetNoExponent(True)
    except Exception:
        pass


def _tune_pad_logx(pad):
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


def parse_args():
    parser = argparse.ArgumentParser(
        description="Fit EE fake OS/SS MET transfer factor or draw MET-weighted fake."
    )
    parser.add_argument("--os-inverted-root", default="./ROOT/260918_KP_OS_inverted/output.root")
    parser.add_argument("--ss-inverted-root", default="./ROOT/260918_KP_SS_inverted2/output.root")
    parser.add_argument("--ss-nominal-root", default="./ROOT/260918_KP_SS_nominal/output.root")
    parser.add_argument("--output-root", default=DEFAULT_FIT_OUTPUT_ROOT)
    parser.add_argument("--output-dir", default="./plots/BKG/EE_FAKE_METFit")
    parser.add_argument("--eras", nargs="+", default=DEFAULT_ERAS)
    parser.add_argument(
        "--cases",
        nargs="+",
        default=DEFAULT_CASES,
        help="Use 'inclusive' for the no-suffix case.",
    )
    parser.add_argument(
        "--GetFake",
        action="store_true",
        help="Draw OS fake prediction from SS nominal h_dielecMass_SStoOSWeighted.",
    )
    parser.add_argument(
        "--overlay-only",
        action="store_true",
        help="Only draw the pre-subtraction MET Data/MC overlay plots; skip the OS/SS ratio fit.",
    )
    parser.add_argument(
        "--no-z-veto",
        action="store_true",
        help="TEMP diagnostic: disable the Z-peak mass bin rejection (include all mass bins).",
    )
    parser.add_argument("--fit-min", type=float, default=MET_XMIN)
    parser.add_argument("--fit-max", type=float, default=MET_XMAX)
    parser.add_argument("--x-max", type=float, default=MET_XMAX, help="Maximum MET shown on plots.")
    parser.add_argument("--mass-x-min", type=float, default=40.0)
    parser.add_argument("--mass-x-max", type=float, default=3000.0)
    return parser.parse_args()


def normalize_cases(cases):
    return ["" if case == "inclusive" else case for case in cases]


def hist_path(base_name, case):
    return f"{case}/{base_name}{case}" if case else base_name


def clone_empty_like(hist, name):
    clone = hist.Clone(name)
    clone.Reset("ICES")
    clone.SetDirectory(0)
    clone.SetStats(0)
    return clone


def subtract_mc(data_hist, mc_hist, name):
    fake = data_hist.Clone(name)
    fake.SetDirectory(0)
    fake.Add(mc_hist, -1.0)
    for i_bin in range(1, fake.GetNbinsX() + 1):
        if fake.GetBinContent(i_bin) <= 0:
            fake.SetBinContent(i_bin, 0.0)
            fake.SetBinError(i_bin, 0.0)
    return fake


def get_met_z_veto(plotter, case, mc_list, label):
    """Return Data-MC MET histogram summed over all mass bins outside the Z peak.

    For each mass suffix in NON_Z_MASS_SUFFIXES the combined jet+mass histogram
    path is  ``{case}{mass_suffix}/{base_name}{case}{mass_suffix}``.
    The resulting histograms are summed bin-by-bin.
    """
    combined_data = None
    combined_mc = None

    for mass_suffix in NON_Z_MASS_SUFFIXES:
        combined_suffix = case + mass_suffix
        met_path = hist_path("h_MET", combined_suffix)

        try:
            data_hist = plotter.GetDataHist(met_path)
            mc_hist = plotter.GetMCHist(met_path, mc_list)
        except Exception:
            continue

        if data_hist is None or mc_hist is None:
            continue

        data_hist.SetDirectory(0)
        mc_hist.SetDirectory(0)

        if combined_data is None:
            combined_data = data_hist.Clone(f"data_zVeto_{label}_{uuid.uuid4()}")
            combined_data.SetDirectory(0)
            combined_mc = mc_hist.Clone(f"mc_zVeto_{label}_{uuid.uuid4()}")
            combined_mc.SetDirectory(0)
        else:
            combined_data.Add(data_hist)
            combined_mc.Add(mc_hist)

    if combined_data is None:
        raise RuntimeError(f"No Z-veto MET histograms found for case='{case}'")

    return subtract_mc(combined_data, combined_mc, f"fake_zVeto_{label}_{uuid.uuid4()}")


def sum_met_over_massbins(case, fetch, label):
    """Sum a histogram fetched per mass bin (via ``fetch(met_path)``) over NON_Z_MASS_SUFFIXES."""
    combined = None
    for mass_suffix in NON_Z_MASS_SUFFIXES:
        met_path = hist_path("h_MET", case + mass_suffix)
        try:
            hist = fetch(met_path)
        except Exception:
            continue

        if hist is None:
            continue

        hist.SetDirectory(0)
        if combined is None:
            combined = hist.Clone(f"{label}_{uuid.uuid4()}")
            combined.SetDirectory(0)
        else:
            combined.Add(hist)

    return combined


def mc_group_partition(mc_list):
    """Split mc_list into the legend groups used for the pre-subtraction MET overlay.

    Order is bottom-to-top stacking order (THStack.Add order); the legend is
    drawn top-to-bottom in the reverse of this order (Data, DY->ee, ttbar,
    WW+WZ+ZZ, Single top, DY->tautau, GG->ee), matching the reference MET plot.
    """
    groups = [
        ("#gamma#gamma#rightarrow ee", [m for m in mc_list if m.startswith("GG")]),
        ("DY#rightarrow#tau#tau", [m for m in mc_list if m == "NNLO_tautau"]),
        ("Single top", [m for m in mc_list if m.startswith("ST_")]),
        ("WW + WZ + ZZ", [m for m in mc_list if m in ("WW", "WZ", "ZZ")]),
        ("t#bar{t}", [m for m in mc_list if m == "TTTo2L2Nu"]),
        ("DY#rightarrow ee", [m for m in mc_list if m.startswith("NNLO_EE")]),
    ]
    return [(label, sub_list) for label, sub_list in groups if sub_list]


def draw_met_overlay(era, case, region_label, plotter, mc_list, output_dir, x_max):
    """Draw the pre-subtraction Data vs MC-stack MET overlay (Z-peak vetoed, same inputs as get_met_z_veto)."""
    os.makedirs(output_dir, exist_ok=True)

    data = sum_met_over_massbins(case, plotter.GetDataHist, f"data_{region_label}_{era}{case}")
    if data is None:
        return

    group_hists = {}
    for group_label, group_mc_list in mc_group_partition(mc_list):
        hist = sum_met_over_massbins(
            case,
            lambda met_path, gm=group_mc_list: plotter.GetMCHist(met_path, gm),
            f"mc_{region_label}_{era}{case}_{group_label}",
        )
        if hist is not None:
            group_hists[group_label] = hist

    if not group_hists:
        return

    total_mc = None
    for hist in group_hists.values():
        if total_mc is None:
            total_mc = hist.Clone(f"totalmc_{region_label}_{era}{case}_{uuid.uuid4()}")
            total_mc.SetDirectory(0)
        else:
            total_mc.Add(hist)

    mc_over_data = plotter.GetRatioHist(total_mc, data)

    region_ranges = MET_OVERLAY_Y_RANGES[region_label]
    ymin, ymax = region_ranges["top"][era]
    yrmin, yrmax = region_ranges["ratio"]

    dicanv = CMS.cmsDiCanvas(
        f"c_met_overlay_{region_label}_{era}{case}_{uuid.uuid4()}",
        MET_XMIN,
        x_max,
        ymin,
        ymax,
        yrmin,
        yrmax,
        "p_{T}^{miss} [GeV]",
        "Events",
        "Pred./Data",
        square=CMS.kSquare,
        extraSpace=0.1,
        iPos=0,
    )

    dicanv.cd(1)
    dicanv.cd(1).SetLogy(True)

    stack = ROOT.THStack(f"stack_{region_label}_{era}{case}_{uuid.uuid4()}", "Stacked")
    leg = CMS.cmsLeg(0.65, 0.89 - 0.05 * (len(group_hists) + 1), 0.89, 0.89, textSize=0.03)
    leg.AddEntry(data, "Data", "lp")
    CMS.cmsDrawStack(stack, leg, group_hists, invertLegendEntries=True)
    CMS.cmsDraw(data, "P", mcolor=ROOT.kBlack)
    dicanv.cd(1).RedrawAxis()

    dicanv.cd(2)
    CMS.cmsDraw(mc_over_data, "P", mcolor=ROOT.kBlack)
    ratio_guide_lines = []
    ratio_step = 0.5 if region_label.startswith("OS") else 0.2
    first_line = math.ceil(yrmin / ratio_step)
    last_line = math.floor(yrmax / ratio_step)
    for line_index in range(first_line, last_line + 1):
        y_value = line_index * ratio_step
        if y_value <= yrmin or y_value >= yrmax:
            continue
        guide_line = ROOT.TLine(MET_XMIN, y_value, x_max, y_value)
        CMS.cmsDrawLine(
            guide_line, lcolor=ROOT.kGray + 1, lstyle=ROOT.kDashed
        )
        ratio_guide_lines.append(guide_line)

    CMS.SaveCanvas(dicanv, os.path.join(output_dir, f"{era}_MET_{region_label}{case}.pdf"))
    dicanv.Close()


def get_weighted_mass_fake(plotter, case, mc_list, label):
    """Return the MET-weighted OS fake prediction from the SS nominal region."""
    hist_name = hist_path("h_dielecMass_SStoOSWeighted", case)
    data_hist = plotter.GetDataHist(hist_name)
    mc_hist = plotter.GetMCHist(hist_name, mc_list)
    return subtract_mc(data_hist, mc_hist, f"fake_metWeighted_{label}_{uuid.uuid4()}")


def build_ratio(num, den, name):
    ratio = clone_empty_like(num, name)
    for i_bin in range(1, ratio.GetNbinsX() + 1):
        n_val = num.GetBinContent(i_bin)
        d_val = den.GetBinContent(i_bin)
        if n_val <= 0 or d_val <= 0:
            continue

        n_err = num.GetBinError(i_bin)
        d_err = den.GetBinError(i_bin)
        value = n_val / d_val
        error = value * ((n_err / n_val) ** 2 + (d_err / d_val) ** 2) ** 0.5
        ratio.SetBinContent(i_bin, value)
        ratio.SetBinError(i_bin, error)
    return ratio


def make_fit(name, fit_min, fit_max):
    fit = ROOT.TF1(name, "[0]-[1]*x-exp([2]-[3]*x)", fit_min, fit_max)
    fit.SetParameters(1.2, 0.001, -0.8, 0.06)
    fit.SetParNames("P0", "P1", "P2", "P3")
    fit.SetLineColor(ROOT.kRed + 1)
    fit.SetLineWidth(2)
    return fit


def fit_ratio(ratio, era, case, fit_min, fit_max):
    fit = make_fit(f"fit_EE_SStoOS_MET_{era}{case}_{uuid.uuid4()}", fit_min, fit_max)
    result = ratio.Fit(fit, "QRS")
    status = int(result.Status()) if result else -1
    return fit, status


def get_log_y_range(hists, x_min, x_max):
    ymax = max(h.GetMaximum() for h in hists)
    ymin = LOG_YMIN_FLOOR
    for hist in hists:
        for i_bin in range(1, hist.GetNbinsX() + 1):
            if hist.GetBinLowEdge(i_bin) >= x_max:
                break
            if hist.GetBinLowEdge(i_bin + 1) <= x_min:
                continue
            content = hist.GetBinContent(i_bin)
            if content > 0:
                ymin = min(ymin, content)
    return max(ymin * 0.5, LOG_YMIN_FLOOR), ymax * 1.35


def draw_fit(era, case, os_fake, ss_fake, ratio, fit, output_dir, x_max):
    os.makedirs(output_dir, exist_ok=True)
    label = CASE_LABELS.get(case, case)

    canvas = ROOT.TCanvas(f"c_met_fit_{era}{case}", "", 800, 900)
    top_pad = ROOT.TPad(f"top_{era}{case}", "", 0.0, 0.30, 1.0, 1.0)
    bottom_pad = ROOT.TPad(f"bottom_{era}{case}", "", 0.0, 0.0, 1.0, 0.30)
    top_pad.SetBottomMargin(0.03)
    top_pad.SetLeftMargin(0.12)
    top_pad.SetTicks(1, 1)
    bottom_pad.SetTopMargin(0.03)
    bottom_pad.SetBottomMargin(0.35)
    bottom_pad.SetLeftMargin(0.12)
    bottom_pad.SetTicks(1, 1)
    top_pad.Draw()
    bottom_pad.Draw()

    top_pad.cd()
    top_pad.SetLogy(True)
    manual_sstoos_top_range = GetManualYRange("SStoOS", "top", era)
    if manual_sstoos_top_range:
        ymin, ymax = manual_sstoos_top_range
    else:
        ymin, ymax = get_log_y_range([os_fake, ss_fake], MET_XMIN, x_max)
    os_fake.SetTitle("")
    os_fake.GetXaxis().SetRangeUser(MET_XMIN, x_max)
    os_fake.GetXaxis().SetLabelSize(0)
    os_fake.GetYaxis().SetTitle("Events")
    os_fake.GetYaxis().SetTitleSize(0.05)
    os_fake.GetYaxis().SetTitleOffset(1.1)
    os_fake.GetYaxis().SetRangeUser(ymin, ymax)
    ss_fake.GetXaxis().SetRangeUser(MET_XMIN, x_max)
    os_fake.SetMarkerStyle(20)
    os_fake.SetMarkerColor(ROOT.kBlack)
    os_fake.SetLineColor(ROOT.kBlack)
    ss_fake.SetMarkerStyle(20)
    ss_fake.SetMarkerColor(ROOT.kRed + 1)
    ss_fake.SetLineColor(ROOT.kRed + 1)
    os_fake.Draw("E1")
    ss_fake.Draw("E1 SAME")

    legend = ROOT.TLegend(0.55, 0.72, 0.77, 0.86)
    legend.SetBorderSize(0)
    legend.SetFillStyle(0)
    legend.SetTextFont(42)
    legend.SetTextSize(0.035)
    legend.AddEntry(os_fake, "Data - MC, OS inverted", "lp")
    legend.AddEntry(ss_fake, "Data - MC, SS inverted", "lp")
    legend.Draw()

    if CMS.cmsStyle is None:
        CMS.setCMSStyle()
    CMS.CMS_lumi(top_pad, iPosX=0)

    latex = ROOT.TLatex()
    latex.SetNDC(True)
    latex.SetTextFont(42)
    latex.SetTextSize(0.04)
    # latex.DrawLatex(0.24, 0.88, f"{era}, ee channel")
    # latex.DrawLatex(0.24, 0.82, "Fake CR (Z veto: 61-121 GeV)")
    # latex.DrawLatex(0.24, 0.76, label)

    bottom_pad.cd()
    ratio.SetTitle("")
    ratio.GetXaxis().SetRangeUser(MET_XMIN, x_max)
    ratio.GetXaxis().SetTitle("MET [GeV]")
    ratio.GetYaxis().SetTitle("OS/SS")
    ratio.GetXaxis().SetTitleSize(0.12)
    ratio.GetXaxis().SetLabelSize(0.10)
    ratio.GetYaxis().SetTitleSize(0.10)
    ratio.GetYaxis().SetLabelSize(0.08)
    ratio.GetYaxis().SetTitleOffset(0.45)
    ratio.GetYaxis().SetNdivisions(505)
    manual_sstoos_ratio_range = GetManualYRange("SStoOS", "ratio", era)
    if manual_sstoos_ratio_range:
        ratio.GetYaxis().SetRangeUser(*manual_sstoos_ratio_range)
    else:
        ratio.GetYaxis().SetRangeUser(0.0, max(2.5, ratio.GetMaximum() * 1.4))
    ratio.SetMarkerStyle(20)
    ratio.SetMarkerColor(ROOT.kBlack)
    ratio.SetLineColor(ROOT.kBlack)
    ratio.Draw("E1")
    fit.Draw("SAME")

    fit_label = ROOT.TLatex()
    fit_label.SetNDC(True)
    fit_label.SetTextFont(42)
    fit_label.SetTextSize(0.07)
    # fit_label.DrawLatex(0.18, 0.86, f"P0={fit.GetParameter(0):.5g}, P1={fit.GetParameter(1):.5g}")
    # fit_label.DrawLatex(0.18, 0.74, f"P2={fit.GetParameter(2):.5g}, P3={fit.GetParameter(3):.5g}")

    canvas.SaveAs(os.path.join(output_dir, f"fit_R_MET_{era}{case}.pdf"))
    canvas.Close()


def draw_weighted_fake(era, case, fake_hist, output_dir, mass_x_min, mass_x_max):
    os.makedirs(output_dir, exist_ok=True)

    manual_fake_range = GetManualYRange("FakeOSDataDriven", era=era)
    if manual_fake_range:
        ymin_fake, ymax_fake = manual_fake_range
    else:
        ymin_fake, ymax_fake = GetAutoLogYRange(fake_hist, mass_x_min, mass_x_max)

    canvas = CMS.cmsCanvas(
        "Canv_EE_OS_FAKE_METWeighted",
        mass_x_min,
        mass_x_max,
        ymin_fake,
        ymax_fake,
        "M(ee) [GeV]",
        "Events",
        square=True,
        extraSpace=0.05,
        iPos=0,
        yTitOffset=1.25,
    )

    canvas.cd(1).SetLogx(True)
    canvas.cd(1).SetLogy(True)
    canvas.cd(2).SetLogx(True)

    fake_stack = ROOT.THStack("EE_OS_FAKE_METWeighted_Stack", "Stacked")
    fake_legend = CMS.cmsLeg(0.80, 0.60, 0.89, 0.70, textSize=0.03)
    fake_stack_sheet = {
        "Fake": fake_hist,
    }

    CMS.cmsDrawStack(fake_stack, fake_legend, fake_stack_sheet)
    _tune_pad_logx(canvas.cd(1))

    CMS.SaveCanvas(
        canvas,
        os.path.join(output_dir, f"plot_{era}_FakeOS_METWeighted{case}.pdf"),
    )


def process_fit_case(era, case, os_plotter, ss_plotter, output_file, args):
    overlay_dir = os.path.join(args.output_dir, f"era_{era}")
    draw_met_overlay(era, case, "OS_inverted", os_plotter, MC_LIST, overlay_dir, args.x_max)
    draw_met_overlay(era, case, "SS_inverted", ss_plotter, MC_LIST, overlay_dir, args.x_max)

    if args.overlay_only:
        return None

    # Sum MET over all mass bins outside the Z peak (|M_ee - 91.2| < 30 GeV → 61-121 GeV).
    # Matches shears' Z peak removal range of 30.0 GeV.
    os_fake = get_met_z_veto(os_plotter, case, MC_LIST, f"OS_{era}{case}")
    ss_fake = get_met_z_veto(ss_plotter, case, MC_LIST, f"SS_{era}{case}")
    ratio = build_ratio(os_fake, ss_fake, f"EE_inverted_SStoOS_MET{case}")
    fit, status = fit_ratio(ratio, era, case, args.fit_min, args.fit_max)

    dir_name = f"{era}/FAKE_EE_SStoOS_MET"
    output_file.cd()
    if not output_file.GetDirectory(dir_name):
        output_file.mkdir(dir_name)
    output_file.cd(dir_name)

    os_fake.SetName(f"EE_OS_inverted_FAKE_MET{case}")
    ss_fake.SetName(f"EE_SS_inverted_FAKE_MET{case}")
    ratio.SetName(f"EE_inverted_SStoOS_MET{case}")
    fit.SetName(f"fit_EE_inverted_SStoOS_MET{case}")

    os_fake.Write("", ROOT.TObject.kOverwrite)
    ss_fake.Write("", ROOT.TObject.kOverwrite)
    ratio.Write("", ROOT.TObject.kOverwrite)
    fit.Write("", ROOT.TObject.kOverwrite)

    draw_fit(
        era,
        case,
        os_fake,
        ss_fake,
        ratio,
        fit,
        os.path.join(args.output_dir, f"era_{era}"),
        args.x_max,
    )

    return {
        "era": era,
        "case": case if case else "inclusive",
        "status": status,
        "p0": fit.GetParameter(0),
        "p1": fit.GetParameter(1),
        "p2": fit.GetParameter(2),
        "p3": fit.GetParameter(3),
        "e0": fit.GetParError(0),
        "e1": fit.GetParError(1),
        "e2": fit.GetParError(2),
        "e3": fit.GetParError(3),
    }


def process_fake_case(era, case, ss_nominal_plotter, output_file, args):
    fake_hist = get_weighted_mass_fake(ss_nominal_plotter, case, MC_LIST, f"SS_nominal_{era}{case}")

    dir_name = f"{era}/FAKE_EE_OS"
    output_file.cd()
    if not output_file.GetDirectory(dir_name):
        output_file.mkdir(dir_name)
    output_file.cd(dir_name)

    fake_hist.SetName(f"EE_OS_FAKE_DataDriven{case}")
    fake_hist.Write("", ROOT.TObject.kOverwrite)

    draw_weighted_fake(
        era,
        case,
        fake_hist,
        os.path.join(args.output_dir, f"era_{era}"),
        args.mass_x_min,
        args.mass_x_max,
    )


def write_parameter_summary(results, output_dir):
    os.makedirs(output_dir, exist_ok=True)
    summary_path = os.path.join(output_dir, "fit_parameters.txt")
    with open(summary_path, "w", encoding="utf-8") as handle:
        handle.write("# era case status P0 P1 P2 P3 eP0 eP1 eP2 eP3\n")
        for row in results:
            handle.write(
                "{era} {case} {status} "
                "{p0:.10g} {p1:.10g} {p2:.10g} {p3:.10g} "
                "{e0:.10g} {e1:.10g} {e2:.10g} {e3:.10g}\n".format(**row)
            )

    print(f"Saved fit parameter summary: {summary_path}")
    print("\nYAML snippets from inclusive fits:")
    for row in results:
        if row["case"] != "inclusive":
            continue
        print(f"\n# {row['era']}")
        print("FakeSStoOS:")
        print("  Use: true")
        print(f"  P0: {row['p0']:.10g}")
        print(f"  P1: {row['p1']:.10g}")
        print(f"  P2: {row['p2']:.10g}")
        print(f"  P3: {row['p3']:.10g}")


def main():
    global NON_Z_MASS_SUFFIXES

    args = parse_args()
    args.cases = normalize_cases(args.cases)
    if args.GetFake and args.output_root == DEFAULT_FIT_OUTPUT_ROOT:
        args.output_root = DEFAULT_FAKE_OUTPUT_ROOT
    if args.no_z_veto:
        # TEMP diagnostic: get_met_z_veto and sum_met_over_massbins both read this
        # module-level list, so reassigning it here turns off the Z-peak rejection
        # everywhere without touching either function. Remove this block (and the
        # --no-z-veto arg above) to roll back.
        NON_Z_MASS_SUFFIXES = ALL_MASS_SUFFIXES

    os.makedirs(args.output_dir, exist_ok=True)
    output_file = ROOT.TFile(args.output_root, "RECREATE")
    results = []

    for era in args.eras:
        CMS.SetLumi(plotterEngine.refLumi[era])

        if args.GetFake:
            ss_nominal_plotter = plotterEngine.Plotter(
                era,
                rootPath=args.ss_nominal_root,
                outputPath=args.output_dir,
                channel="EE",
                region="SS",
            )
            for case in args.cases:
                process_fake_case(era, case, ss_nominal_plotter, output_file, args)
        else:
            os_plotter = plotterEngine.Plotter(
                era,
                rootPath=args.os_inverted_root,
                outputPath=args.output_dir,
                channel="EE",
                region="OS_inverted",
            )
            ss_plotter = plotterEngine.Plotter(
                era,
                rootPath=args.ss_inverted_root,
                outputPath=args.output_dir,
                channel="EE",
                region="SS_inverted",
            )

            for case in args.cases:
                result = process_fit_case(era, case, os_plotter, ss_plotter, output_file, args)
                if result is not None:
                    results.append(result)

    output_file.Close()
    if not args.GetFake and not args.overlay_only:
        write_parameter_summary(results, args.output_dir)
    print(f"Saved ROOT output: {args.output_root}")


if __name__ == "__main__":
    ROOT.TH1.AddDirectory(False)
    main()
