<?php

/**
 * @file
 * Provides easy API hook functionality for other Drupal modules
 * wanting to access AWS Elastic Compute Cloud (EC2)
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */

/**
 * Registers an AMI with Amazon EC2. Images must be registered before they can be launched
 * @param string $location   Full path to your AMI manifest in Amazon S3 storage.
 * @return std class object containing response headers & raw body
 */
function aws_ec2_api_register_image($cloud_context, $location) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'RegisterImage',
    'ImageLocation' => $location,
    'Version' => $api_version,
  );
  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_deregister_image($cloud_context, $image_id) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DeregisterImage',
    'ImageId' => $image_id,
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * Returns information about AMIs available to the user. This includes public AMIs available
 * for any user to launch,  private AMIs owned by the user making the request,  and private AMIs
 * owned by other users for which the user has explicit launch permissions.
 * @param array $images   A list of image descriptions
 * @param array $owners   Owners of AMIs to describe.
 * @param array $users    Users who have access.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_describe_images($cloud_context, $host, $images = array(), $owners = array(), $users = array()) {

  $api_version = cloud_get_info( $cloud_context, 'api_version' );
  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );

  $request = array(
    'Action' => 'DescribeImages',
    'Version' => $api_version,
  );

  $request = array_merge($request, aws_number_keys('ImageId'     , $images));
  $request = array_merge($request, aws_number_keys('Owner'       , $owners));
  $request = array_merge($request, aws_number_keys('ExecutableBy', $users ));

  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Deregisters an AMI. Once deregistered,  instances of the AMI can no longer be launched.
 * @param string $image_id   Unique ID of a machine image,  returned by a call to aws_ec2_api_RegisterImage or aws_ec2_api_DescribeImages.
 * @return object std class object containing response headers & raw body
 */


/**
 * Launches a specified number of instances.
 * @param $image_id    string  ID of the AMI with which to launch instances.
 * @param $min         int     Minimum number of instances to launch.
 * @param $max         int     Maximum number of instances to launch.
 * @param $key_name    string  Name of the key pair with which to launch instances.
 * @param $groups      array   Names of the security groups with which to associate the instances.
 * @param $data        string  The user data available to the launched instances.
 * @param $type        string  This specifies the instance type.  Options include m1.small,  m1.large,  and m1.xlarge.
 * @param $zone        string  This specifies the placement constraints (Availability Zones) for launching the instances.
 * @param $kernel_id   string  This specifies the ID of the kernel with which to launch the instance.
 * @param $ramdisk_id  string  This specifies the ID of the RAM disk with which to launch the instance.
 * @return object     std class object containing response headers & raw body
 */
function aws_ec2_api_run_instances($cloud_context, $image_id, $min, $max, $key_name = NULL, $groups = array(), $data = NULL, $type = NULL, $zone = NULL, $kernel_id  = NULL, $ramdisk_id  = NULL, $params = array()) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'RunInstances',
    'ImageId' => $image_id,
    'MinCount' => $min,
    'MaxCount' => $max,
    'InstanceType' => $type,
    'Version' => $api_version,
  );

  if ( isset($data) && empty($data) == FALSE ) {

    $request['UserData'] = drupal_encode_path($data);
  }

  if ( isset($key_name)
    && $key_name != AWS_EC2_LIB_CLOUD_NONE) {

    $request['KeyName'] = $key_name;
  }

  if ( isset($groups) == FALSE || count($groups) == 0 ) {

    $groups[] = 'default';
  }

  if ( $zone != 'any' ) {

    $request['Placement.AvailabilityZone'] = $zone;
  }

  if ( $kernel_id != AWS_EC2_LIB_CLOUD_NONE) {

    $request['KernelId'] = $kernel_id;
  }

  if ( $ramdisk_id != AWS_EC2_LIB_CLOUD_NONE) {

    $request['RamdiskId'] = $ramdisk_id;
  }

  $request = array_merge($request,    aws_number_keys('SecurityGroup', $groups));
  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Returns information about instances that you own.  If you specify one or more instance IDs,  Amazon EC2
 * returns information for those instances. If you do not specify instance IDs,  Amazon EC2 returns
 * information for all relevant instances.
 * @param array $instances   A list of instance IDs.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_describe_instances($cloud_context, $instances = array()) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DescribeInstances',
    'Version' => $api_version,
    //  'Version' => "\'$api_version\'",
  );
  $request = array_merge($request,    aws_number_keys('InstanceId', $instances));

  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * Reboot one or more instances.
 * @param array $instances   One or more instance IDs.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_reboot_instances($cloud_context, $instances) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'RebootInstances',
    'Version' => $api_version,
  );

  $request = array_merge($request, aws_number_keys('InstanceId', $instances));

  return aws_query_request($cloud_context, $host_uri, $request);
}



