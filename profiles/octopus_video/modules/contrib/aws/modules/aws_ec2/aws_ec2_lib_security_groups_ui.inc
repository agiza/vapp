<?php

/**
 * @file
 * aws_ec2_lib Security Group UI
 *
 * Copyright (c) 2010-2011 DOCOMO Innovations, Inc.
 *
 */

/**
 * Updated by yas   2011/02/14
 * Updated by yas   2011/02/10
 * Updated by yas   2011/02/06
 * Updated by yas   2011/02/02
 */

/**
 *
 * @param     $form_submit
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_display_security_group($form_submit = '', $form_state, $cloud_context) {

  $group_name = isset( $_REQUEST['group_name'] ) ? $_REQUEST['group_name'] : FALSE;

  $cloud_menu_path = cloud_get_menu_path( $cloud_context );
  if (empty($group_name) || strlen($group_name) == 0  ) {

    drupal_goto( $cloud_menu_path . '/security_groups' );
    return;
  }

  $txt_disabled = '';
  if (user_access($cloud_context . ' setup security group') === FALSE ) {

    $txt_disabled = TRUE;
  }

  $operation_type = cloud_get_filter_value( $form_state, 'AddIPs_TCP_Button' );
  if ( empty($operation_type) ) {

    $operation_type = cloud_get_filter_value( $form_state, 'AddIPs_ICMP_Button' );
    if ( empty($operation_type) ) {

      $operation_type = cloud_get_filter_value( $form_state, 'Add_Grp_Button' );
      if ( empty($operation_type) ) {

        $operation_type = FALSE;
      }
      else {

        $operation_type = 'Add_Grp_Button';
      }
    }
    else {

      $operation_type = 'AddIPs_ICMP_Button';
    }
  }
  else {

    $operation_type = 'AddIPs_TCP_Button';
  }

  $form['operation_type'] = array(
    '#type' => 'hidden',
    '#value' => $operation_type,
  );
  $form['cloud_context'] = array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );

  $query        = _aws_ec2_lib_get_security_group_query($cloud_context);
  $query_args   = array(
    ':group_name' => $group_name,
    ':cloud_type' => $cloud_context,
  );


  $result = db_query( $query, $query_args );
  $owner_id = '';

  $spacer = array(
    '#type' => 'image_button',
    '#src' => cloud_get_module_base() . '/images/spacer.gif',
    '#attributes' => array(
      'width' => 300,
      'height' => 1,
    ),
  );


  foreach ($result as $key) {

    $form['group_name'] = array(
      '#type' => 'hidden'  ,
      '#name' => 'group_name',
      '#value' => t( $group_name),
    );
    $form['fieldset_SG'] = array(
      '#type' => 'fieldset',
      '#title' => t('Security Group'),
    );

    $form['SG_details']['security_group_label'] = array(
      '#type' => 'item',
      '#title' => t('Security Group'),
    );
    $form['SG_details']['security_group_text'] = array(array('#markup' => check_plain( $group_name)));
    $form['SG_details']['security_group_blank'] = $spacer;

    $form['SG_details']['security_group_id_label'] = array(
      '#type' => 'item',
      '#title' => t('Security Group Id'),
    );
    $form['SG_details']['security_group_id_text'] = array(array('#markup' => check_plain( $key->group_id)));
    $form['SG_details']['security_group_id_blank'] = $spacer;

    $form['SG_details']['description_label'   ] = array(
      '#type' => 'item',
      '#title' => t('Description'),
    );
    $form['SG_details']['description_text'    ] = array(array('#markup' => check_plain( $key->group_description )));
    $form['SG_details']['description_blank'   ] = $spacer;

    $form['SG_details']['owner_label'         ] = array(
      '#type' => 'item',
      '#title' => t('Owner'),
    );
    $form['SG_details']['owner_id'            ] = array(array('#markup' => t( $key->owner_id )));
    $form['SG_details']['owner_blank'         ] = $spacer;

    $owner_id = $key->owner_id;
    $security_group_details = _aws_ec2_lib_get_security_groups_details_db( $cloud_context, $group_name );

    $cnt = 0;
    $tmp_table_rows = array();
    $rows = array();

    if (isset($security_group_details[$group_name]) ) {

      $sg_details_list = $security_group_details[$group_name];
      $form['fieldset_permissions'] = array(
        '#type' => 'fieldset',
        '#title' => t('Permissions'),
      );

      foreach ($sg_details_list as $sg_details) {

        $tmp_table_rows = array();
        $form['sg_details_data']['sg_details_' . $cnt]          = array();
        $form['sg_details_data']['sg_details_' . $cnt]          = array();
        $form['sg_details_data']['sg_details_' . $cnt]['label'] = array(
          '#type' => 'item',
          '#title' => t('Allow'),
        );

        $permissions = drupal_strtoupper( $sg_details['ip_protocol'] ) . ' ';

        if ( empty($sg_details['cidr_ip']) === FALSE ) {

          $permissions .= 'IPs: ' . $sg_details['cidr_ip'] . ' ';
        }

        if ( $sg_details['ip_protocol'] !== 'icmp' ) {

          if ($sg_details['from_port'] === $sg_details['to_port'] ) {

            $permissions .= 'Port: ' . $sg_details['from_port'] . ' ';
          }
          else {

            $permissions .= 'Ports: ' . $sg_details['from_port'] . ' .. ' . $sg_details['to_port'] . ' ';
          }
        }
        else { // If protocol is ICMP
          if ($sg_details['from_port'] === $sg_details['to_port'] && $sg_details['to_port'] === '-1') {

            $permissions .= 'Types: Any' . ' ';
          }
          else {

            $permissions .= 'Type: ' . $sg_details['from_port'] . ', Code: ' . $sg_details['to_port'] . ' ';
          }
        }

        if ( empty($sg_details['cidr_ip']) ) { // for group
          $permissions .= 'Owner: ' . $sg_details['user_id'] . ' Group: ' . $sg_details['tgt_group_name'] . ' ';
        }

        $form['sg_details_data']['sg_details_' . $cnt]['content'] = array(
          '#type' => 'item',
          '#markup' => $permissions,
        );
        if ( empty($txt_disabled) ) {

          $form['sg_details_data']['sg_details_' . $cnt]['button'] = array(
            array(
              '#markup' => l( t('Revoke'),
                  $cloud_menu_path . '/security_groups/revoke',
                  array(
                'query' => array(
                  'group_name' => urlencode($group_name),
                  'ip_protocol' => $sg_details['ip_protocol'   ],
                  'from_port' => $sg_details['from_port'     ],
                  'to_port' => $sg_details['to_port'       ],
                  'user_id' => $sg_details['user_id'       ],
                  'tgt_group_name' => $sg_details['tgt_group_name'],
                  'cidr_ip' => $sg_details['cidr_ip'       ],
                ),
              )
                ),
            ),
          );
        }
        $cnt++;
      } // end of foreach
    } // end of if
  } // end of while


  $form['fieldset_add_ips'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add IPs'),
  );

  // Add TCp/UDP details
  $form['SG_details']['AddIPs_TCP_label'] = array(
    '#type' => 'item',
    '#title' => t('TCP / UDP'),
    '#description' => t('Use 0.0.0.0/0 to designate "Any" IP address;<br />Use Ports 0 .. 65535 to designate "Any" Port'),
  );

  $tcp_ips = array(
    '#type' => 'fieldset',
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );

  $protocol_options = array(
    'tcp' => 'TCP',
    'udp' => 'UDP',
  );

  asort($protocol_options);
  $tcp_ips['protocol'       ] = array(
    '#type' => 'select'   ,
    '#options' => $protocol_options               ,
    '#default_value' => 'tcp'        ,
    '#disabled' => $txt_disabled,
  );
  $tcp_ips['tcp_IPs'        ] = array(
    '#type' => 'textfield',
    '#size' => '14',
    '#title' => t('IPs'  ),
    '#default_value' => '0.0.0.0/32',
    '#disabled' => $txt_disabled,
  );
  $tcp_ips['tcp_Ports_start'] = array(
    '#type' => 'textfield',
    '#size' => '4',
    '#title' => t('Ports'),
    '#default_value' => '0'          ,
    '#disabled' => $txt_disabled,
  );
  $tcp_ips['tcp_Ports_end'  ] = array(
    '#type' => 'textfield',
    '#size' => '4',
    '#field_prefix' => ' .. '    ,
    '#default_value' => '0'          ,
    '#disabled' => $txt_disabled,
  );

  $form['SG_details']['AddIPs_TCP_Content']   = $tcp_ips  ;

  if (user_access($cloud_context . ' setup security group') ) {
    $form['SG_details']['AddIPs_TCP_Button']   = array(
      '#type' => 'submit',
      '#name' => 'AddIPs_TCP_Button',
      '#value' => t('Add'),
    );
  }
  else {
    $form['SG_details']['AddIPs_TCP_Button']   = array(
      '#type' => 'item',
      '#markup' => '',
    );
  }

  // Add ICMP details

  $form['SG_details']['AddIPs_ICMP_label'] = array(
    '#type' => 'item',
    '#title' => t('ICMP'),
    '#description' => t('Use 0.0.0.0/0 to designate "Any" IP address;<br />Use Type: -1 and Code: -1 to designate "Any" Type/Code'),
  );

  $icmp_ips = array(
    '#type' => 'fieldset',
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );

  $icmp_ips['icmp_IPs'        ] = array(
    '#type' => 'textfield',
    '#size' => '14',
    '#title' => t('IPs' ),
    '#default_value' => '0.0.0.0/32',
    '#disabled' => $txt_disabled,
  );
  $icmp_ips['icmp_Ports_start'] = array(
    '#type' => 'textfield',
    '#size' => '4',
    '#title' => t('Type'),
    '#default_value' => '-1'         ,
    '#disabled' => $txt_disabled,
  );
  $icmp_ips['icmp_Ports_end'  ] = array(
    '#type' => 'textfield',
    '#size' => '4',
    '#title' => t('Code'),
    '#default_value' => '-1'         ,
    '#disabled' => $txt_disabled,
  );

  $form['SG_details']['AddIPs_ICMP_Content']   = $icmp_ips  ;

  if (user_access($cloud_context . ' setup security group') ) {
    $form['SG_details']['AddIPs_ICMP_Button'] = array(
      '#type' => 'submit',
      '#name' => 'AddIPs_ICMP_Button',
      '#value' => t('Add'),
    );
  }
  else {
    $form['SG_details']['AddIPs_ICMP_Button'] = array(
      '#type' => 'item',
      '#markup' => '',
    );
  }

  // Add Group details
  $form['SG_details']['AddIPs_Grp_label'    ] = array(
    '#type' => 'item',
    '#title' => t('Add Group'),
  );

  $grp = array(
    '#type' => 'fieldset',
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );

  $grp['grp_owner'] = array(
    '#type' => 'textfield',
    '#size' => '24',
    '#title' => t('Owner')                                ,
    '#disabled' => $txt_disabled,
  );
  $grp['grp'      ] = array(
    '#type' => 'textfield',
    '#size' => '24',
    '#title' => t('Group'),
    '#default_value' => 'default',
    '#disabled' => $txt_disabled,
  );

  $grp['grp_owner']['#default_value'] = ($owner_id)
                                      ?  $owner_id
                                      : '';
  $form['SG_details']['AddIPs_Grp_Content']   = $grp  ;

  if (user_access($cloud_context . ' setup security group') ) {
    $form['SG_details']['Add_Grp_Button'] = array(
      '#type' => 'submit',
      '#name' => 'Add_Grp_Button',
      '#value' => t('Add'),
    );
  }
  else {
    $form['SG_details']['Add_Grp_Button'] = array(
      '#type' => 'item',
      '#markup' => '',
    );
  }

  $form['submit_buttons'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<span class="clear"></span><div class="container-inline"><div class="buttons">',
    '#suffix' => '</div></div>',
  );

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  if (user_access($cloud_context . ' delete security group') ) {

    $location    = check_url(url($cloud_menu_path . '/security_groups/delete')) . '&sg_name=' . urlencode($group_name);
    $action      = "document.forms[0].action='" . $location . "';";
    $confirm_msg = cloud_get_messagebox(t('Are you sure you want to delete the Security Group "@group_name" ?', array('@group_name' => $group_name)), $action );
    $form['submit_buttons']['Delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#attributes' => array('onclick' => $confirm_msg),
    );
  }

  $form['submit_buttons']['SG_List'] = array(
    '#type' => 'submit',
    '#value' => t('List Security Groups'),
  );

  return $form;
}


/**
 *
 * @param     $form
 * @return
 */
