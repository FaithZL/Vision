
import os
import shutil
import glob

def copy_file(src_file, dst_dir):
    filename = os.path.basename(src_file)
    dst_file = os.path.join(dst_dir, filename)

    src_time = os.path.getmtime(src_file)
    dst_time = os.path.getmtime(dst_file)
    
    if src_time > dst_time:
        shutil.copy2(src_file, dst_file)
        print(f"Copied {src_file} to {dst_file}")
    

def copy_files(src_dir, dst_dir):
    os.makedirs(dst_dir, exist_ok=True)

    for src_file in glob.glob(os.path.join(src_dir, '*.pyd')):
        copy_file(src_file, dst_dir)
        
    for src_file in glob.glob(os.path.join(src_dir, '*.dll')):
        copy_file(src_file, dst_dir)


src = os.path.join(os.getcwd(), "cmake-build-debug\\bin")
dst = os.path.join(os.getcwd(), "src\\python\\vision")
print(src)
print(dst)
copy_files(src, dst)