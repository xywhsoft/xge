"""Build a light 3D + basic 2D tutorial; no text, ShapeEx, XUI or external assets."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build' / 'xge-3d-walk'


def run(args):
    subprocess.run([str(a) for a in args], cwd=ROOT, check=True)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--run', action='store_true', help='open the interactive demo (no automatic exit)')
    p.add_argument('--frames', type=int, help='stop after N frames')
    p.add_argument('--seed', type=int)
    p.add_argument('--autopilot', action='store_true')
    p.add_argument('--first-person', action='store_true')
    p.add_argument('--capture', help='save the rendered scene at exit; combine with --frames')
    a = p.parse_args()
    if a.frames is not None and a.frames <= 0:
        p.error('--frames must be positive')
    definitions = ['XGE_ENABLE_2D=1', 'XGE_ENABLE_TEXT=0', 'XGE_ENABLE_SHAPE_EX=0',
                   'XGE_ENABLE_PARTICLES=0', 'XGE3D_ENABLE_IBL=0', 'XGE3D_ENABLE_ASYNC=0']
    run([sys.executable, 'tools/build_profile.py', '3d', OUT,
         *[arg for d in definitions for arg in ['--define', d]]])
    source = ROOT / 'examples/xge_3d_walk/assets/explorer.gltf'
    if not source.exists():
        run([sys.executable, 'examples/xge_3d_walk/generate_assets.py'])
    (OUT / 'assets').mkdir(exist_ok=True)
    shutil.copy2(source, OUT / 'assets/explorer.gltf')
    exe = OUT / 'xge_3d_walk.exe'
    run(['gcc', '-O2', '-Wall', '-Wextra', '-Werror', '-Wno-missing-field-initializers', '-DXGE_DLL', '-I.',
         '-include', OUT / 'xge_build_config.h', 'examples/xge_3d_walk/main.c', OUT / 'xge.lib', '-lm', '-o', exe])
    print('Interactive demo:', exe, flush=True)
    if a.run:
        args = [exe]
        for name in ['frames', 'seed', 'capture']:
            if getattr(a, name) is not None:
                args.extend(['--' + name, getattr(a, name)])
        for name in ['autopilot', 'first_person']:
            if getattr(a, name):
                args.append('--' + name.replace('_', '-'))
        run(args)


if __name__ == '__main__':
    main()
