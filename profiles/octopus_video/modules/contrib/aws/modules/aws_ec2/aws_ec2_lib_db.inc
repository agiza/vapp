<?php

/**
 * @file
 * Database handling for aws_ec2_lib
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 */


module_load_include('inc', 'aws_ec2_lib', 'aws_ec2_lib_ebs_volumes_db'    );
module_load_include('inc', 'aws_ec2_lib', 'aws_ec2_lib_elastic_ips_db'    );
module_load_include('inc', 'aws_ec2_lib', 'aws_ec2_lib_images_db'         );
module_load_include('inc', 'aws_ec2_lib', 'aws_ec2_lib_instances_db'      );
module_load_include('inc', 'aws_ec2_lib', 'aws_ec2_lib_instances_lock_db' );
module_load_include('inc', 'aws_ec2_lib', 'aws_ec2_lib_register_image_db' );
module_load_include('inc', 'aws_ec2_lib', 'aws_ec2_lib_security_groups_db');
module_load_include('inc', 'aws_ec2_lib', 'aws_ec2_lib_snapshots_db'      );
module_load_include('inc', 'aws_ec2_lib', 'aws_ec2_lib_ssh_keys_db'       );
module_load_include('inc', 'aws_ec2_lib', 'aws_ec2_lib_user_keys_db'      );


/**
 * Retrieve and store information from the server about the sub-cloud.
 *
 * This function will download the avaliable information for the following.
 *     viz. Elastic IP Info,
 *          Instances Info,
 *          SSH Keys Info,
 *          Security Group Info,
 *          Zone Info,
 *          Snapshot Info,
 *          Volume Info
 *
 * In case the expected paramters are not set there will be a errror message displayed asking user to first set the variables.
 *
 * @param $cloud_context
 *         This is the sub-cloud who's information is to be downloaded
 * @return
 *         The information is downloded using REST calls and is stored in the database in the respective cloud_context tables.
 */
function _aws_ec2_lib_download_all($cloud_context) {

  $host_uri   = cloud_get_info( $cloud_context, 'host_uri'       );
  $access_key = cloud_get_info( $cloud_context, 'aws_access_key' );
  $sec_key    = cloud_get_info( $cloud_context, 'aws_secret_key' );

  // Checking if all ec2 variables are set
  if ( empty( $host_uri   )
    ||     empty( $access_key )
    ||     empty( $sec_key    ) ) {

    $cloud_list = cloud_get_all_clouds_display_name();
    cloud_set_config_error_message($cloud_list[$cloud_context], $cloud_context);
    return FALSE;
  }

  if ( strpos( $host_uri, 'http://' ) !== FALSE) {

    $cloud_list = cloud_get_all_clouds_display_name();
    $admin_url = cloud_get_admin_url($cloud_list[$cloud_context], $cloud_context);
    drupal_set_message(check_plain(t("'http://' is not required in AWS API. Please remove 'http://' from API URI:")) . $admin_url, 'error');

    return FALSE;
  }

  $resp = _aws_ec2_lib_download_describe_elastic_ips($cloud_context);

  if ( $resp == FALSE ) {

    $cloud_list = cloud_get_all_clouds_display_name();
    cloud_set_config_error_message($cloud_list[$cloud_context], $cloud_context);

    return FALSE;
  }

  $resp = _aws_ec2_lib_download_describe_instances($cloud_context);
  if ( $resp == FALSE ) {
    return FALSE;
  }

  $resp = _aws_ec2_lib_download_describe_ssh_keys($cloud_context);
  if ( $resp == FALSE ) {
    return FALSE;
  }

  $resp = _aws_ec2_lib_download_describe_security_groups($cloud_context);
  if ( $resp == FALSE ) {
    return FALSE;
  }

  $resp = _aws_ec2_lib_download_describe_zones($cloud_context);
  if ( $resp == FALSE ) {
    return FALSE;
  }

  $resp = _aws_ec2_lib_download_describe_snapshots($cloud_context);
  if ( $resp == FALSE ) {
    return FALSE;
  }

  $resp = _aws_ec2_lib_download_describe_volumes($cloud_context);
  if ( $resp == FALSE ) {
    return FALSE;
  }


  return TRUE;
}


/**
 * Retrieve the images information and store it in the database of the respective sub-cloud(cloud_context)
 *
 * @param $cloud_context
 *        This is the sub-cloud who's information is to be downloaded
 * @return
 *        The downloaded image information is stored in the database tables of the respective sub-cloud
 */
function _aws_ec2_lib_download_all_images($cloud_context) {

  $resp = _aws_ec2_lib_download_describe_images($cloud_context);
  if ( $resp == FALSE ) {
    return;
  }
}

// Start : DB functions Related to Instance Type

// Start : DB functions Related to Availability Zone

/**
 * Download Zone related information
 * @param $cloud_context
 * @return
 */
function _aws_ec2_lib_get_describe_zones_query($cloud_context) {

  $ec2_lib_zone_table  = aws_ec2_lib_get_table_info( AWS_EC2_LIB_ZONE_TABLE );
  return $query =   'SELECT c.zone_name,  c.zone_state, c.region FROM {' . $ec2_lib_zone_table . '} c  where c.cloud_type = :cloud_type ';
}


