import io
import time
import argparse
import struct


real_weap_map = {
    b"REAL": [
        b"OBJT",
        b"APPR",
    ],
    b"OBJT": [
        b"MISS",
        b"BOMB",
        b"TRCR",
        b"PODR",
        b"DURD"
    ],
    b"MISS": [
        b"DYNM",
    ],
    b"BOMB": [
        b"DYNM",
    ],
    b"DYNM": [],
    b"APPR": [
        b"POLY",
        b"TRCR"
    ],
    b"POLY": [
        b"DETA",
        b"TRIS"
    ],
}
def parse_wdata(f, dec):
    
    s1_value = struct.unpack('<h', f[0:2])
    s2_value = struct.unpack('<h', f[2:4])
    s3_value = struct.unpack('<h', f[4:6])  
    
    f1 = f[6]
    f2 = f[7]
    f3 = f[8]
    
    i1_value = struct.unpack('<i', f[9:13])
    f4 = f[13]
    i2_value = struct.unpack('<i', f[14:18])
    f5 = f[18]
    f6 = f[19]
    f7 = f[20]

    print(' ' * dec, f"Damage: {s1_value[0]}")
    print(' ' * dec, f"Radius of effect: {s2_value[0]}")
    print(' ' * dec, f"Short 3: {s3_value[0]}")
    print(' ' * dec, f"flag 1: {f1}")
    print(' ' * dec, f"flag 2: {f2}")
    print(' ' * dec, f"flag 3: {f3}")
    print(' ' * dec, f"target lock: {i1_value[0]}")
    print(' ' * dec, f"tracking cone: {f4}")
    print(' ' * dec, f"effective range: {i2_value[0]}")
    print(' ' * dec, f"flag 5: {f5}")
    print(' ' * dec, f"flag 6: {f6}")
    print(' ' * dec, f"flag 7: {f7}")


def parse_iff_filereader(f, file_path, dec=0, subType=[
                        b"REAL",
                    ]):
    curren_type = None
    while True:
        try:
            # Lire le code de type (4 octets)
            chunk_type = f.read(4)
            if not chunk_type:
                break  # Fin du fichier
            
            if chunk_type in subType:
                print(' ' * dec, f"TYPE: {chunk_type.decode()}")
                curren_type = chunk_type
                chunk_type = f.read(4)

            # Lire la taille (4 octets, big-endian)
            chunk_size = int.from_bytes(f.read(4), byteorder='big')
            if chunk_size % 2 != 0:
                chunk_size = chunk_size + 1
            # Lire les données (taille spécifiée)
            chunk_data = f.read(chunk_size)

            print(' ' * dec, f"Chunk: {chunk_type.decode()}, Taille: {chunk_size}")
            
            if chunk_type == b"FORM":
                nextfile = io.BytesIO(chunk_data)
                if curren_type in real_weap_map:
                    parse_iff_filereader(nextfile, file_path, dec+4, real_weap_map[curren_type])
                else:
                    parse_iff_filereader(nextfile, file_path, dec+4)
            elif chunk_type == b"MISS":
                nextfile = io.BytesIO(chunk_data)
                print_int_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"WDAT":
                parse_wdata(chunk_data, dec+2)
            elif chunk_type == b"DATA":
                print_int_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"DYNM":
                print_int_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"ATMO":
                print_int_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"DAMG":
                print_int_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"SIGN":
                print_int_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"TRGT":
                print_int_from_chunk(chunk_data, dec+2)
        except EOFError:
            break

def parse_basic_iff_chunk(f, dec):
    while True:
        try:
            # Lire le code de type (4 octets)
            chunk_type = f.read(4)
            if not chunk_type:
                break
            chunk_size = int.from_bytes(f.read(4), byteorder='big')
            if chunk_size % 2 != 0:
                chunk_size = chunk_size + 1
            chunk_data = f.read(chunk_size)
            print(' ' * dec, f"Chunk: {chunk_type.decode()}, Taille: {chunk_size}")
            print_int_from_chunk(chunk_data, dec+2)
        except EOFError:
            break


def parse_iff_file(file_path):
    with open(file_path, 'rb') as f:
        parse_iff_filereader(f, file_path)

def print_int_from_chunk(chunk_data, dec):
    for c in chunk_data:
        print(' ' * dec, '0x{:02X}\t'.format(c), c)



if __name__ == "__main__":
    arg_parser = argparse.ArgumentParser(description='Parse IFF Mission file')  
    arg_parser.add_argument('file', type=str, help='IFF Mission file to parse')
    args = arg_parser.parse_args()
    parse_iff_file(args.file)
