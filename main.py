from Grafo import GrafoFromFile


if __name__ == "__main__":
    g = GrafoFromFile("grafo.txt", separador=",")
    
    g.showMapa2D()
    g.showMapa3D()
