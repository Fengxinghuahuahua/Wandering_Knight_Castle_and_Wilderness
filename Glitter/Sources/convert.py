import os
import chardet  # 确保安装 chardet 库以检测文件编码

def convert_to_utf8(file_path):
    try:
        # 读取文件内容并检测当前编码
        with open(file_path, 'rb') as f:
            content = f.read()
            detected_encoding = chardet.detect(content)['encoding']

        # 如果文件编码不是 UTF-8，则进行转换
        if detected_encoding and detected_encoding.lower() != 'utf-8':
            print(f"Converting {file_path} from {detected_encoding} to UTF-8")
            content = content.decode(detected_encoding)
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
        else:
            print(f"{file_path} is already in UTF-8 or has unknown encoding")
    except Exception as e:
        print(f"Failed to convert {file_path}: {e}")

def convert_folder_to_utf8(folder_path):
    for root, _, files in os.walk(folder_path):
        for file in files:
            if file.endswith(('.cpp', '.c', '.h')):
                file_path = os.path.join(root, file)
                convert_to_utf8(file_path)

# 使用指定的文件夹路径调用该函数
folder_path = '.'
convert_folder_to_utf8(folder_path)
