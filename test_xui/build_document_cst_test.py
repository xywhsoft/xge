"""Build shared C kernel objects once per variant, then run CST regression drivers."""
from pathlib import Path
import argparse, json, os, subprocess, sys
root = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--sanitizer', action='store_true')
parser.add_argument('--out-dir', default='build/document/cst')
args = parser.parse_args()
out = root / args.out_dir; out.mkdir(parents=True, exist_ok=True)
sources = []
for line in (root / 'xui_document_sources.bat').read_text(encoding='utf-8').splitlines():
    if line.startswith('set XUI_DOCUMENT_SRC='):
        sources.extend(p.replace('\\', '/') for p in line.split('=', 1)[1].replace('%XUI_DOCUMENT_SRC%', '').split())
assert sources and len(sources) == len(set(sources))
tests = ['collector', 'incremental', 'fault', 'field', 'index', 'used_fault', 'memory', 'transition', 'sharing', 'link_definition', 'link_definition_transition', 'link_definition_fault', 'link_definition_tokens', 'whole', 'syntax_budget', 'body_index', 'candidate', 'body_sharing', 'default_grammar', 'trivia', 'trivia_index', 'store_share', 'index_workspace', 'trivia_graft', 'table_source', 'table_row', 'table_reference', 'footnote_projection', 'delete_source', 'inline_range', 'inline_boundary', 'container_prefix', 'deep_quote', 'deep_quote_fault', 'nested_list', 'nested_list_fault', 'nonparagraph_prefix', 'nonparagraph_long_header', 'nonparagraph_prefix_fault', 'inline_split', 'footnote_inline_newline', 'footnote_inline_newline_fault', 'deep_footnote_newline', 'wide_container_newline', 'deep_footnote_newline_fault', 'implicit_reference_split', 'reference_text', 'gfm_code_pipe_diagnostic', 'gfm_code_pipe_source', 'gfm_code_pipe_edit', 'html_runs', 'html_rowspan', 'html_table_numbers', 'html_character_reference', 'html_named_reference', 'html_markdown_reference', 'html_markdown_reference_fault', 'html_newline', 'literal_bracket_cst', 'gfm_code_pipe_fault', 'code_boundary_diagnostic', 'code_boundary_cst', 'code_boundary_fault', 'html_buffer_fault', 'footnote_local_reference_diagnostic', 'footnote_local_code_cst', 'footnote_local_source', 'footnote_local_fault', 'footnote_local_code_fault', 'list_split', 'join', 'join_fault']
tests.append('input_mapping')
tests.append('prepared_semantic')
rows = []
def run(command):
    result = subprocess.run(command, cwd=root)
    rows.append({'command': command, 'exit': result.returncode})
    (out / 'checks.json').write_text(json.dumps(rows, indent=2), encoding='utf-8')
    if result.returncode: raise SystemExit(result.returncode)
base = ['gcc', '-std=c11', '-O1' if args.sanitizer else '-O2', '-g', '-Wall', '-Wextra', '-Werror', '-Wno-unused-function', '-I' + str(root)]
if args.sanitizer:
    base.extend(['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-no-pie'])
    os.environ.setdefault('ASAN_OPTIONS', 'detect_leaks=1:halt_on_error=1')
    os.environ.setdefault('UBSAN_OPTIONS', 'halt_on_error=1')