function theme_aws_ec2_lib_display_security_group($form) {


  if (isset($form['form'])) { // drupal 7 passed form as an array
    $form = $form['form'];
  }

  $rows = array(
    array(
      drupal_render($form['SG_details']['security_group_label']),
      drupal_render($form['SG_details']['security_group_text']),
      drupal_render($form['SG_details']['security_group_blank']),
    ),
    array(
      drupal_render($form['SG_details']['security_group_id_label']),
      drupal_render($form['SG_details']['security_group_id_text']),
      drupal_render($form['SG_details']['security_group_id_blank']),
    ),
    array(
      drupal_render($form['SG_details']['description_label'   ]),
      drupal_render($form['SG_details']['description_text'    ]),
      drupal_render($form['SG_details']['description_blank'   ]),
    ),
    array(
      drupal_render($form['SG_details']['owner_label'         ]),
      drupal_render($form['SG_details']['owner_id'            ]),
      drupal_render($form['SG_details']['owner_blank'         ]),
    ),
  );

  $table = theme('table', array('header' => NULL, 'rows' => $rows));
  $form['fieldset_SG']['#children'] = $table;

  //cf. Waning by Coder module: Potential problem: when FAPI element '#type' is set to 'markup' (default), '#value' only accepts filtered text, be sure to use check_plain(), filter_xss() or similar to ensure your $variable is fully sanitized.
  $form['fieldset_SG']['list'] = array(
    '#type' => 'markup',
    '#value' => $table,
  );

  if (isset($form['sg_details_data']) ) {

    $cnt = 0;
    $rows = array();

    foreach ($form['sg_details_data'] as $sg_key => $details_data) {

      if (strpos($sg_key, 'sg_details' ) === FALSE ) {

        continue;
      }

      $rows[] = array(
        drupal_render($form['sg_details_data']['sg_details_' . $cnt]['label'   ]),
        drupal_render($form['sg_details_data']['sg_details_' . $cnt]['content']),
        drupal_render($form['sg_details_data']['sg_details_' . $cnt]['button'  ]),
      );
      $cnt++;
    }
    $table = theme('table', array('header' => NULL, 'rows' => $rows));
    $form['fieldset_permissions']['#children'] = $table;

    // cf. Waning by Coder module: Potential problem: when FAPI element '#type' is set to 'markup' (default), '#value' only accepts filtered text, be sure to use check_plain(), filter_xss() or similar to ensure your $variable is fully sanitized.
    // $form['fieldset_permissions']['list'] = array('#type' => 'markup', '#value' => $table);
  }

  $rows = array(
    array(
      drupal_render($form['SG_details']['AddIPs_TCP_label'   ]),
      drupal_render($form['SG_details']['AddIPs_TCP_Content']),
      drupal_render($form['SG_details']['AddIPs_TCP_Button'  ]),
    ),
    array(
      drupal_render($form['SG_details']['AddIPs_ICMP_label'  ]),
      drupal_render($form['SG_details']['AddIPs_ICMP_Content']),
      drupal_render($form['SG_details']['AddIPs_ICMP_Button']),
    ),
    array(
      drupal_render($form['SG_details']['AddIPs_Grp_label'   ]),
      drupal_render($form['SG_details']['AddIPs_Grp_Content']),
      drupal_render($form['SG_details']['Add_Grp_Button'     ]),
    ),
  );

  $table = theme('table', array('header' => NULL, 'rows' => $rows));
  $form['fieldset_add_ips']['#children'] = $table;

  //cf. Waning by Coder module: Potential problem: when FAPI element '#type' is set to 'markup' (default), '#value' only accepts filtered text, be sure to use check_plain(), filter_xss() or similar to ensure your $variable is fully sanitized.
  //$form['fieldset_add_ips']['list'] = array('#type' => 'markup', '#value' => $table);

  $output  = drupal_render($form['cloud_context'       ]);
  $output .= drupal_render($form['operation_type'      ]);
  $output .= drupal_render($form['submit_buttons'      ]);
  $output .= drupal_render($form['fieldset_SG'         ]);
  $output .= drupal_render($form['fieldset_permissions']);
  $output .= drupal_render($form['fieldset_add_ips'    ]);

  $output .= drupal_render_children($form);

  return $output;
}

