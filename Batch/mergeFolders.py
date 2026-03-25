"""
Merge matching simulation output files from two folder hierarchies.

This script merges ROOT files from folderA/x and folderB/y into folderC/z,
matching by Light Yield (LY) parameter. Files with the same name in matching
folders are merged after checking parameter consistency.

Usage:
    # Full hierarchy merge (match by LY):
    python mergeFolders.py
    
    # Direct subfolder merge (two specific subfolders):
    python mergeFolders.py --direct folderA/x folderA/y outputFolder/z

Configuration:
    Set folderA, folderB, and folderC paths below for full hierarchy mode.
"""

import os
import sys
import glob
import shutil
import argparse
from tqdm import tqdm

import ROOT
import uproot

# Add helpers to path
current_directory = os.path.dirname(os.path.abspath(__file__))
sys.path.append(current_directory)

from helpers.G4_runClasses import FullMCRun, RunParameters, PhysicsParameters, DetectorParameters, build_mac_from_items
from helpers.FileGetters import get_all_root_files, get_all_mac_files

# -- Configuration --
path = os.getenv("CF4G4")
if path is None:
    path = "/Users/austindestcroix/Desktop/school_holders/projects/sbc/g4cf4_newOptics/"

# SET THESE TO THE FOLDERS TO MERGE (for full hierarchy mode)
folderA = path + "batchOutput/gammaLP_Jan26/"
folderB = path + "batchOutput/gammaLP_Jan26_moreStats/"
folderC = path + "batchOutput/gammaLP_Jan26_merged/"

verbose = True


def check_root_valid(rootPath):
    """Check if a ROOT file has the 'simOutput' tree."""
    try:
        f = uproot.open(rootPath)
        tree = f["simOutput"]
        return True
    except:
        return False


def get_ly_for_folder(folderPath):
    """
    Get the Light Yield (LY) parameter for a folder by reading any .mac file in it.
    Returns (ly, lyUnit) tuple or (None, None) if not found.
    """
    macFiles = get_all_mac_files(folderPath if folderPath.endswith('/') else folderPath + '/')
    if len(macFiles) == 0:
        return None, None
    
    # Read LY from the first mac file
    physPars = PhysicsParameters(macPath=macFiles[0])
    return physPars.ly, physPars.lyUnit


def get_numbered_subfolders(parentFolder):
    """Get list of numbered subfolders (e.g., 0/, 1/, 2/, etc.)"""
    subfolders = []
    if not os.path.exists(parentFolder):
        return subfolders
    
    for item in os.listdir(parentFolder):
        itemPath = os.path.join(parentFolder, item)
        if os.path.isdir(itemPath):
            # Check if folder name is a number
            try:
                int(item)
                subfolders.append(itemPath)
            except ValueError:
                continue
    
    return sorted(subfolders, key=lambda x: int(os.path.basename(x)))


def check_parameters_consistent(macPathA, macPathB):
    """
    Check if two .mac files have consistent physics parameters.
    Returns (consistent: bool, message: str)
    """
    physParsA = PhysicsParameters(macPath=macPathA)
    physParsB = PhysicsParameters(macPath=macPathB)
    
    runParsA = RunParameters(macPath=macPathA)
    runParsB = RunParameters(macPath=macPathB)
    
    detParsA = DetectorParameters(macPath=macPathA)
    detParsB = DetectorParameters(macPath=macPathB)
    
    # Check physics parameters match
    if physParsA != physParsB:
        return False, f"Physics parameters mismatch: A={physParsA}, B={physParsB}"
    
    # Check run parameters (except Nbeam and fname which will differ)
    if runParsA.gammaHeight != runParsB.gammaHeight:
        return False, f"gammaHeight mismatch: A={runParsA.gammaHeight}, B={runParsB.gammaHeight}"
    if runParsA.gammaSrc != runParsB.gammaSrc:
        return False, f"gammaSrc mismatch: A={runParsA.gammaSrc}, B={runParsB.gammaSrc}"
    if runParsA.alphaHeight != runParsB.alphaHeight:
        return False, f"alphaHeight mismatch: A={runParsA.alphaHeight}, B={runParsB.alphaHeight}"
    if runParsA.gammaAngle != runParsB.gammaAngle:
        return False, f"gammaAngle mismatch: A={runParsA.gammaAngle}, B={runParsB.gammaAngle}"
    if runParsA.lpFilter != runParsB.lpFilter:
        return False, f"lpFilter mismatch: A={runParsA.lpFilter}, B={runParsB.lpFilter}"
    if runParsA.voltage != runParsB.voltage:
        return False, f"voltage mismatch: A={runParsA.voltage}, B={runParsB.voltage}"
    
    # Detector parameters
    if detParsA != detParsB:
        return False, f"Detector parameters mismatch"
    
    return True, "OK"


