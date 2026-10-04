import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtempSync, cpSync, mkdirSync, readFileSync, writeFileSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { spawnSync } from 'node:child_process';

function checkMutation(mutate) {
  const root = mkdtempSync(join(tmpdir(), 'results-integrity-'));
  try {
    for (const path of ['web/scripts', 'web/raw', 'MPI_Optimization/src']) {
      cpSync(new URL(`../../${path}`, import.meta.url), join(root, path), { recursive: true });
    }
    mkdirSync(join(root, 'web/data'));
    mutate(root);
    const result = spawnSync(process.execPath, [join(root, 'web/scripts/generate.mjs')], { encoding: 'utf8' });
    assert.notEqual(result.status, 0, 'Corrupted evidence must stop generation');
  } finally { rmSync(root, { recursive: true, force: true }); }
}
function editJson(root, path, mutate) {
  const file = join(root, 'web/raw', path);
  const value = JSON.parse(readFileSync(file, 'utf8'));
  mutate(value);
  writeFileSync(file, JSON.stringify(value));
}
test('generator rejects modified stdout, stderr, source and input', () => {
  for (const path of ['web/raw/64-1.stdout', 'web/raw/64-1.stderr', 'web/raw/input-64.fasta', 'MPI_Optimization/src/serial.cpp']) {
    checkMutation(root => writeFileSync(join(root, path), 'corrupt evidence'));
  }
});
test('generator rejects duplicate input provenance', () => {
  checkMutation(root => editJson(root, 'machine.json', machine => { machine.inputs[1] = machine.inputs[0]; }));
});
test('generator rejects unsupported protocol and fractional trials', () => {
  checkMutation(root => editJson(root, 'machine.json', machine => { machine.flags = '-O0'; }));
  checkMutation(root => editJson(root, 'timings.json', rows => { rows[0].trial = 1.5; }));
});
