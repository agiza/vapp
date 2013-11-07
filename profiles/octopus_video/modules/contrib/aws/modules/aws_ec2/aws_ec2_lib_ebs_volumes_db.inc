<?php

/**
 * @file
 * DB functions Related to Volumes for Amazon EC2
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */


/**
 * Update the databse after the request to attach a volume to an instance was successfully completed
 *
 * @param $cloud_context
 *      The instance of sub-cloud on which attach volume operation was performed.
 * @param $myclient
 *      The response of the REST call
 * @return
 *      FALSE: If the reponse was empty
 *      TRUE:  If the database update was successfull.
 */
function _aws_ec2_lib_save_attach_volume($cloud_context, $myclient) {

  if ( empty($myclient) ) {
    return FALSE;
  }

  $ec2_lib_volumes_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );

  $obj_response = new SimpleXMLElement($myclient->data);

  if ( isset($obj_response->instanceId) == FALSE ) {
    return FALSE;
  }

  $num_updated = db_update($ec2_lib_volumes_table)
    ->fields(array(
    'instance_id' => $obj_response->instanceId,
    'device' => $obj_response->device,
    'status' => $obj_response->status,
    'attached' => $obj_response->attachTime,
  ))
    ->condition('volume_id', $obj_response->volumeId, '=')
    ->execute();

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => t('Volume has been modified: @volume_id', array('@volume_id' => $obj_response->volumeId)),
    'link' => '',
  )
  );

  return TRUE;
}

function _aws_ec2_lib_update_attach_volume($cloud_context) {

  $ec2_lib_volumes_table   = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );
  return db_update($ec2_lib_volumes_table);
}


function _aws_ec2_lib_get_volume_deviceoptions_query($cloud_context) {

  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_VOLUMES_DEVICE_OPTIONS_TABLE );
  return $query =  'SELECT c.deviceid, c.device FROM {' . $ec2_lib_volumes_device_opt_table . '} c where c.cloud_type = ' . $cloud_context;
}

function _aws_ec2_lib_get_volumes_by_instance_query($cloud_context) {

  $ec2_lib_volumes_table   = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );
  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE   );


  return $query =  'SELECT c.volume_id,  c.nickname,  d.instance_nickname,  c.device,  c.status,  c.created,  c.vol_size,  c.snapshot_id,  c.vol_zone,  c.attached FROM {' . $ec2_lib_volumes_table . '} c
  LEFT JOIN  {' . $ec2_lib_instances_table . '} d on c.instance_id=d.instance_id WHERE c.instance_id=:instance_id and c.cloud_type = :cloud_type order by c.nickname ';
}

/**
 *
 * @param  $cloud_context
 * @param  $instance_id
 * @return
 */
function _aws_ec2_lib_get_describe_volumes_query($cloud_context, $instance_id = '', $status = '') {
  $ec2_lib_volumes_table   = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE ) ;
  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE ) ;
  
  $search_where = " and c.cloud_type = '$cloud_context'";
  if ($instance_id) {
    $search_where .= " and c.vol_zone in (select placement from {" . $ec2_lib_instances_table . "} where instance_id='$instance_id' ) ";
  }

  return $query =  "SELECT c.volume_id,  c.nickname,  d.instance_nickname, c.instance_id, c.device,  c.status,  c.created,  c.vol_size,  c.snapshot_id,  c.vol_zone,  c.attached,  CAST(SUBSTRING_INDEX(c.vol_size,  ' ',  1) as UNSIGNED) as volume_size 
  FROM {" . $ec2_lib_volumes_table . "} c LEFT JOIN  {" . $ec2_lib_instances_table . "} d on c.instance_id=d.instance_id  
  WHERE status like '%%$status%%' " . $search_where;
}


/**
 *
 * @param  $cloud_context
 * @param  $instance_id
 * @return
 */
function _aws_ec2_lib_get_describe_volumes_list_query($cloud_context) {
  $ec2_lib_volumes_table   = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );
  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );

  $query = db_select($ec2_lib_volumes_table, 'c');
  $query
    ->leftjoin($ec2_lib_instances_table, 'd', 'c.instance_id = d.instance_id');

  $query->fields('c');
  $query->fields('d', array('instance_nickname') );

  return $query;
}


/**
 *
 * @param  $cloud_context
 * @param  $instance_id
 * @return
 */
function _aws_ec2_lib_get_volume($cloud_context, $instance_id = '') {
  $ec2_lib_volumes_table   = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );
  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );

  $search_where = " and c.cloud_type = :cloud_type ";
  if ($instance_id) {
    $search_where = " and c.vol_zone in (select placement from {" . $ec2_lib_instances_table . "} where instance_id='" . $instance_id . "')";
  }

  return $query =  "SELECT c.volume_id,  c.nickname,  d.instance_nickname,  c.device,  c.status,  c.created,  c.vol_size,  c.snapshot_id,  c.vol_zone,  c.attached,  CAST(SUBSTRING_INDEX(c.vol_size,  ' ',  1) as UNSIGNED) as volume_size FROM {" . $ec2_lib_volumes_table . "} c LEFT JOIN  {" . $ec2_lib_instances_table . "} d on c.instance_id=d.instance_id  WHERE nickname = :nickname " . $search_where;
}


/**
 *
 * @param  $cloud_context
 * @return
 */
