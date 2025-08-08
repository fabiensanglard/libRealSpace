import struct

def convert_bytes_to_float32(data: bytes) -> float:
    """
    Convertit un tableau de 4 octets en un float de précision simple (float32).
    
    Paramètres:
        data (bytes): Tableau d'octets de taille 4 encodé en little endian.
    
    Retourne:
        float: Le nombre flottant en précision simple.
    
    Remarque:
        La donnée doit être exactement 4 octets.
    """
    if len(data) != 4:
        raise ValueError("Les données doivent être un tableau de 4 octets")
    
    # Déballage du tableau d'octets en un float (little endian)
    return struct.unpack('<f', data)[0]

def convert_bytes_to_float(data: bytes, int_bits: int, dec_bits: int) -> float:
    """
    Convertit un tableau d'octets little endian en un nombre flottant.
    
    Paramètres:
        data (bytes): Tableau d'octets de 2 à 4 octets (16, 24 ou 32 bits).
        int_bits (int): Nombre de bits pour la partie entière.
        dec_bits (int): Nombre de bits pour la partie décimale.
    
    Retourne:
        float: Le nombre flottant résultant.
    
    Remarque:
        La somme int_bits + dec_bits doit être égale à la taille totale en bits dans data.
    """
    total_bits = len(data) * 8
    if int_bits + dec_bits != total_bits:
        raise ValueError("La somme des bits pour les parties entière et décimale doit correspondre à la taille totale des bits.")
    
    # On convertit les octets en un entier en utilisant le format little endian.
    total_value = int.from_bytes(data, byteorder='little', signed=False)
    
    # Extraire la partie entière en décalant la valeur décimale.
    integer_part = total_value >> dec_bits
    
    # Masque pour extraire les bits de la partie décimale.
    fractional_mask = (1 << dec_bits) - 1
    fractional_part = total_value & fractional_mask
    
    # Conversion de la partie décimale en nombre flottant.
    fractional_value = fractional_part / (1 << dec_bits)
    
    return integer_part + fractional_value


jdyn_parts = [
    "747.REAL.OBJT.JETP.DYNM.JDYN",
    "A-10.REAL.OBJT.JETP.DYNM.JDYN",
    "C130DES.REAL.OBJT.JETP.DYNM.JDYN",
    "C130GRN.REAL.OBJT.JETP.DYNM.JDYN",
    "F-4.REAL.OBJT.JETP.DYNM.JDYN",
    "F-15.REAL.OBJT.JETP.DYNM.JDYN",
    "F-16DES.REAL.OBJT.JETP.DYNM.JDYN",
    "F-16GRAY.REAL.OBJT.JETP.DYNM.JDYN",
    "F-18.REAL.OBJT.JETP.DYNM.JDYN",
    "F-22.REAL.OBJT.JETP.DYNM.JDYN",
    "LEARJET.REAL.OBJT.JETP.DYNM.JDYN",
    "MIG21.REAL.OBJT.JETP.DYNM.JDYN",
    "MIG29.REAL.OBJT.JETP.DYNM.JDYN",
    "MIRAGE.REAL.OBJT.JETP.DYNM.JDYN",
    "SU27.REAL.OBJT.JETP.DYNM.JDYN",
    "TORNCG.REAL.OBJT.JETP.DYNM.JDYN",
    "TU-20.REAL.OBJT.JETP.DYNM.JDYN",
    "YF23.REAL.OBJT.JETP.DYNM.JDYN"
]
thrs_part_data = [
    "747.REAL.OBJT.JETP.DYNM.THRS",
    "A-10.REAL.OBJT.JETP.DYNM.THRS",
    "C130DES.REAL.OBJT.JETP.DYNM.THRS",
    "C130GRN.REAL.OBJT.JETP.DYNM.THRS",
    "F-4.REAL.OBJT.JETP.DYNM.THRS",
    "F-15.REAL.OBJT.JETP.DYNM.THRS",
    "F-16DES.REAL.OBJT.JETP.DYNM.THRS",
    "F-16GRAY.REAL.OBJT.JETP.DYNM.THRS",
    "F-18.REAL.OBJT.JETP.DYNM.THRS",
    "F-22.REAL.OBJT.JETP.DYNM.THRS",
    "LEARJET.REAL.OBJT.JETP.DYNM.THRS",
    "MIG21.REAL.OBJT.JETP.DYNM.THRS",
    "MIG29.REAL.OBJT.JETP.DYNM.THRS",
    "MIRAGE.REAL.OBJT.JETP.DYNM.THRS",
    "SU27.REAL.OBJT.JETP.DYNM.THRS",
    "TORNCG.REAL.OBJT.JETP.DYNM.THRS",
    "TU-20.REAL.OBJT.JETP.DYNM.THRS",
    "YF23.REAL.OBJT.JETP.DYNM.THRS",
]
# Exemple d'utilisation
if __name__ == "__main__":
    for a in thrs_part_data:
        with open(f"H:/Originator/dumps/{a}.DAT", "rb") as f:
            data = f.read()
            csv_lines = [
                "offset,8bit,short,24bit,32bit, fixed_8.8, fixed_10.6, fixed_12.4, fixed_16.8, fixed_18.6, fixed_20.4, fixed_16.16,fixed_24.8,fixed_26.6,fixed_28.4, float32"
            ]
            for i in range(0, len(data), 1):
                v8 = data[i]
                v16 = 0
                v24 = 0
                v32 = 0
                v8_8 = 0
                v10_6 = 0
                v12_4 = 0
                v16_8 = 0
                v18_6 = 0
                v20_4 = 0
                v16_16 = 0
                v24_8 = 0
                v26_6 = 0
                v28_4 = 0
                vfloat = 0
                if i + 1 < len(data):
                    v16 = int.from_bytes(data[i:i+2], byteorder='little', signed=False)
                    v8_8 = convert_bytes_to_float(data[i:i+2], 8, 8)
                    v10_6 = convert_bytes_to_float(data[i:i+2], 10, 6)
                    v12_4 = convert_bytes_to_float(data[i:i+2], 12, 4)
                if i + 2 < len(data):
                    v24 = int.from_bytes(data[i:i+3], byteorder='little', signed=False)
                    v16_8 = convert_bytes_to_float(data[i:i+3], 16, 8)
                    v18_6 = convert_bytes_to_float(data[i:i+3], 18, 6)
                    v20_4 = convert_bytes_to_float(data[i:i+3], 20, 4)
                if i + 3 < len(data):
                    v32 = int.from_bytes(data[i:i+4], byteorder='little', signed=False)
                    v16_16 = convert_bytes_to_float(data[i:i+4], 16, 16)
                    v24_8 = convert_bytes_to_float(data[i:i+4], 24, 8)
                    v26_6 = convert_bytes_to_float(data[i:i+4], 26, 6)
                    v28_4 = convert_bytes_to_float(data[i:i+4], 28, 4)
                    vfloat = convert_bytes_to_float32(data[i:i+4])
                csv_lines.append(
                    f"{i}, {v8}, {v16}, {v24}, {v32}, {v8_8}, {v10_6}, {v12_4}, {v16_8}, {v18_6}, {v20_4}, {v16_16}, {v24_8}, {v26_6}, {v28_4}, {vfloat}"
                )
            with open(f"{a}.csv", "w") as csv_file:
                csv_file.write("\n".join(csv_lines))
            