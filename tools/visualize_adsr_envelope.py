"""Plot ADSR envelope gain values from CSV files for visual verification.

The CSV samples are expected to come from
score-audio-studio/packages/dsp-core/tests/AdsrEnvelopeTest.cpp.

Script:
    tools/visualize_adsr_envelope.py

Inputs:
    score-audio-studio/packages/dsp-core/tests/data/Adsr-Envelope/
    ├── Linear
    │   └── A:[attack_secs]-D:[decay_secs]-S:[sustain_secs]-R:[release_secs].csv
    └── Exponential
        └── A:[attack_secs]-D:[decay_secs]-S:[sustain_secs]-R:[release_secs].csv

Outputs:
    score-audio-studio/docs/Adsr-Envelope/
    ├── Linear
    │   └── A:[attack_secs]-D:[decay_secs]-S:[sustain_secs]-R:[release_secs].png
    └── Exponential
        └── A:[attack_secs]-D:[decay_secs]-S:[sustain_secs]-R:[release_secs].png
"""
import argparse
import sys
from pathlib import Path

PJT_ROOT = Path(__file__).resolve().parents[1]

PARENTS_DIRS = {
    'INPUT': f"{PJT_ROOT}/packages/dsp-core/tests/data/Adsr-Envelope/",
    'OUTPUT': f"{PJT_ROOT}/docs/Adsr-Envelope/"
}

CHILDERN_DIRS = [
    "Linear",
    "Exponential"
]

def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Manage ADSR envelope test data"
            "visualization images."
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=\
        """
        examples:
            python tools/visualize_adsr_envelope.py --clear-inputs
            python tools/visualize_adsr_envelope.py --clear-outputs
            python tools/visualize_adsr_envelope.py --generate-inputs
            python tools/visualize_adsr_envelope.py --generate-outputs
        """
    )
    actions = parser.add_mutually_exclusive_group(required=True)
    actions.add_argument(
        '-ci', '--clear-inputs',
        action='store_true',
        help="Delete ADSR envelope input CSV files"
    )
    actions.add_argument(
        '-co', '--clear-outputs',
        action='store_true',
        help="Delete ADSR envelope output PNG files"
    )
    actions.add_argument(
        '-gi', '--generate-inputs',
        action='store_true',
        help="Execute ADSR envelope binary to generate inputs"
    )
    actions.add_argument(
        '-go', '--generate-outputs',
        action='store_true',
        help="Generate PNG plots from the all CSV files"
    )
    return parser


def clear_generated_files(mode: str) -> int:
    if mode != 'INPUT' and mode != 'OUTPUT':
        print("something went wrong", file=sys.stderr)
        return 1

    # set a root directory
    pattern = PARENTS_DIRS[mode]
    root = Path(pattern)

    # validate the root directory
    if not root.is_dir():
        print(
            f'Directory "{root.absolute()}" does not exist.',
            file=sys.stderr
        )
        return 1

    dirs = [p for p in root.iterdir() if p.is_dir()]

    for d in dirs:
        files = [f for f in d.iterdir() if f.is_file()]
        if not files:
            print(f'Directory "{d} is already empty.')
            continue
        print(f'Command "rm {d}/*" is executing.')
        for f in files:
            # something is wrong if there is other type file in designated directory
            file_path = Path(f)
            if (
                mode == 'INPUT' and file_path.suffix.lower() != '.csv'
                or mode == 'OUTPUT' and file_path.suffix.lower() != '.png'
            ):
                print(
                    f'Unexpected file is found: "{file_path}"',
                    file=sys.stderr
                )
                return 1
            file_path.unlink()
            print(f'    The csv format file "{file_path}" is removed.')

        # check if there is no file in designated directory
        flag = len([f for f in d.iterdir() if f.is_file()]) == 0
        if not flag:
            print(
                f'Directory "{d}" is not empty after cleanup.',
                file=sys.stderr
            )
            return 1
        print(f'Command "rm {d}/*" is executed.')
        print()
    return 0


def generate_inputs() -> int:
    import subprocess

    # verify the test code first.
    subprocess.run(
        ["bash", str(PJT_ROOT / "tools/verify-dsp-core.sh")],
        cwd=PJT_ROOT,
        check=True,
    )
    # run the test code with a parameter "--generate-inputs".
    oscillator_test = (PJT_ROOT / "build/dsp-core/tests/adsr_envelope_test")
    subprocess.run(
        [str(oscillator_test), "--generate-inputs"],
        cwd=PJT_ROOT,
        check=True,
    )
    return 0


