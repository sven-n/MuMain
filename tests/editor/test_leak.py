import sys
import os

def main():
    if len(sys.argv) < 2:
        print("Usage: test_leak.py <sources_file>")
        sys.exit(1)
        
    sources_file = sys.argv[1]
    
    with open(sources_file, 'r') as f:
        sources = f.read().splitlines()
        
    leaked_files = [s for s in sources if 'MuEditor' in s]
    
    if leaked_files:
        print("FAIL: MuEditor leaked into the build! The following editor files were found in the Main target sources:")
        for f in leaked_files:
            print(f"  - {f}")
        sys.exit(1)
    else:
        print("PASS: No MuEditor files leaked into the build.")
        sys.exit(0)

if __name__ == '__main__':
    main()
