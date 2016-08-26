import sys
from PIL import Image

def main():
    f = open(sys.argv[1], 'rb')
    data = f.read()
    f.close()

    use_bits = True

    BYTESPERLINE = 128
    BITSPERLINE = BYTESPERLINE * 8

    if use_bits:
        width = BITSPERLINE
        height = (len(data) * 8 + BITSPERLINE - 1) / BITSPERLINE
    else:
        width = BYTESPERLINE
        height = (len(data) + BYTESPERLINE - 1) / BYTESPERLINE

    img = Image.new('RGBA', (width, height))
    for y in xrange(height):
        if use_bits:
            for x in xrange(width):
                idx = y * BYTESPERLINE + (x / 8)
                bitidx = x % 8  # 0 = topmost

                if idx >= len(data):
                    img.putpixel((x,y), (255, 255, 255, 255))
                else:
                    v = ord(data[idx])
                    v = (v >> (7 - bitidx)) & 0x01
                    if v > 0:
                        v = 0
                    else:
                        v = 255
                    img.putpixel((x,y), (v, v, v, 255))
        else:
            for x in xrange(width):
                idx = y * BYTESPERLINE + x

                if idx >= len(data):
                    img.putpixel((x,y), (255, 255, 255, 255))
                else:
                    v = ord(data[idx])
                    img.putpixel((x,y), (v, v, v, 255))

    img.save(sys.argv[2])

if __name__ == '__main__':
    main()
