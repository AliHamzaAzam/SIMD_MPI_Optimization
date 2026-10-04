import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import ts from 'typescript';
const source = readFileSync(new URL('../src/report.ts', import.meta.url), 'utf8');
const compiled = ts.transpileModule(source, {compilerOptions: {target: ts.ScriptTarget.ES2022, module: ts.ModuleKind.ESNext}}).outputText;
const {renderReport} = await import(`data:text/javascript;base64,${Buffer.from(compiled).toString('base64')}`);
const data = JSON.parse(readFileSync(new URL('../data/results.json', import.meta.url), 'utf8'));
test('empty and malformed data show a clear state without invented timings', () => {
  assert.match(renderReport({...data, results: []}), /No measurements available/);
  for (const bad of [null, {}, {...data, machine: null}, {...data, results: [data.results[0]]}]) {
    const html = renderReport(bad);
    assert.match(html, /Results could not be displayed/);
    assert.doesNotMatch(html, /class="bar"|NaN|Infinity/);
  }
});
test('bad aggregates and invalid numeric samples are rejected', () => {
  for (const seconds of [0, -1, NaN, Infinity, 12345]) {
    const bad = structuredClone(data); bad.results[0].seconds = seconds;
    assert.match(renderReport(bad), /Results could not be displayed/);
  }
});
test('metadata is escaped and source URLs require an actual commit hash', () => {
  const changed = structuredClone(data); changed.machine.machine = '<img src=x onerror=alert(1)>';
  assert.doesNotMatch(renderReport(changed), /<img/);
  assert.match(renderReport(changed), /&lt;img/);
  changed.machine.sourceCommit = '" onclick="alert(1)';
  assert.match(renderReport(changed), /Results could not be displayed/);
});
test('chart uses milliseconds and exposes the matching accessible table', () => {
  const replacement = structuredClone(data);
  replacement.results[0] = {length:64, seconds:0.003, min:0.001, max:0.005, samples:[0.001,0.002,0.003,0.004,0.005]};
  const html = renderReport(replacement);
  assert.match(html, /3\.00 ms/);
  assert.match(html, /aria-describedby="runtime-table-caption"/);
  assert.match(html, /id="runtime-table-caption"/);
  assert.match(html, /tabindex="0" role="region" aria-label="Recorded runtime table/);
  assert.match(html, /Speedup: not measured/);
  assert.match(html, /Efficiency: not measured/);
});
