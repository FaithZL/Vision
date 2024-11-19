# -*- coding: utf-8 -*-

import os
import zipfile

def zip_directory(dirname, zipfilename):
    with zipfile.ZipFile(zipfilename, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(dirname):
            for file in files:
                file_path = os.path.join(root, file)
                file_in_zip_path = os.path.relpath(file_path, os.path.dirname(dirname))
                zipf.write(file_path, file_in_zip_path)
                
                
def main():
    cur_path = os.path.dirname(__file__)
    print(cur_path)
    os.chdir(cur_path)
    dirname_to_compress = os.path.join(cur_path, "b2v")
    zip_filename = 'b2v.zip'  
    zip_directory(dirname_to_compress, zip_filename)

if __name__ == "__main__":
    main()