/**
 * Shuts down one or more instances. This operation is idempotent; if you terminate an instance more than once, each call will succeed.
 * @param array $instances   One or more instance IDs.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_terminate_instances($cloud_context, $instances) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'TerminateInstances',
    'Version' => $api_version,
  );

  $request = array_merge($request, aws_number_keys('InstanceId', $instances));

  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * Shuts down one or more instances. This operation is idempotent; if you terminate an instance more than once,  each call will succeed.
 * @param array $instances   One or more instance IDs.
 * @return object std class object containing response headers & raw body
 */
// TODO: NEEDS TO REVIEW - Function is not being used anywhere.
/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_scaling_manager_terminate_instances($cloud_context, $instances) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    // TODO: DEPENDENCY
    'Action' => '_cloud_scaling_manager_terminate_instances',
    'Version' => $api_version,
  );
  $request = array_merge($request, aws_number_keys('InstanceId', $instances));

  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Returns TRUE if the specified product code is attached to the specified instance.  The operation returns FALSE
 * if the product code is not attached to the instance.  This feature is useful when an AMI owner is providing
 * support and wants to verify whether a user's instance is eligible.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_confirm_product_instance($cloud_context, $image_id, $min, $max, $key_name = NULL, $groups = array(), $data = NULL, $type = NULL) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'ConfirmProductInstance',
    'ProductCode' => $image_id,
    'InstanceId' => $min,
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Creates a new 2048 bit RSA key pair and returns a unique ID that can be used to reference this key
 * pair when launching new instances.
 * @param string $key_name   A unique name for the key pair.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_create_keypair($cloud_context, $key_name) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'CreateKeyPair',
    'KeyName' => $key_name,
    'Version' => $api_version,
  );
  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Returns information about key pairs available to you. If you specify key pairs,  information
 * about those key pairs is returned. Otherwise,  information for all registered key pairs is returned.
 * @param string $key_name   Key pair IDs to describe.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_describe_keypairs($cloud_context, $key_name = array()) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DescribeKeyPairs',
    'Version' => $api_version,
  );
  $request = array_merge($request,    aws_number_keys('KeyName', $key_name));
  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Deletes a key pair.
 * @param string $key_name   Name of the key pair to delete.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_delete_keypair($cloud_context, $key_name) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DeleteKeyPair',
    'KeyName' => $key_name,
    'Version' => $api_version,
  );
  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Modifies an attribute of an AMI.
 * @param string $image_id   AMI ID to edit.
 * @param string $attribute Specifies the attribute to edit. 'launchPermission' and 'productCodes' supported.
 * @param string $op_type    Specifies the operation to perform on the attribute. 'add' and 'remove' supported.
 * @param array  $users     User IDs to add to or remove from the launchPermission attribute.
 * @param array  $groups    User groups to add to or remove from the launchPermission attribute. Currently,  the all group is available,  which will make it a public AMI.
 * @param array  $products  Attaches a product code to the AMI. Currently only one product code can be associated with an AMI. Once set,  the product code cannot be changed or reset.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_modify_image_attribute($cloud_context, $image_id, $attribute, $op_type = NULL, $users = array(), $groups = array(), $products = array()) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'ModifyImageAttribute',
    'ImageId' => $image_id,
    'Attribute' => $attribute,
    'OperationType' => $op_type,
    'Version' => $api_version,
  );
  $request = array_merge($request, aws_number_keys('UserId'     , $users   ));
  $request = array_merge($request, aws_number_keys('UserGroup'  , $groups  ));
  $request = array_merge($request, aws_number_keys('ProductCode', $products));

  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Returns information about an attribute of an AMI. Only one attribute can be specified per call.
 * @param string $image_id   ID of the AMI for which an attribute will be described.
 * @param string $attribute Specifies the attribute to describe. 'launchPermission' and 'productCodes' supported.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_describe_image_attribute($cloud_context, $image_id, $attribute) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DescribeImageAttribute',
    'ImageId' => $image_id,
    'Attribute' => $attribute,
    'Version' => $api_version,
  );
  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Resets an attribute of an AMI to its default value.  The productCodes attribute cannot be reset.
 * @param string $image_id   ID of the AMI for which an attribute will be reset.
 * @param string $attribute  Specifies the attribute to reset. Currently,  only launchPermission is supported. In the case of launchPermission,  all public and explicit launch permissions for the AMI are revoked.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_reset_image_attribute($cloud_context, $image_id, $attribute) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'ResetImageAttribute',
    'ImageId' => $image_id,
    'Attribute' => $attribute,
    'Version' => $api_version,
  );
  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Creates a new security group.
 * @param string $name   Name of the new security group.
 * @param string $description   Description of the new security group.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_create_security_group($cloud_context, $name, $description) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'CreateSecurityGroup',
    'GroupName' => $name,
    'GroupDescription' => $description,
    'Version' => $api_version,
  );
  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Returns information about security groups that you own.
 * @param string $groups   List of security groups to describe.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_describe_security_groups($cloud_context, $groups = array()) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DescribeSecurityGroups',
    'Version' => $api_version,
  );
  if (! is_array($groups)) {
    $groups[] = $groups;
  }
  $request = array_merge($request,    aws_number_keys('GroupName', $groups));
  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Deletes a security group.
 * @param string $name   Name of the security group to delete.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_delete_security_group($cloud_context, $name) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DeleteSecurityGroup',
    'GroupName' => $name,
    'Version' => $api_version,
  );
  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Adds permissions to a security group.
 * @param string $name   Name of the group to edit.
 * @param string $src_name   Name of security group to authorize access to when operating on a user/group pair.
 * @param string $src_owner   Owner of security group to authorize access to when operating on a user/group pair.
 * @param string $protocol   IP protocol to authorize access to when operating on a CIDR IP.  "tcp",  "udp" and "icmp" allowed.
 * @param string $from_port   Bottom of port range to authorize access to when operating on a CIDR IP. This contains the ICMP type if ICMP is being authorized.
 * @param string $to_port   Top of port range to authorize access to when operating on a CIDR IP. This contains the ICMP code if ICMP is being authorized.
 * @param string $src_ip   CIDR IP range to authorize access to when operating on a CIDR IP.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_authorize_security_group_ingress($cloud_context, $name, $src_name, $src_owner, $protocol = NULL, $from_port = NULL, $to_port = NULL, $src_ip = NULL) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'AuthorizeSecurityGroupIngress',
    'GroupName' => $name,
    'Version' => $api_version,
  );

  if ( $src_name != NULL && empty($src_name) == FALSE ) {
    $request['SourceSecurityGroupName'] = $src_name;
  }

  if ( $src_owner != NULL && empty($src_owner) == FALSE ) {
    $request['SourceSecurityGroupOwnerId'] = $src_owner;
  }

  if ( $protocol != NULL && empty($protocol) == FALSE ) {
    $request['IpProtocol'] = $protocol;
  }


  if ( $from_port != NULL && strlen($from_port) > 0 ) {
    $request['FromPort'] = $from_port;
  }

  if ( $to_port != NULL && strlen($to_port) > 0 ) {
    $request['ToPort'] = $to_port;
  }

  if ( $src_ip != NULL && empty($src_ip) == FALSE ) {
    $request['CidrIp'] = $src_ip;
  }

  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * Revokes permissions from a security group. The permissions used to revoke must be specified using the same values used to grant the permissions.
 * @param string $name   Name of the group to edit.
 * @param string $src_name   Name of security group to revoke access to when operating on a user/group pair.
 * @param string $src_owner   Owner of security group to revoke access to when operating on a user/group pair.
 * @param string $protocol   IP protocol to revoke access to when operating on a CIDR IP.  "tcp",  "udp" and "icmp" allowed.
 * @param string $from_port   Bottom of port range to revoke access to when operating on a CIDR IP. This contains the ICMP type if ICMP is being authorized.
 * @param string $to_port   Top of port range to revoke access to when operating on a CIDR IP. This contains the ICMP code if ICMP is being authorized.
 * @param string $src_ip   CIDR IP range to revoke access to when operating on a CIDR IP.
 * @return object std class object containing response headers & raw body
 */
