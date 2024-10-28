
import os
import shutil
import glob



def copy_pyd_files(src_dir, dst_dir):
    os.makedirs(dst_dir, exist_ok=True)

    for src_file in glob.glob(os.path.join(src_dir, '*.pyd')):
        # 获取文件名
        filename = os.path.basename(src_file)
        # 构建目标文件路径
        dst_file = os.path.join(dst_dir, filename)

        # 复制文件
        shutil.copy2(src_file, dst_file)
        print(f"Copied {src_file} to {dst_file}")


src = os.path.join(os.getcwd(), "cmake-build-debug\\bin")
dst = os.path.join(os.getcwd(), "src\\python\\vision")
print(src)
print(dst)
copy_pyd_files(src, dst)