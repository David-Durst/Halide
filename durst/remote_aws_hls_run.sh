#!/bin/bash -x
clean_name=$1
echo $AWS_ACCESS_KEY_ID
# clear arguments so not passed to sourced scripts
set --
source $AWS_FPGA_REPO_DIR/vitis_runtime_setup.sh

rm -f ${clean_name}_result.log
rm -f ${clean_name}.awsxclbin
rm -f ${clean_name}_host
aws s3 cp s3://durstfpgae1/fpga/${clean_name}.awsxclbin .
aws s3 cp s3://durstfpgae1/fpga/${clean_name}_host .
chmod u+x ${clean_name}_host
sleep 20
./${clean_name}_host ${clean_name}.awsxclbin &> ${clean_name}_result.log

