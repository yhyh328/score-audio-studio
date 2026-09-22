"""Plot oscillator waveform samples from CSV files for visual verification.

The CSV samples are expected to come from
score-audio-studio/packages/dsp-core/tests/OscillatorTest.cpp.

Supported waveforms: Sine, Triangle, Square, and Sawtooth.

Script:
    tools/visualize_oscillator.py

Inputs:
    score-audio-studio/packages/dsp-core/tests/data/Oscillator/
    ├── test-default-sine
    │   └── Sine_yyyymmdd_hhmmss.csv
    ├── test-default-wave-forms
    │   ├── Sine_yyyymmdd_hhmmss.csv
    │   ├── Triangle_yyyymmdd_hhmmss.csv
    │   ├── Square_yyyymmdd_hhmmss.csv
    │   └── Sawtooth_yyyymmdd_hhmmss.csv
    ├── test-harmonic-limit
    │   ├── Sine_yyyymmdd_hhmmss.csv
    │   ├── Triangle_yyyymmdd_hhmmss.csv
    │   ├── Square_yyyymmdd_hhmmss.csv
    │   └── Sawtooth_yyyymmdd_hhmmss.csv
    ├── test-aliasing-allowed
    │   ├── Sine_yyyymmdd_hhmmss.csv
    │   ├── Triangle_yyyymmdd_hhmmss.csv
    │   ├── Square_yyyymmdd_hhmmss.csv
    │   └── Sawtooth_yyyymmdd_hhmmss.csv
    └── test-harmonics-above-nyquist-are-filtered
        ├── Triangle_yyyymmdd_hhmmss.csv
        ├── Square_yyyymmdd_hhmmss.csv
        └── Sawtooth_yyyymmdd_hhmmss.csv

Outputs:
    score-audio-studio/docs/Oscillator/
    ├── test-default-sine
    │   └── Sine_yyyymmdd_hhmmss.png
    ├── test-default-wave-forms
    │   ├── Sine_yyyymmdd_hhmmss.png
    │   ├── Triangle_yyyymmdd_hhmmss.png
    │   ├── Square_yyyymmdd_hhmmss.png
    │   └── Sawtooth_yyyymmdd_hhmmss.png
    ├── test-harmonic-limit
    │   ├── Sine_yyyymmdd_hhmmss.png
    │   ├── Triangle_yyyymmdd_hhmmss.png
    │   ├── Square_yyyymmdd_hhmmss.png
    │   └── Sawtooth_yyyymmdd_hhmmss.png
    ├── test-aliasing-allowed
    │   ├── Sine_yyyymmdd_hhmmss.png
    │   ├── Triangle_yyyymmdd_hhmmss.png
    │   ├── Square_yyyymmdd_hhmmss.png
    │   └── Sawtooth_yyyymmdd_hhmmss.png
    └── test-harmonics-above-nyquist-are-filtered
        ├── Triangle_yyyymmdd_hhmmss.png
        ├── Square_yyyymmdd_hhmmss.png
        └── Sawtooth_yyyymmdd_hhmmss.png
"""
import argparse
import sys
from pathlib import Path

PJT_ROOT = Path(__file__).resolve().parents[1]

PARENTS_DIRS = {
    'INPUT' : f"{PJT_ROOT}/packages/dsp-core/tests/data/Oscillator/",
    'OUTPUT': f"{PJT_ROOT}/docs/Oscillator/"
}

CHILDERN_DIRS = [
   "test-default-sine",
   "test-default-wave-forms",
   "test-harmonic-limit",
   "test-aliasing-allowed",
   "test-harmonics-above-nyquist-are-filtered"
]

