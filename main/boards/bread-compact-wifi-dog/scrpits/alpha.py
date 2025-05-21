import os
from PIL import Image

def white_to_transparent(input_path, output_path):
    img = Image.open(input_path).convert("RGBA")
    datas = img.getdata()

    new_data = []
    for item in datas:
        # 判断是否为白色像素
        if item[0] > 240 and item[1] > 240 and item[2] > 240:
            # 将白色像素设置为透明
            new_data.append((255, 255, 255, 0))
        else:
            new_data.append(item)

    img.putdata(new_data)
    img.save(output_path, "PNG")

def process_emoji_folder():
    emoji_folder = ".\\main\\boards\\bread-compact-wifi-dog\\emoji_img_source"
    output_folder = ".\\main\\boards\\bread-compact-wifi-dog\\emoji_img"

    # 创建输出文件夹
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    # 遍历 emoji 文件夹中的 BMP 文件
    for root, dirs, files in os.walk(emoji_folder):
        for file in files:
            if file.lower().endswith('.bmp'):
                input_path = os.path.join(root, file)
                file_name, _ = os.path.splitext(file)
                output_path = os.path.join(output_folder, f"{file_name}.png")
                white_to_transparent(input_path, output_path)
                print(f"Processed {input_path} -> {output_path}")

if __name__ == "__main__":
    process_emoji_folder()
