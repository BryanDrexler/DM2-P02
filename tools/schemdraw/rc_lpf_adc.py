#!/usr/bin/env python3
"""
RC Low-Pass Filter schematic for STM32 ADC input.
Circuit: Vin → 10kΩ → node → 33nF to GND → PA0 (ADC pin)

Generates SVG, PNG, and PDF outputs — all headless, no display required.

Usage:
    python3 rc_lpf_adc.py               # generates all three formats
    python3 rc_lpf_adc.py --svg         # SVG only
    python3 rc_lpf_adc.py --png         # PNG only (requires matplotlib)
    python3 rc_lpf_adc.py --pdf         # PDF only (requires matplotlib)

Output files: rc_lpf_adc.svg / .png / .pdf
"""

import argparse
import schemdraw
import schemdraw.elements as elm

# Use the pure-SVG backend — works headless, no X11/display needed.
# Switch to 'matplotlib' if you need PNG/PDF (and set Agg backend first).
schemdraw.use('svg')


def draw_rc_lpf(output_formats: list[str]) -> None:
    """Draw an RC low-pass filter connected to an STM32 ADC pin."""

    with schemdraw.Drawing() as d:
        # ── Input source label (Vin) ─────────────────────────────────────
        # A short line coming in from the left acts as the input wire.
        vin_line = d.add(elm.Line().right().length(d.unit * 0.8))
        d.add(elm.Label().at(vin_line.start).label("$V_{in}$", loc="left"))

        # ── 10 kΩ series resistor ────────────────────────────────────────
        r = d.add(elm.Resistor().right().label("10 kΩ", loc="top"))

        # ── Node dot where resistor meets cap & ADC wire ─────────────────
        node = d.add(elm.Dot())

        # ── 33 nF capacitor to GND (vertical, going down) ───────────────
        # Save position at node to branch down.
        d.add(elm.Capacitor().down().label("33 nF", loc="right").length(d.unit))
        d.add(elm.Ground())

        # ── Move back to the node and continue right to ADC pin ──────────
        d.add(elm.Line().at(node.end).right().length(d.unit * 0.8))
        d.add(elm.Label().label("PA0 / ADC1_IN0", loc="right"))

        # ── Title annotation ─────────────────────────────────────────────
        d.add(
            elm.Label()
            .at((r.center[0], r.end[1] + d.unit * 0.9))
            .label("RC Low-Pass Filter — STM32 ADC Input", loc="center")
        )

        # ── Cutoff frequency annotation ──────────────────────────────────
        # f_c = 1 / (2π·R·C) = 1 / (2π · 10kΩ · 33nF) ≈ 482 Hz
        d.add(
            elm.Label()
            .at((r.center[0], r.end[1] + d.unit * 0.55))
            .label("$f_c = \\frac{1}{2\\pi RC} = \\frac{1}{2\\pi \\cdot 10k\\Omega \\cdot 33nF} \\approx 482\\,Hz$", loc="center")
        )

        # ── Save in requested formats ────────────────────────────────────
        for fmt in output_formats:
            fname = f"rc_lpf_adc.{fmt}"
            d.save(fname)
            print(f"Saved: {fname}")


def draw_rc_lpf_svg_only(output_file: str = "rc_lpf_adc.svg") -> None:
    """
    Minimal version: pure SVG backend, zero external dependencies beyond schemdraw.
    Suitable for CI / GitHub Actions / headless Docker.
    """
    schemdraw.use('svg')

    with schemdraw.Drawing(file=output_file) as d:
        vin_line = d.add(elm.Line().right().length(d.unit * 0.8))
        d.add(elm.Label().at(vin_line.start).label("Vin", loc="left"))
        r = d.add(elm.Resistor().right().label("10 kΩ", loc="top"))
        node = d.add(elm.Dot())
        d.add(elm.Capacitor().down().label("33 nF", loc="right").length(d.unit))
        d.add(elm.Ground())
        d.add(elm.Line().at(node.end).right().length(d.unit * 0.8))
        d.add(elm.Label().label("PA0 (ADC)", loc="right"))

    print(f"Saved: {output_file}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate RC LPF schematic")
    parser.add_argument("--svg", action="store_true", help="Output SVG")
    parser.add_argument("--png", action="store_true", help="Output PNG (needs matplotlib)")
    parser.add_argument("--pdf", action="store_true", help="Output PDF (needs matplotlib)")
    args = parser.parse_args()

    fmts = []
    if args.svg: fmts.append("svg")
    if args.png: fmts.append("png")
    if args.pdf: fmts.append("pdf")
    if not fmts:
        fmts = ["svg"]   # default: SVG only (headless-safe)

    if fmts == ["svg"]:
        # Use the simple headless path
        draw_rc_lpf_svg_only()
    else:
        # PNG/PDF require matplotlib's Agg backend
        import matplotlib
        matplotlib.use("Agg")
        schemdraw.use('matplotlib')
        draw_rc_lpf(fmts)
