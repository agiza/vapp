<?php

/**
 * @file
 * aws_ec2_lib UI for Images
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */

/**
 * Updated by yas   2011/02/17
 * Updated by yas   2011/02/11
 * Updated by yas   2011/02/02
 */

/**
 *
 * @param   $form_details
 * @param   $cloud_context
 * @param   $owner_flag
 * @return
 */
function aws_ec2_lib_display_images_list($form_details, $form_state, $cloud_context, $owner_flag) {

  drupal_add_js(cloud_get_module_base() . 'js/cloud.js');

  $column      = 'image_nickname';
  $query_args  = array();
  $options     = array(
    t('NickName'    ),
    t('Image ID'    ),
    t('Architecture'),
    t('Location'    ),
    t('Ownership'   ),
    // t('Launch'      ) ,
  );


  $filter     = isset($_REQUEST['filter'   ]) ? $_REQUEST['filter'   ] : '';
  $filter_col = isset($_REQUEST['operation']) ? $_REQUEST['operation'] : 0  ; // default: Nickname
  $filter     = trim($filter);

  $sql_col   = 'image_nickname';

  if ( $filter_col == 0 ) {
    $column  = 'Image Name'     ;
    $sql_col = 'image_nickname';
  }
  elseif ($filter_col == 1 ) {
    $column  = 'Image ID'       ;
    $sql_col = 'image_id'       ;
  }
  elseif ($filter_col == 2 ) {
    $column  = 'Architecture'   ;
    $sql_col = 'architecture'   ;
  }
  elseif ($filter_col == 3 ) {
    $column  = 'Location'       ;
    $sql_col = 'image_location';
  }
  elseif ($filter_col == 4 ) {
    $column  = 'Ownership'      ;
    $sql_col = 'imageowner_id'  ;
  }
  elseif ($filter_col == 5 ) {
    $column  = 'Launch'         ;
    $sql_col = 'image_id'       ;
  }

  if ( isset($filter) ) {

    $query_args[] = $sql_col;
    $query_args[] = $filter  ;
  }

  $form['options'] = array(
    '#prefix' => '<div class="container-inline">',
    '#type' => 'fieldset',
    '#suffix' => '</div>'
 , // '#title' => t('Filter'), 
  );

  // asort($options);

  $image_count = _aws_ec2_lib_get_images_list_count($cloud_context);
  $filter_disabled = '';
  if ( $image_count < 2 ) {

    $filter_disabled = TRUE;
  }

  $form['options']['label'    ] = array(
    '#type' => 'item'     ,
    '#title' => t('Filter'),
  );
  $form['options']['operation'] = array(
    '#type' => 'select'   ,
    '#options' => $options,
    '#default_value' => $filter_col,
    '#disabled' => $filter_disabled,
  );
  $form['options']['filter'   ] = array(
    '#type' => 'textfield',
    '#size' => 40      ,
    '#default_value' => $filter    ,
    '#disabled' => $filter_disabled,
  );
  $form['options']['submit'   ] = array(
    '#type' => 'submit'   ,
    '#value' => t('Apply')                               ,
    '#disabled' => $filter_disabled,
  );

  $form['header'] = array(
    '#type' => 'value',
    '#value' => array(
      array(
        'data' => t('Nickname'    ),
        'field' => 'image_nickname'   ,
        //'class' => 'nickname-column'  ,
        'sort' => 'asc',
      ),
      array(
        'data' => t('ID'          ),
        'field' => 'image_id',
      ),
      array(
        'data' => t('Architecture'),
        'field' => 'architecture',
      ),
      array(
        'data' => t('Location'    ),
        'field' => 'image_location',
      ),
      array(
        'data' => t('Ownership'   ),
        'field' => 'imageowner_id',
      ),
      array(
        'data' => t('Action'      ),
        'class' => 'action-column',
      ),
    ),
  );
  if ($owner_flag === 'all' ) {
    $query = _aws_ec2_lib_get_describe_images_query($cloud_context);
    $query->condition('c.cloud_type', $cloud_context, '=');
    $query->condition($sql_col, '%' . $filter . '%', 'like');


  }
  else {

    $cloud_user_id = cloud_get_info( $cloud_context, 'user_id' )  ;
    $query         = _aws_ec2_lib_get_describe_owner_images_query($cloud_context);
    $query
      ->condition('imageowner_id', $cloud_user_id, '=')
      ->condition('cloud_type', $cloud_context, '=')
      ->condition($sql_col, '%' . $filter . '%', 'like');
  }


  $query = $query
      ->extend('TableSort')
      ->extend('PagerDefault')
      ->orderByHeader($form['header']['#value'])
      ->limit(AWS_EC2_LIB_PAGER_LIMIT)
      ->range(0, AWS_EC2_LIB_PAGER_LIMIT);;

  $result = $query->execute();
  $destination = drupal_get_destination();


  foreach ($result as $image) {

    $form['hdnName'     ][$image->image_id] = array(
      '#type' => 'hidden',
      '#value' => addslashes($image->image_nickname),
    );
    $form['Nickname'    ][$image->image_id] = array(array(
        '#prefix' => '<span class=\'anchor\'>',
        '#markup' => t( $image->image_nickname ),
        '#suffix' => '</span>',
      ));
    $form['ID'          ][$image->image_id] = array(array('#markup' => t( $image->image_id       )));
    $form['Architecture'][$image->image_id] = array(array('#markup' => t( $image->architecture   )));
    $form['Location'    ][$image->image_id] = array(array('#markup' => t( $image->image_location )));
    $form['Ownership'   ][$image->image_id] = array(array('#markup' => t( $image->imageowner_id  )));
    $form['Image_type'  ][$image->image_id] = array(array(
        '#value' => t( $image->image_type     ),
        '#type' => 'hidden',
      ));
  }

  $form['cloud_context'] = array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );

  $form['pager'] = array('#value' => theme('pager', array('tags' => NULL, 'element' => 0)));
  $form_state['#redirect']  = FALSE;
  $form['#action'] = url($_REQUEST['q']);

  return $form;
}


