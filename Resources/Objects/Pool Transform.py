import pandas as pd 

commonScenes = ["FAIRY_FOUNTAIN", "GROTTOS", "GORON_SHOP", "LOST_WOODS", "SHOOTING_GALLERY", "CUTSCENE_MAP", "TREASURE_SHOP", "LABORATORY", "SPIDER_HOUSE_SWAMP", "SPIDER_HOUSE_OCEAN"]
commonID = ["SONG_STORMS"]
excludeSpoiler = ["INSIDE_EGGS", "MARKET", "MOUNTAIN_VILLAGE", "TWIN_ISLANDS", "MOON", "GORON_SHRINE", "MILK_ROAD", "GORON_VILLAGE_WINTER", "ROMANI_RANCH"]


def wrap_cpp_file(outfile, content, game):
    if (game == "OOT_"):
        pragmaGame = "OOT"
    else:
        pragmaGame = "MM"

    header = "#pragma once\n\n/*\n*	IMPORTANT NOTE: This file should only be include one time as all of these object arrays are not constant / static and should exist only one time !\n*\t\t\t\t\tThey were part of Objects.cpp but were moved here for clarity and IDE lagging\n*\n*	Currently included by Objects.cpp\n*/\n\n#include \"Objects.h\"\n#include \"Scenes.h\"\n\n#pragma region " + pragmaGame
    outfile.write(header + "\n" + content + "\n" + "#pragma endregion")
    

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

            requierements = str(row["requierements"])
            if len(requierements) == 0 or requierements == "nan":
                requierements = None
            else:
                reqs = requierements.split(", ")
                requierements = "<b>Requirements:</b>"
                for req in reqs:
                    requierements += "<br>- " + req

            tooltip = str(row["tooltip"])
            if len(tooltip) == 0 or tooltip == "nan":
                if requierements is None:
                    tooltip = "NULL"
                else:
                    tooltip = "\"" + requierements + "\""
            else:
                tooltip = "\"" + tooltip + "<br><br>" + requierements + "\""
            
            objectstr = objectstr + "\t{ " + idstr + ", " + scenestr + ", \"" + str(row["location"]) + "\", ObjectType::" + str(row["type"]) + ", {" + str(row["x"]) + ", " + str(row["y"]) + ", " + str(row["z"]) + "}, " + renderscene + ", ObjectType::" + str(row["rendertype"]) + ", ObjectIconMap::" + str(row["icontype"]) + ", GameLayout::" + str(row['game_layout']) + ", LocType::" + str(row["loc_type"]) + ", " + tooltip + " }"
            outfile.write(objectstr)
        outfile.write("\n};")

def parse_file2(input_file, output_file, prefix):
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

            requierements = str(row["requierements"])
            if len(requierements) == 0 or requierements == "nan":
                requierements = None
            else:
                reqs = requierements.split(", ")
                requierements = "<b>Requirements:</b>"
                for req in reqs:
                    requierements += "<br>- " + req

            tooltip = str(row["tooltip"])
            if len(tooltip) == 0 or tooltip == "nan":
                if requierements is None:
                    tooltip = "NULL"
                else:
                    tooltip = "\"" + requierements + "\""
            elif requierements is not None:
                tooltip = "\"" + tooltip + "<br><br>" + requierements + "\""
            else:
                tooltip = "\"" + tooltip + "\""

            objectstr = "\t{ " + idstr + ", " + scenestr + ", \"" + str(row["friendly_name"]) + "\", \"" + str(row["location"]) + "\", ObjectType::" + str(row["type"]) + ", {" + str(int(row["x"])) + ", " + str(int(row["y"])) + ", " + str(int(row["z"])) + "}, " + renderscene + ", ObjectType::" + str(row["rendertype"]) + ", ObjectIconMap::" + str(row["icontype"]) + ", ObjectContext::" + str(row["context"]) + ", " + str(row["room"]) + ", GameLayout::" + str(row['game_layout']) + ", LocType::" + str(row["loc_type"]) + ", " + tooltip  + " }"
            fin[scenestr].append(objectstr)
            
            if renderscene != scenestr:
                if str(row["type"]) != "none":
                    if fin.__contains__(renderscene) == False:
                        fin[renderscene] = []
                    if objectstr not in fin[renderscene]:
                        fin[renderscene].append(objectstr)


            #outfile.write(objectstr)
        content = ""
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
            content += strb
