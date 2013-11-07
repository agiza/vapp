<?php

/**
 * @file
 * aws_ec2_lib DB handling for Elastic IP
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */


/**
 * Updated by yas 2011/02/22
 */

// Start : DB functions Related to Images

/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_describe_images_query($cloud_context) {

  $ec2_lib_images_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_IMAGES_TABLE );

  $query = db_select($ec2_lib_images_table, 'c');
  $query->fields('c');
  return $query;
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_images_list_count($cloud_context) {

  $ec2_lib_images_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_IMAGES_TABLE );

  $query        = 'SELECT COUNT(*) AS images_count FROM {' . $ec2_lib_images_table . '} c where cloud_type = :cloud_type'  ;


  $result       = db_query( $query, array(':cloud_type' => $cloud_context) );
  $images_count = 0;


  foreach ($result as $img) {

    $images_count = $img->images_count;
  }
  return $images_count;
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_describe_owner_images_query($cloud_context) {

  $ec2_lib_images_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_IMAGES_TABLE );
  $query = db_select($ec2_lib_images_table, 'c');
  $query
    ->fields('c');

  return $query;
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_image_query($cloud_context) {

  $ec2_lib_images_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_IMAGES_TABLE );
  return $query = 'SELECT c.image_nickname, c.image_id, c.image_location, c.imageowner_id, c.image_state, c.architecture FROM {' . $ec2_lib_images_table . '} c WHERE image_id=:image_id  '  ;
}



/**
 *
 * @param   $location
 * @return
 */
function _aws_ec2_lib_get_image_nickname($location) {

  $tok  = explode('/', $location );
  $len  = sizeof($tok);

  $name =  $tok[$len - 1];

  $nickname = explode('.manifest.xml', $name );

  return $nickname[0];
}


/**
 *
 * @param   $cloud_context
 * @param   $image_url_arr
 * @param   $base_url
 * @return
 */
function _aws_ec2_lib_download_images($cloud_context, $image_url_arr, $base_url) {

  if ( empty($base_url)) {

    $cloud_list = cloud_get_all_clouds_display_name();
    cloud_set_config_error_message($cloud_list[$cloud_context], $cloud_context);
    return FALSE;
  }

  $ec2_lib_images_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_IMAGES_TABLE );
  $host_uri = $base_url;
  $myclient = aws_ec2_api_describe_images($cloud_context, $host_uri);
  if ( $myclient->code != 200  ) {
    return FALSE;
  }

  $obj_response = new SimpleXMLElement($myclient->data);

  $id_nick_arr = cloud_get_all_nickname('image_id', 'image_nickname', $ec2_lib_images_table );

  $insert_query = db_insert($ec2_lib_images_table)
      ->fields(
        array(
    'image_id',
    'cloud_type',
    'image_nickname',
    'image_location',
    'image_state',
    'imageowner_id',
    'is_public',
    'architecture',
    'image_type',
    'kernel_id',
    'ramdisk_id',
    'region',
  )
     );

  $count           = 0;
  $records_deleted = FALSE;
  $query_args      = array();

  // Moved the delete out of the loop
  //If Old records not deleted then delete those
  if ($records_deleted === FALSE ) {
    $num_deleted = db_delete($ec2_lib_images_table)
      ->condition('cloud_type', $cloud_context)
      ->execute();

    $records_deleted = TRUE;
  }

  foreach ($obj_response->imagesSet->item as $item) {

    $exists = db_query("select image_id from {" . $ec2_lib_images_table . "} where image_id = :image_id ", array(':image_id' => $item->imageId))->fetchField();
    if (TRUE) {
      $args_index = 0;
      $query_args = array();

      $query_args[$args_index++] = $item->imageId;
      $query_args[$args_index++] = $cloud_context;
      $tmp_nickname = cloud_check_in_array($id_nick_arr, $item->imageId );
      $query_args[$args_index++] = $tmp_nickname != NULL
                    ? $tmp_nickname
                    : _aws_ec2_lib_get_image_nickname($item->imageLocation);
      $query_args[$args_index++] = $item->imageLocation;
      $query_args[$args_index++] = $item->imageState    ;
      $query_args[$args_index++] = $item->imageOwnerId  ;
      $query_args[$args_index++] = $item->isPublic      ;
      $query_args[$args_index++] = isset( $item->architecture ) ? $item->architecture : '';

      //get the base_cloud and make sure it is openstack_nova for the switch statement
      $base_cloud = cloud_get_base_cloud($cloud_context);
      switch ($base_cloud) { // Needed for OpenStack
        // That should be imageType.  In Austin release, there are two types of attributes like
        // 'type' and 'imageType'.  However in Baxer release, they left 'type' and removed 'imageType'!
        case AWS_EC2_LIB_OPENSTACK_NOVA_CONTEXT:
          $query_args[$args_index++] =  isset($item->type     ) ? $item->type
                        : (isset($item->imageType) ? $item->imageType : '');
          break;

        default:
          $query_args[$args_index++] =  isset($item->imageType) ? $item->imageType : ''  ;
      }

      $query_args[$args_index++] = isset( $item->kernelId     ) ? $item->kernelId  : ''  ;
      $query_args[$args_index++] = isset( $item->ramdiskId    ) ? $item->ramdiskId : ''  ;
      $query_args[$args_index++] = !empty($region_url         ) ? $region_url      : ''  ;


      $args_index = 0;
      $insert_query
        ->values(
            array(
        'image_id' => $query_args[$args_index++],
        'cloud_type' => $query_args[$args_index++],
        'image_nickname' => $query_args[$args_index++],
        'image_location' => $query_args[$args_index++],
        'image_state' => $query_args[$args_index++],
        'imageowner_id' => $query_args[$args_index++],
        'is_public' => $query_args[$args_index++],
        'architecture' => $query_args[$args_index++],
        'image_type' => $query_args[$args_index++],
        'kernel_id' => $query_args[$args_index++],
        'ramdisk_id' => $query_args[$args_index++],
        'region' => $query_args[$args_index++],
      )
        );

      $count++;
    }

    if ($count % 900 == 0 ) { // Insert 1000 Records{
      $insert_query->execute();

      $insert_query = db_insert($ec2_lib_images_table)
        ->fields(
          array(
        'image_id',
        'cloud_type',
        'image_nickname',
        'image_location',
        'image_state',
        'imageowner_id',
        'is_public',
        'architecture',
        'image_type',
        'kernel_id',
        'ramdisk_id',
        'region',
      )
       );
    }
  }

  // User Activity Log
  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => 'Update: Image(s)',
    'link' => '',
  )
  );

  if ( $count == 0 ) {
    continue;
  }

  if ($records_deleted === FALSE ) {
    $num_deleted = db_delete($ec2_lib_images_table)
      ->condition('cloud_type', $cloud_context)
      ->execute();
  }

  $insert_query->execute();
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_download_describe_images($cloud_context) {

  ini_set('memory_limit', '512M');

  $region = ''; // getAmazonRegions() ;
  $host = cloud_get_info( $cloud_context, 'host_uri' )  ;
  _aws_ec2_lib_download_images( $cloud_context, $region, $host );

  return TRUE;
}


