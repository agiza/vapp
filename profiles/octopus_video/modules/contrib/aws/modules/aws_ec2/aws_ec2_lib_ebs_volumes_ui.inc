<?php

/**
 * @file
 * aws_ec2_lib UI for EBS Volumes
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */

/**
 * Updated by yas   2011/03/23
 * Updated by yas   2011/02/17
 * Updated by yas   2011/02/14
 * Updated by yas   2011/02/11
 * Updated by yas   2011/02/10
 * Updated by yas   2011/02/02
 */

/**
 *  Display Volumes list
 *
 * @param  $form_submit
 *              Form
 * @param  $cloud_context
 *          Instance belongs to this sub-cloud
 * @return
 */
function aws_ec2_lib_display_ebs_volmues_list($form_submit = '', $form_state, $cloud_context) {

  drupal_add_js(cloud_get_module_base() . 'js/cloud.js');
  drupal_add_js( drupal_get_path('module', 'aws_ec2_lib')
    . CLOUD_PATH_SEPARATOR
    . 'js/aws_ec2_lib_auto_refresh_list_volumes.js'
    );

  $column = 'volume_id';

  $options = array(
    t('Nickname'),
    t('ID'),
    t('Zone'),
    /*
 t('Instance'),
 t('Status'),
 */
  );

  $sql_col = '';

  $filter     = isset($_REQUEST['filter'   ]) ? $_REQUEST['filter'   ] : '';
  $filter_col = isset($_REQUEST['operation']) ? $_REQUEST['operation'] : 0  ; // default: Nickname
  $filter     = trim($filter);

  if ( $filter_col == 0 ) {
    $column  = 'Nickname'    ;
    $sql_col = 'nickname'    ;
  }
  elseif ($filter_col == 1 ) {
    $column  = 'Volume ID'   ;
    $sql_col = 'volume_id'   ;
  }
  elseif ($filter_col == 2 ) {
    $column  = 'Zone'        ;
    $sql_col = 'vol_zone'    ;
  }
  elseif ($filter_col == 3 ) {
    $column  = 'Instance'    ;
    $sql_col = 'instance_id';
  }
  elseif ($filter_col == 4 ) {
    $column  = 'Status'      ;
    $sql_col = 'status'      ;
  }

  $query_args = array();

  if ( isset($filter) ) {
    $query_args[] = $sql_col;
    $query_args[] = $filter;
    $query_args[] = $cloud_context;
  }
  else {
    $filter = ' 1 ';
    $query_args[] = ' ';
    $query_args[] = $cloud_context;
  }

  // asort($options);

  $form['options'] = array(
    '#prefix' => '<div class="container-inline">',
    '#type' => 'fieldset',
    '#suffix' => '</div>',
    // '#title' => t('Operations'),
  );

  $volume_count = _aws_ec2_lib_get_volumes_list_count($cloud_context);
  $filter_disabled  = '';
  if ( $volume_count < 2 ) {

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
  if (user_access($cloud_context . ' create volume') ) {
    $form['options']['new_key'] = array(
      '#type' => 'submit',
      '#value' => t('Create'),
    );
  }

  $form['header'] = array(
    '#type' => 'value',
    '#value' => array(
      array(
        'data' => t('Nickname'),
        'field' => 'nickname'           ,
        //'class' => 'nickname-column'    ,
        'sort' => 'asc',
      ),
      array(
        'data' => t('ID'      ),
        'field' => 'volume_id',
      ),
      array(
        'data' => t('Status'  ),
        'field' => 'status',
      ),
      array(
        'data' => t('Zone'    ),
        'field' => 'vol_zone',
      ),
      array(
        'data' => t('Size'    ),
        'field' => 'vol_size',
      ),
      array(
        'data' => t('Instance'),
        'field' => 'instance_nickname',
      ),
      array(
        'data' => t('Device'  ),
        'field' => 'device',
      ),
      array(
        'data' => t('Created' ),
        'field' => 'created',
      ),
      array(
        'data' => t('Actions' ),
        'class' => 'action-column',
      ),
    ),
  );

  $query  = _aws_ec2_lib_get_describe_volumes_list_query($cloud_context);

  $query = $query
      ->condition('c.cloud_type', $cloud_context, '=')
      ->condition($sql_col, '%' . $filter . '%', 'like')
      ->extend('TableSort')
      ->extend('PagerDefault')
      ->orderByHeader($form['header']['#value'])
      ->limit(AWS_EC2_LIB_PAGER_LIMIT)
      ->range(0, AWS_EC2_LIB_PAGER_LIMIT);

  $result = $query->execute();
  $destination = drupal_get_destination();

  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  foreach ($result as $vol) {

    // $form['Nickname'][$vol->volume_id] = array( array('#value' => t( $vol->nickname ) ) );
    $form['Nickname'][$vol->volume_id] = array(array('#markup' => l( $vol->nickname, $cloud_menu_path . '/ebs_volumes/describe',    array('query' => array('vol_id' => urlencode($vol->volume_id ))))));
    $form['hdnName'][$vol->volume_id] = array(
      '#type' => 'hidden',
      '#value' => addslashes($vol->nickname),
    );
    $form['ID'      ][$vol->volume_id] = array(array('#markup' => t( $vol->volume_id        )));
    $form['Status'  ][$vol->volume_id] = array(array('#markup' => t( $vol->status           )));
    $form['Zone'    ][$vol->volume_id] = array(array('#markup' => t( $vol->vol_zone         )));
    $form['Size'    ][$vol->volume_id] = array(array('#markup' => t( $vol->vol_size         )));
    $form['Instance'][$vol->volume_id] = array(array('#markup' => l( $vol->instance_nickname, $cloud_menu_path . '/instances/describe/' . $vol->instance_id . '/info', array('query' => array('instance_id' => urlencode($vol->instance_id))))));
    $form['Device'  ][$vol->volume_id] = array(array('#markup' => t( $vol->device           )));
    $form['VolName'][$vol->volume_id] = array(
      '#type' => 'hidden',
      '#value' => $vol->nickname,
    );
    
    $created = '' ;
    if ($vol->created) {

      $created = format_date(strtotime($vol->created), 'short');
    }

    $form['Created'][$vol->volume_id] = array(array('#markup' => $created));
    if (empty($vol->instance_id) === FALSE ) {

      $lock_status = _aws_ec2_lib_get_lock_status_by_id_query($cloud_context, $vol->instance_id);
      $form['locked'  ][$vol->volume_id] = array(
        '#type' => 'hidden',
        '#value' => $lock_status,
      );
      if ($lock_status === 'Lock' ) {
        $form['Nickname'][$vol->volume_id][0]['#markup'] = $form['Nickname'][$vol->volume_id][0]['#markup'] . '<a class=lockIcon title=Lock>&nbsp;&nbsp;&nbsp;&nbsp;</a>';
      }
    }
    else {

      $form['locked'  ][$vol->volume_id] = array(
        '#type' => 'hidden',
        '#value' => 'Available',
      );
    }
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
  $form['#redirect'    ]  = FALSE;
  $form['#action'] = url($_REQUEST['q']);

  return $form;
}


/**
 *
 * @param  $form
 * @return
 */
function theme_aws_ec2_lib_display_ebs_volmues_list($form) {


  if (isset($form['form'])) { // drupal 7 passed form as an array
    $form = $form['form'];
  }

  $cloud_context = $form['cloud_context']['#value'];

  $output = '';
  if (user_access($cloud_context . ' create volume') ) {
    $output = drupal_render($form['new_key']);
  }
  $output .= drupal_render($form['options']);

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  $form['Nickname'] = !empty($form['Nickname']) ? $form['Nickname'] : array();
  
  $rows = array();
  foreach (element_children($form['Nickname']) as $key) {

    $row = array(
      array(
        'data' => drupal_render($form['Nickname'][$key]),
        //'class' => 'nickname-column',
      ),
      drupal_render($form['ID'      ][$key]),
      drupal_render($form['Status'  ][$key]),
      drupal_render($form['Zone'    ][$key]),
      drupal_render($form['Size'    ][$key]),
      drupal_render($form['Instance'][$key]),
      drupal_render($form['Device'  ][$key]),
      drupal_render($form['Created'][$key]),
    );

    $prop['onclick'] = cloud_get_messagebox('Are you sure you want to delete the EBS-Volume "' . $form['hdnName'][$key]['#value'] . '" ?');

    $action_data  = '';
    if (user_access($cloud_context . ' create snapshot') ) {
      $action_data .= cloud_display_action( 'images/icon_play', 'create snapshot', $cloud_menu_path . '/ebs_volumes/create_snapshot',    array('query' => array('vol_id' => urlencode($key), 'volume_name' => urlencode($form['VolName'][$key]['#value'])), 'html' => TRUE)); //,  'volume_id=' . urlencode($key) . "&volume_name=" . urlencode($form['VolName'][$key]['#value']) );
    }

    if (user_access($cloud_context . ' delete volume') ) {
      if ( $form['Status'][$key][0]['#markup'] == 'available' ) {
        $action_data .= cloud_display_action( 'images/icon_delete', t('Delete'), $cloud_menu_path . '/ebs_volumes/delete',    array('query' => array('vol_id' => urlencode($key))  , 'html' => TRUE), $prop['onclick']);
      }
    }

    if ($form['locked'][$key]['#value'] !== 'Available' && $form['locked'][$key]['#value'] !== 'Lock'    && user_access($cloud_context . ' detach volume') ) {

      $destination = drupal_get_destination();
      $prop_detach = cloud_get_messagebox('Are you sure you want to detach the EBS-Volume "' . $form['hdnName'][$key]['#value'] . '" ?');
      $destination['vol_id'] =  urlencode($key);
      $action_data .= cloud_display_action( 'images/icon_clear', 'Detach', $cloud_menu_path . '/ebs_volumes/detach',    array('query' => $destination, 'html' => TRUE), $prop_detach );
    }

    if (user_access($cloud_context . ' delete volume') ) {

      $action_data .= cloud_display_action( 'images/icon_edit', t('Edit'), $cloud_menu_path . '/ebs_volumes/describe', array('query' => array('vol_id' => urlencode($key)), 'html' => TRUE));
    }

    /*if ( substr_count( $form['Status'][$key][0]['#value'] ,  'attached') > 0 )
     $row[] = cloud_display_action( 'images/icon_clear', 'Detach' , $cloud_menu_path . '/instances/describe/' . $key .  '/volume/detach_volume', 'vol_id=' . urlencode($key) );
     */

    $row[]  = array(
      'data' => $action_data,
      'class' => 'action-column',
    );
    $rows[] = $row;
  }

  $table_attr = array();
  $table_attr['id'] = 'volumes_list_table';
  $table_attr['autoupdate_url'] = url( $cloud_menu_path . '/callback_get_volumes_list' );

  $output .= theme('table', array('header' => $form['header']['#value'], 'rows' => $rows, 'attributes' => $table_attr));

  
  $reload_link = l( t( '- Refresh Page -' ), $cloud_menu_path . '/get_volumes_data',    array() );
  $ref_link    = array(
    '#type' => 'item',
    '#prefix' => '<div id="link_reload" align="right">',
    '#suffix' => '</div>',
    '#markup' => $reload_link,
  );

  $output .= theme('pager', array('tags' => array() ));
  $output .= drupal_render($ref_link             );
  $output .= drupal_render($form['cloud_context']);
  $output .= drupal_render($form['filter_hdn']);
  $output .= drupal_render($form['operation_hdn']);

  $output .= drupal_render_children($form);

  return $output;
}


function _aws_ec2_lib_callback_get_volumes_list($cloud_context) {

  $column = 'volume_id';

  $sql_col    = '';
  $filter     = isset($_REQUEST['filter'   ]) ? $_REQUEST['filter'   ] : '';
  $filter_col = isset($_REQUEST['operation']) ? $_REQUEST['operation'] : 0; // Default: Nickname

  $filter = trim($filter);

  if ( $filter_col == 0 ) {
    $column  = 'Nickname';
    $sql_col = 'nickname';
  }
  elseif ($filter_col == 1 ) {
    $column  = 'Volume ID';
    $sql_col = 'volume_id';
  }
  elseif ($filter_col == 2 ) {
    $column  = 'Zone';
    $sql_col = 'vol_zone';
  }
  elseif ($filter_col == 3 ) {
    $column  = 'Instance';
    $sql_col = 'instance_id';
  }
  elseif ($filter_col == 4 ) {
    $column  = 'Status';
    $sql_col = 'status';
  }

  $query_args = array();

  if ( isset($filter) ) {

    $query_args[] = $sql_col;
    $query_args[] = $filter;
    $query_args[] = $cloud_context;
  }
  else {
    $filter       = ' 1 ';
    $query_args[] = ' ';
    $query_args[] = $cloud_context;
  }

  $table_data = array();

  $table_data['header'] = array(
    '#type' => 'value',
    '#value' => array(
      array(
        'data' => t('Nickname'),
        'field' => 'nickname'           ,
        //'class' => 'nickname-column'    ,
        'sort' => 'asc',
      ),
      array(
        'data' => t('ID'      ),
        'field' => 'volume_id',
      ),
      array(
        'data' => t('Status'  ),
        'field' => 'status',
      ),
      array(
        'data' => t('Zone'    ),
        'field' => 'vol_zone',
      ),
      array(
        'data' => t('Size'    ),
        'field' => 'vol_size',
      ),
      array(
        'data' => t('Instance'),
        'field' => 'instance_nickname',
      ),
      array(
        'data' => t('Device'  ),
        'field' => 'device',
      ),
      array(
        'data' => t('Created' ),
        'field' => 'created',
      ),
      array(
        'data' => t('Actions' ),
        'class' => 'action-column',
      ),
    ),
  );

  $query  = _aws_ec2_lib_get_describe_volumes_list_query($cloud_context);

  $query
      ->condition('c.cloud_type', $cloud_context, '=')
      ->condition($sql_col, '%' . $filter . '%', 'like')
      ->extend('TableSort')
      ->orderByHeader($table_data['header']['#value']);

  $result = $query->execute();

  $prop = array();

  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  foreach ($result as $vol) {

    $table_data['Nickname'][$vol->volume_id] = array(array('#markup' => l( $vol->nickname, $cloud_menu_path . '/ebs_volumes/describe',    array('query' => array('vol_id' => urlencode($vol->volume_id))))));
    $table_data['hdnName'][$vol->volume_id] = array(
      '#type' => 'hidden',
      '#value' => addslashes($vol->nickname),
    );
    $table_data['ID'      ][$vol->volume_id] = array(array('#markup' => t( $vol->volume_id        )));
    $table_data['Status'  ][$vol->volume_id] = array(array('#markup' => t( $vol->status           )));
    $table_data['Zone'    ][$vol->volume_id] = array(array('#markup' => t( $vol->vol_zone         )));
    $table_data['Size'    ][$vol->volume_id] = array(array('#markup' => t( $vol->vol_size         )));
    $table_data['Instance'][$vol->volume_id] = array(array('#markup' => t( $vol->instance_nickname)));
    $table_data['Device'  ][$vol->volume_id] = array(array('#markup' => t( $vol->device           )));
    $table_data['VolName'][$vol->volume_id] = array(
      '#type' => 'hidden',
      '#value' => $vol->nickname,
    );
    $table_data['Created'][$vol->volume_id] = array(array('#markup' => format_date(strtotime($vol->created), 'short')));

    if (empty($vol->instance_id) === FALSE ) {

      $lock_status = _aws_ec2_lib_get_lock_status_by_id_query($cloud_context, $vol->instance_id);
      $table_data['locked'  ][$vol->volume_id] = array(
        '#type' => 'hidden',
        '#value' => $lock_status,
      );
      if ($lock_status === 'Lock' ) {
        $table_data['Nickname'][$vol->volume_id][0]['#markup'] = $table_data['Nickname'][$vol->volume_id][0]['#markup'] . '<a class=lockIcon title=Lock>&nbsp;&nbsp;&nbsp;&nbsp;</a>';
      }
    }
    else {

      $table_data['locked'  ][$vol->volume_id] = array(
        '#type' => 'hidden',
        '#value' => 'Available',
      );
    }

  }

  // Theme for the table
  $output = '';

  $table_data['Nickname'] = !empty($table_data['Nickname']) ? $table_data['Nickname'] : array();


  $rows = array();
  foreach (element_children($table_data['Nickname']) as $key) {

    $row = array(
      array(
        'data' => drupal_render($table_data['Nickname'][$key]),
        //'class' => 'nickname-column',
      ),
      drupal_render($table_data['ID'      ][$key]),
      drupal_render($table_data['Status'  ][$key]),
      drupal_render($table_data['Zone'    ][$key]),
      drupal_render($table_data['Size'    ][$key]),
      drupal_render($table_data['Instance'][$key]),
      drupal_render($table_data['Device'  ][$key]),
      drupal_render($table_data['Created'][$key]),
    );

    $prop['onclick'] = cloud_get_messagebox('Are you sure you want to delete the EBS-Volume "' . $table_data['hdnName'][$key]['#value'] . '" ?');

    $action_data  = '';
    if (user_access($cloud_context . ' create snapshot') ) {
      $action_data .= cloud_display_action( 'images/icon_play', 'create snapshot', $cloud_menu_path . '/ebs_volumes/create_snapshot',    array('query' => array('vol_id' => urlencode($key), 'volume_name' => urlencode($table_data['VolName'][$key]['#value'])), 'html' => TRUE));
    }

    if (user_access($cloud_context . ' delete volume') ) {
      if ( $table_data['Status'][$key][0]['#markup'] == 'available' ) {
        $action_data .= cloud_display_action( 'images/icon_delete', t('Delete'), $cloud_menu_path . '/ebs_volumes/delete',    array('query' => array('vol_id' => urlencode($key))  , 'html' => TRUE), $prop['onclick']);
      }
    }


    if ($table_data['locked'][$key]['#value'] !== 'Available' && $table_data['locked'][$key]['#value'] !== 'Lock'    && user_access($cloud_context . ' detach volume') ) {

      $destination  = 'destination=' . $cloud_menu_path . '/ebs_volumes'  ;
      $prop_detach  = cloud_get_messagebox('Are you sure you want to detach the EBS-Volume "' . $table_data['hdnName'][$key]['#value'] . '" ?');
      $destination['vol_id'] = urlencode($key);
      $action_data .= cloud_display_action( 'images/icon_clear', 'Detach', $cloud_menu_path . '/ebs_volumes/detach',    array('query' => $destination, 'html' => TRUE), $prop_detach );
    }

    if (user_access($cloud_context . ' delete volume') ) {

      $action_data .= cloud_display_action( 'images/icon_edit', t('Edit'), $cloud_menu_path . '/ebs_volumes/describe', array('query' => array('vol_id' => urlencode($key)), 'html' => TRUE));
    }


    /*if ( substr_count( $table_data['Status'][$key][0]['#value'] ,  'attached') > 0 )
     $row[] = cloud_display_action( 'images/icon_clear', 'Detach' , $cloud_menu_path . '/instances/describe/' . $key .  '/volume/detach_volume', 'vol_id=' . urlencode($key) );
     */

    $row[]  = array(
      'data' => $action_data,
      'class' => 'action-column',
    );
    $rows[] = $row;
  }

  $output .= theme('table', array('header' => $table_data['header']['#value'], 'rows' => $rows));

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
 * @param  $form_id
 * @param  $form_values
 * @return
 */
function aws_ec2_lib_display_ebs_volmues_list_submit($form_id, &$form_values) {

  $form_values   = $form_values['values'       ];
  $cloud_context = $form_values['cloud_context'];

  if ( $form_values['op'] == t('Create')) {

    $cloud_menu_path = cloud_get_menu_path($cloud_context );
    return drupal_goto( $cloud_menu_path . '/ebs_volumes/create');

  }
  elseif ( $form_values['op'] == t('List Volumes')) {

    $cloud_menu_path = cloud_get_menu_path($cloud_context);
    drupal_goto($cloud_menu_path . '/ebs_volumes');
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
 * Create Volume
 *
 * @param  $form_submit
 *              Form to be object
 * @param  $cloud_context
 *                 sub-cloud
 * @return
 */
function aws_ec2_lib_create_ebs_volume($form, $form_submit = '', $cloud_context) {

  $form['fieldset_volume_info'             ] = array('#type' => 'fieldset' /* '#title' => t('Volume Info') */);

  $ss_id = isset( $_REQUEST['ss_id'] ) ? $_REQUEST['ss_id'] : '';
  $form['Volume_details']['Name_label'     ] = array(
    '#type' => 'item',
    '#title' => t('Nickname'),
    '#required' => TRUE,
  );
  $form['Volume_details']['Name_text'      ] = array('#type' => 'textfield');

  $snapshot_options = _aws_ec2_lib_get_snapshot_options($cloud_context);
  $form['Volume_details']['Snapshot_label'] = array(
    '#type' => 'item',
    '#title' => t('Snapshot'),
  );
  $form['Volume_details']['Snapshot_select'] = array(
    '#type' => 'select',
    '#options' => $snapshot_options,
  );

  $zone_options = aws_ec2_lib_get_zones($cloud_context);
  unset($zone_options['any']);

  $form['Volume_details']['zone_label'     ] = array(
    '#type' => 'item',
    '#title' => t('Availability Zone'),
  );
  $form['Volume_details']['zone_select'    ] = array(
    '#type' => 'select',
    '#options' => $zone_options,
  );

  $form['Volume_details']['Size_label'     ] = array(
    '#type' => 'item',
    '#title' => t('Size in GB'),
    '#required' => TRUE,
  );
  $form['Volume_details']['Size_text'      ] = array('#type' => 'textfield');

  if ( empty($ss_id) === FALSE ) { // If Snapshot id is present set the default values
    $form['Volume_details']['Snapshot_select']['#default_value'] =  $ss_id;
    $snapshot_data = _aws_ec2_lib_get_snapshots_by_id_db($cloud_context, $ss_id );
    $size          = $snapshot_data['vol_size'];
    $size_arr      = preg_split( '[ ]', $size );
    if ( isset($size_arr[0]) ) {

      $size = $size_arr[0];
      $form['Volume_details']['Size_text']['#default_value'] =  $size  ;
    }

    $form['Volume_details']['zone_select']['#default_value'] =  $snapshot_data['vol_zone']  ;
  }

  $form['submit_buttons'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<span class="clear"></span><div class="container-inline"><div class="buttons">',
    '#suffix' => '</div></div>',
  );

  $form['cloud_context'] = array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );

  $form['submit_buttons']['Create'] = array(
    '#type' => 'submit',
    '#value' => t('Create'),
  );
  $form['submit_buttons']['Cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
  );

  return $form;
}


/**
 *
 * @param  $form
 * @return
 */
function theme_aws_ec2_lib_create_ebs_volume($form) {


  if (isset($form['form'])) {

    $form = $form['form'];
  }

  $rows = array(
    array(
      drupal_render($form['Volume_details']['Name_label'     ] ),
      drupal_render($form['Volume_details']['Name_text'      ] ),
    ),
    array(
      drupal_render($form['Volume_details']['Snapshot_label'] ),
      drupal_render($form['Volume_details']['Snapshot_select'] ),
    ),
    array(
      drupal_render($form['Volume_details']['zone_label'     ] ),
      drupal_render($form['Volume_details']['zone_select'    ] ),
    ),
    array(
      drupal_render($form['Volume_details']['Size_label'     ] ),
      drupal_render($form['Volume_details']['Size_text'      ] ),
    ),
  );

  $table = theme('table', array('header' => NULL, 'rows' => $rows));
  $form['fieldset_volume_info']['#children'] = $table;

  $output  = drupal_render($form['fieldset_volume_info']);
  $output .= drupal_render($form['cloud_context']);
  $output .= drupal_render($form['submit_buttons']);

  $output .= drupal_render_children($form);

  return $output;
}


/**
 *
 * @param  $form_id
 * @param  $form_values
 * @return
 */
function aws_ec2_lib_create_ebs_volume_validate($form_id, $form_values) {

  $form_values     = $form_values['values'];
  $cloud_context   = $form_values['cloud_context'];
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if ( $form_values['op'] == t('Create') ) {

    if ( empty($form_values['Name_text']) ) {

      form_set_error('', t('You must enter valid Name.'));
    }
    else {
      $query = _aws_ec2_lib_get_volume($cloud_context);

      $query_args = array(
        ':nickname' => $form_values['Name_text'],
        ':cloud_type' => $cloud_context,
      );

      $result = db_query( $query, $query_args );
      
      foreach ($result as $db_obj) {
        $vol_id = $db_obj->volume_id;
        form_set_error('', t('Name is already used by a EBS-Volume.'));
        break;
      }
    }

    if (empty($form_values['Size_text']) ) {

      form_set_error('',    t('You must enter Size'));
    }
  }

  return;
}



/**
 * Create Volume
 *
 * @param  $form_id
 * @param  $form_values
 * @return
 */
function aws_ec2_lib_create_ebs_volume_submit($form_id, $form_values) {

  $form_values = $form_values['values'];
  $cloud_context = $form_values['cloud_context'];
  if ( $form_values['op'] == t('Cancel') ) {

    $cloud_menu_path = cloud_get_menu_path($cloud_context );
    drupal_goto( $cloud_menu_path . '/ebs_volumes');
    return FALSE;

  }
  elseif ($form_values['op'] == t('Create')) {

    $myclient = aws_ec2_api_create_volume( $cloud_context, $form_values['Size_text'], $form_values['zone_select'], $form_values['Snapshot_select'] );
    if ( $myclient->code != 200 ) {
      $startpos = strpos($myclient->data, '<Message>') + strlen('<Message>');
      $endpos   = strpos($myclient->data, '</Message>');
      $msg      = str_replace('GiB', 'GB', substr($myclient->data, $startpos, $endpos -$startpos));

      drupal_set_message( check_plain(t('There was a error. Unable to create Volume. @message', array('@message' => $msg) ) ),    'error');

      return FALSE;
    }

    $result = _aws_ec2_lib_save_new_volume($cloud_context, $myclient, $form_values['Name_text'] );

    if ($result) {
      drupal_set_message(t('Creating Volume.'));
      $cloud_menu_path = cloud_get_menu_path($cloud_context );
      drupal_goto($cloud_menu_path . '/ebs_volumes' );

      return TRUE;
    }
    else {
      drupal_set_message(t('Unable to save Volume: @name', array('@name' => $form_values['Name_text'])));
    }

    return TRUE;
  }
}


/**
 *
 * @param  $cloud_context
 * @return
 */
function aws_ec2_lib_delete_ebs_volume($cloud_context ) {

  $vol_id = isset( $_REQUEST['vol_id'] ) ? $_REQUEST['vol_id'] : FALSE;

  if ( empty($vol_id) ) {

    $cloud_menu_path = cloud_get_menu_path($cloud_context );
    drupal_goto($cloud_menu_path . '/ebs_volumes');
    return;
  }

  $result = aws_ec2_api_delete_volume($cloud_context, $vol_id);
  if ( _aws_ec2_lib_delete_volume($cloud_context, $result, $vol_id ) ) {

    drupal_set_message(t('Volume deleted successfully: @volume_id', array('@volume_id' => $vol_id)));
  }
  else {

    drupal_set_message( check_plain( t('Unable to delete the Volume: @volume_id', array('@volume_id' => $vol_id) ) ), 'error' );
  }

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  drupal_goto($cloud_menu_path . '/ebs_volumes');

  return;
}


/**
 *
 * @param  $cloud_context
 * @return
 */
function aws_ec2_lib_detach_ebs_volume($cloud_context ) {

  $vol_id = isset( $_REQUEST['vol_id'] ) ? $_REQUEST['vol_id'] : FALSE;

  if ( empty($vol_id) ) {

    $cloud_menu_path = cloud_get_menu_path($cloud_context );
    drupal_goto($cloud_menu_path . '/ebs_volumes');

    return;
  }

  $myclient        = aws_ec2_api_detach_volume($cloud_context, $vol_id);
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if ( $myclient->code == 200 ) {

    _aws_ec2_lib_save_attach_volume( $cloud_context, $myclient );
    drupal_set_message(t('Detaching Volume: @vol_id. This may take some time.', array('@vol_id' => $vol_id)));
  }
  else { // There was an error
    drupal_set_message(t('There was a problem while detaching the volume: @vol_id', array('@vol_id' => $vol_id)), 'error');
  }

  $destination = drupal_get_destination();
  drupal_goto( $destination );

  return;
}

/**
 * Display Volume details
 *
 * @param  $form_submit
 *           form object
 * @param  $cloud_context
 *              sub-cloud
 * @return
 */
function aws_ec2_lib_display_ebs_volume($form, $form_submit = '', $cloud_context) {

  $vol_id = isset( $_REQUEST['vol_id'] ) ? $_REQUEST['vol_id'] : FALSE;
  if ( empty($vol_id) ) {

    $cloud_menu_path = cloud_get_menu_path($cloud_context );
    drupal_goto($cloud_menu_path . '/ebs_volumes');

    return;
  }

  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if (empty($vol_id) || strlen($vol_id) == 0  ) {

    drupal_goto( $cloud_menu_path . '/ebs_volumes' );
    return;
  }

  $form['volume_id'           ] = array(
    '#type' => 'hidden'  ,
    '#value' => $vol_id,
  );
  $form['cloud_context'       ] = array(
    '#type' => 'hidden'  ,
    '#value' => $cloud_context,
  );
  $form['fieldset_volume_info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Volume Info'),
  );
  $form['instance_locked'     ] = array(
    '#type' => 'hidden'  ,
    '#value' => '',
  );

  $vol_name = $vol_id;

  $query = _aws_ec2_lib_get_volumes_query($cloud_context);
  $query_args = array(
    ':volume_id' => $vol_id,
    ':cloud_type' => $cloud_context,
  );

  $result          = db_query( $query, $query_args );
  $instance_id     = '';
  $button_disabled = '';
  $lock_status     = FALSE;

  $cnt = 0;

  foreach ($result as $key) {

    $form['vol_name']['vol_name_label'] = array(
      '#type' => 'item',
      '#title' => t('Nickname'),
      '#required' => TRUE,
    );
    $vol_name = $key->nickname;
    $form['vol_name_details'] = array(
      '#prefix' => '<div class="container-inline">',
      '#suffix' => '</div>',
    );

    if (user_access($cloud_context . ' create volume')) {

      $form['vol_name_details']['vol_name_text'] = array(
        '#type' => 'textfield',
        '#default_value' => $key->nickname,
      );
    }
    else {

      $form['vol_name_details']['vol_name_text'] = array(
        '#type' => 'textfield',
        '#default_value' => $key->nickname,
        '#disabled' => TRUE,
      );
    }

    if (user_access($cloud_context . ' create volume')) {
      $form['vol_name_details']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Update'),
      );
    }

    $form['vol_details']['vol_label'    ] = array(
      '#type' => 'item',
      '#title' => t('ID'),
    );
    $form['vol_details']['vol_text'     ] = array(array('#markup' => t( $key->volume_id )));

    $form['vol_details']['vol_Size'     ] = array(
      '#type' => 'item',
      '#title' => t('Size'),
    );
    $form['vol_details']['vol_txt_size'] = array(array('#markup' => t( $key->vol_size )));

    $form['vol_details']['vol_Zone'     ] = array(
      '#type' => 'item',
      '#title' => t('Availability Zone'),
    );
    $form['vol_details']['vol_text_zone'] = array(array('#markup' => t( $key->vol_zone )));

    $form['vol_details']['vol_Server'   ] = array(
      '#type' => 'item',
      '#title' => t('Instance'),
    );
    if (empty($key->instance_id) === FALSE ) {

      $instance_id = $key->instance_id;
      $lock_status = _aws_ec2_lib_get_lock_status_by_id_query($cloud_context, $instance_id);
      $lock_icon = '';
      if (empty($lock_status) === FALSE && $lock_status === 'Lock'  ) {

        $button_disabled = TRUE;
        $form['instance_locked']['#value'] = TRUE;
        $lock_icon = '<a class=lockIcon title=Lock>&nbsp;&nbsp;&nbsp;&nbsp;</a>';
        if (user_access($cloud_context . ' create volume')) {

          // If we want to restrict to update the nickname, uncomment out the following.
//        $form['vol_name_details']['submit'       ]['#disabled']  = TRUE ;
//        $form['vol_name_details']['vol_name_text']['#disabled']  = TRUE ;
        }
      }

      $form['vol_details']['vol_text_server'] = array(array('#markup' => l( $key->instance_nickname, $cloud_menu_path . '/instances/describe/' . $key->instance_id . '/info', array('query' => array('instance_id' => urlencode($key->instance_id)))) . $lock_icon));
    }
    else {

      $form['vol_details']['vol_text_server'] = array(array('#markup' => t( '' )));
    }

    $form['vol_details']['vol_Status'       ] = array(
      '#type' => 'item',
      '#title' => t('Status'),
    );
    $form['vol_details']['vol_text_status'  ] = array(array('#markup' => t( $key->status   )));

    $form['vol_details']['vol_Device'       ] = array(
      '#type' => 'item',
      '#title' => t('Device'),
    );
    $form['vol_details']['vol_text_device'  ] = array(array('#markup' => t( $key->device   )));

    $form['vol_details']['vol_Created'      ] = array(
      '#type' => 'item',
      '#title' => t('Created'),
    );

    $vol_text_created = '' ;
    if (empty($key->created) === FALSE) {
      
      $vol_text_created = format_date(strtotime($key->created), 'short');
    }

    $form['vol_details']['vol_text_created'] = array(array('#markup' => $vol_text_created));

    $cnt = 1;
  }

  if ($cnt == 0) {

    drupal_set_message(t('Volume not present'),    'error' );
    drupal_goto( $cloud_menu_path . '/ebs_volumes' );

    return;
  }

  $form['submit_buttons'] = array(
    '#prefix' => '<span class="clear"></span><div class="container-inline"><div class="buttons">',
    '#type' => 'fieldset',
    '#suffix' => '</div></div>',
  );

  // Create Snapshot Button
  if (user_access($cloud_context . ' create snapshot') ) {

    $location = check_url(url($cloud_menu_path . '/ebs_volumes/create_snapshot')) . '&vol_id=' . $vol_id . '&volume_name=' . urlencode($vol_name);
    $action   = "document.forms[0].action='" . $location . "';";
    $onclick  = $action;
    $form['submit_buttons']['Create Snapshot'] = array(
      '#type' => 'submit',
      '#value' => t('Create Snapshot'),
      '#attributes' => array('onclick' => $onclick),
    );
  }

  // Delete Button
  if (empty($instance_id) && user_access($cloud_context . ' delete volume') ) {

    $location    = check_url(url($cloud_menu_path . '/ebs_volumes/delete')) . '&vol_id=' . urlencode($vol_id)    ;
    $action      = "document.forms[0].action='" . $location . "';";
    $confirm_msg = cloud_get_messagebox('Are you sure you want to delete the Volume "' . $vol_name . '" ?', $action);

    $form['submit_buttons']['Delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#attributes' => array('onclick' => $confirm_msg),
    );
  }

  // Detach Button
  if (user_access($cloud_context . ' detach volume') && empty($instance_id) === FALSE ) {

    $destination = drupal_get_destination();
    $location    = check_url(url($cloud_menu_path . '/ebs_volumes/detach')) . '&' . $destination . '&vol_id=' . urlencode($vol_id)    ;
    $action      = "document.forms[0].action='" . $location . "';";
    $confirm_msg = cloud_get_messagebox('Are you sure you want to detach the Volume "' . $vol_name . '" ?', $action );

    $form['submit_buttons']['Detach'] = array(
      '#type' => 'submit',
      '#value' => t('Detach'),
      '#attributes' => array('onclick' => $confirm_msg),
      '#disabled' => $button_disabled,
    );
  }


  // Volumes List
  $form['submit_buttons']['List Volumes'] = array(
    '#type' => 'submit',
    '#value' => t('List Volumes'),
  );

  return $form;
}


/**
 *
 * @param  $form
 * @return
 */
function theme_aws_ec2_lib_display_ebs_volume($form) {


  if (isset($form['form'])) {

    $form = $form['form'];
  }

  $cloud_context = $form['cloud_context']['#value'];

  $rows = array(
    array(
      drupal_render($form['vol_name'        ]['vol_name_label']    ),
      drupal_render($form['vol_name_details']                      )
        . drupal_render($form['vol_name_details']['vol_name_text'])
        . drupal_render($form['vol_name_details']['submit'        ]),
    ),
    array(
      drupal_render($form['vol_details']['vol_label']       ),
      drupal_render($form['vol_details']['vol_text']       ),
    ),
    array(
      drupal_render($form['vol_details']['vol_Size']        ),
      drupal_render($form['vol_details']['vol_txt_size']    ),
    ),
    array(
      drupal_render($form['vol_details']['vol_Zone']        ),
      drupal_render($form['vol_details']['vol_text_zone']   ),
    ),
    array(
      drupal_render($form['vol_details']['vol_Server']      ),
      drupal_render($form['vol_details']['vol_text_server'] ),
    ),
    array(
      drupal_render($form['vol_details']['vol_Status']      ),
      drupal_render($form['vol_details']['vol_text_status'] ),
    ),
    array(
      drupal_render($form['vol_details']['vol_Device'     ] ),
      drupal_render($form['vol_details']['vol_text_device'] ),
    ),
    array(
      drupal_render($form['vol_details']['vol_Created'     ] ),
      drupal_render($form['vol_details']['vol_text_created'] ),
    ),
  );

  $table = theme('table', array('header' => NULL, 'rows' => $rows));
  $form['fieldset_volume_info']['#children'] = $table;

  //cf. Waning by Coder module: Potential problem: when FAPI element '#type' is set to 'markup' (default), '#value' only accepts filtered text, be sure to use check_plain(), filter_xss() or similar to ensure your $variable is fully sanitized.
  //$form['fieldset_volume_info']['list'] = array('#type' => 'markup', '#value' => $table);

  $output  = drupal_render($form['submit_buttons'      ]);
  $output .= drupal_render($form['fieldset_volume_info']);
  $output .= drupal_render($form['cloud_context'       ]);
  //$output .= drupal_render($form);
  $output .= drupal_render_children($form);

  return $output;
}



/**
 *
 * @param  $form_id
 * @param  $form_values
 * @return
 */
function aws_ec2_lib_display_ebs_volume_validate($form_id, $form_values) {

  $form_values = $form_values['values'];
  if ( $form_values['op'] == t('Update') ) {

    $vol_name = trim( $form_values['vol_name_text'] );
    if ( empty($vol_name) == TRUE) {
      form_set_error('',    t('Please specify Volume Nickname'));
      return;
    }
  }
}


/**
 *
 * @param  $form_id
 * @param  $form_values
 * @return
 */
function aws_ec2_lib_display_ebs_volume_submit($form_id, $form_values) {

  $form_values     = $form_values['values'];
  $cloud_context   = $form_values['cloud_context'];
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if ( $form_values['op'] == t('Update')) {

    $vol_name = trim( $form_values['vol_name_text'] );
    $query = _aws_ec2_lib_update_attach_volume($cloud_context);

    $query->fields(
      array(
      'nickname' => $vol_name,
    ))
      ->condition('volume_id', $form_values['volume_id'], '=')
      ->execute();


    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('Volume has been modified: @volume_id', array('@volume_id' => $form_values['volume_id'])),
      'link' => '',
    )
    );

    drupal_set_message( t('Updated nickname: @volume_name (@volume_id)', array(
      '@volume_name' => $vol_name,
      '@volume_id' => $form_values['volume_id'],
    ) ) );
    drupal_goto( $cloud_menu_path . '/ebs_volumes/describe', array('query' => array('vol_id' => $form_values['volume_id'])));
  }
  return;
}


