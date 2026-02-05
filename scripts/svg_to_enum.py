#!/usr/bin/env python3
"""
Generate a C++ header file with an enum of SVG element IDs.

Usage:
    python scripts/svg_to_enum.py assets/ship_wireframe.svg
    python scripts/svg_to_enum.py assets/ship_wireframe.svg -o src/ship_ids.h
    python scripts/svg_to_enum.py assets/ship_wireframe.svg --enum-name ShipShapeId
"""

import argparse
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


def kebab_to_pascal(s: str) -> str:
    """Convert kebab-case to PascalCase."""
    # Handle IDs that start with numbers by prefixing with underscore
    if s and s[0].isdigit():
        s = "_" + s
    # Split on hyphens and underscores, capitalize each part
    parts = re.split(r"[-_]", s)
    return "".join(word.capitalize() for word in parts)


def extract_ids(svg_path: str) -> list[str]:
    """Extract all id attributes from an SVG file."""
    tree = ET.parse(svg_path)
    root = tree.getroot()

    # Handle SVG namespace
    ns = {"svg": "http://www.w3.org/2000/svg"}

    ids = []

    def collect_ids(elem):
        if "id" in elem.attrib:
            ids.append(elem.attrib["id"])
        for child in elem:
            collect_ids(child)

    collect_ids(root)
    return ids


def generate_header(
    ids: list[str], enum_name: str, svg_filename: str, include_strings: bool
) -> str:
    """Generate C++ header content."""
    lines = [
        "#pragma once",
        "",
        f'// Auto-generated from {svg_filename}',
        f"// Run: python scripts/svg_to_enum.py assets/{svg_filename}",
        "",
    ]

    # Generate enum
    lines.append(f"enum class {enum_name} {{")
    for id_str in ids:
        pascal = kebab_to_pascal(id_str)
        lines.append(f"    {pascal},")
    lines.append("")
    lines.append("    COUNT")
    lines.append("};")

    if include_strings:
        lines.append("")
        lines.append(f"// String table for {enum_name}")
        lines.append(f"inline const char* const k{enum_name}Strings[] = {{")
        for id_str in ids:
            lines.append(f'    "{id_str}",')
        lines.append("};")
        lines.append("")
        lines.append(f"inline const char* {enum_name}ToString({enum_name} id) {{")
        lines.append(f"    int idx = static_cast<int>(id);")
        lines.append(
            f"    if (idx < 0 || idx >= static_cast<int>({enum_name}::COUNT)) return nullptr;"
        )
        lines.append(f"    return k{enum_name}Strings[idx];")
        lines.append("}")

    lines.append("")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description="Generate C++ enum header from SVG element IDs"
    )
    parser.add_argument("svg_file", help="Path to SVG file")
    parser.add_argument(
        "-o", "--output", help="Output header file path (default: stdout)"
    )
    parser.add_argument(
        "--enum-name",
        help="Name for the enum (default: derived from filename, e.g., ShipShapeId)",
    )
    parser.add_argument(
        "--no-strings",
        action="store_true",
        help="Don't generate string table and ToString function",
    )

    args = parser.parse_args()

    svg_path = Path(args.svg_file)
    if not svg_path.exists():
        print(f"Error: File not found: {svg_path}", file=sys.stderr)
        sys.exit(1)

    # Extract IDs from SVG
    try:
        ids = extract_ids(str(svg_path))
    except ET.ParseError as e:
        print(f"Error parsing SVG: {e}", file=sys.stderr)
        sys.exit(1)

    if not ids:
        print(f"Warning: No IDs found in {svg_path}", file=sys.stderr)

    # Determine enum name
    if args.enum_name:
        enum_name = args.enum_name
    else:
        # Derive from filename: ship_wireframe.svg -> ShipWireframeId
        stem = svg_path.stem
        enum_name = kebab_to_pascal(stem.replace("_", "-")) + "Id"

    # Generate header
    header = generate_header(ids, enum_name, svg_path.name, not args.no_strings)

    # Output
    if args.output:
        output_path = Path(args.output)
        output_path.write_text(header)
        print(f"Generated: {output_path}")
        print(f"  - {len(ids)} shape IDs extracted")
        print(f"  - Enum: {enum_name}")
    else:
        print(header)


if __name__ == "__main__":
    main()