/**
 *
 * @param   $form
 * @return
 */
function theme_aws_ec2_lib_display_images_list($form) {


  if (isset($form['form'])) { // drupal 7 passed form as an array 
    $form = $form['form'];
  }

  $cloud_context    = $form['cloud_context']['#value'];
  $cloud_menu_path  = cloud_get_menu_path($cloud_context );
  $output           = drupal_render($form['options']);
  $form['Nickname'] = !empty($form['Nickname']) ? $form['Nickname'] : array();
  $form['pager']['#value'] = !empty($form['pager']['#value']) ? $form['pager']['#value'] : '';
  $rows = array();

  $launch_instance_access = FALSE;
  if (user_access($cloud_context . ' launch instance') ) {

    $launch_instance_access = TRUE;
  }

  foreach (element_children($form['Nickname']) as $key) {

    $row = array();
    $row[] = array(
      'data' => drupal_render($form['Nickname'][$key]),
      //'class' => 'nickname-column',
    );
    $row[] = drupal_render( $form['ID'          ][$key] );
    $row[] = drupal_render( $form['Architecture'][$key] );
    $row[] = drupal_render( $form['Location'    ][$key] );
    $row[] = drupal_render( $form['Ownership'   ][$key] );


    $img_name = $form['Nickname'][$key][0]['#markup'] . ' (' . $key . ')';

    if ($launch_instance_access ) {

      $action_data = cloud_display_action( 'images/icon_play.png', 'Template',    CLOUD_SERVER_TEMPLATES_PATH . '/' . $cloud_context . '/create',    array('query' => array('image_type' => $form['Image_type'   ][$key][0]['#value']  , 'ec2_image_id_select' => urlencode($img_name)), 'html' => TRUE) );
    }
    else {

      $action_data = '';
    }

    $row[]  = array(
      'data' => $action_data,
      'class' => 'action-column',
    );
    $rows[] = $row;
  }

  $output .= theme('table', array('header' => $form['header']['#value'], 'rows' => $rows));

  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $destination = drupal_get_destination();
  $reload_link = l( t('- Refresh Page -'), $cloud_menu_path . '/getimagedata',    array('query' => $destination) );
  $ref_link = array(
    '#type' => 'item',
    '#prefix' => '<div id="link_reload" align="right">',
    '#suffix' => '</div>',
    '#markup' => $reload_link,
  );
  $output .= theme('pager', array('tags' => array() ));
  $output .= drupal_render($ref_link);
  $output .= drupal_render( $form['cloud_context'] );
  $output .= drupal_render_children($form);

  return $output;
}


/**
 *
 * @param   $form_id
 * @param   $form_values
 * @return
 */
