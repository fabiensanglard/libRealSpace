import io
import time
import argparse

progs_map = {}
encode_progs_map = {}
parts = {}
parts_array = []
prof = {}
areas = {}
spots = {}
messages = {}
scenes = []

opcodes = {
    161: '[161] TAKE OFF FROM WAYPOINT ID',
    162: '[162] LAND TO WAYPOINT ID',
    165: '[165] FLY TO WAYPOINT ID',
    166: '[166] FLY TO WAY AREA ID',
    167: '[167] DESTROY TARGET ID',
    168: '[168] DEFEND ALLY ID',
    170: '[170] FOLLOW TARGET ID',
    171: '[171] PLAY MESSAGE ID',
    146: '[146] IF PART ID IS SAME AREA',
    148: '[148] INSTANT KILL TARGET ID',
    190: '[190] DE ACTIVATE AI ON TARGET',
    69: '[69 ] GET FLAG ID',
    65: '[65 ] FLAG INSTRUCTION 2',
    82: '[82 ] SET FLAG ID TO TRUE',
    70: '[70 ] IF OBJECTIVE IS FALSE GOTO',
    71: '[71 ] AI STATE INSTRUCTION 2',
    72: '[72 ] IF TRUE GOTO',
    73: '[73 ] IF FALSE GOTO',
    16: '[16 ] SOME SORT OF GOTO',
    8: '[8  ] CREATE LABEL'
}

def parse_iff_filereader(f, file_path, dec=0):
    while True:
        try:
            # Lire le code de type (4 octets)
            chunk_type = f.read(4)
            if not chunk_type:
                break  # Fin du fichier
            
            subType = [
                        b"MISN",
                        b"WRLD",
                        b"CAMR",
                        b"PLAY"
                    ]

            if chunk_type in subType:
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
            if chunk_type == b"CAST":
                parse_CAST_chunk(chunk_data, chunk_size, dec+2)
            if chunk_type == b"AREA":
                try:
                    parse_AREA_chunk(chunk_data, chunk_size, dec+2)
                except:
                    pass
            if chunk_type == b"PART":
                parse_PART_chunk(chunk_data, chunk_size, dec+2)
            if chunk_type == b"TEAM":
                print_int_from_chunk(chunk_data, dec+2)
            if chunk_type == b"LOAD":
                parse_LOAD_chunk(chunk_data, chunk_size, dec+2)
            if chunk_type == b"SCNE":
                parse_SCNE_chunk(chunk_data, chunk_size, dec+2)
            if chunk_type == b"FLAG":
                print_int_from_chunk(chunk_data, dec+2)
            if chunk_type == b"SPOT":
                parse_SPOT_chunk(chunk_data,chunk_size, dec+2)
            if chunk_type == b"PROG":
                print_prog_from_chunk(chunk_data, dec+2)
            if chunk_type == b"VERS":
                print_int_from_chunk(chunk_data, dec+2)
            if chunk_type == b"INFO":
                parse_NAME_chunk(chunk_data, chunk_size, dec+2)
            if chunk_type == b"TUNE":
                print_int_from_chunk(chunk_data, dec+2)
            if chunk_type == b"MSGS":
                parse_MSGS_chunk(chunk_data, chunk_size, dec+2)
            if chunk_type == b"NAME":
                parse_NAME_chunk(chunk_data, chunk_size, dec+2)
            if chunk_type == b"FILE":
                parse_NAME_chunk(chunk_data, chunk_size, dec+2)
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


def parse_SPOT_chunk(chunk_data, chunk_size, dec):
    nbspot = chunk_size / 14
    print(' ' * dec, f"Nombre de spot: {nbspot}")
    for i in range(int(nbspot)):
        offset = i*14
        areaid = chunk_data[offset]
        offset = offset + 1
        for j in range(2):
            print(' ' * dec, '0x{:02X}\t'.format(chunk_data[offset+j]), chunk_data[offset+j])
        offset = offset + 3
        XAxis = int.from_bytes(chunk_data[offset:offset+3], byteorder='little', signed=True)
        offset = offset + 4
        ZAxis = int.from_bytes(chunk_data[offset:offset+3], byteorder='little', signed=True)
        offset = offset + 4
        YAxis = int.from_bytes(chunk_data[offset:offset+2], byteorder='little', signed=True)
        offset = offset + 2
        print (' ' * dec, f"AREA {areaid}, X: {XAxis}, Y: {YAxis}, Z: {ZAxis}")
        spot = {
            'area': areaid,
            'X': XAxis,
            'Y': YAxis,
            'Z': ZAxis
        }
        spots[i] = spot
        

