#!/bin/bash -x
clean_name=$1
# clear arguments so not passed to sourced scripts
set --
source $AWS_FPGA_REPO_DIR/vitis_setup.sh
source ../set_app.sh

make TARGET=hw DEVICE=$AWS_PLATFORM all   

xclbin=$(find . -name \*.xclbin)
rm -rf to_aws/
# since this script is launched in the directory of a project, will
# use it's name as the hw project's name
$VITIS_DIR/tools/create_vitis_afi.sh -xclbin=$xclbin -o=$clean_name -s3_bucket=durstfpgae1 -s3_dcp_key=fgpa -s3_logs_key=logs
aws s3 cp ${clean_name}.awsxclbin s3://durstfpgae1/fpga/
aws s3 cp host s3://durstfpgae1/fpga/${clean_name}_host
