

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

    def isCicle(self, s) -> bool:
        visitados, restantes = [], [s]

        while restantes:
            vertice = restantes.pop()
            visitados.append(vertice)

            for vizinho in self.grafo[vertice]:
                if vizinho in visitados:
                    return True
                else:
                    restantes.append(vizinho)

        return False

    def bfs(self, s):
        visitados, fila, resultado = [s], [s], []

        while fila:
            s = fila.pop(0)
            for vertice in self.grafo[s]:
                if vertice not in visitados:
                    visitados.append(vertice)
                    fila.append(vertice)
            resultado.append(s)

        return resultado

    def dfs(self, s):
        pilha, caminho = [s], []
        while pilha:
            vertice = pilha.pop()
            if vertice not in caminho:
                caminho.append(vertice)
                pilha.extend(self.grafo[vertice])
        
        return caminho

        

if __name__ == "__main__":
    pass