"""
Rerun simulations that need more statistics and merge the results.

For each .root file in the batch folder subfolders:
  1. Check if it passes selection criteria (LY range, minimum Nentries)
  2. If selected, rerun the associated .mac file to produce a second .root file
  3. Validate the new output
  4. Merge the two TTrees into one .root file
  5. Update the .mac file with the doubled Nbeam

Configuration is done via the variables below (no CLI args except --dry-run).

Usage:
  python newRerunAndMerge.py
  python newRerunAndMerge.py --dry-run
"""

import os
import sys
import glob
import subprocess
import shutil
from multiprocessing import Pool
from tqdm import tqdm
import uproot
import argparse

import ROOT

from helpers.G4_runClasses import RunParameters, PhysicsParameters, DetectorParameters, build_mac_from_items

# ==================== CONFIGURATION ====================

path = os.getenv("CF4G4")
PBUILD = path + "build/"
# pBuild = path+"buildHighRef/"
EXECNAME = "cf4Chamber"

# Batch folder to work in (relative to CF4G4 or absolute)
# BATCH_FOLDER = "batchOutput/gammaBare_2p6Base_june3/"

BATCH_FOLDER = "2batchOutput/gammaLP_jun12_128K/"


# Selection criteria: which runs to rerun
LY_MIN = 0.0    # minimum LY (keVLY) to include (None = no lower bound)
LY_MAX = None    # maximum LY (keVLY) to include (None = no upper bound)
MAX_ENTRIES = 4e5  # rerun if Nentries is LESS than this value
SOURCE_FILTER = None  # "Co57", "Cs137", or None for all sources

# Execution settings
NPROC = 4
VERBOSE_G4 = False  # sh


# ==================== END CONFIGURATION ====================

def check_root_file(rootFile, treeName="simOutput"):
  """
  Check if a .root file is valid and has a filled TTree.
  Returns (status_string, num_entries).
  """
  if not os.path.exists(rootFile):
    return "missing", -1
  if os.path.getsize(rootFile) == 0:
    return "empty file (0 bytes)", -1
  try:
    f = uproot.open(rootFile)
  except Exception as e:
    return f"corrupted: {e}", -1
  if treeName not in f:
    return f"no '{treeName}' TTree", -1
  try:
    tree = f[treeName]
    nEntries = tree.num_entries
    if nEntries == 0:
      return "TTree has 0 entries", 0
  except Exception as e:
    return f"error reading TTree: {e}", -1
  return "ok", nEntries


def get_ly_from_mac(macFile):
  """Extract the LY value from a .mac file. Returns (ly_value, ly_unit) or (None, None)."""
  try:
    physPars = PhysicsParameters(macPath=macFile)
    return physPars.ly, physPars.lyUnit
  except:
    return None, None


def passes_selection(ly, lyUnit, nEntries, rootFile=""):
  """Check if a run passes the LY range, Nentries, and source criteria."""
  if ly is None:
    return False
  if LY_MIN is not None and ly < LY_MIN:
    return False
  if LY_MAX is not None and ly > LY_MAX:
    return False
  if nEntries >= MAX_ENTRIES:
    return False
  if SOURCE_FILTER is not None and SOURCE_FILTER not in os.path.basename(rootFile):
    return False
  return True


def merge_root_files(rootFile1, rootFile2, outputFile, treeName="simOutput"):
  """
  Merge two ROOT TTrees into a single output file using PyROOT.
  Returns True on success, False on failure.
  """
  try:
    f1 = ROOT.TFile(rootFile1, "READ")
    f2 = ROOT.TFile(rootFile2, "READ")
    fout = ROOT.TFile(outputFile, "RECREATE")

    tree1 = f1.Get(treeName)
    tree2 = f2.Get(treeName)

    tlist = ROOT.TList()
    tlist.Add(tree1)
    tlist.Add(tree2)
    merged_tree = ROOT.TTree.MergeTrees(tlist)

    fout.cd()
    merged_tree.Write()
    fout.Close()
    f1.Close()
    f2.Close()
    return True
  except Exception as e:
    print(f"  ERROR merging {rootFile1} and {rootFile2}: {e}")
    return False


