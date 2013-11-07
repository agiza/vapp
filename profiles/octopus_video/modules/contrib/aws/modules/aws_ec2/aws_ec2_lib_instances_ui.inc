<?php

/**
 * @file
 * aws_ec2_lib Instance UI file
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */

/**
 * Updated by Abhijit 2011/09/08
 * Updated by Abhijit 2011/09/07
 * Updated by Jiten   2011/07/06
 * Updated by Jiten   2011/06/24
 * Updated by Jamir   2011/04/30
 * Updated by yas     2011/03/24
 * Updated by yas     2011/03/14
 * Updated by Jamir   2011/03/14
 * Updated by yas     2011/02/17
 * Updated by yas     2011/02/14
 * Updated by yas     2011/02/11
 * Updated by yas     2011/02/10
 * Updated by yas     2011/02/08
 * Updated by yas     2011/02/03
 * Updated by yas     2011/02/02
 */


/**
 * Return the status of an instance
 *
 * @param     $instance_id
 * @param     $status
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_get_instance_status($instance_id, $status, $cloud_context) {

  if ( $status != 'running') {
    return $status;
  }

  $inst_lock_query      = _aws_ec2_lib_get_lock_status_by_cloud_query($cloud_context);
  $inst_lock_query_args = array(
    ':instance_id' => $instance_id,
    ':cloud_type' => $cloud_context,
  );


  $result_inst_lock_query = db_query( $inst_lock_query, $inst_lock_query_args )->fetchObject();

  $status_obj             = $result_inst_lock_query;
  $state = '';

  if ($status_obj != NULL ) {
    $state = $status_obj->script_status;
  }

  return $state;
}


/**
 *
 * @param     $form_submit
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_display_instances_list($form, $form_state = '', $cloud_context) {

  $column = 'instance_nickname';

  drupal_add_js(cloud_get_module_base() . 'js/cloud.js');
  drupal_add_js(drupal_get_path('module', 'aws_ec2_lib')
              . CLOUD_PATH_SEPARATOR
              . 'js/aws_ec2_lib_auto_refresh_list_instances.js');

  $options = array(
    t('Nickname'),
    t('ID'      ),
    t('Zone'    ),
    t('State'   ),
    t('User'    ),
    t('Hostname'),
    //  t('Private-DNS-Name' ) ,
  );


  $filter     = isset($_REQUEST['filter'   ]) ? $_REQUEST['filter'   ] : '';
  $filter_col = isset($_REQUEST['operation']) ? $_REQUEST['operation'] : 0  ; // default: Nickname
  $filter     = trim($filter);

  if ( $filter_col == 0 ) {
    $column  = 'Nickname'            ;
    $sql_col = 'instance_nickname'   ;
  }
  elseif ($filter_col == 1 ) {
    $column  = 'AWS-ID'              ;
    $sql_col = 'c.instance_id'       ;
  }
  elseif ($filter_col == 2 ) {
    $column  = 'Availability Zone'   ;
    $sql_col = 'placement'           ;
  }
  elseif ($filter_col == 3 ) {
    $column  = 'State'               ;
    $sql_col = 'instance_state_name';
  }
  elseif ($filter_col == 4 ) {
    $column  = 'User'                ;
    $sql_col = 'key_name'            ;
  }
  elseif ($filter_col == 5 ) {
    $column  = 'Public-DNS-Name'     ;
    $sql_col = 'dns_name'            ;
  }
  elseif ($filter_col == 5 ) {
    $column  = 'Private-DNS-Name'    ;
    $sql_col = 'private_dns_name'    ;
  }

  if ( isset($filter) ) {

    $query_args[] = $sql_col;
    $query_args[] = $filter  ;
    $query_args[] = $cloud_context;
  }
  else {

    $filter       = ' 1 ';
    $query_args[] = ' '   ;
    $query_args[] = $cloud_context;
  }

  //asort($options);

  $form['options'] = array(
    '#prefix' => '<div class="container-inline">',
    '#type' => 'fieldset',
    '#suffix' => '</div>',
    // '#title'  => t('Filter'),
  );

  $instance_count = _aws_ec2_lib_get_instances_list_count($cloud_context);
  $filter_disabled = '';
  if ( $instance_count < 2 ) {

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
        'data' => t('Nickname'  ),
        'field' => 'instance_nickname'           ,
        //'class' => 'nickname-column',
      ),
      array(
        'data' => t('ID'        ),
        'field' => 'instance_id',
      ),
      array(
        'data' => t('Hostname'  ),
        'field' => 'dns_name',
      ),
      array(
        'data' => t('State'     ),
        'field' => 'instance_state_name',
      ),
      array(
        'data' => t('Zone'      ),
        'field' => 'placement',
        'sort' => 'asc',
      ),
      array(
        'data' => t('User'      ),
        'field' => 'key_name',
      ),
      array(
        'data' => t('Runtime'   ),
        'field' => 'launch_time',
      ),
      array(
        'data' => t('Action'    ),
        'class' => 'action-column',
      ),
    ),
  );

  if (empty($_REQUEST['sort']) && !$filter) {

    $query = _aws_ec2_lib_get_instances_list_manual_sort_query($cloud_context);
    $query->condition('c.cloud_type', $cloud_context, '=');

    $result = $query->execute();

  }
  else {

    $query  = _aws_ec2_lib_get_instances_list_query($cloud_context);
    $query->condition('c.cloud_type', $cloud_context, '=');
    if ( isset($filter) ) {

      $query->condition($sql_col, '%' . $filter . '%', 'like');
    }

    $query
      ->extend('TableSort')
      ->orderByHeader($form['header']['#value']);

    $result = $query->execute();

  }

  $destination = drupal_get_destination();
  $lock_status = _aws_ec2_lib_get_instances_lock_status($cloud_context);

  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  foreach ($result as $instance) {

    $form['Nickname'][$instance->instance_id] = array(
      array('#markup' => l( $instance->instance_nickname,  
            $cloud_menu_path . '/instances/describe/' . $instance->instance_id . '/info',  
            array('query' => array('instance_id' => $instance->instance_id)) )
                                            . (( !empty($lock_status[$instance->instance_id]) && $lock_status[$instance->instance_id] == 'Lock' )
                                            ? '<a class=lockIcon title=Lock>&nbsp;&nbsp;&nbsp;&nbsp;</a>'
                                            : '' )),
    );
    $form['keyUser'][$instance->instance_id] = array(
      array('#markup' => l( $instance->key_name,  
            $cloud_menu_path . '/ssh_keys/describe',  
            array('query' => array('key_name' => $instance->key_name )))),
    );
    $form['hdnName'][$instance->instance_id] = array(
      '#type' => 'hidden',
      '#value' => addslashes($instance->instance_nickname),
    );
    $form['ID'      ][$instance->instance_id] = array(
      array('#markup' => t( substr($instance->instance_id, 0, 10) )),
    );

    if (isset($instance->image_nickname) == FALSE
      ||    empty($instance->image_nickname) ) {
      $instance->image_nickname = $instance->image_id;
    }

    $status     = $instance->instance_state_name;
    $status     = _aws_ec2_lib_get_instance_status($instance->instance_id, $status, $cloud_context );

    $public_dns = substr( $instance->dns_name, 0, strpos($instance->dns_name,    '.') );
    if ( strlen($public_dns) > 3 ) { // truncate only in case the name is long
      $public_dns = substr($instance->dns_name, 0, strpos($instance->dns_name,    '.')) . '...';
    }
    else {

      $public_dns = $instance->dns_name;
    }


    $form['State'     ][$instance->instance_id] = array(array(
        '#type' => 'hidden',
        '#value' => t( $instance->instance_state_name ),
      ));
    $form['Status'    ][$instance->instance_id] = array(array('#markup' => t( ucwords($instance->instance_state_name) )));
    $form['Public_DNS'][$instance->instance_id] = array(array('#markup' => l($public_dns, 'http://' . $instance->dns_name, array('attributes' => array('target' => '_blank'), 'query' => array()) )));
    $form['Zone'      ][$instance->instance_id] = array(array('#markup' => t( $instance->placement)));
    $form['Runtime'   ][$instance->instance_id] = array(array('#markup' => t( cloud_get_runtime($instance->launch_time) )));
    $form['KeyName'   ][$instance->instance_id] = array(
      '#type' => 'hidden',
      '#value' => $instance->key_name,
    );
  }

  $form['filter_hdn'] = array(
    '#type' => 'hidden',
    '#value' => $filter,
  );

  $form['operation_hdn'] = array(
    '#type' => 'hidden',
    '#value' => $filter_col,
  );

  $form['cloud_context'] = array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );
  $form['pager'        ] = array('#value' => theme('pager', array('tags' => NULL, 'element' => 0)));
  $form['#redirect'    ] = FALSE;
  $form['#action'] = url($_REQUEST['q']);

  return $form;
}


/**
 *
 * @param     $launch_time
 * @return
 */
function _aws_ec2_lib_get_launch_time_diff($launch_time) {

  $launch_tmp_time = date_parse($launch_time);
  $gm_launch_time  = gmmktime( $launch_tmp_time['hour'], $launch_tmp_time['minute'], $launch_tmp_time['second'], $launch_tmp_time['month'], $launch_tmp_time['day'], $launch_tmp_time['year'] );
  $curr_time       = gmmktime();
  $diff            = $curr_time - $gm_launch_time;

  return $diff;
}


/**
 *
 * @param     $cloud_context
 * @param     $key_name
 * @param     $key_material
 * @param     $instance_ip
 * @return
 */
function _aws_ec2_lib_create_key_file( $cloud_context, $key_name, $key_material, $instance_ip) {


  if (empty($key_material)) {
    return '';
  }

  $output_dir = file_create_path(AWS_EC2_LIB_TEMP_DIR);

  // if files/cloud directory doesn't exist:
  if (file_exists($output_dir) === FALSE ) {

    mkdir($output_dir, 0777, TRUE );
  }

  // Clean up temporary files
  if ($handle = opendir($output_dir)) {
    while (false !== ($file = readdir($handle))) {
      if ($file != '.'
        &&    $file != '..') {
        $part = split('_', $file); // get timestamp = $part[0]
        if ($part[0] < (REQUEST_TIME - 600) && !is_dir( "{$output_dir}/{$file}")) { // file created before 10 min (600 sec)
          @unlink("$output_dir/$file"); // delete
        }
      }
    }
  }
  closedir($handle);

  $secret_key = cloud_get_info( $cloud_context, 'aws_secret_key' )  ;

  $filename = $output_dir . '/'
            . REQUEST_TIME . '_' . sha1( $_SERVER['HTTP_HOST']
                           . REQUEST_TIME
                           . $key_name
                           . $instance_ip
                           . AWS_EC2_LIB_SEED
                           . $secret_key
                           );

  $fh = fopen($filename, 'w');
  if ( $fh == FALSE) {
    return '';
  }

  fwrite($fh, $key_material );
  fclose($fh);

  return $filename;
}


/**
 *
 * @param     $cloud_context
 * @param     $key_name
 * @param     $key_material
 * @param     $instance_ip
 * @param     $key_file_name
 * @return
 */
function _aws_ec2_lib_get_ssh_key_manager( $cloud_context, $key_name, $key_material, $instance_ip, $key_file_name ) {

  $base_path         = base_path();
  //Key file is no longer generated.  It is now a url call with user access control  
  $file_url = url("clouds/$cloud_context/ssh_keys/key_material/$key_name", array('absolute' => TRUE));
  $cloud_path        = drupal_get_path('module', 'cloud');
  $jar_path = $base_path . $cloud_path . '/CloudSSHKeyManager.jar';
  $keymanager_applet = "<applet archive=\"{$jar_path}\"    code=com.clanavi.CloudSSHKeyManager.class width=0 height=0><param name=file_url value=$file_url><param name=private-key value=$key_file_name></applet>";

  return $keymanager_applet;
}