function _aws_ec2_lib_get_volumes_query($cloud_context) {

  $ec2_lib_volumes_table   = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );
  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE   );

  return $query =  'SELECT c.volume_id,  c.nickname,  d.instance_nickname,  c.device,  c.status,  c.created,  c.vol_size,  c.snapshot_id,  c.vol_zone,  c.attached,  c.instance_id
  FROM {' . $ec2_lib_volumes_table . '} c LEFT JOIN  {' . $ec2_lib_instances_table . '} d
  on c.instance_id=d.instance_id WHERE volume_id= :volume_id and c.cloud_type = :cloud_type ';
}

/**
 *
 * @param  $cloud_context
 * @return
 */
function _aws_ec2_lib_get_volumes_list_count($cloud_context) {

  $ec2_lib_volumes_table   = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );
  $ec2_lib_instances_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE );

  $query = 'SELECT count(*) as item_count FROM {' . $ec2_lib_volumes_table . '} c
  LEFT JOIN  {' . $ec2_lib_instances_table . '} d on c.instance_id=d.instance_id where c.cloud_type=:cloud_type ';

  $args = array(
    ':cloud_type' => $cloud_context,
  );

  $result = db_query( $query, $args );

  foreach ($result as $item) {

    $item_count = $item->item_count;
  }

  return $item_count;
}

/**
 *
 * @param  $cloud_context
 * @return
 */
function _aws_ec2_lib_download_describe_volumes($cloud_context) {

  $ec2_lib_volumes_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );

  $myclient = aws_ec2_api_describe_volumes($cloud_context);
  if ( $myclient->code != 200  ) {
    return FALSE;
  }

  $obj_response = new SimpleXMLElement($myclient->data);
  $id_nick_arr  = cloud_get_all_nickname('volume_id', 'nickname', $ec2_lib_volumes_table );

  $num_deleted = db_delete($ec2_lib_volumes_table)
    ->condition('cloud_type', $cloud_context)
    ->execute();


  $insert_query = db_insert($ec2_lib_volumes_table)
    ->fields(array(
    'volume_id',
    'cloud_type',
    'nickname',
    'instance_id',
    'device',
    'status',
    'created',
    'vol_size',
    'snapshot_id',
    'vol_zone',
    'attached',
  )
   );

  $count = 0;

  foreach ($obj_response->volumeSet->item as $item) {

    $exists = db_query("select volume_id from {" . $ec2_lib_volumes_table . "} where volume_id = :volume_id ", array(':volume_id' => $item->volumeId))->fetchField();
    if ($exists == FALSE) {
      $attachment   = $item->attachmentSet->item;

      $tmp_nickname = cloud_check_in_array($id_nick_arr, $item->volumeId);
      $insert_query->values(
        array(
        'volume_id' => $item->volumeId,
        'cloud_type' => $cloud_context,
        'nickname' => $tmp_nickname != NULL ? $tmp_nickname : $item->volumeId,
        'instance_id' => $attachment->instanceId,
        'device' => $attachment->device,
        'status' => isset( $attachment->status) ? $item->status . '(' . $attachment->status . ')' : $item->status,
        'created' => $item->createTime,
        'vol_size' => $item->size . ' GB',
        'snapshot_id' => ( isset($item->snapshotId) &&  empty($item->snapshotId) == FALSE ) ? $item->snapshotId : '',
        'vol_zone' => $item->availabilityZone,
        'attached' => $attachment->attachTime,
      )
      );
      $count++;
    }
  }

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => 'Updated: Volume(s)',
    'link' => '',
  )
  );

  if ( $count == 0 ) {
    return TRUE;
  }

  $insert_query->execute();

  return TRUE;
}

/**
 *
 * @param  $cloud_context
 * @param  $myclient
 * @param  $nickname
 * @return
 */
function _aws_ec2_lib_save_new_volume($cloud_context, $myclient, $nickname ) {

  $ec2_lib_volumes_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );

  $obj_response = new SimpleXMLElement($myclient->data);
  $count = 0;

  $insert_query = db_insert($ec2_lib_volumes_table)
    ->fields(array(
    'volume_id',
    'cloud_type',
    'nickname',
    'status',
    'created',
    'vol_size',
    'vol_zone',
    'snapshot_id',
  )
   );

  $insert_query->values(
     array(
    'volume_id' => '' . $obj_response->volumeId,
    'cloud_type' => $cloud_context,
    'nickname' => $nickname,
    'status' => '' . $obj_response->status,
    'created' => '' . $obj_response->startTime,
    'vol_size' => '' . $obj_response->volumeSize,
    'vol_zone' => '' . $obj_response->availabilityZone,
    'snapshot_id' => '' . $obj_response->snapshotId,
  )
  );

  $insert_query->execute();

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => t('Volume has been added: @volume_id', array('@volume_id' => $obj_response->volumeId)),
    'link' => '',
  )
  );

  return TRUE;
}

/**
 * First a REST request to delete a volume is submitted and this function will
 * store the response of this request
 *
 * @param  $cloud_context
 *           The sub-cloud who's volume is to be deleted
 * @param  $result
 *           The REST call response
 * @param  $vol_id
 *           The Id of volume to be deleted
 * @return
 */
function _aws_ec2_lib_delete_volume($cloud_context, $result, $vol_id ) {

  if ( $result->code != 200 ) {
    return FALSE;
  }

  $ec2_lib_volumes_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );
  $obj_response = new SimpleXMLElement($result->data);
  if ( $obj_response->return == 'true' ) {

    $num_deleted = db_delete($ec2_lib_volumes_table)
      ->condition('volume_id', $vol_id)
      ->execute();

    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('Volume has been deleted: @volume_id', array('@volume_id' => $vol_id)),
      'link' => '',
    )
    );

    return TRUE;
  }

  return FALSE;
}

// End : DB functions Related to Volumes
