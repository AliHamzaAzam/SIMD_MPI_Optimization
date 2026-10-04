import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {validateOutput} from '../scripts/validate-output.mjs';

const read=name=>readFileSync(new URL(`../raw/${name}`,import.meta.url),'utf8');
const input=read('input-64.fasta');
const output=read('64-1.stdout');

test('all timed and warm-up outputs preserve complete alignments and valid distance matrices',()=>{
  for(const length of [64,256,512]) for(let trial=0;trial<=5;trial++)
    assert.doesNotThrow(()=>validateOutput(read(`${length}-${trial}.stdout`),read(`input-${length}.fasta`)));
});

test('rejects incomplete, nonfinite, asymmetric and out-of-range matrices',()=>{
  for(const replacement of ['NaN','Infinity','-0.1','2.1']) {
    const lines=output.split('\n');
    const row=lines[1].split(' ');row[1]=replacement;lines[1]=row.join(' ');
    assert.throws(()=>validateOutput(lines.join('\n'),input));
  }
  assert.throws(()=>validateOutput(output.replace('Distance Matrix:\n',''),input));
  const lines=output.split('\n');lines[1]='0 0 0 0';
  assert.throws(()=>validateOutput(lines.join('\n'),input));
  lines[1]='1 0 0 0';
  assert.throws(()=>validateOutput(lines.join('\n'),input));
});

test('rejects truncated, altered, missing and unequal-width alignment records',()=>{
  assert.throws(()=>validateOutput(output.split('Final Progressive Alignment:')[0]+'Final Progressive Alignment:\n',input));
  assert.throws(()=>validateOutput(output.replace(/sequence1\s+[A-Z. -]+/,'sequence1 A'),input));
  assert.throws(()=>validateOutput(output.replaceAll('sequence4','sequence5'),input));
  assert.throws(()=>validateOutput(output.replace(/(sequence1\s+)([A-Z])/, '$1X'),input));
  assert.throws(()=>validateOutput(output.replace(/(sequence1\s+[^\n]+)/,'$1.'),input));
});

test('recorded distances agree with an independent direct cosine transform and Pearson oracle',()=>{
  for(const length of [64,256,512]) {
    const sequences=read(`input-${length}.fasta`).trim().split('\n').filter(line=>!line.startsWith('>'));
    const alphabet=[...new Set(sequences.join(''))].sort();
    // Real FFT coefficients are independently evaluated as direct cosine sums.
    const coefficients=sequences.map(sequence=>Array.from({length},(_,k)=>
      [...sequence].reduce((sum,amino,j)=>sum+alphabet.indexOf(amino)*Math.cos(2*Math.PI*k*j/length),0)));
    const centered=coefficients.map(values=>{const mean=values.reduce((a,b)=>a+b,0)/length;return values.map(v=>v-mean)});
    const matrix=read(`${length}-1.stdout`).split('\n').slice(1,5).map(line=>line.split(' ').map(Number));
    for(let i=0;i<4;i++) for(let j=i+1;j<4;j++) {
      const dot=centered[i].reduce((sum,x,k)=>sum+x*centered[j][k],0);
      const norm=values=>Math.sqrt(values.reduce((sum,x)=>sum+x*x,0));
      const distance=1-dot/(norm(centered[i])*norm(centered[j]));
      assert.ok(Math.abs(matrix[i][j]-distance)<1e-6,`distance ${length}/${i}/${j} differs from oracle`);
    }
  }
});
