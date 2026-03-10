#!/usr/bin/env python3
"""
svgbob demo — convert ASCII circuit art to SVG via the Kroki.io public API.

svgbob itself is a Rust CLI tool (cargo install svgbob_cli).
Since Rust/cargo may not be available in every environment, this script uses
the free Kroki.io public API which bundles svgbob and accepts POST requests.

Usage:
    python3 svgbob_demo.py
    python3 svgbob_demo.py --dry-run    # prints ASCII source only, no HTTP

Outputs:
    rc_lpf_svgbob.svg           — RC filter
    voltage_divider_svgbob.svg  — resistor divider + RC filter
"""

import argparse
import sys
import zlib
import base64
import urllib.request
import urllib.error
import json

KROKI_URL = "https://kroki.io"


# ─── ASCII diagram sources ────────────────────────────────────────────────────

RC_LPF = """
        10kΩ           PA0
Vin o--[====]--+--------o
               |
              [=]  33nF
               |
              GND
"""

# The divider from the lab brief:
#   3V3 → 1kΩ → node → 10kΩ → node → PA0
#               |                |
#             1kΩ              33nF
#               |                |
#              GND              GND
VOLTAGE_DIVIDER = """
  3V3
   |
  [R]  1kΩ
   |
   +-------[R]-------+--- PA0
   |       10kΩ      |
  [R]  1kΩ          [C]  33nF
   |                 |
  GND               GND
"""

# More faithful svgbob notation using its ASCII art conventions:
# svgbob supports: - | / \\ + . lines, arrows (-> <-), boxes ([...])
VOLTAGE_DIVIDER_SVGBOB = r"""
    3V3
     |
   +-+-+
   |   |  1kΩ
   +---+
     |
     +----------+-----------> PA0
     |          |
   +-+-+      +-+-+
   |   |      |   |   1kΩ / 33nF
   +---+      +---+
     |          |
    GND        GND
"""


# ─── Kroki API helpers ────────────────────────────────────────────────────────

def encode_for_kroki(diagram_source: str) -> str:
    """
    Kroki GET API: diagram source must be deflate-compressed + base64url-encoded.
    For POST we send raw text, but GET URLs are useful for embedding in Markdown.
    """
    compressed = zlib.compress(diagram_source.encode(), level=9)
    return base64.urlsafe_b64encode(compressed).decode()


def get_kroki_url(diagram_type: str, output_format: str, source: str) -> str:
    """Return a GET URL suitable for embedding in Markdown / README."""
    encoded = encode_for_kroki(source)
    return f"{KROKI_URL}/{diagram_type}/{output_format}/{encoded}"


def post_to_kroki(diagram_type: str, output_format: str, source: str) -> bytes:
    """
    POST the diagram source to Kroki and return the raw SVG bytes.
    Uses only stdlib (urllib) — no requests dependency.
    """
    url = f"{KROKI_URL}/{diagram_type}/{output_format}"
    payload = json.dumps({
        "diagram_source": source,
        "diagram_type": diagram_type,
        "output_format": output_format,
    }).encode("utf-8")

    req = urllib.request.Request(
        url,
        data=payload,
        headers={
            "Content-Type": "application/json",
            "Accept": "image/svg+xml",
        },
        method="POST",
    )
    with urllib.request.urlopen(req, timeout=15) as resp:
        return resp.read()


def save_from_kroki(diagram_type: str, source: str, output_file: str) -> None:
    print(f"Fetching {output_file} from Kroki ({diagram_type})...")
    try:
        svg_bytes = post_to_kroki(diagram_type, "svg", source)
        with open(output_file, "wb") as f:
            f.write(svg_bytes)
        print(f"  Saved: {output_file}  ({len(svg_bytes):,} bytes)")
    except urllib.error.URLError as e:
        print(f"  ERROR contacting Kroki: {e}")
        print("  Check network or try the online editor: https://ivanceras.github.io/svgbob-editor/")


# ─── Main ─────────────────────────────────────────────────────────────────────

def main() -> None:
    parser = argparse.ArgumentParser(description="svgbob via Kroki.io demo")
    parser.add_argument("--dry-run", action="store_true",
                        help="Print ASCII source and GET URLs only, no HTTP requests")
    args = parser.parse_args()

    diagrams = [
        ("svgbob", RC_LPF, "rc_lpf_svgbob.svg"),
        ("svgbob", VOLTAGE_DIVIDER_SVGBOB, "voltage_divider_svgbob.svg"),
    ]

    for dtype, source, outfile in diagrams:
        print(f"\n{'='*60}")
        print(f"Diagram: {outfile}")
        print(f"{'='*60}")
        print("ASCII source:")
        print(source)

        get_url = get_kroki_url(dtype, "svg", source)
        print(f"\nKroki GET URL (embeddable in Markdown):")
        print(f"  {get_url[:80]}...")
        print(f"\nMarkdown embed:")
        print(f"  ![{outfile}]({get_url})")

        if not args.dry_run:
            save_from_kroki(dtype, source, outfile)


    print("\n\nNOTES ON svgbob:")
    print("-" * 60)
    print("1. svgbob is a Rust CLI: cargo install svgbob_cli")
    print("   CLI usage: svgbob input.txt -o output.svg")
    print("   Or:        echo 'diagram' | svgbob > out.svg")
    print("")
    print("2. svgbob DOES NOT understand component labels like [10kΩ].")
    print("   It treats any text as plain annotations alongside geometry.")
    print("   Resistor symbols must be drawn with box notation: [===]")
    print("   or just labeled text next to lines.")
    print("")
    print("3. For README embedding without CLI, use Kroki.io GET URLs:")
    print("   ![diagram](https://kroki.io/svgbob/svg/<base64url>)")
    print("   These render in GitHub README, GitLab, and most Markdown renderers.")
    print("")
    print("4. Sphinx users: pip install sphinxcontrib-svgbob")
    print("   mdBook users:  cargo install mdbook-svgbob")


if __name__ == "__main__":
    main()