/**
 *  Create Snapshot
 *
 * @param  $form_submit
 *              Form object
 * @param  $cloud_context
 *              sub-cloud
 * @return
 */
function aws_ec2_lib_create_ebs_volume_snapshot($form, $form_submit = '', $cloud_context) {

  $vol_id = isset( $_REQUEST['vol_id'] )
    ?  $_REQUEST['vol_id']
    :  FALSE;

  if ( empty($vol_id) ) {

    $cloud_menu_path = cloud_get_menu_path($cloud_context );
    drupal_goto($cloud_menu_path . '/ebs_volumes');
    return;
  }

  $volume_name = isset($_REQUEST['volume_name'])
    ?  $_REQUEST['volume_name']
    :  '';

  $form['cloud_context'] = array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );

  $form['SSName_details']['SSName_label'] = array(
    '#type' => 'item',
    '#title' => t('Nickname'),
  );
  $form['SSName_details']['SSName_text'  ] = array('#type' => 'textfield');

  $form['SSName_details']['SSVol_label'  ] = array(
    '#type' => 'item',
    '#title' => t('Volume'),
  );
  $form['SSName_details']['SSVol_text'   ] = array(array('#markup' => filter_xss( $volume_name    )));

  $form['SSName_details']['SSVolId_label'] = array(
    '#type' => 'item',
    '#title' => t('Volume ID'),
  );
  $form['SSName_details']['SSVolId_text'] = array(array('#markup' => t( $vol_id)));

  $form['SSName_details']['SSDesc_label'] = array(
    '#type' => 'item',
    '#title' => t('Description'),
  );
  $form['SSName_details']['SSDesc_text'  ] = array('#type' => 'textarea');

  $form['create_ss_submit_buttons'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<span class="clear"></span><div class="container-inline"><div class="buttons">',
    '#suffix' => '</div></div>',
  );

  $form['create_ss_submit_buttons']['Create'] = array(
    '#type' => 'submit',
    '#value' => t('Create'),
  );
  $form['create_ss_submit_buttons']['Cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
  );

  $form['volume_id']  = array(
    '#type' => 'hidden',
    '#value' => $vol_id,
  );

  return $form;
}


