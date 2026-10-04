/** Check structural correctness of this benchmark's four-sequence serial output.
 * This establishes preserved inputs and sane distances, not biological optimality.
 */
export function validateOutput(output, input) {
  const fail=()=>{throw Error('Invalid serial benchmark output')};
  const records=new Map();
  let header;
  for(const line of input.trim().split(/\r?\n/)) {
    if(line.startsWith('>')) {
      header=line.slice(1).trim();
      if(!header||/\s/.test(header)||records.has(header)) fail();
      records.set(header,'');
    } else {
      if(!header||!/^[A-Z]+$/.test(line.trim())) fail();
      records.set(header,records.get(header)+line.trim());
    }
  }
  if(records.size!==4||[...records.values()].some(value=>!value)) fail();
  const parts=output.trim().split('Final Progressive Alignment:');
  if(parts.length!==2) fail();
  const lines=parts[0].trim().split(/\r?\n/);
  if(lines.shift()!=='Distance Matrix:'||lines.length!==4) fail();
  const matrix=lines.map(line=>line.trim().split(/\s+/).map(Number));
  if(matrix.some(row=>row.length!==4||row.some(value=>!Number.isFinite(value)||value<0||value>2))) fail();
  for(let i=0;i<4;i++) {
    if(matrix[i][i]!==0) fail();
    for(let j=0;j<4;j++) if(Math.abs(matrix[i][j]-matrix[j][i])>1e-6) fail();
  }
  const alignment=new Map([...records.keys()].map(key=>[key,'']));
  for(const line of parts[1].split(/\r?\n/).filter(line=>line.trim())) {
    const fields=line.trim().split(/\s+/);
    const key=fields.shift(), fragment=fields.join('');
    if(!alignment.has(key)||!fragment||!/^[A-Z.-]+$/.test(fragment)) fail();
    alignment.set(key,alignment.get(key)+fragment);
  }
  const widths=new Set([...alignment.values()].map(value=>value.length));
  if(widths.size!==1) fail();
  for(const [key,original] of records) if(alignment.get(key).replace(/[.-]/g,'')!==original) fail();
  return {matrix,alignment:Object.fromEntries(alignment)};
}
