<?php

/**
 * @file
 * Constants for aws_ec2_lib
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */

/**
 * Updated by yas 2011/02/04
 */

define('AWS_EC2_LIB_BUNDLE_IMAGE_SCRIPT', 'BUNDLE_IMAGE_SCRIPT');
/*
 Following constants are used for creating the table names
 A table will be CLOUD_PREFIX . $cloud_context . '_' . $table_name
 Check out the method aws_ec2_lib_get_table_info($cloud_context , $table_name )
 $table_name is ( AWS_EC2_LIB_INSTANCES_TABLE | CLOUD_BILLING_INSTANCES_DETAILS_TABLE | AWS_EC2_LIB_INSTANCES_LOCK_TABLE | ...)
 */

//After the database consolidation, aws_ec2_lib will be responsible for creating
//the database tables, the new tables will be cloud_aws_ebs_volumes, cloud_aws_images..etc
define('AWS_EC2_LIB_DB_CONTEXT'                 , 'aws');

define('AWS_EC2_LIB_CLOUD_PREFIX'                  , 'cloud_'                    );
define('AWS_EC2_LIB_CLOUD_NONE'                    , '- none -'                  );
define('AWS_EC2_LIB_SEED'                          , 'Your favorite word here'   );
define('AWS_EC2_LIB_TEMP_DIR'                      , 'cloud'                     );
define('AWS_EC2_LIB_INSTANCES_TABLE'               , 'instances'                 );
define('AWS_EC2_LIB_INSTANCES_LOCK_TABLE'          , 'instances_lock'            );
define('AWS_EC2_LIB_IMAGES_TABLE'                  , 'images'                    );
define('AWS_EC2_LIB_SSH_KEYS_TABLE'                , 'ssh_keys'                  );
define('AWS_EC2_LIB_SECURITY_GROUPS_TABLE'         , 'security_groups'           );
define('AWS_EC2_LIB_SECURITY_GROUPS_DETAILS_TABLE' , 'security_groups_details'   );
define('AWS_EC2_LIB_ZONE_TABLE'                    , 'zone'                      );
define('AWS_EC2_LIB_ELASTIC_IP_TABLE'              , 'elastic_ip'                );
define('AWS_EC2_LIB_SNAPSHOTS_TABLE'               , 'snapshots'                 );
define('AWS_EC2_LIB_EBS_VOLUMES_TABLE'             , 'ebs_volumes'               );
define('AWS_EC2_LIB_VOLUMES_DEVICE_OPTIONS_TABLE'  , 'ebs_volumes_device_options');
define('AWS_EC2_LIB_USER_KEYS_TABLE'               , 'user_keys'                 );
define('AWS_EC2_LIB_REGISTER_IMAGE_STATUS_TABLE'   , 'register_image_status'     );
define('AWS_EC2_LIB_SSH_CLIENT_JAR_FILE'           , 'mindterm.jar'              );
define('AWS_EC2_LIB_API_VERSION'                   , '2012-06-01'                );
define('AWS_EC2_LIB_HOST_URI'                      , 'ec2.amazonaws.com'         );
define('AWS_EC2_LIB_IMAGE_UPLOAD_URL'              , 'https://s3.amazonaws.com'  );
define('AWS_EC2_LIB_IMAGE_REGISTER_URL'            , 'https://ec2.amazonaws.com' );
define('AWS_EC2_LIB_OPENSTACK_NOVA_CONTEXT'        , 'openstack_nova'            );

define('AWS_EC2_LIB_PAGER_LIMIT'                   , 50                          );
define('AWS_EC2_LIB_TIME_LIMIT'                    , 1000                        );
define('AWS_EC2_LIB_BUNDLE_INSTANCES_TABLE'        , AWS_EC2_LIB_CLOUD_PREFIX . 'bundle_instances' );
