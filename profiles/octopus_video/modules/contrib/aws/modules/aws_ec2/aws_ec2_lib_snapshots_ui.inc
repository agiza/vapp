<?php

/**
 * @file
 * aws_ec2_lib Snapshot UI
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */

/**
 * Updated by yas   2011/02/17
 * Updated by yas   2011/02/14
 * Updated by yas   2011/02/11
 * Updated by yas   2011/02/10
 * Updated by yas   2011/02/02
 */

/**
 * Display list of Snapshots
 *
 * @param     $form_submit
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_display_snapshot_list($form_submit = '', $form_state, $cloud_context) {

  drupal_add_js(cloud_get_module_base() . 'js/cloud.js');
  drupal_add_js(drupal_get_path('module', 'aws_ec2_lib')
              . CLOUD_PATH_SEPARATOR
              . 'js/aws_ec2_lib_auto_refresh_list_snapshots.js'
              );

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  $column  = 'nickname';
  $sql_col = 'c.nickname';

  $options = array(
    t('Nickname'),
    t('ID'      ),
    t('Volume'  ),
    t('Status'  ),
  );


  $filter     = isset($_REQUEST['filter'   ]) ? $_REQUEST['filter'   ] : '';
  $filter_col = isset($_REQUEST['operation']) ? $_REQUEST['operation'] : 0  ; // default: Nickname
  $filter     = trim($filter);

  if ( $filter_col == 0 ) {
    $column  = 'Nickname'    ;
    $sql_col = 'c.nickname'    ;
  }
  elseif ($filter_col == 1 ) {
    $column  = 'AWS-ID'      ;
    $sql_col = 'c.snapshot_id';
  }
  elseif ($filter_col == 2 ) {
    $column  = 'Volume ID'   ;
    $sql_col = 'c.volume_id'   ;
  }
  elseif ($filter_col == 3 ) {
    $column  = 'Status'      ;
    $sql_col = 'c.status'      ;
  }

  $query_args = array();
  if ( isset($filter) ) {

    $query_args[] = $sql_col;
    $query_args[] = $filter  ;
    $query_args[] = $cloud_context;
  }
  else {

    $filter       = ' 1 ';
    $query_args[] = ' ';
    $query_args[] = $cloud_context;
  }

  // asort($options);

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

  $form['options'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
    // '#title' => t('Operations'),
  );

  $snapshot_count = _aws_ec2_lib_get_snapshots_list_count($cloud_context);
  $filter_disabled = '';
  if ( $snapshot_count < 2 ) {

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
        'field' => 'c.nickname',
        //'class' => 'nickname-column',
      ),
      array(
        'data' => t('ID'        ),
        'field' => 'c.snapshot_id',
      ),
      array(
        'data' => t('Volume'    ),
        'field' => 'c.volume_id',
      ),
      array(
        'data' => t('Status'    ),
        'field' => 'c.status',
      ),
      array(
        'data' => t('Created'   ),
        'field' => 'created'          ,
        'sort' => 'c.desc',
      ),
      array(
        'data' => t('Actions'   ),
        'class' => 'action-column',
      ),
    ),
  );


  $query  = _aws_ec2_lib_get_describe_snapshots_query($cloud_context);
  $query  = $query
      ->condition('c.cloud_type', $cloud_context, '=')
      ->condition($sql_col, '%' . $filter . '%', 'like')
      ->extend('TableSort')
      ->extend('PagerDefault')
      ->orderByHeader($form['header']['#value'])
      ->limit(AWS_EC2_LIB_PAGER_LIMIT)
      ->range(0, AWS_EC2_LIB_PAGER_LIMIT);

  $result = $query->execute();

  $destination = drupal_get_destination();


  foreach ($result as $ss) {

    $form['Nickname'][$ss->snapshot_id] = array(array('#markup' => l( $ss->nickname, $cloud_menu_path . '/ebs_snapshots/describe',    array('query' => array('ss_id' => urlencode($ss->snapshot_id))) )));
    $form['AWS_ID'  ][$ss->snapshot_id] = array(array('#markup' => t( $ss->snapshot_id                  )));

    if ( empty($ss->vol_nickname) === FALSE ) {

      $form['Volume'][$ss->snapshot_id] =  array(array('#markup' => l($ss->vol_nickname, $cloud_menu_path . '/ebs_volumes/describe', array('query' => array('vol_id' => urlencode($ss->volume_id))))));
    }
    else {

      $form['Volume'][$ss->snapshot_id] = array(array('#markup' => t( $ss->volume_id                    )));
    }
    $form['Status'  ][$ss->snapshot_id] = array(array('#markup' => t( $ss->status . ' ' . $ss->progress )));
    $form['Created'][$ss->snapshot_id] = array(array('#markup' => format_date(strtotime($ss->created), 'short')));
    $form['hdnName'][$ss->snapshot_id] = array(
      '#type' => 'hidden',
      '#value' => addslashes( $ss->nickname ),
    );
  }

  $form['pager'    ] = array('#value' => theme('pager', array('tags' => NULL, 'element' => 0)));
  $form_state['#redirect'] = FALSE;
  $form['#action'] = url($_REQUEST['q']);

  return $form;
}


/**
 *
 * @param     $form
 * @return
 *
 */