function _aws_ec2_lib_callback_get_instances_list($cloud_context) {

  drupal_add_html_head_link(array(
    'type' => 'text/css',
    'rel' => 'stylesheet',
    'media' => 'all',
    'href' => base_path()
             . drupal_get_path('module', 'cloud')
             . CLOUD_PATH_SEPARATOR
             . 'css/cloud.css',
  )
  );

  drupal_add_js(cloud_get_module_base() . 'js/cloud.js');

  $filter     = isset($_REQUEST['filter'   ]) ? $_REQUEST['filter'   ] : '';
  $filter_col = isset($_REQUEST['operation']) ? $_REQUEST['operation'] : 0  ; // default: Nickname
  $filter     = trim($filter);

  if ( $filter_col == 0 ) {
    $column  = 'Nickname'            ;
    $sql_col = 'instance_nickname'   ;
  }
  elseif ($filter_col == 1 ) {
    $column  = 'AWS-ID'              ;
    $sql_col = 'c.instance_id'       ;
  }
  elseif ($filter_col == 2 ) {
    $column  = 'Availability Zone'   ;
    $sql_col = 'placement'           ;
  }
  elseif ($filter_col == 3 ) {
    $column  = 'State'               ;
    $sql_col = 'instance_state_name';
  }
  elseif ($filter_col == 4 ) {
    $column  = 'User'                ;
    $sql_col = 'key_name'            ;
  }
  elseif ($filter_col == 5 ) {
    $column  = 'Public-DNS-Name'     ;
    $sql_col = 'dns_name'            ;
  }
  elseif ($filter_col == 5 ) {
    $column  = 'Private-DNS-Name'    ;
    $sql_col = 'private_dns_name'    ;
  }
  $query_args = array();

  if ( isset($filter) ) {

    $query_args[] = $sql_col;
    $query_args[] = $filter  ;
    $query_args[] = $cloud_context;
  }
  else {
    $filter       = ' 1 ';
    $query_args[] = ' '   ;
    $query_args[] = $cloud_context;
  }

  $table_data = array();
  $table_data['header'] = array(
    '#type' => 'value',
    '#value' => array(
      array(
        'data' => t('Nickname'  ),
        'field' => 'instance_nickname'           ,
        //'class' => 'nickname-column',
      ),
      array(
        'data' => t('ID'        ),
        'field' => 'instance_id',
      ),
      array(
        'data' => t('Hostname'  ),
        'field' => 'dns_name',
      ),
      array(
        'data' => t('State'     ),
        'field' => 'instance_state_name',
      ),
      array(
        'data' => t('Zone'      ),
        'field' => 'placement',
        'sort' => 'asc',
      ),
      array(
        'data' => t('User'      ),
        'field' => 'key_name',
      ),
      array(
        'data' => t('Runtime'   ),
        'field' => 'launch_time',
      ),
      array(
        'data' => t('Action'    ),
        'class' => 'action-column',
      ),
    ),
  );
  if (empty($_REQUEST['sort']) && !$filter) {
    $query = _aws_ec2_lib_get_instances_list_manual_sort_query($cloud_context);

    $query->condition('c.cloud_type', $cloud_context, '=');
    $result = $query->execute();

  }
  else {
    $query  = _aws_ec2_lib_get_instances_list_query($cloud_context);
    $query->condition('c.cloud_type', $cloud_context, '=');
    if ( isset($filter) ) {

      $query->condition($sql_col, '%' . $filter . '%', 'like');
    }

    $sort = 'ASC';
    if ($_REQUEST['sort'] == 'asc' ) {

      $sort = 'DESC';
    }

    $query
      ->extend('TableSort')
      ->orderBy($sql_col, $sort);


    $result = $query->execute();

  }

  $lock_status = _aws_ec2_lib_get_instances_lock_status($cloud_context);

  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  foreach ($result as $instance) {

    $table_data['Nickname'][$instance->instance_id] = array(
      array('#markup' => l( $instance->instance_nickname,  
            $cloud_menu_path . '/instances/describe/' . $instance->instance_id . '/info',  
            array('query' => array('instance_id' => $instance->instance_id)) )
                                            . (( !empty($lock_status[$instance->instance_id]) && $lock_status[$instance->instance_id] == 'Lock' )
                                            ? '<a class=lockIcon title=Lock>&nbsp;&nbsp;&nbsp;&nbsp;</a>'
                                            : '' )),
    );
    $table_data['keyUser'][$instance->instance_id] = array(
      array('#markup' => l( $instance->key_name,  
            $cloud_menu_path . '/ssh_keys/describe',  
            array('query' => array('key_name' => $instance->key_name )))),
    );
    $table_data['hdnName'][$instance->instance_id] = array(
      '#type' => 'hidden',
      '#value' => addslashes($instance->instance_nickname),
    );
    $table_data['ID'      ][$instance->instance_id] = array(
      array('#markup' => t( substr($instance->instance_id, 0, 10) )),
    );

    if (isset($instance->image_nickname) == FALSE
      ||    empty($instance->image_nickname) ) {
      $instance->image_nickname = $instance->image_id;
    }

    $status     = $instance->instance_state_name;
    $status     = _aws_ec2_lib_get_instance_status($instance->instance_id, $status, $cloud_context );

    $public_dns = substr( $instance->dns_name, 0, strpos($instance->dns_name,    '.') );
    if ( strlen($public_dns) > 3 ) { // truncate only in case the name is long
      $public_dns = substr($instance->dns_name, 0, strpos($instance->dns_name,    '.')) . '...';
    }
    else {

      $public_dns = $instance->dns_name;
    }

    $table_data['State'     ][$instance->instance_id] = array(array(
        '#type' => 'hidden',
        '#value' => t( $instance->instance_state_name ),
      ));
    $table_data['Status'    ][$instance->instance_id] = array(array('#markup' => t( ucwords($instance->instance_state_name) )));
    $table_data['Public_DNS'][$instance->instance_id] = array(array('#markup' => l($public_dns, 'http://' . $instance->dns_name, array('attributes' => array('target' => '_blank'), 'query' => array()) )));
    $table_data['Zone'      ][$instance->instance_id] = array(array('#markup' => t( $instance->placement)));
    $table_data['Runtime'   ][$instance->instance_id] = array(array('#markup' => t( cloud_get_runtime($instance->launch_time) )));
    $table_data['KeyName'   ][$instance->instance_id] = array(
      '#type' => 'hidden',
      '#value' => $instance->key_name,
    );
  }


  // The theme for displaying the table
  $lock_status   =  _aws_ec2_lib_get_instances_lock_status($cloud_context);
  $user_keys_all = aws_ec2_lib_get_all_keys($cloud_context);

  $output           = '';
  $cloud_menu_path  = cloud_get_menu_path($cloud_context );

  $prop = array();
  $prop_monitor = array();
  $prop_not_monitor = array();

  $rows = array();
  $nickname_arr = isset($table_data['Nickname']) ? $table_data['Nickname'] : array();

  foreach (element_children($nickname_arr) as $key) {

    $lock_image = '';

    if ( !empty($lock_status[$key]) && $lock_status[$key] == 'Lock') {
      $src_img_path = 'images/lock';
      $path         = cloud_get_module_base() . $src_img_path;
      $image_info   = image_get_info( $path );

      $msg = '';
      $attributes = array();
      $attributes['class'] = 'image image-$label ' . (isset($attributes['class']) ? $attributes['class'] : '');
      $attributes['width'] = $image_info['width'];
      $attributes['height'] = $image_info['height'];
      $out = theme( 'image', array(
        'path' => $path,
        'alt' => $msg,
        'title' => $msg,
        'width' => $image_info['width'],
        'height' => $image_info['height'],
        'attributes' => array('class' => $attributes['class']),
      ));
    }

    $row = array(
      array(
        'data' => drupal_render($table_data['Nickname'  ][$key]) . $lock_image,
        //'class' => 'nickname-column',
      ),
      drupal_render($table_data['ID'        ][$key]),
      drupal_render($table_data['Public_DNS'][$key]),
      drupal_render($table_data['Status'    ][$key]),
      drupal_render($table_data['Zone'      ][$key]),
      drupal_render($table_data['keyUser'   ][$key]),
      drupal_render($table_data['Runtime'   ][$key]),
    );

    $prop[            'onclick'] = cloud_get_messagebox('Are you sure you want to terminate the instance "' . trim($table_data['hdnName'][$key]['#value']) . '" ?');
    $prop_reboot[     'onclick'] = cloud_get_messagebox('Are you sure you want to reboot the instance "' . trim($table_data['hdnName'][$key]['#value']) . '" ?');
    $prop_monitor[    'onclick'] = cloud_get_messagebox('Are you sure you want to monitor the instance "' . $table_data['hdnName'][$key]['#value'] . '" ?');
    $prop_not_monitor['onclick'] = cloud_get_messagebox('Are you sure you want turn off monitoring the instance "' . $table_data['hdnName'][$key]['#value'] . '" ?');

    $action_data = '';
    $action_data .= cloud_display_action( 'images/icon_clear', t('Edit'), $cloud_menu_path . '/instances/describe/' . $key . '/info',    array('query' => array('instance_id' => $key), 'html' => TRUE) );

    if ( $table_data['State'][$key][0]['#value'] != 'terminated' && $table_data['State'][$key][0]['#value'] != 'stopped' ) {
      //monitoring actions
      if (isset($lock_status[$key . '_monitor']) == FALSE
        ||          $lock_status[$key . '_monitor']  == 0 ) {
        // Monitoring disabled
        //$action_data .= cloud_display_action( 'images/icon_monitor', 'Monitor' , $cloud_menu_path . '/instances/describe/' . $key . '/monitor' ,  array('query' => 'instance_id=' . urlencode($key) . '&name=' . urlencode($form['Nickname'][$key][0]['#value']) . "&action=Monitor",  'html' => TRUE));
      }
      elseif (isset($lock_status[$key . '_monitor']) == FALSE
        ||              $lock_status[$key . '_monitor']  == 1 ) {
        /*$action_data .= cloud_display_action(
         'images/icon_monitor', 'Monitor Enabled'         ,
         $cloud_menu_path . '/instances/describe/' . $key . '/monitor' ,
         array('query' => 'instance_id=' . urlencode($key) . '&name=' . urlencode($form['Nickname'][$key][0]['#value']) . "&action=Monitor",
         'html' => TRUE)
         ); */

        // Monitoring disabled
      }
      //end
      $arr = array();
      $result         = aws_ec2_lib_get_key_material_by_id($cloud_context, $key);
      $key_file_name  = bin2hex($result[0]);
      $instance_ip    = $result[2];
      $key_material   = $result[1];

      $console_url = drupal_get_path('module', 'cloud') . '/cloud_console.inc';

      $console_data = '<H1>Use the Console window</H1>';
      $close_window = "<input type=button value=\"Close Window\" onClick=\"javascript:window.close();\">";

      $base_path = base_path();

      $key_name  = $table_data['KeyName'][$key]['#value'];

      $ssh_user_name = aws_ec2_lib_get_ssh_user( $cloud_context, $key );

      $keymanager_applet = _aws_ec2_lib_get_ssh_key_manager( $cloud_context, $key_name, $key_material, $instance_ip  , $key_file_name );

      $key_applet = "<applet archive=\"" . $base_path . AWS_EC2_LIB_SSH_CLIENT_JAR_FILE . "\" code=com.mindbright.application.MindTerm.class width=0 height=0><param name=server value=$instance_ip><param name=username value=$ssh_user_name><param name=port value=22><param name=sepframe value=true><param name=private-key value=$key_file_name><param name=auth-method value=publickey><param name=quiet value=true><param name=verbose value=true><param name=debug value=true><param name=autoprops value=both><param name=geometry value=\"100x35\"> </applet>";

      $arr['onclick'] = 'popWin = open(\'' . $console_url . '\', \'Console\', \'width=300, height=300\');  popWin.focus();
      popWin.document.write(\'' . $console_data . $close_window . $keymanager_applet . $key_applet . '\') ; return false ';
      if ( empty($key_material) ) {

        $key_user_name = $table_data['KeyName'][$key]['#value'];
        $err_msg       = 'popWin = open(\'' . $console_url . '\', \'Console\', \'width=300, height=300\');  popWin.focus();
        popWin.document.write(\' Key not found for the user <b>' . $key_user_name . '. </b> Please set the key first.\') ; return false ';
        $action_data .= cloud_display_action( 'images/icon_ssh', 'Console', $console_url,    array('query' => array('id' => $key, 'instanceId' => $key, 'keyFileName' => $key_file_name, 'instanceIp' => $result[2], 'keyMaterial' => url('sites/all/modules/cloud/tmp/abc.txt')), 'html' => TRUE), $err_msg );
      }
      elseif (user_access($cloud_context . ' access all console')) {
        $action_data .= cloud_display_action( 'images/icon_ssh', 'Console', $console_url,    array('query' => array('id' => $key, 'instanceId' => $key, 'keyFileName' => $key_file_name, 'instanceIp' => $result[2], 'keyMaterial' => url('sites/all/modules/cloud/tmp/abc.txt')), 'html' => TRUE), $arr['onclick']); //,   '' , $arr )  ;
      }
      elseif (user_access($cloud_context . ' access own console')) {
        if (in_array($table_data['KeyName'][$key]['#value'], $user_keys_all)) {
          $action_data .= cloud_display_action( 'images/icon_ssh', 'Console', $console_url,    array('query' => array('id' => $key), 'html' => TRUE), $arr['onclick']); //,   '' , $arr )  ;
        }
      }
    }

    if ( $table_data['State'][$key][0]['#value'] != 'terminated' && $table_data['State'][$key][0]['#value'] != 'stopped' ) {

      $inst_name =  trim($table_data['hdnName'][$key]['#value']);
      if (isset($lock_status[$key]) == FALSE || $lock_status[$key] == 'Unlock') {
        if (user_access($cloud_context . ' terminate all instances') ) {
          $action_data .= cloud_display_action( 'images/icon_reboot', 'Reboot', $cloud_menu_path . '/instances/reboot',    array('query' => array('instance_id' => $key, 'name' => $inst_name), 'html' => TRUE), $prop_reboot['onclick']);
          $action_data .= cloud_display_action( 'images/action_stop', 'Terminate', $cloud_menu_path . '/instances/terminate',    array('query' => array('instance_id' => $key, 'name' => $inst_name), 'html' => TRUE), $prop['onclick']); //,   'instance_id=' . urlencode($key) . '&name=' . $inst_name  , $prop );
        }
        elseif (user_access($cloud_context . ' terminate own instance')) {
          if (in_array($table_data['KeyName'][$key]['#value'], $user_keys_all)) {

            $action_data .= cloud_display_action( 'images/icon_reboot', 'Reboot', $cloud_menu_path . '/instances/reboot',    array('query' => array('instance_id' => $key, 'name' => $inst_name), 'html' => TRUE), $prop_reboot['onclick']);
            $action_data .= cloud_display_action( 'images/action_stop', 'Terminate', $cloud_menu_path . '/instances/terminate',    array('query' => array('instance_id' => $key, 'name' => $inst_name)  , 'html' => TRUE), $prop['onclick']); //,   'instance_id=' . urlencode($key) . '&name=' . urlencode($form['Nickname'][$key][0]['#value']) , $prop );
          }
        }
      }
    }
    else {
      $action_data = '';
    }

    $row[]  =  array(
      'data' => $action_data,
      'class' => 'action-column',
    );
    $rows[] = $row;
  }

  $table_attr = array();
  $table_attr['id'] = 'instances_list_table';
  $table_attr['autoupdate_url'] = url( $cloud_menu_path . '/callback_get_instances_list' );
  $output .= theme('table', array('header' => $table_data['header']['#value'], 'rows' => $rows, 'attributes' => $table_attr));

  // Send only the body do not send the headers
  $index_start = strrpos( $output, '<tbody>' );
  $index_end   = strrpos( $output, '</tbody>' );
  if ( isset($table_data['Nickname']) === FALSE || sizeof($table_data['Nickname']) == 0 ) { // No element present
    $output = 'NULL';
  }
  else {

    $output = substr( $output, $index_start, $index_end - $index_start );
    $output .= '</tbody>';
  }

  $html = $output;

  print drupal_json_encode(array('html' => $html));

  // The exit() call is critical!
  exit();
}

/**
 *
 * @param     $form
 * @return
 */
function theme_aws_ec2_lib_display_instances_list($form) {


  if (isset($form['form'])) { // drupal 7 passed form as an array 
    $form = $form['form'];
  }
  $cloud_context = $form['cloud_context']['#value'];
  $lock_status   =  _aws_ec2_lib_get_instances_lock_status($cloud_context);
  $user_keys_all = aws_ec2_lib_get_all_keys($cloud_context);

  $output           = drupal_render($form['options']);
  $cloud_menu_path  = cloud_get_menu_path($cloud_context );
  $form['Nickname'] = !empty($form['Nickname']) ? $form['Nickname'] : array();
  $form['pager']['#value'] = !empty($form['pager']['#value']) ? $form['pager']['#value'] : '';

  $rows = array();
  foreach (element_children($form['Nickname']) as $key) {

    $lock_image = '';

    if ( !empty($lock_status[$key]) && $lock_status[$key] == 'Lock') {
      $src_img_path = 'images/lock';
      $path         = cloud_get_module_base() . $src_img_path;
      $image_info   = image_get_info( $path );

      $msg = '';
      $attributes['class'] = 'image image-$label ' . (isset($attributes['class']) ? $attributes['class'] : '');
      $attributes['width'] = $image_info['width'];
      $attributes['height'] = $image_info['height'];
      $out = theme( 'image', array(
        'path' => $path,
        'alt' => $msg,
        'title' => $msg,
        'width' => $image_info['width'],
        'height' => $image_info['height'],
        'attributes' => array('class' => $attributes['class']),
      ));
    }

    $row = array(
      array(
        'data' => drupal_render($form['Nickname'  ][$key]) . $lock_image,
        //'class' => 'nickname-column',
      ),
      drupal_render($form['ID'        ][$key]),
      drupal_render($form['Public_DNS'][$key]),
      drupal_render($form['Status'    ][$key]),
      drupal_render($form['Zone'      ][$key]),
      drupal_render($form['keyUser'   ][$key]),
      drupal_render($form['Runtime'   ][$key]),
    );

    $prop_reboot[     'onclick'] = cloud_get_messagebox('Are you sure you want to reboot the instance "' . trim($form['hdnName'][$key]['#value']) . '" ?');
    $prop[            'onclick'] = cloud_get_messagebox('Are you sure you want to terminate the instance "' . trim($form['hdnName'][$key]['#value']) . '" ?');
    $prop_monitor[    'onclick'] = cloud_get_messagebox('Are you sure you want to monitor the instance "' . $form['hdnName'][$key]['#value'] . '" ?');
    $prop_not_monitor['onclick'] = cloud_get_messagebox('Are you sure you want turn off monitoring the instance "' . $form['hdnName'][$key]['#value'] . '" ?');

    $action_data = '';
    $action_data .= cloud_display_action( 'images/icon_clear', t('Edit'), $cloud_menu_path . '/instances/describe/' . $key . '/info',    array('query' => array('instance_id' => $key), 'html' => TRUE) );

    if ( $form['State'][$key][0]['#value'] != 'terminated' && $form['State'][$key][0]['#value'] != 'stopped' ) {
      //monitoring actions  
      if (isset($lock_status[$key . '_monitor']) == FALSE
        ||          $lock_status[$key . '_monitor']  == 0 ) {
        // Monitoring disabled
        //$action_data .= cloud_display_action( 'images/icon_monitor', 'Monitor' , $cloud_menu_path . '/instances/describe/' . $key . '/monitor' ,  array('query' => 'instance_id=' . urlencode($key) . '&name=' . urlencode($form['Nickname'][$key][0]['#value']) . "&action=Monitor",  'html' => TRUE));
      }
      elseif (isset($lock_status[$key . '_monitor']) == FALSE
        ||              $lock_status[$key . '_monitor']  == 1 ) {
        /*$action_data .= cloud_display_action(
         'images/icon_monitor', 'Monitor Enabled'         ,
         $cloud_menu_path . '/instances/describe/' . $key . '/monitor' ,
         array('query' => 'instance_id=' . urlencode($key) . '&name=' . urlencode($form['Nickname'][$key][0]['#value']) . "&action=Monitor",
         'html' => TRUE)
         ); */

        // Monitoring disabled                
      }
      //end
      $arr = array();
      $result         = aws_ec2_lib_get_key_material_by_id($cloud_context, $key);
      $key_file_name  = bin2hex($result[0]);
      $instance_ip    = $result[2];
      $key_material   = $result[1];

      $console_url = drupal_get_path('module', 'cloud') . '/cloud_console.inc';

      $console_data = '<H1>Use the Console window</H1>';
      $close_window = "<input type=button value=\"Close Window\" onClick=\"javascript:window.close();\">";

      $base_path = base_path();

      $key_name  = $form['KeyName'][$key]['#value'];

      $keymanager_applet = _aws_ec2_lib_get_ssh_key_manager( $cloud_context, $key_name, $key_material, $instance_ip  , $key_file_name );

      $ssh_user_name = aws_ec2_lib_get_ssh_user( $cloud_context, $key );

      $key_applet = "<applet archive=\"" . $base_path . AWS_EC2_LIB_SSH_CLIENT_JAR_FILE . "\" code=com.mindbright.application.MindTerm.class width=0 height=0><param name=server value=$instance_ip><param name=username value=$ssh_user_name><param name=port value=22><param name=sepframe value=true><param name=private-key value=$key_file_name><param name=auth-method value=publickey><param name=quiet value=true><param name=verbose value=true><param name=debug value=true><param name=autoprops value=both><param name=geometry value=\"100x35\"> </applet>";

      $arr['onclick'] = 'popWin = open(\'' . $console_url . '\', \'Console\', \'width=300, height=300\');  popWin.focus();
      popWin.document.write(\'' . $console_data . $close_window . $keymanager_applet . $key_applet . '\') ; return false ';
      if ( empty($key_material) ) {

        $key_user_name = $form['KeyName'][$key]['#value'];
        $err_msg       = 'popWin = open(\'' . $console_url . '\', \'Console\', \'width=300, height=300\');  popWin.focus();
        popWin.document.write(\' Key not found for the user <b>' . $key_user_name . '. </b> Please set the key first.\') ; return false ';
        $action_data .= cloud_display_action( 'images/icon_ssh', 'Console', $console_url,    array('query' => array('id' => $key, 'instanceId' => $key, 'keyFileName' => $key_file_name, 'instanceIp' => $result[2], 'keyMaterial' => url('sites/all/modules/cloud/tmp/abc.txt')), 'html' => TRUE), $err_msg );
      }
      elseif (user_access($cloud_context . ' access all console')) {
        $action_data .= cloud_display_action( 'images/icon_ssh', 'Console', $console_url,    array('query' => array('id' => $key, 'instanceId' => $key, 'keyFileName' => $key_file_name, 'instanceIp' => $result[2], 'keyMaterial' => url('sites/all/modules/cloud/tmp/abc.txt')), 'html' => TRUE), $arr['onclick']); //,   '' , $arr )  ;
      }
      elseif (user_access($cloud_context . ' access own console')) {
        if (in_array($form['KeyName'][$key]['#value'], $user_keys_all)) {
          $action_data .= cloud_display_action( 'images/icon_ssh', 'Console', $console_url,    array('query' => array('id' => $key), 'html' => TRUE), $arr['onclick']); //,   '' , $arr )  ;
        }
      }
    }

    if ( $form['State'][$key][0]['#value'] != 'terminated' && $form['State'][$key][0]['#value'] != 'stopped' ) {

      $inst_name =  trim($form['hdnName'][$key]['#value']);
      if (isset($lock_status[$key]) == FALSE || $lock_status[$key] == 'Unlock') {
        if (user_access($cloud_context . ' terminate all instances') ) {
          $action_data .= cloud_display_action( 'images/icon_reboot', 'Reboot'    , $cloud_menu_path . '/instances/reboot',    array('query' => array('instance_id' => $key, 'name' => $inst_name), 'html' => TRUE), $prop_reboot['onclick']);
          $action_data .= cloud_display_action( 'images/action_stop', 'Terminate', $cloud_menu_path . '/instances/terminate',    array('query' => array('instance_id' => $key, 'name' => $inst_name), 'html' => TRUE), $prop['onclick']); //,   'instance_id=' . urlencode($key) . '&name=' . $inst_name  , $prop );
        }
        elseif (user_access($cloud_context . ' terminate own instance')) {
          if (in_array($form['KeyName'][$key]['#value'], $user_keys_all)) {
            $action_data .= cloud_display_action( 'images/icon_reboot', 'Reboot'    , $cloud_menu_path . '/instances/reboot',    array('query' => array('instance_id' => $key, 'name' => $inst_name), 'html' => TRUE), $prop_reboot['onclick']);
            $action_data .= cloud_display_action( 'images/action_stop', 'Terminate', $cloud_menu_path . '/instances/terminate',    array('query' => array('instance_id' => $key, 'name' => $inst_name)  , 'html' => TRUE), $prop['onclick']); //,   'instance_id=' . urlencode($key) . '&name=' . urlencode($form['Nickname'][$key][0]['#value']) , $prop );
          }
        }
      }
    }
    else {
      $action_data = '';
    }

    $row[]  =  array(
      'data' => $action_data,
      'class' => 'action-column',
    );
    $rows[] = $row;
  }

  $table_attr = array();
  $table_attr['id'] = 'instances_list_table';
  $table_attr['autoupdate_url'] = url( $cloud_menu_path . '/callback_get_instances_list' );
  $output .= theme('table', array('header' => $form['header']['#value'], 'rows' => $rows, 'attributes' => $table_attr));

  $output .= drupal_render($form['cloud_context']);
  $output .= drupal_render($form['filter_hdn']);
  $output .= drupal_render($form['operation_hdn']);
  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $destination = drupal_get_destination();
  $reload_link = l( t('- Refresh Page -'), $cloud_menu_path . '/get_instances_data',    array('query' => $destination) );
  $ref_link    = array(
    '#type' => 'item',
    '#prefix' => '<div id="link_reload" align="right">',
    '#markup' => $reload_link,
    '#suffix' => '</div>',
  );
  $output .= drupal_render($ref_link);
  $output .= drupal_render_children($form);

  return $output;
}


/**
 *
 * @param     $form_id
 * @param     $form_values
 * @return
 */
function aws_ec2_lib_display_instances_list_submit($form_id, &$form_values) {

  $form_values   = $form_values['values'       ];
  $cloud_context = $form_values['cloud_context'];

  if ( $form_values['op'] == t('Apply') ) {

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
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_reboot_instance($cloud_context, $params = array()) {

  if (empty($params) ) {

    $instance_id   = isset( $_REQUEST['instance_id'  ] ) ? $_REQUEST['instance_id'  ] : FALSE;
  }
  else {

    $instance_id   = $params[0];
  }

  $instance_name = isset( $_REQUEST['name'         ] ) ? $_REQUEST['name'         ] : ''    ;

  $reboot_instances = array(
    $instance_id,
  );

  $instance_info = _aws_ec2_lib_get_instance_details_by_id($cloud_context, $instance_id);
  if ( empty($instance_info) === FALSE ) {

    $instance_name = $instance_info['name'];
  }

  $result = aws_ec2_api_reboot_instances($cloud_context, $reboot_instances);
  if ($result->code == 200 ) {

    drupal_set_message(  t('Instance rebooting: @instance_name. This will take some time.', array('@instance_name' => $instance_name) ) );
  }
  else {

    drupal_set_message(  t('There was a problem while trying to reboot the instance: @instance_name', array('@instance_name' => $instance_name) ),    'error' );
  }
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if (empty($params) ) { // Came from instances page
    drupal_goto($cloud_menu_path . '/instances');
  }

  return;
}


/**
 *
 * @param     $cloud_context
 * @return
 */

function aws_ec2_lib_terminate_instance_using_params($cloud_context, $params = array()) {

  $instance_id = $params['instance_id'];

  $terminate_instances = array(
    $instance_id,
  );

  $result = aws_ec2_api_terminate_instances($cloud_context, $terminate_instances);
  if ($result->code == 200 ) {

    aws_ec2_lib_save_terminate_instance($cloud_context, $result, $instance_id);
    //$dns_name = cloud_get_dns_name($instance_id, $cloud_context);
    //cloud_snmp_update_entry($cloud_context , 'remove', $dns_name);
  }

  return $result;
}
/**
 *
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_terminate_instance($cloud_context, $params = array()) {

  if (empty($params) ) {

    $instance_id = isset( $_REQUEST['instance_id'  ] ) ? $_REQUEST['instance_id'] : FALSE;
  }
  else {

    $instance_id = $params[0];
  }

  $instance_name = isset( $_REQUEST['name'         ] ) ? $_REQUEST['name'       ] : ''    ;

  $terminate_instances = array(
    $instance_id,
  );

  $instance_info = _aws_ec2_lib_get_instance_details_by_id($cloud_context, $instance_id);
  if ( empty($instance_info) === FALSE ) {

    $instance_name = $instance_info['name'];
  }
  if (module_exists('cloud_failover')) {

    $fail_over_flag = cloud_failover_get_failover_flag($cloud_context, $instance_id);

    if ($fail_over_flag != CLOUD_FAILOVER_FLAG_NO_FAILOVER) {

      drupal_set_message(  t('You cannot terminate a server while failover enabled: @instance_name', array('@instance_name' => $instance_name) ),    'error' );
      $cloud_menu_path = cloud_get_menu_path($cloud_context );

      if (empty($params) ) { // Came from instances page
        drupal_goto($cloud_menu_path . '/instances');
      }
    }


  }
  $result = aws_ec2_api_terminate_instances($cloud_context, $terminate_instances);
  if ($result->code == 200 ) {

    aws_ec2_lib_save_terminate_instance($cloud_context, $result, $instance_id);
    //adding a delete hook for other modules to do something
    module_invoke_all('aws_ec2_terminate_instance', $cloud_context, $instance_id);
    drupal_set_message(  t('Instance terminating: @instance_name', array('@instance_name' => $instance_name) ) );
    $dns_name = cloud_get_dns_name($instance_id, $cloud_context);
    cloud_snmp_update_entry($cloud_context, 'remove', $dns_name);
  }
  else {

    drupal_set_message(  t('There was a problem while trying to terminate the instance: @instance_name', array('@instance_name' => $instance_name) ),    'error' );
  }
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if (empty($params) ) { // Came from instances page
    drupal_goto($cloud_menu_path . '/instances');
  }

  return;
}


/**
 *
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_monitor_instance($cloud_context) {

  $instance_id   = isset( $_REQUEST['instance_id'] ) ? $_REQUEST['instance_id'] : FALSE;
  $instance_name = isset( $_REQUEST['name'       ] ) ? $_REQUEST['name'       ] : ''    ;
  $action        = isset( $_REQUEST['action'     ] ) ? $_REQUEST['action'     ] : ''    ;

  if ($action == 'Monitor') {

    $script_id = cloud_script_operation($cloud_context, 'get_master_script_id', array('script_type' => 'MONITOR_SCRIPT') )  ;

    if ( empty($script_id) || $script_id == '' ) {

      drupal_set_message( t('Master script not set fo Monitor. Pelase set it first') );
      $cloud_menu_path = cloud_get_menu_path($cloud_context );
      drupal_goto($cloud_menu_path . '/instances');
      return;
    }

    $check_record_query = _cloud_scripting_get_by_script_id_instance_id();
    $check_record_query_args = array(
      ':instance_id' => $instance_id,
      ':script_id' => $script_id,
    );

    $result_check_record = db_query( $check_record_query, $check_record_query_args );

    $found = FALSE;

    foreach ($result_check_record as $result_data) {

      $found = TRUE;
    }

    // If the record is not found then insert it in the database so that script will be executed
    if ($found == FALSE ) {

      _cloud_scripting_register_script_save($instance_id, $script_id,    '0', $cloud_context,    '' );
    }

    aws_ec2_lib_instance_monitoring_status_update($cloud_context, $instance_id,    1);

    drupal_set_message(  check_plain(t('Monitoring turned on for instance $instance_name.  This will require some time.') ) );

  }
  else {

    drupal_set_message(  check_plain( t('Monitoring turned OFF for instance $instance_name. This will require some time.') ) );
    aws_ec2_lib_instance_monitoring_status_update($cloud_context, $instance_id,    0);
  }

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  drupal_goto($cloud_menu_path . '/instances');

  return;
}


// Describe the Instances
/**
 *
 * @param     $cloud_context
 * @param     $instance_id
 * @return
 */
function aws_ec2_lib_get_elastic_ips($cloud_context, $instance_id) {

  $resp = array();

  // Get the available Elastic IP values
  $elastic_ip_options = array(
    AWS_EC2_LIB_CLOUD_NONE => AWS_EC2_LIB_CLOUD_NONE,
  );
  $default_rec  = AWS_EC2_LIB_CLOUD_NONE;
  $default_ip   = '';


  $query        = _aws_ec2_lib_get_describe_elastic_ips_query($cloud_context);
  $query->condition('c.cloud_type', $cloud_context, '=');
  $result       = $query->execute();

  foreach ($result as $elastic_ip) {

    if (empty($instance_id) == FALSE && $elastic_ip->instance_id == $instance_id ) {

      $default_rec     =  'org_value';

      if ( $elastic_ip->public_ip_name == '- changeme -') {
        $elastic_ip_options['org_value'] = $elastic_ip->public_ip;
      }

      elseif ( $elastic_ip->public_ip_name === $elastic_ip->public_ip ) {
        $elastic_ip_options['org_value'] = $elastic_ip->public_ip;
      }
      else {
        $elastic_ip_options['org_value'] = $elastic_ip->public_ip_name . ' (' . $elastic_ip->public_ip . ')';
      }

      $default_ip      =  $elastic_ip->public_ip;
    }
    elseif ( strlen($elastic_ip->instance_id) == 0 || $elastic_ip->instance_id == 'nobody' ) {
      if ( $elastic_ip->public_ip_name == '- changeme -') {
        $elastic_ip_options[$elastic_ip->public_ip] = $elastic_ip->public_ip;
      }
    }

    elseif ( $elastic_ip->public_ip_name === $elastic_ip->public_ip ) {
      $elastic_ip_options[$elastic_ip->public_ip] = $elastic_ip->public_ip;
    }
    else {
      $elastic_ip_options[$elastic_ip->public_ip] = $elastic_ip->public_ip_name . ' (' . $elastic_ip->public_ip . ')';
    }
  }
  //asort($elastic_ip_options);
  $resp[0] = $elastic_ip_options;
  $resp[1] = $default_rec        ;
  $resp[2] = $default_ip         ;

  return $resp;
}


/**
 *
 * @param     $cloud_context
 * @param     $instance_id
 * @return
 */
function aws_ec2_lib_get_lock_status($cloud_context, $instance_id) {

  $lock_query   = _aws_ec2_lib_get_lock_status_query($cloud_context);
  $query_args = array(
    ':instance_id' => $instance_id,
    ':cloud_type' => $cloud_context,
  );

  $lock_status = 'Unlock';

  $result      = db_query( $lock_query, $query_args );
  $count       = 0;


  foreach ($result as $key) {

    $lock_status = $key->is_locked;
  }

  //asort($lock_status);
  return $lock_status;
}


/**
 *
 * @param     $cloud_context
 * @param     $instance_id
 * @return
 */
function aws_ec2_lib_get_ssh_user($cloud_context, $instance_id) {

  $ssh_user_query   = _aws_ec2_lib_get_lock_status_query($cloud_context);
  $query_args = array(
    ':instance_id' => $instance_id,
    ':cloud_type' => $cloud_context,
  );

  $ssh_user_name = CLOUD_SSH_USER_NAME;

  $result = db_query( $ssh_user_query, $query_args );


  foreach ($result as $key) {

    $ssh_user_name = $key->ssh_user_name;
  }

  $ssh_user_name = empty( $ssh_user_name ) ? CLOUD_SSH_USER_NAME : $ssh_user_name  ;
  return $ssh_user_name;
}

/**
 * Display instance information.
 * This will display the details of an instance
 *
 * @param     $form_submit
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_display_instance_info($form_submit = '', $form_state, $cloud_context, $instance_id) {

  // $instance_id = _aws_ec2_lib_get_instance_id($cloud_context) ;

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  if (empty($instance_id) || strlen($instance_id) == 0  ) {

    drupal_goto( $cloud_menu_path . '/instances');

    return;
  }

  //$_SESSION['instance_id_desc_info'] = $instance_id;

  $query = _aws_ec2_lib_get_instance_query($cloud_context);
  $query_args = array(
    ':instance_id' => $instance_id,
    ':cloud_type' => $cloud_context,
  );


  $result = db_query( $query, $query_args )->fetchObject();
  $count = 0;

  $form['instance_info_header'] = array(
    '#prefix' => '<h2>',
    '#markup' => t('Instance Info'),
    '#suffix' => '</h2>',
  );

  $form['fieldset_instance_info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Details'),
  );

  $key = NULL;

  if ( $result != NULL ) {

    $key = $result;
  }

  if ( $key ) {
    $instance_state_name = $key->instance_state_name;

    $instance_nickname   = cloud_get_filter_value( $form_submit, 'Name_text', $key->instance_nickname );
    $key_name            = $key->key_name;
    $lock_status = aws_ec2_lib_get_lock_status($cloud_context, $key->instance_id);
    $form['Instance_Name']['Name_label'] = array(
      '#type' => 'item',
      '#title' => t('Nickname'),
    );
    $form['Instance_Name_details'      ] = array(
      '#prefix' => '<div class="container-inline">',
      '#suffix' => '</div>',
    );

    if (user_access($cloud_context . ' update instance details') && $lock_status == 'Unlock' ) {

      $form['Instance_Name_details']['Name_text'] =  array(
        '#type' => 'textfield',
        '#value' => t( $instance_nickname),
      );
    }
    else {
      $form['Instance_Name_details']['Name_text'] =  array(
        '#type' => 'textfield',
        '#value' => t( $instance_nickname),
        '#disabled' => TRUE,
      );
    }

    if ($lock_status == 'Lock') {
      $form['Instance_Name_details']['Name_text']['#attributes'] = array('disabled' => 'disabled');
    }

    $form['Instance_State'  ]['State_label'  ] = array(
      '#type' => 'item',
      '#title' => t('State'),
    );
    $form['Instance_State'  ]['State_text'   ] = array(array('#markup' => t( ucwords($key->instance_state_name ) )));

    $form['Instance_pub_dns']['pub_dns_label'] = array(
      '#type' => 'item',
      '#title' => t('Public DNS Name'),
    );
    $form['Instance_pub_dns']['pub_dns_text'] = array(array('#markup' => l(t( $key->dns_name),    'http://' . $key->dns_name,    array('attributes' => array('target' => '_blank'), 'query' => array()) )));

    $form['Instance_pvt_dns']['pvt_dns_label'] = array(
      '#type' => 'item',
      '#title' => t('Private DNS Name'),
    );
    $form['Instance_pvt_dns']['pvt_dns_text'] = array(array('#markup' => t( $key->private_dns_name)));

    $elastic_ip_options = aws_ec2_lib_get_elastic_ips($cloud_context, $key->instance_id);

    $form['Instance_elastic_ip']['elastic_ip_label'] = array(
      '#type' => 'item',
      '#title' => t('Current Elastic IP'),
    );
    $form['Instance_elastic_ip_details'] = array(
      '#prefix' => '<div class="container-inline">',
      '#suffix' => '</div>',
    );

    if (user_access($cloud_context . ' assign IP') && $lock_status == 'Unlock' ) {

      $form['Instance_elastic_ip_details']['elastic_ip_text'] = array(
        '#type' => 'select',
        '#options' => $elastic_ip_options[0],
        '#default_value' => $elastic_ip_options[1],
      );
    }
    else {

      $form['Instance_elastic_ip_details']['elastic_ip_text'] = array(
        '#type' => 'select',
        '#options' => $elastic_ip_options[0],
        '#default_value' => $elastic_ip_options[1],
        '#disabled' => TRUE,
      );
    }

    if ($lock_status == 'Lock') {

      $form['Instance_elastic_ip_details']['elastic_ip_text']['#attributes'] = array('disabled' => 'disabled');
    }

    $form['Instance_elastic_ip']['elastic_ip_text_default_val'] = array(
      '#type' => 'hidden',
      '#value' => $elastic_ip_options[1],
    );
    $form['Instance_elastic_ip']['elastic_ip_text_default_ip'] = array(
      '#type' => 'hidden',
      '#value' => $elastic_ip_options[2],
    );

    // $form['Instance_EC2_info']['EC2_info_label']       = array('#prefix' =>  '<h3>', '#type' => 'item', '#title' => t('Ec2 Info') ,   '#suffix' =>  '</h3>'  ); 
    // $form['Instance_EC2_info']['EC2_info_label']       = array(   '#type' => 'item', '#title' => t('Ec2 Info') );
    // $form['Instance_EC2_info']['EC2_info_text']       = array( array('#value' => t( '') ) );

    $lock_options =  array(
      'Lock' => 'Lock'  ,
      'Unlock' => 'Unlock',
    );

    $form['Instance_lock']['lock_label'] = array(
      '#type' => 'item',
      '#title' => t('Safety Status'),
    );
    $form['Instance_lock_details'      ] = array(
      '#prefix' => '<div class="container-inline">',
      '#suffix' => '</div>',
    );

    if (user_access($cloud_context . ' update instance details') ) {

      $form['Instance_lock_details']['lock_text'] = array(
        '#type' => 'select',
        '#options' => $lock_options,
        '#default_value' => $lock_status,
      );
    }
    else {

      $form['Instance_lock_details']['lock_text'] = array(
        '#type' => 'select',
        '#options' => $lock_options,
        '#default_value' => $lock_status  ,
        '#disabled' => TRUE,
      );
    }


    $form['Instance_aws']['aws_label'] = array(
      '#type' => 'item',
      '#title' => t('ID'),
    );
    $form['Instance_aws']['aws_text'] = array(array('#markup' => t( $key->instance_id )));

    $form['user_name_details'      ] = array(
      '#prefix' => '<div class="container-inline">',
      '#suffix' => '</div>',
    );

    $form['user_name_details']['user_name_label'] = array(
      '#type' => 'item'      ,
      '#title' => t('User Name'),
    );
    $user_name = aws_ec2_lib_get_ssh_user($cloud_context, $key->instance_id);
    $user_name = cloud_get_filter_value( $form_submit, 'user_name_text', $user_name );

    if (user_access($cloud_context . ' update instance details') && $lock_status == 'Unlock' ) {

      $form['user_name_details']['user_name_text']  = array(
        '#type' => 'textfield',
        '#value' => $user_name,
        '#size' => 32,
      );
    }
    else {

      $form['user_name_details']['user_name_text']  = array(
        '#type' => 'textfield',
        '#value' => $user_name,
        '#size' => 32,
        '#disabled' => TRUE,
      );
    }

    $form['Instance_ssh']['ssh_label'    ] = array(
      '#type' => 'item',
      '#title' => t('SSH Key'),
    );
    $form['Instance_ssh']['ssh_text'     ] =  array(array('#markup' => l($key->key_name, $cloud_menu_path . '/ssh_keys/describe', array('query' => array('key_name' => $key->key_name)))));

    $form['Instance_sg']['sg_label'      ] = array(
      '#type' => 'item',
      '#title' => t('Security group(s)'),
    );

    $sg_arr = explode( ',', $key->group_id );
    $sg_link_arr = '';

    foreach ($sg_arr as $sg_item) {
      //find the name mapping
      $group_name = _aws_ec2_lib_get_security_group_name_by_id($cloud_context, $sg_item);
      if ($group_name != FALSE) {
        $sg_link_arr .= l( $group_name, $cloud_menu_path . '/security_groups/describe', array('query' => array('group_name' => $group_name)) ) . ' ,';
      }
    }
    $sg_link_arr = substr( $sg_link_arr, 0, -1 );

    $form['Instance_sg']['sg_text'       ] = array(array('#markup' => $sg_link_arr));

    $form['Instance_zone']['zone_label'  ] = array(
      '#type' => 'item',
      '#title' => t('Availability Zone'),
    );
    $form['Instance_zone']['zone_text'   ] = array(array('#markup' => t( $key->placement )));

    $form['Instance_type']['type_label'  ] = array(
      '#type' => 'item',
      '#title' => t('Instance type'),
    );
    $form['Instance_type']['type_text'   ] = array(array('#markup' => t( $key->instance_type )));

    $form['Instance_image']['image_label'] = array(
      '#type' => 'item',
      '#title' => t('Image'),
    );
    $form['Instance_image']['image_text'] = array(array('#markup' => t('@image_nickname (@image_id)', array(
          '@image_nickname' => $key->image_nickname,
          '@image_id' => $key->image_id,
        ) )));
    $form['Instance_owner']['owner_label'] = array(
      '#type' => 'item',
      '#title' => t('Owner'),
    );
    $form['Instance_owner']['owner_text'] = array(array('#markup' => t( $key->owner_id )));



    $form['Instance_Id'   ]['instance_id'] = array(
      '#type' => 'hidden',
      '#value' => $key->instance_id,
    );

    $launch_time = '';
    $runtime     = '';
    if (!empty($key->launch_time)) {

      $runtime     = cloud_get_runtime($key->launch_time);

      $launch_time = date_parse($key->launch_time);
      $launch_time = gmmktime( $launch_time['hour'], $launch_time['minute'], $launch_time['second'],  
                                  $launch_time['month'], $launch_time['day'   ], $launch_time['year'  ]);
      $launch_time = gmdate('Y-m-d H:i', $launch_time);


    }

    $form['instance_launch_time']['launch_time_label'] = array(
      '#type' => 'item',
      '#title' => t('Launch Time'),
    );
    $form['instance_launch_time']['launch_time_text']  = array(array('#markup' => t($launch_time)));

    $form['instance_runtime']['runtime_label']         = array(
      '#type' => 'item',
      '#title' => t('Runtime'),
    );
    $form['instance_runtime']['runtime_text']          = array(array('#markup' => t($runtime)));

    $save_disabled = TRUE;
    if (user_access($cloud_context . ' update instance details')) {
      $save_disabled = FALSE;
    }
    $form['save_button'] = array(
      '#type' => 'submit',
      '#value' => t('Update Details'),
      '#disabled' => $save_disabled,
    );

    $count++;
  }

  if ( $count == 0 ) {

    drupal_goto( $cloud_menu_path . '/instances');

    return;
  }

  $form['submit_buttons'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<span class="clear"></span><div class="container-inline"><div class="buttons">',
    '#suffix' => '</div></div>',
  );

  $form['submit_buttons']['save_button'] = array(
    '#type' => 'submit',
    '#value' => t('Update Details'),
    //'#attributes' => array('class' => 'instance-update-details-1'),
  );

  $cloud_menu_path       = cloud_get_menu_path($cloud_context );
  $lock_status           = _aws_ec2_lib_get_instances_lock_status($cloud_context);
  $user_keys_all         =  aws_ec2_lib_get_all_keys($cloud_context);

  $confirm_reboot_msg    = cloud_get_messagebox(t('Are you sure you want to reboot the instance "@instance_nickname" ?'             , array('@instance_nickname' => $instance_nickname)));
  $confirm_msg           = cloud_get_messagebox(t('Are you sure you want to terminate the instance "@instance_nickname" ?'          , array('@instance_nickname' => $instance_nickname)));
  $confirm_msgMonitor    = cloud_get_messagebox(t('Are you sure you want to monitor the instance "@instance_nickname" ?'            , array('@instance_nickname' => $instance_nickname)));
  $confirm_msgNotMonitor = cloud_get_messagebox(t('Are you sure you want to turn off monitoring the instance "@instance_nickname" ?', array('@instance_nickname' => $instance_nickname)));

  if ( $instance_state_name != 'terminated' && $instance_state_name != 'stopped' ) {

    if (isset($lock_status[$instance_id]) == FALSE || $lock_status[$instance_id] == 'Unlock') {

      if (user_access($cloud_context . ' terminate all instances') ||   
              (user_access($cloud_context . ' terminate own instance') && in_array($key_name, $user_keys_all))) {
        //$action_data = cloud_display_action( 'images/action_stop', 'Terminate' , $cloud_menu_path . '/instances/terminate' ,  array('query' => 'instance_id=' . urlencode($key) . '&name=' . urlencode($form['Nickname'][$key][0]['#value']) ,  'html' => TRUE), $prop['onclick']);//,   'instance_id=' . urlencode($key) . '&name=' . urlencode($form['Nickname'][$key][0]['#value']) , $prop );

        $location_reboot = check_url(url($cloud_menu_path . '/instances/reboot')) . '&instance_id=' . $instance_id . '&name=' . $instance_nickname;
        $action_reboot   = "document.forms[0].action='" . $location_reboot . "';";
        $onclick         = $action_reboot . $confirm_reboot_msg;
        $form['submit_buttons']['Reboot'] = array(
          '#type' => 'submit',
          '#value' => t('Reboot'),
          '#attributes' => array('onclick' => $onclick),
        );

        $location = check_url(url($cloud_menu_path . '/instances/terminate')) . '&instance_id=' . $instance_id . '&name=' . $instance_nickname;
        $action   = "document.forms[0].action='" . $location . "';";
        $onclick  = $action . $confirm_msg;
        $form['submit_buttons']['Terminate'] = array(
          '#type' => 'submit',
          '#value' => t('Terminate'),
          '#attributes' => array('onclick' => $onclick),
        );
      }
      /*
       elseif (user_access($cloud_context . ' terminate own instance')) {

       if (in_array($key_name, $user_keys_all)) {

       $location_reboot = check_url(url($cloud_menu_path . '/instances/reboot')) . '&instance_id=' . $instance_id . '&name=' . $instance_nickname;
       $action_reboot   = "document.forms[0].action='" . $location_reboot . "';";
       $onclick         = $action_reboot . $confirm_reboot_msg;
       $form['submit_buttons']['Reboot'] = array( '#type' => 'submit', '#value' => t('Reboot'),  '#attributes' => array('onclick' => $onclick) );

       //$action_data = cloud_display_action( 'images/action_stop', 'Terminate' , $cloud_menu_path . '/instances/terminate' ,  array('query' => 'instance_id=' . urlencode($key) . '&name=' . urlencode($form['Nickname'][$key][0]['#value']),  'html' => TRUE), $prop['onclick']);//,   'instance_id=' . urlencode($key) . '&name=' . urlencode($form['Nickname'][$key][0]['#value']) , $prop );
       $location    = check_url(url($cloud_menu_path . '/instances/terminate')) . '&instance_id=' . $instance_id . '&name=' . $instance_nickname;
       $action      = "document.forms[0].action='" . $location . "';";
       $onclick     = $action . $confirm_msg;
       $form['submit_buttons']['Terminate'] = array( '#type' => 'submit', '#value' => t('Terminate'),  '#attributes' => array('onclick' => $onclick) );
       }
       }*/



      $disabled = FALSE;
      $key_material = cloud_get_ssh_key($cloud_context, $key_name);
      if (empty($key_material) ) {

        $disabled = TRUE;
      }

      if (user_access($cloud_context . ' bundle instance') ) {

        $location = check_url(url($cloud_menu_path . '/instances/bundle_instance')) . '&instance_id=' . $instance_id  ;
        $action   = "document.forms[0].action='" . $location . "';";
        $onclick  = $action;
        $form['submit_buttons']['Bundle'] = array(
          '#type' => 'submit',
          '#value' => t('Bundle Instance'),
          '#attributes' => array('onclick' => $onclick),
          '#disabled' => $disabled,
        );
      }
      if (module_exists('cloud_failover')) {

        $fail_over_flag = cloud_failover_get_failover_flag($cloud_context, $instance_id);

        if (isset($fail_over_flag) && $fail_over_flag !=  CLOUD_FAILOVER_FLAG_NO_FAILOVER ) {

          $location = check_url(url($cloud_menu_path . '/instances/disable_failover')) . '&instance_id=' . $instance_id  ;
          $action   = "document.forms[0].action='" . $location . "';";
          $onclick  = $action;

          if (user_access($cloud_context . ' disable failover')) {
            $form['submit_buttons']['Failover'] = array(
              '#type' => 'submit',
              '#value' => t('Disable Failover'),
              '#attributes' => array('onclick' => $onclick),
              '#disabled' => $disabled,
            );
          }
          else {
            $form['submit_buttons']['Failover'] = array(
              '#type' => 'submit',
              '#value' => t('Disable Failover'),
              '#disabled' => TRUE,
            );
          }
        }
        else {

          $location = check_url(url($cloud_menu_path . '/instances/enable_failover')) . '&instance_id=' . $instance_id  ;
          $action   = "document.forms[0].action='" . $location . "';";
          $onclick  = $action;

          if (user_access($cloud_context . ' enable failover')) {
            $form['submit_buttons']['Failover'] = array(
              '#type' => 'submit',
              '#value' => t('Enable Failover'),
              '#attributes' => array('onclick' => $onclick),
              '#disabled' => $disabled,
            );
          }
          else {
            $form['submit_buttons']['Failover'] = array(
              '#type' => 'submit',
              '#value' => t('Enable Failover'),
              '#disabled' => TRUE,
            );
          }
        }
      }

    }
    else {

      if (user_access($cloud_context . ' terminate all instances') &&    user_access($cloud_context . ' terminate own instance') ) {

        $form['submit_buttons']['Reboot'   ] = array(
          '#type' => 'submit',
          '#value' => t('Reboot'   ),
          '#disabled' => TRUE,
        );
        $form['submit_buttons']['Terminate'] = array(
          '#type' => 'submit',
          '#value' => t('Terminate'),
          '#disabled' => TRUE,
        );
      }

      if (user_access($cloud_context . ' bundle instance') ) {

        $form['submit_buttons']['Bundle'] = array(
          '#type' => 'submit',
          '#value' => t('Bundle Instance'),
          '#disabled' => TRUE,
        );
      }

      if (user_access($cloud_context . ' enable failover') ) {

        $form['submit_buttons']['Failover'] = array(
          '#type' => 'submit',
          '#value' => t('Enable Failover'),
          '#disabled' => TRUE,
        );
      }

      // $action_data = cloud_display_action( 'images/icon_padlock.png', 'Locked', '' ,   '' );
      $src_img_path  = 'images/icon_padlock.png';
      $path = cloud_get_module_base() . $src_img_path;
      $image_info = image_get_info( $path );

      $attributes['class'] = 'image image-$label ' . (isset($attributes['class']) ? $attributes['class'] : '');
      $attributes['width'] = $image_info['width'];
      $attributes['height'] = $image_info['height'];

      //$action_data = theme( 'image' , $path , $msg , $msg , $attributes ) ;
      $msg = '';
      $out = theme( 'image', array(
        'path' => $path,
        'alt' => $msg,
        'title' => $msg,
        'width' => $image_info['width'],
        'height' => $image_info['height'],
        'attributes' => array('class' => $attributes['class']),
      ));

      $action_data = '<div class="lockIcon">' . l( $out, $target = '') . '</div>';
    }
  }
  else {
    $action_data = '';
  }

  $form['submit_buttons']['List Instances'] = array(
    '#type' => 'submit',
    '#value' => t('List Instances'),
  );
  $form['cloud_context']                   = array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );

  return $form;
}


/**
 *
 * @param     $form
 * @return
 */
function theme_aws_ec2_lib_display_instance_info($form) {


  if (isset($form['form'])) { // drupal 7 passed form as an array 
    $form = $form['form'];
  }

  $rows = array(
    array(
      drupal_render($form['Instance_Name']['Name_label']),
      drupal_render($form['Instance_Name_details'])
    . drupal_render($form['Instance_Name_details']['Name_text'])
    . drupal_render($form['Instance_Name_details']['submit'   ]),
    ),
    array(
      drupal_render($form['Instance_State']['State_label']),
      drupal_render($form['Instance_State']['State_text']),
    ),
    array(
      drupal_render($form['Instance_lock']['lock_label']),
      drupal_render($form['Instance_lock_details'])
    . drupal_render($form['Instance_lock_details']['lock_text'])
    . drupal_render($form['Instance_lock_details']['submit'   ]),
    ),
    array(
      drupal_render($form['Instance_pub_dns']['pub_dns_label']),
      drupal_render($form['Instance_pub_dns']['pub_dns_text']),
    ),
    array(
      drupal_render($form['Instance_pvt_dns']['pvt_dns_label']),
      drupal_render($form['Instance_pvt_dns']['pvt_dns_text']),
    ),
    array(
      drupal_render($form['Instance_elastic_ip']['elastic_ip_label']),
      drupal_render($form['Instance_elastic_ip_details'])
    . drupal_render($form['Instance_elastic_ip_details']['elastic_ip_text'])
    . drupal_render($form['Instance_elastic_ip_details']['submit'         ]),
    ),
    //  array(
    //    drupal_render($form['Instance_EC2_info']['EC2_info_label']),
    //    drupal_render($form['Instance_EC2_info']['EC2_info_text' ]),
    //  ),
    array(
      drupal_render($form['user_name_details']['user_name_label']),
      drupal_render($form['user_name_details'])
    . drupal_render($form['user_name_details']['user_name_text'])
    . drupal_render($form['user_name_details']['submit'         ]),
    ),
    array(
      drupal_render($form['Instance_ssh']['ssh_label']),
      drupal_render($form['Instance_ssh']['ssh_text']),
    ),
    array(
      drupal_render($form['Instance_sg']['sg_label']),
      drupal_render($form['Instance_sg']['sg_text']),
    ),
    array(
      drupal_render($form['Instance_zone']['zone_label']),
      drupal_render($form['Instance_zone']['zone_text']),
    ),
    array(
      drupal_render($form['Instance_type']['type_label']),
      drupal_render($form['Instance_type']['type_text']),
    ),
    array(
      drupal_render($form['Instance_aws']['aws_label']),
      drupal_render($form['Instance_aws']['aws_text']),
    ),
    array(
      drupal_render($form['Instance_image']['image_label']),
      drupal_render($form['Instance_image']['image_text'] ),
    ),
    array(
      drupal_render($form['Instance_owner']['owner_label']),
      drupal_render($form['Instance_owner']['owner_text'] ),
    ),
    array(
      drupal_render($form['instance_launch_time']['launch_time_label']),
      drupal_render($form['instance_launch_time']['launch_time_text'] ),
    ),
    array(
      drupal_render($form['instance_runtime']['runtime_label']),
      drupal_render($form['instance_runtime']['runtime_text'] ),
    ),
    array(
      drupal_render($form['save_button']),
      '',
    ),
  );


  $table = theme('table', array('header' => NULL, 'rows' => $rows));
  $form['fieldset_instance_info']['#children'] = $table;

  //cf. Warning by Coder module: Potential problem: when FAPI element '#type' is set to 'markup' (default), '#value' only accepts filtered text, be sure to use check_plain(), filter_xss() or similar to ensure your $variable is fully sanitized.
  //$form['fieldset_instance_info']['list'] = array('#type' => 'markup', '#value' => $table);

  $output  = drupal_render($form['instance_info_header'  ]);
  $output .= drupal_render($form['submit_buttons'        ]);
  $output .= drupal_render($form['fieldset_instance_info']);
  $output .= drupal_render($form['Instance_Id'           ]);
  $output .= drupal_render($form['cloud_context'         ]);
  $output .= drupal_render_children($form);

  return $output;
}


/**
 *
 * @param     $cloud_context
 * @return
 */
function _aws_ec2_lib_get_instance_id($cloud_context) {

  $instance_id = isset( $_REQUEST['instance_id']) ? $_REQUEST['instance_id'] : FALSE;

  /*global $user ;
   // If fetch from REQUST Parameter fails
   if (empty($instance_id) || strlen($instance_id) == 0  )
   $instance_id = $_SESSION['instance_id_desc_info'];
   */
  return $instance_id;
}


/**
 *
 * @param type $form_id
 * @param type $form_values
 * @return type
 */
function aws_ec2_lib_display_instance_info_submit($form_id, $form_values) {

  $form_values     = $form_values['input'];

  $cloud_context   = $form_values['cloud_context'];
  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  $instance_id     = _aws_ec2_lib_get_instance_id($cloud_context);

  switch ($form_values['op']) {

    case t('List Instances'):

      drupal_goto( $cloud_menu_path . '/instances');
      return;
      //update the instance nickname    
      aws_ec2_lib_update_instance_nickname($instance_id, $cloud_context, $form_values['Name_text']);

    case t('Bundle Instance'):

      drupal_set_message( t('Trying for Bundle Image: @instance_id', array('@instance_id' => $instance_id) ) );
      drupal_goto( $cloud_menu_path . '/instances/bundle_instance');
      return;


    case t('Update Details'):

      //-- Updating the Nickname

      if (isset($form_values['Name_text']))
        $num_updated = _aws_ec2_lib_update_instance_query($cloud_context, $instance_id, $form_values['Name_text']);

      //Update the cloud billing instances details
      if (module_exists('cloud_billing')) {
        if (isset($form_values['Name_text']))
          $query_billing        = _cloud_billing_update_instance_query($cloud_context, $instance_id, $form_values['Name_text']);
      }

      //-- Finished updating the Nickname

      //-- Assigning Elastic IP

      $ip_new = FALSE;
      
      if (isset($form_values['elastic_ip_text']) ) {
        $ip_new = $form_values['elastic_ip_text'];
      }

      $ip_org = $form_values['elastic_ip_text_default_val'];

      if ( $ip_org == $ip_new ) {

        // Do nothing Its the same IP selected
      }
      elseif ($ip_new !== FALSE && $ip_org == AWS_EC2_LIB_CLOUD_NONE ) {

        $response     = aws_ec2_api_associate_address( $cloud_context, $instance_id, $ip_new );
        $result       = _aws_ec2_lib_save_associate_address( $cloud_context, $response, $instance_id, $ip_new );
        if ( $result ) {
          drupal_set_message(t('Assigned the IP (@ip) to the Instance (@instance_id) ( This may take some time)',  
                                          array('@ip' => $ip_new, '@instance_id' => $instance_id)) );

          if (module_exists('cloud_failover')) {

            cloud_action_notify('update_eip_info', array('cloud_context' => $cloud_context, 'instance_id' => $instance_id, 'ip' => $ip_new));
          }
        }
        else {
          drupal_set_message(t('There was a problem while assigning the IP (@ip) to the instance (@instance_id)',  
                                                                     array('@ip' => $ip_new, '@instance_id' => $instance_id)),  
                                                                     'error');
        }
      }
      elseif ($ip_new !== FALSE &&  $ip_new == AWS_EC2_LIB_CLOUD_NONE ) {

        $ip_old   = $form_values['elastic_ip_text_default_ip'];
        $response = aws_ec2_api_disassociate_address( $cloud_context, $ip_old );
        $result   = _aws_ec2_lib_save_associate_address( $cloud_context, $response,    '', $ip_old );

        if ( $result ) {

          drupal_set_message(t('The IP has been released: @ip ( This may take some time)', array('@ip' => $ip_old)));

          if (module_exists('cloud_failover')) {

            cloud_action_notify('update_eip_info', array('cloud_context' => $cloud_context, 'instance_id' => $instance_id));
          }
        }
        else {
          drupal_set_message(t('There was a problem while releasing the IP: @ip', array('@ip' => $ip_old)), 'error');
        }
      }

      //-- Finished assigning IP

      //-- Setting the lock status
      $status_lock = $form_values['lock_text'];

      // Check if record exists
      $lock_query   = _aws_ec2_lib_get_lock_status_query($cloud_context);
      $query_args = array(
        ':instance_id' => $instance_id,
        ':cloud_type' => $cloud_context,
      );


      $result = db_query( $lock_query, $query_args );
      $lock_record_exists = FALSE;

      foreach ($result as $key) {
        $lock_record_exists = TRUE;
        break;
      }

      if ( !$lock_record_exists ) {

        $insert_query = _aws_ec2_lib_set_lock_status_query($cloud_context);
        $insert_query
          ->values(
            array(
          'instance_id' => $instance_id,
          'cloud_type' => $cloud_context,
          'is_locked' => $status_lock,
          'ssh_user_name' => CLOUD_SSH_USER_NAME,
        )
          )
          ->execute();

        $lock_record_exists = TRUE;
      }
      else {


        $update_query = _aws_ec2_lib_update_lock_status_query($cloud_context);
        $update_query
          ->fields(
            array(
          'is_locked' => $status_lock,
        ))
          ->condition('instance_id', $instance_id, '=')
          ->condition('cloud_type', $cloud_context, '=')
          ->execute();
      }

      //-- Finished updating the lock status

      //-- Updating user

      $ssh_user_name  = FALSE;
      if (isset($form_values['user_name_text'])) {
        
        $ssh_user_name  = $form_values['user_name_text'];
      }

      if ($ssh_user_name !== FALSE && $lock_record_exists === FALSE ) {

        $insert_query = _aws_ec2_lib_set_ssh_user_query($cloud_context);

        $insert_query
          ->values(
            array(
          'instance_id' => $instance_id,
          'cloud_type' => $cloud_context,
          'is_locked' => 'Unlock',
          'ssh_user_name' => $ssh_user_name,
        ))
          ->execute();

      }
      elseif ($ssh_user_name !== FALSE) {

        _aws_ec2_lib_update_ssh_user_query($cloud_context, $instance_id, $ssh_user_name);
      }
      break;
  }



  drupal_set_message(t('Instance information updated: @instance_id', array('@instance_id' => $instance_id)));

  cloud_audit_user_activity( array(
    'type' => 'user_activity',
    'message' => t('Instance information updated: @instance_id', array('@instance_id' => $instance_id)),
    'link' => '',
  )
  );

  if (empty($instance_id) === FALSE ) {

    drupal_goto( $cloud_menu_path . '/instances/describe/' . $instance_id . '/info');
  }
  else {

    drupal_goto( $cloud_menu_path . '/instances');
  }

  return;
}


/**
 *
 * @param     $form_submit
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_display_instance_volume_info($form_submit = '', $form_state, $cloud_context, $instance_id) {

  drupal_add_js(cloud_get_module_base() . 'js/cloud.js');

  if (user_access($cloud_context . ' list volume') === FALSE ) {

    $form['cloud_context'] = array(
      '#type' => 'hidden',
      '#value' => $cloud_context,
    );
    $form['instance_id'] = array(
      '#type' => 'hidden',
      '#value' => $instance_id,
    );
    drupal_set_message(t('You are not authorized to access this page.'));
    return $form;
  }

  $form['instance_volume_info_header'] = array(
    '#prefix' => '<h2>',
    '#markup' => t('Volume Info'),
    '#suffix' => '</h2>',
  );


  $form['fieldset_volumes_info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Currently Attached Volumes'),
  );

  $form['cloud_context'] = array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  // $instance_id     = _aws_ec2_lib_get_instance_id($cloud_context) ;

  if (empty($instance_id)
    || strlen($instance_id) == 0  ) {
    drupal_goto( $cloud_menu_path . '/instances');
    return;
  }

  if (user_access($cloud_context . ' attach volume') ) {

    $attach_vol_disabled = '';
    $lock_status = _aws_ec2_lib_get_lock_status_by_id_query($cloud_context, $instance_id);
    if (empty($lock_status) === FALSE && $lock_status === 'Lock' ) {

      $attach_vol_disabled = TRUE;
    }

    $form['submit_buttons'] = array(
      '#type' => 'fieldset',
      '#prefix' => '<span class="clear"></span><div class="container-inline"><div class="buttons">',
      '#suffix' => '</div></div>',
    );

    $form['submit_buttons']['attach_volume'] = array(
      '#type' => 'submit',
      '#value' => t('Attach Volume'),
      '#disabled' => $attach_vol_disabled,
    );
  }
  else {
    $form['submit_buttons']['attach_volume'] = array('#value' => '');
  }

  $query = _aws_ec2_lib_get_volumes_by_instance_query($cloud_context);
  $query_args = array(
    ':instance_id' => $instance_id,
    ':cloud_type' => $cloud_context,
  );

  $form['instance_id'] = array(
    '#type' => 'hidden',
    '#value' => $instance_id,
  );

  $result = db_query( $query, $query_args );

  $form['header'] = array(
    '#type' => 'value',
    '#value' => array(
      array(
        'data' => t('Nickname'),
        //'class' => 'nickname-column',
      ),
      array('data' => t('ID'      )),
      array('data' => t('Size'    )),
      array('data' => t('Device'  )),
      array('data' => t('Status'  )),
      array(
        'data' => t('Actions' ),
        'class' => 'action-column',
      ),
    ),
  );

  $destination = drupal_get_destination();

  foreach ($result as $vol) {

    $form['Nickname'][$vol->volume_id] = array(array('#markup' => l( $vol->nickname, $cloud_menu_path . '/ebs_volumes/describe',    array('query' => array('vol_id' => $vol->volume_id)) )));
    $form['ID'      ][$vol->volume_id] = array(array('#markup' => t( $vol->volume_id)));
    $form['Size'    ][$vol->volume_id] = array(array('#markup' => t( $vol->vol_size )));
    $form['Device'  ][$vol->volume_id] = array(array('#markup' => t( $vol->device   )));
    $form['Status'  ][$vol->volume_id] = array(array('#markup' => t( $vol->status   )));
  }

  return $form;
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_aws_ec2_lib_display_instance_volume_info($form) {


  if (isset($form['form'])) { // drupal 7 passed form as an array 
    $form = $form['form'];
  }

  $form['Nickname'] = !empty($form['Nickname']) ? $form['Nickname'] : array();
  $rows = array();

  $cloud_context = $form['cloud_context']['#value'];

  if (user_access($cloud_context . ' list volume') === FALSE ) {

    $output = drupal_render($form['cloud_context'              ]);
    $output .= drupal_render($form['instance_id'                ]);
    return $output;
  }


  foreach (element_children($form['Nickname']) as $key) {

    $row = array(
      array(
        'data' => drupal_render($form['Nickname'][$key]),
        //'class' => 'nickname-column',
      ),
      drupal_render($form['ID'      ][$key]),
      drupal_render($form['Size'    ][$key]),
      drupal_render($form['Device'  ][$key]),
      drupal_render($form['Status'  ][$key]),
    );

    $cloud_menu_path = cloud_get_menu_path($cloud_context );

    $action_data = '';
    if (user_access($cloud_context . ' create snapshot') ) {

      $action_data .= cloud_display_action( 'images/icon_play',  
                                            'Create Snapshot',  
                                            $cloud_menu_path . '/ebs_volumes/create_snapshot',  
                                            array('query' => array('vol_id' => $key, 'volume_name' => $form['Nickname'][$key][0]['#markup']), 'html' => TRUE));
    }

    $lock_status = _aws_ec2_lib_get_lock_status_by_id_query($cloud_context, $form['instance_id']['#value']);
    if (user_access($cloud_context . ' detach volume') && (empty($lock_status) || $lock_status === 'Unlock' ) ) {
      if ( substr_count( $form['Status'][$key][0]['#markup'], 'attached') > 0 ) {
        $action_data .= cloud_display_action( 'images/icon_clear', 'Detach', $cloud_menu_path . '/instances/describe/' . $form['instance_id']['#value'] . '/volume/detach_volume',    array('query' => array('vol_id' => urlencode($key)), 'html' => TRUE));
      } //,  'vol_id=' . urlencode($key) );
    }

    $row[]  = array(
      'data' => $action_data,
      'class' => 'action-column',
    );
    $rows[] = $row;
  }

  $table = theme('table', array('header' => $form['header']['#value'], 'rows' => $rows));
  $form['fieldset_volumes_info']['#children'] = $table;

  //cf. Waning by Coder module: Potential problem: when FAPI element '#type' is set to 'markup' (default), '#value' only accepts filtered text, be sure to use check_plain(), filter_xss() or similar to ensure your $variable is fully sanitized.
  //$form['fieldset_volumes_info']['list'] = array('#type' => 'markup', '#value' => $table);

  $output  = drupal_render($form['instance_volume_info_header']);
  $output .= drupal_render($form['submit_buttons'             ]);
  $output .= drupal_render($form['fieldset_volumes_info'      ]);
  $output .= drupal_render($form['cloud_context'              ]);
  $output .= drupal_render($form['instance_id'                ]);
  //$output .= drupal_render($form);
  $output .= drupal_render_children($form);

  return $output;
}


/**
 *
 * @param     $form_id
 * @param     $form_values
 * @return
 */
function aws_ec2_lib_display_instance_volume_info_submit($form_id, $form_values) {

  $form_values     = $form_values['values'       ];
  $cloud_context   = $form_values['cloud_context'];
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if ( $form_values['op'] == t('Attach Volume') ) {
    drupal_goto( $cloud_menu_path . '/instances/describe/' . $form_values['instance_id'] . '/volume/attach_volume' );
  }
}


/**
 * Attach Volume to instance
 *
 * @param     $form_submit
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_attach_instance_volume($form, $form_submit = '', $cloud_context, $instance_id) {

  $form['fieldset_volume_info'] = array('#type' => 'fieldset' /* '#title' => 'Volume Info' */);
  $form['cloud_context'] = array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );
  $form['instance_id'  ] = array(
    '#type' => 'hidden',
    '#value' => $instance_id,
  );


  $form['attach_header']  = array(
    '#prefix' => '<h2>',
    '#markup' => t( 'Volume to Server Attachment'),
    '#suffix' => '</h2>',
  );

  $query      = _aws_ec2_lib_get_describe_volumes_query( $cloud_context, $instance_id, 'available');

  $result = db_query( $query);
  $volume_options = array();
  $volume_options[''] = '- Select Volume -';

  foreach ($result as $vol) {

    $volume_options[$vol->volume_id] = $vol->nickname;
  }
  //asort($volume_options);

  $query       = _aws_ec2_lib_get_volumes_by_instance_query($cloud_context);
  $instance_id = _aws_ec2_lib_get_instance_id($cloud_context);
  $query_args = array(
    ':instance_id' => $instance_id,
    ':cloud_type' => $cloud_context,
  );


  $result = db_query( $query, $query_args );

  foreach ($result as $vol) {

    $server_name = $vol->instance_nickname;
  }

  $instance_id = _aws_ec2_lib_get_instance_id($cloud_context);
  $server_name = $instance_id;

  $volume_device_options = array(
    '/dev/sdj' => '/dev/sdj',
    '/dev/sdk' => '/dev/sdk',
    '/dev/sdl' => '/dev/sdl',
    '/dev/sdm' => '/dev/sdm',
    '/dev/xvdb' => '/dev/xvdb',
    '/dev/xvdc' => '/dev/xvdc',
    '/dev/xvdd' => '/dev/xvdd',
    '/dev/xvde' => '/dev/xvde',
    '/dev/xvdf' => '/dev/xvdf',
    '/dev/xvdj' => '/dev/xvdj',
    '/dev/xvdk' => '/dev/xvdk',
    '/dev/xvdl' => '/dev/xvdl',
    '/dev/xvdm' => '/dev/xvdm',
  );

  //asort($volume_device_options); 

  $form['Volume_details']['volume_label'    ] = array(
    '#type' => 'item'  ,
    '#title' => t('Volume'),
  );
  $form['Volume_details']['volume_select'   ] = array(
    '#type' => 'select',
    '#options' => $volume_options,
  );

  $form['Volume_details']['server_label'    ] = array(
    '#type' => 'item'  ,
    '#title' => t('Server'),
  );
  $form['Volume_details']['server_text'     ] = array(array('#markup' => t( $server_name )));

  $form['Volume_details']['device_label'    ] = array(
    '#type' => 'item'  ,
    '#title' => t('Device'),
  );
  $form['Volume_details']['device_select'   ] = array(
    '#type' => 'select',
    '#options' => $volume_device_options,
  );


  $form['Volume_details']['attach_opt_label'] = array(
    '#type' => 'item',
    '#title' => t('Attachment Options'),
  );
  $form['Volume_details']['attach_opt_text'] = array(array('#markup' => t( 'Attach now' )));

  $form['submit_buttons'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<span class="clear"></span><div class="container-inline"><div class="buttons">',
    '#suffix' => '</div></div>',
  );

  $form['submit_buttons']['OK'    ] = array(
    '#type' => 'submit',
    '#value' => t('OK'    ),
  );
  $form['submit_buttons']['Cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
  );

  return $form;
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_aws_ec2_lib_attach_instance_volume($form) {

  if (isset($form['form'])) { // drupal 7 passed form as an array  
    $form = $form['form'];
  }

  $rows = array(
    array(
      drupal_render($form['Volume_details']['volume_label'    ]),
      drupal_render($form['Volume_details']['volume_select'   ]),
    ),
    array(
      drupal_render($form['Volume_details']['server_label'    ]),
      drupal_render($form['Volume_details']['server_text'     ]),
    ),
    array(
      drupal_render($form['Volume_details']['device_label'    ]),
      drupal_render($form['Volume_details']['device_select'   ]),
    ),
    array(
      drupal_render($form['Volume_details']['attach_opt_label']),
      drupal_render($form['Volume_details']['attach_opt_text']),
    ),
  );

  $table = theme('table', array('header' => NULL, 'rows' => $rows));
  $form['fieldset_volume_info']['#children'] = $table;

  //cf. Waning by Coder module: Coder module: Potential problem: when FAPI element '#type' is set to 'markup' (default), '#value' only accepts filtered text, be sure to use check_plain(), filter_xss() or similar to ensure your $variable is fully sanitized.
  //$form['fieldset_volume_info']['list'] = array('#type' => 'markup', '#value' => $table);

  $output  = drupal_render($form['attach_header'        ]);
  $output .= drupal_render($form['fieldset_volume_info']);
  $output .= drupal_render($form['submit_buttons'       ]);
  $output .= drupal_render($form['cloud_context'        ]);
  $output .= drupal_render($form['instance_id'          ]);
  $output .= drupal_render_children($form);  

  return $output;
}


