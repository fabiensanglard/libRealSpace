import io
import time
files = [
    "GAMEFLOW.IFF",
]

changes = []
is_change_chunk_type = False
change_id = {}

def parse_iff_filereader(f, file_path, dec=0):
    global is_change_chunk_type
    while True:
        try:
            # Lire le code de type (4 octets)
            chunk_type = f.read(4)
            if not chunk_type:
                break  # Fin du fichier
            

            subType = {
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
            }
            if chunk_type == b"CHNG":
                is_change_chunk_type = True

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
            elif chunk_type == b"REQU":
                print_int_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"CASH":
                print_cach_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"OVER":
                print_cach_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"PILT":
                print_int_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"WEAP":
                print_weap_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"DATA":
                print_strings_list_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"PRTL":
                print_strings_list_return_from_chunk(chunk_data, dec+2)
            elif chunk_type == b"INFO":
                if is_change_chunk_type:
                    change_id[chunk_data[0]]=True
                    is_change_chunk_type = False
                print(' ' * (dec+2),'VALUE = ', chunk_data[0])
                print(' ' * (dec+2),'VALUE = ', chunk_data[1])
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


def print_cach_from_chunk(chunk_data, dec):
    op = chunk_data[0]
    print(' ' * dec, f"OP: {op}")
    amount = int.from_bytes(chunk_data[1:3], byteorder='little', signed=True)
    print(' ' * dec, f"Amount: {amount}")

def print_strings_list_from_chunk(chunk_data, dec):
    is_string = False
    thestring = ""
    len = 0
    strings = []
    print (" " * dec, "Chunk content: ")
    for c in chunk_data:
        print(' ' * (dec+2), '0x{:02X}\t'.format(c), c)
        if c != 0 and c>50:
            thestring += chr(c)
            len += 1
            is_string = True
            if len > 7:
                strings.append(thestring)
                thestring = ""
                is_string = False
                len = 0
        else:
            strings.append(thestring)
            thestring = ""
            is_string = False
            len = 0
    print(" " * dec, "Strings: ")
    for s in strings:
        try:
            print(' ' * (dec+2), s)
        except:
            pass

def print_strings_list_return_from_chunk(chunk_data, dec):
    is_string = False
    thestring = ""
    len = 0
    for c in chunk_data:
        # print(' ' * dec, '0x{:02X}\t'.format(c), c)
        try:
            if c != 0 and c != 10 and c >= 50:
                thestring += chr(c)
                len += 1
                is_string = True
            else:
                if is_string:
                    print(' ' * dec, thestring)
                thestring = ""
                is_string = False
                len = 0
        except:
            pass
def print_weap_from_chunk(chunk_data, dec):
    rec_size = 4
    offset = 0
    read = 0
    weapoin_index = {
        0: "AIM-9J",
        1: "AIM-9M",
        2: "AGM65-D",
        3: "DURANDAL",
        4: "MK-20",
        5: "MK-82",
        6: "GBU-15",
        7: "LAU-3",
        8: "AIM-120",

    }
    while read < len(chunk_data):
        op = chunk_data[offset]
        offset += 1
        print(' ' * dec, f"OP: {op}")
        weap_type = chunk_data[offset]
        offset += 1
        print(' ' * dec, f"Type: {weapoin_index[weap_type]}")
        weap_number = int.from_bytes(chunk_data[offset:offset+2], byteorder='little', signed=True)
        offset +=2
        read +=4
        print(' ' * dec, f"Number: {weap_number}")



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
    OPCODE = {
        0: "[OPCODE  0] PLAY CONV",
        1: "[OPCODE  1] PLAY SCENE",
        3: "[OPCODE  3] FLY MISSION AND GOTO NEXT MISSION",
        6: "[OPCODE  6] SET TRUE",
        7: "[OPCODE  7] SET FALSE",
        8: "[OPCODE  8] PLAY SHOT",
        9: "[OPCODE  9] IF NOT",
        10: "[OPCODE 10] IF",
        12: "[OPCODE 12] PLAY FLY MISSION",
        13: "[OPCODE 13] SET NEXT MISSION",
        15: "[OPCODE 15] GOTO NEXT MISSION",
        18: "[OPCODE 18] PLAY SHOW MAP",
        32: "[OPCODE 32] OTHER IF",
        30: "[OPCODE 30] ELSE",
        31: "[OPCODE 31] ENDIF",
        34: "[OPCODE 34] APPLY CHANGES",
    }
    change_op = False
    for c in chunk_data:
        if i == 0:
            if c in OPCODE:
                print(' ' * dec, OPCODE[c], end='')
                if c == 34:
                    change_op = True   
            else:
                print(' ' * dec, 'OPCODE', '0x{:02X}\t'.format(c), c, end='')
            i = 1
        elif i == 1:
            print(' ' * dec, 'VALUE','0x{:02X}\t'.format(c), c)
            if change_op:
                changes.append(c)
                change_op = False
            i = 0


def extract_bin_to_file(chunk_data):
    timestampinms = int(time.time() * 1000)
    filename = f'{timestampinms}.bin'
    with open(filename, 'wb') as f:
        f.write(chunk_data)


parse_iff_file(files[0])
print("VALIDATION OF CHANGE APPLY 34 OPCODE")
for c in changes:
    print(c)
    if c not in change_id:
        print("34 is not APPLY CHANGES", c)
print("VALIDATED")