function theme_aws_ec2_lib_display_snapshot_list($form) {


  if (isset($form['form'])) { // drupal 7 passed form as an array
    $form = $form['form'];
  }

  $output        = drupal_render($form['options']);
  $cloud_context = $form['cloud_context']['#value'];

  $form['Nickname'       ] = !empty($form['Nickname'       ]) ? $form['Nickname'] : array();

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  $cloud_menu_path = !empty($cloud_menu_path) ? $cloud_menu_path : '';

  $rows = array();
  //$prop['onclick'] = cloud_get_messagebox('Are you sure you want to delete the snapshot') ;
  $prop = array();
  foreach (element_children($form['Nickname']) as $key) {

    $prop['onclick'] = cloud_get_messagebox('Are you sure you want to delete the snapshot "' . trim($form['hdnName'][$key]['#value']) . '" ?');

    $row   = array();
    $row[] = array(
      'data' => drupal_render($form['Nickname'][$key]),
      //'class' => 'nickname-column',
    );
    $row[] = drupal_render($form['AWS_ID'  ][$key]);
    $row[] = drupal_render($form['Volume'  ][$key]);
    $row[] = drupal_render($form['Status'  ][$key]);
    $row[] = drupal_render($form['Created'][$key]);

    $action_data = '';
    if (user_access($cloud_context . ' create volume') ) {

      $action_data .= cloud_display_action(
                          'images/icon_play'                           ,
                          'create volume'                              ,
                          $cloud_menu_path . '/ebs_volumes/create',
                          array('query' => array('ss_id' => urlencode($key)), 'html' => TRUE)
                        );
    }

    if (user_access($cloud_context . ' delete snapshot') ) {
      $action_data .= cloud_display_action(
                       'images/icon_delete',
                       t('Delete'),
                       $cloud_menu_path . '/ebs_snapshots/delete',
                       array('query' => array('ss_id' => urlencode($key)), 'html' => TRUE),
                       $prop['onclick']
                     ); //,  'ss_id=' . urlencode($key) , $prop );

      $action_data .= cloud_display_action( 'images/icon_edit', t('Edit'), $cloud_menu_path . '/ebs_snapshots/describe', array('query' => array('ss_id' => urlencode($key)), 'html' => TRUE));
    }

    $row[]  = array(
      'data' => $action_data,
      'class' => 'action-column',
    );
    $rows[] = $row;
  }

  $table_attr = array();
  $table_attr['id'] = 'snapshots_list_table';
  $table_attr['autoupdate_url'] = url( $cloud_menu_path . '/callback_get_snapshots_list' );
  $output .= theme('table', array('header' => $form['header']['#value'], 'rows' => $rows, 'attributes' => $table_attr));


  $reload_link = l( t('- Refresh Page -'), $cloud_menu_path . '/get_snapshots_data', array() );
  $ref_link = array(
    '#type' => 'item',
    '#prefix' => '<div id="link_reload" align="right">',
    '#suffix' => '</div>',
    '#markup' => $reload_link,
  );
  $output .= theme('pager', array('tags' => array() ));
  $output .= drupal_render($ref_link);
  $output .= drupal_render($form['cloud_context']);
  $output .= drupal_render($form['filter_hdn']);
  $output .= drupal_render($form['operation_hdn']);

  $output .= drupal_render_children($form);

  return $output;
}