function aws_ec2_lib_display_images_list_submit($form_id, &$form_values) {

  $form_values = $form_values['values'];

  $cloud_context = $form_values['cloud_context'];
  if ( $form_values['op'] == t('Bundle') ) {

    $cloud_menu_path = cloud_get_menu_path($cloud_context );
    drupal_goto($cloud_menu_path . '/images/bundle_image');
  }
  elseif ( $form_values['op'] == t('Apply') ) {

    $param_arr = array(
      'filter' => $form_values['filter'],
      'operation' => $form_values['operation'],
    );

    $destination = drupal_get_destination();
    $form_values['redirect']     = array($destination['destination'], array('query' => $param_arr));
  }

  return;
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function _aws_ec2_lib_get_instances($cloud_context) {

  $instance_options = array();
  $query  = _aws_ec2_lib_get_instance_names($cloud_context);
  $query_args = array(
    ':cloud_type' => $cloud_context,
  );

  $result = db_query( $query, $query_args );


  foreach ($result as $instance) {

    $instance_options[$instance->instance_id] = $instance->instance_nickname;
  }
  asort($instance_options);
  return $instance_options;
}


/**
 * Form for Image Bundling
 *
 * @param   $form_submit
 *              Form Object
 * @param   $cloud_context
 *              Sub-Cloud
 * @return
 */
function aws_ec2_lib_display_bundle_image_form($form_submit = '', $cloud_context) {

  $instance_options = _aws_ec2_lib_get_instances($cloud_context);
  $form['bundl_image_details']['instance_label'] = array(
    '#type' => 'item'     ,
    '#title' => t('EC2 Image'),
  );
  $form['bundl_image_details']['instance_select'] = array(
    '#type' => 'select'   ,
    '#options' => $instance_options,
  );

  $nickname = date( "Y_n_j G:i:s T");
  $form['bundl_image_details']['nickname_label'] = array(
    '#type' => 'item'     ,
    '#title' => t('Nickname'),
  );
  $form['bundl_image_details']['nickname_text'  ] = array(
    '#type' => 'textfield',
    '#value' => $nickname,
  );

  $form['bundl_image_details']['vol_size_label'] = array(
    '#type' => 'item'     ,
    '#title' => t('Root Volume Size(in MB)'),
  );
  $form['bundl_image_details']['vol_size_text'  ] = array(
    '#type' => 'textfield',
    '#value' => '10240',
  );

  $form['bundl_image_details']['cloud_text'     ] = array(
    '#type' => 'hidden'   ,
    '#value' => $cloud_context,
  );

  $form['cloud_context'] =  array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );

  return $form;
}


/**
 *
 * @param   $form
 * @return
 */
function theme_aws_ec2_lib_display_bundle_image_form($form) {


  $rows = array(
    array(
      drupal_render($form['bundl_image_details']['instance_label'] ),
      drupal_render($form['bundl_image_details']['instance_select']),
    ),
    array(
      drupal_render($form['bundl_image_details']['nickname_label']),
      drupal_render($form['bundl_image_details']['nickname_text'] ),
    ),
    array(
      drupal_render($form['bundl_image_details']['vol_size_label']),
      drupal_render($form['bundl_image_details']['vol_size_text'] ),
    ),
    array(
      drupal_render($form['bundl_image_details']['cloud_text']    ),
    ),
  );

  $output  = theme('table', array('header' => NULL, 'rows' => $rows));
  $output .= drupal_render($form['bundle_submit_buttons']);
  $output .= drupal_render($form['cloud_context'        ]);
  $output .= drupal_render($form);

  return $output;
}


/**
 *
 * @param   $form_id
 * @param   $form_values @
 */
function aws_ec2_lib_display_bundle_image_form_validate($form_id, $form_values) {

  $form_values = $form_values['values'];
  if ( $form_values['op'] == t('Bundle') ) {

    $size =  $form_values['vol_size_text'];
    if ( empty($form_values['nickname_text']) ) {

      form_set_error('', t('You must enter Nickname'));
    }
    elseif ( !preg_match(CLOUD_VALID_DIGIT, $size) ) {

      form_set_error('',    t('You must enter valid volume Size'));
    }
  }
}

/**
 *
 * @param   $form_id
 * @param   $form_values
 * @return
 */
