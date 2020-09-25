import subprocess
import os
import pandas as pd
from io import StringIO
env = os.environ.copy()
env["STENCILS"] = str(1)
env["HL_TARGET"] = "host"

def do_one_run(logs_txt, time_csv, x_size, stencil_size, muladds, i):
    time_csv.write(str(x_size) + "," + str(stencil_size) + ","
                   + str(muladds*26) + "," + str(i) + ",")
    env["X_SIZE"] = str(x_size)
    env["STENCILS"] = str(i)
    env["MULADDS"] = str(muladds)
    env["STENCIL_SIZE"] = str(stencil_size)
    print("going to run " + str(i))
    subprocess.run(['make', 'clean'],
                    stdout=subprocess.PIPE, env=env).stdout.decode('utf-8')
    res_run = subprocess.run(['make', 'test'],
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
    res_stdout = res_run.stdout.decode('utf-8')
    res_stderr = res_run.stderr.decode('utf-8')
    logs_txt.write(res_stdout)
    logs_txt.write(res_stderr)
    with open('tmp/' + str(i) + ".txt", 'w') as tmp_txt:
        tmp_txt.write(res_stdout)
        tmp_txt.write(res_stderr)
    filter_stdout = subprocess.run(['awk', '-f', 'filter_times.awk',
                        'tmp/' + str(i) + '.txt'],
                        stdout=subprocess.PIPE).stdout.decode('utf-8')
    best_time = pd.read_csv(StringIO(filter_stdout), header=None).min()[0]
    time_csv.write(str(best_time) + "\n")

subprocess.run(['mkdir', '-p', 'tmp'])
with open('logs.txt', 'w') as logs_txt:
    with open('stencil_time.csv', 'w') as time_csv:
        time_csv.write("Image Width,Stencil Size, Ops Per Stage, Stencil Stages, Latency (ms)\n")
        for x_size in [10, 20, 50, 100,200,500]: #[10, 20, 50, 100]:
            for stencil_size in [5,7]:
                for muladds in [1,2,3]:
                    print("x_size:" + str(x_size) + ", stencil_size: " +
                          str(stencil_size) + ", muladds: " + str(muladds))
                    for i in range(1, 52, 10):
                        do_one_run(logs_txt, time_csv, x_size, stencil_size, muladds, i)
                

