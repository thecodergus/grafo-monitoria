

import re


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

    def bfs(self, s):
        visitados, fila, resultado = [], [], []
        fila.append(s)
        visitados.append(s)

        while len(fila) > 0:
            s = fila.pop(0)
            for vertice in self.grafo[s]:
                if vertice not in visitados:
                    visitados.append(vertice)
                    fila.append(vertice)
            resultado.append(s)

        return resultado
        

if __name__ == "__main__":
    pass