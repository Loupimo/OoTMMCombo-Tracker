import cv2
import numpy as np
import os

# === PARAMÈTRES ===

def do_contour(in_path):

    INPUT_IMAGE = in_path + ".png"
    OUTPUT_IMAGE = in_path + "_minimap.png"

    img = cv2.imread(INPUT_IMAGE)

    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # Seuillage adaptatif
    thresh = cv2.adaptiveThreshold(
        gray,
        255,
        cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
        cv2.THRESH_BINARY_INV,
        11,
        2
    )

    # Nettoyage
    kernel = np.ones((3,3), np.uint8)
    clean = cv2.morphologyEx(
        thresh,
        cv2.MORPH_CLOSE,
        kernel,
        iterations=2
    )

    # Contours
    contours, _ = cv2.findContours(
        clean,
        cv2.RETR_EXTERNAL,
        cv2.CHAIN_APPROX_SIMPLE
    )

    result = np.zeros_like(gray)

    cv2.drawContours(
        result,
        contours,
        -1,
        255,
        2
    )

    cv2.imwrite(OUTPUT_IMAGE, result)

do_contour("OoT_No_Minimap_Entrances_test")
path = "OoT_No_Minimap_Entrances_"

for i in range (1, 8):
    do_contour(path + str(i))