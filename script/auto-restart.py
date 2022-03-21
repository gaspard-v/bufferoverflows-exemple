#!/usr/bin/env python3

import subprocess
import sys

def main():
    i = 0
    print("[script] Autorestart script is launched")
    while(True):
        process = subprocess.run([sys.argv[0]])
        i += 1
        print(f"[script] [{i}] Process has exited, return code: {process.returncode}")


if __name__ == '__main__':
    main()