function _aws_ec2_lib_callback_get_snapshots_list($cloud_context) {

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

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  $column  = 'nickname';
  $sql_col = 'c.nickname';

  $filter     = isset($_REQUEST['filter'   ]) ? $_REQUEST['filter'   ] : '';
  $filter_col = isset($_REQUEST['operation']) ? $_REQUEST['operation'] : ''; // default: Nickname
  $filter     = trim($filter);


  if ( $filter_col == 0 ) {
    $column  = 'Nickname'     ;
    $sql_col = 'c.nickname'     ;
  }
  elseif ($filter_col == 1 ) {
    $column  = 'AWS-ID'      ;
    $sql_col = 'c.snapshot_id';
  }
  elseif ($filter_col == 2 ) {
    $column  = 'Volume ID'   ;
    $sql_col = 'c.volume_id'   ;
  }
  elseif ($filter_col == 3 ) {
    $column  = 'Status'      ;
    $sql_col = 'c.status'      ;
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

  $table_data = array();

  $table_data['header'] = array(
    '#type' => 'value',
    '#value' => array(
      array(
        'data' => t('Nickname'  ),
        'field' => 'c.nickname',
        //'class' => 'nickname-column',
      ),
      array(
        'data' => t('ID'        ),
        'field' => 'c.snapshot_id',
      ),
      array(
        'data' => t('Volume'    ),
        'field' => 'c.volume_id',
      ),
      array(
        'data' => t('Status'    ),
        'field' => 'c.status',
      ),
      array(
        'data' => t('Created'   ),
        'field' => 'c.created'          ,
        'sort' => 'desc',
      ),
      array(
        'data' => t('Actions'   ),
        'class' => 'action-column',
      ),
    ),
  );


  $query  = _aws_ec2_lib_get_describe_snapshots_query($cloud_context);
  $query = $query
      ->condition('c.cloud_type', $cloud_context, '=')
      ->condition($sql_col, '%' . $filter . '%', 'like')
      ->extend('TableSort')
      ->extend('PagerDefault')
      ->orderByHeader($table_data['header']['#value'])
      ->limit(AWS_EC2_LIB_PAGER_LIMIT)
      ->range(0, AWS_EC2_LIB_PAGER_LIMIT);


  $result = $query->execute();

  $table_data['Nickname'] = array();

  foreach ($result as $ss) {

    $table_data['Nickname'][$ss->snapshot_id] = array(array('#markup' => l( $ss->nickname, $cloud_menu_path . '/ebs_snapshots/describe',    array('query' => array('ss_id' => urlencode($ss->snapshot_id))) )));
    $table_data['AWS_ID'  ][$ss->snapshot_id] = array(array('#markup' => t( $ss->snapshot_id                  )));
    if ( empty($ss->vol_nickname) === FALSE ) {

      $table_data['Volume'  ][$ss->snapshot_id] =  array(array('#markup' => l($ss->vol_nickname, $cloud_menu_path . '/ebs_volumes/describe', array('query' => array('vol_id' => urlencode($ss->volume_id))))));
    }
    else {

      $table_data['Volume'  ][$ss->snapshot_id] = array(array('#markup' => t( $ss->volume_id                    )));
    }
    $table_data['Status'  ][$ss->snapshot_id] = array(array('#markup' => t( $ss->status . ' ' . $ss->progress )));
    $table_data['Created'][$ss->snapshot_id] = array(array('#markup' => format_date(strtotime($ss->created), 'short')));
    $table_data['hdnName'][$ss->snapshot_id] = array(
      '#type' => 'hidden',
      '#value' => addslashes( $ss->nickname ),
    );
  }


  //Theme for the table data
  $output = '';
  $rows   = array();
  $prop   = array();

  foreach (element_children($table_data['Nickname']) as $key) {

    $prop['onclick'] = cloud_get_messagebox('Are you sure you want to delete the snapshot "' . trim($table_data['hdnName'][$key]['#value']) . '" ?');

    $row   = array();
    $row[] = array(
      'data' => drupal_render($table_data['Nickname'][$key]),
      //'class' => 'nickname-column',
    );
    $row[] = drupal_render($table_data['AWS_ID'  ][$key]);
    $row[] = drupal_render($table_data['Volume'  ][$key]);
    $row[] = drupal_render($table_data['Status'  ][$key]);
    $row[] = drupal_render($table_data['Created'][$key]);

    $action_data = '';
    if (user_access($cloud_context . ' create volume') ) {

      $action_data .= cloud_display_action(
                          'images/icon_play'                       ,
                          t('Create Volume from This Snapshot'),
                          $cloud_menu_path . '/ebs_volumes/create',
                          array('query' => array('ss_id' => urlencode($key)), 'html' => TRUE)
                        );
    }

    if (user_access($cloud_context . ' delete snapshot') ) {
      $action_data .= cloud_display_action(
                       'images/icon_delete',
                       t('Delete'),
                       $cloud_menu_path . '/ebs_snapshots/delete',
                       array('query' => array('ss_id' => urlencode($key)), 'html' => TRUE),
                       $prop['onclick']
                     ); //,  'ss_id=' . urlencode($key) , $prop );

      $action_data .= cloud_display_action( 'images/icon_edit', t('Edit'), $cloud_menu_path . '/ebs_snapshots/describe', array('query' => array('ss_id' => urlencode($key)), 'html' => TRUE));
    }

    $row[]  = array(
      'data' => $action_data,
      'class' => 'action-column',
    );
    $rows[] = $row;
  }

  $output .= theme('table', array('header' => $table_data['header']['#value'], 'rows' => $rows));

  // Send only the body do not send the headers
  $index_start = strrpos( $output, '<tbody>'  );
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
 * @param     $form_id
 * @param     $form_values
 *
 */
function aws_ec2_lib_display_snapshot_list_submit($form_id, &$form_values) {

  $form_values   = $form_values['values'];
  $cloud_context = $form_values['cloud_context'];
  if ( $form_values['op'] == t('Apply') ) {

    $param_arr = array(
      'filter' => $form_values['filter'],
      'operation' => $form_values['operation'],
    );

    $destination = drupal_get_destination();
    $form_values['redirect']     = array($destination['destination'], array('query' => $param_arr));
  }

}

/**
 * Delete a Snapshot
 *
 * @param  $cloud_context
 * @return
 */
function aws_ec2_lib_delete_snapshot($cloud_context) {

  $ss_id = isset( $_REQUEST['ss_id'] ) ? $_REQUEST['ss_id'] : '';
  $tmp_arr = array();
  $tmp_arr[1] = $ss_id;
  $result = aws_ec2_api_delete_snapshot($cloud_context, $tmp_arr);
  if ( _aws_ec2_lib_delete_snapshot_db($cloud_context, $result, $ss_id ) ) {

    drupal_set_message(  check_plain(t('Deleting Snapshot: @snapshot_id.  This may take some time.', array('@snapshot_id' => $ss_id) ) ) );
  }
  else {

    drupal_set_message(  check_plain(t('Unable to delete the Snapshot: @snapshot_id', array('@snapshot_id' => $ss_id) ) ),    'error');
  }

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  drupal_goto($cloud_menu_path . '/ebs_snapshots');

  return;
}




/**
 * Display Snapshot details info
 *
 * @param     $form_submit
 * @param     $cloud_context
 * @return
 *
 */
function aws_ec2_lib_display_snapshot_info($form, $form_submit = '', $cloud_context) {

  $ss_id = isset( $_REQUEST['ss_id'] ) ? $_REQUEST['ss_id'] : '';
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if (empty($ss_id) ) {

    drupal_goto( $cloud_menu_path . '/ebs_snapshots' );

    return;
  }

  $form['fieldset_snapshot_info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Snapshot Info'),
  );

  $form['ss_id'] = array(
    '#type' => 'hidden',
    '#value' => $ss_id,
  );
  $form['cloud_context'] = array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );

  $snapshot_data = _aws_ec2_lib_get_snapshots_by_id_db( $cloud_context, $ss_id );

  $form['ss_name_label'] = array(
    '#type' => 'item',
    '#title' => t('Nickname'),
  );
  $ss_name = $snapshot_data['snap_nickname'];
  $form['ss_name_details'] = array(
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );

  if (user_access($cloud_context . ' create snapshot') ) {

    $form['ss_name_details']['ss_name_text'] = array(
      '#type' => 'textfield',
      '#default_value' => $ss_name,
    );

    $form['ss_name_details']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Update'),
    );
  }
  else {

    $form['ss_name_details']['ss_name_text'] = array(
      '#type' => 'textfield',
      '#default_value' => $ss_name,
      '#disabled' => TRUE,
    );
  }

  $form['ss_label'] = array(
    '#type' => 'item',
    '#title' => t('ID'),
  );
  $form['ss_text'] = array(array(
      '#markup' => t( $snapshot_data['snapshot_id'] ),
    ));

  $form['vol_label']         = array(
    '#type' => 'item',
    '#title' => t('Volume'),
  );
  $vol_nickname = $snapshot_data['vol_nickname'];
  $vol_id = $snapshot_data['snap_volume_id'];
  if ( empty($vol_nickname) ) {

    $vol_nickname     = $vol_id;
    $form['vol_text'] = array(array('#markup' => t( $vol_nickname )));
  }
  else {

    $form['vol_text']  =  array(
      array(
        '#markup' => l($vol_nickname,
                    $cloud_menu_path . '/ebs_volumes/describe',
                    array('query' => array('vol_id' => urlencode($vol_id)))),
      ),
    );
  }

  $form['ss_vol_size_label'] = array(
    '#type' => 'item',
    '#title' => t('Volume Size'),
  );
  $form['ss_vol_size_text'] = array(array(
      '#markup' => t( $snapshot_data['vol_size'] ),
    ));

  $form['ss_status_label'] = array(
    '#type' => 'item',
    '#title' => t('Status'),
  );
  $form['ss_status_text'] = array(array(
      '#markup' => t( $snapshot_data['snap_status'] ),
    ));

  $form['ss_created_label'] = array(
    '#type' => 'item',
    '#title' => t('Created'),
  );
  $form['ss_created_text'] = array(array(
      '#markup' => format_date(strtotime($snapshot_data['snap_created']), 'short'),
    ));

  $form['submit_buttons'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<span class="clear"></span><div class="container-inline"><div class="buttons">',
    '#suffix' => '</div></div>',
  );

  // Delete Button
  if (user_access($cloud_context . ' create volume') ) {

    $location = check_url(url($cloud_menu_path . '/ebs_volume/create')) . '&ss_id=' . urlencode($ss_id)    ;
    $form['submit_buttons']['create'      ] = array(
      '#type' => 'submit',
      '#value' => t('Create Volume'),
    );
  }

  // Delete Button
  if (user_access($cloud_context . ' delete volume') ) {

    $location    = check_url(url($cloud_menu_path . '/ebs_snapshots/delete')) . '&ss_id=' . urlencode($ss_id)    ;
    $action      = "document.forms[0].action='" . $location . "';";
    $confirm_msg = cloud_get_messagebox('Are you sure you want to delete the Snapshot "' . $ss_name . '" ?', $action );
    $form['submit_buttons']['delete'      ] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#attributes' => array('onclick' => $confirm_msg),
    );
  }

  $form['submit_buttons']['snapshots_list'] = array(
    '#type' => 'submit',
    '#value' => t('List Snapshots'),
  );

  return $form;
}