def generate_outputs() -> int:
    import numpy as np
    import pandas as pd
    import matplotlib.pyplot as plt

    # set a root directory
    pattern = PARENTS_DIRS['INPUT']
    root = Path(pattern)

    # validate the root directory
    if not root.is_dir():
        print(
            f'Directory "{root.absolute()}" does not exist.',
            file=sys.stderr
        )
        return 1

    # make a input directory list
    input_dirs = [root / d for d in CHILDERN_DIRS]

    for input_dir in input_dirs:
        input_dir.mkdir(parents=True, exist_ok=True)
        """
        Match the envelope curves generated by each C++ test.
        """
        target_csvs = sorted([
            p for p in input_dir.iterdir()
            if p.is_file()
            and
            p.suffix.lower() == ".csv"
        ])
        if not target_csvs:
            print(f'Directory "{input_dir}" is empty.')
            continue

        curve = input_dir.name
        for target_csv in target_csvs:
            fig = None
            try:
                metadata = read_metadata(target_csv)
                required_metadata = {
                    "sample_rate",
                    "curve",
                    "attack_seconds",
                    "attack_target",
                    "decay_seconds",
                    "decay_target",
                    "sustain_seconds",
                    "sustain_target",
                    "release_seconds",
                    "release_target",
                }
                missing_metadata = required_metadata - metadata.keys()
                if missing_metadata:
                    missing = ", ".join(sorted(missing_metadata))
                    raise ValueError(
                        f'Missing metadata in "{target_csv}": {missing}'
                    )

                curve = metadata["curve"]
                if curve != input_dir.name:
                    raise ValueError(
                        f'Curve metadata "{curve}" does not match '
                        f'directory "{input_dir.name}"'
                    )

                samples = pd.read_csv(target_csv, comment="#")
                gains = pd.to_numeric(samples['gains'], errors='raise')

                output_dir = Path(PARENTS_DIRS['OUTPUT']) / input_dir.name
                output_dir.mkdir(parents=True, exist_ok=True)
                output_file = output_dir / f"{target_csv.stem}.png"

                fig, ax = plt.subplots(figsize=(10, 5))

                sample_rate = float(metadata["sample_rate"])
                times = np.arange(len(gains)) / sample_rate
                ax.plot(times, gains, label="gain", linewidth=2)

                stages = [
                    (
                        "Attack",
                        float(metadata["attack_seconds"]),
                        float(metadata["attack_target"]),
                        "tab:red",
                    ),
                    (
                        "Decay",
                        float(metadata["decay_seconds"]),
                        float(metadata["decay_target"]),
                        "tab:orange",
                    ),
                    (
                        "Sustain",
                        float(metadata["sustain_seconds"]),
                        float(metadata["sustain_target"]),
                        "tab:green",
                    ),
                    (
                        "Release",
                        float(metadata["release_seconds"]),
                        float(metadata["release_target"]),
                        "tab:blue",
                    ),
                ]

                stage_start = 0.0
                for name, duration, _target, color in stages:
                    stage_end = stage_start + duration
                    if stage_start < stage_end:
                        ax.axvspan(
                            stage_start,
                            stage_end,
                            color=color,
                            alpha=0.08,
                            label=name,
                        )
                    ax.axvline(
                        stage_end,
                        color=color,
                        linestyle="--",
                        linewidth=1,
                    )
                    stage_start = stage_end

                stage_summary = " | ".join(
                    f"{name[0]} {duration * 1000:g} ms → {target:g}"
                    for name, duration, target, _color in stages
                )

                ax.set(
                    title=f"ADSR Envelope - {curve}\n{stage_summary}",
                    xlabel="Time (s)",
                    ylabel="Gains"
                )
                ax.set_ylim(-0.05, 1.05)
                ax.grid(alpha=0.25)
                ax.legend()
                fig.savefig(output_file, dpi=300, bbox_inches='tight')

            except (
                OSError,
                ValueError,
                pd.errors.EmptyDataError,
                pd.errors.ParserError
            ) as exc:
                print(
                    f"Failed to generate {curve} graph: {exc}",
                    file=sys.stderr
                )
                return 1

            finally:
                if fig is not None:
                    plt.close(fig)

    return 0

def read_metadata(csv_path: Path) -> dict[str, str]:
    metadata = {}
    with csv_path.open(encoding='utf-8') as f:
        for line in f:
            if line.startswith('#'):
                key, value = line[1:].strip().split('=', 1)
                metadata[key.strip()] = value.strip()
            else:
                break
    return metadata

def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    if args.clear_inputs:
        res = clear_generated_files('INPUT')
        if res != 0:
            return res
    elif args.clear_outputs:
        res = clear_generated_files('OUTPUT')
        if res != 0:
            return res
    elif args.generate_inputs:
        res = generate_inputs()
        if res != 0:
            return res
    elif args.generate_outputs:
        res = generate_outputs()
        if res != 0:
            return res
    return 0


if __name__ == "__main__":
    sys.exit(main())
