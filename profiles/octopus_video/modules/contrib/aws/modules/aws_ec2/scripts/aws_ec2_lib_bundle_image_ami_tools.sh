#!/bin/bash

#
# @file
# Used for this module
# 
# Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
#
#

function updatestatus
{
    touch /tmp/clanavi/bundle_instance.$1;
}

updatestatus "initiated" 

DEBUG=1;

TMP_FILE_NAME=`date +%Y-%m-%d-%T`
#TODO
#LOG_FILE='/tmp/clanavi/bundle_image-'$TMP_FILE_NAME'.log';
LOG_FILE='/tmp/clanavi/bundle_image-test.log';


function log
{
  if [ "$DEBUG" -eq 1 ]; then
    echo $1 >> $LOG_FILE ;
  fi;
}

log "Started the Bundling Instance Process"


log "Checking if user is root"

if [ "$(id -u)" != "0" ]; then
  echo "This script must be run as root" 1>&2
  log "This script must be run as root" 1>&2
  updatestatus "failed" 
  exit 1
fi


GREP="`which grep`";
if which apt-get >/dev/null; then
  PACKAGE_MANAGEMENT="`which apt-get` "
else
  PACKAGE_MANAGEMENT="`which yum`"
fi

log "Using Package Manager $PACKAGE_MANAGEMENT"

CURL_OPTS=" --silent --retry 1 --retry-delay 1 --retry-max-time 1 "


function log_error
{
  echo -e $1;
  log $1
  updatestatus "failed" 
  exit 1;
}


function check_env
{
  if [ -f ~/.bashrc ]; then
    . ~/.bashrc;
  fi

  log "Check for Amazon EC2 toolkit"
  
  # Tools already exist
  if [ -z `which ec2-upload-bundle` ] ; then
    log "Amazon EC2 toolkit missing!"
    install_ec2;
  fi

  log "Done Check for Amazon EC2 toolkit"
  
  # Keys
  EC2_HOME_DIR='.ec2';
  EC2_PRIVATE_KEY_FILE="/tmp/clanavi/$EC2_HOME_DIR/pk.pem";
  EC2_CERT_FILE="/tmp/clanavi/$EC2_HOME_DIR/cert.pem";

  if [ ! -d "/tmp/clanavi/$EC2_HOME_DIR" ]; then
    mkdir -p "/tmp/clanavi/$EC2_HOME_DIR";
  fi

  log "Check for CERT file and PVT file"  
  install_ec2_env EC2_PRIVATE_KEY "$EC2_PRIVATE_KEY_FILE";
  install_ec2_env EC2_CERT        "$EC2_CERT_FILE";

  install_ec2_keys_files "$EC2_PRIVATE_KEY_FILE" "Private key";
  install_ec2_keys_files "$EC2_CERT_FILE" "Certificate";

  log "Done Check for CERT file and PVT file"  
  
}

function install_ec2_env
{
  # Variable Variable for $1
  EC2_VARIABLE=${!1};
  EC2_VARIABLE_NAME=$1;
  EC2_FILE=$2;

  # Variable Variable
  if [ -z "$EC2_VARIABLE" ]; then
    log "Amazon $EC2_VARIABLE_NAME is not set-up correctly!";

    if ! grep -q "$EC2_VARIABLE_NAME" ~/.bashrc > /dev/null; then
      echo "export $EC2_VARIABLE_NAME=$EC2_FILE" >> ~/.bashrc;
    fi;
    export $EC2_VARIABLE_NAME=$EC2_FILE;
    source ~/.bashrc
  else
    if ! grep -q "$EC2_VARIABLE_NAME" ~/.bashrc > /dev/null; then
      echo "export $EC2_VARIABLE_NAME=$EC2_FILE" >> ~/.bashrc;
    else
      log "Amazon $EC2_VARIABLE_NAME var installed";
    fi;
  fi
}

function install_ec2_keys_files
{
  EC2_FILE=$1;
  EC2_DESCRIPTION=$2;
  EC2_CONTENTS='';

  if [ ! -f "$EC2_FILE" ]; then
    log_error "Amazon $EC2_FILE does not exist, please copy your $EC2_DESCRIPTION to $EC2_FILE and re-run this script";
  else
    log "Amazon $EC2_FILE file installed";
  fi
}

