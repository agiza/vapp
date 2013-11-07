<?php

/**
 * @file
 * Constants for aws_ec2_lib
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */


/**
 *
 * @param   $cloud_context
 * @param   $myclient
 * @param   $instance_id
 * @param   $public_ip
 * @return
 */
function _aws_ec2_lib_save_associate_address($cloud_context, $myclient, $instance_id, $public_ip) {

  if ( $myclient->code != 200) {
    return FALSE;
  }

  $ec2_lib_elastic_ip_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_ELASTIC_IP_TABLE );

  $obj_response = new SimpleXMLElement($myclient->data);

  if ( isset($obj_response->return) && $obj_response->return == 'true' ) {

    $num_updated = db_update($ec2_lib_elastic_ip_table)
      ->fields(array(
      'instance_id' => $instance_id,
    ))
      ->condition('public_ip', $public_ip, '=')
      ->condition('cloud_type', $cloud_context, '=')
      ->execute();

    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('Elastic IP "@public_id" has been assigned to instance "@instance_id".', array(
        '@public_id' => $public_ip,
        '@instance_id' => $instance_id,
      )),
      'link' => '',
    )
    );

    return TRUE;
  }

  return FALSE;
}


/**
 *
 * @param   $cloud_context
 * @param   $myclient
 * @return
 */
function _aws_ec2_lib_save_allocate_address($cloud_context, $myclient) {

  $ec2_lib_elastic_ip_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_ELASTIC_IP_TABLE );

  $obj_response = new SimpleXMLElement($myclient->data);

  if ( isset($obj_response->publicIp) ) {

    $insert_query = db_insert($ec2_lib_elastic_ip_table)
      ->fields(array('public_ip', 'cloud_type', 'public_ip_name'))
      ->values(
          array(
      'public_ip' => $obj_response->publicIp,
      'cloud_type' => $cloud_context,
      'public_ip_name' => '- changeme -',
    )
      )
      ->execute();

    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('Elastic IP has been added: @public_ip', array('@public_ip' => $obj_response->publicIp)),
      'link' => '',
    )
    );

    return TRUE;
  }

  return FALSE;
}

/**
 *
 * @param   $cloud_context
 * @param   $myclient
 * @param   $ip
 * @return
 */
function _aws_ec2_lib_release_address_save($cloud_context, $myclient, $ip) {

  $ec2_lib_elastic_ip_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_ELASTIC_IP_TABLE );

  $obj_response = new SimpleXMLElement($myclient->data);

  if ( isset($obj_response->return) && $obj_response->return == 'true' ) {

    $num_deleted = db_delete($ec2_lib_elastic_ip_table)
      ->condition('public_ip', $ip)
      ->condition('cloud_type', $cloud_context)
      ->execute();

    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('Elastic IP has been deleted: @ip', array('@ip' => $ip)),
      'link' => '',
    )
    );

    return TRUE;
  }

  return FALSE;
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_download_describe_elastic_ips($cloud_context) {

  $ec2_lib_elastic_ip_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_ELASTIC_IP_TABLE );

  try {

    $myclient = aws_ec2_api_describe_address($cloud_context);

    if (empty($myclient)
      ||    isset($myclient->data) === FALSE
      ||          $myclient->code  !=  200) { // Needed for OpenStack
      return FALSE;
    }

    $obj_response = new SimpleXMLElement($myclient->data);

  }
  catch (Exception $e) {

    $cloud_list = cloud_get_all_clouds_display_name();
    cloud_set_config_error_message($cloud_list[$cloud_context], $cloud_context);

    return FALSE;
  }

  $id_nick_arr = cloud_get_all_nickname('public_ip', 'public_ip_name', $ec2_lib_elastic_ip_table );


  $num_deleted = db_delete($ec2_lib_elastic_ip_table)
      ->condition('cloud_type', $cloud_context)
      ->execute();


  $insert_query = db_insert($ec2_lib_elastic_ip_table)
    ->fields(
      array(
    'public_ip',
    'cloud_type',
    'public_ip_name',
    'instance_id',
  )
   );

  $count = 0;
  if (isset($obj_response->addressesSet)
    &&    isset($obj_response->addressesSet->item)
    &&    count($obj_response->addressesSet->item) > 0) {
    foreach ($obj_response->addressesSet->item as $item) {

      $exists = db_query("select public_ip from {" . $ec2_lib_elastic_ip_table . "} where public_ip = :public_ip ", array(':public_ip' => $item->publicIp))->fetchField();

      $instance_id  = '';
      if ($exists == FALSE) {

        if ( empty($item->instanceId) || substr_count( $item->instanceId, 'available') > 0 ) {
          $instance_id  = '';
        }
        else {

          $instance_id  = $item->instanceId;
          $instance_id  = trim( str_replace( '(eucalyptus)', '', $instance_id ) );
          $instance_id  = trim( str_replace( '(admin)'      ,    '', $instance_id ) );
        }

        $tmp_nickname = cloud_check_in_array($id_nick_arr, $item->publicIp);
        $insert_query->values( 
           array(
          'public_ip' => $item->publicIp,
          'cloud_type' => $cloud_context,
          'public_ip_name' => $tmp_nickname != NULL ? $tmp_nickname : $item->publicIp,
          'instance_id' => $instance_id,
        )
        );

        $count++;
      }
    }
  }

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => 'Updated: Elastic IP(s)',
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
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_describe_elastic_ips_query($cloud_context) {

  $ec2_lib_elastic_ip_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_ELASTIC_IP_TABLE );
  $ec2_lib_instances_table  = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE  );

  $query = db_select($ec2_lib_elastic_ip_table, 'c');
  $query
    ->leftjoin($ec2_lib_instances_table, 'd', 'c.instance_id = d.instance_id');

  $query->fields('c');
  $query->fields('d', array('instance_nickname') );

  return $query;
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_elastic_ips_query($cloud_context) {

  $ec2_lib_elastic_ip_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_ELASTIC_IP_TABLE );
  $ec2_lib_instances_table  = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE  );

  return $query = 'SELECT c.public_ip, c.public_ip_name, c.instance_id, d.instance_nickname FROM {' . $ec2_lib_elastic_ip_table . '} c 
  LEFT JOIN {' . $ec2_lib_instances_table . '} d on c.instance_id=d.instance_id where c.public_ip=:public_ip and c.cloud_type=:cloud_type '  ;
}

