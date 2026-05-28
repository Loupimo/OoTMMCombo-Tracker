import pandas as pd 

commonScenes = ["FAIRY_FOUNTAIN", "GROTTOS", "GORON_SHOP", "LOST_WOODS", "SHOOTING_GALLERY", "CUTSCENE_MAP", "TREASURE_SHOP", "LABORATORY", "SPIDER_HOUSE_SWAMP", "SPIDER_HOUSE_OCEAN"]
commonID = ["SONG_STORMS"]
excludeSpoiler = ["INSIDE_EGGS", "MARKET", "MOUNTAIN_VILLAGE", "TWIN_ISLANDS", "MOON", "GORON_SHRINE", "MILK_ROAD", "GORON_VILLAGE_WINTER", "ROMANI_RANCH"]


def wrap_cpp_file(outfile, content, game):
    if (game == "OOT_"):
        pragmaGame = "OOT"
        includePrefix = "OoT"
    else:
        pragmaGame = "MM"
        includePrefix = "MM"

    #header = "#pragma once\n\n/*\n*	IMPORTANT NOTE: This file should only be include one time as all of these object arrays are not constant / static and should exist only one time !\n*\t\t\t\t\tThey were part of Objects.cpp but were moved here for clarity and IDE lagging\n*\n*	Currently included by Objects.cpp\n*/\n\n#include \"Objects.h\"\n#include \"Scenes.h\"\n\n#pragma region " + pragmaGame
    header = "#include \"Combo/" + includePrefix + "ObjectScene.h\"\n#include \"Combo/Objects.h\"\n#include \"Combo/Scenes.h\"\n\n#pragma region " + pragmaGame
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
            
            objectstr = objectstr + "\t{ " + idstr + ", " + scenestr + ", \"" + str(row["location"]) + "\", ObjectType::" + str(row["type"]) + ", {" + str(row["x"]) + ", " + str(row["y"]) + ", " + str(row["z"]) + "}, " + renderscene + ", ObjectType::" + str(row["rendertype"]) + ", EGameIcon::" + str(row["icontype"]) + ", GameLayout::" + str(row['game_layout']) + ", LocType::" + str(row["loc_type"]) + ", " + tooltip + " }"
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

            objectstr = "\t{ " + idstr + ", " + scenestr + ", \"" + str(row["friendly_name"]) + "\", \"" + str(row["location"]) + "\", ObjectType::" + str(row["type"]) + ", {" + str(int(row["x"])) + ", " + str(int(row["y"])) + ", " + str(int(row["z"])) + "}, " + renderscene + ", ObjectType::" + str(row["rendertype"]) + ", EGameIcon::" + str(row["icontype"]) + ", ObjectContext::" + str(row["context"]) + ", " + str(row["room"]) + ", GameLayout::" + str(row['game_layout']) + ", LocType::" + str(row["loc_type"]) + ", " + tooltip  + " }"
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
            strb = "\nconst size_t " + r + "NumOfObjs = " + str(le) + ";\nstatic ObjectInfo " + r + "SceneObjects_Data [" + r + "NumOfObjs" + "] =\n{\n"
            #strb = "\nCreateObjectsForScene(" + r + ", " + str(le) + ",\n"
            for u in fin[r]:
                i = i + 1
                strb = strb + u 
                if i < le:
                    strb = strb + ",\n"
            strb = strb + "\n};\nObjectInfo * " + r + "SceneObjects = " + r + "SceneObjects_Data;\n"
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
            minimap_path = row["minimap_path"]
            parent_region = row["parent_region"]
            has_context = row["has_context"]
            active_layout = row["active_layout"]

            if game == "OOT":
                parent_region = "OoTRegions::" + parent_region
            else:
                parent_region = "MMRegions::" + parent_region

            objectstr = objectstr + "\t{ \"" + scenestr + "\", \"" + image_path + "\", \"" + minimap_path + "\", (uint8_t) " + parent_region + ", " + str(has_context).lower() + ", GameLayout::" + active_layout + " }"
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
            in_X = row["Anchor_X"]
            in_Y = row["Anchor_Y"]
            in_Z = row["Anchor_Z"]
            out_X = row["Text_X"]
            out_Y = row["Text_Y"]
            out_Z = row["Text_Z"]
            render_Icon = row["Entrance_Icon"]
            active_layout = row["Active_Layout"]

            if in_Z == "Default" or in_Z == 0:
                in_Z = 0
            elif in_Z == "Up":
                in_Z = 1
            elif in_Z == "Down":
                in_Z = 2
            elif in_Z == "Left":
                in_Z = 3
            elif in_Z == "Right":
                in_Z = 4
            else:
                in_Z = 255

            if out_Z == "Default" or out_Z == 0:
                out_Z = 0
            elif out_Z == "Up":
                out_Z = 1
            elif out_Z == "Down":
                out_Z = 2
            elif out_Z == "Left":
                out_Z = 3
            elif out_Z == "Right":
                out_Z = 4
            else:
                out_Z = 255
            objectstr = objectstr + "\t{ " + str(entrance_code) + ", { " + str(fromentridstr) + ", " + str(entrance_code) + ", " + str(fromsceneid) + ", " + str(tosceneid) + ", \"" + from_str + "\", \"" + to_str + "\", EntranceType::" + type + ", " + str(in_X) + ", " + str(in_Y) + ", " + str(in_Z) + ", " + str(out_X) + ", " + str(out_Y) + ", " + str(out_Z) + ", EntranceIcons::" + render_Icon + ", GameLayout::" + active_layout + " } }"

            #outfile.write(objectstr)
            defines += "#define " + entrance_code + " " + toentridstr + "\n"


            if scene_entr_arr.__contains__(tosceneid) == False:
                scene_entr_arr[tosceneid] = []
                scene_regions[tosceneid] = region_prefix + regionstr
            
            if scene_entr_arr[tosceneid].__contains__(entrance_code) == True:
                print ("Duplicate entrance : " + str(entrance_code) + ", Achor Pos = " + str(in_X) + ", " + str(in_Y) + ", " + str(in_Z) )
            else:
                scene_entr_arr[tosceneid].append(entrance_code)
                objectstrings.append(objectstr)

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
                scene_str += "\t\t\t\t{ " + str (entr) + ", { { { UINT32_MAX, NO_GAME } }, UINT32_MAX } },\n"
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

