#!/usr/bin/env python3
"""
Voltage divider + RC filter schematic for STM32 ADC input.

Circuit mirrors the resistor divider from the lab:
  3V3 → 1kΩ → node_top → 10kΩ → node_bot → PA0
                |                     |
              1kΩ                   33nF
                |                     |
               GND                   GND

This demonstrates:
 - Branching (vertical elements off a horizontal rail)
 - Multiple ground symbols
 - Node dots
 - Labels on arbitrary anchors

Usage:
    python3 voltage_divider_adc.py
Output: voltage_divider_adc.svg
"""

import schemdraw
import schemdraw.elements as elm

schemdraw.use('svg')


def main() -> None:
    with schemdraw.Drawing(file="voltage_divider_adc.svg") as d:
        # ── 3V3 supply label ─────────────────────────────────────────────
        d.add(elm.Vdd().label("3V3", loc="right"))

        # ── Top 1kΩ shunt resistor (vertical, going down from 3V3) ───────
        r_shunt1 = d.add(elm.Resistor().down().label("1 kΩ", loc="right"))

        # ── Node A (top junction: 1kΩ shunt meets 10kΩ series) ──────────
        node_a = d.add(elm.Dot())

        # ── Series 10kΩ resistor going right from node A ─────────────────
        d.add(elm.Line().at(node_a.end).right().length(d.unit * 0.5))
        r_series = d.add(elm.Resistor().right().label("10 kΩ", loc="top"))

        # ── Node B (right junction: 10kΩ meets 33nF and ADC wire) ────────
        node_b = d.add(elm.Dot())

        # ── 33nF capacitor to GND off node B ─────────────────────────────
        d.add(elm.Capacitor().at(node_b.end).down()
              .label("33 nF", loc="right").length(d.unit))
        d.add(elm.Ground())

        # ── ADC wire from node B to PA0 label ────────────────────────────
        d.add(elm.Line().at(node_b.end).right().length(d.unit * 0.5))
        d.add(elm.Label().label("PA0 / ADC1_IN0", loc="right"))

        # ── Bottom 1kΩ shunt resistor to GND (from node A downward) ──────
        d.add(elm.Resistor().at(node_a.end).down()
              .label("1 kΩ", loc="right").length(d.unit))
        d.add(elm.Ground())

    print("Saved: voltage_divider_adc.svg")


if __name__ == "__main__":
    main()
