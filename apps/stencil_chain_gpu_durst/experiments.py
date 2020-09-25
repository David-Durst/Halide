import subprocess, os, re, sys
import pandas as pd
env = os.environ.copy()
env["STENCILS"] = str(1)
env["HL_TARGET"] = "host-cuda"
#p = re.compile(r'Manually-tuned time: (\d+.\d+)ms')
subprocess.run(['mkdir', '-p', 'tmp'])
#print(subprocess.run(['make'], stdout=subprocess.PIPE, env=env).stdout.decode('utf-8'))
for stencil_size in [3,5]:
    for muladds in [1,2,3,4]:
        with open('time_' + str(muladds*25) + '_' + str(stencil_size) + 'x' + str(stencil_size) + '.csv', 'w') as time_csv:
            print("stencil_size: " + str(stencil_size) + ", muladds: " + str(muladds))
            time_csv.write("stencils,GPU Time Per Stage (us)\n")
            with open('logs.txt', 'w') as logs_txt:
                for i in range(1, 62, 10):
                    env["STENCILS"] = str(i)
                    env["MULADDS"] = str(muladds)
                    env["STENCIL_SIZE"] = str(stencil_size)
                    print("going to run " + str(i))
                    #print(subprocess.run(['rm', '-f', 'bin/host/stencil_chain.a'], stdout=subprocess.PIPE).stdout.decode('utf-8'))
                    #print(subprocess.run(['make', 'bin/host/stencil_chain.a'], stdout=subprocess.PIPE, env=env).stdout.decode('utf-8'))
                    subprocess.run(['make', 'clean'], stdout=subprocess.PIPE, env=env).stdout.decode('utf-8')
                    #print(subprocess.run(['make'], stdout=subprocess.PIPE, env=env).stdout.decode('utf-8'))
                    #res_run = subprocess.run(['nvprof', 'bin/host/process', '../images/rgb.png', '10', 'bin/host/out.png'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
                    res_run = subprocess.run(['make', 'test'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
                    res_stdout = res_run.stdout.decode('utf-8')
                    res_stderr = res_run.stderr.decode('utf-8')
                    #print("res_stdout: " + res_stdout)
                    #print("res_stderr: " + res_stderr)
                    #res_match = p.search(res_stdout)
                    logs_txt.write(res_stderr)
                    with open('tmp/' + str(i) + ".txt", 'w') as tmp_txt:
                        tmp_txt.write(res_stderr)
                    subprocess.run(['bash', 'get_per_stage_times.sh', 'tmp/' + str(i) + '.txt' ])
                    avg = pd.read_csv("tmp/" + str(i) + ".txt.nums", header=None).mean()[0]
                    time_csv.write(str(i) + "," + str(avg) + "\n")
                