function aws_ec2_lib_display_bundle_image_form_submit($form_id, $form_values) {

  $form_values = $form_values['values'];
  $cloud_context = $form_values['cloud_context'];
  if ( $form_values['op'] == t('Cancel') ) {

    $cloud_menu_path = cloud_get_menu_path($cloud_context );
    drupal_goto( $cloud_menu_path . '/images');

    return;
  }
  elseif ($form_values['op'] == t('Bundle')) {

    _aws_ec2_lib_bundle_image( $cloud_context, $form_values['instance_select'], $form_values['vol_size_text'],    '', $form_values['nickname_text'], $form_values['cloud_text'] );
    drupal_set_message(t('Initiated Bundle Image request. This will take some time'));

  }
}


/**
 *
 * @param   $cloud_context
 * @return
 */
function aws_ec2_lib_register_image($cloud_context) {

  return drupal_get_form('aws_ec2_lib_register_image_response');
}


/**
 *
 * @return
 */
function aws_ec2_lib_register_image_response($form) {

  $id            = $_REQUEST['image_id'           ];
  $location      = $_REQUEST['location'     ];
  $cloud_context = $_REQUEST['cloud_context'];

  if ( empty($id) || empty($location)  ) {
    print( _aws_ec2_lib_compose_response('ID/Location empty : FAILED') );
    return NULL;
  }

  $query      = _aws_ec2_lib_get_register_image_query($cloud_context);
  $query_args = array(
    ':id' => $id,
    ':cloud_type' => $cloud_context,
  );

  $result     = db_query( $query, $query_args );

  $count = 0;

  foreach ($result as $rec) {

    $status = $rec->status;
    $imgid  = $rec->image_id;
    $count++;
  }

  if ( $count == 0 ) {
    print( _aws_ec2_lib_compose_response('ID not found in database : FAILED') );
    return NULL;
  }

  if ( $status == 'SUCCESS' ) {
    print( _aws_ec2_lib_compose_response('SUCCESS: ' . $imgid) );
    return NULL;
  }

  $myclient     = aws_ec2_api_register_image($cloud_context, $location);
  if ($myclient->code == 200 ) {
    $obj_response = new SimpleXMLElement($myclient->data);
  }
  else {
    $obj_response = NULL;
  }

  if (     $obj_response != NULL
    && isset($obj_response->imageId)
    && empty($obj_response->imageId) == FALSE ) {

    $imgid = $obj_response->imageId;
    _aws_ec2_lib_update_image_register_status( $cloud_context, $id, 'SUCCESS', $imgid )  ;
    print( _aws_ec2_lib_compose_response('SUCCESS:' . $imgid) );
    return NULL;
  }

  _aws_ec2_lib_update_image_register_status( $cloud_context, $id,    'FAILED', '' )  ;
  print( _aws_ec2_lib_compose_response(t('Tried to register but FAILED')) );
  return NULL;
}


/**
 *
 * @param   $msg
 * @return
 */
function _aws_ec2_lib_compose_response($msg) {

  $RESPONSE_LEN = 40;
  $RESPONSE_TERMINATOR = ':';

  $len = 0;
  if (isset($msg) == FALSE || empty($msg) ) {
    $len = 0;
    $resp_msg = '';
  }
  else {
    $len = strlen($msg);
    $resp_msg = $msg;
  }

  for ($count =  $len; $count < $RESPONSE_LEN; $count++ ) {
    $resp_msg .= '_';
  }

  $resp_msg .= $RESPONSE_TERMINATOR  ;

  return $resp_msg;
}



/**
 *
 * @param   $cloud_context
 * @param   $instance_id
 * @param   $key_name
 * @param   $TEMP_FOLDER
 * @return
 */
function _aws_ec2_lib_create_instance_permission_file($cloud_context, $instance_id, $key_name, $TEMP_FOLDER) {

  $query        = aws_ec2_lib_get_ssh_keys_query($cloud_context);
  $query_args   = array(
    ':key_name' => $key_name,
    ':cloud_type' => $cloud_context,
  );

  $result       = db_query( $query, $query_args );


  foreach ($result as $key) {
    $key_material  =  $key->key_material;
  }

  $perm_file = $TEMP_FOLDER . CLOUD_PATH_SEPARATOR . $key_name;

  $fh = fopen($perm_file,    'w');
  if ( $fh == FALSE) {

    drupal_set_message( t('Unable to create the key file. Contact administrator'),    'error' );

    return;
  }

  fwrite($fh, $key_material );
  fclose($fh);

  $command = "chmod 500 $perm_file"; // Make the file readonly
  exec( $command, $output, $retval);

  return $perm_file;
}


/**
 *
 * @param   $cloud_context
 * @param   $instance_id
 * @param   $TEMP_FOLDER
 * @param   $BUCKET_NAME
 * @param   $SIZE
 * @return
 */
