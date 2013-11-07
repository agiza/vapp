<?php

/**
 * @file
 * aws_ec2_lib SSH Key DB
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */


/**
 *
 * @param     $cloud_context
 * @param     $result
 * @param     $key_name
 * @return
 *
 */
function _aws_ec2_lib_delete_key_pair($cloud_context, $result, $key_name ) {

  if ( $result->code != 200 ) {
    return FALSE;
  }

  $ec2_lib_ssh_keys_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SSH_KEYS_TABLE )  ;

  $obj_response = new SimpleXMLElement($result->data);
  if ( $obj_response->return == 'true' ) {


    $num_deleted = db_delete($ec2_lib_ssh_keys_table)
      ->condition('key_name', $key_name)
      ->execute();

    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('SSH Key has been deleted: @key_name', array('@key_name' => $key_name)),
      'link' => '',
    )
    );

    _aws_ec2_lib_remove_user_key($cloud_context, $key_name);
    return TRUE;
  }

  return FALSE;
}


/**
 *
 * @param     $cloud_context
 * @param     $myclient
 * @return
 *
 */
function _aws_ec2_lib_save_new_ssh_keypair($cloud_context, $myclient) {

  $ec2_lib_ssh_keys_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SSH_KEYS_TABLE )  ;

  $obj_response = new SimpleXMLElement($myclient->data);
  $count = 0;

  $num_insert = db_insert($ec2_lib_ssh_keys_table)
    ->fields(array(
    'key_name' => $obj_response->keyName,
    'cloud_type' => $cloud_context,
    'fingerprint' => $obj_response->keyFingerprint,
    'key_material' => $obj_response->keyMaterial,
  ))
    ->execute();

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => t('SSH Key has been added: @ssh_key', array('@ssh_key' => $obj_response->keyName)),
    'link' => '',
  )
  );
  _aws_ec2_lib_add_user_key($cloud_context, $obj_response->keyName);

  return;
}


/**
 *
 * @param     $params
 * @return
 *
 */
function _aws_ec2_lib_get_ssh_key($params) {
  $key = "";

  $key_name = $params['key_name'];
  $cloud_context = $params['cloud_context'];
  $query = aws_ec2_lib_get_ssh_keys_query($cloud_context);
  $query_args   = array(
    ':key_name' => $key_name,
    ':cloud_type' => $cloud_context,
  );


  $result = db_query($query, $query_args);

  foreach ($result as $row) {
    $key = $row->key_material;
  }
  return $key;
}


/**
 *
 * @param     $cloud_context
 * @return
 *
 */
function aws_ec2_lib_get_ssh_keys_query($cloud_context) {
  $ec2_lib_ssh_keys_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SSH_KEYS_TABLE )  ;

  return $query =   'SELECT c.key_name,  c.fingerprint ,  c.key_material FROM {' . $ec2_lib_ssh_keys_table . '} c WHERE key_name=:key_name  and cloud_type = :cloud_type order by key_name'  ;
}


/**
 *
 * @param     $cloud_context
 * @return
 *
 */
function _aws_ec2_lib_get_describe_ssh_keys_query($cloud_context) {

  $ec2_lib_ssh_keys_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SSH_KEYS_TABLE )  ;

  $query = db_select($ec2_lib_ssh_keys_table, 'c');
  $query
    ->fields('c');

  return $query;
}


