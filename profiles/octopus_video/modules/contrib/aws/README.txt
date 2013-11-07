Version
=======
Cloud 7.x-1.x

BASIC INFO
==========
- Redesigned simonc's work for Amazon EC2 API-related module.


INCLUDE FILES
=============
* Include Files:
  Consists of a pair of
- ec2_lib_xxxxx_ui.inc
- ec2_lib_Xxxxx_db.inc (xxxxx: EC2 entitiy name)


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

iaas
  +-modules
    *-amazon_ec2              (depends on ec2_api        ) (Amazon EC2        is part of AWS)
    x-amazon s3               (depends on s3_api         ) (Amazon S3         is part of AWS)
    x-amazon_smpledb          (depends on simpledb_api   ) (Amazon SimpleDB   is part of AWS)
    x-amazon_cloud_watch      (depends on cloud_watch_api) (Amazon CloudWatch is part of AWS)
    x-amazon_s3
      - ...
    x-amazon_sqs
      - ...
    x-amazon_simpledb
      - ...
    x-amazon_cloud_watch
      - ...


Copyright
=========
Copyright (c) 2011-2012 DOCOMO Innovations, Inc.


End of README.txt