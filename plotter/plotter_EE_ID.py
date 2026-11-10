#!/usr/bin/env python3

import argparse
import os
import sys
import uuid
from collections import OrderedDict

import ROOT
import cmsstyle as CMS


CMS.SetExtraText("Private work")
CMS.SetEnergy("13")
ROOT.gROOT.SetBatch(ROOT.kTRUE)
ROOT.TH1.AddDirectory(False)


ERAS = OrderedDict([
    ("2016_preVFP", "2016 preVFP"),
    ("2016_postVFP", "2016 postVFP"),
    ("2017", "2017"),
    ("2018", "2018"),
])


def parse_args():
    parser = argparse.ArgumentParser(
        description="Overlay selected electron cutBased ID index for one EE ROOT output."
    )
    parser.add_argument(
        "root_path",
        nargs="?",
        default="./ROOT/260622_KP_OS_inverted/output.root",
        help="Input ROOT file, e.g. ./ROOT/<tag>/output.root.",
    )
    parser.add_argument(
        "--source",
        choices=["data", "mc", "both"],
        default="both",
        help="Histogram source to draw. MC is lumi-normalized total MC.",
    )
    parser.add_argument(
        "--no-normalize",
        dest="normalize",
        action="store_false",
        help="Draw raw event counts instead of unit-area normalized shapes.",
    )
    parser.set_defaults(normalize=True)
    parser.add_argument(
        "--output-dir",
        default=None,
        help="Directory where output plots are written. Default: ./plots/<input tag>/ID.",
    )
    parser.add_argument(
        "--label",
        default=None,
        help="Region label shown on the plot. Default is inferred from the input path.",
    )
    parser.add_argument(
        "--formats",
        nargs="+",
        default=["pdf"],
        help="Output formats passed to CMS.SaveCanvas, e.g. pdf png.",
    )

    return parser.parse_args()


def load_plotter_metadata():
    """Reuse MC lists, cross sections, and luminosities from the main EE plotter."""
    saved_argv = sys.argv[:]
    try:
        sys.argv = [sys.argv[0]]
        import plotter_EE as base_plotter
    finally:
        sys.argv = saved_argv

    return base_plotter.mcList, base_plotter.xSec, base_plotter.refLumi


def get_cms_colors():
    try:
        colors = [
            color if isinstance(color, int) else ROOT.TColor.GetColor(color)
            for color in list(CMS.petroff_8)
        ]
        if len(colors) >= len(ERAS):
            return colors
    except Exception:
        pass

    return [ROOT.kBlack, ROOT.kRed + 1, ROOT.kBlue + 1, ROOT.kGreen + 2]


def get_input_tag(root_path):
    normalized = os.path.normpath(root_path)
    parent = os.path.basename(os.path.dirname(normalized))
    if parent and parent != ".":
        return parent

    filename = os.path.basename(normalized)
    return os.path.splitext(filename)[0]


def get_default_output_dir(root_path):
    return os.path.join(".", "plots", get_input_tag(root_path), "ID")


def infer_region_label(root_path):
    tag = get_input_tag(root_path)
    lower_tag = tag.lower()

    charge = "OS" if "_os_" in lower_tag else "SS" if "_ss_" in lower_tag else ""
    region = "inverted" if "inverted" in lower_tag else "nominal" if "nominal" in lower_tag else ""

    if charge and region:
        return f"{charge} {region}"
    if charge:
        return charge
    if region:
        return region
    return tag


def clone_hist(hist, name):
    if not hist:
        return None

    cloned = hist.Clone(name)
    cloned.SetDirectory(0)
    return cloned


def get_data_hist(root_file, era):
    hist = root_file.Get(f"{era}/Data/h_ElecID")
    return clone_hist(hist, f"h_ElecID_data_{era}_{uuid.uuid4()}")


def get_mc_hist(root_file, era, mc_list, xsec, ref_lumi):
    total = None
    lumi = ref_lumi[era]

    for sample in mc_list:
        hist = root_file.Get(f"{era}/{sample}/h_ElecID")
        event_info = root_file.Get(f"{era}/{sample}/h_EventInfo")
        if not hist or not event_info:
            continue

        n_event = event_info.GetBinContent(4)
        if n_event <= 0 or sample not in xsec:
            continue

        scaled = clone_hist(hist, f"h_ElecID_{sample}_{era}_{uuid.uuid4()}")
        scaled.Scale((1000. * lumi * xsec[sample]) / n_event)

        if total is None:
            total = scaled.Clone(f"h_ElecID_mc_{era}_{uuid.uuid4()}")
            total.SetDirectory(0)
        else:
            total.Add(scaled)

    return total


def prepare_hist(hist, color, marker, normalize):
    if normalize and hist.Integral() > 0:
        hist.Scale(1. / hist.Integral())

    hist.SetLineColor(color)
    hist.SetMarkerColor(color)
    hist.SetMarkerStyle(marker)
    hist.SetMarkerSize(1.1)
    hist.SetLineWidth(3)
    hist.GetXaxis().SetNdivisions(505)


def count_id_regions(hist):
    below_medium = 0.
    medium_or_tighter = 0.

    for bin_idx in range(1, hist.GetNbinsX() + 1):
        bin_center = hist.GetBinCenter(bin_idx)
        content = hist.GetBinContent(bin_idx)

        if bin_center < 3:
            below_medium += content
        else:
            medium_or_tighter += content

    return below_medium, medium_or_tighter


