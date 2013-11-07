<?php

/**
 * @file
 * Constants for aws_ec2_lib_instances_db.inc
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */

// Start : DB functions Related to Instances


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_instance_names($cloud_context) {

  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );
  return $query =  'SELECT c.instance_nickname, c.instance_id FROM {' . $ec2_lib_instances_table . '} c  where cloud_type = :cloud_type ';
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_instance_query($cloud_context) {

  $ec2_lib_instances_table      = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );
  $ec2_lib_images_table         = aws_ec2_lib_get_table_info( AWS_EC2_LIB_IMAGES_TABLE );
  $ec2_lib_instances_lock_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_LOCK_TABLE );

  $query = 'SELECT c.instance_nickname, c.instance_id, d.image_nickname, c.instance_state_name, c.image_id, c.dns_name, c.private_dns_name, c.launch_time, c.key_name, c.group_id, c.placement, c.instance_type, c.owner_id, e.monitor FROM {' . $ec2_lib_instances_table . '} c
  LEFT JOIN  {' . $ec2_lib_images_table . '} d on c.image_id=d.image_id
  left join {' . $ec2_lib_instances_lock_table . '} e on c.instance_id=e.instance_id
  WHERE c.instance_id=:instance_id and c.cloud_type = :cloud_type ';

  return $query;
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_instances_list_query($cloud_context) {

  $ec2_lib_instances_table      = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );
  $ec2_lib_images_table         = aws_ec2_lib_get_table_info( AWS_EC2_LIB_IMAGES_TABLE );
  $ec2_lib_instances_lock_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_LOCK_TABLE );

  $query = db_select($ec2_lib_instances_table, 'c');
  $query->leftjoin($ec2_lib_images_table, 'd', 'c.image_id=d.image_id');
  $query->leftjoin($ec2_lib_instances_lock_table, 'e', 'c.instance_id=e.instance_id');
  $query->fields('c',
      array(
    'instance_nickname',
    'instance_id',
    'instance_state_name',
    'image_id',
    'dns_name',
    'private_dns_name',
    'launch_time',
    'key_name',
    'placement',
    'instance_state_name',
  ));
  $query->fields('d',
      array(
    'image_nickname',
  ));

  return $query;
}

/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_instances_list_count($cloud_context) {

  $ec2_lib_instances_table      = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );
  $ec2_lib_images_table         = aws_ec2_lib_get_table_info( AWS_EC2_LIB_IMAGES_TABLE );
  $ec2_lib_instances_lock_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_LOCK_TABLE );

  $query      = 'SELECT COUNT(*) as item_count FROM {' . $ec2_lib_instances_table . '} c LEFT JOIN  {' . $ec2_lib_images_table . '} d
  on c.image_id=d.image_id left join {' . $ec2_lib_instances_lock_table . '} e
  on c.instance_id=e.instance_id and c.cloud_type = :cloud_type ';

  $result     = db_query( $query, array(':cloud_type' => $cloud_context) );
  $item_count = 0;


  foreach ($result as $item) {

    $item_count = $item->item_count;
  }

  return $item_count;
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_instances_list_manual_sort_query($cloud_context) {

  $ec2_lib_instances_table      = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );
  $ec2_lib_images_table         = aws_ec2_lib_get_table_info( AWS_EC2_LIB_IMAGES_TABLE );
  $ec2_lib_instances_lock_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_LOCK_TABLE );


  $query = db_select($ec2_lib_instances_table, 'c');
  $query->leftjoin($ec2_lib_images_table, 'd', 'c.image_id=d.image_id');
  $query->leftjoin($ec2_lib_instances_lock_table, 'e', 'c.instance_id=e.instance_id');
  $query->fields('c',
      array(
    'instance_nickname',
    'instance_id',
    'instance_state_name',
    'image_id',
    'dns_name',
    'private_dns_name',
    'launch_time',
    'key_name',
    'placement',
    'instance_state_name',
  ));
  $query->fields('d',
      array(
    'image_nickname',
  ));

  return $query;
}



