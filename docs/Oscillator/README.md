# Oscillator Waveform Graphs

These graphs are generated from CSV samples produced by
`packages/dsp-core/tests/OscillatorTest.cpp`.

The generated files are organized by test case:

```text
docs/Oscillator/
├── test-default-sine/
├── test-default-wave-forms/
├── test-harmonic-limit/
├── test-aliasing-allowed/
└── test-harmonics-above-nyquist-are-filtered/
```

Each graph file uses the corresponding CSV file name with a `.png` extension.

## Common notes

- The oscillator uses additive synthesis.
- Harmonic amplitudes are not normalized before the samples are written.
- Some waveforms can therefore exceed the amplitude range `[-1, 1]`.
- A finite number of harmonics is used. Discontinuous waveforms can show
  ringing near their transitions because of the Gibbs phenomenon.
- A CSV with one sample buffer has one column. A comparison CSV has two
  columns named `<Wave>_normal` and `<Wave>_aliasing`.
- `test-reset` does not generate a graph because it verifies phase restoration.

## Default sine

`test-default-sine` generates one Sine CSV and one graph. It verifies the
default oscillator configuration.

## Default waveforms

`test-default-wave-forms` uses the default harmonic settings for Sine,
Triangle, Square, and Sawtooth.

The Triangle waveform uses the minimum harmonic setting. It contains the
fundamental and the third harmonic, so it looks and sounds similar to a sine
wave. It is still a triangle-wave approximation in the additive model.

The default graphs are therefore useful for checking the default settings,
but they do not represent fully developed ideal waveforms.

## Harmonic limit

`test-harmonic-limit` requests more than the supported maximum number of
harmonics. The oscillator clamps the value to 64.

For Triangle and Square, only odd harmonics contribute. Thus, a maximum of 64
means that the active harmonic numbers are `1, 3, 5, ..., 63`.

## Aliasing allowed

In `test-aliasing-allowed`, the fundamental frequency itself is above the
Nyquist frequency.

- `normal` filters the fundamental and is silent.
- `aliasing` keeps the out-of-band frequency, which produces an aliased signal.

This test intentionally compares a silent normal buffer with a non-silent
aliasing buffer.

## Harmonics above Nyquist

In `test-harmonics-above-nyquist-are-filtered`, the fundamental frequency is
below Nyquist, but higher harmonics exceed it.

- `normal` keeps harmonics at or below Nyquist.
- `aliasing` keeps the higher harmonics as well.
- `normal` is not silent because the fundamental and lower harmonics remain.
- Triangle graphs can look almost identical because triangle-wave harmonic
  amplitudes decrease as `1 / n²`.

## Regenerating the graphs

Run the commands from the project root:

```bash
python3 tools/visualize_oscillator.py --generate-inputs
python3 tools/visualize_oscillator.py --generate-outputs
```

To remove generated files:

```bash
python3 tools/visualize_oscillator.py --clear-inputs
python3 tools/visualize_oscillator.py --clear-outputs
```
