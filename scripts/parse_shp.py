import png
import os
import glob

def convertpak(filename):
    bname = os.path.basename(filename)
    shpname = f"{bname}.SHP"
    palname = f"{bname}.PAL"
    with open(filename, "rb") as f:
        data = f.read(2)
        size = int.from_bytes(data, byteorder='little')
        if size == 0:
            return
        SHP = f.read(size-2)
        with open(shpname, "wb") as fo:
            fo.write(data+SHP)
        PAL = f.read(4)
        if PAL == b"FORM":
            size_b = f.read(4)
            size = int.from_bytes(size_b, byteorder='big')
            CHUNK = f.read(8)
            print(CHUNK)
            size_b = f.read(4)
            size = int.from_bytes(size_b, byteorder='big')
            width = 256
            height = 16
            img = []
            pal = []
            palet = []
            read = 0
            cpt = 0
            while read < size:
                r = f.read(1)
                v = f.read(1)
                b = f.read(1)
                read += 3
                pal.append(cpt)
                cpt += 1
                palet.append(
                    (
                        int.from_bytes(r),
                        int.from_bytes(v),
                        int.from_bytes(b)
                    )
                )
            print(len(palet))
            for i in range(height):
                row = []
                for j in pal:
                    row.append(j)
                img.append(row)
            w = png.Writer(size=(width, height), palette=palet, bitdepth=8)
            fpng = open(f"{palname}.png", "wb")
            w.write(fpng, img)
            fpng.close()
            os.system(f"WCToolsCmd.exe wcarm:pc:unmakeShape .\\{shpname} -palettefile:.\\{palname}.png")


for filename in glob.iglob('H:\\temp\\sc\\DATA\\MIDGAMES\\*.PCK'):
    convertpak(filename)
