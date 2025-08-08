import io
import time
files = [
    "OPTIONS.IFF",
    "GAMEFLOW.IFF",
    "CONVDATA.IFF",
    "OBJVIEW.IFF",
    "OBJWORLD.IFF",
    "MIDGAMES.IFF",
    "MISN-3A.IFF",
    "MISN-2A.IFF",
    "ANDMAL1.IFF",
    "OP2MISN.IFF",
    "MISN2OP.IFF",
    "CREDITS.IFF",
    "PLAQUES.IFF",
    "NAVMAP.IFF",
    "PALETTE.IFF",
    "F16-CKPT.IFF",
    "SMOKESET.IFF",
    "ARENA.IFF",
]


def parse_iff_filereader(f, file_path, dec=0):
    while True:
        try:
            # Lire le code de type (4 octets)
            chunk_type = f.read(4)
            if not chunk_type:
                break  # Fin du fichier
            

            subType = {
                "OPTIONS.IFF":
                    [
                        b"OPTS",
                        b"ESTB",
                        b"MARK",
                        b"SHOT",
                        b"SCEN",
                        b"SHPS",
                        b"PALS",
                        b"BACK",
                        b"FORE",
                        b"SPRT",
                        b"EXTR"
                    ],
                "GAMEFLOW.IFF":
                    [
                        b"GAME",
                        b"WRLD",
                        b"MLST",
                        b"WNGS",
                        b"STAT",
                        b"MISS",
                        b"MAPS",
                        b"SCEN",
                        b"CHNG",
                        b"SPRT",
                        b"WING"
                    ],
                "CONVDATA.IFF":
                    [   
                        b"BCKS",
                        b"FACE",
                        b"FIGR",
                        b"PFIG",
                        b"FCPL",
                        b"FGPL",
                        b"BACK"
                    ],
                "OBJVIEW.IFF":
                    [
                        b"VIEW",
                    ],
                "OBJWORLD.IFF":
                    [
                        b"WRLD",
                        b"CAMR"
                    ],
                "MIDGAMES.IFF":
                    [
                        b"VOLU",
                        b"DATA",
                        b"SPOK",
                        b"MG01",
                        b"MG02",
                        b"MG05",
                        b"MG12",
                        b"MG15",
                        b"MG16",
                        b"MG17",
                        b"MG20",
                        b"MG33",
                        b"MG34",
                        b"MG36"
                    ],
                "MISN-3A.IFF":
                    [
                        b"MISN",
                        b"WRLD",
                        b"PLAY"
                    ],
                "MISN-2A.IFF":
                    [
                        b"MISN",
                        b"WRLD",
                        b"PLAY"
                    ],
                "ANDMAL1.IFF":
                    [
                        b"WRLD",
                        b"CAMR"
                    ],
                "OP2MISN.IFF":
                    [
                        b"INVN",
                        b"PLYR",
                        b"WEAP"
                    ],
                "MISN2OP.IFF":
                    [
                        b"INVN",
                        b"PLYR",
                        b"WEAP"
                    ],
                "CREDITS.IFF":
                    [
                        b"CRED",
                        b"PALT",
                        b"FONT"
                    ],
                "PLAQUES.IFF":
                    [
                        b"PLAQ",
                        b"FONT"
                    ],
                "NAVMAP.IFF":
                    [
                        b"NMAP",
                    ],
                "PALETTE.IFF":
                    [
                        b"PALR",
                    ],
                "F16-CKPT.IFF":
                    [
                        b"CKPT",
                        b"INST",
                        b"RAWS",
                        b"AIRS",
                        b"ALTI",
                        b"MONI",
                        b"CHUD",
                        b"REAL",
                        b"MWRN",
                    ],
                "SMOKESET.IFF":
                    [

                    ],
                "ARENA.IFF":
                    [
                        b"WRLD",
                        b"CAMR",
                    ],
            }

            if chunk_type in subType[file_path]:
                print(' ' * dec, f"TYPE: {chunk_type.decode()}")
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
                parse_iff_filereader(nextfile, file_path, dec+4)
            elif chunk_type == b"LOAD":
                nextfile = io.BytesIO(chunk_type+chunk_data)
                parse_basic_iff_chunk(nextfile, dec+2)
            elif chunk_type == b"BILD":
                nextfile = io.BytesIO(chunk_type+chunk_data)
                parse_basic_iff_chunk(nextfile, dec+2)
            elif chunk_type == b"SCNE":
                print_int_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"EFCT":
                print_efect_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"RECT":
                print_int_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"QUAD":
                print_int_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"SHAP":
                print_int_from_chunk(chunk_data, dec+2)
                extract_bin_to_file(chunk_data)
            elif chunk_type == b"MAPS":
                print_int_from_chunk(chunk_data, dec+2)
                extract_bin_to_file(chunk_data)
            elif chunk_type == b"PALT":
                print_int_from_chunk(chunk_data, dec+2)
                # extract_bin_to_file(chunk_data)
            elif chunk_type == b"SKYC":
                print_int_from_chunk(chunk_data, dec+2)
                extract_bin_to_file(chunk_data)
            elif chunk_type == b"MOBL":
                print_int_from_chunk(chunk_data, dec+2)
            elif chunk_size > 4:
                print(' ' * (dec+2), chunk_data)
            elif chunk_size == 2:
                # print(' ' * (dec+2),'VALUE = ', int.from_bytes(chunk_data, byteorder="big"))
                print(' ' * (dec+2),'VALUE = ', chunk_data[0])
                print(' ' * (dec+2),'VALUE = ', chunk_data[1])
            elif chunk_size == 4:
                # print(' ' * (dec+2),'VALUE = ', int.from_bytes(chunk_data, byteorder="big"))
                print(' ' * (dec+2),'VALUE = ', chunk_data[0])
                print(' ' * (dec+2),'VALUE = ', chunk_data[1])
                print(' ' * (dec+2),'VALUE = ', chunk_data[2])
                print(' ' * (dec+2),'VALUE = ', chunk_data[3])

            
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

def print_efect_from_chunk(chunk_data, dec):
    i = 0
    for c in chunk_data:
        if i == 0:
            print(' ' * dec, 'OPCODE', '0x{:02X}\t'.format(c), c, end='')
            i = 1
        elif i == 1:
            print(' ' * dec, 'VALUE','0x{:02X}\t'.format(c), c)
            i = 0


def extract_bin_to_file(chunk_data):
    timestampinms = int(time.time() * 1000)
    filename = f'{timestampinms}.bin'
    with open(filename, 'wb') as f:
        f.write(chunk_data)


parse_iff_file(files[17])
