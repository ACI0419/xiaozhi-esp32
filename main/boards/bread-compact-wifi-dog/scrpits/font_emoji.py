import os
import sys
import argparse
from LVGLImage import LVGLImage, ColorFormat, CompressMethod, OutputFormat
from PIL import Image

# map to emoji
emoji_mapping = {
    "neutral": 0x1f636, # 😶
    "happy": 0x1f642,    # 🙂
    "laughing": 0x1f606, # 😆
    "funny": 0x1f602,    # 😂
    "sad": 0x1f614,      # 😔
    "angry": 0x1f620,    # 😠
    "crying": 0x1f62d,   # 😭
    "loving": 0x1f60d,   # 😍
    "embarrassed": 0x1f633, # 😳
    "surprised": 0x1f62f,   # 😯
    "shocked": 0x1f631,     # 😱
    "thinking": 0x1f914,    # 🤔
    "winking": 0x1f609,     # 😉
    "cool": 0x1f60e,        # 😎
    "relaxed": 0x1f60c,     # 😌
    "delicious": 0x1f924,   # 🤤
    "kissy": 0x1f618,       # 😘
    "confident": 0x1f60f,   # 😏
    "sleepy": 0x1f634,      # 😴
    "silly": 0x1f61c,       # 😜
    "confused": 0x1f644     # 🙄
}


# download emoji from https://raw.githubusercontent.com/twitter/twemoji/refs/heads/master/assets/svg/1f644.svg
# to folder ./build
def get_emoji_png(emoji_utf8):
    if not os.path.exists("./build"):
        os.makedirs("./build")
    
    # 定义 emoji 图像目录
    emoji_dir = os.path.join("main", "boards", "bread-compact-wifi-dog", "emoji_img")
    
    # 将 Unicode 编码转换为对应的 emoji 名称
    reverse_emoji_mapping = {v: k for k, v in emoji_mapping.items()}
    emoji_name = reverse_emoji_mapping.get(int(emoji_utf8, 16))
    
    # 尝试获取目标 BMP 文件路径
    bmp_path = os.path.join(emoji_dir, f"{emoji_name}.bmp")
    
    # 若目标 BMP 文件不存在，使用 "neutral" 对应的图像
    if not os.path.exists(bmp_path):
        neutral_name = list(emoji_mapping.keys())[0]
        bmp_path = os.path.join(emoji_dir, f"{neutral_name}.bmp")
    
    # 生成 PNG 文件路径
    png_path = f"./build/emoji_{emoji_utf8}.png"
    
    if not os.path.exists(png_path):
        try:
            # 打开 BMP 图像并保存为 PNG
            with Image.open(bmp_path) as img:
                img.save(png_path, "PNG")
        except Exception as e:
            print(f"Error processing {bmp_path}: {e}")
    
    return png_path

def parse_arguments():
    parser = argparse.ArgumentParser(description='Emoji font converter utility')
    # parser.add_argument('--size', type=int, default=32, help='Emoji size in pixels (default: 32)')
    parser.add_argument('--type', choices=['png', 'lvgl'], default='png', help='Output type: png or lvgl')
    parser.add_argument('--cf', 
                       choices=['I1', 'I2', 'I4', 'I8', 'ARGB8888', 'RGB565A8'],
                       default='RGB565A8',
                       help='Color format for LVGL output (default: RGB565A8)')
    parser.add_argument('--compress',
                       choices=['NONE', 'RLE', 'LZ4'],
                       default='NONE',
                       help='Compression method for LVGL output (default: NONE)')
    return parser.parse_args()

def generate_lvgl_image(png_path, cf_str, compress_str):
    cf = ColorFormat[cf_str]
    compress = CompressMethod[compress_str]
    img = LVGLImage().from_png(png_path, cf=cf)
    
    # 生成 C 文件
    c_path = png_path.replace('.png', '.c')
    img.to_c_array(c_path, compress=compress)
    
    # 生成 bin 文件
    bin_path = png_path.replace('.png', '.bin')
    img.to_bin(bin_path, compress=compress)
    
    return c_path, bin_path

def main():
    args = parse_arguments()
    
    # 创建输出目录
    if not os.path.exists("./build/emoji"):
        os.makedirs("./build/emoji")
    
    # 处理每个表情符号
    for name, code in emoji_mapping.items():
        # 将十六进制代码转换为字符串格式
        emoji_utf8 = format(code, 'x')
        
        # 获取源文件
        png_path = get_emoji_png(emoji_utf8)
        
        if args.type == 'lvgl':
            # 生成 LVGL 图像文件
            c_path, bin_path = generate_lvgl_image(png_path, args.cf, args.compress)
            print(f"Generated LVGL files for {name}: {c_path}, {bin_path}")
        else:
            print(f"Generated PNG file for {name}: {png_path}")

if __name__ == "__main__":
    sys.exit(main())