def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Manage oscillator test data and waveform "
            "visualization images."
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=\
        """
        examples:
            python tools/visualize_oscillator.py --clear-inputs
            python tools/visualize_oscillator.py --clear-outputs
            python tools/visualize_oscillator.py --generate-inputs
            python tools/visualize_oscillator.py --generate-outputs
        """
    )
    actions = parser.add_mutually_exclusive_group(required=True)
    actions.add_argument(
        '-ci', '--clear-inputs',
        action='store_true',
        help="Delete oscillator input CSV files"
    )
    actions.add_argument(
        '-co', '--clear-outputs',
        action='store_true',
        help="Delete oscillator output PNG files"
    )
    actions.add_argument(
        '-gi', '--generate-inputs',
        action='store_true',
        help="Execute oscillator binary to generate inputs"
    )
    actions.add_argument(
        '-go', '--generate-outputs',
        action='store_true',
        help="Generate PNG plots from the newest CSV files"
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

    dirs = [ p for p in root.iterdir() if p.is_dir() ]

    for d in dirs:
        files = [ f for f in d.iterdir() if f.is_file() ]
        if not files:
            print(f'Directory "{d} is already empty.')
            continue
        print(f'Command "rm {d}/*" is executing.')
        for f in files:
            # something is wrong if there is other type file in designated directory
            file_path = Path(f)
            if mode == 'INPUT' and file_path.suffix.lower() != '.csv' \
            or \
            mode == 'OUTPUT' and file_path.suffix.lower() != '.png':
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
    oscillator_test = (PJT_ROOT / "build/dsp-core/tests/oscillator_test")
    subprocess.run(
        [str(oscillator_test), "--generate-inputs"],
        cwd=PJT_ROOT,
        check=True,
    )
    return 0

def generate_outputs() -> int:
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
    input_dirs = [ root / d for d in CHILDERN_DIRS ]

    for input_dir in input_dirs:
        input_dir.mkdir(parents=True, exist_ok=True)
        """
        Match the waveforms generated by each C++ test.
        The default-sine test emits only Sine; the harmonic-filter test
        skips Sine because it has no higher harmonics to filter.
        """
        waves = ("Sine", "Triangle", "Square", "Sawtooth")
        if input_dir.name == "test-default-sine":
            waves = ("Sine",)
        elif input_dir.name == "test-harmonics-above-nyquist-are-filtered":
            waves = ("Triangle", "Square", "Sawtooth")

        for wave_type in waves:
            target_csv = setTargetCSVFile(input_dir, wave_type)
            if target_csv is None:
                print(f'Directory "{input_dir}" is empty.')
                continue

            fig = None

            try:
                samples = pd.read_csv(target_csv)
                amps = pd.to_numeric(samples.iloc[:, 0], errors='raise')

                output_dir = Path(PARENTS_DIRS['OUTPUT']) / input_dir.name
                output_dir.mkdir(parents=True, exist_ok=True)
                output_file = output_dir / f"{target_csv.stem}.png"

                fig, ax = plt.subplots(figsize=(8, 5))

                if len(samples.columns) == 2: # aliasing test
                    normal = pd.to_numeric(samples.iloc[:, 0], errors="raise")
                    aliasing = pd.to_numeric(samples.iloc[:, 1], errors="raise")
                    ax.plot(normal, label="normal", linewidth=2)
                    ax.plot(aliasing, label="aliasing", linewidth=2, alpha=0.6)
                else:
                    amps = pd.to_numeric(samples.iloc[:, 0], errors="raise")
                    ax.plot(amps, label=samples.columns[0], linewidth=2)

                ax.set(
                    title=f"Oscillator - {input_dir.name} - {wave_type}",
                    xlabel="Samples",
                    ylabel="Amplitude"
                )
                plt.legend()
                plt.savefig(output_file, dpi=300, bbox_inches='tight')

            except (
                OSError,
                ValueError,
                pd.errors.EmptyDataError,
                pd.errors.ParserError
            ) as exc:
                print(
                    f"Failed to generate {wave_type} graph: {exc}",
                    file=sys.stderr
                )
                return 1

            finally:
                if fig is not None:
                    plt.close(fig)

            if input_dir.name == "test-default-sine":
                continue

    return 0

def setTargetCSVFile(d: Path, wave_type: str) -> Path | None:
    csv_files =  [
        p for p in d.iterdir()
        if p.is_file()
        and
        p.name.startswith(f"{wave_type}_")
        and
        p.suffix.lower() == '.csv'
    ]
    if not csv_files:
        return None
    # choose the newest CSV file.
    target_csv = max(csv_files, key=lambda path: path.stat().st_mtime)
    return target_csv

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
