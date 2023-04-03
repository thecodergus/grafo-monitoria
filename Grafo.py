from collections import defaultdict
from pyvis import network as net
from IPython.display import display, HTML
import plotly.graph_objects as go
import random
import minify_html
from typing import List, Tuple, Union


class Graph:
    def __init__(self, directed: bool = False) -> None:
        self.graph = defaultdict(list)
        self.directed = directed

    def add_edge(self, vertex: any, neighbor: any) -> None:
        if neighbor not in self.graph[vertex]:
            self.graph[vertex].append(neighbor)
            if not self.directed:
                self.graph[neighbor].append(vertex)

    def remove_edge(self, vertex: any, neighbor: any) -> None:
        if vertex in self.graph:
            self.graph[vertex].remove(neighbor)
            if not self.directed:
                self.graph[neighbor].remove(vertex)

    def get_num_vertices(self) -> int:
        return len(self.graph.keys())

    def get_degree(self, vertex: any) -> int:
        return len(self.graph[vertex])

    def get_vertices(self) -> List[any]:
        return list(self.graph.keys())

    def bfs(self, vertex: any) -> List[any]:
        visited, queue, result = [vertex], [vertex], []

        while queue:
            vertex = queue.pop(0)
            for neighbor in self.graph[vertex]:
                if neighbor not in visited:
                    visited.append(neighbor)
                    queue.append(neighbor)
            result.append(vertex)

        return result

    def dfs(self, vertex: any) -> List[any]:
        stack, path = [vertex], []
        while stack:
            vertex = stack.pop()
            if vertex not in path:
                path.append(vertex)
                stack.extend(self.graph[vertex])

        return path

    def __has_cycle(self, vertex: any, visited: List[bool], parent: any) -> bool:
        visited[vertex] = True

        for neighbor in self.graph[vertex]:
            if not visited[neighbor]:
                if self.__has_cycle(neighbor, visited, vertex):
                    return True
            elif parent != neighbor:
                return True

        return False

    def has_cycle(self) -> bool:
        num_vertices = self.get_num_vertices()
        visited = [False] * (num_vertices + 1)

        for vertex in range(num_vertices):
            if not visited[vertex]:
                if self.__has_cycle(vertex, visited, -1):
                    return True

        return False
    
    def get_edges(self, as_tuple: bool = False) -> List[Union[Tuple[any, any], List[any]]]:
        result = []
        processed_edges = set()

        for vertex in self.get_vertices():
            for neighbor in self.graph[vertex]:
                edge = (vertex, neighbor) if vertex < neighbor else (neighbor, vertex)
                if not self.directed and edge in processed_edges:
                    continue
                processed_edges.add(edge)

                if as_tuple:
                    result.append(edge)
                else:
                    result.append([vertex, neighbor])

        return result

    def __str__(self) -> str:
        vertices_str = ", ".join(map(str, self.get_vertices()))
        edges_str = ", ".join([f"({v1}, {v2})" for v1, v2 in self.get_edges(as_tuple=True)])

        return f"Vertices: {vertices_str}\nEdges: {edges_str}"


    def show_2d_map(self) -> None:
        vertices, edges = self.get_vertices(), self.get_edges()
        print(vertices)
        print(edges)

        interface = net.Network(
            height='100%',
            width='100%',
            notebook=True,
            heading='2D Graph'
        )

        for v in vertices:
            interface.add_node(v, label=str(v))

        interface.add_edges(edges)

        output_file = '2DGraph.html'
        interface.show(output_file)
        display(HTML(output_file))



    def show_3d_map(self) -> None:
        num_vertices = self.get_num_vertices()
        edges_weights = [1] * num_vertices
        edges = self.get_edges(as_tuple=True)

        graph_3d = {
            v: [random.random(), random.random(), random.random()] for v in self.get_vertices()
        }

        x_vertices = [graph_3d[key][0] for key in graph_3d.keys()]
        y_vertices = [graph_3d[key][1] for key in graph_3d.keys()]
        z_vertices = [graph_3d[key][2] for key in graph_3d.keys()]

        x_edges, y_edges, z_edges = [], [], []
        xtp, ytp, ztp = [], [], []

        for edge in edges:
            x_coords = [graph_3d[edge[0]][0], graph_3d[edge[1]][0], None]
            x_edges.extend(x_coords)
            xtp.append(0.5 * (graph_3d[edge[0]][0] + graph_3d[edge[1]][0]))

            y_coords = [graph_3d[edge[0]][1], graph_3d[edge[1]][1], None]
            y_edges.extend(y_coords)
            ytp.append(0.5 * (graph_3d[edge[0]][1] + graph_3d[edge[1]][1]))

            z_coords = [graph_3d[edge[0]][2], graph_3d[edge[1]][2], None]
            z_edges.extend(z_coords)
            ztp.append(0.5 * (graph_3d[edge[0]][2] + graph_3d[edge[1]][2]))

        etext = [f'weight={w}' for w in edges_weights]

        trace_weights = go.Scatter3d(
            x=xtp,
            y=ytp,
            z=ztp,
            mode='markers',
            marker=dict(color='rgb(125,125,125)', size=1),
            text=etext,
            hoverinfo='text'
        )

        trace_edges = go.Scatter3d(
            x=x_edges,
            y=y_edges,
            z=z_edges,
            mode='lines',
            line=dict(color='black', width=2),
            hoverinfo='none'
        )

        trace_vertices = go.Scatter3d(
            x=x_vertices,
            y=y_vertices,
            z=z_vertices,
            mode='markers',
            marker=dict(symbol='circle',
                        size=10,
                        color='skyblue')
        )

        layout = go.Layout(
            title="3D Graph",
            autosize=True,
            showlegend=True,
            margin=dict(t=100),
            hovermode='closest'
        )

        data = [trace_edges, trace_vertices, trace_weights]
        fig = go.Figure(data=data, layout=layout)

        fig.show()





def graph_from_file(file_path: str = "", separator: str = " ", directed: bool = False) -> Graph:
    graph = Graph(directed)
    split = lambda string: [*map(int, string.split(separator))]

    with open(file_path, "r") as file:
        for line in file.readlines():
            edge = split(line)
            if len(edge) > 1:
                a, b = edge
                graph.add_edge(a, b)

    return graph