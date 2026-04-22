# Topology binary-file generator

from enum import Enum
import json
import numpy as np

LOGO_FILENAME = "logo.txt"
EXPORT_FILENAME = "data.json"

def export_topology(X, open_indexes, filename=EXPORT_FILENAME):
    data = {
        "X": X,
        "topology": open_indexes
    }
    
    with open(filename, 'w') as f:
        json.dump(data, f, indent=4)

puntos = [0.0, 1.0, 2.0]
abiertos = [[], [0], [0, 1, 2]]
export_topology(puntos, abiertos)

def main():
    X = []
    f1 = open(LOGO_FILENAME, "r")

    print(f1.read())

    print("Welcome to this shitty topology generator! Please select your choice")
    print("\t[1] - Create a topology\n",
        "\t[2] - Load from binary file\n",
        "\t[3] - Add an element to a current topology\n",
        "\t[4] - Modify X\n",
        "\t[5] - Exit")
    
    c = input("Choice:")
    
    match c:
        case 1:
            print("What do you want to do with your topology?")
        case 2:
            print()
        case 3:
            print()
        case 4:
            modify_X()
        case 5:
            return

def insert(X, P, preset=0):
    if preset == 0:
        X.append(P)
    else:
        match preset:
            case 1: # Natural numbers
                X = ["naturals"]
            case 2:
                X = ["integers"]
            case 3:
                X = ["rationals"]
            case 4:
                X = ["reals"]
            case 5:
                X = ["complex"]
                

def modify_X():
    print("What do you want to introduce into X?")
    print("\t[1] Already created presets")
    print("\t[2] Insert points")
    print("\t[3] Add a linspace (i.e. intervals)")

    c = input("Choice:")
    match c:
        case 1:
            print("Integrity domain:")
            print("[1] Naturals")
            print("[2] Integers")
            print("[3] Rationals")
            print("[4] Reals")
            print("[5] Complex")
            d = input("Choice:")
            insert(X, [], d)
        case 2:
            val = input("Atom to insert:")
            insert(X, val)
        case 3:
            val = np.linspace(0, 100)
            insert(X, val)
main()