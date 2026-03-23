"""
Convert electron trigger efficiency SF ROOT files (TH2D "sf")
to correctionlib schema v2 JSON.

Structure:
  - One JSON file per era containing TWO corrections: Leg1 and Leg2
  - Inputs: (eta [signed SCEta], pt, "nominal")
  - Output: SF value from the "sf" histogram

Usage:
  python3 convert_root_to_json.py
"""

import uproot
import numpy as np
import json
import math
import os

ERAS = {
    "2016_preVFP":  "2016_preVFP",
    "2016_postVFP": "2016_postVFP",
    "2017":         "2017",
    "2018":         "2018",
}

BASE = os.path.dirname(os.path.abspath(__file__))


def read_th2d(root_path: str, hist_name: str):
    """Return (values 2-D array, x_edges, y_edges) excluding flow bins."""
    f = uproot.open(root_path)
    h = f[hist_name]
    result = h.to_numpy()
    vals = result[0]   # shape (nx, ny)
    xedges = result[1] # x (eta) bin edges, length nx+1
    yedges = result[2] # y (pt)  bin edges, length ny+1
    return vals, xedges, yedges


def round_edge(v: float) -> float:
    """Round floating-point edge to 2 decimal places."""
    return round(float(v), 2)


def build_binning_node(vals, xedges, yedges):
    """
    Build a correctionlib binning tree (eta outer, pt inner).
    Each (eta_bin, pt_bin) leaf is a category node with key "nominal".
    """
    nx = vals.shape[0]   # number of eta bins
    ny = vals.shape[1]   # number of pt bins

    x_edges_clean = [round_edge(e) for e in xedges]
    y_edges_clean = [round_edge(e) for e in yedges]
    # Last pt bin extends to infinity
    y_edges_clean[-1] = "Infinity"

    pt_bins_list = []
    for i in range(nx):
        pt_content = []
        for j in range(ny):
            sf_val = float(vals[i, j])
            pt_content.append({
                "nodetype": "category",
                "input": "scale_factors",
                "content": [
                    {"key": "nominal", "value": sf_val}
                ]
            })
        pt_bins_list.append({
            "nodetype": "binning",
            "input": "pt",
            "edges": y_edges_clean,
            "content": pt_content,
            "flow": "clamp"
        })

    return {
        "nodetype": "binning",
        "input": "eta",
        "edges": x_edges_clean,
        "content": pt_bins_list,
        "flow": "clamp"
    }


def make_correction(name: str, description: str, data_node: dict) -> dict:
    return {
        "name": name,
        "description": description,
        "version": 1,
        "inputs": [
            {"name": "eta",           "type": "real",   "description": "Probe SCEta (signed)"},
            {"name": "pt",            "type": "real",   "description": "Probe pt"},
            {"name": "scale_factors", "type": "string", "description": "Choose scale factor type"}
        ],
        "output": {
            "name": "weight",
            "type": "real",
            "description": "Scale factor"
        },
        "data": data_node
    }


def convert_era(era: str):
    era_dir = os.path.join(BASE, era)
    out_path = os.path.join(era_dir, f"elTrig_{era}.json")

    corrections = []
    for leg, label in [("Leg1", "leading"), ("Leg2", "subleading")]:
        root_path = os.path.join(era_dir, f"eff_el_TRIG_{leg}.root")
        vals, xedges, yedges = read_th2d(root_path, "sf")
        data_node = build_binning_node(vals, xedges, yedges)
        corr = make_correction(
            name=f"ElectronTrigger_{leg}",
            description=f"Electron trigger efficiency SF for {leg} ({label} electron, era {era})",
            data_node=data_node
        )
        corrections.append(corr)
        print(f"  {era} {leg}: eta bins={vals.shape[0]}, pt bins={vals.shape[1]}")

    payload = {
        "schema_version": 2,
        "description": f"Electron double-trigger efficiency scale factors for era {era}. "
                       "Leg1 applies to the leading electron, Leg2 to the sub-leading electron. "
                       "Event trigger SF = Leg1_SF * Leg2_SF.",
        "corrections": corrections
    }

    # Write JSON with Infinity as a bare keyword (correctionlib requires this)
    json_str = json.dumps(payload, indent=4)
    json_str = json_str.replace('"Infinity"', 'Infinity')

    with open(out_path, "w") as f:
        f.write(json_str)
    print(f"  -> Written: {out_path}")


def main():
    print("=== Converting electron trigger ROOT files to correctionlib JSON ===")
    for era in ERAS:
        print(f"\nEra: {era}")
        convert_era(era)
    print("\nDone.")


if __name__ == "__main__":
    main()
