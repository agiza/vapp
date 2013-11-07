<?php

/**
 * @file
 * aws_ec2_lib UI for Elastic IP
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */

/**
 * Updated by yas   2011/10/03
 * Updated by jiten 2011/09/07
 * Updated by yas   2011/03/23
 * Updated by yas   2011/02/17
 * Updated by yas   2011/02/14
 * Updated by yas   2011/02/11
 * Updated by yas   2011/02/10
 * Updated by yas   2011/02/02
 */

/**
 * Display details of Elastic IP
 *
 * @param   $form_submit
 * @param   $cloud_context
 * @return
 */
function aws_ec2_lib_display_elastic_ip($form, $form_submit = '', $cloud_context) {


  $ip = isset( $_REQUEST['ip'] ) ? $_REQUEST['ip'] : FALSE;

  if (empty($ip) || strlen($ip) == 0  ) {

    $cloud_menu_path = cloud_get_menu_path( $cloud_context );
    drupal_goto( $cloud_menu_path . '/elastic_ips' );
    return;
  }

  $form['fieldset_elastic_ip_info']  = array(
    '#type' => 'fieldset',
    '#title' => t('Elastic IP Info'),
  );
  $form['ip'                      ]  = array(
    '#type' => 'hidden',
    '#value' => $ip,
  );
  $form['instance_locked'         ]  = array(
    '#type' => 'hidden'  ,
    '#value' => '',
  );

  $query            = _aws_ec2_lib_get_elastic_ips_query($cloud_context);
  $query_args       = array(
    ':public_ip' => $ip,
    ':cloud_type' => $cloud_context,
  );
  $button_disabled  = '';

  $result           = db_query( $query, $query_args );


  foreach ($result as $key) {

    $form['IP_name']['IP_Name_label'] = array(
      '#type' => 'item',
      '#title' => t('Nickname'),
    );

    $instance_id = $key->instance_id;
    $lock_status = _aws_ec2_lib_get_lock_status_by_id_query($cloud_context, $instance_id);
    $lock_icon   = '';

    if (empty($lock_status) === FALSE
      &&          $lock_status  === 'Lock'  ) {

      $button_disabled                   = TRUE;
      $form['instance_locked']['#value'] = TRUE;
      $lock_icon                         = '<a class=lockIcon title=Lock>&nbsp;&nbsp;&nbsp;&nbsp;</a>';
    }

    $form['IP_name_details'] = array(
      '#prefix' => '<div class="container-inline">',
      '#suffix' => '</div>',
    );

    if (user_access($cloud_context . ' add IP') && empty($button_disabled) ) {

      $form['IP_name_details']['IP_Name_text'] = array(
        '#type' => 'textfield',
        '#default_value' => $key->public_ip_name,
      );
    }
    else {

      $form['IP_name_details']['IP_Name_text'] = array(
        '#type' => 'textfield',
        '#default_value' => $key->public_ip_name,
        // If we want to restrict to update the nickname, uncomment out the following.  //      '#disabled' => TRUE
      );
    }

    if (user_access($cloud_context . ' add IP') && empty($button_disabled) ) {

      $form['IP_name_details']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Update'),
      );
    }
    else {

      $form['IP_name_details']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Update'),
        // If we want to restrict to update the nickname, uncomment out the following.  //      '#disabled'  => TRUE
      );
    }

    $form['IP_details']['IP_label'          ] = array(
      '#type' => 'item',
      '#title' => t('IP'),
    );
    $form['IP_details']['IP_text'           ] = array(array('#markup' => t( $key->public_ip )));
    $form['IP_Instance']['IP_instance_label'] = array(
      '#type' => 'item',
      '#title' => t('Instance'),
    );

    $cloud_menu_path = cloud_get_menu_path( $cloud_context );

    if ( empty($key->instance_nickname) ) {

      $form['IP_Instance']['IP_instance_text'] = array(array('#markup' => t( 'available' )));
    }
    else {

      $form['IP_Instance']['IP_instance_text'] = array(array('#markup' => l( $key->instance_nickname, $cloud_menu_path . '/instances/describe/' . $key->instance_id . '/info', array('query' => array('instance_id' => urlencode($key->instance_id)))) . $lock_icon));
    }
  }


  $form['submit_buttons'] = array(
    '#prefix' => '<span class="clear"></span><div class="container-inline"><div class="buttons">',
    '#type' => 'fieldset',
    '#suffix' => '</div></div>',
  );

  // Delete Button
  if (user_access($cloud_context . ' delete IP')) {
    $confirm_msg    = cloud_get_messagebox('Are you sure you want to delete the Elastic IP "' . $ip . '" ?');
    $location       = check_url(url( $cloud_menu_path . '/elastic_ips/delete')) . '&ip=' . urlencode($ip);
    $action         = "document.forms[0].action='" . $location . "';";
    $onclick        = $action . $confirm_msg;

    $form['submit_buttons']['Delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#attributes' => array('onclick' => $onclick),
      '#disabled' => $button_disabled,
    );
  }

  // List Elastip IPs button
  if (user_access($cloud_context . ' list IPs')) {

    $form['submit_buttons']['List Elastic IPs'] = array(
      '#type' => 'submit',
      '#value' => t('List Elastic IPs'),
    );
  }

  $form['cloud_context'] =  array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  )  ;

  return $form;
}