/**
 *
 * @global   $user
 * @param   $cloud_context
 * @param   $nickname
 * @param   $image_id
 * @param   $count
 * @param   $key_name
 * @param   $groups
 * @param   $data
 * @param   $instance_type
 * @param   $zone
 * @param   $kernel_id
 * @param   $ramdisk_id
 * @param   $params
 * @return
 */
function _aws_ec2_lib_launch_new_instance($cloud_context, $nickname, $image_id, $count, $key_name, $groups, $data, $instance_type, $zone, $kernel_id, $ramdisk_id, $params = array(), $user_name) {

  // Will launch instance here:
  // aws_ec2_api_run_instances($image_id, $min, $max, $key_name = NULL, $groups = array(), $data = NULL, $type = NULL, $zone = NULL, $kernel_id  = NULL, $ramdisk_id  = NULL) {

  global $user;
  $uname = $user->name;

  $myclient = aws_ec2_api_run_instances(

    $cloud_context,
    $image_id      , // $image_id
    $count         , // $min
    $count         , // $max
    $key_name      , // $key_name
    $groups        , // $groups
    $data          , // $data
    $instance_type, // $instance_type
    $zone          , // $zone
    $kernel_id     , // $kernel_id
    $ramdisk_id    , // $ramdisk_id
    $params
  );

  if ( $myclient->code == 200 ) {

    $curr_date =  date('Ymd-His');
    $instance_name = $uname . '-' . $nickname . '-' . $curr_date;

    _aws_ec2_lib_save_launch_new_instance($cloud_context, $myclient, $instance_name, $user_name );
    drupal_set_message(t('Initiated request for a new instance. This may take some time. Use Refresh to update the status.'));

    return TRUE;
  }
  else {
    $startpos = strpos($myclient->data, '<Message>') + strlen('<Message>');
    $endpos   = strpos($myclient->data, '</Message>');
    $msg      = substr($myclient->data, $startpos, $endpos -$startpos);
    drupal_set_message(t('There was a problem while launching the instance. @message', array('@message' => $msg)), 'error');

    return FALSE;
  }
}


/**
 *
 * @param   $cloud_context
 * @param   $myclient
 * @param   $nickname
 * @return
 */