/**
 *
 * @param  $form
 * @return
 */
function theme_aws_ec2_lib_create_ebs_volume_snapshot($form) {


  if (isset($form['form'])) {

    $form = $form['form'];
  }

  $rows = array(
    array(
      drupal_render($form['SSName_details']['SSName_label']),
      drupal_render($form['SSName_details']['SSName_text'  ]),
    ),
    array(
      drupal_render($form['SSName_details']['SSVol_label'  ]),
      drupal_render($form['SSName_details']['SSVol_text'   ]),
    ),
    array(
      drupal_render($form['SSName_details']['SSVolId_label']),
      drupal_render($form['SSName_details']['SSVolId_text']),
    ),
    array(
      drupal_render($form['SSName_details']['SSDesc_label']),
      drupal_render($form['SSName_details']['SSDesc_text'  ]),
    ),
  );

  $output  = theme('table', array('header' => NULL, 'rows' => $rows));
  $output .= drupal_render($form['create_ss_submit_buttons'] );
  $output .= drupal_render($form['volume_id'              ] );
  $output .= drupal_render($form['cloud_context'          ] );
  $output .= drupal_render_children($form);

  return $output;
}


/**
 *
 * @param  $form_id
 * @param  $form_values
 * @return
 */
function aws_ec2_lib_create_ebs_volume_snapshot_validate($form_id, $form_values) {

  $form_values   = $form_values['values'       ];
  $cloud_context = $form_values['cloud_context'];

  if ( $form_values['op'] == t('Create') ) {

    if ( empty($form_values['SSName_text']) ) {

      form_set_error('',    t('You must enter valid Snap Shot name'));
    }
    else {

      $query      = _aws_ec2_lib_get_count_name_snapshots_query($cloud_context);
      $query_args = array(
        ':nickname' => $form_values['SSName_text'],
      );

      $result = db_query( $query, $query_args );

      $t_count =  0;

      foreach ($result as $key) {

        $t_count = $key->count;
      }


      if ($t_count > 0) {

        form_set_error('', t('Name is already used by a EBS-Snapshot.'));
      }
    }
  }

  return;
}


/**
 *
 * @param  $form_id
 * @param  $form_values
 * @return
 */
function aws_ec2_lib_create_ebs_volume_snapshot_submit($form_id, $form_values) {

  $form_values     = $form_values['values'];
  $cloud_context   = $form_values['cloud_context'];
  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  if ( $form_values['op'] == t('Cancel') ) {

    drupal_goto( $cloud_menu_path . '/ebs_volumes');
    return;

  }
  elseif ($form_values['op'] == t('Create')) {

    $myclient =  aws_ec2_api_create_snapshot($cloud_context, $form_values['volume_id'], $form_values['SSDesc_text'] );
    $result   = _aws_ec2_lib_save_create_snapshot($cloud_context, $myclient, $form_values['SSName_text'], $form_values['SSDesc_text'] );

    if ($result) {

      drupal_set_message(t('Initiated create Snapshot request. This may take some time.'));
      drupal_goto($cloud_menu_path . '/ebs_snapshots');

      return;
    }

    else {
      drupal_set_message(t('Unable to create Snapshot'),    'error' );
    }
  }

  return;
}
