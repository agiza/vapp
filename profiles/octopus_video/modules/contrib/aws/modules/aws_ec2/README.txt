ORIGINAL PROJECT
================
- Please see at:
  http://drupal.org/project/aws
  http://drupal.org/node/214944


BASIC INFO
==========

- Amazon EC2 API is originally created by simonc.
- We redesigned simonc's work for Amazon EC2 API-related module.
- Amazon EC2 Library provides wrapper functions for Amazon EC2 API
  It handles accessing to Amazon EC2 cloud by calling EC2 REST API.


LIMITATION
==========

- Some of features are not implemented


MODULE DEPENDENCY
=================

This module works with the following modules:

- Cloud
- AWS Common (Parent module)
- REST Client
 

DIRECTORY STRUCTURE FOR AWS MODULE FAMILY
=========================================

aws
  +-aws (depends on REST Client module)
  +-modules (Will be called by other EC2 API compatible clouds like Eucalyptus,  OpenStack nova and etc.)
    +-aws_ec2
      - aws_ec2_api.module (depends on aws_common) 
      - aws_ec2_lib.module (depends on ec2_api)
      - << EBS Volume wrapper     >> (.inc)
      - << Elastic IP wrapper     >> (.inc)
      - << Images wrapper         >> (.inc)
      - << Instances wrapper      >> (.inc)
      - << Register Image wrapper >> (.inc)
      - << Security Group wrapper >> (.inc)
      - << Snapshot wrapper       >> (.inc)
      - << SSH Key>> (User Keys Management based on Permission) wrapper (.inc)
      - ...

cloud/modules/iaas
  +-modules
    *-aws_cloud               (depends on ec2_api        ) (Amazon EC2        is part of AWS)
    x-amazon_s3               (depends on s3_api         ) (Amazon S3         is part of AWS)
    x-amazon_smpledb          (depends on simpledb_api   ) (Amazon SimpleDB   is part of AWS)
    x-amazon_cloud_watch      (depends on cloud_watch_api) (Amazon CloudWatch is part of AWS)
      - ...
    x-amazon_sqs
      - ...
    x-amazon_simpledb
      - ...
    x-amazon_cloud_watch
      - ...


x... NOTE: NOT IMPLEMENTED.  SHOWN ONLY AS A REFERENCE AND PROPOSED STRUCTURE SPEC.


FUTURE REFACTORING FOR FUNCTION NAMES
=====================================

(Files)

* ec2_lib_common.inc
* ec2_lib_common_db.inc

(Functions)

- ec2_lib_get_data
- ec2_lib_get_image_data

(Files)

* ec2_lib_ebs_volume_ui.inc
* ec2_lib_ebs_volume_db.inc

(Functions)

- ec2_lib_ebs_volume_create
- ec2_lib_ebs_volume_delest
- ec2_lib_ebs_volume_list

(Files)

* ec2_lib_image_ui.inc
* ec2_lib_image_db.inc

(Functions)

- ec2_lib_image_register
- ec2_lib_image_register_response
- ec2_lib_image_list
- ec2_lib_image_bundle
- ec2_lib_image_delete

(Files)

* ec2_lib_instance_ui.inc
* ec2_lib_instance_db.inc

(Functions)

- ec2_lib_instance_info
- ec2_lib_instance_volume_info
- ec2_lib_instance_monitor_info
- ec2_lib_instance_console
- ec2_lib_instance_script_exec_output
- ec2_lib_instance_get_lock_status
- ec2_lib_instance_terminate
- ec2_lib_instance_get_status
- ec2_lib_instance_get_key_material
- ec2_lib_instance_volume_attach
- ec2_lib_instance_volume_dettach
- ec2_lib_elastic_ips_get

(Files)

* ec2_lib_snapshot_ui.inc
* ec2_lib_snapshot_db.inc

(Functions)

- ec2_lib_snapshot_delete
- ec2_lib_snapshot_list

(Files)

* ec2_lib_ssh_keys_ui.inc
* ec2_lib_ssh_keys_db.inc
* ec2_lib_user_keys_db.inc

(Functions)

- ec2_lib_ssh_key_create
- ec2_lib_ssh_key_info
- ec2_lib_ssh_key_list
- ec2_lib_ssh_keys_check_load
- ec2_lib_user_key_get_all

(Files)

* ec2_lib_register_image_db.inc

(Functions)

- ec2_lib_resgister_image_get
- ec2_lib_resgister_image_save
- ec2_lib_resgister_image_update


Copyright
=========
Copyright (c) 2011-2012 DOCOMO Innovations, Inc.


End of README.txt