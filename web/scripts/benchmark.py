"""Supported cloud serial smoke benchmark, without changes to C++ sources."""
import hashlib, json, os, pathlib, platform, subprocess, time
root = pathlib.Path(__file__).resolve().parents[2]
raw = root / 'web/raw'
raw.mkdir(exist_ok=True)
flags = ['-std=c++17', '-O2']
binary = raw/'serial-benchmark'
subprocess.run(['g++', *flags, str(root/'MPI_Optimization/src/serial.cpp'), '-o', str(binary)], check=True)
rows=[]
inputs=[]
for length in (64,256,512):
    sequence='ARNDCQEGHILKMFPSTWYV' * (length//20+2)
    fixture=''.join(f'>sequence{i+1}\n{sequence[i:i+length]}\n' for i in range(4))
    path=raw/f'input-{length}.fasta'
    path.write_text(fixture)
    inputs.append({'length':length,'sha256':hashlib.sha256(fixture.encode()).hexdigest()})
    for trial in range(6):
        start=time.perf_counter_ns()
        result=subprocess.run([str(binary),str(path)],capture_output=True,check=True,timeout=30)
        seconds=(time.perf_counter_ns()-start)/1e9
        label=f'{length}-{trial}'
        (raw/f'{label}.stdout').write_bytes(result.stdout)
        (raw/f'{label}.stderr').write_bytes(result.stderr)
        if trial: rows.append({'length':length,'trial':trial,'seconds':seconds,'stdoutSha256':hashlib.sha256(result.stdout).hexdigest()})
(raw/'timings.json').write_text(json.dumps(rows,indent=2)+'\n')
cpu=next((x.split(':',1)[1].strip() for x in pathlib.Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name')),platform.machine())
source_hashes={p:hashlib.sha256((root/p).read_bytes()).hexdigest() for p in ['MPI_Optimization/src/serial.cpp','MPI_Optimization/src/utility.h']}
meta={'capturedAt':time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),'sourceCommit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),'sourceSha256':source_hashes,'machine':cpu,'os':platform.platform(),'architecture':platform.machine(),'visibleCpus':os.cpu_count(),'compiler':subprocess.check_output(['g++','--version'],text=True).splitlines()[0],'flags':' '.join(flags),'compileCommand':'g++ -std=c++17 -O2 ../MPI_Optimization/src/serial.cpp -o raw/serial-benchmark','runCommand':'raw/serial-benchmark raw/input-{length}.fasta','inputs':inputs,'input':'4 synthetic protein sequences at each length','scope':'Whole-process wall time including process launch, FASTA I/O, FFT, distance matrix, progressive alignment and output','trials':5,'warmups':1}
(raw/'machine.json').write_text(json.dumps(meta,indent=2)+'\n')
print(json.dumps(rows,indent=2))