link_flags = ['-lm', '-lws2_32', '-liphlpapi'] if sys.platform == 'win32' else ['-lm', '-ldl', '-lpthread']
for variant in ['normal', 'collision']:
    folder = out / variant; folder.mkdir(exist_ok=True)
    flags = [] if variant == 'normal' else ['-DXUI_DOC_REFERENCE_HASH_COLLISION_TEST']
    objects = []
    for p in sources + ['test_xui/xui_test_xrt_impl.c']:
        obj = folder / (Path(p).stem + '.o'); objects.append(str(obj))
        run([*base, *flags, '-c', str(root / p), '-o', str(obj)])
    library = folder / 'document.a'
    # A fresh archive cannot retain removed manifest members from an earlier run.
    staging = folder / 'document-next.a'
    if staging.exists(): staging.unlink()
    run(['ar', 'rcs', str(staging), *objects])
    staging.replace(library)
    for name in tests:
        exe = folder / ('cst_' + name + '.exe')
        source_name = {
            'link_definition': 'xui_document_link_definition_cst_test.c',
            'link_definition_transition': 'xui_document_link_definition_transition_test.c',
            'link_definition_fault': 'xui_document_link_definition_fault_test.c',
            'link_definition_tokens': 'xui_document_link_definition_tokens_test.c',
            'whole': 'xui_document_whole_cst_test.c',
            'syntax_budget': 'xui_document_syntax_budget_test.c',
            'body_index': 'xui_document_body_index_test.c',
            'candidate': 'xui_document_candidate_cst_test.c',
            'body_sharing': 'xui_document_body_sharing_test.c',
            'default_grammar': 'xui_document_default_grammar_test.c',
            'trivia': 'xui_document_trivia_cst_test.c',
            'trivia_index': 'xui_document_trivia_index_test.c',
            'store_share': 'xui_document_store_share_test.c',
            'index_workspace': 'xui_document_index_workspace_test.c',
            'trivia_graft': 'xui_document_trivia_graft_test.c',
            'table_source': 'xui_document_table_source_cst_test.c',
            'table_row': 'xui_document_table_row_cst_test.c',
            'table_reference': 'xui_document_table_reference_cst_test.c',
            'footnote_projection': 'xui_document_footnote_projection_cst_test.c',
            'delete_source': 'xui_document_delete_source_cst_test.c',
            'inline_range': 'xui_document_inline_range_cst_test.c',
            'inline_boundary': 'xui_document_inline_boundary_cst_test.c',
            'container_prefix': 'xui_document_container_prefix_cst_test.c',
            'deep_quote': 'xui_document_deep_quote_cst_test.c',
            'deep_quote_fault': 'xui_document_deep_quote_fault_test.c',
            'nested_list': 'xui_document_nested_list_cst_test.c',
            'nested_list_fault': 'xui_document_nested_list_fault_test.c',
            'nonparagraph_prefix': 'xui_document_nonparagraph_prefix_cst_test.c',
            'nonparagraph_long_header': 'xui_document_nonparagraph_long_header_test.c',
            'nonparagraph_prefix_fault': 'xui_document_nonparagraph_prefix_fault_test.c',
            'inline_split': 'xui_document_inline_split_cst_test.c',
            'footnote_inline_newline': 'xui_document_footnote_inline_newline_test.c',
            'footnote_inline_newline_fault': 'xui_document_footnote_inline_newline_fault_test.c',
            'deep_footnote_newline': 'xui_document_deep_footnote_newline_test.c',
            'wide_container_newline': 'xui_document_wide_container_newline_test.c',
            'deep_footnote_newline_fault': 'xui_document_deep_footnote_newline_fault_test.c',
            'implicit_reference_split': 'xui_document_implicit_reference_split_cst_test.c',
            'reference_text': 'xui_document_reference_text_cst_test.c',
            'gfm_code_pipe_diagnostic': 'xui_document_gfm_code_pipe_diagnostic_test.c',
            'gfm_code_pipe_source': 'xui_document_gfm_code_pipe_source_test.c',
            'gfm_code_pipe_edit': 'xui_document_gfm_code_pipe_edit_test.c',
            'html_runs': 'xui_document_html_runs_test.c',
            'html_rowspan': 'xui_document_html_rowspan_test.c',
            'html_table_numbers': 'xui_document_html_table_numbers_test.c',
            'html_character_reference': 'xui_document_html_character_reference_test.c',
            'html_named_reference': 'xui_document_html_named_reference_test.c',
            'html_markdown_reference': 'xui_document_html_markdown_reference_test.c',
            'html_markdown_reference_fault': 'xui_document_html_markdown_reference_fault_test.c',
            'html_newline': 'xui_document_html_newline_test.c',
            'literal_bracket_cst': 'xui_document_literal_bracket_cst_test.c',
            'gfm_code_pipe_fault': 'xui_document_gfm_code_pipe_fault_test.c',
            'code_boundary_diagnostic': 'xui_document_code_boundary_diagnostic_test.c',
            'code_boundary_cst': 'xui_document_code_boundary_cst_test.c',
            'code_boundary_fault': 'xui_document_code_boundary_fault_test.c',
            'html_buffer_fault': 'xui_document_html_buffer_fault_test.c',
            'footnote_local_reference_diagnostic': 'xui_document_footnote_local_reference_diagnostic_test.c',
            'footnote_local_code_cst': 'xui_document_footnote_local_code_cst_test.c',
            'footnote_local_source': 'xui_document_footnote_local_source_test.c',
            'footnote_local_fault': 'xui_document_footnote_local_fault_test.c',
            'footnote_local_code_fault': 'xui_document_footnote_local_code_fault_test.c',
            'list_split': 'xui_document_list_split_cst_test.c',
            'join': 'xui_document_join_cst_test.c',
            'join_fault': 'xui_document_join_fault_test.c',
            'input_mapping': 'xui_document_input_mapping_test.c',
            'prepared_semantic': 'xui_document_prepared_semantic_test.c'
        }.get(name, 'xui_document_cst_' + name + '_test.c')
        run([*base, *flags, str(root / ('test_xui/' + source_name)),
            str(library), *link_flags, '-o', str(exe)])
        run([str(exe)])
