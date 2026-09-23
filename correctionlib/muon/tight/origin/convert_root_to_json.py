"""Convert Muon POG Tight ID, isolation, and single-trigger ROOT payloads.

The output contains one ID/ISO and one trigger correctionlib schema-v2 JSON
file per era. Nominal, statistical, and systematic values are preserved.

Usage:
  python3 convert_root_to_json.py
  python3 convert_root_to_json.py --input-dir /path/to/HighBoostedZ/EffSF
"""

import argparse
import json
from pathlib import Path

import numpy as np
import uproot


ERAS = {
    "2016_preVFP": "Run2016_UL_preVFP",
    "2016_postVFP": "Run2016_UL_postVFP",
    "2017": "Run2017_UL",
    "2018": "Run2018_UL",
}

CORRECTIONS = {
    "ID": "NUM_TightID_DEN_TrackerMuons_abseta_pt",
    "ISO": "NUM_TightRelIso_DEN_TightIDandIPCut_abseta_pt",
}

TRIGGERS = {
    "2016_preVFP": "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight_abseta_pt",
    "2016_postVFP": "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight_abseta_pt",
    "2017": "NUM_IsoMu27_DEN_CutBasedIdTight_and_PFIsoTight_abseta_pt",
    "2018": "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight_abseta_pt",
}

BASE = Path(__file__).resolve().parent
DEFAULT_INPUT_DIR = (
    BASE.parents[6] / "HighBoostedZ" / "Validation" / "HighBoostedZ" / "EffSF"
)
DEFAULT_OUTPUT_DIR = BASE.parent


def read_histograms(root_path: Path, hist_name: str):
    """Read nominal/stat/syst TH2 values and their unmodified bin edges."""
    arrays = {}
    xedges = None
    yedges = None

    with uproot.open(root_path) as root_file:
        for variation in ("nominal", "stat", "syst"):
            key = hist_name if variation == "nominal" else f"{hist_name}_{variation}"
            values, current_xedges, current_yedges = root_file[key].to_numpy(
                flow=False
            )

            if xedges is None:
                xedges = current_xedges
                yedges = current_yedges
            elif not (
                np.array_equal(xedges, current_xedges)
                and np.array_equal(yedges, current_yedges)
            ):
                raise ValueError(f"Inconsistent binning in {root_path}: {key}")

            arrays[variation] = values

    return arrays, xedges, yedges


def build_binning_node(arrays, xedges, yedges):
    """Build an |eta|-outer, pT-inner correctionlib binning tree."""
    eta_content = []

    for eta_index in range(len(xedges) - 1):
        pt_content = []
        for pt_index in range(len(yedges) - 1):
            pt_content.append(
                {
                    "nodetype": "category",
                    "input": "scale_factors",
                    "content": [
                        {
                            "key": variation,
                            "value": float(values[eta_index, pt_index]),
                        }
                        for variation, values in arrays.items()
                    ],
                }
            )

        eta_content.append(
            {
                "nodetype": "binning",
                "input": "pt",
                "edges": [float(edge) for edge in yedges],
                "content": pt_content,
                "flow": "clamp",
            }
        )

    return {
        "nodetype": "binning",
        "input": "abseta",
        "edges": [float(edge) for edge in xedges],
        "content": eta_content,
        "flow": "clamp",
    }


def make_correction(name: str, era: str, data_node: dict) -> dict:
    return {
        "name": name,
        "description": f"Muon POG scale factor for {name}, era {era}",
        "version": 1,
        "inputs": [
            {
                "name": "abseta",
                "type": "real",
                "description": "Absolute value of the muon eta",
            },
            {"name": "pt", "type": "real", "description": "Muon pt"},
            {
                "name": "scale_factors",
                "type": "string",
                "description": "nominal, stat, or syst",
            },
        ],
        "output": {
            "name": "weight",
            "type": "real",
            "description": "Scale factor or uncertainty",
        },
        "data": data_node,
    }


def write_payload(output_path: Path, description: str, corrections: list):
    payload = {
        "schema_version": 2,
        "description": description,
        "corrections": corrections,
    }

    with output_path.open("w", encoding="utf-8") as output_file:
        json.dump(payload, output_file, indent=2, allow_nan=False)
        output_file.write("\n")
    print(f"  -> Written: {output_path}")


def convert_era(era: str, file_prefix: str, input_dir: Path, output_dir: Path):
    id_iso_corrections = []

    for correction_type, hist_name in CORRECTIONS.items():
        root_path = input_dir / f"{file_prefix}_{correction_type}.root"
        arrays, xedges, yedges = read_histograms(root_path, hist_name)
        id_iso_corrections.append(
            make_correction(
                hist_name,
                era,
                build_binning_node(arrays, xedges, yedges),
            )
        )
        print(
            f"  {correction_type}: eta bins={len(xedges) - 1}, "
            f"pt bins={len(yedges) - 1}"
        )

    output_dir.mkdir(parents=True, exist_ok=True)
    write_payload(
        output_dir / f"ScaleFactors_Muon_IDISO_{era}.json",
        f"Muon Tight ID and Tight relative-isolation scale factors for {era}",
        id_iso_corrections,
    )

    trigger_corrections = []
    trigger_base_name = TRIGGERS[era]
    trigger_path = input_dir / f"{file_prefix}_Trigger.root"
    for sample_type in ("Data", "MC"):
        hist_name = f"{trigger_base_name}_efficiency{sample_type}"
        arrays, xedges, yedges = read_histograms(trigger_path, hist_name)
        trigger_corrections.append(
            make_correction(
                hist_name,
                era,
                build_binning_node(arrays, xedges, yedges),
            )
        )
        print(
            f"  Trigger {sample_type}: eta bins={len(xedges) - 1}, "
            f"pt bins={len(yedges) - 1}"
        )

    write_payload(
        output_dir / f"ScaleFactors_Muon_Trigger_{era}.json",
        f"Muon single-trigger Data and MC efficiencies for {era}",
        trigger_corrections,
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--input-dir",
        type=Path,
        default=DEFAULT_INPUT_DIR,
        help="Directory containing the Muon POG ROOT files",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=DEFAULT_OUTPUT_DIR,
        help="Directory for generated correctionlib JSON files",
    )
    args = parser.parse_args()

    print("=== Converting muon ID/ISO/trigger ROOT files to correctionlib JSON ===")
    for era, file_prefix in ERAS.items():
        print(f"\n{era}")
        convert_era(era, file_prefix, args.input_dir, args.output_dir)
    print("\nDone.")


if __name__ == "__main__":
    main()
