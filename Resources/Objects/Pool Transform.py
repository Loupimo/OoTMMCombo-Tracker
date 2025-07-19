import pandas as pd 

commonScenes = ["FAIRY_FOUNTAIN", "GROTTOS", "GORON_SHOP", "LOST_WOODS", "SHOOTING_GALLERY", "CUTSCENE_MAP", "TREASURE_SHOP", "LABORATORY", "SPIDER_HOUSE_SWAMP", "SPIDER_HOUSE_OCEAN"]
commonID = ["SONG_STORMS"]
excludeSpoiler = ["INSIDE_EGGS", "MARKET", "MOUNTAIN_VILLAGE", "TWIN_ISLANDS", "MOON", "GORON_SHRINE", "MILK_ROAD", "GORON_VILLAGE_WINTER", "ROMANI_RANCH"]

def parse_file(input_file, output_file, arrayname, prefix):
    """Parse un fichier pour convertir les lignes RGB en hexadécimal."""
    with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
        filereader = pd.read_csv(infile, delimiter=";", header=0)
        outfile.write ("const ObjectInfo " + arrayname + "[" + str(len(filereader)) + "] =\n{\n")
        isFirst = True
        #print(filereader)
        for i, row in filereader.iterrows():
            objectstr = ""
            if isFirst == False :
                objectstr = ",\n"
            else:
                isFirst = False

            idstr = row["id"]
            if row["id"] in commonID:
                idstr = prefix + idstr
            scenestr = row["scene"]
            if row["scene"] in commonScenes:
                scenestr = prefix + scenestr
            renderscene = row["renderscene"]
            if row["renderscene"] in commonScenes:
                renderscene = prefix + renderscene
            
            objectstr = objectstr + "\t{ " + idstr + ", " + scenestr + ", \"" + str(row["location"]) + "\", ObjectType::" + str(row["type"]) + ", {" + str(row["x"]) + ", " + str(row["y"]) + "}, " + renderscene + ", ObjectType::" + str(row["rendertype"]) + ", ZGame::" + str(row['game']) + " }"
            outfile.write(objectstr)
        outfile.write("\n};")

def parse_file2(input_file, output_file, arrayname, prefix):
    """Parse un fichier pour convertir les lignes RGB en hexadécimal."""
    with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
        filereader = pd.read_csv(infile, delimiter=";", header=0)
        isFirst = True
        fin = {}
        #print(filereader)
        for i, row in filereader.iterrows():
            objectstr = ""
            if isFirst == False :
                objectstr = ",\n"
            else:
                isFirst = False

            idstr = row["id"]
            if row["id"] in commonID:
                idstr = prefix + idstr
            scenestr = row["scene"]
            if row["scene"] in commonScenes:
                scenestr = prefix + scenestr
            renderscene = row["renderscene"]
            if row["renderscene"] in commonScenes:
                renderscene = prefix + renderscene
            
            if fin.__contains__(scenestr) == False:
                fin[scenestr] = []

            objectstr = "\t{ " + idstr + ", " + scenestr + ", \"" + str(row["friendly_name"]) + "\", \"" + str(row["location"]) + "\", ObjectType::" + str(row["type"]) + ", {" + str(int(row["x"])) + ", " + str(int(row["y"])) + "}, " + renderscene + ", ObjectType::" + str(row["rendertype"]) + ", ObjectContext::" + str(row["context"]) + ", " + str(row["room"]) + ", ZGame::" + str(row['game']) + " }"
            fin[scenestr].append(objectstr)
            
            if renderscene != scenestr:
                if str(row["type"]) != "none":
                    if fin.__contains__(renderscene) == False:
                        fin[renderscene] = []
                    if objectstr not in fin[renderscene]:
                        fin[renderscene].append(objectstr)


            #outfile.write(objectstr)
        for r in fin:
            le = len(fin[r])
            i = 0
            strb = "\nconst size_t " + r + "NumOfObjs = " + str(le) + ";\nObjectInfo " + r + "SceneObjects [" + r + "NumOfObjs" + "] =\n{\n"
            #strb = "\nCreateObjectsForScene(" + r + ", " + str(le) + ",\n"
            for u in fin[r]:
                i = i + 1
                strb = strb + u 
                if i < le:
                    strb = strb + ",\n"
            strb = strb + "\n};\n"
            #strb = strb + ")\n"
            #print(strb)
            outfile.write(strb)

def parse_scene(input_file, output_file, game):
    """Parse un fichier pour convertir les lignes en SceneMetaInfo."""
    with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
        filereader = pd.read_csv(infile, delimiter=";", header=0, keep_default_na=False)
        isFirst = True
        for i, row in filereader.iterrows():
            objectstr = ""
            if isFirst == False :
                objectstr = ",\n"
            else:
                isFirst = False

            idstr = row["scene_id"]
            scenestr = row["scene_name"]
            image_path = row["image_path"]
            parent_region = row["parent_region"]

            if game == "OOT":
                parent_region = "OoTRegions::" + parent_region
            else:
                parent_region = "MMRegions::" + parent_region

            objectstr = objectstr + "\t{ \"" + scenestr + "\", \"" + image_path + "\", (uint8_t) " + parent_region + " }"
            outfile.write(objectstr)

