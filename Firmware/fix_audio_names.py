import os

target_dir = "pregen_audio"
expected_names = [
    "1", "2", "3", "A", "4", "5", "6", "B", "7", "8", "9", "C", "POINT", "0", "POUND", "D",
    "DTMF1", "DTMF2", "DTMF3", "DTMFA", "DTMF4", "DTMF5", "DTMF6", "DTMFB", "DTMF7", "DTMF8", 
    "DTMF9", "DTMFC", "DTMFASTERISK", "DTMF0", "DTMFPOUND", "DTMFD"
]
# Note: filenames on disk should be {name}.wav

if not os.path.exists(target_dir):
    print(f"Error: Directory '{target_dir}' not found. Run this from the Firmware directory.")
    exit(1)

files = os.listdir(target_dir)

print("--- Analysis ---")
for name in expected_names:
    expected_file = name + ".wav"
    if expected_file in files:
        print(f"OK: {expected_file} exists")
    else:
        # Try to find a partial match or fuzzy match
        found = False
        for f in files:
            # Check for "1 .wav" matching "1"
            # Logic: remove spaces from filename and compare?
            clean_f = f.replace(" ", "")
            if clean_f == expected_file:
                print(f"MISMATCH: Expected {expected_file}, found {f}. Renaming...")
                old_path = os.path.join(target_dir, f)
                new_path = os.path.join(target_dir, expected_file)
                try:
                    os.rename(old_path, new_path)
                    print(f"  -> Renamed {f} to {expected_file}")
                    found = True
                    break
                except Exception as e:
                    print(f"  -> ERROR renaming: {e}")
        
        if not found:
            # Check specific patterns like "DTMF 1 .wav" -> "DTMF1.wav"
            # My clean_f logic above handles "DTMF 1 .wav" -> "DTMF1.wav"
            print(f"MISSING: {expected_file}")

print("--- Done ---")