def run_process_quiet(command):
  subprocess.run(command, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
  return command

def run_process_verbose(command):
  subprocess.run(command, shell=True)
  return command


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Rerun and merge G4 simulations for more statistics.")
  parser.add_argument("--dry-run", action="store_true", help="Only print what would be done, do not execute.")
  args = parser.parse_args()

  # Resolve batch folder path
  batchFolder = BATCH_FOLDER
  if not os.path.isabs(batchFolder):
    batchFolder = os.path.join(path, batchFolder)
  if not os.path.isdir(batchFolder):
    print(f"Error: {batchFolder} is not a directory.")
    sys.exit(1)

  # Find all .root files recursively
  root_files = sorted(glob.glob(os.path.join(batchFolder, "**/*.root"), recursive=True))
  if len(root_files) == 0:
    print(f"No .root files found in {batchFolder}")
    sys.exit(0)

  print(f"Found {len(root_files)} .root files in {batchFolder}")
  print(f"Selection: LY in [{LY_MIN}, {LY_MAX}], Nentries < {MAX_ENTRIES:.0f}")
  print("Scanning...\n")

  # ---- Phase 1: Scan and select runs to rerun ----
  selectedRuns = []  # list of (rootFile, macFile, ly, nEntries, nbeam)

  headerFmt = f"  {'File':<55} {'LY':>6} {'Nentries':>10} {'Nbeam':>10} {'Selected':>10}"
  print(headerFmt)
  print("  " + "-" * 95)

  for rootFile in root_files:
    macFile = rootFile.replace(".root", ".mac")
    status, nEntries = check_root_file(rootFile)

    # Get LY and Nbeam from mac
    ly, lyUnit = get_ly_from_mac(macFile) if os.path.exists(macFile) else (None, None)
    try:
      rp = RunParameters(macPath=macFile)
      nbeam = int(float(rp.Nbeam))
    except:
      nbeam = -1

    ly_str = f"{ly:.1f}" if ly is not None else "—"
    entries_str = str(nEntries) if nEntries >= 0 else "—"
    nbeam_str = str(nbeam) if nbeam > 0 else "—"
    relPath = os.path.relpath(rootFile, batchFolder)

    selected = False
    if status == "ok" and os.path.exists(macFile) and passes_selection(ly, lyUnit, nEntries, rootFile):
      selected = True
      selectedRuns.append((rootFile, macFile, ly, nEntries, nbeam))

    tag = "YES" if selected else ""
    print(f"  {relPath:<55} {ly_str:>6} {entries_str:>10} {nbeam_str:>10} {tag:>10}")

  print(f"\n{len(selectedRuns)} runs selected for rerun out of {len(root_files)} total.\n")

  if len(selectedRuns) == 0:
    print("Nothing to do!")
    sys.exit(0)

  # ---- Phase 2: Dry-run check before creating any files ----
  if args.dry_run:
    print("--dry-run: would rerun and merge the following:\n")
    for rootFile, macFile, ly, nEntries, nbeam in selectedRuns:
      relOrig = os.path.relpath(rootFile, batchFolder)
      relV2 = os.path.relpath(rootFile[:-5] + "_V2.root", batchFolder)
      print(f"  {relOrig}  →  rerun as {relV2}")
    print(f"\n  Total: {len(selectedRuns)} G4 runs, then merge each pair.")
    sys.exit(0)

  # ---- Phase 3: Build G4 rerun commands ----
  # For each selected run, create a _V2 version of the mac with the same Nbeam
  rerunInfo = []  # (originalRoot, v2Root, v2Mac, macFile, g4Command)

  for rootFile, macFile, ly, nEntries, nbeam in selectedRuns:
    baseName = rootFile[:-5]  # strip ".root" (safe, we know it ends with .root)
    v2Name = baseName + "_V2"

    # Parse the original mac and create V2 mac with same physics, same Nbeam, different fname
    physPars = PhysicsParameters(macPath=macFile)
    detPars = DetectorParameters(macPath=macFile)
    runPars = RunParameters(macPath=macFile)
    runPars.fname = v2Name  # output to _V2 file

    v2Mac = build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars,
                                  fname=runPars.fname, savePath="", myVerbosity=0)
    v2Root = v2Name + ".root"

    g4Command = f"cd {PBUILD}; ./{EXECNAME} {v2Mac}"
    rerunInfo.append((rootFile, v2Root, v2Mac, macFile, g4Command))

  # ---- Phase 4: Execute G4 reruns ----
  print(f"Running {len(rerunInfo)} G4 simulations with {NPROC} workers...")
  run_process = run_process_verbose if VERBOSE_G4 else run_process_quiet
  g4Commands = [info[4] for info in rerunInfo]

  try:
    with Pool(NPROC) as pool:
      progress_bar = tqdm(total=len(g4Commands), desc="Rerunning G4 sims")
      for result in pool.imap(run_process, g4Commands):
        progress_bar.update(1)
      progress_bar.close()
  except KeyboardInterrupt:
    print("\n⚠️  Interrupted by user (Ctrl+C)")
    sys.exit(1)

  # ---- Phase 4: Validate V2 outputs and merge ----
  print("\nValidating new outputs and merging...")
  nMerged = 0
  nFailed = 0

  for originalRoot, v2Root, v2Mac, macFile, g4Command in rerunInfo:
    relOrig = os.path.relpath(originalRoot, batchFolder)

    # Validate the V2 output
    status, v2Entries = check_root_file(v2Root)
    if status != "ok":
      print(f"  FAIL: {os.path.relpath(v2Root, batchFolder)} — {status}. Skipping merge.")
      nFailed += 1
      continue

    # Merge original + V2 into a temporary file
    mergedTmp = originalRoot.replace(".root", "_MERGED_TMP.root")
    success = merge_root_files(originalRoot, v2Root, mergedTmp)
    if not success:
      nFailed += 1
      continue

    # Validate the merged output
    mergedStatus, mergedEntries = check_root_file(mergedTmp)
    if mergedStatus != "ok":
      print(f"  FAIL: merged file invalid — {mergedStatus}. Keeping originals.")
      os.remove(mergedTmp)
      nFailed += 1
      continue

    # All good — replace original with merged
    os.remove(originalRoot)       # delete original .root
    os.remove(v2Root)             # delete V2 .root
    shutil.move(mergedTmp, originalRoot)  # rename merged → original

    # Update the original .mac file with doubled Nbeam (do this before deleting V2 mac)
    runPars = RunParameters(macPath=macFile)
    physPars = PhysicsParameters(macPath=macFile)
    detPars = DetectorParameters(macPath=macFile)

    # V2 had the same Nbeam as original, so just double it
    runPars.Nbeam = int(float(runPars.Nbeam)) * 2

    # Rewrite the mac file with updated Nbeam (fname already correct from original mac)
    build_mac_from_items(physPar=physPars, detPar=detPars, runPar=runPars,
                          fname=runPars.fname, savePath="", myVerbosity=0)

    # Clean up V2 mac
    if os.path.exists(v2Mac):
      os.remove(v2Mac)

    print(f"  OK: {relOrig}  — merged {mergedEntries} entries, Nbeam now {int(runPars.Nbeam)}")
    nMerged += 1

  # ---- Summary ----
  print("\n" + "=" * 60)
  print(f"  Merged:  {nMerged}")
  print(f"  Failed:  {nFailed}")
  print(f"  Total:   {len(rerunInfo)}")
  print("=" * 60)
