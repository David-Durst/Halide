import subprocess
import os
import pandas as pd
from io import StringIO
env = os.environ.copy()

def do_one_run(logs_txt, time_csv, channels, layers, batch_size, samples):
    time_csv.write(str(channels) + "," + str(layers) + "," + str(batch_size) + ","
                   + str(samples) + ",")
    env["CHANNELS"] = str(channels)
    env["LAYERS"] = str(layers)
    env["BATCH_SIZE"] = str(batch_size)
    env["SAMPLES"] = str(samples)
    subprocess.run(['make', 'clean'],
                    stdout=subprocess.PIPE, env=env).stdout.decode('utf-8')
    res_run = subprocess.run(['make', 'test'],
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
    res_stdout = res_run.stdout.decode('utf-8')
    res_stderr = res_run.stderr.decode('utf-8')
    logs_txt.write(res_stdout)
    logs_txt.write(res_stderr)
    with open('tmp/' + str(channels) + "_" + str(layers) + ".txt", 'w') as tmp_txt:
        tmp_txt.write(res_stdout)
        tmp_txt.write(res_stderr)
    latency_filtered = subprocess.run(['awk', '-f', 'filter_latency.awk',
                        'tmp/' + str(channels) + "_" + str(layers) + '.txt'],
                        stdout=subprocess.PIPE).stdout.decode('utf-8')
    throughput_filtered = subprocess.run(['awk', '-f', 'filter_throughput.awk',
                        'tmp/' + str(channels) + "_" + str(layers) + '.txt'],
                        stdout=subprocess.PIPE).stdout.decode('utf-8')
    avg_latency = pd.read_csv(StringIO(latency_filtered), header=None).mean()[0]
    avg_throughput = pd.read_csv(StringIO(throughput_filtered), header=None).mean()[0]
    time_csv.write(str(avg_latency) + ",")
    time_csv.write(str(avg_throughput) + "\n")

subprocess.run(['mkdir', '-p', 'tmp'])
with open('logs.txt', 'w') as logs_txt:
    with open('audio_results_x86.csv', 'w') as time_csv:
        time_csv.write("Channels,Layers,Batch Size,Time Series Length,Latency (ms),Throughput (GOps)\n")
        for channels in [256]:#[32, 64, 128, 256, 512, 1024]: #[10, 20, 50, 100]:
            for layers in [32]:#[8, 16, 32, 64]:
                for batch_size in [1, 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32, 40, 48, 64, 80, 96]:
                    samples = 12000
                    print("channels:" + str(channels) + ", layers: " + str(layers) +
                            ", batch_size: " + str(batch_size) + ", samples: " + str(samples))
                    do_one_run(logs_txt, time_csv, channels, layers, batch_size, samples)
                

