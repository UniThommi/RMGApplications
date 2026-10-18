#!/usr/bin/env python3
"""
Reflectivity table of the sensitive surface detector (SSD).

The SSD surface is an area-weighted mixture of PMT cathode and Tyvek foil:

    R_SSD(lambda) = S * R_cathode + (1 - S) * R_Tyvek(lambda)

with S the fraction of the surface covered by PMT cathodes, the wavelength-independent
cathode reflectivity R_cathode = 0.0287... and the Tyvek reflectivity table below. The
same formula is used in efficiencyDerivationSSD. S is given as a number or fraction
on the command line; the table is printed as "<wavelength in nm>  <reflectivity>".
"""
import sys
from fractions import Fraction

# Original table (nm, reflectivity): Tyvek reflectivity R_Tyvek
data = [
    (250, 0.80),
    (300, 0.90),
    (355, 0.945),
    (400, 0.96),
    (440, 0.96),
    (500, 0.96),
    (800, 0.96),
]

def parse_fraction(s: str) -> Fraction:
    """Parse "a/b" or a plain number exactly (e.g. "1/5", "0.2")."""
    if "/" in s:
        num, den = s.split("/")
        return Fraction(num) / Fraction(den)
    return Fraction(s)

def generate_table(S_str):
    """Return [(wavelength in nm, R_SSD)] for the cathode area fraction S."""
    S = float(parse_fraction(S_str))
    result = []
    for nm, base_value in data:
        # 0.028725... is the cathode reflectivity R_cathode
        new_value = S * 0.028725181851905604 + (1 - S) * base_value
        result.append((nm, new_value))
    return result

def main():
    if len(sys.argv) != 2:
        print("Usage: python calculateSSDReflectivity.py S_fraction")
        print("Example: python calculateSSDReflectivity.py 1/5")
        sys.exit(1)

    S_str = sys.argv[1]
    table = generate_table(S_str)

    print("# nm   dimensionless")
    for nm, val in table:
        print(f"{nm}    {val:.15f}")

if __name__ == "__main__":
    main()