/**
 *
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_download_describe_ssh_keys($cloud_context) {

  $ec2_lib_ssh_keys_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SSH_KEYS_TABLE )  ;
  $myclient = aws_ec2_api_describe_keypairs($cloud_context);
  if ( $myclient->code != 200  ) {
    return FALSE;
  }

  $obj_response = new SimpleXMLElement($myclient->data);

  $key_material_arr = cloud_get_all_nickname('key_name', 'key_material', $ec2_lib_ssh_keys_table );


  db_query('delete from  {' . $ec2_lib_ssh_keys_table . '} where cloud_type = :cloud_type ', array(':cloud_type' => $cloud_context));

  $insert_query = db_insert($ec2_lib_ssh_keys_table)
    ->fields(array(
    'key_name'      ,
    'cloud_type'    ,
    'fingerprint'   ,
    'key_material'  ,
  ));

  //use base_cloud because cloud_context is now user entered
  //and cannot be guaranteed
  $base_cloud = cloud_get_base_cloud($cloud_context);
  switch ($base_cloud) { // Needed for OpenStack
    case AWS_EC2_LIB_OPENSTACK_NOVA_CONTEXT:
      $items = $obj_response->keypairsSet->item
             ? $obj_response->keypairsSet->item
             : $obj_response->keySet->item;
      break;

    default:
      $items = $obj_response->keySet->item;
  }

  if ( empty($items) ) {

    $items = array();
  }

  $insert_values = array();
  $query_args    = array();

  $count = 0;
  foreach ($items as $item) {

    $exists = db_query("select key_name from {" . $ec2_lib_ssh_keys_table . "} where key_name = :key_name and cloud_type = :cloud_type ",
                          array(
      ':key_name' => $item->keyName,
      ':cloud_type' => $cloud_context,
    ))->fetchField();
    if ($exists == FALSE) {

      $query_args[] = $item->keyName;
      $query_args[] = $cloud_context;
      $query_args[] = $item->keyFingerprint;
      $tmp_key_material = cloud_check_in_array($key_material_arr, $item->keyName );
      $query_args[] = $tmp_key_material != NULL ? $tmp_key_material :  '';

      $tmp_key_material = cloud_check_in_array($key_material_arr, $item->keyName );
      $insert_query->values(
            array(
        'key_name' => '' . $item->keyName,
        'cloud_type' => $cloud_context,
        'fingerprint' => '' . $item->keyFingerprint,
        'key_material' => $tmp_key_material != NULL ? $tmp_key_material :  '',
      ));
      $count++;
    }
  }

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => 'Updated: SSH Key(s)',
    'link' => '',
  )
  );

  if ( $count == 0 ) {
    return TRUE;
  }

  $insert_query->execute();

  // save in user-info
  _aws_ec2_lib_download_all_user_keys($cloud_context, $query_args);

  return TRUE;
}


/**
 *
 * @param     $cloud_context
 * @param     $key_name
 * @param     $key_material
 * @return
 *
 */
function _aws_ec2_lib_update_key_material($cloud_context, $key_name, $key_material) {

  $ec2_lib_ssh_keys_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SSH_KEYS_TABLE )  ;

  $key_material = str_replace("\r\n",    "\n", $key_material );
  $key_material = trim($key_material);

  $num_updated = db_update($ec2_lib_ssh_keys_table)
    ->fields(
      array(
    'key_material' => $key_material,
  ))
    ->condition('key_name', $key_name, '=')
    ->condition('cloud_type', $cloud_context, '=')
    ->execute();

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => t('SSH Key has been modified: @key_name', array('@key_name' => $key_name)),
    'link' => '',
  )
  );

  return;
}

/**
 *
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_get_ssh_keys($cloud_context) {


  $ec2_lib_ssh_keys_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SSH_KEYS_TABLE )  ;
  $query =   'SELECT c.key_name,  c.fingerprint ,  c.key_material FROM {' . $ec2_lib_ssh_keys_table . '} c
          WHERE cloud_type =  :cloud_type ';

  $query_args = array(
    ':cloud_type' => $cloud_context,
  );

  $ssh_keys_result = db_query( $query, $query_args );
  $ssh_key_options = array(
    '-1' => AWS_EC2_LIB_CLOUD_NONE,
  );

  foreach ($ssh_keys_result as $ssh_key) {

    $ssh_key_options[$ssh_key->key_name] = drupal_strtolower($ssh_key->key_name);
  }
  asort( $ssh_key_options);

  return $ssh_key_options;
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_ssh_keys_list_count($cloud_context) {

  $ec2_lib_ssh_keys_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SSH_KEYS_TABLE );
  if (empty($ec2_lib_ssh_keys_table) ) {

    return TRUE;
  }

  $query     = 'SELECT COUNT(*) as ssh_count FROM {' . $ec2_lib_ssh_keys_table . '} where cloud_type = :cloud_type '  ;

  $result    = db_query( $query, array(':cloud_type' => $cloud_context) );
  $ssh_count = 0;

  foreach ($result as $ssh) {

    $ssh_count = $ssh->ssh_count;
  }

  return $ssh_count;
}


// End : DB functions Related to SSH Keys