/**
 *
 * @param   $cloud_context
 * @param   $image_type
 * @return
 */
function aws_ec2_lib_get_all_images_db($cloud_context, $image_type = '') {

  $query = '';
  $query_args    = array();
  $image_options = array();

  $ec2_lib_images_table    = aws_ec2_lib_get_table_info( AWS_EC2_LIB_IMAGES_TABLE );

  $query .= ' SELECT distinct * FROM {' . $ec2_lib_images_table . ' }'
         . ' WHERE image_type like :image_type and cloud_type = :cloud_type '  ;

  $query_args = array(
    ':image_type' => $image_type . '%', // % is used for wildcard
    ':cloud_type' => $cloud_context,
  );

  $image_options = array();
  $image_options[AWS_EC2_LIB_CLOUD_NONE] = AWS_EC2_LIB_CLOUD_NONE;
  $result = db_query( $query, $query_args );

  foreach ($result as $image) {

    $image_options[$image->image_id] =  $image->image_nickname . ' (' . $image->image_id . ')';
  }

  asort( $image_options);
  return $image_options;
}



/**
 *
 * @param   $cloud_context
 * @param   $image_type
 * @param   $search_str
 * @return
 */
function aws_ec2_lib_get_all_images_by_str_db($cloud_context, $image_type = '', $search_str = '') {

  $ec2_lib_images_table = aws_ec2_lib_get_table_info( AWS_EC2_LIB_IMAGES_TABLE );

  $query      = 'SELECT distinct * FROM {' . $ec2_lib_images_table . '} WHERE 
  image_type=:image_type and ( image_id like :image_id or image_nickname like :image_nickname ) 
  and cloud_type = :cloud_type
  order by image_id ';

  $query_args = array(
    ':image_type' => $image_type,
    ':image_id' => '%' . $search_str . '%',
    ':image_nickname' => '%' . $search_str . '%',
    ':cloud_type' => $cloud_context,
  );

  $image_options = array();
  $image_options[AWS_EC2_LIB_CLOUD_NONE] = AWS_EC2_LIB_CLOUD_NONE;
  $result = db_query( $query, $query_args );

  foreach ($result as $image) {

    $image_options[$image->image_nickname . ' (' . $image->image_id . ')'] =  $image->image_nickname . ' (' . $image->image_id . ')';
  }
  asort($image_options);
  return $image_options;
}


/**
 *
 * @param   $cloud_context
 * @param   $img_id @
 */
function aws_ec2_lib_callback_list_machine_images($cloud_context, $img_id) {

  $matches = array();
  $matches =  aws_ec2_lib_get_all_images_by_str_db($cloud_context, 'machine', $img_id );
  print drupal_json_encode($matches);

  exit();
}


/**
 *
 * @param   $cloud_context
 * @param   $img_id
 */
function aws_ec2_lib_callback_list_kernel_images($cloud_context, $img_id) {

  $matches = array();
  $matches =  aws_ec2_lib_get_all_images_by_str_db($cloud_context, 'kernel'  , $img_id );
  print drupal_json_encode($matches);

  exit();
}



/**
 *
 * @param   $cloud_context
 * @param   $img_id @
 */
function aws_ec2_lib_callback_list_ramdisk_images($cloud_context, $img_id) {

  $matches = array();
  $matches = aws_ec2_lib_get_all_images_by_str_db($cloud_context, 'ramdisk', $img_id );
  print drupal_json_encode($matches);

  exit();
}


// End : DB functions Related to Images