function _aws_ec2_lib_save_launch_new_instance($cloud_context, $myclient, $nickname, $ssh_user_name) {

  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );

  $obj_response = new SimpleXMLElement($myclient->data);
  $count = 0;

  $insert_query = db_insert($ec2_lib_instances_table)
      ->fields(
        array(
    'instance_id',
    'cloud_type',
    'instance_nickname',
    'image_id',
    'instance_state_code',
    'instance_state_name',
    'private_dns_name',
    'dns_name',
    'reason',
    'key_name',
    'ami_launch_index',
    'product_codes',
    'instance_type',
    'launch_time',
    'placement',
    'kernel_id',
    'ramdisk_id',
    'platform',
    'monitoring',
    'owner_id',
    'group_id',
    'reservation_id',
  )
     );


  $reservation_id = $obj_response->reservationId;
  $owner_id       = $obj_response->ownerId;

  $cnt = count($obj_response->instancesSet->item );

  $instances_id_arr = array();
  $instance_values = array();

  foreach ($obj_response->instancesSet->item as $instance ) {

    $count++;



    if ( $cnt > 1 ) {
      $tmp_nickname = $nickname . ' #' . $count;
    }
    else {
      $tmp_nickname = $nickname;
    }

    $instances_id_arr[] = $instance->instanceId;

    $instance_values[] =
            array(
      'instance_id' => $instance->instanceId,
      'cloud_type' => $cloud_context,
      'instance_nickname' => $tmp_nickname,
      'image_id' => $instance->imageId,
      'instance_state_code' => $instance->instanceState->code,
      'instance_state_name' => $instance->instanceState->name,
      'private_dns_name' => $instance->privateDnsName,
      'dns_name' => $instance->dnsName,
      'reason' => isset( $instance->reason ) ? $instance->reason  : '',
      'key_name' => $instance->keyName,
      'ami_launch_index' => $instance->amiLaunchIndex,
      'product_codes' => isset( $instance->productCodes ) ? $instance->productCodes  : '',
      'instance_type' => $instance->instanceType,
      'launch_time' => $instance->launchTime,
      'placement' => isset( $instance->placement ) ? $instance->placement  : '',
      'kernel_id' => isset( $instance->kernel_id ) ? $instance->kernel_id  : '',
      'ramdisk_id' => isset( $instance->ramdisk_id ) ? $instance->ramdisk_id : '',
      'platform' => isset( $instance->platform ) ? $instance->platform : '',
      'monitoring' => isset( $instance->monitoring ) ? isset( $instance->monitoring->state ) ?  $instance->monitoring->state : ''  : '',
      'owner_id' => $owner_id,
      'group_id' => '',
      'reservation_id' => $reservation_id,
    );

    $insert_user_name_query = _aws_ec2_lib_set_ssh_user_query($cloud_context);
    $insert_user_name_query
      ->values(
        array(
      'instance_id' => $instance->instanceId,
      'cloud_type' => $cloud_context,
      'is_locked' => 'Unlock',
      'ssh_user_name' => $ssh_user_name,
    ))
      ->execute();

  }

  $insert_query->values($instance_values);

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => t('Instances have been launched: @instances', array('@instances' => implode(',', $instances_id_arr))),
    'link' => '',
  )
  );

  $_REQUEST['new_instance_ids'] = $instances_id_arr;

  if ( $count == 0 ) {
    return;
  }

  $insert_query->execute();


  //check wheather billing module is enabled or not
  if (module_exists('cloud_billing')) {

    $detail_insert_query = db_insert(CLOUD_BILLING_INSTANCES_DETAILS_TABLE)
      ->fields(
        array(
      'instance_id',
      'cloud_type',
      'instance_nickname',
      'image_id',
      'instance_state_code',
      'instance_state_name',
      'private_dns_name',
      'dns_name',
      'reason',
      'key_name',
      'ami_launch_index',
      'product_codes',
      'instance_type',
      'launch_time',
      'placement',
      'kernel_id',
      'ramdisk_id',
      'platform',
      'monitoring',
      'owner_id',
      'group_id',
      'reservation_id',
    )
     );

    $detail_insert_query->values($instance_values)->execute();
  }
  //////

  return;
}


/**
 *
 * @param   $cloud_context
 * @param   $table
 * @return
 */
function _aws_ec2_lib_get_all_dns($cloud_context, $table) {


  $db_result = db_query( 'select dns_name from {' . $table . '} where cloud_type = :cloud_type ', array(':cloud_type' => $cloud_context));
  $db_dns = array();

  foreach ($db_result as $dns) {
    $db_dns[] = $dns->dns_name;
  }
  return $db_dns;
}



/**
 * This function download's instance related information for a particular sub-cloud
 * REST request is sent to the sub-cloud
 * The reponse is parsed.
 * Existing names of instances are maintained and new data is inserted in the instance table for the sub-cloud
 * Billing information is updated.
 *
 * @param $cloud_context
 * @return
 *      TRUE  In case the download of instances information was successfully completed.
 *      FALSE In case there of an error return.
 */
