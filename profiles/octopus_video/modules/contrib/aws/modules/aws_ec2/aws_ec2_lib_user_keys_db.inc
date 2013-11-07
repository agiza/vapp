<?php


/**
 * @file
 * EC2_lib User Keys DB
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */


// Start : DB functions Related to User Info keys
// This should be in sync with the ssh keys table


/**
 *
 * @param     $cloud_context
 * @param     $ssh_keys
 * @return
 */
function _aws_ec2_lib_download_all_user_keys($cloud_context, $ssh_keys) {

  $ec2_lib_user_keys_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_USER_KEYS_TABLE );

  $user_key_arr = cloud_get_all_nickname('key_name', 'owner', $ec2_lib_user_keys_table);


  $num_deleted = db_delete($ec2_lib_user_keys_table)
      ->condition('cloud_type', $cloud_context)
      ->execute();


  $insert_query = db_insert($ec2_lib_user_keys_table)
      ->fields(
        array(
    'key_name',
    'cloud_type',
    'owner',
  )
     );

  $query_args = array();

  $len = sizeof($ssh_keys);
  $count = 0;

  while ($count < $len) {

    $exists = db_query("select key_name from {" . $ec2_lib_user_keys_table . "} where key_name = :key_name and cloud_type = :cloud_type ", array(':key_name' => $ssh_keys[$count], ':cloud_type' => $cloud_context))->fetchField();
    if ($exists == FALSE) {

      $tmp_key = cloud_check_in_array($user_key_arr, $ssh_keys[$count]);
      $insert_query
        ->values(array(
        'key_name' => $ssh_keys[$count],
        'cloud_type' => $cloud_context,
        'owner' => $tmp_key == NULL ? $ssh_keys[$count] : $tmp_key,
      ));

      $count = $count + 4;
    }
  }

  $insert_query->execute();

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => 'Updated: User Key(s)',
    'link' => '',
  )
  );


  return TRUE;
}


/**
 * Store the user Key
 * @global     $user
 * @param     $cloud_context
 * @param     $key
 * @return
 */
function _aws_ec2_lib_add_user_key($cloud_context, $key) {

  $ec2_lib_user_keys_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_USER_KEYS_TABLE );
  global $user;

  $owner = NULL;
  if ( isset( $user->name ) ) {

    $owner = $user->name;
  }


  if ($owner == NULL || empty($owner)) {
    $owner = $key;
  }

  $num_deleted = db_delete($ec2_lib_user_keys_table)
      ->condition('key_name', $key)
      ->condition('cloud_type', $cloud_context)
      ->execute();


  $insert_query = db_insert($ec2_lib_user_keys_table)
      ->fields(
        array(
    'key_name' => $key,
    'cloud_type' => $cloud_context,
    'owner' => $owner,
  ))
      ->execute();

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => t('User Key has been added: @key', array('@key' => $key)),
    'link' => '',
  )
  ); // //
  return;
}


/**
 *
 * @global     $user
 * @param     $cloud_context
 * @param     $key
 * @return
 */
function _aws_ec2_lib_remove_user_key($cloud_context, $key) {

  $ec2_lib_user_keys_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_USER_KEYS_TABLE );
  global $user;

  $owner = $user->name;

  $num_deleted = db_delete($ec2_lib_user_keys_table)
      ->condition('key_name', $key)
      ->condition('cloud_type', $cloud_context)
      ->execute();

  return;
}


/**
 *
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_get_user_keys_query($cloud_context) {

  $ec2_lib_user_keys_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_USER_KEYS_TABLE );

  return $query = 'SELECT c.key_name,  c.owner from {' . $ec2_lib_user_keys_table . '} c WHERE key_name=:key_name and cloud_type = :cloud_type ';
}


/**
 *
 * @global     $user
 * @param     $cloud_context
 * @param     $source
 * @return
 */
function _aws_ec2_lib_check_key($cloud_context, $source) {

  global $user;

  $user_keys_all = array();
  $user_keys_all[] = $user->name;
  $query = _aws_ec2_lib_get_user_keys_query($cloud_context);
  $query_args = array(
    ':key_name' => $user->name,
    ':cloud_type' => $cloud_context,
  );


  $result_keys = db_query( $query, $query_args );

  foreach ($result_keys as $key) {
    $user_keys_all[] = $key->key_name;
  }

  if (in_array($source, $user_keys_all)) {
    return TRUE;
  }

  return FALSE;
}


/**
 *
 * @global     $user
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_get_all_keys($cloud_context) {
  global $user;
  $user_keys_all = array();
  $user_keys_all[] = $user->name;

  $query = _aws_ec2_lib_get_user_keys_query($cloud_context);
  $query_args = array(
    ':key_name' => $user->name,
    ':cloud_type' => $cloud_context,
  );


  $result_keys = db_query( $query, $query_args );

  foreach ($result_keys as $key) {
    $user_keys_all[] = $key->key_name;
  }
  return $user_keys_all;
}
// End : DB functions Related to User Info keys

