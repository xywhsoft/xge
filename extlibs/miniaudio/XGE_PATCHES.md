# XGE local miniaudio patch

Base: bundled miniaudio v0.11.25 (2026-03-04), retaining its public-domain/MIT-0 notices.
Unpatched header SHA-256: `ac7af4de748b7e26b777f37e01cee313a308a7296a3eb080e2906b320cc55c89`.
Patched header SHA-256: `884b16bb398a5104753692bdd543f5c91062d4604636ed110a5d3adc4ff3c004`.

The synchronous resource-manager data-buffer connector failure returned without
releasing the acquired cache-node reference or uninitializing the data-source
base. A subsequent failed Sound construction freed only the outer buffer.
The 112-byte cache node remained until the entire engine was destroyed.

The local change returns that node reference, uninitializes the base and clears
its pointer on connector failure. It changes no public ABI, successful data path,
PCM copy, allocation policy or steady-state playback call. No retry or error
suppression is introduced. Recheck/remove this patch when updating miniaudio.

`tests/xge/xge_audio_lifetime_test.c` modes 13-15 keep the same engine alive across
64 caller-thread allocation budgets for Sound, Music and Stream. Each iteration
requires exact heap/owner/invalid-free balance before any engine shutdown.
The unpatched Sound control fails under Windows/Linux at O0/O2; patched controls
pass. This does not claim injection into miniaudio worker-thread allocations.
Evidence: `docs/validation/audio-resident-20261111.json`.
