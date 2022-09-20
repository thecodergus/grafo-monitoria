

class Grafo:
    grafo = {}

    def __init__(self, arquivo = "", dirigido = False):
        self.dirigido = dirigido
        self.enderecoArquivo = arquivo

    def addAresta(self, vertice, vizinho):
        if vertice not in self.grafo:
            self.grafo[vertice] = [vizinho]
        elif vizinho not in self.grafo[vertice]:
            self.grafo[vertice].append(vizinho)

    def rmAresta(self, vertice, vizinho):
        pass

if __name__ == "__main__":
    pass