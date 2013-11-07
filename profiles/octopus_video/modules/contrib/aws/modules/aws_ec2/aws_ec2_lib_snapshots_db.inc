<?php

/**
 * @file
 * DB functions Related to Snapshot for Amazon EC2
 *
 * Copyright (c) 2010-2012 DOCOMO Innovations, Inc.
 *
 */

/**
 *
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_get_describe_snapshots_query($cloud_context) {

  $ec2_lib_snapshot_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SNAPSHOTS_TABLE );
  $ec2_lib_ebs_volumes_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );

  $query = db_select($ec2_lib_snapshot_table, 'c');
  $query
    ->leftjoin($ec2_lib_ebs_volumes_table, 'vol', 'c.volume_id = vol.volume_id');

  $query->fields('c');
  $query->addField('vol', 'nickname', 'vol_nickname');

  return $query;
}




/**
 *
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_get_snapshots_list_count($cloud_context) {

  $ec2_lib_snapshot_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SNAPSHOTS_TABLE );

  $query      = 'SELECT COUNT(*) as item_count FROM {' . $ec2_lib_snapshot_table . '} where cloud_type = :cloud_type ';

  $result     = db_query( $query, array(':cloud_type' => $cloud_context) );
  $item_count = 0;


  foreach ($result as $item) {

    $item_count = $item->item_count;
  }

  return $item_count;
}


function _aws_ec2_lib_get_count_name_snapshots_query($cloud_context) {

  $ec2_lib_snapshot_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SNAPSHOTS_TABLE );

  return $query = 'SELECT count(*) as count FROM {' . $ec2_lib_snapshot_table . '} c WHERE nickname =:nickname ';
}


/**
 *
 * @param     $cloud_context
 * @param     $ss_id
 * @return
 */
function _aws_ec2_lib_get_snapshots_by_id_db($cloud_context, $ss_id) {

  $ec2_lib_snapshot_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SNAPSHOTS_TABLE );
  $ec2_lib_ebs_volumes_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );

  $query =  'select snap.nickname    as snap_nickname  ,
                    snap.snapshot_id as snapshot_id    ,
                    snap.status      as snap_status    ,
                    snap.volume_id   as snap_volume_id ,
                    snap.created     as snap_created   ,
                    vol.nickname as vol_nickname ,
                    snap.volume_size as vol_size ,
                    vol.vol_zone as vol_zone
                    from
                    ( SELECT c.snapshot_id,  c.nickname,   c.volume_id,  c.status,  c.created,  c.progress, c.volume_size FROM {' . $ec2_lib_snapshot_table . '} c
                    WHERE c.snapshot_id= :snapshot_id  ) snap
                    LEFT JOIN
                     {' . $ec2_lib_ebs_volumes_table . '} vol
                    on snap.volume_id = vol.volume_id
                    and cloud_type = :cloud_type
                    ';
  $query_args = array(
    ':snapshot_id' => $ss_id,
    ':cloud_type' => $cloud_context,
  );

  $result_arr = array();

  $db_result  = db_query( $query, $query_args );

  foreach ($db_result as $row) {

    foreach ($row as $key => $val) {

      $result_arr[$key] = $val;
    }
  }

  return $result_arr;
}


/**
 *
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_download_describe_snapshots($cloud_context) {

  $ec2_lib_snapshot_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SNAPSHOTS_TABLE );
  $myclient               = aws_ec2_api_describe_snapshots($cloud_context);

  if ( $myclient->code != 200  ) {
    return FALSE;
  }

  $obj_response   = new SimpleXMLElement($myclient->data);

  $id_nick_arr = cloud_get_all_nickname('snapshot_id', 'nickname', $ec2_lib_snapshot_table );

  $num_deleted = db_delete($ec2_lib_snapshot_table)
      ->condition('cloud_type', $cloud_context)
      ->execute();

  $insert_query = db_insert($ec2_lib_snapshot_table)
      ->fields(
          array(
    'snapshot_id',
    'cloud_type'  ,
    'nickname'    ,
    'volume_id'   ,
    'status'      ,
    'created'     ,
    'progress'    ,
    'owner_id'    ,
    'volume_size',
    'description',
  )
     );

  $count = 0;
  $owner_id = cloud_get_info($cloud_context , 'user_id');

  foreach ($obj_response->snapshotSet->item as $item) {
    $id = '' . $item->snapshotId;

    $exists = db_query("select snapshot_id from {" . $ec2_lib_snapshot_table . "} where snapshot_id = :snapshot_id ", array(':snapshot_id' => $id))->fetchField();
    if ($exists == FALSE
    &&  $item->ownerId == $owner_id) {
    		
      $tmp_nickname = cloud_check_in_array($id_nick_arr, $item->snapshotId)     ;

      $insert_query
        ->values(
            array(
        'snapshot_id' => '' . $item->snapshotId,
        'cloud_type'  => $cloud_context,
        'nickname'    => $tmp_nickname != NULL ? $tmp_nickname : '' . $item->snapshotId,
        'volume_id'   => '' . $item->volumeId,
        'status'      => '' . $item->status,
        'created'     => '' . $item->startTime,
        'progress'    => '' . $item->progress,
        'owner_id'    => isset($item->ownerId     ) ? '' . $item->ownerId     : '',
        'volume_size' => isset($item->volumeSize  ) ? '' . $item->volumeSize  : '',
        'description' => isset($item->description ) ? '' . $item->description : '',
      )
       );
      $count++;
    }
  }

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => 'Updated: Snapshot(s)',
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
 * @param     $cloud_context
 * @param     $myclient
 * @param     $ss_name
 * @param     $ss_desc
 * @return
 */