function _aws_ec2_lib_create_bundle_image_script_file( $cloud_context, $instance_id, $TEMP_FOLDER, $BUCKET_NAME, $SIZE) {

  $script_id   = _get_scriptIdfromMaster(AWS_EC2_LIB_BUNDLE_IMAGE_SCRIPT);
  $scripts_obj = _get_script_by_id($script_id);
  $script_str  = $scripts_obj->script_template;

  $script_str = str_replace("\r\n",    "\n", $script_str, $count);

  $user_id            = cloud_get_info( $cloud_context, 'user_id'            )  ;
  $access_key         = cloud_get_info( $cloud_context, 'aws_access_key'     );
  $aws_secret_key     = cloud_get_info( $cloud_context, 'aws_secret_key'     );
  $image_upload_url   = cloud_get_info( $cloud_context, 'image_upload_url'   );
  $image_register_url = cloud_get_info( $cloud_context, 'image_register_url' );

  $script_str =  str_replace( 'BUCKET_NAME'           , "$BUCKET_NAME", $script_str );
  $script_str =  str_replace( 'SIZE'                  , "$SIZE", $script_str );
  $script_str =  str_replace( 'AWS_USER_ID'           , $user_id           , $script_str );
  $script_str =  str_replace( 'AWS_ACCESS_KEY_ID'     , $access_key        , $script_str );
  $script_str =  str_replace( 'AWS_SECRET_ACCESS_KEY', $aws_secret_key    , $script_str );
  $script_str =  str_replace( 'IMAGE_UPLOAD_URL'      , $image_upload_url  , $script_str );
  $script_str =  str_replace( 'IMAGE_REGISTER_URL'    , $image_register_url, $script_str );

  $script = $script_str;
  $script_file = $TEMP_FOLDER . CLOUD_PATH_SEPARATOR . $script_id;

  $fh = fopen($script_file, 'w');
  if ( $fh == FALSE) {

    drupal_set_message( t('Unable to create the master script file.'), 'error' );

    return;
  }

  fwrite($fh, $script );
  fclose($fh);

  return $script_id;
}


/**
 *
 * @param   $file_path
 * @param   $TARGET
 * @return
 */
function _aws_ec2_lib_create_key_files($file_path, $TARGET) {

  $command = "cp $file_path $TARGET";
  exec($command, $output, $retval);

  return $file_path;
}



/**
 *
 * @param   $cloud_context
 * @param   $instance_id
 * @param   $size
 * @param   $bucket_id
 * @param   $nickname
 * @return
 */
function _aws_ec2_lib_bundle_image( $cloud_context, $instance_id, $size, $bucket_id, $nickname) {

  $bucket_id = 'bucket/' . $instance_id;
  $register_image_id = REQUEST_TIME;

  $source_folder = '/tmp/' . $instance_id . CLOUD_PATH_SEPARATOR . $register_image_id . CLOUD_PATH_SEPARATOR . '.ec2';

  $command = "rm -fr $source_folder"; //Command for removing already existing folder
  exec($command, $output, $retval);

  $command = "mkdir -p $source_folder";
  exec($command, $output, $retval); //command for creating new empty folder

  $instance_key_dns_name = _cloud_scripting_get_instance_key_dns_name( $cloud_context, $instance_id);

  $instance_ip         = $instance_key_dns_name['instanceIp'       ];
  $instance_state_name = $instance_key_dns_name['instance_state_name'];
  $key_name            = $instance_key_dns_name['key_name'          ];

  if ($instance_state_name == 'running') {

    //Permission file for login to remote instance      
    $perm_file = _aws_ec2_lib_create_instance_permission_file($cloud_context, $instance_id, $key_name, $source_folder );

    if (!empty($key_name)) {

      $script_files = _aws_ec2_lib_create_bundle_image_script_file( $cloud_context, $instance_id, $source_folder, $bucket_id, $size);
    }
  }

  // Write the Public and Private key file
  $tmp_pvt_key_file_path = cloud_get_info( $cloud_context, 'tmp_pvt_key_file_path' )  ;
  $tmp_key_file_path     = cloud_get_info( $cloud_context, 'tmp_key_file_path'     )  ;
  $tmp_log_file_path     = cloud_get_info( $cloud_context, 'tmp_log_file_path'     )  ;

  _aws_ec2_lib_create_key_files($tmp_key_file_path    , $source_folder . CLOUD_PATH_SEPARATOR . 'cert.pem');
  _aws_ec2_lib_create_key_files($tmp_pvt_key_file_path, $source_folder . CLOUD_PATH_SEPARATOR . 'pk.pem'  );

  $log_filename = $tmp_log_file_path . $instance_id;

  $output_dir = file_create_path(AWS_EC2_LIB_TEMP_DIR);
  $tmp_args = " $instance_ip $perm_file $log_filename '$source_folder' '$script_files' " . $output_dir;

  $command  = PHP_PATH . ' -f ' . dirname(__FILE__) . CLOUD_PATH_SEPARATOR . 'aws_ec2_lib_bundle_image.inc ' . $tmp_args . ' >> ' . $tmp_log_file_path . 'tmp.log'  ;

  exec($command, $output, $retval);

  return $retval;
}