def parse_CAST_chunk(chunk_data, chunk_size, dec):
    nbcast = chunk_size / 9
    print(' ' * dec, f"Nombre de cast: {nbcast}")
    for i in range(int(nbcast)):
        offset = i*9
        
        name = chunk_data[offset:offset+8].decode().strip('\0')
        id = i
        print (' ' * dec, f"ID: {id}, Nom: {name}")


def parse_AREA_chunk(chunk_data, chunk_size, dec):
   
    offset = 0
    read = 0
    id = 0
    while offset < chunk_size:
        check = offset
        area_type = chunk_data[offset]
        offset += 1
        if area_type == ord('S'):
            try:
                area_name = chunk_data[offset:offset+33].decode().strip('\0')
            except UnicodeDecodeError:
                area_name = "ERROR UTF8 DECODE"
            offset += 33
            XAxis = int.from_bytes(chunk_data[offset:offset+3], byteorder='little', signed=True)
            offset += 4
            ZAxis = int.from_bytes(chunk_data[offset:offset+3], byteorder='little', signed=True)
            offset += 4
            YAxis = int.from_bytes(chunk_data[offset:offset+3], byteorder='little', signed=True)
            offset += 4
            AreaWidth = chunk_data[offset]
            ubyte = chunk_data[offset+1:offset+4]
            offset += 3
            print(' ' * dec, f"ID {id}, {area_name}, Area type: {chr(area_type)})")
            print(' ' * (dec+2), f"X: {XAxis}, Y: {YAxis}, Z: {ZAxis}, Width: {AreaWidth}")
            print(' ' * (dec+2), "remaining data", chunk_size-offset, offset, offset - check)
            for c in ubyte:
                print(' ' * (dec+4), '0x{:02X}\t'.format(c), c)
            area = {
                'name': area_name,
                'X': XAxis,
                'Y': YAxis,
                'Z': ZAxis,
                'width': AreaWidth,
                'type': area_type
            }
            areas[id] = area
            id = id + 1
            
        elif area_type == ord('C'):
            area_name = chunk_data[offset:offset+32].decode().strip('\0')
            offset += 32
            XAxis = int.from_bytes(chunk_data[offset:offset+3], byteorder='little', signed=True)
            offset += 4
            ZAxis = int.from_bytes(chunk_data[offset:offset+3], byteorder='little', signed=True)
            offset += 4
            YAxis = int.from_bytes(chunk_data[offset:offset+3], byteorder='little', signed=True)
            offset += 4
            AreaWidth = chunk_data[offset]
            offset += 1
            u0 = chunk_data[offset]
            offset += 1
            u1 = chunk_data[offset]
            offset += 1
            area_height = chunk_data[offset]   
            offset += 1
            u3 = chunk_data[offset]
            print(' ' * dec, f"ID: {id}, {area_name} Area type: {chr(area_type)})")
            print(' ' * (dec+2), f"X: {XAxis}, Y: {YAxis}, Z: {ZAxis}, Width: {AreaWidth}, Height: {area_height}")
            print(' ' * (dec+2), "remaining data", chunk_size-offset, offset, offset - check)
            area = {
                'name': area_name,
                'X': XAxis,
                'Y': YAxis,
                'Z': ZAxis,
                'width': AreaWidth,
                'height': area_height,
                'type': area_type
            }
            areas[id] = area
            id = id + 1
        elif area_type == ord('B'):
            area_name = chunk_data[offset:offset+33].decode().strip('\0')
            offset += 33
            XAxis = int.from_bytes(chunk_data[offset:offset+3], byteorder='little', signed=True)
            Xrelative_u = chunk_data[offset] + (chunk_data[offset+1] << 8) + (chunk_data[offset+2] << 16)
            Xrelative = Xrelative_u if not (Xrelative_u & 0x800000) else Xrelative_u - 0x1000000
        
            offset += 4
            ZAxis = int.from_bytes(chunk_data[offset:offset+3], byteorder='little', signed=True)
            Yrelative_u = chunk_data[offset] + (chunk_data[offset+1] << 8) + (chunk_data[offset+2] << 16)
            Yrelative = Yrelative_u if not (Yrelative_u & 0x800000) else Yrelative_u - 0x1000000
        
            offset += 4
            YAxis = int.from_bytes(chunk_data[offset:offset+3], byteorder='little', signed=True)
            offset += 4
            AreaWidth = chunk_data[offset]
            offset += 2
            ubyte = chunk_data[offset:offset+10]
            offset += 10
            area_height = chunk_data[offset]
            offset += 2
            ubytes2 = chunk_data[offset:offset+5]
            offset += 5
            print(' ' * dec, f"ID: {id}, {area_name} Area type: {chr(area_type)})")
            print(' ' * (dec+2), f"X: {XAxis}, Y: {YAxis}, Z: {ZAxis}, Width: {AreaWidth}, Height: {area_height}")
            print(' ' * (dec+2), f"Xrelativ (control): {Xrelative}, Zrelativ (control): {Yrelative}")
            print(' ' * (dec+2), "unknown bytes 1 : ")
            for c in ubyte:
                print(' ' * (dec+4), '0x{:02X}\t'.format(c), c)
            print(' ' * (dec+2), "unknown bytes 2 : ")
            for c in ubytes2:
                print(' ' * (dec+4), '0x{:02X}\t'.format(c), c)

            print(' ' * (dec+2), "remaining data", chunk_size-offset, offset, offset - check)
            area = {
                'name': area_name,
                'X': XAxis,
                'Y': YAxis,
                'Z': ZAxis,
                'width': AreaWidth,
                'height': area_height,
                'type': area_type
            }
            areas[id] = area
            id = id + 1
        else:
            print(' ' * dec, f"Unknown byte: {area_type}", '0x{:02X}\t'.format(area_type))


