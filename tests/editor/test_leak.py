import sys

def main():
    if len(sys.argv) < 2:
        print("Usage: test_leak.py <sources_file>")
        sys.exit(1)
        
    sources_file = sys.argv[1]
    
    with open(sources_file, 'r', encoding='utf-8') as f:
        sources = f.read().splitlines()
        
    # Normalise to forward slashes so the path-segment check is
    # platform-independent and won't fire just because the repository
    # happens to be cloned inside a directory called "MuEditor".
    leaked_files = [s for s in sources if '/MuEditor/' in s.replace('\\', '/')]
    
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