/**
 *
 * @param   $form
 * @return
 */
function theme_aws_ec2_lib_display_elastic_ip($form) {


  if (isset($form['form'])) {

    $form = $form['form'];
  }

  $cloud_context = $form['cloud_context']['#value'];

  $rows = array(
    array(
      drupal_render($form['IP_name'        ]['IP_Name_label'    ] ),
      drupal_render($form['IP_name_details']                      )
    . drupal_render($form['IP_name_details']['IP_Name_text'     ] )
    . drupal_render($form['IP_name_details']['submit'           ] ),
    ),
    array(
      drupal_render($form['IP_details'    ]['IP_label'         ] ),
      drupal_render($form['IP_details'    ]['IP_text'          ] ),
    ),
    array(
      drupal_render($form['IP_Instance'   ]['IP_instance_label'] ),
      drupal_render($form['IP_Instance'   ]['IP_instance_text'] ),
    ),
  );

  $table = theme('table', array('header' => NULL, 'rows' => $rows));
  $form['fieldset_elastic_ip_info']['#children'] = $table;

  //cf. Waning by Coder module: Potential problem: when FAPI element '#type' is set to 'markup' (default), '#value' only accepts filtered text, be sure to use check_plain(), filter_xss() or similar to ensure your $variable is fully sanitized.
  //$form['fieldset_elastic_ip_info']['list'] = array('#type' => 'markup', '#value' => $table);

  $output  = drupal_render($form['submit_buttons']);
  $output .= drupal_render($form['fieldset_elastic_ip_info']);
  $output .= drupal_render($form['ip']);
  $output .= drupal_render($form['cloud_context']);
  //$output .= drupal_render($form);
  $output .= drupal_render_children($form);

  return $output;
}


/**
 *
 * @param   $form_id
 * @param   $form_values
 * @return
 */
function aws_ec2_lib_display_elastic_ip_validate($form_id, $form_values) {

  $form_values = $form_values['values'];
  if ( $form_values['op'] == t('Update') ) {

    if ( empty( $form_values['IP_Name_text']) == TRUE) {
      form_set_error('',    t('Please specify IP Nickname'));
      return;
    }
  }
}


/**
 *
 * @param   $form_id
 * @param   $form_values
 * @return
 */