/**
 *
 * @param     $form
 * @return
 *
 */
function theme_aws_ec2_lib_display_snapshot_info($form) {


  if (isset($form['form'])) { // drupal 7 passed form as an array
    $form = $form['form'];
  }

  $cloud_context = $form['cloud_context']['#value'];

  $rows = array(
    array(
      drupal_render($form['ss_name_label'                  ]),
      drupal_render($form['ss_name_details']                )
    . drupal_render($form['ss_name_details']['ss_name_text'])
    . drupal_render($form['ss_name_details']['submit'      ]),
    ),
    array(
      drupal_render($form['ss_label'        ]),
      drupal_render($form['ss_text'         ]),
    ),
    array(
      drupal_render($form['vol_label'       ]),
      drupal_render($form['vol_text'        ]),
    ),
    array(
      drupal_render($form['ss_vol_size_label']),
      drupal_render($form['ss_vol_size_text'  ]),
    ),
    array(
      drupal_render($form['ss_status_label']),
      drupal_render($form['ss_status_text'  ]),
    ),
    array(
      drupal_render($form['ss_created_label']),
      drupal_render($form['ss_created_text']),
    ),
  );

  $table = theme('table', array('header' => NULL, 'rows' => $rows));
  $form['fieldset_snapshot_info']['#children'] = $table;

  //cf. Waning by Coder module: Potential problem: when FAPI element '#type' is set to 'markup' (default), '#value' only accepts filtered text, be sure to use check_plain(), filter_xss() or similar to ensure your $variable is fully sanitized.
  //$form['fieldset_snapshot_info']['list'] = array('#type' => 'markup', '#value' => $table);

  $output  = drupal_render($form['submit_buttons'        ]);
  $output .= drupal_render($form['fieldset_snapshot_info']);
  $output .= drupal_render($form['cloud_context'         ]);
  //$output .= drupal_render($form);
  $output .= drupal_render_children($form);

  return $output;
}


