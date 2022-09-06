import os
from os import path

current_dir = os.getcwd()
files = [file for file in os.listdir(current_dir)
        if path.isfile(path.join(current_dir, file)) and file != "line_counter.py"] 

print(f"Files: {files}")
print(f"File count: {len(files)}")

line_count = 0

for filename in files:
    with open(filename, "r") as file:
        contents = file.readlines()

    line_count += len(contents)

print(f"Line count: {line_count}")