function _aws_ec2_lib_download_describe_instances($cloud_context) {

  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );

  $myclient = aws_ec2_api_describe_instances($cloud_context);
  if ( $myclient->code != 200  ) {
    return FALSE;
  }

  $obj_response = new SimpleXMLElement($myclient->data);

  $id_nick_arr = cloud_get_all_nickname('instance_id', 'instance_nickname', $ec2_lib_instances_table );

  $already_existing_dns = _aws_ec2_lib_get_all_dns($cloud_context, $ec2_lib_instances_table);
  $dns_from_cloud = array();

  $collectd_snmp_host_entries = '';

  $num_deleted = db_delete($ec2_lib_instances_table)
      ->condition('cloud_type', $cloud_context)
      ->execute();

  $insert_query = db_insert($ec2_lib_instances_table)
      ->fields(
        array(
    'instance_id'         ,
    'cloud_type'          ,
    'instance_nickname'   ,
    'image_id'            ,
    'instance_state_code',
    'instance_state_name',
    'private_dns_name'    ,
    'dns_name'            ,
    'reason'              ,
    'key_name'            ,
    'ami_launch_index'    ,
    'product_codes'       ,
    'instance_type'       ,
    'launch_time'         ,
    'placement'           ,
    'kernel_id'           ,
    'ramdisk_id'          ,
    'platform'            ,
    'monitoring'          ,
    'owner_id'            ,
    'group_id'            ,
    'reservation_id'      ,
  ));

  $count = 0;

  foreach ($obj_response->reservationSet->item as $item) {

    foreach ($item->instancesSet->item as $instance ) {

      $exists = db_query("select instance_id from {" . $ec2_lib_instances_table . "} where instance_id = :instance_id ", array(':instance_id' => $instance->instanceId))->fetchField();

      if ($exists == FALSE) {
        $dns_from_cloud[] = $instance->dnsName;


        $tmp_nickname = cloud_check_in_array($id_nick_arr, $instance->instanceId);
        $keyname = $instance->keyName;

        $sg_str = '';
        foreach ($item->groupSet->item as $grp_item) {

          $sg_str = $sg_str . $grp_item->groupId . ',';
        }

        $sg_str = substr( $sg_str, 0, -1 );

        $base_cloud = cloud_get_base_cloud($cloud_context);
        switch ($base_cloud) { // Needed for OpenStack
          case AWS_EC2_LIB_OPENSTACK_NOVA_CONTEXT:

            if ($pos = strripos( $keyname, '(' )) {
              $keyname = substr( $keyname, 0, $pos - 1 );
            }
            break;
        }

        $insert_query
          ->values(
            array(
          'instance_id' => $instance->instanceId,
          'cloud_type' => $cloud_context,
          'instance_nickname' => $tmp_nickname != NULL ? $tmp_nickname : $instance->instanceId,
          'image_id' => $instance->imageId,
          'instance_state_code' => $instance->instanceState->code,
          'instance_state_name' => $instance->instanceState->name,
          'private_dns_name' => $instance->privateDnsName,
          'dns_name' => $instance->dnsName,
          'reason' => isset( $instance->reason ) ? $instance->reason  : '',
          'key_name' => $keyname,
          'ami_launch_index' => $instance->amiLaunchIndex,
          'product_codes' => isset( $instance->productCodes ) ? $instance->productCodes  : '',
          'instance_type' => $instance->instanceType,
          'launch_time' => $instance->launchTime,
          'placement' => isset( $instance->placement  ) ? $instance->placement->availabilityZone : '',
          'kernel_id' => isset( $instance->kernelId   ) ? $instance->kernelId  : '',
          'ramdisk_id' => isset( $instance->ramdiskId  ) ? $instance->ramdiskId : '',
          'platform' => isset( $instance->platform   ) ? $instance->platform  : '',
          'monitoring' => isset( $instance->monitoring ) ? isset( $instance->monitoring->state ) ? $instance->monitoring->state : ''  : '',
          'owner_id' => $item->ownerId,
          'group_id' => $sg_str,
          'reservation_id' => $item->reservationId,
        ));

        $count++;

        //check wheather billing module is enabled or not
        if (module_exists('cloud_billing')) {
          cloud_billing_update($cloud_context                ,
                             $instance->instanceId         ,
                             $instance->instanceState->name,
                             $instance->launchTime         ,
                             $instance->instanceType
                           );
        }
      }
      //end extra check here
    }
  }

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => 'Updated: Instances',
    'link' => '',
  )
  );

  if ( $count == 0 ) {
    return TRUE;
  }

  $insert_query->execute();

  /*
   * Code for periodically cleaning the junk Host Entries in Collectd.conf
   */
  $snmp_last_update_diff_in_min = cloud_get_last_update_snmp_timediff($cloud_context);
  cloud_log_to_db('Snmp Time Difference', $snmp_last_update_diff_in_min);

  $collectd_snmp_host_entries = aws_ec2_lib_get_latest_snmp_entries($cloud_context);
  cloud_log_to_db('Snmp Host Entries', $collectd_snmp_host_entries);

  if (!empty($collectd_snmp_host_entries)
    && $snmp_last_update_diff_in_min > 0) {

    cloud_log_to_db('Snmp Host Entries', $collectd_snmp_host_entries);

    cloud_snmp_update_entry($cloud_context, 'all', $collectd_snmp_host_entries);
    cloud_log_to_db('SNMP_HOST_ENTRIES', $collectd_snmp_host_entries);

    cloud_update_host_entries_last_update_time($cloud_context);
    cloud_log_to_db('Hurray Db Updated', 'Snmp Entries Updated');
  }
  else {

    //Code for checking new entry. This will add all the host entries without cleaning old entries.
    $new_instances = array_diff($dns_from_cloud, $already_existing_dns);
    foreach ($new_instances as $new_dns) {
      cloud_snmp_update_entry($cloud_context, 'add', $new_dns);
    }
  }

  $new_instance_flag = FALSE;
  $new_instances     = array_diff($dns_from_cloud, $already_existing_dns);

  return TRUE;
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function aws_ec2_lib_get_latest_snmp_entries($cloud_context) {

  $ec2_lib_instances_table      = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );
  $ec2_lib_instances_lock_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_LOCK_TABLE );

  $query = 'SELECT inst.dns_name as dns_name FROM {' . $ec2_lib_instances_table . "} inst
              LEFT JOIN {" . $ec2_lib_instances_lock_table . "} moni
              on inst.instance_id =  moni.instance_id
              where moni.monitor=1 and inst.instance_state_name='running' and inst.cloud_type = :cloud_type ;
            "; // Added to select all the instances which are to be monitored apart from the array.

  $dns_names  = '';

  $db_result = db_query( $query, array(':cloud_type' => $cloud_context) );

  foreach ($db_result as $row) {
    $dns_names .=  $row->dns_name . ", ";
  }

  $dns_names  = substr($dns_names, 0, -1);
  return $dns_names;
}