/**
 *
 * @param     $form_id
 * @param     $form_values
 * @return
 *
 */
function aws_ec2_lib_display_snapshot_info_validate($form_id, $form_values) {

  $form_values = $form_values['values'];
  if ( $form_values['op'] == t('Update') ) {

    if ( empty( $form_values['ss_name_text']) == TRUE) {
      form_set_error('',    t('Please specify Snapshot Nickname'));
      return;
    }
  }
}



/**
 *
 * @param     $form_id
 * @param     $form_values
 * @return
 *
 */
function aws_ec2_lib_display_snapshot_info_submit($form_id, $form_values) {

  $form_values     = $form_values['values'];
  $cloud_context   = $form_values['cloud_context'];
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if ( $form_values['op'] == t('Create Volume')) {

    drupal_goto( $cloud_menu_path . '/ebs_volumes/create', array('query' => array('ss_id' => $form_values['ss_id'])));
  }
  elseif ( $form_values['op'] == t('Update')) {

    _aws_ec2_lib_snapshot_update( $cloud_context, $form_values['ss_id'], 'nickname', $form_values['ss_name_text']  );

    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('Snapshot has been modified: @snapshot_id', array('@snapshot_id' => $form_values['ss_id'])),
      'link' => '',
    )
    );

    drupal_set_message(  check_plain(t('Updated nickname: @snapshot_nickname (@snapshot_id)', array(
      '@snapshot_nickname' => $form_values['ss_name_text'],
      '@snapshot_id' => $form_values['ss_id'],
    ))));
    drupal_goto( $cloud_menu_path . '/ebs_snapshots' );
  }
  elseif ( $form_values['op'] == t('List Snapshots')) {

    $cloud_menu_path = cloud_get_menu_path($cloud_context);
    drupal_goto($cloud_menu_path . '/ebs_snapshots');
  }
  return;
}
