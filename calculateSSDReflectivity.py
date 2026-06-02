#!/usr/bin/env python3
import sys
from fractions import Fraction

# Original table (nm, reflectivity)
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
    if "/" in s:
        num, den = s.split("/")
        return Fraction(num) / Fraction(den)
    return Fraction(s)

def generate_table(S_str):
    S = float(parse_fraction(S_str))
    result = []
    for nm, base_value in data:
        new_value = S * 0.028725181851905604 + (1 - S) * base_value
        result.append((nm, new_value))
    return result

def main():
    if len(sys.argv) != 2:
        print("Usage: python reflekt.py S_fraction")
        print("Example: python reflekt.py 1/5")
        sys.exit(1)

    S_str = sys.argv[1]
    table = generate_table(S_str)

    print("# nm   dimensionless")
    for nm, val in table:
        print(f"{nm}    {val:.15f}")

if __name__ == "__main__":
    main()