def parse_PART_chunk(chunk_data, chunk_size, dec):
    nbpart = chunk_size / 62
    print(' ' * dec, f"Nombre de part: {nbpart}")
    
    for i in range(int(nbpart)):
        current_part = {}
        offset = i*62
        id = chunk_data[offset] + (chunk_data[offset+1] << 8)
        offset = offset + 2
        print (' ' * dec, f"ID: {id}")
        current_part['id'] = id
        name = chunk_data[offset:offset+8].decode().strip('\0')
        current_part['name'] = name
        offset = offset + 8
        print (' ' * (dec+2), f"Nom: {name}")
        destroyed = chunk_data[offset:offset+8].decode().strip('\0')
        current_part['destroyed'] = destroyed
        offset = offset + 8
        print (' ' * (dec+2), f"destroyed: {destroyed}")
        load = chunk_data[offset:offset+8].decode().strip('\0')
        print (' ' * (dec+2), f"Load: {load}")
        offset = offset + 8
        for j in range(4):
            print(' ' * (dec+2), '0x{:02X}\t'.format(chunk_data[offset+j]), chunk_data[offset+j])
        unknown = chunk_data[offset] + (chunk_data[offset+1] << 8)
        unknown2 = chunk_data[offset+2] + (chunk_data[offset+3] << 8)
        unknown3 = chunk_data[offset] + (chunk_data[offset+1] << 8) + (chunk_data[offset+2] << 16) + (chunk_data[offset+3] << 24)
        print (' ' * (dec+2), f"Unknown: {unknown}, {unknown2}, {unknown3}")
        offset = offset + 4

        Xrelative_u = chunk_data[offset] + (chunk_data[offset+1] << 8) + (chunk_data[offset+2] << 16)
        Xrelative = Xrelative_u if not (Xrelative_u & 0x800000) else Xrelative_u - 0x1000000
        offset = offset + 4
        Yrelative_u = chunk_data[offset] + (chunk_data[offset+1] << 8) + (chunk_data[offset+2] << 16) 
        Zrelative = Yrelative_u if not (Yrelative_u & 0x800000) else Yrelative_u - 0x1000000
        offset = offset + 4
        Yrelative = chunk_data[offset] + (chunk_data[offset+1] << 8)
        print (' ' * (dec+2), f"Relative: {Xrelative}, {Yrelative}, {Zrelative}")
        offset = offset + 2
        current_part['position'] = (Xrelative, Yrelative, Zrelative)
        ub = chunk_data[offset]
        print (' ' * (dec+2), f"UB: {ub}")
        offset = offset + 1
        Azymuth = chunk_data[offset] + (chunk_data[offset+1] << 8)
        offset = offset + 2
        print (' ' * (dec+2), f"Azymuth: {Azymuth}")
        print (' ' * (dec+2), "remaining data", 62-(offset-(62*i+1)))
        for j in range((62-(offset-(62*i+1))-1)):
            print(' ' * (dec+4), '0x{:02X}\t'.format(chunk_data[offset+j]), chunk_data[offset+j])
        
        progs_offset = 11
        current_part['prog'] = []
        for j in range(4):
            prog = chunk_data[offset+progs_offset] + (chunk_data[offset+progs_offset+1] << 8)
            print(' ' * (dec+2), f"Program: {prog}")
            progs_offset = progs_offset + 2
            current_part['prog'].append(prog)
            if prog != 0xFFFF:
                for k in progs_map[prog]:
                    print (' ' * (dec+4), k)
        parts[id] = current_part
        parts_array.append(current_part)
        