def mergeTTrees_toPath(rootPathA, rootPathB, outputPath):
    """
    Merge two ROOT TTrees into a new output file.
    Returns the output path on success, None on failure.
    """
    try:
        # Open the input files
        file1 = ROOT.TFile(rootPathA, "READ")
        file2 = ROOT.TFile(rootPathB, "READ")
        
        if file1.IsZombie() or file2.IsZombie():
            print(f"  Error: Could not open ROOT files")
            return None
        
        # Get the TTrees from the input files
        tree1 = file1.Get("simOutput")
        tree2 = file2.Get("simOutput")
        
        if not tree1 or not tree2:
            print(f"  Error: Could not get simOutput tree")
            file1.Close()
            file2.Close()
            return None
        
        # Create output file and merge
        output_file = ROOT.TFile(outputPath, "RECREATE")
        
        tlist = ROOT.TList()
        tlist.Add(tree1)
        tlist.Add(tree2)
        merged_tree = ROOT.TTree.MergeTrees(tlist)
        
        # Write to output
        output_file.cd()
        merged_tree.Write()
        output_file.Close()
        
        # Close input files
        file1.Close()
        file2.Close()
        
        return outputPath
    except Exception as e:
        print(f"  Error merging TTrees: {e}")
        return None


def create_merged_mac(macPathA, macPathB, outputMacPath):
    """
    Create a new .mac file for the merged output.
    CRITICAL: Nbeam is set to the sum of Nbeam from both source files.
    """
    physPars = PhysicsParameters(macPath=macPathA)
    detPars = DetectorParameters(macPath=macPathA)
    runParsA = RunParameters(macPath=macPathA)
    runParsB = RunParameters(macPath=macPathB)
    
    # Sum the Nbeam values
    nbeamA = int(runParsA.Nbeam)
    nbeamB = int(runParsB.Nbeam)
    totalNbeam = nbeamA + nbeamB
    
    # Update runPars with new Nbeam and output path
    runParsA.Nbeam = totalNbeam
    runParsA.fname = outputMacPath.rstrip(".mac")
    
    # Build the new mac file
    build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runParsA, 
                         fname=runParsA.fname, savePath="")
    
    return totalNbeam, nbeamA, nbeamB


def merge_two_subfolders(subfolderA, subfolderB, outputFolder):
    """
    Directly merge two specific subfolders into an output folder.
    Files are matched by filename.
    """
    print("=" * 70)
    print("Direct Subfolder Merge")
    print("=" * 70)
    print(f"Subfolder A: {subfolderA}")
    print(f"Subfolder B: {subfolderB}")
    print(f"Output:      {outputFolder}")
    print("=" * 70)
    
    # Normalize paths
    subfolderA = subfolderA.rstrip('/')
    subfolderB = subfolderB.rstrip('/')
    outputFolder = outputFolder.rstrip('/')
    
    # Check source folders exist
    if not os.path.exists(subfolderA):
        print(f"Error: Subfolder A does not exist: {subfolderA}")
        sys.exit(1)
    if not os.path.exists(subfolderB):
        print(f"Error: Subfolder B does not exist: {subfolderB}")
        sys.exit(1)
    
    # Create output folder
    if not os.path.exists(outputFolder):
        os.makedirs(outputFolder)
        print(f"Created output folder: {outputFolder}")
    
    # Get root files from both folders
    rootFilesA = get_all_root_files(subfolderA + '/')
    rootFilesB = get_all_root_files(subfolderB + '/')
    
    print(f"\nFound {len(rootFilesA)} .root files in A")
    print(f"Found {len(rootFilesB)} .root files in B")
    
    # Build index of B files by filename
    bFileIndex = {os.path.basename(f): f for f in rootFilesB}
    
    # Track statistics
    totalMerged = 0
    totalCopied = 0
    totalSkipped = 0
    
    print("\n" + "-" * 70)
    print("Merging files...")
    print("-" * 70)
    
    for rootPathA in tqdm(rootFilesA, desc="Files"):
        rootFileName = os.path.basename(rootPathA)
        macPathA = rootPathA.replace(".root", ".mac")
        
        if not os.path.exists(macPathA):
            print(f"  Warning: No .mac file for {rootFileName}, skipping")
            totalSkipped += 1
            continue
        
        # Output paths
        outputRootPath = os.path.join(outputFolder, rootFileName)
        outputMacPath = outputRootPath.replace(".root", ".mac")
        
        # Look for matching file in B
        rootPathB = bFileIndex.get(rootFileName)
        
        if rootPathB is None:
            # No matching file in B, just copy from A
            if verbose:
                print(f"  No matching file in B for {rootFileName}, copying from A")
            shutil.copy2(rootPathA, outputRootPath)
            shutil.copy2(macPathA, outputMacPath)
            totalCopied += 1
            continue
        
        macPathB = rootPathB.replace(".root", ".mac")
        
        if not os.path.exists(macPathB):
            if verbose:
                print(f"  No .mac file in B for {rootFileName}, copying from A")
            shutil.copy2(rootPathA, outputRootPath)
            shutil.copy2(macPathA, outputMacPath)
            totalCopied += 1
            continue
        
        # Check ROOT file validity
        if not check_root_valid(rootPathA):
            print(f"  Warning: Invalid ROOT file {rootPathA}, skipping")
            totalSkipped += 1
            continue
        if not check_root_valid(rootPathB):
            print(f"  Warning: Invalid ROOT file {rootPathB}, copying only from A")
            shutil.copy2(rootPathA, outputRootPath)
            shutil.copy2(macPathA, outputMacPath)
            totalCopied += 1
            continue
        
        # Check parameter consistency
        consistent, msg = check_parameters_consistent(macPathA, macPathB)
        if not consistent:
            print(f"  Warning: Parameter mismatch for {rootFileName}: {msg}")
            print(f"    Copying only from A")
            shutil.copy2(rootPathA, outputRootPath)
            shutil.copy2(macPathA, outputMacPath)
            totalCopied += 1
            continue
        
        # Merge the ROOT files
        mergedPath = mergeTTrees_toPath(rootPathA, rootPathB, outputRootPath)
        
        if mergedPath is None:
            print(f"  Error: Failed to merge {rootFileName}, copying from A")
            shutil.copy2(rootPathA, outputRootPath)
            shutil.copy2(macPathA, outputMacPath)
            totalCopied += 1
            continue
        
        # Create merged .mac file with updated Nbeam
        totalNbeam, nbeamA, nbeamB = create_merged_mac(macPathA, macPathB, outputMacPath)
        
        if verbose:
            print(f"  Merged {rootFileName}: Nbeam={nbeamA}+{nbeamB}={totalNbeam}")
        
        totalMerged += 1
    
    # Summary
    print("\n" + "=" * 70)
    print("MERGE COMPLETE")
    print("=" * 70)
    print(f"Files merged:  {totalMerged}")
    print(f"Files copied:  {totalCopied} (no match in B)")
    print(f"Files skipped: {totalSkipped} (invalid or missing)")
    print(f"Output folder: {outputFolder}")
    print("=" * 70)
    
    return totalNbeam, nbeamA, nbeamB