/**
 * Update the instance nickname
 */
function aws_ec2_lib_update_instance_nickname($instance_id, $cloud_context, $nickname) {

  $query = _aws_ec2_lib_update_instance_query($cloud_context, $instance_id, $nickname);
}

/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_update_instance_query($cloud_context, $instance_id, $nickname) {

  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );


  $num_updated = db_update($ec2_lib_instances_table)
    ->fields(array(
    'instance_nickname' => $nickname,
  ))
    ->condition('instance_id', $instance_id, '=')
    ->condition('cloud_type', $cloud_context, '=')
    ->execute();

  return $num_updated;
}


/**
 *
 * @param   $cloud_context
 * @param   $filter
 * @return
 */
function _aws_ec2_lib_get_all_instances_db($cloud_context, $filter = array() ) {

  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );

  $where_sql = '';

  $query_args = array();

  // Check if Filter is present
  if ( isset($filter['filter_value'] ) && !empty($filter['filter_value'])  ) {

    $column       = $filter['column'];
    $where_sql    = " and $column like :filter_value ";
    $query_args[':filter_value'] = '%' . $filter['filter_value'] . '%';
  }

  if ( isset($filter['filter_state'] ) && !empty($filter['filter_state'])  ) {

    if ( $filter['filter_state'] === 'all' ) { // Get all instances
      $query_args[':cloud_type'] = $cloud_context;

      $db_result = db_query( 'select * from {' . $ec2_lib_instances_table . "} where instance_state_name like '%%' " . $where_sql . " and cloud_type = :cloud_type ", $query_args );
    }
    else {
      $query_args[':cloud_type'] = $cloud_context;

      $db_result = db_query( 'select * from {' . $ec2_lib_instances_table . "} where instance_state_name like '%" . $filter['filter_state'] . "%' " . $where_sql . " and cloud_type = :cloud_type ", $query_args );
    }
  }
  else {
    $query_args[':cloud_type'] = $cloud_context;

    $db_result = db_query( 'select * from {' . $ec2_lib_instances_table . "} where instance_state_name = 'running' " . $where_sql . " and cloud_type = :cloud_type ", $query_args );
  }



  $instances_list = array();

  foreach ($db_result as $instance) {

    $instances_list[$instance->instance_id] = array();
    $instances_list[$instance->instance_id]['instance_id'  ] = $instance->instance_id;
    $instances_list[$instance->instance_id]['name'         ] = $instance->instance_nickname;
    $instances_list[$instance->instance_id]['hostname'     ] = $instance->dns_name    ;
    $instances_list[$instance->instance_id]['state'        ] = $instance->instance_state_name;
    $instances_list[$instance->instance_id]['zone'         ] = $instance->placement   ;
    $instances_list[$instance->instance_id]['user'         ] = $instance->key_name    ;
    $instances_list[$instance->instance_id]['runtime'      ] = $instance->launch_time;
    $instances_list[$instance->instance_id]['key_name'     ] = $instance->key_name    ;
    $instances_list[$instance->instance_id]['dns_name'     ] = $instance->dns_name    ;
    $instances_list[$instance->instance_id]['instance_type'] = $instance->instance_type;
    $instances_list[$instance->instance_id]['cloud_context'] = $cloud_context         ;
    $instances_list[$instance->instance_id]['lock_status'  ] = _aws_ec2_lib_get_lock_status_by_id_query( $cloud_context, $instance->instance_id );
  }

  return $instances_list;
}