function aws_ec2_lib_display_elastic_ip_submit($form_id, $form_values) {

  $form_values          = $form_values['values'       ];
  $cloud_context        = $form_values['cloud_context'];
  $cloud_menu_path      = cloud_get_menu_path($cloud_context );

  if ( $form_values['op'] == t('Update')) {

    $query = _aws_ec2_lib_update_ip_query($cloud_context);

    $ip = $form_values['ip'];
    $query->fields(array(
      'public_ip_name' => $form_values['IP_Name_text'],
    ))
      ->condition('public_ip', $ip, '=')
      ->condition('cloud_type', $cloud_context, '=')
      ->execute();

    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('IP has been modified: @ip', array('@ip' => $ip)),
      'link' => '',
    )
    );

    drupal_set_message(t('Updated Elastic IP nickname: @ip', array('@ip' => $ip)) );
    drupal_goto( $cloud_menu_path . '/elastic_ips' );
  }
  elseif ( $form_values['op'] == t('List Elastic IPs')) {

    $cloud_menu_path = cloud_get_menu_path($cloud_context);
    drupal_goto($cloud_menu_path . '/elastic_ips');
  }

  return;
}


/**
 *
 *  Display List of Elastic IP's
 *
 * @param   $form_submit
 *              form object
 * @param   $cloud_context
 *              sub-cloud of whcih IPs are to be displayed
 * @return
 */
function aws_ec2_lib_display_elastic_ip_list($form_submit = '', $form_state, $cloud_context) {

  drupal_add_js(cloud_get_module_base() . 'js/cloud.js');

  $options = array(
    t('Nickname'  ),
    t('IP Address'),
    t('Instance'  ),
  );
  $query_args = array();


  $filter     = isset($_REQUEST['filter'   ]) ? $_REQUEST['filter'   ] : '';
  $filter_col = isset($_REQUEST['operation']) ? $_REQUEST['operation'] : 0  ; // default: Nickname
  $filter     = trim($filter);


  if ( $filter_col == 0 ) {
    $column  = 'Nickname'       ;
    $sql_col = 'public_ip_name';
  }
  elseif ($filter_col == 1 ) {
    $column  = 'IP Address'     ;
    $sql_col = 'public_ip'      ;
  }
  elseif ($filter_col == 2 ) {
    $column  = 'Instance'       ;
    $sql_col = !empty($filter)
      ? 'instance_nickname'
      : 'public_ip_name';
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

  // asort($options);

  $form['options'] = array(
    '#prefix' => '<div class="container-inline">',
    '#type' => 'fieldset',
    '#suffix' => '</div>'  ,
    // '#title' => t('Operations'),
  );

  $elastic_ip_count = _aws_ec2_lib_get_elastic_ips_list_count($cloud_context);
  $filter_disabled  = '';
  if ( $elastic_ip_count < 2 ) {

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

  // Create button
  if (user_access($cloud_context . ' add IP')) {
    $form['options']['new_ip'] = array(
      '#type' => 'submit',
      '#value' => t('Create'),
    );
  }

  $form['cloud_context'] = array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );

  $form['header'] = array(
    '#type' => 'value',
    '#value' => array(
      array(
        'data' => t('Nickname'  ),
        'field' => 'public_ip_name'      ,
        //'class' => 'nickname-column'     ,
        'sort' => 'asc',
      ),
      array(
        'data' => t('IP Address'),
        'field' => 'public_ip',
      ),
      array(
        'data' => t('Instance'  ),
        'field' => 'instance_nickname',
      ),
      array(
        'data' => t('Actions'   ),
        'class' => 'action-column',
      ),
    ),
  );



  $query    = _aws_ec2_lib_get_describe_elastic_ips_query($cloud_context);

  $query
      ->condition('c.cloud_type', $cloud_context, '=')
      ->condition($sql_col, '%' . $filter . '%', 'like')
      ->extend('TableSort')
      ->orderByHeader($form['header']['#value']);

  $result = $query->execute();

  $destination = drupal_get_destination();

  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  foreach ($result as $ips) {

    // $form['Nickname'][$ips->public_ip]   = array( array('#value' => t( $ips->public_ip_name ) ) );
    $form['Nickname'][$ips->public_ip] = array(array('#markup' => l( $ips->public_ip_name, $cloud_menu_path . '/elastic_ips/describe',    array('query' => array('ip' => urlencode($ips->public_ip)))))); //,  array(),  'ip=' . urlencode($ips->public_ip) ,  NULL,  FALSE,  TRUE ) ) );
    $form['hdnName'][$ips->public_ip] = array(
      '#type' => 'hidden',
      '#value' => addslashes($ips->public_ip_name),
    );
    $form['Address'][$ips->public_ip] = array(array('#markup' => t( $ips->public_ip )));
    $form['Instance'][$ips->public_ip] = array(array('#markup' => l( $ips->instance_nickname, $cloud_menu_path . '/instances/describe/' . $ips->instance_id . '/info', array('query' => array('instance_id' => urlencode($ips->instance_id))))));

    if (empty($ips->instance_id) === FALSE ) {

      $lock_status = _aws_ec2_lib_get_lock_status_by_id_query($cloud_context, $ips->instance_id);
      $form['locked'  ][$ips->public_ip] = array(
        '#type' => 'hidden',
        '#value' => $lock_status,
      );
      if ($lock_status === 'Lock' ) {
        $form['Nickname'][$ips->public_ip][0]['#markup'] .= '<a class=lockIcon title=Lock>&nbsp;&nbsp;&nbsp;&nbsp;</a>';
      }
    }
    else {

      $form['locked'  ][$ips->public_ip] = array(
        '#type' => 'hidden',
        '#value' => 'Available',
      );
    }
  }

  $form['pager'        ] = array('#value' => theme('pager', array('tags' => NULL, 'element' => 0)));
  $form['#redirect'    ] = FALSE;
  $form['#action'] = url($_REQUEST['q']);
  $form['cloud_context'] = array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  )  ;

  return $form;
}


