"""Generate the demo's own small glTF character; Python standard library only."""
import base64
import json
import math
from pathlib import Path
import struct

OUT = Path(__file__).parent / 'assets' / 'explorer.gltf'


def generate():
    parents = [None, 0, 1, 2, 3, 3, 5, 6, 3, 8, 9, 1, 11, 12, 1, 14, 15]
    names = ['root', 'hips', 'spine', 'chest', 'head', 'arm.L', 'forearm.L', 'hand.L',
             'arm.R', 'forearm.R', 'hand.R', 'thigh.L', 'shin.L', 'foot.L', 'thigh.R', 'shin.R', 'foot.R']
    translations = [[0, 0, 0], [0, .97, 0], [0, .16, 0], [0, .23, 0], [0, .30, 0],
                    [.30, .03, 0], [0, -.30, 0], [0, -.26, 0], [-.30, .03, 0], [0, -.30, 0], [0, -.26, 0],
                    [.14, -.06, 0], [0, -.42, 0], [0, -.42, 0], [-.14, -.06, 0], [0, -.42, 0], [0, -.42, 0]]
    world = []
    nodes = []
    for i, name in enumerate(names):
        parent = parents[i]
        world.append([translations[i][k] + (world[parent][k] if parent is not None else 0) for k in range(3)])
        node = {'name': name, 'translation': translations[i]}
        children = [j for j, p in enumerate(parents) if p == i]
        if children:
            node['children'] = children
        nodes.append(node)
    colors = [[.95, .42, .055, 1], [.045, .11, .19, 1], [.48, .25, .11, 1], [.91, .67, .40, 1], [.018, .04, .065, 1]]
    data = {'asset': {'version': '2.0', 'generator': 'XGE walking tutorial: own procedural explorer'},
            'scene': 0, 'scenes': [{'nodes': [0, 17]}], 'nodes': nodes + [{'name': 'Explorer', 'mesh': 0, 'skin': 0}],
            'skins': [{'joints': list(range(17)), 'skeleton': 0}], 'meshes': [{'primitives': []}],
            'materials': [{'pbrMetallicRoughness': {'baseColorFactor': color, 'metallicFactor': 0, 'roughnessFactor': .85},
                           'emissiveFactor': [v * .04 for v in color[:3]]} for color in colors],
            'buffers': [], 'bufferViews': [], 'accessors': [], 'animations': []}
    binary = bytearray()

    def accessor(values, fmt, kind, count, component=5126):
        binary.extend(b'\0' * (-len(binary) % 4))
        offset = len(binary)
        packed = struct.pack('<' + str(len(values)) + fmt, *values)
        binary.extend(packed)
        index = len(data['accessors'])
        data['bufferViews'].append({'buffer': 0, 'byteOffset': offset, 'byteLength': len(packed)})
        entry = {'bufferView': index, 'componentType': component, 'count': count, 'type': kind}
        if kind in ['SCALAR', 'VEC3']:
            width = 1 if kind == 'SCALAR' else 3
            entry['min'] = [min(values[k::width]) for k in range(width)]
            entry['max'] = [max(values[k::width]) for k in range(width)]
        data['accessors'].append(entry)
        return index

    # All boxes belong to actual skin joints; different materials share a palette.
    groups = [[] for _ in colors]
    def box(joint, offset, half, material):
        center = [world[joint][k] + offset[k] for k in range(3)]
        for face in range(6):
            k, sign = face // 2, (1 if face % 2 else -1)
            a, b = (k + 1) % 3, (k + 2) % 3
            corners = [(-1, -1), (1, -1), (1, 1), (-1, -1), (1, 1), (-1, 1)]
            if sign < 0:
                corners.reverse()
            for u, v in corners:
                s = [0, 0, 0]; s[k], s[a], s[b] = sign, u, v
                groups[material].append(([center[j] + half[j] * s[j] for j in range(3)],
                                         [sign if j == k else 0 for j in range(3)], joint))
    box(1, [0, 0, 0], [.23, .13, .14], 1)
    box(3, [0, -.10, 0], [.26, .25, .15], 0)
    box(3, [0, -.10, -.20], [.18, .21, .065], 2)  # Backpack.
    box(4, [0, .10, 0], [.14, .17, .135], 3)
    box(4, [0, -.09, 0], [.055, .075, .055], 3)
    box(4, [0, .25, 0], [.155, .045, .15], 1)
    box(4, [0, .12, .139], [.10, .045, .012], 4)  # Visor, facing +Z.
    for arm, forearm, hand, thigh, shin, foot in [(5, 6, 7, 11, 12, 13), (8, 9, 10, 14, 15, 16)]:
        box(arm, [0, -.13, 0], [.085, .16, .095], 0)
        box(forearm, [0, -.12, 0], [.073, .14, .08], 1)
        box(hand, [0, -.03, .01], [.07, .07, .075], 3)
        box(thigh, [0, -.20, 0], [.10, .21, .105], 1)
        box(shin, [0, -.19, 0], [.085, .20, .095], 1)
        box(foot, [0, 0, .07], [.10, .065, .16], 2)
    for material, vertices in enumerate(groups):
        attrs = {'POSITION': accessor([v for p, n, j in vertices for v in p], 'f', 'VEC3', len(vertices)),
                 'NORMAL': accessor([v for p, n, j in vertices for v in n], 'f', 'VEC3', len(vertices)),
                 'JOINTS_0': accessor([v for p, n, j in vertices for v in [j, 0, 0, 0]], 'H', 'VEC4', len(vertices), 5123),
                 'WEIGHTS_0': accessor([v for _ in vertices for v in [1, 0, 0, 0]], 'f', 'VEC4', len(vertices))}
        data['meshes'][0]['primitives'].append({'attributes': attrs, 'material': material})
    matrices = [v for p in world for v in [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -p[0], -p[1], -p[2], 1]]
    data['skins'][0]['inverseBindMatrices'] = accessor(matrices, 'f', 'MAT4', 17)
    for name, duration, amplitude in [('idle', 2.4, .035), ('walk', .9, .55), ('run', .62, .85), ('jump', .85, .30)]:
        animation = {'name': name, 'samplers': [], 'channels': []}
        times = [duration * i / 24 for i in range(25)]
        ti = accessor(times, 'f', 'SCALAR', len(times))
        for joint in [3, 5, 6, 8, 9, 11, 12, 14, 15]:
            angles = []
            for i in range(25):
                phase = 2 * math.pi * i / 24
                angle = amplitude * math.sin(phase) * (1 if joint in [8, 9, 11, 12] else -1)
                if joint in [6, 9, 12, 15]: angle = max(0, angle) * .75
                if joint == 3: angle *= .07
                if name == 'jump': angle = amplitude * math.sin(math.pi * i / 24) * (-1 if joint in [5, 8, 11, 14] else 1)
                angles.extend([math.sin(angle / 2), 0, 0, math.cos(angle / 2)])
            output = accessor(angles, 'f', 'VEC4', len(times))
            animation['channels'].append({'sampler': len(animation['samplers']), 'target': {'node': joint, 'path': 'rotation'}})
            animation['samplers'].append({'input': ti, 'output': output, 'interpolation': 'LINEAR'})
        data['animations'].append(animation)
    data['buffers'] = [{'byteLength': len(binary), 'uri': 'data:application/octet-stream;base64,' + base64.b64encode(binary).decode()}]
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(data, separators=(',', ':')) + '\n', encoding='utf-8')
    print('Own glTF explorer:', OUT, OUT.stat().st_size, 'bytes')


if __name__ == '__main__':
    generate()
