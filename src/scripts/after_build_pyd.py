
import os
import shutil
import glob
import sys
import subprocess

def copy_file(src_file, dst_dir, force=False):
    filename = os.path.basename(src_file)
    dst_file = os.path.join(dst_dir, filename)

    src_time = os.path.getmtime(src_file)
    if os.path.exists(dst_file) and not force:
        dst_time = os.path.getmtime(dst_file)
        if src_time > dst_time:
            shutil.copy2(src_file, dst_file)
            print(f"Copied {src_file} to {dst_file}")
    else:
        shutil.copy2(src_file, dst_file)
        print(f"Copied {src_file} to {dst_file}")
    

def copy_files(src_dir, dst_dir, force=False):
    os.makedirs(dst_dir, exist_ok=True)

    for src_file in glob.glob(os.path.join(src_dir, '*.pyd')):
        copy_file(src_file, dst_dir, force)
        
    for src_file in glob.glob(os.path.join(src_dir, '*.dll')):
        copy_file(src_file, dst_dir, force)
        

def generate_pyi(module_name):
    try:
        result = subprocess.run(
            [sys.executable, '-m', 'pybind11_stubgen', module_name, "-o", dst],
            capture_output=True,
            text=True,
            check=True
        )
        
        print('generate_pyi success Return Code:', result.returncode)
        print('Output:', result.stdout)
        print('Error:', result.stderr)
        
    except subprocess.CalledProcessError as e:
        print('An error occurred while generating .pyi file:')
        print('Return Code:', e.returncode)
        print('Output:', e.output)
        print('Error:', e.stderr)

args = sys.argv
if len(args) > 1:
    src = args[1]
else:
    src = "cmake-build-debug\\bin"
src = os.path.join(os.getcwd(), src)
dst = os.path.join(os.getcwd(), "src\\python\\cpplibs")

if not os.path.exists(dst):
    os.makedirs(dst)
os.chdir(dst)

print("from ", src)
print("to ",dst)

def read_config():
    fn = "last_src.txt"
    force = True
    if os.path.exists(fn):
        with open(fn, "r") as f:
            content = f.read()
            print("last src is", content)
            print("current src is", src)
            force = content != src
            f.close()
    else:
        print(fn, " is not exist")
            
    print("Force is ", force)
    with open(fn , "w") as f:
        f.write(src)
        f.close()
    return force
        

def main():
    force = read_config()
    copy_files(src, dst, force)
    os.environ['PYTHONPATH'] = dst
    generate_pyi("ocapi")
    generate_pyi("vsapi")

if __name__ == "__main__":
    main()