if sys.platform != 'win32':
    print('Definition CST:', len(tests), 'normal/collision C drivers passed with native sanitizers.')
    raise SystemExit(0)
env = os.environ.copy(); env['PATH'] = str(root / 'build') + os.pathsep + env.get('PATH', '')
for name in ['cst_index', 'body_index', 'default_grammar', 'trivia_index', 'table_source_cst', 'table_row_cst', 'table_reference_cst', 'footnote_projection_cst', 'delete_source_cst', 'inline_range_cst', 'inline_boundary_cst', 'container_prefix_cst', 'deep_quote_cst', 'deep_quote_fault', 'nested_list_cst', 'nested_list_fault', 'nonparagraph_prefix_cst', 'nonparagraph_long_header', 'nonparagraph_prefix_fault', 'inline_split_cst', 'footnote_inline_newline', 'footnote_inline_newline_fault', 'deep_footnote_newline', 'wide_container_newline', 'deep_footnote_newline_fault', 'inline_split_diagnostic', 'implicit_reference_split_cst', 'reference_text_cst', 'reference_text_diagnostic', 'gfm_code_pipe_diagnostic', 'gfm_code_pipe_source', 'gfm_code_pipe_edit', 'html_runs', 'html_rowspan', 'html_table_numbers', 'html_character_reference', 'html_named_reference', 'html_markdown_reference', 'html_markdown_reference_fault', 'html_newline', 'literal_bracket_cst', 'gfm_code_pipe_fault', 'code_boundary_diagnostic', 'code_boundary_cst', 'code_boundary_fault', 'footnote_local_reference_diagnostic', 'footnote_local_code_cst', 'footnote_local_source', 'footnote_local_fault', 'footnote_local_code_fault', 'list_split_cst', 'join_cst', 'input_mapping']:
    exe = out / (name + '_dll.exe')
    run([*base, '-DXGE_DLL', '-DXUI_DLL', str(root / ('test_xui/xui_document_' + name + '_test.c')),
        str(root / 'src/xui_unicode.c'), str(root / 'build/xge.lib'), '-lm', '-lws2_32', '-liphlpapi', '-o', str(exe)])
    result = subprocess.run([str(exe)], cwd=root, env=env)
    rows.append({'command': [str(exe)], 'exit': result.returncode})
    (out / 'checks.json').write_text(json.dumps(rows, indent=2), encoding='utf-8')
    if result.returncode: raise SystemExit(result.returncode)
print('Definition CST:', len(tests), 'normal/collision C drivers and actual DLL definition/body/table public query/edit/stream/history passed.')
