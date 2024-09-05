import time

filename = "D:/Projects/perso/SCDATA/MUSICS/DATA/SOUND/GAMEFLOW.ADL"

# create empty byte array
xdirdata = bytearray()
isXdir = False
cpt=0
with open(filename, "rb") as f:
    for data in iter(lambda: f.read(4), b''):
        if data == b"FORM":
            size_b = f.read(4)
            size = int.from_bytes(size_b, byteorder='big')
            CHUNK = f.read(4)
            if CHUNK == b"XDIR":
                print(f"XDIR found at {f.tell()}")
                if not isXdir:
                    xdirdata = data+size_b+CHUNK
                    isXdir = True
                elif isXdir:
                    with open(f"{filename}_{cpt}.xmi", "wb") as fo:
                        fo.write(xdirdata)
                    cpt+=1
                    isXdir = False
                    xdirdata = data+size_b+CHUNK
            else:
                if isXdir:
                    xdirdata += data+size_b+CHUNK
        else:
            xdirdata += data