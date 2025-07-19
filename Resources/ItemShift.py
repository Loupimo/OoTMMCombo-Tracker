import csv

input_file = "item.csv"   # Ton fichier source
output_file = "output.csv" # Le fichier résultat

with open(input_file, "r", newline='', encoding='utf-8') as csvfile_in, \
     open(output_file, "w", newline='', encoding='utf-8') as csvfile_out:

    reader = csv.reader(csvfile_in)
    writer = csv.writer(csvfile_out)
    currnum = 1
    for row in reader:
        if not row:
            continue  # Ignore les lignes vides

        new_hex = "{ " f"0x{currnum:X}"  # hex en majuscules

        # Remplacer la première colonne
        #if currnum < 451:
        #    row[0] = new_hex + ", " + row[0] +  " (OoT) }"
        #else:
        #    row[0] = new_hex + ", " + row[0] +  " (MM) }"
        
        fin_row = new_hex + ', \"' + row[0] + "\" },\n"
        #print(fin_row)
        csvfile_out.write(fin_row)
        #writer.writerow(fin_row)
        currnum = currnum + 1

print(f"Fichier écrit : {output_file}")