/**
 *
 * @param   $cloud_context
 * @param   $filter
 * @return
 */
function _aws_ec2_lib_get_all_volumes_db($cloud_context, $instance_id ) {

  $ec2_lib_volumes_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );
  $query_args = array(
    ':cloud_type' => $cloud_context,
    ':instance_id' => $instance_id,
  );


  $db_result = db_query( 'select * from {' . $ec2_lib_volumes_table . "} where cloud_type = :cloud_type and instance_id = :instance_id", $query_args );

  $volumes_list = array();

  foreach ($db_result as $volume) {

    $volumes_list[$volume->volume_id] = array();
    $volumes_list[$volume->volume_id]['instance_id'] = $volume->instance_id;
    $volumes_list[$volume->volume_id]['name'] = $volume->nickname;
    $volumes_list[$volume->volume_id]['cloud_type'] = $volume->cloud_type;
    $volumes_list[$volume->volume_id]['snapshot_id'] = $volume->snapshot_id;
    $volumes_list[$volume->volume_id]['size'] = $volume->vol_size;
    $volumes_list[$volume->volume_id]['zone'] = $volume->vol_zone;
    $volumes_list[$volume->volume_id]['device'] = $volume->device;
    $volumes_list[$volume->volume_id]['status'] = $volume->status;
    $volumes_list[$volume->volume_id]['created'] = $volume->created;
    $volumes_list[$volume->volume_id]['attached'] = $volume->attached;
  }

  return $volumes_list;
}


/**
 *
 * @param   $cloud_context
 * @param   $myclient @
 */
function aws_ec2_lib_save_terminate_instance($cloud_context, $myclient, $instance_id) {

  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );

  $obj_response = new SimpleXMLElement($myclient->data);

  $base_cloud = cloud_get_base_cloud($cloud_context);
  switch ($base_cloud) { // Needed for OpenStack
    case AWS_EC2_LIB_OPENSTACK_NOVA_CONTEXT:
      if ( strcasecmp( $obj_response->return, 'true') == 0 ) {

        $new_state           = 'terminating';

        $num_updated = db_update($ec2_lib_instances_table)
          ->fields(array(
          'instance_state_name' => $new_state,
        ))
          ->condition('instance_id', $instance_id, '=')
          ->condition('cloud_type', $cloud_context, '=')
          ->execute();
      }
      break;

    default:
      if (isset($obj_response->instancesSet) ) {

        foreach ($obj_response->instancesSet->item as $instance ) {

          $instance_id         = $instance->instanceId;
          $new_state           = $instance->currentState->name;

          $num_updated = db_update($ec2_lib_instances_table)
            ->fields(array(
            'instance_state_name' => $new_state,
          ))
            ->condition('instance_id', $instance_id, '=')
            ->condition('cloud_type', $cloud_context, '=')
            ->execute();

          cloud_billing_update($cloud_context);
        }
      }
  }

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => t('Instance has been terminated: @instance_id', array('@instance_id' => $instance_id)),
    'link' => '',
  )
  );
}

