# Source layout

- `xge/` contains engine implementation grouped into core, render, text,
  input, resource, audio, particle, 3D, and platform subsystems.
- `xui/` contains UI implementation grouped into core, controls, code editor,
  document, workflow, and accessibility subsystems.

These are private implementation files. Public declarations live in
`../include/`; build source selection lives in `../config/modules.json`.
