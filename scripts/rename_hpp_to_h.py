#!/usr/bin/env python3
"""
Script to rename all .hpp files to .h and update all include statements
"""

import os
import re
from pathlib import Path

def find_all_hpp_files(root_dir):
    """Find all .hpp files in the source directory"""
    hpp_files = []
    for root, dirs, files in os.walk(root_dir):
        for file in files:
            if file.endswith('.hpp'):
                hpp_files.append(os.path.join(root, file))
    return hpp_files

def find_all_source_files(root_dir):
    """Find all .cpp, .h, and .hpp files that might contain includes"""
    source_files = []
    for root, dirs, files in os.walk(root_dir):
        for file in files:
            if file.endswith(('.cpp', '.h', '.hpp', '.c', '.cc', '.cxx')):
                source_files.append(os.path.join(root, file))
    return source_files

def update_includes_in_file(file_path, hpp_to_h_map):
    """Update all include statements in a file"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        original_content = content

        # Update include statements
        for hpp_file, h_file in hpp_to_h_map.items():
            # Match various include patterns
            patterns = [
                (rf'#include\s+"([^"]*){re.escape(hpp_file)}"', rf'#include "\1{h_file}"'),
                (rf'#include\s+<([^>]*){re.escape(hpp_file)}>', rf'#include <\1{h_file}>'),
            ]

            for pattern, replacement in patterns:
                content = re.sub(pattern, replacement, content)

        # Only write if content changed
        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        return False
    except Exception as e:
        print(f"Error processing {file_path}: {e}")
        return False

def rename_hpp_files(root_dir):
    """Rename all .hpp files to .h"""
    # Find all .hpp files
    hpp_files = find_all_hpp_files(root_dir)

    if not hpp_files:
        print("No .hpp files found!")
        return

    print(f"Found {len(hpp_files)} .hpp files")

    # Create mapping of old names to new names
    hpp_to_h_map = {}
    for hpp_file in hpp_files:
        # Get just the filename
        basename = os.path.basename(hpp_file)
        new_basename = basename.replace('.hpp', '.h')
        hpp_to_h_map[basename] = new_basename

    print("\nFiles to rename:")
    for old, new in hpp_to_h_map.items():
        print(f"  {old} -> {new}")

    # Update all includes in source files first
    print("\nUpdating include statements...")
    source_files = find_all_source_files(root_dir)
    updated_count = 0

    for source_file in source_files:
        if update_includes_in_file(source_file, hpp_to_h_map):
            updated_count += 1
            print(f"  Updated: {source_file}")

    print(f"\nUpdated {updated_count} files with new includes")

    # Now rename the actual files
    print("\nRenaming files...")
    renamed_count = 0
    for hpp_file in hpp_files:
        h_file = hpp_file.replace('.hpp', '.h')
        try:
            os.rename(hpp_file, h_file)
            print(f"  Renamed: {os.path.basename(hpp_file)} -> {os.path.basename(h_file)}")
            renamed_count += 1
        except Exception as e:
            print(f"  Error renaming {hpp_file}: {e}")

    print(f"\nRenamed {renamed_count} files")
    print("\nDone! Please verify the changes and update CMakeLists.txt if needed.")

if __name__ == "__main__":
    # Get the repository root (parent of scripts directory)
    script_dir = Path(__file__).parent
    repo_root = script_dir.parent
    source_dir = repo_root / "source"

    print(f"Repository root: {repo_root}")
    print(f"Source directory: {source_dir}")
    print("\nStarting .hpp to .h conversion...")
    print("=" * 60)

    rename_hpp_files(str(source_dir))

    print("\n" + "=" * 60)
    print("Conversion complete!")
    print("\nNext steps:")
    print("1. Review the changes with: git diff")
    print("2. Update CMakeLists.txt (source/CMakeLists.txt)")
    print("3. Test build: cmake --build --preset windows-debug")
    print("4. Update CLAUDE.md if it references .hpp files")
