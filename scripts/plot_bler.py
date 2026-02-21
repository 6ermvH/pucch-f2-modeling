#!/usr/bin/env python3
"""
Строит кривые BLER(SNR) для всех кодов PUCCH F2.

Использование:
    python3 scripts/plot_bler.py [--binary PATH] [--out FILE] [--iters N]

По умолчанию бинарник ищется в ./build/pucch_f2.
"""

import argparse
import json
import os
import subprocess
import sys
import tempfile

import matplotlib.pyplot as plt

SNR_RANGE   = range(-5, 11)   # от -5 до 10 дБ включительно
N_BITS_LIST = [2, 4, 6, 8, 11]
DEFAULT_BIN = os.path.join(os.path.dirname(__file__), "..", "build", "pucch_f2")


def run_simulation(binary: str, n_bits: int, snr_db: float, iterations: int) -> float:
    """Запускает симуляцию и возвращает BLER."""
    payload = {
        "mode": "channel simulation",
        "num_of_pucch_f2_bits": n_bits,
        "snr_db": snr_db,
        "iterations": iterations,
    }

    with tempfile.TemporaryDirectory() as tmp:
        input_path  = os.path.join(tmp, "input.json")
        result_path = os.path.join(tmp, "result.json")

        with open(input_path, "w") as f:
            json.dump(payload, f)

        result = subprocess.run(
            [binary, input_path],
            capture_output=True,
            text=True,
            cwd=tmp,
        )

        if result.returncode != 0:
            print(f"Error: {result.stderr.strip()}", file=sys.stderr)
            sys.exit(1)

        with open(result_path) as f:
            data = json.load(f)

    return data["bler"]


def main():
    parser = argparse.ArgumentParser(description="Plot BLER curves for PUCCH F2")
    parser.add_argument("--binary", default=DEFAULT_BIN,
                        help="Path to pucch_f2 binary")
    parser.add_argument("--out",    default="bler_curves.png",
                        help="Output image file")
    parser.add_argument("--iters",  type=int, default=5000,
                        help="Iterations per SNR point")
    args = parser.parse_args()

    binary = os.path.abspath(args.binary)
    if not os.path.isfile(binary):
        print(f"Binary not found: {binary}", file=sys.stderr)
        sys.exit(1)

    snr_values = list(SNR_RANGE)
    results    = {}  # n_bits → list of BLER

    total = len(N_BITS_LIST) * len(snr_values)
    done  = 0

    for n in N_BITS_LIST:
        blers = []
        for snr in snr_values:
            bler = run_simulation(binary, n, float(snr), args.iters)
            blers.append(bler if bler > 0 else 1e-5)  # избегаем log(0) на графике
            done += 1
            print(f"[{done:3d}/{total}] n={n:2d}, SNR={snr:+3d} dB → BLER={bler:.4f}")
        results[n] = blers

    # ── График ────────────────────────────────────────────────────────────────
    fig, ax = plt.subplots(figsize=(9, 6))

    for n in N_BITS_LIST:
        ax.semilogy(snr_values, results[n], marker="o", markersize=4,
                    label=f"n={n} (20,{n})")

    ax.set_xlabel("SNR, дБ")
    ax.set_ylabel("BLER")
    ax.set_title("BLER vs SNR — PUCCH FORMAT2 блочные коды")
    ax.legend(title="Код")
    ax.grid(True, which="both", linestyle="--", alpha=0.5)
    ax.set_ylim(1e-5, 1.1)

    plt.tight_layout()
    plt.savefig(args.out, dpi=150)
    print(f"\nГрафик сохранён: {args.out}")


if __name__ == "__main__":
    main()
