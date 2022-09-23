from Grafo import GrafoFromFile


if __name__ == "__main__":
    g = GrafoFromFile("grafo_virgula.txt", delimitador=",")
    
    # g.showMapa2D()
    g.showMapa3D()