#            outfile.write(strb)
        wrap_cpp_file(outfile, content, prefix)


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
            has_context = row["has_context"]
            active_layout = row["active_layout"]

            if game == "OOT":
                parent_region = "OoTRegions::" + parent_region
            else:
                parent_region = "MMRegions::" + parent_region

            objectstr = objectstr + "\t{ \"" + scenestr + "\", \"" + image_path + "\", (uint8_t) " + parent_region + ", " + str(has_context).lower() + ", GameLayout::" + active_layout + "}"
            outfile.write(objectstr)


def parse_entrance(input_file, output_file_h, output_file_cpp, output_filemeta, prefix):
    """Parse un fichier pour convertir les lignes en EntranceMetaInfo."""
    with open(input_file, 'r') as infile, open(output_file_h, 'w') as outHfile, open(output_file_cpp, 'w') as outCPPfile, open(output_filemeta, 'w') as outfilemeta:
        filereader = pd.read_csv(infile, delimiter=";", header=0, keep_default_na=False)
        isFirst = True
        scene_entr_arr = {}
        scene_regions = {}
        objectstrings = []
        if prefix == "OoT":
            region_prefix = "OoTRegions::"
        else:
            region_prefix = "MMRegions::"
        defines = ""
        for i, row in filereader.iterrows():
            objectstr = ""
            if isFirst == False :
                objectstr = ",\n"
            else:
                isFirst = False

            regionstr = row["Region_Name"]
            entrance_code = row["Code_Name"]
            fromentridstr = row["From_ID"]
            toentridstr = row["To_ID"]
            fromsceneid = row["From_Scene"]
            tosceneid = row["To_Scene"]
            from_str = row["From_Name"]
            to_str = row["To_Name"]
            type = row["Type"]
            in_X = row["In_X"]
            in_Y = row["In_Y"]
            in_Z = row["In_Z"]
            out_X = row["Out_X"]
            out_Y = row["Out_Y"]
            out_Z = row["Out_Z"]
            arrow_rot = row["Arrow_Rot"]
            active_layout = row["Active_Layout"]

            objectstr = objectstr + "\t{ " + str(entrance_code) + ", { " + str(fromentridstr) + ", " + str(entrance_code) + ", " + str(fromsceneid) + ", " + str(tosceneid) + ", \"" + from_str + "\", \"" + to_str + "\", EntranceType::" + type + ", " + str(in_X) + ", " + str(in_Y) + ", " + str(in_Z) + ", " + str(out_X) + ", " + str(out_Y) + ", " + str(out_Z) + ", " + str(arrow_rot) + ", GameLayout::" + active_layout + " } }"
            objectstrings.append(objectstr)
            #outfile.write(objectstr)
            defines += "#define " + entrance_code + " " + toentridstr + "\n"


            if scene_entr_arr.__contains__(tosceneid) == False:
                scene_entr_arr[tosceneid] = []
                scene_regions[tosceneid] = region_prefix + regionstr
            
            scene_entr_arr[tosceneid].append(entrance_code)

        # write to header file
        outHfile.write("#pragma once\n\n#include \"Entrances.h\"\n")
        outHfile.write("\n#pragma region Defines\n\n")
        outHfile.write(defines)
        outHfile.write("\n#pragma endregion\n\nextern std::map<int, EntranceMetaInfo> " + prefix + "Entrances;\n")

        # write to cpp file
        outCPPfile.write("#include \"Combo/" + prefix + "Entrances.h\"\n#include \"Combo/Scenes.h\"\n\nstd::map<int, EntranceMetaInfo> " + prefix + "Entrances =\n{\n")

        for object in objectstrings:
            outCPPfile.write(object)

        outCPPfile.write("\n};\n")

        # write meta
        outfilemeta.write("#include \"UI/SceneEntrance.h\"\n#include \"Combo/Scenes.h\"\n\nstd::map<uint32_t, SceneEntranceMetaInf>" + prefix + "SceneEntranceMeta =\n{\n")
        scene_str = ""
        i = 0
        num_of_scenes = len (scene_entr_arr)
        for scene in scene_entr_arr:
            scene_str = "\t{\n\t\t" + str(scene) + ",\n\t\t{\n\t\t\t" + str(scene) + ", (uint8_t)" + scene_regions[scene] + ",\n\t\t\t{\n"
            for entr in scene_entr_arr[scene]:
                scene_str += "\t\t\t\t{ " + str (entr) + ", { UINT32_MAX, UINT32_MAX } },\n"
            scene_str += "\t\t\t},\n\t\t\tNULL\n\t\t}\n"
            scene_str += "\t}"
            if i < num_of_scenes:
                scene_str += ",\n"
            outfilemeta.write(scene_str)
            i += 1
            #print (scene_str)
        outfilemeta.write("\n};\n")
        


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