function install_ec2
{
  log "Installing packages..."  
  for PACKAGE in curl wget tar bzip2 unzip zip symlinks unzip ruby; do
    if ! which "$PACKAGE" >/dev/null; then
      $PACKAGE_MANAGEMENT install -y $PACKAGE;
    fi
  done;

  log "Done installing packages..."  
  log "Checking AMI tools"  

  # AMI Tools
  if [ -z "`which ec2-upload-bundle`" ]; then
    curl -o /tmp/ec2-ami-tools.zip $CURL_OPTS --max-time 30 http://s3.amazonaws.com/ec2-downloads/ec2-ami-tools.zip
    rm -rf /usr/local/ec2-ami-tools;
    cd /usr/local && unzip /tmp/ec2-ami-tools.zip
    ln -svf `find . -type d -name ec2-ami-tools*` ec2-ami-tools
    chmod -R go-rwsx ec2* && rm -rvf /tmp/ec2*
  fi

  log "Done Checking AMI tools"  
  
  ln -sf /usr/local/ec2-ami-tools/bin/* /usr/bin/;
  rm -f /usr/bin/ec2-*.cmd;
}

log "Checking environment [ec2 tools]"
check_env
log "Done Checking environment [ec2 tools]"


imagename=@CLANAVI_IMAGE_NAME
bucket=@CLANAVI_BUCKET_NAME/@CLANAVI_FOLDER_NAME
size=@CLANAVI_SIZE

export AWS_USER_ID='@CLANAVI_AWS_USER_ID'
export AWS_ACCESS_KEY_ID='@CLANAVI_AWS_ACCESS_KEY_ID'
export AWS_SECRET_ACCESS_KEY='@CLANAVI_AWS_SECRET_ACCESS_KEY'
export AWS_IMAGE_UPLOAD_URL='@CLANAVI_IMAGE_UPLOAD_URL'

if [ $(uname -m) = 'x86_64' ]; then
  arch=x86_64
else
  arch=i386
fi


log "Starting ec2-bundle-vol---Help"
ec2-bundle-vol --help >> $LOG_FILE
RESULT=$?
if [ $RESULT -eq 0 ];then
   log "ec2-bundle-vol---Help : Success"
else
   log "ec2-bundle-vol---Help : Failed"
   updatestatus "failed" 
   exit 125
fi

log "Logging:: $arch -d /mnt -p $imagename  -u $AWS_USER_ID -k $EC2_PRIVATE_KEY_FILE -c $EC2_CERT_FILE -s $size -e /mnt,/root/.ssh,/tmp/clanavi/$EC2_HOME_DIR >> $LOG_FILE"
log "Starting ec2-bundle-vol"
ec2-bundle-vol -r $arch -d /mnt -p $imagename  -u $AWS_USER_ID -k $EC2_PRIVATE_KEY_FILE -c $EC2_CERT_FILE -s $size -e /mnt,/root/.ssh,/tmp/clanavi/$EC2_HOME_DIR >> $LOG_FILE
RESULT=$?
if [ $RESULT -eq 0 ];then
   log "ec2-bundle-vol : Success"
else
   log "ec2-bundle-vol : Failed $RESULT "
   updatestatus "failed" 
   exit 125
fi


log "Starting ec2-upload-bundle"
ec2-upload-bundle  -b $bucket  -m /mnt/$imagename.manifest.xml -a "$AWS_ACCESS_KEY_ID" -s "$AWS_SECRET_ACCESS_KEY" --url "$AWS_IMAGE_UPLOAD_URL" >> $LOG_FILE
RESULT=$?
if [ $RESULT -eq 0 ];then
   log "ec2-upload-bundle : Success"
else
   log "ec2-upload-bundle : Failed"
   updatestatus "failed" 
   exit 126
fi


#log "Starting ec2-register"
#ec2-register  -K $EC2_PRIVATE_KEY_FILE -C $EC2_CERT_FILE $bucket/$imagename.manifest.xml -U '@CLANAVI_IMAGE_REGISTER_URL' >> $LOG_FILE
#RESULT=$?
#if [ $RESULT -eq 0 ];then
#   log "ec2-register : Success"
#else
#   log "ec2-register : Failed"
#   updatestatus "failed" 
#   exit 127
#fi

#log "Completed the Bundling Instance Process Successfully"

rm -fr /tmp/clanavi/$EC2_HOME_DIR ;

updatestatus "success"





