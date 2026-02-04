#!/usr/bin/env python3
"""Analyze an SVG file for LCARS UI compatibility.

This script checks whether an SVG file meets the requirements for use
with the LCARS UI's SvgRenderer, which relies on:
- Shape IDs for runtime color changes and label overlays
- Presentation attributes (not inline styles) for reliable parsing
- Real text elements (not stroked paths) for proper rendering
"""

import re
import sys
import os
from pathlib import Path

import click


def is_closed_path(d_attr: str) -> bool:
    """Check if a path's d attribute represents a closed shape."""
    if not d_attr:
        return False
    # A closed path ends with 'z' or 'Z' command
    d_stripped = d_attr.strip()
    return d_stripped.endswith('z') or d_stripped.endswith('Z')


def check_shape_has_area(element_str: str, tag: str) -> bool:
    """Check if a shape element has area (not a line-only element)."""
    # These shapes always have area
    if tag in ('rect', 'circle', 'ellipse', 'polygon'):
        return True

    # Lines never have area
    if tag in ('line', 'polyline'):
        return False

    # For paths, check if closed
    if tag == 'path':
        d_match = re.search(r'd="([^"]*)"', element_str)
        if d_match:
            return is_closed_path(d_match.group(1))
        return False

    # Other elements (g, use, etc.) - assume OK
    return True


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

    # Check for IDs on line-only shapes (no area for label anchoring)
    # Match elements with id attribute: <tag ... id="..." ...>
    element_pattern = r'<(\w+)\s[^>]*id="([^"]*)"[^>]*>'
    line_only_ids = []
    for match in re.finditer(element_pattern, content):
        tag = match.group(1)
        element_id = match.group(2)
        element_str = match.group(0)

        # Skip label IDs - they don't need area
        if element_id.startswith('label-'):
            continue

        if not check_shape_has_area(element_str, tag):
            line_only_ids.append((element_id, tag))

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
        'line_only_ids': line_only_ids,
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


