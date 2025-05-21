import os
import shutil

# 定义要生成的字体配置
cf = 'RGB565A8'

def main():
    print(f"\n正在生成emoji，{cf}...")
    
    # 构建命令并执行
    cmd = f"python main/boards/bread-compact-wifi-dog/scrpits/font_emoji.py --type lvgl --cf {cf} --compress NONE"
    ret = os.system(cmd)
    
    if ret != 0:
        print(f"生成emoji失败")
    else:
        # 复制 build/emoji_*.c 到 src/emoji/*
        src_dir = "./build"
        dst_dir = "./main/boards/bread-compact-wifi-dog/emoji"
        if not os.path.exists(dst_dir):
            os.makedirs(dst_dir)
        else:
            # 清空目标目录中的所有文件
            for file in os.listdir(dst_dir):
                file_path = os.path.join(dst_dir, file)
                if os.path.isfile(file_path):
                    os.remove(file_path)
        
        for file in os.listdir(src_dir):
            if file.startswith("emoji_") and file.endswith(".c"):
                shutil.copy(os.path.join(src_dir, file), os.path.join(dst_dir, file))
        print(f"生成emoji成功")

if __name__ == "__main__":
    main()
