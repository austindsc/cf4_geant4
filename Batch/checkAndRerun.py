"""
Check quality/completion of batch simulation output and rerun any corrupted or empty runs.

Usage:
  python checkAndRerun.py <batchFolder>
  python checkAndRerun.py batchOutput/gammaLP_apr1_newSS/

For each subfolder (0/, 1/, ...), checks every .root file:
  1. Can it be opened by uproot?
  2. Does it contain the "simOutput" TTree?
  3. Does the TTree have entries (fNPE branch)?

Any failing files are listed and their corresponding .mac files are rerun with G4.

Use --dry-run to only print failures without rerunning.
Use --nproc N to set number of parallel workers (default 4).

python checkAndRerun.py batchOutput/gammaLP_apr1_newSS/ --dry-run

"""

import os
import sys
import glob
import subprocess
from multiprocessing import Pool
from tqdm import tqdm
import uproot
import argparse

from helpers.G4_runClasses import RunParameters

EXECNAME = "cf4Chamber"
path = os.getenv("CF4G4")
# PBUILD = path + "build/"

# PBUILD = path+"buildHighRef/"
PBUILD = path+"build/"


def check_root_file(rootFile, treeName="simOutput"):
  """
  Check if a .root file is valid and has a filled TTree.
  Returns (rootFile, status_string, num_entries).
  status_string is "ok" or an error description. num_entries is -1 on failure.
  """
  if not os.path.exists(rootFile):
    return rootFile, "missing", -1

  if os.path.getsize(rootFile) == 0:
    return rootFile, "empty file (0 bytes)", -1

  try:
    f = uproot.open(rootFile)
  except Exception as e:
    return rootFile, f"corrupted (cannot open): {e}", -1

  if treeName not in f:
    return rootFile, f"no '{treeName}' TTree", -1

  try:
    tree = f[treeName]
    nEntries = tree.num_entries
    if nEntries == 0:
      return rootFile, "TTree has 0 entries", 0
  except Exception as e:
    return rootFile, f"error reading TTree: {e}", -1
  return rootFile, "ok", nEntries

def run_process_quiet(command):
  subprocess.run(command, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
  return command

def run_process_verbose(command):
  subprocess.run(command, shell=True)
  return command

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Check and rerun corrupted/empty G4 simulation runs.")
  parser.add_argument("batchFolder", type=str, help="Path to the batch output folder (e.g. batchOutput/gammaLP_apr1_newSS/)")
  parser.add_argument("--dry-run", action="store_true", help="Only print failures, do not rerun.")
  parser.add_argument("--nproc", type=int, default=4, help="Number of parallel workers (default 4).")
  parser.add_argument("--verbose", action="store_true", help="Show G4 output during reruns.")
  args = parser.parse_args()

  batchFolder = args.batchFolder
  if not os.path.isabs(batchFolder):
    batchFolder = os.path.join(path, batchFolder)

  if not os.path.isdir(batchFolder):
    print(f"Error: {batchFolder} is not a directory.")
    sys.exit(1)

  # Find all .root files recursively
  root_files = sorted(glob.glob(os.path.join(batchFolder, "**/*.root"), recursive=True))

  # Find .mac files with no corresponding .root file
  mac_files = sorted(glob.glob(os.path.join(batchFolder, "**/*.mac"), recursive=True))
  root_basenames = {os.path.splitext(rf)[0] for rf in root_files}
  orphan_macs = [mf for mf in mac_files if os.path.splitext(mf)[0] not in root_basenames]

  if len(root_files) == 0 and len(orphan_macs) == 0:
    print(f"No .root or .mac files found in {batchFolder}")
    sys.exit(0)

  print(f"Found {len(root_files)} .root files in {batchFolder}")
  if orphan_macs:
    print(f"Found {len(orphan_macs)} .mac files with no .root file")
  print("Checking...\n")

  # Check each existing .root file
  failList = []
  allResults = []  # (rootFile, status, nEntries)
  for rootFile in root_files:
    rootFile, status, nEntries = check_root_file(rootFile)
    allResults.append((rootFile, status, nEntries))
    if status != "ok":
      failList.append((rootFile, status))
      print(f"  FAIL: {os.path.relpath(rootFile, batchFolder)}  —  {status}")

  # Add orphan .mac files (no .root) as failures
  for macFile in orphan_macs:
    rootFile = macFile.replace(".mac", ".root")
    allResults.append((rootFile, "no .root file (mac only)", -1))
    failList.append((rootFile, "no .root file (mac only)"))
    print(f"  FAIL: {os.path.relpath(rootFile, batchFolder)}  —  no .root file (mac only)")

  nGood = len(root_files) - len(failList)
  print(f"\nResults: {nGood}/{len(root_files)} OK, {len(failList)} failed.")

  # In dry-run mode, print a summary table of all runs
  if args.dry_run:
    print(f"\n{'File':<60} {'Status':<12} {'Nentries':>10} {'Nbeam':>10}")
    print("-" * 95)
    for rootFile, status, nEntries in allResults:
      macFile = rootFile.replace(".root", ".mac")
      nbeam_str = "—"
      if os.path.exists(macFile):
        try:
          rp = RunParameters(macPath=macFile)
          nbeam_str = str(int(float(rp.Nbeam)))
        except:
          nbeam_str = "err"
      relPath = os.path.relpath(rootFile, batchFolder)
      entries_str = str(nEntries) if nEntries >= 0 else "—"
      tag = "OK" if status == "ok" else "FAIL"
      print(f"  {relPath:<58} {tag:<12} {entries_str:>10} {nbeam_str:>10}")
    if len(failList) == 0:
      print("\nAll files are valid!")
    sys.exit(0)

  if len(failList) == 0:
    print("All files are valid!")
    sys.exit(0)

  # Build rerun commands
  geantCommandList = []
  for rootFile, status in failList:
    macFile = rootFile.replace(".root", ".mac")
    if not os.path.exists(macFile):
      print(f"  WARNING: no matching .mac file for {rootFile}, skipping rerun.")
      continue
    # remove the bad .root file first, then rerun
    g4Command = f"rm -f {rootFile}; cd {PBUILD}; ./{EXECNAME} {macFile}"
    geantCommandList.append(g4Command)

  if args.dry_run:
    print("\n--dry-run: would rerun the following commands:")
    for cmd in geantCommandList:
      print(f"  {cmd}")
    sys.exit(0)

  if len(geantCommandList) == 0:
    print("No commands to rerun (missing .mac files?).")
    sys.exit(0)

  # Execute reruns
  print(f"\nRerunning {len(geantCommandList)} simulations with {args.nproc} workers...")
  run_process = run_process_verbose if args.verbose else run_process_quiet

  try:
    with Pool(args.nproc) as pool:
      progress_bar = tqdm(total=len(geantCommandList), desc="Rerunning G4 sims")
      for result in pool.imap(run_process, geantCommandList):
        progress_bar.update(1)
      progress_bar.close()
  except KeyboardInterrupt:
    print("\n⚠️  Interrupted by user (Ctrl+C)")
    pool.terminate()
    pool.join()
    sys.exit(1)

  print("\n-------- Done rerunning failed simulations --------")
