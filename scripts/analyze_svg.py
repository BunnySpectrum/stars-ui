#!/usr/bin/env python3
"""Analyze an SVG file for LCARS UI compatibility.

This script checks whether an SVG file meets the requirements for use
with the LCARS UI's SvgRenderer, which relies on:
- Shape IDs for runtime color changes and label overlays
- Presentation attributes (not inline styles) for reliable parsing
- Real text elements (not stroked paths) for proper rendering

Usage:
    python3 scripts/analyze_svg.py <path-to-svg>
    python3 scripts/analyze_svg.py assets/ship_wireframe.svg
"""

import re
import sys
import os
from pathlib import Path


def analyze_svg(filepath: str) -> dict:
    """Analyze an SVG file and return metrics."""

    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    lines = content.count('\n') + 1
    size_bytes = os.path.getsize(filepath)

    # Count various elements and attributes
    id_matches = re.findall(r'id="([^"]*)"', content)
    style_count = len(re.findall(r'style="[^"]*"', content))
    text_count = len(re.findall(r'<text\b', content))
    stroked_text_count = len(re.findall(r'stroked-text', content))
    path_count = len(re.findall(r'<path\b', content))

    # Separate component IDs from label IDs
    component_ids = [id for id in id_matches if not id.startswith('label-')]
    label_ids = [id for id in id_matches if id.startswith('label-')]

    # Check for duplicate IDs
    id_counts = {}
    for id in id_matches:
        id_counts[id] = id_counts.get(id, 0) + 1
    duplicate_ids = {id: count for id, count in id_counts.items() if count > 1}

    # Check viewBox
    viewbox_match = re.search(r'viewBox="([^"]*)"', content)
    viewbox = viewbox_match.group(1) if viewbox_match else None

    # Check for width/height with units
    width_match = re.search(r'width="([^"]*)"', content)
    height_match = re.search(r'height="([^"]*)"', content)
    width = width_match.group(1) if width_match else None
    height = height_match.group(1) if height_match else None

    return {
        'filepath': filepath,
        'lines': lines,
        'size_bytes': size_bytes,
        'size_kb': size_bytes / 1024,
        'id_count': len(id_matches),
        'component_ids': component_ids,
        'label_ids': label_ids,
        'duplicate_ids': duplicate_ids,
        'style_count': style_count,
        'text_count': text_count,
        'stroked_text_count': stroked_text_count,
        'path_count': path_count,
        'viewbox': viewbox,
        'width': width,
        'height': height,
    }


def format_size(bytes: int) -> str:
    """Format bytes as human-readable size."""
    if bytes < 1024:
        return f"{bytes} B"
    elif bytes < 1024 * 1024:
        return f"{bytes / 1024:.1f} KB"
    else:
        return f"{bytes / (1024 * 1024):.1f} MB"


def print_report(metrics: dict) -> bool:
    """Print analysis report and return True if SVG is suitable."""

    print(f"\n{'=' * 60}")
    print(f"SVG Analysis: {metrics['filepath']}")
    print(f"{'=' * 60}\n")

    # Track pass/fail
    checks = []

    # File info
    print("FILE INFO")
    print("-" * 40)
    print(f"  Lines:      {metrics['lines']:,}")
    print(f"  Size:       {format_size(metrics['size_bytes'])}")
    print(f"  ViewBox:    {metrics['viewbox'] or 'NOT SET'}")
    print(f"  Dimensions: {metrics['width']} x {metrics['height']}")
    print()

    # Checks table
    print("COMPATIBILITY CHECKS")
    print("-" * 40)
    print(f"  {'Check':<30} {'Value':>8}  {'Status':<6}")
    print(f"  {'-' * 30} {'-' * 8}  {'-' * 6}")

    # Check 1: Has IDs
    id_pass = metrics['id_count'] > 0
    checks.append(id_pass)
    status = "PASS" if id_pass else "FAIL"
    print(f"  {'ID attributes':<30} {metrics['id_count']:>8}  {status:<6}")

    # Check 2: No inline styles (or very few)
    style_pass = metrics['style_count'] == 0
    checks.append(style_pass)
    status = "PASS" if style_pass else "WARN" if metrics['style_count'] < 10 else "FAIL"
    print(f"  {'Inline styles (should be 0)':<30} {metrics['style_count']:>8}  {status:<6}")

    # Check 3: Has text elements (if there are labels)
    text_pass = metrics['text_count'] > 0 or metrics['stroked_text_count'] == 0
    checks.append(text_pass)
    status = "PASS" if metrics['text_count'] > 0 else "WARN" if metrics['stroked_text_count'] == 0 else "FAIL"
    print(f"  {'Text elements':<30} {metrics['text_count']:>8}  {status:<6}")

    # Check 4: No stroked-text paths
    stroked_pass = metrics['stroked_text_count'] == 0
    checks.append(stroked_pass)
    status = "PASS" if stroked_pass else "FAIL"
    print(f"  {'Stroked-text (should be 0)':<30} {metrics['stroked_text_count']:>8}  {status:<6}")

    # Check 5: No duplicate IDs
    dup_pass = len(metrics['duplicate_ids']) == 0
    checks.append(dup_pass)
    status = "PASS" if dup_pass else "WARN"
    print(f"  {'Duplicate IDs (should be 0)':<30} {len(metrics['duplicate_ids']):>8}  {status:<6}")

    # Check 6: Reasonable file size (under 100KB for good performance)
    size_pass = metrics['size_kb'] < 100
    checks.append(size_pass)
    status = "PASS" if metrics['size_kb'] < 50 else "WARN" if metrics['size_kb'] < 100 else "FAIL"
    print(f"  {'File size (< 100KB)':<30} {metrics['size_kb']:>7.1f}K  {status:<6}")

    print()

    # Component IDs
    if metrics['component_ids']:
        print("COMPONENT IDs (for data binding)")
        print("-" * 40)
        for id in sorted(metrics['component_ids']):
            print(f"  {id}")
        print()

    # Duplicate IDs warning
    if metrics['duplicate_ids']:
        print("DUPLICATE IDs (may cause issues)")
        print("-" * 40)
        for id, count in sorted(metrics['duplicate_ids'].items()):
            print(f"  {id}: {count} occurrences")
        print()

    # Summary
    all_pass = all(checks)
    critical_pass = checks[0] and checks[3]  # Must have IDs and no stroked-text

    print("SUMMARY")
    print("-" * 40)

    if all_pass:
        print("  [OK] SVG is fully compatible with LCARS UI")
        verdict = True
    elif critical_pass:
        print("  [OK] SVG is compatible (minor warnings)")
        verdict = True
    else:
        print("  [!!] SVG needs modifications before use")
        verdict = False

        if not checks[0]:
            print("       - Add id attributes to shapes for data binding")
        if not checks[3]:
            print("       - Convert stroked-text paths to <text> elements")
        if not checks[1] and metrics['style_count'] >= 10:
            print("       - Convert inline styles to presentation attributes")

    print()
    return verdict


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    filepath = sys.argv[1]

    if not os.path.exists(filepath):
        print(f"Error: File not found: {filepath}")
        sys.exit(1)

    if not filepath.lower().endswith('.svg'):
        print(f"Warning: File does not have .svg extension: {filepath}")

    metrics = analyze_svg(filepath)
    suitable = print_report(metrics)

    sys.exit(0 if suitable else 1)


if __name__ == '__main__':
    main()
