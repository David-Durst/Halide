import subprocess, os, re, sys
import pandas as pd
from io import StringIO
env = os.environ.copy()
env["HL_TARGET"] = "host-cuda"
#p = re.compile(r'Manually-tuned time: (\d+.\d+)ms')
subprocess.run(['mkdir', '-p', 'tmp'])
#print(subprocess.run(['make'], stdout=subprocess.PIPE, env=env).stdout.decode('utf-8'))
with open('exposure_time.csv', 'w') as time_csv:
    time_csv.write("Image Size,GPU Time (us)\n")
    with open('logs.txt', 'w') as logs_txt:
        for x_size in [256,512,1024,2048,4096,8192]:
            print("going to run " + str(x_size) + "x" + str(x_size))
            env["X_MAX_PARAM"] = str(x_size)
            env["Y_MAX_PARAM"] = str(x_size)
            env["RGB_FILE"] = "rgba_" + str(x_size) + "x" + str(x_size) + ".png"
            subprocess.run(['make', 'clean'], stdout=subprocess.PIPE, env=env).stdout.decode('utf-8')
            res_run = subprocess.run(['make', 'prof'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
            res_stdout = res_run.stdout.decode('utf-8')
            res_stderr = res_run.stderr.decode('utf-8')
            #print("res_stdout: " + res_stdout)
            #print("res_stderr: " + res_stderr)
            #res_match = p.search(res_stdout)
            logs_txt.write(res_stderr)
            with open('tmp/' + str(x_size) + ".txt", 'w') as tmp_txt:
                tmp_txt.write(res_stderr)
            #subprocess.run(['bash', 'get_per_stage_times.sh', 'tmp/' + str(x_size) + '.txt' ])
            filter_stdout = subprocess.run(['awk', '-f', 'filter_times.awk',
                                            'tmp/' + str(x_size) + '.txt'],
                                           stdout=subprocess.PIPE).stdout.decode('utf-8')
            total = pd.read_csv(StringIO(filter_stdout), header=None).sum()[0]
            # divide by 100 since 100 samples
            time_csv.write(str(x_size) + "x" + str(x_size) + "," + str(total/100) + "\n")


