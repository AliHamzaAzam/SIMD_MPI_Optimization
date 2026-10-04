import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtempSync, readFileSync, rmSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {fileURLToPath} from 'node:url';
import {spawnSync} from 'node:child_process';

test('unchanged optimized native program reproduces recorded outputs without timing it', () => {
  const root = fileURLToPath(new URL('../../', import.meta.url));
  const temporary = mkdtempSync(join(tmpdir(), 'serial-correctness-'));
  try {
    const binary = join(temporary, 'serial');
    const compile = spawnSync('g++', ['-std=c++17', '-O2', join(root, 'MPI_Optimization/src/serial.cpp'), '-o', binary], {encoding: 'utf8', timeout: 30000});
    assert.equal(compile.status, 0, compile.stderr || String(compile.error));
    for (const length of [64, 256, 512]) {
      const result = spawnSync(binary, [join(root, `web/raw/input-${length}.fasta`)], {encoding: 'utf8', timeout: 30000});
      assert.equal(result.status, 0, result.stderr || String(result.error));
      assert.equal(result.stderr, '');
      assert.equal(result.stdout, readFileSync(join(root, `web/raw/${length}-1.stdout`), 'utf8'));
    }
  } finally { rmSync(temporary, {recursive: true, force: true}); }
});