/**
 *
 * @param   $form
 * @return
 */
function theme_aws_ec2_lib_display_elastic_ip_list($form) {


  if (isset($form['form'])) { // drupal 7 passed form as an array
    $form = $form['form'];
  }

  $cloud_context   = $form['cloud_context']['#value'];
  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  $cloud_menu_path = !empty($cloud_menu_path) ? $cloud_menu_path : '';

  $output  = drupal_render($form['options']);
  $output .= drupal_render($form['new_ip']);

  $form['Nickname']        = !empty($form['Nickname']) ? $form['Nickname'] : array();
  $form['pager']['#value'] = !empty($form['pager']['#value']) ? $form['pager']['#value'] : '';

  $rows = array();
  foreach (element_children($form['Nickname']) as $ip) {

    $row = array(
      array(
        'data' => drupal_render($form['Nickname'][$ip] ),
        //'class' => 'nickname-column',
      ),
      drupal_render($form['Address'][$ip] ),
      drupal_render($form['Instance'][$ip] ),
    );

    $prop['onclick'] = cloud_get_messagebox('Are you sure you want to delete the Elastic IP "' . $form['hdnName'][$ip]['#value'] . '" ?');
    if ($form['locked'][$ip]['#value'] !== 'Available' && $form['locked'][$ip]['#value'] !== 'Lock'    && user_access($cloud_context . ' delete IP')) {
      $action_data = cloud_display_action(
        'images/icon_delete',
        t('Delete'),
        $cloud_menu_path . '/elastic_ips/delete',
        array(
        'query' => array('ip' => urlencode($ip)),
        'html' => TRUE,
      ),
        $prop['onclick']
      ); //,  'ip=' . urlencode($ip) , $prop );

      $action_data .= cloud_display_action( 'images/icon_edit', t('Edit'), $cloud_menu_path . '/elastic_ips/describe', array('query' => array('ip' => urlencode($ip)), 'html' => TRUE));
      $row[] = array(
        'data' => $action_data,
        'class' => 'action-column',
      );
    }
    elseif (user_access($cloud_context . ' delete IP')) {

      $action_data  = cloud_display_action( 'images/icon_edit', t('Edit'), $cloud_menu_path . '/elastic_ips/describe', array('query' => array('ip' => urlencode($ip)), 'html' => TRUE));
      $row[]        = array(
        'data' => $action_data,
        'class' => 'action-column',
      );
    }
    else {
      $row[] = '';
    }
    $rows[] = $row;
  }

  $output .= theme('table', array('header' => $form['header']['#value'], 'rows' => $rows));

  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $reload_link = l(t('- Refresh Page -'), $cloud_menu_path . '/get_elastic_ips_data',    array() );
  $ref_link = array(
    '#type' => 'item',
    '#prefix' => '<div id="link_reload" align="right">',
    '#markup' => $reload_link,
    '#suffix' => '</div>',
  );

  $output .= drupal_render($ref_link);
  $output .= drupal_render($form['cloud_context']);
  $output .= drupal_render_children($form);

  return $output;
}


