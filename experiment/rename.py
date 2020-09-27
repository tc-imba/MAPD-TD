import os
import shutil

name = "result"
project_root = os.path.dirname(os.path.dirname(__file__))
experiment_dir = os.path.dirname(__file__)
result_src_dir = os.path.join(project_root, "result-new2")
result_dest_dir = os.path.join(project_root, "result")

os.makedirs(result_dest_dir, exist_ok=True)

for f in os.listdir(result_src_dir):
    row = f.split('-')
    row = row[:7] + ["0"] + row[7:]
    filename = '-'.join(row)
    shutil.copy(os.path.join(result_src_dir, f), os.path.join(result_dest_dir, filename))