function _aws_ec2_lib_download_describe_zones($cloud_context) {

  $ec2_lib_zone_table  = aws_ec2_lib_get_table_info( AWS_EC2_LIB_ZONE_TABLE );
  $myclient = aws_ec2_api_describe_zones($cloud_context);
  if ( $myclient->code != 200  ) {
    return FALSE;
  }

  $obj_response = new SimpleXMLElement($myclient->data);

  db_delete($ec2_lib_zone_table)->condition('cloud_type', $cloud_context)->execute();
  $insert_query = db_insert('{' . $ec2_lib_zone_table . '}')
    ->fields(array('zone_name', 'zone_state', 'cloud_type', 'region'));


  $count = 0;
  foreach ($obj_response->availabilityZoneInfo->item as $item) {

    $insert_query->values(
      array(
      'zone_name' => $item->zoneName,
      'zone_state' => $item->zoneState,
      'cloud_type' => $cloud_context,
      'region' => isset( $item->regionName ) ? $item->regionName : '',
    )
    );

    $count++;
  }

  $insert_query->execute();

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => 'Updated: Availability Zone(s)',
    'link' => '',
  )
  );

  if ( $count == 0 ) {
    return TRUE;
  }

  return TRUE;
}


// End : DB functions Related to Availability Zone

/**
 * Remove all rows for a particular cloud_context
 */
function aws_ec2_lib_delete_all($cloud_context) {

  db_delete(aws_ec2_lib_get_table_info(AWS_EC2_LIB_INSTANCES_TABLE))->condition('cloud_type', $cloud_context)->execute();
  db_delete(aws_ec2_lib_get_table_info(AWS_EC2_LIB_INSTANCES_LOCK_TABLE))->condition('cloud_type', $cloud_context)->execute();
  db_delete(aws_ec2_lib_get_table_info(AWS_EC2_LIB_IMAGES_TABLE))->condition('cloud_type', $cloud_context)->execute();
  db_delete(aws_ec2_lib_get_table_info(AWS_EC2_LIB_SSH_KEYS_TABLE))->condition('cloud_type', $cloud_context)->execute();
  db_delete(aws_ec2_lib_get_table_info(AWS_EC2_LIB_SECURITY_GROUPS_TABLE))->condition('cloud_type', $cloud_context)->execute();
  db_delete(aws_ec2_lib_get_table_info(AWS_EC2_LIB_SECURITY_GROUPS_DETAILS_TABLE))->condition('cloud_type', $cloud_context)->execute();
  db_delete(aws_ec2_lib_get_table_info(AWS_EC2_LIB_ZONE_TABLE))->condition('cloud_type', $cloud_context)->execute();
  db_delete(aws_ec2_lib_get_table_info(AWS_EC2_LIB_ELASTIC_IP_TABLE))->condition('cloud_type', $cloud_context)->execute();
  db_delete(aws_ec2_lib_get_table_info(AWS_EC2_LIB_EBS_VOLUMES_TABLE))->condition('cloud_type', $cloud_context)->execute();
  db_delete(aws_ec2_lib_get_table_info(AWS_EC2_LIB_USER_KEYS_TABLE))->condition('cloud_type', $cloud_context)->execute();
  db_delete(aws_ec2_lib_get_table_info(AWS_EC2_LIB_REGISTER_IMAGE_STATUS_TABLE))->condition('cloud_type', $cloud_context)->execute();
  db_delete(aws_ec2_lib_get_table_info(AWS_EC2_LIB_SNAPSHOTS_TABLE))->condition('cloud_type', $cloud_context)->execute();
}


// Start : DB functions Related to Bundle Instance

/** NOTE: no need to change these two **/

function aws_ec2_lib_db_bundle_instance($cloud_context, $instance_id, $nickname, $bucketname, $foldername, $volume_size, $status) {

  $aws_ec2_lib_bundle_instances_table  = AWS_EC2_LIB_BUNDLE_INSTANCES_TABLE;

  $insert_query = db_insert($aws_ec2_lib_bundle_instances_table)
    ->fields(array('cloud_context', 'instance_id', 'nickname', 'bucketname', 'foldername', 'volume_size', 'status'));

  $insert_query->values(
    array(
    'cloud_context' => $cloud_context,
    'instance_id' => $instance_id,
    'nickname' => $nickname,
    'bucketname' => $bucketname,
    'foldername' => $foldername,
    'volume_size' => $volume_size,
    'status' => $status,
  )
  );

  $insert_query->execute();

  return TRUE;
}

function aws_ec2_lib_db_bundle_instance_get($status) {

  $aws_ec2_lib_bundle_instances_table  = AWS_EC2_LIB_BUNDLE_INSTANCES_TABLE;

  $script_count = 0;
  $return_result = array();
  $result = db_select($aws_ec2_lib_bundle_instances_table, 'c')
    ->fields('c')
    ->condition('status', $status, '=')
    ->execute();

  foreach ($result as $details) {

    $values = array();
    foreach ($details as $key => $val) {
      $values[$key] = $val;
    }

    $return_result[] = $values;
  }

  return $return_result;
}


function aws_ec2_lib_db_bundle_instance_update_status($instance_id, $status) {

  $aws_ec2_lib_bundle_instances_table  = AWS_EC2_LIB_BUNDLE_INSTANCES_TABLE;

  $num_updated = db_update($aws_ec2_lib_bundle_instances_table)
    ->fields(array(
    'status' => $status,
  ))
    ->condition('instance_id', $instance_id, '=')
    ->execute();

  return $num_updated;
}

// End : DB functions Related to Bundle Instance