def parse_entrance_costs(input_file, oot_output_file, mm_output_file):
    """Parse entrance_costs.csv and generate the per-game EntranceCostMeasurement arrays.

    The CSV is produced live by the TIMER_COST writer in Entrances.cpp; every row is a
    single intra-scene travel measurement (player arrived in Scene through 'FromEntranceID',
    walked to the exit that emits 'ToEntranceID', logged 'ElapsedSec' seconds). When the
    same (Game, Scene, From, To) key appears multiple times we keep the FASTEST run, same
    rule as EntranceCostModel::LoadCsv on the UI side.

    The output cpp files (one per game) contain a static EntranceCostMeasurement array and
    an InitializeXxxMeasuredCosts function. They are wired into InitializeEntranceCosts so
    the measured times override the default cost of 1 once they are loaded.
    """
    with open(input_file, 'r', encoding='utf-8') as infile:
        df = pd.read_csv(infile, delimiter=";", header=0, keep_default_na=False)

    # Normalise the columns we need into ints / floats. The CSV stores entrance / scene IDs
    # as 0x-prefixed hex strings, and elapsed times as plain floats.
    def parse_uint(s):
        s = str(s).strip()
        if s.lower().startswith("0x"):
            return int(s, 16)
        return int(s)

    measurements_by_game = {0: {}, 1: {}}        # game -> {(scene, from, to): cost_sec}
    for _, row in df.iterrows():
        try:
            game = int(str(row["Game"]).strip())
            scene = parse_uint(row["SceneID"])
            from_id = parse_uint(row["FromEntranceID"])
            to_id = parse_uint(row["ToEntranceID"])
            elapsed = float(str(row["ElapsedSec"]).strip())
        except (ValueError, KeyError):
            continue
        if elapsed < 0:
            continue
        if game not in measurements_by_game:
            continue
        # Round to the nearest second, clamp to 1 so 0-cost edges don't break Dijkstra.
        cost = max(1, int(round(elapsed)))
        key = (scene, from_id, to_id)
        existing = measurements_by_game[game].get(key)
        if existing is None or cost < existing:
            measurements_by_game[game][key] = cost

    GAME_NAMES = {0: "OoT", 1: "MM"}
    OUTPUTS = {0: oot_output_file, 1: mm_output_file}

    for game, out_path in OUTPUTS.items():
        prefix = GAME_NAMES[game]
        rows = sorted(measurements_by_game[game].items(), key=lambda kv: kv[0])

        with open(out_path, 'w', encoding='utf-8') as outfile:
            outfile.write('#include "Combo/Entrances.h"\n')
            outfile.write('#include "Combo/' + prefix + 'Entrances.h"\n\n')
            outfile.write('/*\n')
            outfile.write('*   Measured intra-scene travel times for ' + prefix + ', imported from entrance_costs.csv\n')
            outfile.write('*   by Pool Transform.py (parse_entrance_costs). Edit the CSV and re-run the\n')
            outfile.write('*   script - do not hand-edit this file.\n')
            outfile.write('*/\n')
            outfile.write('static const EntranceCostMeasurement ' + prefix + 'MeasuredCosts[] =\n{\n')
            if not rows:
                outfile.write('    // No ' + prefix + ' measurements yet - run Pool Transform.py to regenerate.\n')
            else:
                for (scene, from_id, to_id), cost in rows:
                    outfile.write('    {{ 0x{scene:03x}, 0x{from_id:03x}, 0x{to_id:04x}, {cost:4d} }},\n'.format(
                        scene=scene, from_id=from_id, to_id=to_id, cost=cost))
            outfile.write('};\n\n\n')
            outfile.write('void Initialize' + prefix + 'MeasuredCosts(std::map<int, EntranceMetaInfo>& Map)\n')
            outfile.write('{\n')
            outfile.write('    for (const EntranceCostMeasurement& M : ' + prefix + 'MeasuredCosts)\n')
            outfile.write('    {\n')
            outfile.write('        // Resolve the OUT key into the actual physical walk target: the OUT entry\'s\n')
            outfile.write('        // FromEntranceID is the spawn point in M.Scene that triggers the portal. Without\n')
            outfile.write('        // this translation, M.To might point at the destination scene\'s spawn instead, and\n')
            outfile.write('        // the cost would never reach the right intra-scene walking edge.\n')
            outfile.write('        auto ToIt = Map.find((int)M.To);\n')
            outfile.write('        if (ToIt == Map.end()) continue;\n')
            outfile.write('        const uint32_t WalkTarget = ToIt->second.FromEntranceID;\n\n')
            outfile.write('        // The Cost table for "walks starting at M.From in M.Scene" lives on the entrance\n')
            outfile.write('        // whose FromEntranceID == M.From AND FromSceneID == M.Scene. Within a single game\n')
            outfile.write('        // and scene the pair is unique, so one match is enough.\n')
            outfile.write('        for (auto& Pair : Map)\n')
            outfile.write('        {\n')
            outfile.write('            EntranceMetaInfo& V = Pair.second;\n')
            outfile.write('            if (V.FromEntranceID != M.From) continue;\n')
            outfile.write('            if (V.FromSceneID != M.Scene) continue;\n')
            outfile.write('            V.Cost.Costs[WalkTarget] = M.CostSeconds;\n')
            outfile.write('            break;\n')
            outfile.write('        }\n')
            outfile.write('    }\n')
            outfile.write('}\n')


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
cpp_file = '..\\..\\Sources\\Combo\\MMObjectScene.cpp'
parse_file2(input_file, cpp_file, "MM_")
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{cpp_file}'.")
#
input_file = '.\\pool_oot.csv'
#output_file = '.\\pool_oot.txt'
cpp_file = '..\\..\\Sources\\Combo\\OoTObjectScene.cpp'
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


input_file = '..\\..\\entrance_costs.csv'
oot_cpp_file = '..\\..\\Sources\\Combo\\OoTEntranceCosts.cpp'
mm_cpp_file  = '..\\..\\Sources\\Combo\\MMEntranceCosts.cpp'
parse_entrance_costs(input_file, oot_cpp_file, mm_cpp_file)
#
print(f"Conversion terminée. Les résultats sont enregistrés dans '{oot_cpp_file}', '{mm_cpp_file}'.")