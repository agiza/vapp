<?php

/**
 * @file
 * aws_ec2_lib Security Group DB
 * DB functions Related to Security Groups
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */


/**
 *
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_get_security_group_query($cloud_context) {

  $ec2_lib_security_groups_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SECURITY_GROUPS_TABLE );
  return $query =   'SELECT c.owner_id, c.group_name, c.group_id, c.group_description, c.permissions FROM {' . $ec2_lib_security_groups_table . '} c WHERE group_name= :group_name and cloud_type = :cloud_type order by group_name asc '  ;
}

function _aws_ec2_lib_get_security_group_name_by_id($cloud_context, $group_id) {
  $ec2_lib_security_groups_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SECURITY_GROUPS_TABLE );

  return db_query("SELECT group_name from {" . $ec2_lib_security_groups_table . "} where cloud_type = :cloud_type and group_id = :group_id ", array(':cloud_type' => $cloud_context, ':group_id' => $group_id))->fetchField();
}

/**
 *
 * @param     $cloud_context
 * @param     $myclient
 * @param     $group_name
 * @param     $grp_desc
 * @return
 */
function _aws_ec2_lib_save_new_security_group($cloud_context, $myclient, $group_name, $grp_desc) {
  //reworked this function a little bit.
  //only start to build the cloud information query if the response is 200
  if ($myclient->code === '200') {
    $ec2_lib_security_groups_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SECURITY_GROUPS_TABLE );
    $obj_response = new SimpleXMLElement($myclient->data);
    $owner_id = cloud_get_info($cloud_context, 'user_id');


    $insert_query = db_insert($ec2_lib_security_groups_table)
      ->fields(
        array(
      'group_name',
      'cloud_type',
      'group_description',
      'owner_id',
      'group_id',
    ))
      ->values(
        array(
      'group_name' => $group_name,
      'cloud_type' => $cloud_context,
      'group_description' => $grp_desc,
      'owner_id' => $owner_id,
      'group_id' => isset($obj_response->groupId) ? $obj_response->groupId : '',
    ))
      ->execute();


    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('Security Group has been added: @group_name', array('@group_name' => $group_name)),
      'link' => '',
    )
    );

    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 *
 * @param     $cloud_context
 * @param     $result
 * @param     $group_name
 * @return
 */
function _aws_ec2_lib_delete_security_group($cloud_context, $result, $group_name ) {

  $ec2_lib_security_groups_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SECURITY_GROUPS_TABLE );
  if ( $result->code != 200 ) {
    return FALSE;
  }

  $obj_response = new SimpleXMLElement($result->data);
  if ( $obj_response->return == 'true' ) {


    $num_deleted = db_delete($ec2_lib_security_groups_table)
      ->condition('group_name', $group_name)
      ->condition('cloud_type', $cloud_context)
      ->execute();

    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('Security Group has been deleted: @group_name', array('@group_name' => $group_name)),
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
function _aws_ec2_lib_get_describe_security_groups_query($cloud_context) {

  $ec2_lib_security_groups_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SECURITY_GROUPS_TABLE );

  $query = db_select($ec2_lib_security_groups_table, 'c');
  $query
    ->fields('c');

  return $query;
}