function aws_ec2_api_revoke_security_group_ingress($cloud_context, $name, $src_name, $src_owner, $protocol = NULL, $from_port = NULL, $to_port = NULL, $src_ip = NULL) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'RevokeSecurityGroupIngress',
    'GroupName' => $name,
    'IpProtocol' => $protocol,
    'FromPort' => $from_port,
    'ToPort' => $to_port,
    'Version' => $api_version,
  );

  if (empty($src_ip) ) {

    $request['SourceSecurityGroupName']      = $src_name;
    $request['SourceSecurityGroupOwnerId']   = $src_owner;
  }
  else {

    $request['CidrIp']   = $src_ip;
  }
  return aws_query_request($cloud_context, $host_uri, $request);
}


// Added
/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_describe_zones($cloud_context, $zone = array() ) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DescribeAvailabilityZones',
    'Version' => $api_version,
  );
  $request = array_merge($request,    aws_number_keys('ZoneName', $zone));
  return aws_query_request($cloud_context, $host_uri, $request);
}



/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_describe_address($cloud_context) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DescribeAddresses',
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}



/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_allocate_address($cloud_context) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'AllocateAddress',
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_release_address($cloud_context, $pub_ip) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'ReleaseAddress',
    'PublicIp' => $pub_ip,
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_associate_address($cloud_context, $instance_id, $pub_ip) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'AssociateAddress',
    'InstanceId' => $instance_id,
    'PublicIp' => $pub_ip,
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_disassociate_address($cloud_context, $pub_ip) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DisassociateAddress',
    'PublicIp' => $pub_ip,
    'Version' => $api_version,
  );

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => t('IP Address has been released: @pub_ip', array('@pub_ip' => $pub_ip)),
    'link' => '',
  )
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}




