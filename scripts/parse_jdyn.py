
files = [
    "747.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "A-10.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "C130DES.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "C130GRN.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "F-4.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "F-15.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "F-16DES.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "F-16GRAY.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "F-18.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "F-22.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "LEARJET.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "MIG21.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "MIG29.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "MIRAGE.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "SU27.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "TORNCG.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "TU-20.REAL.OBJT.JETP.DYNM.JDYN.DAT",
    "YF23.REAL.OBJT.JETP.DYNM.JDYN.DAT"
]

for a in files:
    csv = "offset,byte,short,24bits,32bits\n"
    with open(f"H:/Originator/dumps/{a}", "rb") as f:
        chunk_size = 73
        chunk = f.read(chunk_size)
        
        for i in range(0,73):
            c1 = chunk[i]
            if i < chunk_size - 2:
                c2 = int.from_bytes(chunk[i:i+2], byteorder='little')
            if i < chunk_size - 3:
                c3 = int.from_bytes(chunk[i:i+3], byteorder='little')
            if i < chunk_size - 4:
                c4 = int.from_bytes(chunk[i:i+4], byteorder='little')
            csv = f"{csv}{i},{c1},{c2},{c3},{c4}\n"
    with open(f"./{a}.csv", "w") as csvf:
        csvf.write(csv) 