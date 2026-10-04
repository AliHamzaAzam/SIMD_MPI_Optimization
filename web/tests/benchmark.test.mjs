import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtempSync, mkdirSync, copyFileSync, writeFileSync, readFileSync, rmSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {spawnSync} from 'node:child_process';

test('a failed capture preserves the previous raw measurements', () => {
  const root = mkdtempSync(join(tmpdir(), 'capture-failure-'));
  try {
    mkdirSync(join(root, 'web/scripts'), {recursive: true});
    mkdirSync(join(root, 'web/raw'));
    mkdirSync(join(root, 'bin'));
    copyFileSync(new URL('../scripts/benchmark.py', import.meta.url), join(root, 'web/scripts/benchmark.py'));
    // Fake compiler succeeds but creates no executable: capture must fail safely.
    writeFileSync(join(root, 'bin/g++'), '#!/bin/sh\nexit 0\n', {mode: 0o755});
    const previous = join(root, 'web/raw/input-64.fasta');
    writeFileSync(previous, 'previous evidence');
    const result = spawnSync('python3', [join(root, 'web/scripts/benchmark.py')], {env: {PATH: `${join(root, 'bin')}:${process.env.PATH}`}, encoding: 'utf8'});
    assert.notEqual(result.status, 0);
    assert.equal(readFileSync(previous, 'utf8'), 'previous evidence');
  } finally { rmSync(root, {recursive: true, force: true}); }
});

test('successful staged capture validates all outputs before replacing evidence', async () => {
  const {cpSync} = await import('node:fs');
  const root = mkdtempSync(join(tmpdir(), 'capture-success-'));
  try {
    for (const path of ['web/scripts', 'web/raw', 'MPI_Optimization/src']) cpSync(new URL(`../../${path}`, import.meta.url), join(root, path), {recursive:true});
    mkdirSync(join(root, 'bin'));
    mkdirSync(join(root, 'fixtures'));
    for (const length of [64,256,512]) copyFileSync(new URL(`../raw/${length}-1.stdout`, import.meta.url), join(root, `fixtures/${length}.stdout`));
    const fakeProgram = `#!/usr/bin/env python3\nimport pathlib,sys\nlength=pathlib.Path(sys.argv[1]).stem.split('-')[1]\nsys.stdout.write((pathlib.Path(${JSON.stringify(root)})/'fixtures'/f'{length}.stdout').read_text())\n`;
    const compiler = `#!/usr/bin/env python3\nimport pathlib,sys\nif '--version' in sys.argv:\n print('Fake compiler for isolated harness test')\nelse:\n output=pathlib.Path(sys.argv[-1]);output.write_text(${JSON.stringify(fakeProgram)});output.chmod(0o755)\n`;
    writeFileSync(join(root,'bin/g++'), compiler, {mode:0o755});
    writeFileSync(join(root,'bin/git'), '#!/bin/sh\nprintf "%s\\n" b3427e5d11a8a8c42c35971435852358f0e4f2d2\n', {mode:0o755});
    const result = spawnSync('python3', [join(root,'web/scripts/benchmark.py')], {env:{PATH:`${join(root,'bin')}:${process.env.PATH}`},encoding:'utf8',timeout:30000});
    assert.equal(result.status,0,result.stderr);
    const machine=JSON.parse(readFileSync(join(root,'web/raw/machine.json'),'utf8'));
    const rows=JSON.parse(readFileSync(join(root,'web/raw/timings.json'),'utf8'));
    assert.equal(machine.compiler,'Fake compiler for isolated harness test');
    assert.equal(machine.warmups,1);
    assert.equal(rows.length,15);
    assert.equal(rows.some(row=>row.trial===0),false);
    assert.equal(rows.every(row=>row.seconds>0),true);
    assert.match(machine.compileCommand,/\.capture-.*serial-benchmark/);
    const validation=spawnSync(process.execPath,[join(root,'web/scripts/generate.mjs'),join(root,'web/raw')],{encoding:'utf8'});
    assert.equal(validation.status,0,validation.stderr);
  } finally {rmSync(root,{recursive:true,force:true});}
});