/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_describe_volumes($cloud_context) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DescribeVolumes',
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_get_console_output($cloud_context, $instance_id) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'GetConsoleOutput',
    'InstanceId' => $instance_id,
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_create_volume($cloud_context, $size, $zone, $snapshot_id = NULL) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'CreateVolume',
    'Size' => $size,
    'AvailabilityZone' => $zone,
    'Version' => $api_version,
  );

  if ( $snapshot_id != NULL
    && empty($snapshot_id) == FALSE
    && $snapshot_id != AWS_EC2_LIB_CLOUD_NONE) {
    $request['SnapshotId']   = $snapshot_id;
  }

  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_delete_volume($cloud_context, $vol_id) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DeleteVolume',
    'VolumeId' => $vol_id,
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_attach_volume($cloud_context, $vol_id, $instance_id, $device) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'AttachVolume',
    'VolumeId' => $vol_id,
    'InstanceId' => $instance_id,
    'Device' => $device,
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_detach_volume($cloud_context, $vol_id) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DetachVolume',
    'VolumeId' => $vol_id,
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_describe_snapshots($cloud_context) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DescribeSnapshots',
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_create_snapshot($cloud_context, $vol_id, $desc) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'CreateSnapshot',
    'VolumeId' => $vol_id,
    //'Description' => $desc ,
    'Version' => $api_version,
  );

  return aws_query_request($cloud_context, $host_uri, $request);
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function aws_ec2_api_delete_snapshot($cloud_context, $snapshot_id) {

  $host_uri    = cloud_get_info( $cloud_context, 'host_uri'    );
  $api_version = cloud_get_info( $cloud_context, 'api_version' );

  $request = array(
    'Action' => 'DeleteSnapshot',
    //'SnapshotId.1' => "$snapshot_id[1]",
    'Version' => $api_version,
  );

  $request = array_merge($request,    aws_number_keys('SnapshotId', $snapshot_id));
  return aws_query_request($cloud_context, $host_uri, $request);
}
