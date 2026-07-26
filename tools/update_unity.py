#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Koji KITAYAMA
"""
Unity Test Framework download and setup script

Usage:
  python scripts/update_unity.py          # get latest version
  python scripts/update_unity.py v2.5.2   # get specific version
"""

import argparse
import json
import os
import shutil
import sys
import tempfile
import zipfile
from datetime import datetime
from pathlib import Path
from urllib.request import urlopen, Request
from urllib.error import URLError, HTTPError

# Configuration
GITHUB_REPO = "ThrowTheSwitch/Unity"
OUTPUT_DIR = Path("tests") / "unity"
REQUIRED_FILES = [
    "src/unity.c",
    "src/unity.h",
    "src/unity_internals.h",
    "LICENSE.txt"
]

def get_latest_release():
    """Get the latest release information from the GitHub API"""
    api_url = f"https://api.github.com/repos/{GITHUB_REPO}/releases/latest"

    try:
        req = Request(api_url)
        req.add_header('Accept', 'application/vnd.github.v3+json')

        with urlopen(req, timeout=10) as response:
            data = json.loads(response.read().decode())
            return data['tag_name']

    except (URLError, HTTPError, KeyError) as e:
        print(f"Error: Failed to get latest release info: {e}", file=sys.stderr)
        return None

def download_unity(version, temp_dir):
    """Download the specified version of Unity"""
    # ZIP file URL
    zip_url = f"https://github.com/{GITHUB_REPO}/archive/refs/tags/{version}.zip"
    zip_path = temp_dir / f"unity_{version}.zip"

    print(f"Downloading: {zip_url}")

    try:
        with urlopen(zip_url, timeout=30) as response:
            with open(zip_path, 'wb') as f:
                f.write(response.read())

        print(f"Download complete: {zip_path}")
        return zip_path

    except (URLError, HTTPError) as e:
        print(f"Error: Failed to download: {e}", file=sys.stderr)
        return None

def extract_files(zip_path, version, temp_dir):
    """Extract necessary files from the ZIP"""
    # Temporary extraction directory
    extract_dir = temp_dir / "unity_extracted"

    # Directory name in Unity-{version} format (remove 'v' prefix)
    version_clean = version.lstrip('v')
    unity_dir = f"Unity-{version_clean}"

    print(f"Extracting: {zip_path}")

    try:
        # Remove existing extraction directory
        if extract_dir.exists():
            shutil.rmtree(extract_dir)

        # Extract ZIP
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            zip_ref.extractall(extract_dir)

        # Check for file existence
        extracted_files = {}
        for file_path in REQUIRED_FILES:
            # Special handling for LICENSE.txt (located in the root)
            if file_path == "LICENSE.txt":
                src = extract_dir / unity_dir / file_path
            else:
                src = extract_dir / unity_dir / file_path

            if not src.exists():
                print(f"Warning: {file_path} not found", file=sys.stderr)
                continue

            extracted_files[file_path] = src

        if len(extracted_files) != len(REQUIRED_FILES):
            print("Error: Required files are missing", file=sys.stderr)
            return None

        print(f"Extraction complete: {len(extracted_files)} files extracted")
        return extracted_files

    except zipfile.BadZipFile as e:
        print(f"Error: Failed to extract ZIP: {e}", file=sys.stderr)
        return None

def copy_to_destination(extracted_files, version):
    """Copy files to the final destination"""
    # Create output directory
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    print(f"Destination: {OUTPUT_DIR}")

    # Copy files
    for file_path, src in extracted_files.items():
        # Output file name (flatten the directory structure)
        filename = Path(file_path).name

        # Rename LICENSE.txt -> LICENSE
        if filename == "LICENSE.txt":
            filename = "LICENSE"

        dst = OUTPUT_DIR / filename

        shutil.copy2(src, dst)
        print(f"  Copied: {filename}")

    # Generate VERSION.txt
    generate_version_file(version)

    print("Deployment complete")

def generate_version_file(version):
    """Generate VERSION.txt"""
    version_file = OUTPUT_DIR / "VERSION.txt"

    content = f"""Unity Test Framework {version}
https://github.com/{GITHUB_REPO}
Downloaded: {datetime.now().strftime('%Y-%m-%d')}
"""

    with open(version_file, 'w', encoding='utf-8') as f:
        f.write(content)

    print(f"  Generated: VERSION.txt")

def cleanup(temp_dir):
    """Clean up temporary files"""
    if temp_dir.exists():
        shutil.rmtree(temp_dir)

def main():
    parser = argparse.ArgumentParser(
        description='Download and deploy the Unity Test Framework'
    )
    parser.add_argument(
        'version',
        nargs='?',
        default='latest',
        help='Specify version (e.g., v2.5.2). Defaults to the latest version.'
    )
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Show what would be done without actually downloading or copying files.'
    )

    args = parser.parse_args()

    if args.version == 'latest':
        print("Checking latest version...")
        version = get_latest_release()
        if not version:
            print("Error: Failed to get latest release info", file=sys.stderr)
            return 1
        print(f"Latest version: {version}")
    else:
        version = args.version
        print(f"Specified version: {version}")

    if args.dry_run:
        print(f"\n[DRY RUN] The following actions would be performed:")
        print(f"  1. Download {version}")
        print(f"  2. Extract necessary files")
        print(f"  3. Deploy to {OUTPUT_DIR}")
        return 0

    temp_dir = Path(tempfile.mkdtemp(prefix='unity_'))

    try:
        zip_path = download_unity(version, temp_dir)
        if not zip_path:
            return 1

        extracted_files = extract_files(zip_path, version, temp_dir)
        if not extracted_files:
            return 1

        copy_to_destination(extracted_files, version)

        print(f"\nSuccess: Unity {version} deployed to {OUTPUT_DIR}")
        return 0

    except Exception as e:
        print(f"Error: An unexpected error occurred: {e}", file=sys.stderr)
        return 1

    finally:
        cleanup(temp_dir)

if __name__ == '__main__':
    sys.exit(main())