function aws_ec2_lib_attach_instance_volume_validate($form_id, $form_values) {
  $form_values = $form_values['values'];
  if ( $form_values['op'] == t('OK') ) {

    if ( empty( $form_values['volume_select']) == TRUE) {
      form_set_error('', t('Please select volume'));
      return;
    }
    elseif ( empty($form_values['device_select']) ) {
      form_set_error('', t('Please select device'));
      return;
    }
  }
}


function aws_ec2_lib_attach_instance_volume_submit($form_id, $form_values) {

  $form_values     = $form_values['values'];
  $cloud_context   = $form_values['cloud_context'];
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if ( $form_values['op'] == t('Cancel') ) {
    $instance_id = _aws_ec2_lib_get_instance_id($cloud_context);
    drupal_goto( $cloud_menu_path . '/instances/describe/' . $instance_id . '/volume' );
  }
  elseif ( $form_values['op'] == t('OK') ) {
    $instance_id = _aws_ec2_lib_get_instance_id($cloud_context);
    $myclient    = aws_ec2_api_attach_volume( $cloud_context, $form_values['volume_select'], $instance_id, $form_values['device_select'] );

    if ( $myclient->code == 200 ) {

      $resp = _aws_ec2_lib_save_attach_volume($cloud_context, $myclient);

      if ($resp) {
        drupal_set_message( check_plain(t('Attached Volume successfully to the instance: @instance_id', array('@instance_id' => $instance_id) ) ) );
        drupal_goto( $cloud_menu_path . '/instances/describe/' . $instance_id . '/volume' );
      }
      else {
        drupal_goto( $cloud_menu_path . '/instances/describe/' . $instance_id . '/volume' );
        drupal_set_message(t('There was a problem while attaching the volume'),    'error');
      }
    }
    else { // There was an error
      $instance_id = _aws_ec2_lib_get_instance_id($cloud_context);
      drupal_set_message(t('There was a problem while attaching the volume'),    'error');
      drupal_goto( $cloud_menu_path . '/instances/describe/' . $instance_id . '/volume' );
    }
  }
}