/**
 *
 * @param     $form_id
 * @param     $form_values
 * @return
 */
function aws_ec2_lib_display_security_group_submit($form_id, $form_values) {

  $form_values   = $form_values['values'       ];
  $cloud_context = $form_values['cloud_context'];
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if ( $form_values['operation_type'] === 'AddIPs_TCP_Button' ) {

    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('TCP IP(s) have been added: @TCP_IPs', array('@TCP_IPs' => $form_values['tcp_IPs'])),
      'link' => '',
    )
    );

    // aws_ec2_api_authorize_security_group_ingress($name, $srcName, $srcOwner, $protocol = NULL, $from_port = NULL, $to_port = NULL, $srcIp = NULL) {
    $result = aws_ec2_api_authorize_security_group_ingress( $cloud_context, $form_values['group_name'],    '', '', $form_values['protocol'], $form_values['tcp_Ports_start'], $form_values['tcp_Ports_end'], $form_values['tcp_IPs']  );

  }
  elseif ( $form_values['operation_type'] === 'AddIPs_ICMP_Button' ) {

    // User Activity Log
    cloud_audit_user_activity( array(
      'type' => 'user_activity',
      'message' => t('ICMP IP(s) have been added: @ICMP_IP', array('@ICMP_IP' => $form_values['icmp_IPs'])),
      'link' => '',
    )
    );

    $result = aws_ec2_api_authorize_security_group_ingress( $cloud_context, $form_values['group_name'],    '', '', 'icmp', $form_values['icmp_Ports_start'], $form_values['icmp_Ports_end'], $form_values['icmp_IPs']  );
  }
  elseif ($form_values['operation_type'] === 'Add_Grp_Button' ) {

    $result = aws_ec2_api_authorize_security_group_ingress( $cloud_context, $form_values['group_name'], $form_values['grp'], $form_values['grp_owner'] );
  }
  elseif (isset($form_values['op']) && $form_values['op'] === t('List Security Groups') ) {

    drupal_goto( $cloud_menu_path . '/security_groups' );
    return;
  }

  if ( isset($result) && $result->code == 200 ) {
    _aws_ec2_lib_download_describe_security_groups($cloud_context);
    drupal_set_message(t('Updated: Security Group(s)') );
  }
  else {
    drupal_set_message(t('Error while updating Security Group'), 'error' );
  }


  drupal_goto( $cloud_menu_path . '/security_groups/describe', array('query' => array('group_name' => $form_values['group_name'])));

  return;
}