// End : DB functions Related to Instances


/**
 *
 * @param   $cloud_context
 * @return
 */
function aws_ec2_lib_get_instances_lock($cloud_context) {

  $instance_lock_table  = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_LOCK_TABLE );
  $lock_query = "select instance_id, is_locked, monitor, '$cloud' as $cloud_context from {$instance_lock_table} where cloud_type = :cloud_type ";

  $result_status = array();


  $result = db_query( $lock_query, array(':cloud_type' => $cloud_context) );


  foreach ($result as $key) {

    $result_status[$key->instance_id] = $key->is_locked;
    $result_status[$key->instance_id . '_monitor'] = $key->monitor;
  }

  return $result_status;
}


/**
 *
 * @param   $instance_ids
 * @param   $template_id
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_set_scripts_to_exec($instance_ids, $template_id, $cloud_context) {

  if (!module_exists('cloud_scripting')) {

    return;
  }

  $query_scr = _cloud_scripting_get_scripts_of_template();
  $query_args_scr = array(
    ':server_template_id' => $template_id,
  );


  $result_scr = db_query( $query_scr, $query_args_scr );
  $scripts_arr = array();


  foreach ($result_scr as $scripts) {

    $scripts_arr[$scripts->script_id] = $scripts->script_id;
  }

  foreach ($instance_ids as $key => $instance_id) {

    foreach ($scripts_arr as $scr_key => $script_id) {

      _cloud_scripting_register_script_save($instance_id, $script_id,    '0', $cloud_context, $template_id );
    }

    if (sizeof($scripts_arr) == 0 ) { // No Script to Execute
      $result_script_status = aws_ec2_lib_instance_status_update($cloud_context, $instance_id, CLOUD_INSTANCE_STATUS_OPERATIONAL );
    }
    else { // Scripts to be executed
      $result_script_status = aws_ec2_lib_instance_status_update($cloud_context, $instance_id, CLOUD_INSTANCE_STATUS_BOOTING );
    }
  }
}

/**
 *
 * @param   $template_id
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_server_template_details_by_id($template_id = '', $cloud_context = '') {

  $ec2_lib_images_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_IMAGES_TABLE );

  return $query = 'SELECT c.image_id, c.kernel_id, c.ramdisk_id, c.*,d.image_nickname FROM {' . CLOUD_SERVER_TEMPLATES_TABLE . '} c
  left join {' . $ec2_lib_images_table . '} d on c.image_id=d.image_id
  WHERE template_id=\'' . $template_id . '\' and c.cloud_type = :cloud_type ';
}


function _aws_ec2_lib_get_instance_details_by_id($cloud_context, $instance_id) {

  $query_args = array();
  $query = _aws_ec2_lib_get_instance_query($cloud_context);
  $query_args = array(
    ':instance_id' => $instance_id,
    ':cloud_type' => $cloud_context,
  );


  $result = db_query( $query, $query_args );
  $count = 0;

  $instance_info  = array();

  foreach ($result as $instance) {

    $instance_info['instance_id'  ] = $instance->instance_id;
    $instance_info['name'         ] = $instance->instance_nickname;
    $instance_info['hostname'     ] = $instance->dns_name    ;
    $instance_info['state'        ] = $instance->instance_state_name;
    $instance_info['zone'         ] = $instance->placement   ;
    $instance_info['user'         ] = $instance->key_name    ;
    $instance_info['runtime'      ] = $instance->launch_time;
    $instance_info['key_name'     ] = $instance->key_name    ;
    $instance_info['dns_name'     ] = $instance->dns_name    ;
    $instance_info['image_nickname'] = $instance->image_nickname;
    $instance_info['cloud_context'] = $cloud_context         ;
    $instance_info['lock_status'  ] = _aws_ec2_lib_get_lock_status_by_id_query( $cloud_context, $instance->instance_id );
  }

  return $instance_info;
}