function _aws_ec2_lib_save_create_snapshot($cloud_context, $myclient, $ss_name, $ss_desc) {

  if ( $myclient->code != 200 ) {
    return FALSE;
  }

  $ec2_lib_snapshot_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SNAPSHOTS_TABLE );

  if ( isset($myclient->data) == FALSE ) {
    return FALSE;
  }

  $obj_response = new SimpleXMLElement($myclient->data);


  $insert_query = db_insert($ec2_lib_snapshot_table)
      ->fields(
          array(
    'snapshot_id',
    'cloud_type'  ,
    'nickname'    ,
    'volume_id'   ,
    'status'      ,
    'created'     ,
    'progress'    ,
    'owner_id'    ,
    'volume_size',
    'description',
  )
     );

  $item = $obj_response;

  $insert_query
        ->values(
            array(
    'snapshot_id' => $item->snapshotId,
    'cloud_type'  => $cloud_context,
    'nickname'    => $ss_name,
    'volume_id'   => $item->volumeId,
    'status'      => $item->status,
    'created'     => $item->startTime,
    'progress'    => $item->progress,
    'owner_id'    => isset($item->ownerId     ) ? $item->ownerId     : '',
    'volume_size' => isset($item->volumeSize  ) ? $item->volumeSize  : '',
    'description' => isset($ss_desc)          ? $ss_desc           : '',
  )
       );

  $insert_query->execute();


  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => t('Snapshot has been added: @snapshot_name', array('@snapshot_name' => $ss_name)),
    'link' => '',
  )
  );

  return TRUE;
}


/**
 *
 * @param     $cloud_context
 * @param     $result
 * @param     $ss_id
 * @return
 */
function _aws_ec2_lib_delete_snapshot_db($cloud_context, $result, $ss_id ) {

  if ( $result->code != 200 ) {
    return FALSE;
  }

  $ec2_lib_snapshot_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SNAPSHOTS_TABLE );

  $obj_response = new SimpleXMLElement($result->data);
  if ( $obj_response->return == 'true' ) {


    $num_deleted = db_delete($ec2_lib_snapshot_table)
      ->condition('snapshot_id', $ss_id)
      ->condition('cloud_type', $cloud_context)
      ->execute();

    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('Snapshot has been deleted: @snapshot_id', array('@snapshot_id' => $ss_id)),
      'link' => '',
    )
    );
    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => 'Updated: User Key(s)',
      'link' => '',
    )
    );

    return TRUE;
  }

  return FALSE;
}


/**
 *
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_get_snapshot_options($cloud_context) {

  $snapshot_options = array(
    AWS_EC2_LIB_CLOUD_NONE => AWS_EC2_LIB_CLOUD_NONE,
  );

  $ec2_lib_snapshot_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SNAPSHOTS_TABLE );
  $ec2_lib_ebs_volumes_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_EBS_VOLUMES_TABLE );

  $query = 'SELECT c.snapshot_id, vol.nickname as vol_nickname , c.nickname, c.volume_id, c.status, c.created, c.progress FROM {' . $ec2_lib_snapshot_table . '} c LEFT JOIN
                     {' . $ec2_lib_ebs_volumes_table . '} vol
                    on c.volume_id = vol.volume_id WHERE c.snapshot_id like :snapshot_id and c.cloud_type = :cloud_type ';

  $query_args = array(
    ':snapshot_id' => '%',
    ':cloud_type' => $cloud_context,
  );

  $snapshot_result = db_query($query, $query_args);

  foreach ($snapshot_result as $snap) {

    $snapshot_options[$snap->snapshot_id] = $snap->nickname . ' (' . $snap->snapshot_id . ')';
  }

  return $snapshot_options;
}


/**
 *
 * @param     $cloud_context
 * @param     $snapshot_id
 * @param     $sql_col
 * @param     $value
 * @return
 */
function _aws_ec2_lib_snapshot_update($cloud_context, $snapshot_id, $sql_col, $value) {

  $ec2_lib_snapshot_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SNAPSHOTS_TABLE );

  $num_updated = db_update($ec2_lib_snapshot_table)
    ->fields(array(
    $sql_col => $value,
  ))
    ->condition('snapshot_id', $snapshot_id, '=')
    ->condition('cloud_type', $cloud_context, '=')
    ->execute();
  return TRUE;
}

// End : DB functions Related to Snapshot