/**
 *
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_detach_instance_volume($cloud_context, $instance_id) {

  $vol_id          = isset( $_REQUEST['vol_id'] ) ? $_REQUEST['vol_id'] : FALSE;
  $myclient        = aws_ec2_api_detach_volume($cloud_context, $vol_id);
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  // $instance_id = _aws_ec2_lib_get_instance_id($cloud_context) ;
  if ( $myclient->code == 200 ) {

    _aws_ec2_lib_save_attach_volume( $cloud_context, $myclient );
    drupal_set_message(t('Detaching Volume: @vol_id. This may take some time.', array('@vol_id' => $vol_id)));
    drupal_goto( $cloud_menu_path . '/instances/describe/' . $instance_id . '/volume' );
  }
  else { // There was an error
    drupal_set_message(t('There was a problem while detaching the volume: @vol_id', array('@vol_id' => $vol_id)), 'error');
    drupal_goto( $cloud_menu_path . '/instances/describe/' . $instance_id . '/volume' );
  }

  return;
}


/**
 * Display the moniroting information
 *
 * @param     $form_submit
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_display_instance_monitor_info($form_submit = '', $cloud_context) {

  $instance_id = ($_REQUEST['instance_id'])
               ?  $_REQUEST['instance_id']
               : _aws_ec2_lib_get_instance_id($cloud_context);
  $query = _aws_ec2_lib_get_instance_query($cloud_context);
  $query_args = array(
    ':instance_id' => $instance_id,
    ':cloud_type' => $cloud_context,
  );


  $result = db_query( $query, $query_args );
  $count = 0;

  foreach ($result as $key) {

    $host_name = $key->dns_name;
    $monitor   = $key->monitor;
    $instance_nickname = $key->instance_nickname;
  }

  if ($monitor == 1) {

    $period_options = array(
      '-600' => 'Minutes-10',
      '-1800' => 'Minutes-30',
      '-3600' => 'Hour'       ,
      '-86400' => 'Day'        ,
      '-604800' => 'Week'       ,
      '-2678400' => 'Month'      ,
      '-31622400' => 'Year'       ,
    );

    // $def_period = variable_get( $user->uid.'instance_graph_period_value', '' );
    $def_period = cloud_get_filter_value( $form_submit, 'period_select' );
    if ( $def_period == '' || empty($def_period) ) {
      $def_period = '-3600'; // default
    }

    $form['monitor_details'] = array(
      '#type' => 'fieldset',
      '#prefix' => '<div class="container-inline">',
      '#suffix' => '</div>',
    );
    asort($period_options);
    $form['monitor_details']['period_label'] = array(
      '#type' => 'item'  ,
      '#title' => t('Period'),
    );
    $form['monitor_details']['period_select'] = array(
      '#type' => 'select',
      '#options' => $period_options,
      '#default_value' => $def_period,
    );
    $form['monitor_details']['period_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Go'),
    );

    $sec_period = $def_period;

    drupal_add_js(base_path()
                . drupal_get_path('module', 'aws_ec2_lib')
                . CLOUD_PATH_SEPARATOR
                . 'js/aws_ec2_lib_monitor.js'
                );

    $ec2_lib_monitor_url      = cloud_get_info( $cloud_context  , 'monitor_url' );
    $str_img = "<div class='padding'><img id='virt_cpu_total' src='" . $ec2_lib_monitor_url . "?hostname=$host_name;plugin=snmp;type=if_octets;type_instance=eth0;begin=$sec_period;TmpParam=" . REQUEST_TIME . "' alt='Please retry after some time' >";
    $form['virt_cpu_total'] = array(
      '#prefix' => "$str_img",
      '#type' => 'item',
      '#suffix' => '</img></div>',
    );
  }
  else {
    drupal_set_message(t('Please enable "Monitor".'));
  }

  $form_state['#redirect']  = FALSE;

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  $lock_status     = _aws_ec2_lib_get_instances_lock_status($cloud_context);
  $user_keys_all   =  aws_ec2_lib_get_all_keys($cloud_context);

  $confirm_msg           = cloud_get_messagebox('Are you sure you want to terminate the instance "' . $instance_nickname . '" ?');
  $confirm_msgMonitor    = cloud_get_messagebox('Are you sure you want to monitor the instance "' . $instance_nickname . '" ?');
  $confirm_msgNotMonitor = cloud_get_messagebox('Are you sure you want to turn off monitoring the instance "' . $instance_nickname . '" ?');

  if ( $instance_state_name != 'terminated' && $instance_state_name != 'stopped' ) {
    $arr = array();

    $result     = aws_ec2_lib_get_key_material_by_id($cloud_context, $instance_id);
    $key_file_name   = bin2hex($result[0]);
    $instance_ip    = $result[2];
    $key_material   = $result[1];

    $console_url = drupal_get_path('module', 'cloud') . '/cloud_console.inc';

    $console_data = '<H1>Use the Console window</H1>';
    $close_window = "<input type=button value=\"Close Window\" onClick=\"javascript:window.close();\">";

    $base_path = base_path();
    $keymanager_applet = _aws_ec2_lib_get_ssh_key_manager( $cloud_context, $key_name, $key_material, $instance_ip  , $key_file_name );

    $ssh_user_name = aws_ec2_lib_get_ssh_user( $cloud_context, $instance_id );
    $key_applet = "<applet archive=\"" . $base_path . AWS_EC2_LIB_SSH_CLIENT_JAR_FILE . "\" code=com.mindbright.application.MindTerm.class width=0 height=0><param name=server value=$instance_ip><param name=username value=$ssh_user_name><param name=port value=22><param name=sepframe value=true><param name=private-key value=$key_file_name><param name=auth-method value=publickey><param name=quiet value=true><param name=verbose value=true><param name=debug value=true><param name=autoprops value=both><param name=geometry value=\"100x35\"> </applet>";

    $arr['onclick'] = 'popWin = open(\'' . $console_url . '\', \'Console\', \'width=300, height=300\');  popWin.focus();
    popWin.document.write(\'' . $console_data . $close_window . $keymanager_applet . $key_applet . '\') ; return false ';

    $arr_msg = 'popWin = open(\'' . $console_url . '\', \'Console\', \'width=300, height=300\');  popWin.focus();
    popWin.document.write(\'' . $console_data . $close_window . $keymanager_applet . $key_applet . '\') ; return false ';

    if (isset($lock_status[$instance_id . '_monitor']) == FALSE || $lock_status[$instance_id . '_monitor'] == 0 ) {

      //$action_data .= cloud_display_action( 'images/icon_monitor.png', 'Monitor' , $cloud_menu_path . '/instances/monitor' ,  array('query' => 'instance_id=' . $instance_id . '&name=' . urlencode($instance_nickname) . "&action=Monitor",  'html' => TRUE), $prop_monitor['onclick']);//,   'id=' . urlencode($key) . '&name=' . urlencode($form['Nickname'][$key][0]['#value']) . "&action=Monitor" , $prop_monitor )  ;
      $location = check_url(url($cloud_menu_path . '/instances/monitor')) . '&instance_id=' . $instance_id . '&name=' . urlencode($instance_nickname) . "&action=Monitor";
      $action = "document.forms[0].action='" . $location . "';";
      $onclick = $action . $confirm_msgMonitor;
      $form['submit_buttons']['Monitor'] = array(
        '#type' => 'submit',
        '#value' => t('Monitor'),
        '#attributes' => array('onclick' => $onclick),
      );
    }
    elseif (isset($lock_status[$instance_id . '_monitor']) == FALSE || $lock_status[$instance_id . '_monitor'] == 1 ) {

      //$action_data .= cloud_display_action( 'images/icon_monitor_stop.png', 'Stop Monitoring' , $cloud_menu_path . '/instances/monitor' ,  array('query' => 'instance_id=' . $instance_id . '&name=' . urlencode($instance_nickname) . "&action=StopMonitor",  'html' => TRUE), $prop_not_monitor['onclick']);//,   'id=' . urlencode($key) . '&name=' . urlencode($form['Nickname'][$key][0]['#value']) . "&action=StopMonitor" , $prop_not_monitor )  ;
      $location = check_url(url($cloud_menu_path . '/instances/monitor')) . '&instance_id=' . $instance_id . '&name=' . urlencode($instance_nickname) . "&action=StopMonitor";
      $action = "document.forms[0].action='" . $location . "';";
      $onclick = $action . $confirm_msgNotMonitor;
      $form['submit_buttons']['Stop Monitoring'] = array(
        '#type' => 'submit',
        '#value' => t('Stop Monitoring'),
        '#attributes' => array('onclick' => $onclick),
      );
    }
  }

  return $form;
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_aws_ec2_lib_display_instance_monitor_info() {


  $output  = drupal_render($form['monitor_details']);
  $output .= drupal_render($form['monitor_display_img']);
  $output .= drupal_render($form);

  return $output;
}


function aws_ec2_lib_display_instance_monitor_info_submit($form_id, $form_values) {

  $form_values = $form_values['values'];
}



function aws_ec2_lib_get_key_material_by_id($cloud_context, $instance_id = '') {

  $query = _aws_ec2_lib_get_instance_query($cloud_context);
  $query_args = array(
    ':instance_id' => $instance_id,
    ':cloud_type' => $cloud_context,
  );


  $result = db_query( $query, $query_args );

  $key_name    = '';
  $instance_ip = '';

  foreach ($result as $key) {

    $key_name  = $key->key_name;
    $instance_ip  = $key->dns_name;
  }

  $query      = aws_ec2_lib_get_ssh_keys_query($cloud_context);
  $query_args   = array(
    ':key_name' => $key_name,
    ':cloud_type' => $cloud_context,
  );


  $result = db_query( $query, $query_args );

  $key_material  =  '';

  foreach ($result as $key) {

    $key_material  =  $key->key_material;
  }

  $key_material = str_replace( "\r\n",    "\n", $key_material );

  $resp = array();
  $resp[0] = $key_name;
  $resp[1] = $key_material;
  $resp[2] = $instance_ip;
  return $resp;
}


/**
 * Display Console of the instance
 *
 * @param     $page_submit
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_display_instance_console($page_submit = '', $form_state, $cloud_context, $instance_id) {

  if (!user_access($cloud_context . ' access all console') ) {

    if (user_access($cloud_context . ' access own console')) {
      $user_keys_all = aws_ec2_lib_get_all_keys($cloud_context);
      if (!in_array($form['KeyName'][$key]['#value'], $user_keys_all)) {
        drupal_set_message(t('You are not authorized to access this page.'));

        return;
      }
    }
  }
  else {

    $action_data = '';
    $instance_ip = '';
    $query = _aws_ec2_lib_get_instance_query($cloud_context);
    $query_args = array(
      ':instance_id' => $instance_id,
      ':cloud_type' => $cloud_context,
    );


    $result = db_query( $query, $query_args );

    foreach ($result as $key) {

      $key_name     = $key->key_name;
      $instance_ip  = $key->dns_name;
    }

    $query = aws_ec2_lib_get_ssh_keys_query($cloud_context);
    $query_args   = array(
      ':key_name' => $key_name,
      ':cloud_type' => $cloud_context,
    );

    $key_material = '';


    $result = db_query( $query, $query_args );


    foreach ($result as $key) {

      $key_material  =  $key->key_material;
    }

    $key_material = str_replace( "\n",    "#", $key_material );
    $key_file_name = bin2hex($key_name);

    $result         = aws_ec2_lib_get_key_material_by_id($cloud_context, $instance_id);
    $key_file_name  = bin2hex($result[0]);
    $instance_ip    = $result[2];
    $key_material   = $result[1];

    $console_url = drupal_get_path('module', 'cloud') . '/cloud_console.inc';

    $console_data = "<h1>Use the Console window</h1>";
    $close_window = "<input type=button value=\"Close Window\" onClick=\"javascript:window.close();\">";

    $base_path = base_path();
    $keymanager_applet = _aws_ec2_lib_get_ssh_key_manager( $cloud_context, $key_name, $key_material, $instance_ip  , $key_file_name );
    $ssh_user_name = aws_ec2_lib_get_ssh_user( $cloud_context, $instance_id );

    $key_applet = "<applet archive=\"" . $base_path . AWS_EC2_LIB_SSH_CLIENT_JAR_FILE . "\" code=com.mindbright.application.MindTerm.class width=0 height=0><param name=server value=$instance_ip><param name=username value=$ssh_user_name><param name=port value=22><param name=sepframe value=true><param name=private-key value=$key_file_name><param name=auth-method value=publickey><param name=quiet value=true><param name=verbose value=true><param name=debug value=true><param name=autoprops value=both><param name=geometry value=\"100x35\"> </applet>";
    $arr['onclick'] = 'popWin = open(\'' . $console_url . '\', \'Console\', \'width=300, height=300\');  popWin.focus();
    popWin.document.write(\'' . $console_data . $close_window . $keymanager_applet . $key_applet . '\') ;';

    $action_data .= cloud_display_action( 'images/icon_ssh', 'Console', $console_url,    array('query' => array('id' => urlencode($instance_id), 'instanceId' => $instance_id, 'keyFileName' => $key_file_name, 'instanceIp' => $result[2], 'keyMaterial' => url('sites/all/modules/cloud/tmp/abc.txt')), 'html' => TRUE), $arr['onclick']); //,   '' , $arr )  ;

    $tmp = $arr['onclick'];

    if ( empty($key_material) ) {

      $key_user_name = $key_name;
      $err_msg = 'alert(\'Key not found for the user ' . $key_user_name . '. Please set the key first.\') ; ';

      $form['onclick'] = array(
        '#type' => 'item',
        '#prefix' => "<script lang='javascript'>
                    $err_msg ; 
                  </script>",
        '#suffix' => '',
      );
    }
    else {
      $form['onclick'] = array(
        '#type' => 'item',
        '#prefix' => "<script lang='javascript'>
                        $tmp ; 
                      </script>",
        '#suffix' => '',
      );
    }

    $form['instance_id'] = array(
      '#type' => 'hidden',
      '#value' => $instance_id,
    );
    return $form;
  }
}


function aws_ec2_lib_display_instance_console_submit($form_id, $form_values) {

  $form_values = $form_values['values'];
  drupal_goto( $cloud_menu_path . '/images');
  return;

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  if ( $form_values['op'] == t('Close') || $form_values['op'] == t('Cancel') ) {

    drupal_goto( $cloud_menu_path . '/images');
    return;
  }
}

function aws_ec2_lib_display_instance_console_output($form_submit = '', $form_state, $cloud_context, $instance_id) {

  if (!user_access($cloud_context . ' access all console') && !user_access($cloud_context . ' access own console')) {

    drupal_set_message(t('You are not authorized to access this page.'));
  }
  else {

    //$instance_id = isset($_REQUEST['instance_id']) ? $_REQUEST['instance_id'] : _aws_ec2_lib_get_instance_id($cloud_context) ;  

    $cloud_menu_path = cloud_get_menu_path($cloud_context );
    if (empty($instance_id) || strlen($instance_id) == 0  ) {
      drupal_goto( $cloud_menu_path . '/instances');

      return;
    }

    $myclient = aws_ec2_api_get_console_output($cloud_context, $instance_id);
    //print_r($myclient); die;
    if ( $myclient->code != 200  ) {
      return FALSE;
    }

    $obj_response = new SimpleXMLElement($myclient->data);
    //print_r($obj_response); die;

    $form['fieldset_output_info'       ] = array('#type' => 'fieldset' /* '#title' => t('Output Info') */);

    $form['timestamp']['timestamp_label'] = array(
      '#type' => 'item',
      '#title' => t('Last output update'),
    );
    $form['timestamp']['timestamp_text'] = array(array('#markup' => ( ($obj_response->timestamp) ))); //array('#type' => 'textfield', '#size' => 100    );

    $form['output'   ]['output_label'   ] = array(
      '#type' => 'item'    ,
      '#title' => t('Output'),
    );
    $form['output'   ]['output_text'    ] = array(
      '#type' => 'textarea',
      '#readonly' => 'readonly',
      '#rows' => 25,
      '#align' => 'left',
    );

    if ($obj_response->output) {
      $form['output']['output_text']['#default_value'] = _aws_ec2_lib_br2nl(base64_decode($obj_response->output));
    }

    $form['cloud_context'] = array(
      '#type' => 'hidden',
      '#value' => $cloud_context,
    );
    $form['instance_id'  ] = array(
      '#type' => 'hidden',
      '#value' => $instance_id,
    );

    return $form;
  }
}


