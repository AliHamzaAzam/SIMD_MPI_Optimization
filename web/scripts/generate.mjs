import {readFileSync,writeFileSync} from 'node:fs';
import {fileURLToPath} from 'node:url';
import {resolve} from 'node:path';
import {createHash} from 'node:crypto';
export function summarize(rows) {
 if(!Array.isArray(rows)||!rows.length) throw Error('No timing samples');
 for(const r of rows) if(![64,256,512].includes(r.length)||!Number.isFinite(r.seconds)||r.seconds<=0||!r.stdoutSha256||!Number.isInteger(r.trial)||r.trial<1||r.trial>5) throw Error('Invalid timing sample');
 return [64,256,512].map(length=>{
 const group=rows.filter(r=>r.length===length);
 if(group.length!==5||new Set(group.map(r=>r.trial)).size!==5) throw Error('Expected five independent samples');
 if(new Set(group.map(r=>r.stdoutSha256)).size!==1) throw Error('Outputs differ within dataset');
 const samples=group.map(r=>r.seconds).sort((a,b)=>a-b);
 return {length,seconds:samples[2],min:samples[0],max:samples[4],samples};
 });
}
export function generate(){
 const raw=new URL('../raw/',import.meta.url);
 const machine=JSON.parse(readFileSync(new URL('machine.json',raw),'utf8'));
 const rows=JSON.parse(readFileSync(new URL('timings.json',raw),'utf8'));
 if(machine.trials!==5||machine.warmups!==1||machine.flags!=='-std=c++17 -O2'||!Array.isArray(machine.inputs)||machine.inputs.length!==3||[64,256,512].some(length=>machine.inputs.filter(input=>input.length===length).length!==1)) throw Error('Unsupported measurement protocol');
 const hash=bytes=>createHash('sha256').update(bytes).digest('hex');
 for(const row of rows){
  const output=readFileSync(new URL(`${row.length}-${row.trial}.stdout`,raw));
  if(hash(output)!==row.stdoutSha256||!output.toString().includes('Final Progressive Alignment:')) throw Error('Raw output integrity failure');
  if(readFileSync(new URL(`${row.length}-${row.trial}.stderr`,raw),'utf8').trim()) throw Error('Unexpected benchmark stderr');
 }
 for(const input of machine.inputs) if(hash(readFileSync(new URL(`input-${input.length}.fasta`,raw)))!==input.sha256) throw Error('Input checksum mismatch');
 for(const path of ['MPI_Optimization/src/serial.cpp','MPI_Optimization/src/utility.h']) if(hash(readFileSync(new URL(`../../${path}`,import.meta.url)))!==machine.sourceSha256[path]) throw Error('Source checksum mismatch');
 return {machine,results:summarize(rows)};
}
if(process.argv[1]&&resolve(process.argv[1])===fileURLToPath(import.meta.url))writeFileSync(new URL('../data/results.json',import.meta.url),JSON.stringify(generate(),null,2)+'\n');