def parse_items(input_file, output_file, arrayname):
    """Parse un fichier pour convertir les lignes RGB en hexadécimal."""
    with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
        filereader = pd.read_csv(infile, delimiter=";", header=0)
        outfile.write ("const ItemInfo " + arrayname + "[" + str(len(filereader)) + "] =\n{\n")
        isFirst = True
        unique_list = set ()
        #print(filereader)
        for i, row in filereader.iterrows():
            if unique_list.__contains__((row["Item ID"], row["Real Item"])) == False:
                unique_list.add((row["Item ID"], row["Real Item"]))
        
        sorted_list = sorted(unique_list, key=lambda x: int(x[0], 16))

        for item in sorted_list:
            objectstr = ""
            if isFirst == False :
                objectstr = ",\n"
            else:
                isFirst = False
            objectstr = objectstr + "\t{ 0x" + str(item[0]) + ", \"" + str(item[1]) + "\" }"
            outfile.write(objectstr)
        outfile.write("\n};")

def match_items(spoiler_log, input_file, output_file):
    """Parse un fichier pour convertir les lignes RGB en hexadécimal."""
    with open(input_file, 'r') as infile, open (spoiler_log, 'r') as spoiler, open(output_file, 'w') as outfile:
        filereader = pd.read_csv(infile, delimiter=";", header=0)
        spoilerreader = pd.read_csv(spoiler, delimiter=";", header=0)
        outfile.write ("Location;Guessed Item;Real Item;Item ID\n")
        outfile.flush()
        max_len = len(spoilerreader.index)
        startID = 0
        for i, row in filereader.iterrows():
            objectstr = ""
            found = False
            for j in range(startID, max_len):
                row_spoil = spoilerreader.iloc[j]
                if row_spoil["Location"] == row["Object"]:
                    objectstr = row_spoil["Location"] + ";" + row["Item"] + ";" + row_spoil["Item"] + ";" + row["ItemID"] + "\n"
                    startID = j
                    #spoilerreader.drop(j, inplace=True)
                    
                    #print("Found : " + row["Object"])
                    found = True
                    break
            
            if found == False:
                print("Not found : " + row["Object"])
            outfile.write(objectstr)
            outfile.flush()

# Exemple d'utilisation
input_file = 'D:\\Jeux\\Multi\\OoTMMCombo-Tracker\\Resources\\Objects\\pool_mm.csv'
output_file = 'D:\\Jeux\\Multi\\OoTMMCombo-Tracker\\Resources\\Objects\\pool_mm.txt'
parse_file2(input_file, output_file, "MMObjects", "MM_")
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{output_file}'.")
#
input_file = 'D:\\Jeux\\Multi\\OoTMMCombo-Tracker\\Resources\\Objects\\pool_oot.csv'
output_file = 'D:\\Jeux\\Multi\\OoTMMCombo-Tracker\\Resources\\Objects\\pool_oot.txt'
parse_file2(input_file, output_file, "OoTObjects", "OOT_")
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{output_file}'.")


input_file = 'D:\\Jeux\\Multi\\OoTMMCombo-Tracker\\Resources\\Scenes\\scenes_oot.csv'
output_file = 'D:\\Jeux\\Multi\\OoTMMCombo-Tracker\\Resources\\Scenes\\scenes_oot.txt'
#parse_scene(input_file, output_file, "OOT")
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{output_file}'.")

input_file = 'D:\\Jeux\\Multi\\OoTMMCombo-Tracker\\Resources\\Scenes\\scenes_mm.csv'
output_file = 'D:\\Jeux\\Multi\\OoTMMCombo-Tracker\\Resources\\Scenes\\scenes_mm.txt'
#parse_scene(input_file, output_file, "MM")
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{output_file}'.")
#
#input_file = 'D:\Emulation\OoTMMCombo-Tracker\Items.csv'
#output_file = 'D:\Emulation\OoTMMCombo-Tracker\Items.txt'
#parse_items(input_file, output_file, "ItemList")
#
#print(f"Conversion terminée. Les résultats sont enregistrés dans '{output_file}'.")

#spoiler_file = 'D:\Emulation\OoTMMCombo-Tracker\Logs\OoTMM-Spoiler-Coop.csv'
#input_file = 'D:\Emulation\OoTMMCombo-Tracker\Logs\Game_Log.csv'
#output_file = 'D:\Emulation\OoTMMCombo-Tracker\Logs\Output.csv'
#match_items(spoiler_file, input_file, output_file)


#input_file = 'D:\Emulation\OoTMMCombo-Tracker\Logs\Output.csv'
#output_file = 'D:\Emulation\OoTMMCombo-Tracker\Logs\Items.txt'
#parse_items(input_file, output_file, "ItemList")