def main():
    print("=" * 70)
    print("Merging Simulation Outputs")
    print("=" * 70)
    print(f"Folder A (source 1): {folderA}")
    print(f"Folder B (source 2): {folderB}")
    print(f"Folder C (output):   {folderC}")
    print("=" * 70)
    
    # Get numbered subfolders in A and B
    subfoldersA = get_numbered_subfolders(folderA)
    subfoldersB = get_numbered_subfolders(folderB)
    
    print(f"\nFound {len(subfoldersA)} subfolders in A")
    print(f"Found {len(subfoldersB)} subfolders in B")
    
    if len(subfoldersA) == 0:
        print("Error: No subfolders found in folder A")
        sys.exit(1)
    
    # Build LY index for folder B
    print("\nIndexing folder B by Light Yield...")
    lyIndexB = {}  # {ly: folderPath}
    for subB in subfoldersB:
        ly, lyUnit = get_ly_for_folder(subB)
        if ly is not None:
            lyIndexB[ly] = subB
            if verbose:
                print(f"  B/{os.path.basename(subB)}: LY={ly} {lyUnit}")
    
    print(f"Indexed {len(lyIndexB)} subfolders in B by LY")
    
    # Create output folder C if needed
    if not os.path.exists(folderC):
        os.makedirs(folderC)
        print(f"\nCreated output folder: {folderC}")
    
    # Track statistics
    totalMerged = 0
    totalCopied = 0
    totalSkipped = 0
    
    # Process each subfolder in A
    print("\n" + "-" * 70)
    print("Processing subfolders...")
    print("-" * 70)
    
    for subA in tqdm(subfoldersA, desc="Subfolders"):
        subfolderNum = os.path.basename(subA)
        lyA, lyUnitA = get_ly_for_folder(subA)
        
        if verbose:
            print(f"\n\nProcessing A/{subfolderNum} (LY={lyA} {lyUnitA})")
        
        # Find matching folder in B by LY
        matchingSubB = lyIndexB.get(lyA)
        
        # Create output subfolder
        outputSubfolder = os.path.join(folderC, subfolderNum)
        if not os.path.exists(outputSubfolder):
            os.makedirs(outputSubfolder)
        
        # Get all root files in A's subfolder
        rootFilesA = get_all_root_files(subA + '/')
        
        if len(rootFilesA) == 0:
            print(f"  Warning: No .root files in {subA}")
            continue
        
        for rootPathA in rootFilesA:
            rootFileName = os.path.basename(rootPathA)
            macPathA = rootPathA.replace(".root", ".mac")
            
            if not os.path.exists(macPathA):
                print(f"  Warning: No .mac file for {rootFileName}, skipping")
                totalSkipped += 1
                continue
            
            # Output paths
            outputRootPath = os.path.join(outputSubfolder, rootFileName)
            outputMacPath = outputRootPath.replace(".root", ".mac")
            
            # Check if we have a matching folder in B
            if matchingSubB is None:
                # No matching LY folder in B, just copy from A
                if verbose:
                    print(f"    No matching LY in B for {rootFileName}, copying from A")
                shutil.copy2(rootPathA, outputRootPath)
                shutil.copy2(macPathA, outputMacPath)
                totalCopied += 1
                continue
            
            # Look for matching file in B
            rootPathB = os.path.join(matchingSubB, rootFileName)
            macPathB = rootPathB.replace(".root", ".mac")
            
            if not os.path.exists(rootPathB) or not os.path.exists(macPathB):
                # No matching file in B, just copy from A
                if verbose:
                    print(f"    No matching file in B for {rootFileName}, copying from A")
                shutil.copy2(rootPathA, outputRootPath)
                shutil.copy2(macPathA, outputMacPath)
                totalCopied += 1
                continue
            
            # Check ROOT file validity
            if not check_root_valid(rootPathA):
                print(f"  Warning: Invalid ROOT file {rootPathA}, skipping")
                totalSkipped += 1
                continue
            if not check_root_valid(rootPathB):
                print(f"  Warning: Invalid ROOT file {rootPathB}, copying only from A")
                shutil.copy2(rootPathA, outputRootPath)
                shutil.copy2(macPathA, outputMacPath)
                totalCopied += 1
                continue
            
            # Check parameter consistency
            consistent, msg = check_parameters_consistent(macPathA, macPathB)
            if not consistent:
                print(f"  Warning: Parameter mismatch for {rootFileName}: {msg}")
                print(f"    Copying only from A")
                shutil.copy2(rootPathA, outputRootPath)
                shutil.copy2(macPathA, outputMacPath)
                totalCopied += 1
                continue
            
            # Merge the ROOT files
            mergedPath = mergeTTrees_toPath(rootPathA, rootPathB, outputRootPath)
            
            if mergedPath is None:
                print(f"  Error: Failed to merge {rootFileName}, copying from A")
                shutil.copy2(rootPathA, outputRootPath)
                shutil.copy2(macPathA, outputMacPath)
                totalCopied += 1
                continue
            
            # Create merged .mac file with updated Nbeam
            totalNbeam, nbeamA, nbeamB = create_merged_mac(macPathA, macPathB, outputMacPath)
            
            if verbose:
                print(f"    Merged {rootFileName}: Nbeam={nbeamA}+{nbeamB}={totalNbeam}")
            
            totalMerged += 1
    
    # Summary
    print("\n" + "=" * 70)
    print("MERGE COMPLETE")
    print("=" * 70)
    print(f"Files merged:  {totalMerged}")
    print(f"Files copied:  {totalCopied} (no match in B)")
    print(f"Files skipped: {totalSkipped} (invalid or missing)")
    print(f"Output folder: {folderC}")
    print("=" * 70)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Merge simulation output files from two folder hierarchies.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Full hierarchy merge (match subfolders by LY):
  python mergeFolders.py
  
  # Direct merge of two specific subfolders:
  python mergeFolders.py --direct path/to/folderA/0 path/to/folderA/1 path/to/output
  
  # Short form:
  python mergeFolders.py -d batchOutput/gammaLP_Jan26/5 batchOutput/gammaLP_Jan26/6 batchOutput/merged_test
        """
    )
    
    parser.add_argument(
        '--direct', '-d',
        nargs=3,
        metavar=('SUBFOLDER_A', 'SUBFOLDER_B', 'OUTPUT'),
        help='Directly merge two specific subfolders into output folder'
    )
    
    parser.add_argument(
        '--quiet', '-q',
        action='store_true',
        help='Reduce output verbosity'
    )
    
    args = parser.parse_args()
    
    if args.quiet:
        verbose = False
    
    if args.direct:
        # Direct subfolder merge mode
        subA, subB, outFolder = args.direct
        
        # Handle relative paths
        if not os.path.isabs(subA):
            subA = os.path.join(path, subA)
        if not os.path.isabs(subB):
            subB = os.path.join(path, subB)
        if not os.path.isabs(outFolder):
            outFolder = os.path.join(path, outFolder)
        
        merge_two_subfolders(subA, subB, outFolder)
    else:
        # Full hierarchy merge mode
        main()