def parse_NAME_chunk(chunk_data, chunk_size, dec):
    name = chunk_data.decode().strip('\0')
    print(' ' * dec, f"Value: {name}")

def parse_MSGS_chunk(chunk_data, chunk_size, dec):
    read = 0
    msg = []
    string_msg = ""
    while read < chunk_size:
        byte = chunk_data[read]
        if byte == 0:
            msg.append(string_msg.replace("\n","#"))
            string_msg = ""
        else:
            string_msg += chr(byte)
        read += 1
    for m in msg:
        print(' ' * dec, f"Message: {m}")

def parse_LOAD_chunk(chunk_data, chunk_size, dec):
    read = 0
    msg = []
    string_msg = ""
    while read < len(chunk_data):
        byte = chunk_data[read]
        if byte == 0:
            if len(string_msg) > 0:
                msg.append(string_msg.replace("\n","#"))
            string_msg = ""
        else:
            string_msg += chr(byte)
        read += 1
    for m in msg:
        print(' ' * dec, f"File: {m}")

def parse_iff_file(file_path):
    with open(file_path, 'rb') as f:
        parse_iff_filereader(f, file_path)


def parse_SCNE_chunk(chunk_data, chunk_size, dec):
    read = 0
    id = 0
    while read < chunk_size:
        active = chunk_data[read]
        print(' ' * dec, f"IS_ACTIVE: {active}")
        read += 1
        area_id = chunk_data[read] + (chunk_data[read+1] << 8)
        print(' ' * dec, f"Area ID: {area_id}")
        read += 2
        prog_id1 = chunk_data[read] + (chunk_data[read+1] << 8)
        prog_id2 = chunk_data[read+2] + (chunk_data[read+3] << 8)
        prog_id3 = chunk_data[read+4] + (chunk_data[read+5] << 8)
        print(' ' * dec, f"Prog ID: {prog_id1}, {prog_id2}, {prog_id3}")
        read += 6
        for i in range(15):
            print(' ' * dec, f"Unknown: {chunk_data[read+i]}")
        read += 15
        nbcast = int((chunk_size - read) / 2)
        print(' ' * dec, f"Nombre de cast: {nbcast}")
        cast = []
        for i in range(nbcast):
            cast_id = chunk_data[read+i*2] + (chunk_data[read+i*2+1] << 8)
            cast.append(cast_id)
            print(' ' * (dec+1), f"Cast ID: {cast_id}")
        scene = {
            'id': id,
            'active': active,
            'area_id': area_id,
            'prog': [prog_id1, prog_id2, prog_id3],
            'cast': cast
        }
        scenes.append(scene)
        read += nbcast*2

def print_int_from_chunk(chunk_data, dec):
    for c in chunk_data:
        print(' ' * dec, '0x{:02X}\t'.format(c), c)

def print_efect_from_chunk(chunk_data, dec):
    i = 0
    for c in chunk_data:
        if i == 0:
            print(' ' * dec, 'OPCODE', '0x{:02X}\t'.format(c), c, end='')
            opcode = f'OPCODE 0x{c} '+str(c)+' '
            i = 1
        elif i == 1:
            print(' ' * dec, 'VALUE','0x{:02X}\t'.format(c), c)
            i = 0
                
            

def print_prog_from_chunk(chunk_data, dec):
    
    progid = 0
    print (' ' * dec, '==== PROG ID', progid)
    i = 0
    progs = []
    encode_progs = []
    instruction = {}
    for c in chunk_data:
        if i == 0:
            if c in opcodes:
                opcode = opcodes[c]
            else:
                opcode = f'OPCODE '+'0x{:02X}'.format(c)+' '+str(c)+' '
            opc = c
            instruction['opcode'] = c
            print(' ' * (dec+2), opcode, end='')
            i = 1
        elif i == 1:
            print(' ' * (dec+2), 'VALUE','0x{:02X}\t'.format(c), c)
            i = 0
            instruction['value'] = c
            progs.append(opcode+' '+'0x{:02X}'.format(c)+' '+str(c)+' ')
            encode_progs.append(instruction)
            instruction = {}
            if opc == 0 and c == 0:
                if len(progs) > 0:
                    progs_map[progid] = progs
                    encode_progs_map[progid] = encode_progs
                    encode_progs = []
                    progs = []
                    progid = progid + 1
                    print (' ' * dec, '==== PROG ID', progid)


        

            