def print_report(metrics: dict, quiet: bool = False) -> bool:
    """Print analysis report and return True if SVG is suitable."""

    if not quiet:
        click.echo(f"\n{'=' * 60}")
        click.echo(f"SVG Analysis: {metrics['filepath']}")
        click.echo(f"{'=' * 60}\n")

    # Track pass/fail
    checks = []

    if not quiet:
        # File info
        click.echo("FILE INFO")
        click.echo("-" * 40)
        click.echo(f"  Lines:      {metrics['lines']:,}")
        click.echo(f"  Size:       {format_size(metrics['size_bytes'])}")
        click.echo(f"  ViewBox:    {metrics['viewbox'] or 'NOT SET'}")
        click.echo(f"  Dimensions: {metrics['width']} x {metrics['height']}")
        click.echo()

    # Check 1: Has IDs
    id_pass = metrics['id_count'] > 0
    checks.append(id_pass)

    # Check 2: No inline styles (or very few)
    style_pass = metrics['style_count'] == 0
    checks.append(style_pass)

    # Check 3: Has text elements (if there are labels)
    text_pass = metrics['text_count'] > 0 or metrics['stroked_text_count'] == 0
    checks.append(text_pass)

    # Check 4: No stroked-text paths
    stroked_pass = metrics['stroked_text_count'] == 0
    checks.append(stroked_pass)

    # Check 5: No duplicate IDs
    dup_pass = len(metrics['duplicate_ids']) == 0
    checks.append(dup_pass)

    # Check 6: No line-only shapes with IDs (need area for label anchoring)
    line_only_pass = len(metrics['line_only_ids']) == 0
    checks.append(line_only_pass)

    # Check 7: Reasonable file size (under 100KB for good performance)
    size_pass = metrics['size_kb'] < 100
    checks.append(size_pass)

    if not quiet:
        # Checks table
        click.echo("COMPATIBILITY CHECKS")
        click.echo("-" * 40)
        click.echo(f"  {'Check':<30} {'Value':>8}  {'Status':<6}")
        click.echo(f"  {'-' * 30} {'-' * 8}  {'-' * 6}")

        status = "PASS" if id_pass else "FAIL"
        click.echo(f"  {'ID attributes':<30} {metrics['id_count']:>8}  {status:<6}")

        status = "PASS" if style_pass else "WARN" if metrics['style_count'] < 10 else "FAIL"
        click.echo(f"  {'Inline styles (should be 0)':<30} {metrics['style_count']:>8}  {status:<6}")

        status = "PASS" if metrics['text_count'] > 0 else "WARN" if metrics['stroked_text_count'] == 0 else "FAIL"
        click.echo(f"  {'Text elements':<30} {metrics['text_count']:>8}  {status:<6}")

        status = "PASS" if stroked_pass else "FAIL"
        click.echo(f"  {'Stroked-text (should be 0)':<30} {metrics['stroked_text_count']:>8}  {status:<6}")

        status = "PASS" if dup_pass else "WARN"
        click.echo(f"  {'Duplicate IDs (should be 0)':<30} {len(metrics['duplicate_ids']):>8}  {status:<6}")

        status = "PASS" if line_only_pass else "WARN"
        click.echo(f"  {'Line-only IDs (should be 0)':<30} {len(metrics['line_only_ids']):>8}  {status:<6}")

        status = "PASS" if metrics['size_kb'] < 50 else "WARN" if metrics['size_kb'] < 100 else "FAIL"
        click.echo(f"  {'File size (< 100KB)':<30} {metrics['size_kb']:>7.1f}K  {status:<6}")

        click.echo()

        # Component IDs
        if metrics['component_ids']:
            click.echo("COMPONENT IDs (for data binding)")
            click.echo("-" * 40)
            for id in sorted(metrics['component_ids']):
                click.echo(f"  {id}")
            click.echo()

        # Duplicate IDs warning
        if metrics['duplicate_ids']:
            click.echo("DUPLICATE IDs (may cause issues)")
            click.echo("-" * 40)
            for id, count in sorted(metrics['duplicate_ids'].items()):
                click.echo(f"  {id}: {count} occurrences")
            click.echo()

        # Line-only IDs warning
        if metrics['line_only_ids']:
            click.echo("LINE-ONLY IDs (cannot anchor labels)")
            click.echo("-" * 40)
            for id, tag in sorted(metrics['line_only_ids']):
                click.echo(f"  {id}: <{tag}> has no area")
            click.echo("  Tip: Use rect, circle, or closed path instead")
            click.echo()

    # Summary
    all_pass = all(checks)
    critical_pass = checks[0] and checks[3]  # Must have IDs and no stroked-text

    click.echo("SUMMARY")
    click.echo("-" * 40)

    if all_pass:
        click.echo("  [OK] SVG is fully compatible with LCARS UI")
        verdict = True
    elif critical_pass:
        click.echo("  [OK] SVG is compatible (minor warnings)")
        verdict = True
    else:
        click.echo("  [!!] SVG needs modifications before use")
        verdict = False

        if not checks[0]:
            click.echo("       - Add id attributes to shapes for data binding")
        if not checks[3]:
            click.echo("       - Convert stroked-text paths to <text> elements")
        if not checks[1] and metrics['style_count'] >= 10:
            click.echo("       - Convert inline styles to presentation attributes")

    # Additional warnings (non-critical but worth noting)
    if not checks[5]:  # line-only IDs
        click.echo("  [!!] Some IDs are on line-only shapes (labels won't anchor)")
        click.echo("       - Use rect, circle, or closed path for label targets")

    click.echo()
    return verdict


@click.command()
@click.argument('svg_file', type=click.Path(exists=True))
@click.option('--quiet', '-q', is_flag=True, help='Only show summary, not full report')
@click.option('--json', 'json_output', is_flag=True, help='Output results as JSON')
def main(svg_file: str, quiet: bool, json_output: bool):
    """Analyze an SVG file for LCARS UI compatibility.

    SVG_FILE is the path to the SVG file to analyze.

    Examples:

        python analyze_svg.py assets/ship_wireframe.svg

        python analyze_svg.py --quiet diagram.svg

        python analyze_svg.py --json circuit.svg
    """
    if not svg_file.lower().endswith('.svg'):
        click.echo(f"Warning: File does not have .svg extension: {svg_file}", err=True)

    metrics = analyze_svg(svg_file)

    if json_output:
        import json
        # Convert tuples to lists for JSON serialization
        metrics['line_only_ids'] = [list(item) for item in metrics['line_only_ids']]
        click.echo(json.dumps(metrics, indent=2))
        sys.exit(0)

    suitable = print_report(metrics, quiet=quiet)
    sys.exit(0 if suitable else 1)


if __name__ == '__main__':
    main()
