#!/bin/bash -x
# example - (base) durst@durst-VirtualBox:~/dev/halide/durst$ ./compile_aws_hls.sh -c ../../clockwork/soda_codes/downplus1/ -i ../../aws/durst-east1.pem -h $(cat ../../aws/hlsip.txt)


exit_abnormal() {
    echo "Usage: $0 [ -c remote_path ] [ -i ssh_identity_file ] [ -h aws_hls_server_ip_address ] "
    exit 1
}

while getopts "c:h:i:s:" options
do
    case ${options} in
        c)
            hls_dir=${OPTARG}
            ;;
        h)
            hls_ip=${OPTARG}
            ;;
        i)
            identity_file=${OPTARG}
            ;;
        \? | h | *)
            exit_abnormal
            ;;
    esac
done

clockwork_dir=${hls_dir}/../../

# since getopts doesn't do anything with no args
if [ $OPTIND -eq 1 ]; then
    exit_abnormal
fi

# check for AWS PERMISSIONS
if [ -z "$AWS_ACCESS_KEY_ID" ]; then
    echo "please set AWS_ACCESS_KEY_ID"
    exit 1
fi 
if [ -z "$AWS_SESSION_TOKEN" ]; then
    echo "please set AWS_SESSION_TOKEN"
    exit 1
fi 
if [ -z "$AWS_SECURITY_TOKEN" ]; then
    echo "please set AWS_SECURITY_TOKEN"
    exit 1
fi 

current_time=$(date "+%Y.%m.%d-%H")
builds_dir="~/clockwork_builds/${current_time}/"
aws_code_dir=${hls_dir}/our_code

result=1
while [ $result -ne 0 ];
do
    ssh -i $identity_file centos@${hls_ip} "tmux ls"
    running=$?
    ssh -i $identity_file centos@${hls_ip} "ls ${aws_code_dir}/*afi*"
    result=$?
    # succeed but won't upload if took too long, aws credentials expired
    ssh -i $identity_file centos@${hls_ip} "ls ${aws_code_dir}/build_dir.hw.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/reports/link/imp/kernel_util_synthed.rpt"
    too_long_result=$?
    if [ $running -ne 0 ] && [ $result -ne 0 ] && [ $too_long_result -ne 0 ];
    then 
        notify-send "Error Compiling ${aws_code_dir}"
        exit 1
    fi
    if [ $running -ne 0 ] && [ $result -ne 0 ] && [ $too_long_result -eq 0 ];
    then 
        ssh -i $identity_file centos@${hls_ip} "cat ${aws_code_dir}/build_dir.hw.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/reports/link/imp/kernel_util_synthed.rpt"
        notify-send "Done Compiling ${aws_code_dir}, too slow to upload automatically"
        exit 0
    fi
    ssh -i $identity_file centos@${hls_ip} "tail -n 2 ${aws_code_dir}/tmux.log"
    sleep 15
done

rm -f afi
scp -i $identity_file centos@${hls_ip}:"${aws_code_dir}/*afi*" afi.txt
afi_num=$(sed -n 's/.*FpgaImageId.*\(afi.*\)\",/\1/p' afi.txt)
agfi_num=$(sed -n 's/.*FpgaImageGlobalId.*\(agfi.*\)\"/\1/p' afi.txt)
echo $afi_num
echo $agfi_num
notify-send "Done Compiling ${aws_code_dir}" "afi: ${afi_num}"