/**
 * Delete Image
 *
 * @param   $cloud_context
 *              Image to be deleted from this Sub-cloud
 */
function aws_ec2_lib_delete_image($cloud_context) {


  $image_id = isset( $_REQUEST['image_id'] ) ? $_REQUEST['image_id'] : FALSE;

  $myclient = aws_ec2_api_deregister_image($cloud_context, $image_id);
  if ( $myclient->code == 200 ) {

    drupal_set_message( t('Image deregistered successfully.'  ) );


    $image_upload_url        = cloud_get_info( $cloud_context, 'image_upload_url' )  ;

    $query = _aws_ec2_lib_get_image_query($cloud_context);
    $query_args = array(
      ':image_id' => $image_id,
    );

    $result = db_query( $query, $query_args );

    foreach ($result as $image) {

      $bucket = $image->image_location;
    }

    $tmp_bucket = split( '/', $bucket );
    $bucket = $tmp_bucket[0];

    $tmp_pub_key_file_path = cloud_get_info( $cloud_context, 'tmp_pub_key_file_path' )  ;
    $tmp_pvt_key_file_path = cloud_get_info( $cloud_context, 'tmp_pvt_key_file_path' )  ;
    $tmp_key_file_path     = cloud_get_info( $cloud_context, 'tmp_key_file_path' )  ;
    $tmp_log_file_path     = cloud_get_info( $cloud_context, 'tmp_log_file_path' )  ;
    $cloud_user_id         = cloud_get_info( $cloud_context, 'user_id' )  ;
    $access_key            = cloud_get_info( $cloud_context, 'aws_access_key' );
    $aws_secret_key        = cloud_get_info( $cloud_context, 'aws_secret_key'  );
    $image_upload_url      = cloud_get_info( $cloud_context, 'image_upload_url' );
    $image_register_url    = cloud_get_info( $cloud_context, 'image_register_url' );
    $java_home             = cloud_get_info( $cloud_context, 'java_home' );
    $ec2_home              = cloud_get_info( $cloud_context, 'ec2_home' );
    $tmp_cert_file_path    = cloud_get_info( $cloud_context, 'tmp_cert_file_path' );
    $host_uri              = cloud_get_info( $cloud_context, 'host_uri' );

    $command  = '';
    $command .= " export EC2_HOME=$ec2_home;";
    $command .= " export JAVA_HOME=$java_home;";
    $command .= " export PATH=\$PATH:\$ec2_home/bin ;";
    $command .= " export S3_URL=$image_upload_url ;";
    $command .= " export EC2_URL=http://" . $host_uri . "  ;";
    $command .= " export EC2_PRIVATE_KEY=$tmp_pvt_key_file_path ;";
    $command .= " export EC2_CERT=$tmp_pub_key_file_path ;";
    $command .= " export CERT=$tmp_cert_file_path ;";
    $command .= " export EC2_ACCESS_KEY=$access_key ;";
    $command .= " export EC2_SECRET_KEY=$aws_secret_key ;";
    $command .= " export EC2_USER_ID=$cloud_user_id ;";


    $command .= " yes  | ec2-delete-bundle -a $access_key -s $aws_secret_key --url $image_upload_url -b $bucket -p 'image' --clear ";

    exec( $command_prefix . $command, $output, $retval);

    if ( $retval == 0) {

      drupal_set_message( t('Deleted the image successfully.'  ) );
    }
    else {

      drupal_set_message( check_plain(t('Failed delete of image: @retval', array('@retval' => $retval))), 'error' );
    }
  }
  else {

    drupal_set_message(t('Unable to delete the image'),    'error');
  }

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  drupal_goto($cloud_menu_path . '/images');
}
