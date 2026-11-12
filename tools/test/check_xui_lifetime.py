"""Offline XUI context/widget ownership and existing drag/drop reentry gates."""
from pathlib import Path
import argparse, hashlib, json, os, shutil, subprocess, time

SOURCES = ['tests/xui/xui_test_xrt_impl.c', 'modules/xlayout/xlayout.c',
    'src/xui/core/xui_core.c', 'src/xui/accessibility/xui_native_accessibility.c',
    'src/xui/core/xui_widget.c', 'src/xui/core/xui_callback_owned.c',
    'src/xui/core/xui_owned_manager.c', 'src/xui/core/xui_owned_resource_slots.c',
    'src/xui/core/xui_proxy_resource.c', 'src/xui/core/xui_icon.c',
    'src/xui/core/xui_layout.c', 'src/xui/core/xui_input.c',
    'src/xui/core/xui_edit.c', 'src/xui/core/xui_drag_drop.c', 'src/xui/accessibility/xui_accessibility.c',
    'src/xui/core/xui_text.c', 'src/xui/core/xui_assets.c', 'src/xui/core/xui_builtin_atlas.c',
    'src/xui/core/xui_unicode.c', 'src/xui/workflow/xui_flow_graph.c', 'src/xui/workflow/xui_workflow.c',
    'src/xui/controls/xui_button.c',
    'src/xui/controls/xui_checkbox.c',
    'src/xui/controls/xui_check_card.c',
    'src/xui/controls/xui_radio.c',
    'src/xui/controls/xui_toggle.c',
    'src/xui/controls/xui_slider.c',
    'src/xui/controls/xui_range_slider.c',
    'src/xui/controls/xui_scrollbar.c',
    'src/xui/controls/xui_hyperlink.c', 'src/xui/controls/xui_page.c',
    'src/xui/controls/xui_carousel.c', 'src/xui/controls/xui_breadcrumb.c']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--cc', default='gcc')
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--tests', nargs='+', choices=('lifetime', 'lifetime_observer', 'drag_drop_reentry', 'owned_callback', 'owned_payload', 'owned_layout', 'owned_widget', 'owned_choice', 'owned_navigation'), default=None)
    args = parser.parse_args()
    root, out, cache = args.root.resolve(), args.out.resolve(), args.cache.resolve()
    cc = shutil.which(args.cc); assert cc
    out.mkdir(parents=True, exist_ok=False); cache.mkdir(parents=True, exist_ok=True)
    (out/'tmp').mkdir()
    env = dict(os.environ, TEMP=str(out/'tmp'), TMP=str(out/'tmp'), TMPDIR=str(out/'tmp'))
    if args.sanitize:
        assert os.name != 'nt'
        env.update(ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1')
    compiler_hash = hashlib.sha256(Path(cc).read_bytes()).hexdigest()
    tool_hash = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    report = dict(complete=False, passed=False, compiler=cc, compilerSha256=compiler_hash,
                  toolSha256=tool_hash, sanitize=args.sanitize, networkTests=False, selectedTests=args.tests, rows=[], objects=[])

    def save(): (out/'results.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    def run(name, command, required=True):
        start = time.monotonic()
        process = subprocess.run(list(map(str, command)), cwd=root, env=env, capture_output=True, timeout=240)
        (out/(name+'.log')).write_bytes(process.stderr if name.endswith('-pre') else process.stdout+process.stderr)
        report['rows'].append(dict(name=name, exitCode=process.returncode,
            seconds=round(time.monotonic()-start, 3), command=list(map(str, command))))
        save()
        if required: assert process.returncode == 0, out/(name+'.log')
        return process

    common = ['-std=c11', '-g', '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter',
        '-Wno-unused-function', '-Wno-cast-function-type', '-DXGE_DEBUGMODE=0',
        '-DXRT_MODULE_MEMORY_DEBUG', '@config/includes.rsp']
    if args.sanitize: common += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie']
    libraries = ['-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32', '-lshell32', '-lopengl32',
                 '-lole32', '-loleaut32', '-lwinmm', '-lavrt'] if os.name == 'nt' else ['-lm', '-ldl', '-lpthread']
    snapshots = []
    for opt in ('O0', 'O2'):
        flags = [*common, '-'+opt]
        def compile_source(source):
            pre = run(Path(source).stem+'-'+opt+'-pre', [cc, *flags, '-E', source]).stdout
            pre_hash = hashlib.sha256(pre).hexdigest()
            key = hashlib.sha256(json.dumps([compiler_hash, str(root), flags, source, pre_hash]).encode()).hexdigest()
            obj, metadata = cache/(key+'.o'), cache/(key+'.json')
            reused = False
            if obj.exists() and metadata.exists():
                recorded = json.loads(metadata.read_text())
                reused = recorded['sha256'] == hashlib.sha256(obj.read_bytes()).hexdigest()
            if not reused:
                run(Path(source).stem+'-'+opt+'-build', [cc, *flags, '-c', source, '-o', obj])
                metadata.write_text(json.dumps(dict(sha256=hashlib.sha256(obj.read_bytes()).hexdigest()))+'\n')
            report['objects'].append(dict(source=source, flags=flags, path=str(obj), preprocessedSha256=pre_hash,
                sha256=hashlib.sha256(obj.read_bytes()).hexdigest(), reused=reused))
            snapshots.append((source, flags, pre_hash)); save()
            print(opt, source, 'reuse' if reused else 'compile', flush=True)
            return obj
        objects = [compile_source(source) for source in SOURCES]
        for test in (args.tests or ('lifetime', 'lifetime_observer', 'drag_drop_reentry', 'owned_callback', 'owned_payload', 'owned_layout', 'owned_widget', 'owned_choice', 'owned_navigation')):
            obj = compile_source('tests/xui/xui_'+test+'_test.c')
            executable = out/(test+'-'+opt+('.exe' if os.name == 'nt' else '.out'))
            run(test+'-'+opt+'-link', [cc, *flags, *(['-no-pie'] if args.sanitize else []),
                obj, *objects, *libraries, '-o', executable])
            if test=='owned_choice':
                for kind in range(8):run(test+'-'+opt+'-'+str(kind)+'-run',[executable,str(kind)],required=False)
            else:run(test+'-'+opt+'-run', [executable], required=False)
    report['inputsUnchanged'] = all(hashlib.sha256(subprocess.check_output(
        [cc, *flags, '-E', source], cwd=root, env=env)).hexdigest() == expected for source, flags, expected in snapshots)
    report['inputsUnchanged'] &= hashlib.sha256(Path(__file__).read_bytes()).hexdigest() == tool_hash
    report['complete'] = True
    report['passed'] = report['inputsUnchanged'] and all(row['exitCode'] == 0 for row in report['rows'])
    save()
    assert report['passed'], out/'results.json'


if __name__ == '__main__': main()