def format_count(value):
    if abs(value - round(value)) < 1e-6:
        return f"{int(round(value))}"
    return f"{value:.1f}"


def tune_axis_text(pad):
    for obj in pad.GetListOfPrimitives():
        try:
            if not obj.InheritsFrom("TH1"):
                continue
        except Exception:
            continue

        obj.GetXaxis().SetLabelSize(0.036)
        obj.GetXaxis().SetTitleSize(0.042)
        obj.GetXaxis().SetTitleOffset(1.25)
        obj.GetYaxis().SetLabelSize(0.036)
        obj.GetYaxis().SetTitleSize(0.042)
        obj.GetYaxis().SetTitleOffset(1.35)


def get_histograms(root_path, source, normalize, mc_list, xsec, ref_lumi):
    root_file = ROOT.TFile.Open(root_path, "READ")
    if not root_file or root_file.IsZombie():
        raise RuntimeError(f"Cannot open ROOT file: {root_path}")

    histograms = []
    colors = get_cms_colors()
    markers = [20, 21, 22, 23]

    for idx, (era, era_label) in enumerate(ERAS.items()):
        if source == "data":
            hist = get_data_hist(root_file, era)
        else:
            hist = get_mc_hist(root_file, era, mc_list, xsec, ref_lumi)

        if not hist:
            print(f"[warning] Missing h_ElecID for {source}, era={era}, file={root_path}")
            continue

        below_medium, medium_or_tighter = count_id_regions(hist)
        prepare_hist(hist, colors[idx], markers[idx], normalize)
        histograms.append((era_label, hist, below_medium, medium_or_tighter))

    root_file.Close()
    return histograms


def draw_input(root_path, region_label, source, normalize, output_dir, formats, mc_list, xsec, ref_lumi):
    histograms = get_histograms(root_path, source, normalize, mc_list, xsec, ref_lumi)
    if not histograms:
        print(f"[warning] No histograms found for {root_path} ({source}); skipping.")
        return

    y_max = max(hist.GetBinContent(hist.GetMaximumBin()) for _, hist, _, _ in histograms)
    y_max = max(y_max * 1.45, 1.0)
    y_title = "Number of electrons (norm)" if normalize else "Electrons"

    CMS.SetLumi("")
    canvas = CMS.cmsCanvas(
        f"Canv_h_ElecID_{get_input_tag(root_path)}_{source}",
        -0.5,
        4.5,
        0.,
        y_max,
        "Electron cutBased ID index",
        y_title,
        square=True,
        extraSpace=0.05,
        iPos=0,
        yTitOffset=1.25,
    )

    canvas.cd(1)
    legend = CMS.cmsLeg(0.66, 0.68, 0.89, 0.88, textSize=0.035)

    for idx, (era_label, hist, _, _) in enumerate(histograms):
        draw_option = "P" if idx == 0 else "P SAME"
        CMS.cmsDraw(hist, draw_option, mcolor=hist.GetMarkerColor(), lcolor=hist.GetLineColor())
        legend.AddEntry(hist, era_label, "lp")

    latex_items = [
        region_label,
        "Data" if source == "data" else "Total MC",
    ]

    latex = ROOT.TLatex()
    latex.SetTextAlign(14)
    latex.SetTextSize(0.04)
    latex.SetTextFont(42)
    for idx, item in enumerate(latex_items):
        latex.DrawLatexNDC(0.20, 0.86 - idx * 0.065, item.encode("utf-8"))

    if "inverted" in region_label.lower():
        count_latex = ROOT.TLatex()
        count_latex.SetTextAlign(12)
        count_latex.SetTextSize(0.026)
        count_latex.SetTextFont(42)

        y_start = 0.70
        for idx, (_, hist, below_medium, medium_or_tighter) in enumerate(histograms):
            count_latex.SetTextColor(hist.GetLineColor())
            count_latex.DrawLatexNDC(
                0.20,
                y_start - idx * 0.045,
                f"ID fail={format_count(below_medium)}, ID pass={format_count(medium_or_tighter)}".encode("utf-8"),
            )
        count_latex.SetTextColor(ROOT.kBlack)

    tune_axis_text(canvas.cd(1))
    canvas.cd(1).Modified()
    canvas.cd(1).Update()

    os.makedirs(output_dir, exist_ok=True)
    for fmt in formats:
        output_name = f"h_ElecID_{get_input_tag(root_path)}_{source}"
        if normalize:
            output_name += "_normalized"
        CMS.SaveCanvas(canvas, os.path.join(output_dir, f"{output_name}.{fmt}"))


def main():
    args = parse_args()
    mc_list, xsec, ref_lumi = load_plotter_metadata()

    output_dir = args.output_dir if args.output_dir else get_default_output_dir(args.root_path)
    region_label = args.label if args.label else infer_region_label(args.root_path)
    sources = ["data", "mc"] if args.source == "both" else [args.source]

    for source in sources:
        draw_input(
            args.root_path,
            region_label,
            source,
            args.normalize,
            output_dir,
            args.formats,
            mc_list,
            xsec,
            ref_lumi,
        )


if __name__ == "__main__":
    main()
