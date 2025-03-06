import cv2
import numpy as np
import os

colors = {"Grass": np.array([0, 128, 0]),
          "FS": np.array([255, 180, 255]),
          "Hive": np.array([200, 150, 200]),
          "Pack": np.array([192, 192, 192]),
          "Chest": np.array([255, 0, 255]),
          "Cow": np.array([245, 245, 245]),
          "Pot": np.array([221, 160, 221]),
          "GS": np.array([200, 50, 200]),
          "Fly": np.array([250, 105, 250]),
          "Wonder": np.array([200, 100, 200]),
          "Crate": np.array([0, 80, 0]),
          "Grotto": np.array([255, 50, 255]),
          "NPCs": np.array([255, 128, 255]),
          }

def find_points(image_path, colorstr):
    # Charger l'image
    image = cv2.imread(image_path)
    
    # Convertir l'image en espace de couleur HSV
    #hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    
    # Définir les plages de couleurs pour le vert (en HSV)
    lower_green = colors[colorstr]  # Limite basse
    upper_green = colors[colorstr]  # Limite haute

    # Créer un masque pour extraire les zones de la couleur souhaitée
    #mask = cv2.inRange(hsv_image, lower_green, upper_green)
    mask = cv2.inRange(image, lower_green, upper_green)

    # Trouver les contours des points colorés
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # Extraire les coordonnées des centres des contours
    coordinates = []
    for contour in contours:
        # Calculer les moments pour trouver le centre
        M = cv2.moments(contour)
        if M["m00"] != 0:
            cx = int(M["m10"] / M["m00"])  # Coordonnée X
            cy = int(M["m01"] / M["m00"])  # Coordonnée Y
            coordinates.append((cx, cy))

    if len(coordinates) == 0:
        return coordinates, ""
    coordStr = "QList<QPair<int, int>> " + colorstr + "Points = {\n"
    # Affichage des coordonnées des points verts
    for i, (x, y) in enumerate(coordinates, start=1):
        coordStr = coordStr + "{" + str(x) + ", " + str(y) + "},\n"

    coordStr = coordStr + "};\n"
    #print(coordStr)
    return coordinates, coordStr

def detect_blobs(image_path, colorstr):
    # Charger l'image et convertir en HSV
    image = cv2.imread(image_path)
    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

    color_lower = colors[colorstr]-10  # Limite basse
    color_upper = colors[colorstr]+10  # Limite haute

    # Créer un masque
    mask = cv2.inRange(image, color_lower, color_upper)

    # Définir les paramètres du détecteur de blobs
    params = cv2.SimpleBlobDetector_Params()
    params.filterByArea = True
    params.minArea = 1  # Ajustez selon vos besoins
    params.filterByCircularity = False
    params.filterByConvexity = False
    params.filterByInertia = False

    # Détecteur de blobs
    detector = cv2.SimpleBlobDetector_create(params)
    keypoints = detector.detect(mask)

    # Récupérer les coordonnées des blobs
    coordinates = [(int(k.pt[0]), int(k.pt[1])) for k in keypoints]
    if len(coordinates) == 0:
        return coordinates, ""
    coordStr = "QList<QPair<int, int>> " + colorstr + "Points = {\n"
    # Affichage des coordonnées des points verts
    for i, (x, y) in enumerate(coordinates, start=1):
        coordStr = coordStr + "{" + str(x) + ", " + str(y) + "},\n"

    coordStr = coordStr + "};\n"
    print(coordStr)
    return coordinates, coordStr

def perform_action(root_folder):
    data = []
    for root, dirs, files in os.walk(root_folder):
        for f in files:
            if f.startswith("AP") and f.endswith(".png"):
                data.append(os.path.join(root,f))
    for file in data:
        if os.path.isfile(file + ".txt") == False:
            with open(file + ".txt", 'w') as f:
                for colorstr in colors.keys():
                    coord, coordstr = find_points(file, colorstr)
                    #coord, coordstr = detect_blobs(file, colorstr)
                    f.write(coordstr)

#perform_action("D:\\Emulation\\OoTMMCombo-Tracker\\Resources\\OoT_AP")
perform_action("D:\\Emulation\\OoTMMCombo-Tracker\\Resources\\MM_AP")