#!/bin/bash -x
# (base) durst@durst-VirtualBox:~/dev/halide/durst$ ./run_aws_hls.sh -c ../../clockwork/soda_codes/brighter/ -i ../../aws/durst-east1.pem -h $(cat ../../aws/fpgaip.txt)


exit_abnormal() {
    echo "Usage: $0 [ -c app_directory_in_soda_codes ] [ -i ssh_identity_file ] [ -h aws_fpga_server_ip_address ] "
    exit 1
}

while getopts "c:h:i:s:" options
do
    case ${options} in
        c)
            hls_dir=${OPTARG}
            ;;
        h)
            fpga_ip=${OPTARG}
            ;;
        i)
            identity_file=${OPTARG}
            ;;
        \? | h | *)
            exit_abnormal
            ;;
    esac
done

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

clean_name=$(sed 's/[^a-zA-Z0-9]//g' <<< $(basename ${hls_dir}))

read -r -d '' run_tmux_command << EOM
export AWS_ACCESS_KEY_ID=$AWS_ACCESS_KEY_ID ;
export AWS_SESSION_TOKEN=$AWS_SESSION_TOKEN ;
export AWS_SECURITY_TOKEN=$AWS_SESSION_TOKEN ;
export AWS_SECRET_ACCESS_KEY=$AWS_SECRET_ACCESS_KEY ;
./remote_aws_hls_run.sh $clean_name
EOM

scp -i $identity_file remote_aws_hls_run.sh centos@${fpga_ip}:
ssh -i $identity_file centos@${fpga_ip} "$run_tmux_command"
scp -i $identity_file centos@${fpga_ip}:${clean_name}_result.log ${clean_name}_result.log
cat ${clean_name}_result.log