/**
 *  Create Security Group
 *
 * @param     $form_submit
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_create_security_group($form_submit, $form_state = '', $cloud_context) {

  $form['fieldset_security_group_info'        ] = array('#type' => 'fieldset' /* '#title' => t('Security Group Info') */);

  $form['cloud_context'    ] = array(
    '#type' => 'hidden',
    '#value' => $cloud_context,
  );

  $form['GroupName_details']['GroupName_label'] = array(
    '#type' => 'item',
    '#title' => t('Security Group name'),
    '#required' => TRUE,
  );
  $form['GroupName_details']['GroupName_text'] = array('#type' => 'textfield');

  $form['GroupName_details']['GroupDesc_label'] = array(
    '#type' => 'item',
    '#title' => t('Security Group Description'),
    '#required' => TRUE,
  );
  $form['GroupName_details']['GroupDesc_text'] = array('#type' => 'textfield');

  $form['submit_buttons'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<span class="clear"></span><div class="container-inline"><div class="buttons">',
    '#suffix' => '</div></div>',
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
 * @param     $form
 * @return
 */
function theme_aws_ec2_lib_create_security_group($form) {


  if (isset($form['form'])) { // drupal 7 passed form as an array
    $form = $form['form'];
  }

  $rows = array(
    array(
      drupal_render($form['GroupName_details']['GroupName_label']),
      drupal_render($form['GroupName_details']['GroupName_text']),
    ),
    array(
      drupal_render($form['GroupName_details']['GroupDesc_label']),
      drupal_render($form['GroupName_details']['GroupDesc_text']),
    ),
  );

  $table = theme('table', array('header' => NULL, 'rows' => $rows));
  $form['fieldset_security_group_info']['#children'] = $table;

  //cf. Waning by Coder module: Potential problem: when FAPI element '#type' is set to 'markup' (default), '#value' only accepts filtered text, be sure to use check_plain(), filter_xss() or similar to ensure your $variable is fully sanitized.
  //$form['fieldset_security_group_info']['list'] = array('#type' => 'markup', '#value' => $table);

  $output  = drupal_render($form['fieldset_security_group_info']);
  $output .= drupal_render($form['submit_buttons']);
  $output .= drupal_render($form['cloud_context']);

  $output .= drupal_render_children($form);

  return $output;
}


/**
 *
 * @param     $form_id
 * @param     $form_values
 * @return
 */
function aws_ec2_lib_create_security_group_validate($form_id, $form_values) {

  $form_values = $form_values['values'];
  if ( $form_values['op'] == t('Create') ) {

    $sg_name = trim( $form_values['GroupName_text'] );
    if ( empty($sg_name) ||    empty($form_values['GroupDesc_text']) ) {

      form_set_error('',    t('You must enter valid Group Name & description'));
    }
  }

  return;
}


/**
 *
 * @param     $form_id
 * @param     $form_values
 * @return
 */
function aws_ec2_lib_create_security_group_submit($form_id, $form_values) {

  $form_values     = $form_values['values'];
  $cloud_context   = $form_values['cloud_context'];
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if ( $form_values['op'] == t('Cancel') ) {

    drupal_goto( $cloud_menu_path . '/security_groups');

    return;
  }
  elseif ($form_values['op'] == t('Create')) {

    $sg_name  = trim($form_values['GroupName_text']);
    $sg_desc  = trim($form_values['GroupDesc_text']);
    $myclient = aws_ec2_api_create_security_group($cloud_context, $sg_name, $sg_desc );
    $result   = _aws_ec2_lib_save_new_security_group($cloud_context, $myclient, $sg_name, $sg_desc );

    if ($result) {
      drupal_set_message(t('Created a Security Group Successfully.'));
      drupal_goto($cloud_menu_path . '/security_groups/describe', array('query' => array('group_name' => $sg_name)));
      return;
    }
    else {
      drupal_set_message(t('Unable to create Security Group'),    'error' );

      // Check whether the name already exists
      $query = _aws_ec2_lib_get_security_group_query($cloud_context);
      $query_args = array(
        ':group_name' => $sg_name,
        ':cloud_type' => $cloud_context,
      );


      $result = db_query( $query, $query_args );

      foreach ($result as $key) {

        drupal_set_message( t('Security Group already exists: @sg_name', array('@sg_name' => $sg_name) ),    'error' );
      }
    }
  }
  return;
}


/**
 * Delete Security Group
 *
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_delete_security_group($cloud_context) {

  $sg_name = isset( $_REQUEST['sg_name'] ) ? $_REQUEST['sg_name'] : FALSE;

  $result = aws_ec2_api_delete_security_group($cloud_context, $sg_name);
  if ( _aws_ec2_lib_delete_security_group($cloud_context, $result, $sg_name ) ) {

    drupal_set_message( t('Security Group deleted successfully: @sg_name', array('@sg_name' => $sg_name) ) );
  }
  else {

    drupal_set_message( check_plain( t('Unable to delete the Security Group: @sg_name', array('@sg_name' => $sg_name) ) ), 'error' );
  }

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  drupal_goto($cloud_menu_path . '/security_groups');

  return;
}



/**
 * Revoke Security Group
 *
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_revoke_security_group($cloud_context) {

  $sg_name      = isset( $_REQUEST['group_name'   ] ) ? $_REQUEST['group_name'    ] : FALSE;
  $src_name     = isset($_REQUEST['tgt_group_name'] ) ? $_REQUEST['tgt_group_name'] : ''    ;
  $src_owner    = isset($_REQUEST['user_id'       ] ) ? $_REQUEST['user_id'       ] : ''    ;
  $protocol     = isset($_REQUEST['ip_protocol'   ] ) ? $_REQUEST['ip_protocol'   ] : ''    ;
  $from_port    = isset($_REQUEST['from_port'     ] ) ? $_REQUEST['from_port'     ] : ''    ;
  $to_port      = isset($_REQUEST['to_port'       ] ) ? $_REQUEST['to_port'       ] : ''    ;
  $src_ip       = isset($_REQUEST['cidr_ip'       ] ) ? $_REQUEST['cidr_ip'       ] : NULL  ;


  // GET /?Action=RevokeSecurityGroupIngress&
  // GroupName=Test_Jamir&
  // SourceSecurityGroupName=Test-Jamir-2&
  // SourceSecurityGroupOwnerId=409856118832&
  // IpProtocol=tcp&
  // FromPort=2&
  // ToPort=7&
  // CidrIp=&Version=2010-11-15&AWSAccessKeyId=AKIAIIKWKEERTNARISFA&SignatureVersion=1&SignatureMethod=HmacSHA1&Timestamp=2011-02-03T01%3A20%3A29Z&Signature=vraqmd49bYU2caLVe23jZjYQgKw%3D HTTP/1.0 Host: ec2.amazonaws.com User-Agent: Drupal (+http://drupal.org/) [data] => InvalidParameterCombinationThe parameter 'SourceSecurityGroupOwnerId' may not be used in combination with 'FromPort'1d8c9b0b-5c0c-488e-a062-4d2ca34d2941 [protocol] =>
  $result = aws_ec2_api_revoke_security_group_ingress($cloud_context,
                                                      $sg_name       ,
                                                      $src_name      ,
                                                      $src_owner     ,
                                                      $protocol      ,
                                                      $from_port     ,
                                                      $to_port       ,
                                                      $src_ip
                                                   );

  if ( $result->code == 200  ) {

    drupal_set_message( t('Updated Security Group successfully: @sg_name', array('@sg_name' => $sg_name) ) );
    _aws_ec2_lib_download_describe_security_groups($cloud_context); // Fetch the latest data from server
  }
  else {

    drupal_set_message( t('There was a problem while updating Security Group: @sg_name', array('@sg_name' => $sg_name) ), 'error' );
  }

  $cloud_menu_path = cloud_get_menu_path($cloud_context );
  drupal_goto( $cloud_menu_path . '/security_groups/describe', array('query' => array('group_name' => $sg_name)));

  return;
}


/**
 * Display List of Security Groups
 *
 * @param     $form_submit
 * @param     $cloud_context
 * @return
 */
function aws_ec2_lib_display_security_group_list($form_submit = '', $form_state, $cloud_context) {

  drupal_add_js(cloud_get_module_base() . 'js/cloud.js');

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

  //$filter  = cloud_get_filter_value( $form_submit, 'filter' );
  //$filter  = trim($filter);
  $filter     = isset($_REQUEST['filter'   ]) ? $_REQUEST['filter'   ] : '';
  $filter     = trim($filter);

  $sql_col = 'group_name';

  if ( isset($filter) ) {

    $query_args[] = $sql_col;
    $query_args[] = $filter;
    $query_args[] = $cloud_context;
  }

  $security_group_count = _aws_ec2_lib_get_security_groups_list_count($cloud_context);
  $filter_disabled      = '';
  if ( $security_group_count < 2 ) {

    $filter_disabled = TRUE;
  }

  $form['options']['label'] = array(
    '#type' => 'item'     ,
    '#title' => t('Filter'),
  );
  $form['options']['filter'] = array(
    '#type' => 'textfield',
    '#size' => 40,
    '#default_value' => $filter,
    '#disabled' => $filter_disabled,
  );

  $form['options']['submit'] = array(
    '#type' => 'submit'   ,
    '#value' => t('Apply')                      ,
    '#disabled' => $filter_disabled,
  );

  if (user_access($cloud_context . ' register security group') ) {

    $form['options']['new_key'] = array(
      '#type' => 'submit',
      '#value' => t('Create'),
    );
  }

  $form['header'] = array(
    '#type' => 'value',
    '#value' => array(
      array(
        'data' => t('Name'       ),
        'field' => 'group_name'         ,
        //'class' => 'nickname-column'    ,
        'sort' => 'asc',
      ),
      array(
        'data' => t('Description'),
        'field' => 'group_description',
      ),
      array(
        'data' => t('Owner'      ),
        'field' => 'owner_id',
      ),
      array(
        'data' => t('Permissions'),
        'field' => 'permissions',
      ),
      array(
        'data' => t('Actions'    ),
        'class' => 'action-column',
      ),
    ),
  );


  $query    = _aws_ec2_lib_get_describe_security_groups_query($cloud_context);

  $query
      ->condition('cloud_type', $cloud_context, '=')
      ->condition($sql_col, '%' . $filter . '%', 'like')
      ->extend('TableSort')
      ->orderByHeader($form['header']['#value']);

  $result = $query->execute();
  $destination     = drupal_get_destination();
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  $security_group_details   = _aws_ec2_lib_get_security_groups_details_db( $cloud_context );
  $sg_permissions           = array();
  $cnt = 0;
  foreach ($security_group_details as $sg_key => $sg_group_data) {

    $permissions = '';
    foreach ($sg_group_data as $sg_details) {

      $permissions .= drupal_strtoupper( $sg_details['ip_protocol'] ) . ' ';
      if ( empty($sg_details['cidr_ip']) === FALSE ) {

        $permissions .= 'IPs: ' . $sg_details['cidr_ip'] . ' ';
      }

      if ( $sg_details['ip_protocol'] !== 'icmp' ) {
        if ($sg_details['from_port'] === $sg_details['to_port'] ) {

          $permissions .= 'Port: ' . $sg_details['from_port'] . ' ';
        }
        else {

          $permissions .= 'Ports: ' . $sg_details['from_port'] . ' .. ' . $sg_details['to_port'] . ' ';
        }
      }
      else { // If protocol is ICMP
        if ($sg_details['from_port'] === $sg_details['to_port'] && $sg_details['to_port'] === '-1') {

          $permissions .= 'Types: Any' . ' ';
        }
        else {

          $permissions .= 'Type: ' . $sg_details['from_port'] . ', Code:' . $sg_details['to_port'] . ' ';
        }
      }

      if ( empty($sg_details['cidr_ip']) ) { // for group
        $permissions .= 'Owner:' . $sg_details['user_id'] . ' Group: ' . $sg_details['tgt_group_name'] . ' ';
      }

      $permissions .= '<br />'; // New Line
    }

    $sg_permissions[$sg_key] = $permissions;
  }


  foreach ($result as $sg) {

    // $form['Name'][$sg->group_name] = array( array('#value' => t( $sg->group_name ) ) );
    $form['Name'       ][$sg->group_name] = array(array('#markup' => l( $sg->group_name, $cloud_menu_path . '/security_groups/describe',    array('query' => array('group_name' => $sg->group_name)) ))); //,  array(),  'group_name=' . urlencode($sg->group_name) ,  NULL,  FALSE,  TRUE ) ) );
    $form['hdnName'    ][$sg->group_name] = array(
      '#type' => 'hidden',
      '#value' => addslashes($sg->group_name),
    );
    $form['Description'][$sg->group_name] = array(array('#markup' => check_plain( $sg->group_description )));
    $form['Owner'      ][$sg->group_name] = array(array('#markup' => t( $sg->owner_id )));
    if (isset($sg_permissions[$sg->group_name])) {
      $form['Permissions'][$sg->group_name] = array(array('#markup' => t( $sg_permissions[$sg->group_name] )));
    }
    else {
      $form['Permissions'][$sg->group_name] = array(array('#markup' => ''));
    }
  }

  $form['pager'    ] = array('#value' => theme('pager', array('tags' => NULL, 'element' => 0)));
  $form_state['#redirect']  = FALSE;
  $form['#action'] = url($_REQUEST['q']);

  return $form;
}


/**
 *
 * @param     $form
 * @return
 */
function theme_aws_ec2_lib_display_security_group_list($form) {


  if (isset($form['form'])) { // drupal 7 passed form as an array
    $form = $form['form'];
  }

  $cloud_context   = $form['cloud_context']['#value'];
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  $form['Name'           ] = !empty($form['Name'           ]) ? $form['Name'           ] : array();
  $form['pager']['#value'] = !empty($form['pager']['#value']) ? $form['pager']['#value'] : '';

  $rows = array();
  foreach (element_children($form['Name']) as $sg) {
    $row = array(
      //  drupal_render($form['Name'][$sg]);
      array(
        'data' => drupal_render($form['Name'       ][$sg]),
        //'class' => 'nickname-column',
      ),
      drupal_render($form['Description'][$sg]),
      drupal_render($form['Owner'      ][$sg]),
      drupal_render($form['Permissions'][$sg]),
    );

    $action_data = '';
    $prop['onclick'] = cloud_get_messagebox('Are you sure you want to delete the Security Group "' . $form['hdnName'][$sg]['#value'] . '" ?');
    if (user_access($cloud_context . ' delete security group') ) {

      $action_data = cloud_display_action( 'images/icon_delete', t('Delete'), $cloud_menu_path . '/security_groups/delete',    array('query' => array('sg_name' => $sg), 'html' => TRUE), $prop['onclick']); //,   'sg_name=' . urlencode($sg) , $prop);
    }

    if (user_access($cloud_context . ' setup security group') ) {

      $action_data .= cloud_display_action( 'images/icon_edit', t('Edit'), $cloud_menu_path . '/security_groups/describe', array('query' => array('group_name' => $sg), 'html' => TRUE));
    }

    if ( empty($action_data) ) {

      $row[] = '';
    }
    else {

      $row[] = array(
        'data' => $action_data,
        'class' => 'action-column',
      );
    }

    $rows[] = $row;
  }

  $output  = drupal_render($form['new_key']);
  $output .= drupal_render($form['options']);
  $output .= theme('table', array('header' => $form['header']['#value'], 'rows' => $rows));

  if ($form['pager']['#value']) {

    $output .= drupal_render($form['pager']);
  }

  $reload_link = l( t('- Refresh Page -'),
                  $cloud_menu_path . '/get_security_group_data',
                  array() );
  $ref_link = array(
    '#type' => 'item',
    '#prefix' => '<div id="link_reload" align="right">',
    '#suffix' => '</div>',
    '#markup' => $reload_link,
  );

  $output .= drupal_render($ref_link             );
  $output .= drupal_render($form['cloud_context']);
  $output .= drupal_render_children($form);

  return $output;
}


/**
 *
 * @param     $form_id
 * @param     $form_values
 * @return
 */
function aws_ec2_lib_display_security_group_list_submit($form_id, &$form_values) {

  $form_values     = $form_values['values'       ];
  $cloud_context   = $form_values['cloud_context'];
  $cloud_menu_path = cloud_get_menu_path($cloud_context );

  if ( $form_values['op'] == t('Create')) {

    return drupal_goto( $cloud_menu_path . '/security_groups/create');
  }
  elseif ( $form_values['op'] == t('Apply') ) {

    $param_arr = array(
      'filter' => $form_values['filter'],
    );

    $destination = drupal_get_destination();
    $form_values['redirect']     = array($destination['destination'], array('query' => $param_arr));
  }

  return;
}