if __name__ == "__main__":
    arg_parser = argparse.ArgumentParser(description='Parse IFF Mission file')  
    arg_parser.add_argument('file', type=str, help='IFF Mission file to parse')
    args = arg_parser.parse_args()
    parse_iff_file(args.file)
    for p in parts:
        print(f"Part {p} {parts[p]['name']} {parts[p]['prog']}")
        for n in parts[p]['prog']:
            if n in encode_progs_map:
                for k in encode_progs_map[n]:
                    if k['opcode'] == 168:
                        print(f"   ** Defend ally {parts[k['value']]['name']}")
                    elif k['opcode'] == 167:
                        print(f"   ** Destroy target {parts[k['value']]['name']}")
                    elif k['opcode'] == 190:
                        if k['value'] in parts:
                            print(f"   -- deactivate {parts[k['value']]['name']}")
                        else:
                            print(f"   -- deactivate {k['value']}")
                    elif k['opcode'] == 148:
                        if k['value'] in parts:
                            print(f"   -- instant kill {parts[k['value']]['name']}")
                        else:
                            print(f"   -- instant kill {k['value']}")
                    elif k['opcode'] == 170:
                        print(f"   ** Follow target {parts[k['value']]['name']}")
                    elif k['opcode'] == 161:
                        if spots[k['value']]['area'] in areas:
                            print(f"   ** Take off from waypoint {k['value']}[{spots[k['value']]['X']}, {spots[k['value']]['Y']}, {spots[k['value']]['Z']}] - Area {areas[spots[k['value']]['area']]['name']}")
                        else:
                            print(f"   ** Take off from waypoint {k['value']}")
                    elif k['opcode'] == 162:
                        if spots[k['value']]['area'] in areas:
                            print(f"   ** Land to waypoint {k['value']}[{spots[k['value']]['X']}, {spots[k['value']]['Y']}, {spots[k['value']]['Z']}] - Area {areas[spots[k['value']]['area']]['name']}")
                        else:
                            print(f"   ** Land to waypoint {k['value']}")
                    elif k['opcode'] == 165:
                        if spots[k['value']]['area'] in areas:
                            print(f"   ** Fly to waypoint {k['value']}[{spots[k['value']]['X']}, {spots[k['value']]['Y']}, {spots[k['value']]['Z']}] - Area {areas[spots[k['value']]['area']]['name']}")
                        else:
                            print(f"   ** Fly to waypoint {k['value']}")
                    elif k['opcode'] == 166:
                        if k['value'] in spots:
                            if spots[k['value']]['area'] in areas:
                                print(f"   ** Fly to area {k['value']}[{spots[k['value']]['X']}, {spots[k['value']]['Y']}, {spots[k['value']]['Z']}] - Area {areas[spots[k['value']]['area']]['name']}")
                            else:
                                print(f"   ** Fly to area {k['value']}")
                        else:
                            print(f"   ** Fly to area {k['value']}")
                    elif k['opcode'] == 146:
                        if k['value'] in parts:
                            print(f"   -- if {parts[k['value']]['name']} is in same area")
                        else:
                            print(f"   -- if {k['value']} (not found in part) is in same area")
                    elif k['opcode'] in opcodes:
                        print(f"   {opcodes[k['opcode']]}\t{k['value']}")
                    else:
                        print(f"   UNDEC OPCODE :[{k['opcode']}]\t{k['value']}")
    id = 0
    for s in scenes:
        print(f"Scene {id} {s['active']}")
        if s['area_id'] in areas:
            print(f"   Area: {areas[s['area_id']]['name']}")
        else:
            print(f"   Area: {s['area_id']} Id not found")
        print("   Prog:")
        for p in s['prog']:
            if p in progs_map:
                print(f"    prog id {p}")
                for op in progs_map[p]:
                    print(f"     {op}")
                
        print("   Cast:")
        for c in s['cast']:
            print(f"     - {parts_array[c]['name']}")
            
        id += 1
