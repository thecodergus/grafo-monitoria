from Grafo import GrafoFromFile


if __name__ == "__main__":
    g = GrafoFromFile("grafo.txt", separador=",")
    
    # Imprime os Grafos em 2D e 3D
    g.showMapa2D()
    # g.showMapa3D()
