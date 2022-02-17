#!/usr/bin/env python3

import subprocess

def main():
    i = 0
    print("[script] Autorestart script is launched")
    while(True):
        process = subprocess.run(['../build/linux-socket-bfosb-x64'])
        i += 1
        print(f"[script] [{i}] Process has exited, return code: {process.returncode}")


if __name__ == '__main__':
    main()