/**
 *
 * @param   $form_id
 * @param   $form_values
 * @return
 */
function aws_ec2_lib_display_elastic_ip_list_submit($form_id, &$form_values) {

  $form_values   = $form_values['values'       ];
  $cloud_context = $form_values['cloud_context'];

  if ( $form_values['op'] == t('Create') ) {

    $myclient = aws_ec2_api_allocate_address($cloud_context);

    if ( $myclient->code == 200 ) {

      $response = _aws_ec2_lib_save_allocate_address($cloud_context, $myclient);
      if ( $response ) {

        drupal_set_message(t('Created Elastic IP.'));
      }
      else {

        drupal_set_message(t('There was a problem while creating IP.'),    'error');
      }
    }
    else { // There was an error while creating IP
      drupal_set_message(t('There was a problem while creating IP.'),    'error');
    }

    $cloud_menu_path = cloud_get_menu_path( $cloud_context );
    drupal_goto( $cloud_menu_path . '/elastic_ips' ); // Goto the Listing Page
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
 * Delete Elastic IP
 *
 * @param   $cloud_context
 *              Sub-cloud of which IP is to be deleted
 * @return
 */
function aws_ec2_lib_delete_elastic_ip($cloud_context) {


  $ip = isset( $_REQUEST['ip'] ) ? $_REQUEST['ip'] : FALSE;

  $query_args = array();
  $instance_id = FALSE;
  $query = _aws_ec2_lib_get_elastic_ips_query($cloud_context);
  $query_args       = array(
    ':public_ip' => $ip,
    ':cloud_type' => $cloud_context,
  );

  $result = db_query( $query, $query_args );

  foreach ($result as $key) {

    $instance_id = $key->instance_id;
  }

  if ( $instance_id !== FALSE ) { // IP assigned to Instance : Check whether instance is locked
    $is_locked = _aws_ec2_lib_get_lock_status_by_id_query( $cloud_context, $instance_id );
    if ($is_locked === 'Lock' ) { // Instance is locked. IP cannot be deleted
      drupal_set_message( check_plain(t('Instance associated with the IP "@IP" is locked.  Cannot delete the IP', array('@IP' => $ip))), 'error')  ;
      $cloud_menu_path = cloud_get_menu_path($cloud_context );
      drupal_goto( $cloud_menu_path . '/elastic_ips' );
      return;
    }
  }

  $myclient = aws_ec2_api_release_address($cloud_context, $ip);

  if ( $myclient->code == 200 ) {

    $response = _aws_ec2_lib_release_address_save($cloud_context, $myclient, $ip );
    if ($response) {
      drupal_set_message(t('Deleted Elastic IP: @ip', array('@ip' => $ip)));
    }

    else {
      drupal_set_message(t('Unable to delete the IP: @ip', array('@ip' => $ip)), 'error');
    }
  }
  else { // There was an error
    drupal_set_message(t('Unable to delete the IP: @ip', array('@ip' => $ip)), 'error');
  }

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  drupal_goto( $cloud_menu_path . '/elastic_ips' );

  return;
}