/**
 *
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_download_describe_security_groups($cloud_context) {

  $ec2_lib_security_groups_table            = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SECURITY_GROUPS_TABLE );
  $ec2_lib_security_groups_details_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SECURITY_GROUPS_DETAILS_TABLE );
  $myclient = aws_ec2_api_describe_security_groups($cloud_context);
  if ( $myclient->code != 200  ) {
    return FALSE;
  }

  $obj_response = new SimpleXMLElement($myclient->data);


  $num_deleted = db_delete($ec2_lib_security_groups_table)
      ->condition('cloud_type', $cloud_context)
      ->execute();

  $num_deleted = db_delete($ec2_lib_security_groups_details_table)
      ->condition('cloud_type', $cloud_context)
      ->execute();


  $insert_query = db_insert($ec2_lib_security_groups_table)
    ->fields(
      array(
    'owner_id',
    'cloud_type',
    'group_name',
    'group_id',
    'group_description',
    'permissions',
  )
  );


  $sg_details_insert_query = db_insert($ec2_lib_security_groups_details_table)
    ->fields(
        array(
    'group_name',
    'cloud_type',
    'ip_protocol',
    'from_port',
    'to_port',
    'user_id',
    'tgt_group_name',
    'cidr_ip',
  )
  );

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => 'Updated: Security Group(s)',
    'link' => '',
  )
  );

  $count = 0;
  $query_args = array();
  $query_details_args = array();

  foreach ($obj_response->securityGroupInfo->item as $item) {


    $exists = db_query("select group_name from {" . $ec2_lib_security_groups_table . "} where group_name = :group_name and cloud_type = :cloud_type ", array(':group_name' => $item->ownerId, ':cloud_type' => $cloud_context))->fetchField();
    if ($exists == FALSE) {

      $insert_query
        ->values(array(
        'owner_id' => $item->ownerId,
        'cloud_type' => $cloud_context,
        'group_name' => $item->groupName,
        'group_id' => $item->groupId,
        'group_description' => $item->groupDescription,
        'permissions' => '',
      )
      );

      $permissions = '';

      foreach ($item->ipPermissions->item as $perm_item) {

        $from_port        = $perm_item->fromPort;
        $to_port          = $perm_item->toPort;
        $ip_protocol      = $perm_item->ipProtocol;

        if ( isset($perm_item->ipRanges) ) {
          foreach ($perm_item->ipRanges->item as $cidr_ip_item) {

            $sg_details_insert_query
              ->values(
                  array(
              'group_name' => $item->groupName,
              'cloud_type' => $cloud_context,
              'ip_protocol' => $ip_protocol,
              'from_port' => $from_port,
              'to_port' => $to_port,
              'user_id' => '',
              'tgt_group_name' => '',
              'cidr_ip' => $cidr_ip_item->cidrIp,
            ));
          }
        }

        if ( isset($perm_item->groups) ) {

          foreach ($perm_item->groups->item as $grp_item) {

            $sg_details_insert_query
              ->values(
                  array(
              'group_name' => $item->groupName,
              'cloud_type' => $cloud_context,
              'ip_protocol' => $ip_protocol,
              'from_port' => $from_port,
              'to_port' => $to_port,
              'user_id' => $grp_item->userId,
              'tgt_group_name' => $grp_item->groupName,
              'cidr_ip' => '',
            ));
          }
        }
      }

      $count++;
    }
  }

  if ( $count == 0 ) {
    return TRUE;
  }

  $insert_query->execute();
  $sg_details_insert_query->execute();

  return TRUE;


}


/**
 *
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_get_security_groups($cloud_context) {

  $security_group_options = array();

  $ec2_lib_security_groups_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SECURITY_GROUPS_TABLE );
  $query        = 'SELECT c.owner_id,  c.group_name ,  c.group_description ,  c.permissions  FROM {' . $ec2_lib_security_groups_table . '} c WHERE group_name like :group_name and cloud_type = :cloud_type '  ;
  $query_args   = array(
    ':group_name' => '%',
    ':cloud_type' => $cloud_context,
  );



  $security_group_result = db_query( $query, $query_args );


  foreach ($security_group_result as $sg) {

    $security_group_options[$sg->group_name] = drupal_strtolower($sg->group_name);
  }
  asort( $security_group_options);
  return $security_group_options;
}



/**
 *
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_get_security_groups_details_db($cloud_context, $sg_name = '' ) {

  $ec2_lib_security_groups_details_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SECURITY_GROUPS_DETAILS_TABLE );
  $security_group_details = array();

  $query_args   = array();
  $query = '';

  if ( empty($sg_name) ) {

    $query        =  'SELECT *  FROM {' . $ec2_lib_security_groups_details_table . '} c where cloud_type = :cloud_type '  ;
    $query_args   = array(
      ':cloud_type' => $cloud_context,
    );
  }
  else {

    $query        =  'SELECT *  FROM {' . $ec2_lib_security_groups_details_table . '} c WHERE group_name= :group_name and cloud_type = :cloud_type '  ;
    $query_args   = array(
      ':group_name' => $sg_name,
      ':cloud_type' => $cloud_context,
    );
    $security_group_details[$sg_name] = array();
  }

  $query .=   ' order by ip_protocol desc ';
  $security_groups_details_result = db_query( $query, $query_args);

  $cnt = 0;

  foreach ($security_groups_details_result as $sg_details) {

    if (isset($security_group_details[$sg_details->group_name]) === FALSE ) {

      $security_group_details[$sg_details->group_name] = array();
    }

    $security_group_details[$sg_details->group_name][$cnt] = array();
    $security_group_details[$sg_details->group_name][$cnt]['group_name'    ]      = $sg_details->group_name;
    $security_group_details[$sg_details->group_name][$cnt]['ip_protocol'   ]      = $sg_details->ip_protocol;
    $security_group_details[$sg_details->group_name][$cnt]['from_port'     ]      = $sg_details->from_port;
    $security_group_details[$sg_details->group_name][$cnt]['to_port'       ]      = $sg_details->to_port;
    $security_group_details[$sg_details->group_name][$cnt]['user_id'       ]      = $sg_details->user_id;
    $security_group_details[$sg_details->group_name][$cnt]['tgt_group_name']      = $sg_details->tgt_group_name;
    $security_group_details[$sg_details->group_name][$cnt]['cidr_ip'       ]      = $sg_details->cidr_ip;
    $cnt++;
  }

  return $security_group_details;
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_security_groups_list_count($cloud_context) {

  $ec2_lib_security_groups_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_SECURITY_GROUPS_TABLE );
  if (empty($ec2_lib_security_groups_table) ) {

    return TRUE;
  }

  $query    = 'SELECT count(*) as sg_count FROM {' . $ec2_lib_security_groups_table . '} c where cloud_type = :cloud_type '  ;

  $result   = db_query( $query, array(':cloud_type' => $cloud_context) );
  $sg_count = 0;

  foreach ($result as $sg) {

    $sg_count = $sg->sg_count;
  }

  return $sg_count;
}


// End : DB functions Related to Security Groups