def parse_settings(input_file, output_file):
    """Parse un fichier pour convertir les lignes RGB en hexadécimal."""
    with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
        filereader = pd.read_csv(infile, delimiter=";", header=0)
        isFirst = True
        for i, row in filereader.iterrows():
            objectstr = ""
            if isFirst == False :
                objectstr = ",\n"
            else:
                isFirst = False

            log_name = row["log_name"]
            friendly_name = row["friendly_name"]
            type = row["type"]
            value = row["value"]

            if type == "boolean" or type == "shuffle":
                value = "ShuffleSetting::" + value

            objectstr = objectstr + "\t{ \"" + log_name + "\", { \"" + friendly_name + "\", ParamType::" + type + ", " + str(value) + " } }"
            outfile.write(objectstr)

# Exemple d'utilisation
input_file = '.\\pool_mm.csv'
#output_file = '.\\pool_mm.txt'
cpp_file = '..\\..\\Headers\\Combo\\MMObjectScene.h'
parse_file2(input_file, cpp_file, "MM_")
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{cpp_file}'.")
#
input_file = '.\\pool_oot.csv'
#output_file = '.\\pool_oot.txt'
cpp_file = '..\\..\\Headers\\Combo\\OoTObjectScene.h'
parse_file2(input_file, cpp_file, "OOT_")
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{cpp_file}'.")


input_file = '..\\Scenes\\scenes_oot.csv'
output_file = '..\\Scenes\\scenes_oot.txt'
parse_scene(input_file, output_file, "OOT")
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{output_file}'.")

input_file = '..\\Scenes\\scenes_mm.csv'
output_file = '..\\Scenes\\scenes_mm.txt'
parse_scene(input_file, output_file, "MM")
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{output_file}'.")


input_file = '..\\Scenes\\entrances_mm.csv'
#output_file = '..\\Scenes\\entrance_mm.txt'
h_file = '..\\..\\Headers\\Combo\\MMEntrances.h'
cpp_file = '..\\..\\Sources\\Combo\\MMEntrances.cpp'
output_file_meta = '..\\..\\Sources\\UI\\SceneMMEntrances.cpp'
parse_entrance(input_file, h_file, cpp_file, output_file_meta, "MM")
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{h_file}', '{cpp_file}', '{output_file_meta}'.")
#

input_file = '..\\Scenes\\entrances_oot.csv'
output_file = '..\\Scenes\\entrance_oot.txt'
h_file = '..\\..\\Headers\\Combo\\OoTEntrances.h'
cpp_file = '..\\..\\Sources\\Combo\\OoTEntrances.cpp'
output_file_meta = '..\\..\\Sources\\UI\\SceneOoTEntrances.cpp'
parse_entrance(input_file, h_file, cpp_file, output_file_meta, "OoT")
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{h_file}', '{cpp_file}', '{output_file_meta}'.")
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


input_file = '..\\Scenes\\settings.csv'
output_file = '..\\Scenes\\settings.txt'
parse_settings(input_file, output_file)
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{output_file}'.")