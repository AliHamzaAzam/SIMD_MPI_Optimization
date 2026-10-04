import {readFileSync, writeFileSync} from 'node:fs';
import {fileURLToPath, pathToFileURL} from 'node:url';
import {resolve, sep} from 'node:path';
import {createHash} from 'node:crypto';
import {validateOutput} from './validate-output.mjs';

export function summarize(rows) {
  if (!Array.isArray(rows) || !rows.length) throw Error('No timing samples');
  for (const row of rows) {
    if (!row || ![64, 256, 512].includes(row.length) || !Number.isFinite(row.seconds) || row.seconds <= 0 || !row.stdoutSha256 || !Number.isInteger(row.trial) || row.trial < 1 || row.trial > 5) throw Error('Invalid timing sample');
  }
  return [64, 256, 512].map(length => {
    const group = rows.filter(row => row.length === length);
    if (group.length !== 5 || new Set(group.map(row => row.trial)).size !== 5) throw Error('Expected five independent samples');
    if (new Set(group.map(row => row.stdoutSha256)).size !== 1) throw Error('Outputs differ within dataset');
    const samples = group.map(row => row.seconds).sort((a,b) => a-b);
    return {length, seconds: samples[2], min: samples[0], max: samples[4], samples};
  });
}

export function generate(rawDirectory = new URL('../raw/', import.meta.url)) {
  const raw = typeof rawDirectory === 'string' ? pathToFileURL(resolve(rawDirectory) + sep) : rawDirectory;
  const machine = JSON.parse(readFileSync(new URL('machine.json', raw), 'utf8'));
  const rows = JSON.parse(readFileSync(new URL('timings.json', raw), 'utf8'));
  const results = summarize(rows);
  if (machine.trials !== 5 || machine.warmups !== 1 || machine.flags !== '-std=c++17 -O2' || !Array.isArray(machine.inputs) || machine.inputs.length !== 3 || [64,256,512].some(length => machine.inputs.filter(input => input?.length === length).length !== 1)) throw Error('Unsupported measurement protocol');
  if (!/^[0-9a-f]{40}$/.test(machine.sourceCommit) || ['capturedAt', 'machine', 'os', 'architecture', 'compiler', 'scope'].some(key => typeof machine[key] !== 'string' || !machine[key].trim())) throw Error('Incomplete machine provenance');
  const hash = bytes => createHash('sha256').update(bytes).digest('hex');
  for (const input of machine.inputs) {
    const fasta = readFileSync(new URL(`input-${input.length}.fasta`, raw), 'utf8');
    if (hash(fasta) !== input.sha256) throw Error('Input checksum mismatch');
    const expectedHash = rows.find(row => row.length === input.length).stdoutSha256;
    // Trial zero is the retained warm-up, excluded from all timing statistics.
    for (let trial = 0; trial <= 5; trial++) {
      const output = readFileSync(new URL(`${input.length}-${trial}.stdout`, raw), 'utf8');
      if (hash(output) !== expectedHash) throw Error('Raw output integrity failure');
      if (readFileSync(new URL(`${input.length}-${trial}.stderr`, raw), 'utf8').trim()) throw Error('Unexpected benchmark stderr');
      validateOutput(output, fasta);
    }
  }
  for (const path of ['MPI_Optimization/src/serial.cpp', 'MPI_Optimization/src/utility.h']) {
    if (hash(readFileSync(new URL(`../../${path}`, import.meta.url))) !== machine.sourceSha256[path]) throw Error('Source checksum mismatch');
  }
  return {machine, results};
}
if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const raw = process.argv[2];
  const result = generate(raw);
  if (raw) process.stdout.write(JSON.stringify(result, null, 2) + '\n');
  else writeFileSync(new URL('../data/results.json', import.meta.url), JSON.stringify(result, null, 2) + '\n');
}