function _aws_ec2_lib_br2nl($str) {

  $str = preg_replace("/(\n|)/",    '', $str);
  return preg_replace("=<br */?>=i",    "\n", $str);
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_aws_ec2_lib_display_instance_console_output($form) {

  if (isset($form['form'])) {

    $form = $form['form'];
  }

  $output = '';
  $rows = array(
    array(
      drupal_render($form['timestamp']['timestamp_label'] ),
      drupal_render($form['timestamp']['timestamp_text'] ),
    ),
    array(
      drupal_render($form['output']['output_label'] ),
      drupal_render($form['output']['output_text'] ),
    ),
  );

  $table = theme('table', array('header' => NULL, 'rows' => $rows));
  $form['fieldset_output_info']['#children'] = $table;

  //cf. Waning by Coder module: Potential problem: when FAPI element '#type' is set to 'markup' (default), '#value' only accepts filtered text, be sure to use check_plain(), filter_xss() or similar to ensure your $variable is fully sanitized.
  //$form['fieldset_output_info']['list'] = array('#type' => 'markup', '#value' => $table);

  $output .= drupal_render($form['fieldset_output_info']);
  $output .= drupal_render($form['cloud_context'       ]);
  $output .= drupal_render($form['instance_id'         ]);

  $output .= drupal_render_children($form);

  return $output;
}

function aws_ec2_lib_display_instance_script_execution_output($form_submit = '', $cloud_context) {

  if (!user_access($cloud_context . ' access all console')
    &&    !user_access($cloud_context . ' access own console')) {

    drupal_set_message(t('You are not authorized to access this page.'));
  }
  else {

    $instance_id = isset($_REQUEST['instance_id'])
                 ?       $_REQUEST['instance_id']
                 : _aws_ec2_lib_get_instance_id($cloud_context);

    $cloud_menu_path = cloud_get_menu_path($cloud_context );
    if (empty($instance_id) || strlen($instance_id) == 0  ) {
      drupal_goto( $cloud_menu_path . '/instances');
      return;
    }

    $script_execution_log_file = "/tmp/clanavi/" . $instance_id . "/script_execution_output.log";
    $log_filename = $script_execution_log_file;
    $contents = '';
    $last_modified = REQUEST_TIME;
    if (file_exists($log_filename)) {

      $handle = fopen($log_filename,    "r");
      $contents = fread($handle,    filesize($log_filename));
      fclose($handle);

      $file_stats = stat( $log_filename );
      $last_modified = $file_stats['mtime'];
    }

    $form['timestamp']['timestamp_label'] = array(
      '#type' => 'item',
      '#title' => t('Last output update'),
    );
    $form['timestamp']['timestamp_text'] = array(array('#markup' => t( date(DATE_ATOM, $last_modified) ))); //array('#type' => 'textfield', '#size' => 100    );

    $form['output']['output_label'] = array(
      '#type' => 'item'    ,
      '#title' => t('Output'),
    );
    $form['output']['output_text'] = array(
      '#type' => 'textarea',
      '#readonly' => 'readonly',
      '#rows' => 25,
      '#align' => 'left',
    );

    if ($contents) {
      $form['output']['output_text']['#default_value'] = $contents;
    }

    return $form;
  }
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_aws_ec2_lib_display_instance_script_execution_output($form) {

  $rows = array(
    array(
      drupal_render($form['timestamp']['timestamp_label'] ),
      drupal_render($form['timestamp']['timestamp_text'] ),
    ),
    array(
      drupal_render($form['output']['output_label'] ),
      drupal_render($form['output']['output_text'] ),
    ),
  );

  $output  = theme('table', array('header' => NULL, 'rows' => $rows));

  $output .= drupal_render($form);

  return $output;
}


/**
 *
 * @param     $cloud_context
 * @param     $instance_info_arr
 * @return
 */
function aws_ec2_lib_get_instances_action($cloud_context, $instance_info_arr) {

  $cloud_menu_path = cloud_get_menu_path( $cloud_context );

  $key      = $instance_info_arr['instance_id'];
  $name     = trim($instance_info_arr['name']);
  $state    = $instance_info_arr['state'];
  $user_key = $instance_info_arr['user'];

  $prop_reboot[     'onclick'] = cloud_get_messagebox('Are you sure you want to reboot the instance "' . trim($name) . '" ?');
  $prop[            'onclick'] = cloud_get_messagebox('Are you sure you want to terminate the instance "' . $name . '" ?');
  $prop_monitor[    'onclick'] = cloud_get_messagebox('Are you sure you want to monitor the instance "' . $name . '" ?');
  $prop_not_monitor['onclick'] = cloud_get_messagebox('Are you sure you want turn off monitoring the instance "' . $name . '" ?');

  $user_keys_all = aws_ec2_lib_get_all_keys($cloud_context);
  $lock_status = _aws_ec2_lib_get_instances_lock_status($cloud_context);

  $action_data = '';

  $action_data .= cloud_display_action('images/icon_clear', t('Edit'), $cloud_menu_path . '/instances/describe/' . $key . '/info', array('query' => array('instance_id' => urlencode($key)), 'html' => TRUE));

  if ($state != 'terminated' && $state != 'stopped' ) {
    if (isset($lock_status[$key . '_monitor']) == FALSE
      ||          $lock_status[$key . '_monitor']  == 0) {
      /*$action_data .= cloud_display_action(
       'images/icon_monitor',
       'Monitor',
       $cloud_menu_path . '/instances/describe/' . $key . '/monitor',
       array('query' => 'instance_id=' . urlencode($key) . '&name=' . urlencode($name) . '&action=Monitor',
       'html' => TRUE)
       );*/
      // Monitoring disabled              
    }
    elseif (isset($lock_status[$key . '_monitor']) == FALSE
      ||              $lock_status[$key . '_monitor']  == 1) {
      /*$action_data .= cloud_display_action(
       'images/icon_monitor',
       'Monitor Enabled',
       $cloud_menu_path . '/instances/describe/' . $key . '/monitor',
       array('query' => 'instance_id=' . urlencode($key) . '&name=' . urlencode($name) . '&action=Monitor',
       'html' => TRUE)
       ); */
      // Monitoring disabled              
    }
    // end

    $arr = array();
    $result        = aws_ec2_lib_get_key_material_by_id($cloud_context, $key);
    $key_file_name = bin2hex($result[0]);
    $instance_ip   = $result[2];
    $key_material  = $result[1];

    $console_url = drupal_get_path('module', 'cloud') . '/cloud_console.inc';

    $console_data = '<H1>Use the Console window</H1>';
    $close_window = "<input type=button value=\"Close Window\" onClick=\"javascript:window.close();\">";

    $base_path         = base_path();
    $keymanager_applet = _aws_ec2_lib_get_ssh_key_manager( $cloud_context, $user_key, $key_material, $instance_ip  , $key_file_name );
    $ssh_user_name     = aws_ec2_lib_get_ssh_user( $cloud_context, $key );

    $key_applet = "<applet archive=\"" . $base_path . AWS_EC2_LIB_SSH_CLIENT_JAR_FILE . "\" code=com.mindbright.application.MindTerm.class width=0 height=0><param name=server value=$instance_ip><param name=username value=$ssh_user_name><param name=port value=22><param name=sepframe value=true><param name=private-key value=$key_file_name><param name=auth-method value=publickey><param name=quiet value=true><param name=verbose value=true><param name=debug value=true><param name=autoprops value=both><param name=geometry value=\"100x35\"> </applet>";

    $arr['onclick'] = 'popWin = open(\'' . $console_url . '\', \'Console\', \'width=300, height=300\');  popWin.focus();
          popWin.document.write(\'' . $console_data . $close_window . $keymanager_applet . $key_applet . '\') ; return false ';
    if ( empty($key_material) ) {

      $key_user_name = $user_key;
      $err_msg = 'popWin = open(\'' . $console_url . '\', \'Console\', \'width=300, height=300\');  popWin.focus();
        popWin.document.write(\' Key not found for the user <b>' . $key_user_name . '. </b> Please set the key first.\') ; return false ';

      $action_data .= cloud_display_action(
                          'images/icon_ssh',  
                          'Console',  
                          $console_url,  
                          array('query' => array('id' => urlencode($key), 'instanceId' => $key, 'keyFileName' => $key_file_name, 'instanceIp' => $result[2], 'keyMaterial' => url('sites/all/modules/cloud/tmp/abc.txt')), 'html' => TRUE),  
                          $err_msg
                        );
    }
    elseif (user_access($cloud_context . ' access all console')) {
      $action_data .= cloud_display_action(
                        'images/icon_ssh',  
                        'Console',  
                        $console_url,  
                        array('query' => array('id' => urlencode($key), 'instanceId' => $key, 'keyFileName' => $key_file_name, 'instanceIp' => $result[2], 'keyMaterial' => url('sites/all/modules/cloud/tmp/abc.txt')), 'html' => TRUE),  
                        $arr['onclick']
                      );
    }
    elseif (user_access($cloud_context . ' access own console')) {
      if (in_array($user_key, $user_keys_all)) {
        $action_data .= cloud_display_action(
                          'images/icon_ssh',  
                          'Console',  
                          $console_url,  
                          array('query' => array('id' => urlencode($key), 'instanceId' => $key, 'keyFileName' => $key_file_name, 'instanceIp' => $result[2], 'keyMaterial' => url('sites/all/modules/cloud/tmp/abc.txt')), 'html' => TRUE),  
                          $arr['onclick']
                        );
      }
    }
  }
  else {
    // $action_data = '' ;
  }

  if ( $state != 'terminated' && $state != 'stopped'  ) {

    $destination = drupal_get_destination();
    $destination['instance_id'] = urlencode($key);
    $destination['name'] = urlencode($name);

    if (isset($lock_status[$key]) == FALSE || $lock_status[$key] == 'Unlock') {
      if (user_access($cloud_context . ' terminate all instances')) {

        $action_data .= cloud_display_action(
                          'images/icon_reboot',  
                          'Reboot',  
                          $cloud_menu_path . '/instances/reboot'
                          , array(
          'query' => $destination,
          'html' => TRUE,
        ),  
                          $prop_reboot['onclick']
                        );

        $action_data .= cloud_display_action(
                          'images/action_stop',  
                          'Terminate',  
                          $cloud_menu_path . '/instances/terminate'
                          , array(
          'query' => $destination,
          'html' => TRUE,
        ),  
                          $prop['onclick']
                        );
      }
      elseif (user_access($cloud_context . ' terminate own instance')) {
        if (in_array($user_key, $user_keys_all)) {

          $action_data .= cloud_display_action(
                            'images/icon_reboot',  
                            'Reboot',  
                            $cloud_menu_path . '/instances/reboot'
                            , array(
            'query' => $destination,
            'html' => TRUE,
          ),  
                            $prop_reboot['onclick']
                          );

          $action_data .= cloud_display_action(
                            'images/action_stop',  
                            'Terminate',  
                            $cloud_menu_path . '/instances/terminate',  
                            array(
            'query' => $destination,
            'html' => TRUE,
          ),  
                            $prop['onclick']
                          );
        } // end if 
      } // end elseif
    } // end if
  } // end if
  else {
    $action_data = '';
  } // end else

  return $action_data;

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
function aws_ec2_lib_bundle_instance($form_submit = '', $cloud_context) {

  $form['bundl_image_details']['instance_label'] = array(
    '#type' => 'item'     ,
    '#title' => t('Instance'),
  );
  $instance_id     = _aws_ec2_lib_get_instance_id($cloud_context);
  $nickname = '';
  $bucketname = '';
  $foldername = '';

  $cloud_menu_path  = cloud_get_menu_path($cloud_context );

  if (empty($instance_id) === FALSE ) {

    $instance_info    = _aws_ec2_lib_get_instance_details_by_id($cloud_context, $instance_id);
    $form['bundl_image_details']['instance_name'] = array(array('#markup' => l( $instance_info['name'], $cloud_menu_path . '/instances/describe/' . $instance_id . '/info', array('query' => array('instance_id' => urlencode($instance_id))))));
    $nickname = $instance_info['name'] . '_' . date( "Y_n_j G:i:s T");
    $nickname = str_replace( array(' ', ':', '-', '_'), '', $nickname );
    $nickname = strtolower($nickname);
    $nickname = empty($_REQUEST['nickname_text']) ? $nickname : $_REQUEST['nickname_text'   ];
    $bucketname = empty($_REQUEST['bucketname_text']) ? '' : $_REQUEST['bucketname_text'   ];
    $foldername = empty($_REQUEST['foldername_text']) ? '' : $_REQUEST['foldername_text'   ];
  }
  else {

    drupal_goto( $cloud_menu_path . '/instances');
    return;
  }

  $vol_size = isset($_REQUEST['vol_size_text']) ? $_REQUEST['vol_size_text'   ] : '10240';

  $form['bundl_image_details']['nickname_label'   ] = array(
    '#type' => 'item'     ,
    '#title' => t('Image Nickname'),
  );
  $form['bundl_image_details']['nickname_text'    ] = array(
    '#type' => 'textfield',
    '#value' => $nickname,
    '#description' => 'No spaces, no special characters, no numbers allowed',
    '#required' => TRUE,
  );

  $form['bundl_image_details']['bucketname_label'] = array(
    '#type' => 'item'     ,
    '#title' => t('Bucket Name'),
  );
  $form['bundl_image_details']['bucketname_text'  ] = array(
    '#type' => 'textfield',
    '#value' => $bucketname,
    '#description' => 'No spaces, no special characters, no numbers allowed',
  );

  $form['bundl_image_details']['foldername_label'] = array(
    '#type' => 'item'     ,
    '#title' => t('Folder Name' ),
  );
  $form['bundl_image_details']['foldername_text'  ] = array(
    '#type' => 'textfield',
    '#value' => $foldername,
  );

  $form['bundl_image_details']['vol_size_label'   ] = array(
    '#type' => 'item'     ,
    '#title' => t('Root Volume Size(in MB)'),
  );
  $form['bundl_image_details']['vol_size_text'    ] = array(
    '#type' => 'textfield',
    '#value' => $vol_size  ,
    '#required' => TRUE,
  );

  $form['bundl_image_details']['cloud_text'       ] = array(
    '#type' => 'hidden'   ,
    '#value' => $cloud_context,
  );

  $form['cloud_context'] =  array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );
  $form['instance_id']   =  array(
    '#type' => 'hidden',
    '#value' => $instance_id,
  );

  $form['submit'] = array(
    '#type' => 'submit'   ,
    '#value' => t('Bundle'),
  );

  return $form;
}


/**
 *
 * @param   $form
 * @return
 */
function theme_aws_ec2_lib_bundle_instance($form) {

  $rows = array(
    array(
      drupal_render($form['bundl_image_details']['instance_label'] ),
      drupal_render($form['bundl_image_details']['instance_name']),
    ),
    array(
      drupal_render($form['bundl_image_details']['nickname_label']),
      drupal_render($form['bundl_image_details']['nickname_text'] ),
    ),
    array(
      drupal_render($form['bundl_image_details']['bucketname_label']),
      drupal_render($form['bundl_image_details']['bucketname_text'] ),
    ),
    array(
      drupal_render($form['bundl_image_details']['foldername_label']),
      drupal_render($form['bundl_image_details']['foldername_text'] ),
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
  $output .= drupal_render($form['submit']);
  $output .= drupal_render($form['cloud_context'        ]);
  $output .= drupal_render($form['instance_id'          ]);
  $output .= drupal_render($form);

  return $output;
}


function aws_ec2_lib_bundle_instance_validate($form_id, &$form_values) {

  $values = $form_values['values'];
  $op     = $values['op'];
  switch ($op) {

    case 'Bundle':
      $result         = preg_match('/[^a-z0-9]/', $values['nickname_text'])   ;
      $result_bucket  = preg_match('/[^a-z]/', $values['bucketname_text'])   ;
      if ( $result  == TRUE ) {

        form_set_error('', t('Please enter only small letters and numbers for Nickname. Spaces and special characters not allowed.'));
      }
      elseif ( empty($values['bucketname_text']) || $result_bucket == TRUE) {

        form_set_error('', t('Please enter only small letters for Bucket Name. Spaces, numbers and special characters not allowed. Bucket name is required and cannot be empty.'));
      }

  }
}


/**
 *
 * @param     $form_id
 * @param     $form_values
 * @return
 */
function aws_ec2_lib_bundle_instance_submit($form_id, &$form_values) {

  $values = $form_values['values'];
  $op     = $values['op'];
  switch ($op) {

    case 'Bundle':
      aws_ec2_lib_db_bundle_instance( 
        $values['cloud_context']   ,   
        $values['instance_id']     ,   
        $values['nickname_text']   ,   
        $values['bucketname_text'],   
        $values['foldername_text'],   
        $values['vol_size_text']   ,   
        'initiated' );
      drupal_set_message(t('Added the instance for Bundling'));
      return;
  }
  return;
}



function aws_ec2_lib_enable_failover($cloud_context) {

  $instance_id = _aws_ec2_lib_get_instance_id($cloud_context);

  cloud_action_notify('enable_failover', array('cloud_context' => $cloud_context, 'instance_id' => $instance_id));

  //aws_ec2_lib_instance_failover_status_update_temp($cloud_context, $instance_id, 'Enabled');

  drupal_set_message(check_plain(t('Failover turned on for instance.  This will require some time.')));

  $cloud_menu_path  = cloud_get_menu_path($cloud_context );
  drupal_goto( $cloud_menu_path . '/instances/describe/' . $instance_id . '/info');

}

function aws_ec2_lib_disable_failover($cloud_context) {

  $instance_id     = _aws_ec2_lib_get_instance_id($cloud_context);

  cloud_action_notify('disable_failover', array('cloud_context' => $cloud_context, 'instance_id' => $instance_id));

  //aws_ec2_lib_instance_failover_status_update($cloud_context, $instance_id, 'Disabled');

  drupal_set_message(  check_plain(t('Failover turned off for instance.  This will require some time.') ) );

  $cloud_menu_path  = cloud_get_menu_path($cloud_context );
  drupal_goto( $cloud_menu_path . '/instances/describe/' . $instance_id . '/info');

}
