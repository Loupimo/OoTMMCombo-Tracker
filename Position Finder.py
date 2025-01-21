import cv2
import numpy as np

def find_green_points(image_path):
    # Charger l'image
    image = cv2.imread(image_path)
    
    # Convertir l'image en espace de couleur HSV
    hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    
    # Définir les plages de couleurs pour le vert (en HSV)
    lower_green = np.array([40, 100, 100])  # Limite basse du vert
    upper_green = np.array([80, 255, 255])  # Limite haute du vert

    # Créer un masque pour extraire les zones vertes
    mask = cv2.inRange(hsv_image, lower_green, upper_green)

    # Trouver les contours des points verts
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

    return coordinates

# Utilisation du code
image_path = "/mnt/data/test.png"  # Chemin de l'image
coordinates = find_green_points(image_path)

# Affichage des coordonnées des points verts
for i, (x, y) in enumerate(coordinates, start=1):
    print(f"Point {i}: X={x}, Y={y}")