/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_elastic_ips_list_count($cloud_context) {

  $ec2_lib_elastic_ip_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_ELASTIC_IP_TABLE );
  $ec2_lib_instances_table  = aws_ec2_lib_get_table_info( AWS_EC2_LIB_INSTANCES_TABLE  );

  $query      = 'SELECT COUNT(*) as item_count FROM {' . $ec2_lib_elastic_ip_table . '} c 
  LEFT JOIN {' . $ec2_lib_instances_table . '} d on c.instance_id=d.instance_id and c.cloud_type =:cloud_type '  ;

  $result     = db_query( $query, array(':cloud_type' => $cloud_context) );
  $item_count = 0;

  foreach ($result as $item) {

    $item_count = $item->item_count;
  }

  return $item_count;
}

/**
 *
 * @param type $cloud_context
 * @return string
 */
function _aws_ec2_lib_update_ip_query($cloud_context) {

  $ec2_lib_elastic_ip_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_ELASTIC_IP_TABLE );
  $sql_instance_update = db_update($ec2_lib_elastic_ip_table);
  return $sql_instance_update;
}



/**
 *
 * @param type $cloud_context
 * @return string
 */
function _aws_ec2_lib_get_instance_pub_ip($cloud_context, $params = array()) {

  $instance_id = $params['instance_id'];

  $ec2_lib_elastic_ip_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_ELASTIC_IP_TABLE );
  $sql_instance_select         = 'select public_ip from  {' . $ec2_lib_elastic_ip_table . "} where instance_id= :instance_id and cloud_type=:cloud_type";

  $sql_instance_select_args = array(
    ':instance_id' => $instance_id,
    ':cloud_type' => $cloud_context,
  );

  $result = db_query($sql_instance_select, $sql_instance_select_args);


  foreach ($result as $row) {
    return $row->public_ip;
  }

  return '';
}


/**
 *
 * @param type $cloud_context
 * @param type $params
 * @return type
 */
function _aws_ec2_lib_update_assigned_instance_id($cloud_context, $params = array()) {

  $ec2_lib_elastic_ip_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_ELASTIC_IP_TABLE );

  $num_updated = db_update($ec2_lib_elastic_ip_table)
    ->fields(array(
    'instance_id' => $params['old_instance_id'],
  ))
    ->condition('instance_id', $params['new_instance_id'], '=')
    ->execute();

  return $num_updated;
}

// End : DB functions Related to Elastic IP's    
